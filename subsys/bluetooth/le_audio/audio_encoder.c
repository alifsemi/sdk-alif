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

#include "lc3_api.h"
#include "audio_queue.h"
#include "sdu_queue.h"
#include "gapi_isooshm.h"
#include "audio_encoder.h"
#include "audio_utils.h"

LOG_MODULE_REGISTER(audio_encoder, CONFIG_BLE_AUDIO_LOG_LEVEL);

struct cb_list {
	audio_encoder_sdu_cb_t cb;
	void *context;
	struct cb_list *next;
};

struct audio_encoder {
	/* Flag set to true if encoder only uses a single channel */
	bool mono;

	/* Encoder thread */
	struct k_thread thread;
	k_thread_stack_t *stack;
	k_tid_t tid;
	bool thread_abort;

	/* LC3 configuration, encoder instances and scratch memory */
	lc3_cfg_t lc3_cfg;
	lc3_encoder_t *lc3_encoder[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
	int32_t *lc3_scratch;

	/* Linked list of registered callbacks */
	struct cb_list *cb_list;

	/* Input and output queues */
	struct sdu_queue *sdu_queue[CONFIG_ALIF_BLE_AUDIO_NMB_CHANNELS];
	struct audio_queue *audio_queue;
};

static void audio_encoder_thread_func(void *p1, void *p2, void *p3)
{
	struct audio_encoder *enc = (struct audio_encoder *)p1;
	(void)p2;
	(void)p3;

	int ret;
	gapi_isooshm_sdu_buf_t *p_sdu_l = NULL;
	gapi_isooshm_sdu_buf_t *p_sdu_r = NULL;
	struct sdu_queue *const p_sdu_queue_l = enc->sdu_queue[0];
	struct sdu_queue *const p_sdu_queue_r = enc->sdu_queue[1];
	struct audio_queue *const p_audio_queue = enc->audio_queue;
	size_t const half_audio_block_samples = p_audio_queue->audio_block_samples / 2;
	/* Sequence number applied to each outging SDU clipped to uint16_t */
	size_t sdu_seq = 0;
	bool const mono = enc->mono;

	while (!enc->thread_abort) {
		p_sdu_l = NULL;
		p_sdu_r = NULL;

		/* Allocate left SDU (always present) and encode audio into it */
		ret = k_mem_slab_alloc(&p_sdu_queue_l->slab, (void *)&p_sdu_l, K_FOREVER);
		__ASSERT(ret == 0, "Failed to get memory from slab");

		/* Allocate right SDU (present in stereo mode) and encode audio into it */
		if (likely(p_sdu_queue_r)) {
			ret = k_mem_slab_alloc(&p_sdu_queue_r->slab, (void *)&p_sdu_r, K_FOREVER);
			__ASSERT(ret == 0, "Failed to get memory from slab");
		}

get_audio:
		/* Get the next audio block */
		struct audio_block *audio = NULL;

		ret = k_msgq_get(&p_audio_queue->msgq, &audio, K_FOREVER);
		__ASSERT(ret == 0, "msgq get failed");

		/* A NULL audio block can be sent to the queue to wake up and abort thread. Continue
		 * and check thread abort flag.
		 */
		if (unlikely(!audio)) {
			if (enc->thread_abort) {
				LOG_WRN("Thread aborted");
				break;
			}
			goto get_audio;
		}

		/* Encode left */
		ret = lc3_api_encode_frame(&enc->lc3_cfg, enc->lc3_encoder[0], audio->buf,
					   p_sdu_l->data, p_sdu_queue_l->payload_size,
					   enc->lc3_scratch);
		__ASSERT(ret == 0, "LC3 encoding failed");

		p_sdu_l->sdu_len = p_sdu_queue_l->payload_size;
		p_sdu_l->has_timestamp = false;
		p_sdu_l->seq_num = sdu_seq;

		/* If only left channel */
		if (unlikely(!p_sdu_queue_r)) {
			goto enc_done;
		}

		p_sdu_r->sdu_len = p_sdu_queue_r->payload_size;
		p_sdu_r->has_timestamp = false;
		p_sdu_r->seq_num = sdu_seq;

		/* Mono mode, copy from left to right */
		if (mono) {
			memcpy(p_sdu_r->data, p_sdu_l->data, p_sdu_queue_r->payload_size);
			goto enc_done;
		}

		/* Encode right */
		int16_t *audio_data = audio->buf + half_audio_block_samples;

		ret = lc3_api_encode_frame(&enc->lc3_cfg, enc->lc3_encoder[1], audio_data,
					   p_sdu_r->data, p_sdu_queue_r->payload_size,
					   enc->lc3_scratch);
		__ASSERT(ret == 0, "LC3 encoding failed");

enc_done:
		/* Temporarily store capture timestamp for callbacks */
		uint32_t const capture_timestamp = audio->timestamp;

		k_mem_slab_free(&p_audio_queue->slab, audio);

		/* Send left */
		ret = k_msgq_put(&p_sdu_queue_l->msgq, &p_sdu_l, K_FOREVER);
		__ASSERT(ret == 0, "msgq put failed");

		/* Send right */
		if (likely(p_sdu_r)) {
			ret = k_msgq_put(&p_sdu_queue_r->msgq, &p_sdu_r, K_FOREVER);
			__ASSERT(ret == 0, "msgq put failed");
		}

		/* Notify listeners that a block is completed */
		struct cb_list *cb_item = enc->cb_list;

		while (cb_item) {
			if (cb_item->cb) {
				cb_item->cb(cb_item->context, capture_timestamp, sdu_seq);
			}
			cb_item = cb_item->next;
		}

		/* Increment sequence number for next SDU */
		sdu_seq++;
	}
}

struct audio_encoder *audio_encoder_create(uint32_t const sampling_frequency,
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

	struct audio_encoder *enc = calloc(1, sizeof(*enc));

	if (!enc) {
		LOG_ERR("Failed to allocate audio encoder");
		return NULL;
	}

	uint8_t num_valid = 0;

	for (uint8_t iter = 0; iter < num_queues; iter++) {
		num_valid += !!p_sdu_queues[iter];
		enc->sdu_queue[iter] = p_sdu_queues[iter];
	}
	enc->mono = (num_valid == 1);
	enc->audio_queue = audio_queue;

	/* Configure LC3 codec and allocate required memory */
	int ret = lc3_api_configure(&enc->lc3_cfg, (int32_t)sampling_frequency, frame_duration);

	if (ret) {
		LOG_ERR("Failed to configure LC3 codec, err %d", ret);
		audio_encoder_delete(enc);
		return NULL;
	}

	size_t scratch_size = lc3_api_encoder_scratch_size(&enc->lc3_cfg);

	enc->lc3_scratch = (int32_t *)malloc(scratch_size);
	if (!enc->lc3_scratch) {
		LOG_ERR("Failed to allocate encoder scratch memory");
		audio_encoder_delete(enc);
		return NULL;
	}

	for (int i = 0; i < num_queues; i++) {
		enc->lc3_encoder[i] = (lc3_encoder_t *)malloc(sizeof(lc3_encoder_t));
		if (!enc->lc3_encoder[i]) {
			LOG_ERR("Failed to allocate LC3 encoder");
			audio_encoder_delete(enc);
			return NULL;
		}

		ret = lc3_api_initialise_encoder(&enc->lc3_cfg, enc->lc3_encoder[i]);
		if (ret) {
			LOG_ERR("Failed to initialise LC3 encoder %d, err %d", i, ret);
			audio_encoder_delete(enc);
			return NULL;
		}
	}

	/* Create and start thread */
	enc->tid = k_thread_create(&enc->thread, stack, stacksize, audio_encoder_thread_func, enc,
				   NULL, NULL, CONFIG_ALIF_BLE_HOST_THREAD_PRIORITY, 0, K_NO_WAIT);
	if (!enc->tid) {
		LOG_ERR("Failed to create encoder thread");
		audio_encoder_delete(enc);
		return NULL;
	}

	k_thread_name_set(enc->tid, "lc3_encoder");

	return enc;
}

int audio_encoder_register_cb(struct audio_encoder *encoder, audio_encoder_sdu_cb_t cb,
			      void *context)
{
	if (!encoder || !cb) {
		return -EINVAL;
	}

	struct cb_list *cb_item = (struct cb_list *)malloc(sizeof(struct cb_list));

	if (!cb_item) {
		return -ENOMEM;
	}

	/* Insert callback in linked list */
	cb_item->cb = cb;
	cb_item->context = context;
	cb_item->next = encoder->cb_list;
	encoder->cb_list = cb_item;

	return 0;
}

int audio_encoder_delete(struct audio_encoder *encoder)
{
	if (!encoder) {
		return -EINVAL;
	}

	/* Signal to thread that it should abort */
	encoder->thread_abort = true;
	void *dummy_queue_item = NULL;

	k_msgq_put(&encoder->audio_queue->msgq, &dummy_queue_item, K_FOREVER);

	/* Join thread before freeing anything */
	k_thread_join(&encoder->thread, K_FOREVER);

	for (int i = 0; i < ARRAY_SIZE(encoder->lc3_encoder); i++) {
		free(encoder->lc3_encoder[i]);
	}

	free(encoder->lc3_scratch);

	/* Free linked list of callbacks */
	while (encoder->cb_list) {
		struct cb_list *tmp = encoder->cb_list;

		encoder->cb_list = tmp->next;
		free(tmp);
	}

	free(encoder);

	return 0;
}
