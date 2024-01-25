/**
 * Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>
#include <math.h>
#include <stdlib.h>
#include "gapi_isooshm.h"
#include "alif_lc3.h"
#include "audio_source_simulated.h"

LOG_MODULE_REGISTER(audio_source_simulated, CONFIG_AUDIO_SOURCE_SIMULATED_LOG_LEVEL);

/* TODO: define somewhere common */
#define AUDIO_FRAME_LENGTH (CONFIG_LE_AUDIO_SAMPLING_FREQUENCY_HZ / 100)

static int16_t audio_frame[AUDIO_FRAME_LENGTH];
static gapi_isooshm_dp_t dp;
static lc3_cfg_t lc3_config;
static lc3_encoder_t lc3_encoder;
static int32_t *p_encoder_scratch;
static gapi_isooshm_sdu_buf_t *p_sdu_buf;
static K_SEM_DEFINE(frame_transmitted_sem, 0, 1);

static int populate_audio_frame(void)
{
	/* Populate the audio frame with a constant tone. Since we are going to send out this frame
	 * repeatedly, the frequency of the chosen tone should be a multiple of the frame rate, to
	 * avoid any discontinuities in the audio signal. So with 10 ms frames, the frame rate is
	 * 100 Hz, and we can choose 400 Hz for the audio tone.
	 */
	const float freq_hz = 400.0f;
	const float amplitude = 10000.0f;
	const float pi = 3.14159265359f;

	for (uint32_t i = 0; i < AUDIO_FRAME_LENGTH; i++) {
		audio_frame[i] = amplitude * sin((2.0f * pi * freq_hz * i) /
						 CONFIG_LE_AUDIO_SAMPLING_FREQUENCY_HZ);
	}

	return 0;
}

static int audio_source_simulated_init(const struct device *dev)
{
	(void)dev;

	populate_audio_frame();

	/* Initialise and configure LC3 codec */
	int ret = alif_lc3_init();

	if (ret) {
		LOG_ERR("Failed to initialise LC3 ROM, err %d", ret);
		return ret;
	}

	ret = lc3_api_configure(&lc3_config, CONFIG_LE_AUDIO_SAMPLING_FREQUENCY_HZ,
				FRAME_DURATION_10_MS);
	if (ret) {
		LOG_ERR("Failed to configure LC3 codec, err %d", ret);
		return ret;
	}

	p_encoder_scratch = malloc(lc3_api_encoder_scratch_size(&lc3_config));
	if (p_encoder_scratch == NULL) {
		LOG_ERR("Failed to allocate LC3 encoder scratch memory");
		return -1;
	}

	ret = lc3_api_initialise_encoder(&lc3_config, &lc3_encoder);
	if (ret) {
		LOG_ERR("Failed to initialise LC3 encoder, err %d", ret);
		return ret;
	}

	/* Allocate SDU buffer */
	p_sdu_buf = malloc(sizeof(gapi_isooshm_sdu_buf_t) + CONFIG_LE_AUDIO_OCTETS_PER_CODEC_FRAME);
	if (p_sdu_buf == NULL) {
		LOG_ERR("Failed to allocate SDU buffer");
		return -1;
	}

	return 0;
}
SYS_INIT(audio_source_simulated_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

static void on_dp_transfer_complete(gapi_isooshm_dp_t *p_dp, gapi_isooshm_sdu_buf_t *p_buf)
{
	k_sem_give(&frame_transmitted_sem);
}

static void audio_source_simulated_thread_func(void *p1, void *p2, void *p3)
{
	uint16_t seq_num = 0;

	while (1) {
		/* Wait until the previous frame has been transmitted */
		k_sem_take(&frame_transmitted_sem, K_FOREVER);

		/* Copy the (constant) audio data into a new static buffer. The LC3 codec overwrites
		 * the input data, and we want to keep it intact for the next frame
		 */
		static int16_t tmp_audio_frame[AUDIO_FRAME_LENGTH];

		memcpy(tmp_audio_frame, audio_frame, sizeof(audio_frame));

		/* Encode the next frame into SDU buffer */
		int ret = lc3_api_encode_frame(
			&lc3_config, &lc3_encoder, tmp_audio_frame, p_sdu_buf->data,
			CONFIG_LE_AUDIO_OCTETS_PER_CODEC_FRAME, p_encoder_scratch);
		if (ret) {
			LOG_ERR("Failed to encode frame, err %d", ret);
		}

		/* Send encoded frame to ISO data path */
		p_sdu_buf->seq_num = seq_num++;
		p_sdu_buf->has_timestamp = false;
		p_sdu_buf->sdu_len = CONFIG_LE_AUDIO_OCTETS_PER_CODEC_FRAME;
		uint16_t err = gapi_isooshm_dp_set_buf(&dp, p_sdu_buf);

		if (err) {
			LOG_ERR("Failed to send frame to ISO data path, err %u", err);
		}

		if ((seq_num % 128) == 0) {
			LOG_DBG("SDU sequence number: %u", seq_num);
		}
	}
}
K_THREAD_DEFINE(audio_source_simulated_thread, CONFIG_MAIN_STACK_SIZE,
		audio_source_simulated_thread_func, NULL, NULL, NULL,
		CONFIG_ALIF_BLE_HOST_THREAD_PRIORITY, 0, 0);

int audio_source_simulated_start(uint8_t stream_lid)
{
	uint16_t err = gapi_isooshm_dp_init(&dp, on_dp_transfer_complete);

	if (err) {
		LOG_ERR("Failed to initialise gapi_isooshm data path, err %u", err);
		return -1;
	}

	err = gapi_isooshm_dp_bind(&dp, stream_lid, GAPI_DP_DIRECTION_INPUT);
	if (err) {
		LOG_ERR("Failed to bind gapi_isooshm data path, err %u", err);
		return -1;
	}

	/* Start encoding of the first frame. It will be sent to data path when encoding is complete
	 */
	k_sem_give(&frame_transmitted_sem);

	return 0;
}
