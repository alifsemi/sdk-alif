/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/__assert.h>
#include <zephyr/sys/check.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

#include <stdlib.h>

#include "alif_lc3.h"
#include "lc3_api.h"
#include "audio_queue.h"
#include "sdu_queue.h"
#include "gapi_isooshm.h"
#include "audio_decoder.h"

LOG_MODULE_REGISTER(audio_decoder, CONFIG_BLE_AUDIO_LOG_LEVEL);

struct cb_list {
	audio_decoder_sdu_cb_t cb;
	void *context;
	struct cb_list *next;
};

struct audio_decoder {
	/* Decoder thread */
	struct k_thread thread;
	k_thread_stack_t *stack;
	k_tid_t tid;
	bool thread_abort;

	/* LC3 configuration, decoder instances and memory */
	lc3_cfg_t lc3_cfg;
	lc3_decoder_t *lc3_decoder[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
	int32_t *lc3_status[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
	int32_t *lc3_scratch;

	/* Linked list of registered callbacks */
	struct cb_list *cb_list;

	/* Input and output queues */
	struct sdu_queue *sdu_queue[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
	struct audio_queue *audio_queue;
};

static void audio_decoder_thread_func(void *p1, void *p2, void *p3)
{
	struct audio_decoder *dec = (struct audio_decoder *)p1;
	(void)p2;
	(void)p3;

	int ret;
	struct audio_block *audio;
	gapi_isooshm_sdu_buf_t *p_sdu;
	size_t const half_audio_block_samples = dec->audio_queue->audio_block_samples / 2;
	uint16_t last_sdu_seq = 0;

	while (!dec->thread_abort) {
		/* Get a free audio block to decode into */
		audio = NULL;
		ret = k_mem_slab_alloc(&dec->audio_queue->slab, (void **)&audio, K_FOREVER);
		__ASSERT(ret == 0, "mem slab alloc failed");

		for (int i = 0; i < ARRAY_SIZE(dec->sdu_queue); i++) {
			/* Decode each channel only if it is present */
			if (!dec->sdu_queue[i]) {
				continue;
			}

			/* Left channel should be decoded into first half of audio buffer,
			 * right channel into second half
			 */
			int16_t *const p_audio_data = audio->buf + half_audio_block_samples * i;

			/* Get an SDU */
			p_sdu = NULL;

			ret = k_msgq_get(&dec->sdu_queue[i]->msgq, &p_sdu, K_FOREVER);
			__ASSERT(ret == 0, "msgq get failed");

			/* A NULL SDU can be sent to the queue to wake up and abort the
			 * thread. Stop processing for this loop and check thread abort
			 * flag.
			 */
			if (!p_sdu) {
				break;
			}

			uint8_t const bad_frame = (p_sdu->status != ISOOSHM_SDU_STATUS_VALID);

			if (bad_frame) {
				LOG_WRN("Bad frame received, SDU status: %u", p_sdu->status);
			}

			uint8_t bec_detect;

			ret = lc3_api_decode_frame(&dec->lc3_cfg, dec->lc3_decoder[i], p_sdu->data,
						   p_sdu->sdu_len, bad_frame, &bec_detect,
						   p_audio_data, dec->lc3_scratch);
			if (ret != 0) {
				LOG_ERR("LC3 decoding failed on channel %d with err %d", i, ret);
				/* Fill with silence on decode error */
				memset(p_audio_data, 0, half_audio_block_samples * sizeof(int16_t));
			}

			if (bec_detect) {
				LOG_WRN("Corrupted input frame is detected");
			}

			audio->timestamp = p_sdu->timestamp;
			last_sdu_seq = p_sdu->seq_num;

			/* SDU is no longer needed, free it */
			k_mem_slab_free(&dec->sdu_queue[i]->slab, p_sdu);
		}

		/* If the right channel is not present, copy the data from the left channel */
		if (!dec->sdu_queue[1]) {
			int16_t *dst = audio->buf + half_audio_block_samples;

			memcpy(dst, audio->buf, half_audio_block_samples * sizeof(int16_t));
		}

		uint32_t const timestamp = audio->timestamp;

		/* Push the audio data to queue */
		ret = k_msgq_put(&dec->audio_queue->msgq, (void **)&audio, K_FOREVER);
		__ASSERT(ret == 0, "msgq put failed");

		/* Notify listeners that a block is completed */
		struct cb_list *cb_item = dec->cb_list;

		while (cb_item) {
			if (cb_item->cb) {
				cb_item->cb(cb_item->context, timestamp, last_sdu_seq);
			}
			cb_item = cb_item->next;
		}
	}

	LOG_DBG("Decoder thread finished");
}

struct audio_decoder *audio_decoder_create(uint32_t const sampling_frequency,
					   k_thread_stack_t *stack, size_t const stacksize,
					   struct sdu_queue *p_sdu_queues[],
					   size_t const num_queues, struct audio_queue *audio_queue,
					   lc3_frame_duration_t frame_duration)
{
	if (!stack) {
		LOG_ERR("Thread stack must be provided");
		return NULL;
	}

	if (!audio_queue) {
		LOG_ERR("Audio queue must be provided");
		return NULL;
	}

	if (!p_sdu_queues || !num_queues) {
		LOG_ERR("At least one SDU queue must be provided");
		return NULL;
	}

	if (CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS < num_queues) {
		LOG_ERR("Too many queues!");
		return NULL;
	}

	CHECKIF(!((frame_duration == FRAME_DURATION_10_MS) ||
		  (frame_duration == FRAME_DURATION_7_5_MS))) {
		LOG_ERR("Invalid frame duration!");
		return NULL;
	}

	struct audio_decoder *dec = (struct audio_decoder *)calloc(1, sizeof(struct audio_decoder));

	if (!dec) {
		LOG_ERR("Failed to allocate audio decoder");
		return NULL;
	}

	uint8_t num_valid = 0;

	for (uint8_t iter = 0; iter < num_queues; iter++) {
		num_valid += !!p_sdu_queues[iter];
		dec->sdu_queue[iter] = p_sdu_queues[iter];
	}
	dec->audio_queue = audio_queue;

	/* Configure LC3 codec and allocate required memory */
	int ret = lc3_api_configure(&dec->lc3_cfg, (int32_t)sampling_frequency, frame_duration);

	if (ret) {
		LOG_ERR("Failed to configure LC3 codec, err %d", ret);
		audio_decoder_delete(dec);
		return NULL;
	}

	size_t const scratch_size = lc3_api_decoder_scratch_size(&dec->lc3_cfg);

	dec->lc3_scratch = (int32_t *)malloc(scratch_size);
	if (!dec->lc3_scratch) {
		LOG_ERR("Failed to allocate decoder scratch memory");
		audio_decoder_delete(dec);
		return NULL;
	}

	size_t const status_size = lc3_api_decoder_status_size(&dec->lc3_cfg);

	for (int i = 0; i < num_valid; i++) {
		dec->lc3_decoder[i] = (lc3_decoder_t *)malloc(sizeof(lc3_decoder_t));
		if (!dec->lc3_decoder[i]) {
			LOG_ERR("Failed to allocate LC3 decoder");
			audio_decoder_delete(dec);
			return NULL;
		}

		dec->lc3_status[i] = (int32_t *)malloc(status_size);
		if (!dec->lc3_status[i]) {
			LOG_ERR("Failed to allocate LC3 status memory");
			audio_decoder_delete(dec);
			return NULL;
		}

		ret = lc3_api_initialise_decoder(&dec->lc3_cfg, dec->lc3_decoder[i],
						 dec->lc3_status[i]);
		if (ret) {
			LOG_ERR("Failed to initialise LC3 decoder %d, err %d", i, ret);
			audio_decoder_delete(dec);
			return NULL;
		}
	}

	/* Create and start thread */
	dec->tid = k_thread_create(&dec->thread, stack, stacksize, audio_decoder_thread_func, dec,
				   NULL, NULL, CONFIG_ALIF_BLE_HOST_THREAD_PRIORITY, 0, K_NO_WAIT);

	if (!dec->tid) {
		LOG_ERR("Failed to create decoder thread");
		audio_decoder_delete(dec);
		return NULL;
	}

	k_thread_name_set(dec->tid, "lc3_decoder");

	return dec;
}

int audio_decoder_register_cb(struct audio_decoder *decoder, audio_decoder_sdu_cb_t cb,
			      void *context)
{
	if (!decoder || !cb) {
		return -EINVAL;
	}

	struct cb_list *cb_item = (struct cb_list *)malloc(sizeof(struct cb_list));

	if (!cb_item) {
		return -ENOMEM;
	}

	/* Insert callback in linked list */
	cb_item->cb = cb;
	cb_item->context = context;
	cb_item->next = decoder->cb_list;
	decoder->cb_list = cb_item;

	return 0;
}

int audio_decoder_delete(struct audio_decoder *decoder)
{
	if (!decoder) {
		return -EINVAL;
	}

	/* Signal to thread that it should abort */
	decoder->thread_abort = true;
	void *dummy_queue_item = NULL;

	k_msgq_put(&decoder->sdu_queue[0]->msgq, &dummy_queue_item, K_FOREVER);

	/* Join thread before freeing anything */
	k_thread_join(&decoder->thread, K_FOREVER);

	for (int i = 0; i < ARRAY_SIZE(decoder->lc3_decoder); i++) {
		free(decoder->lc3_decoder[i]);
		free(decoder->lc3_status[i]);
	}

	free(decoder->lc3_scratch);

	/* Free linked list of callbacks */
	while (decoder->cb_list) {
		struct cb_list *tmp = decoder->cb_list;

		decoder->cb_list = tmp->next;
		free(tmp);
	}

	free(decoder);

	return 0;
}
