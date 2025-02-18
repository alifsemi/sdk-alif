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
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include "alif_lc3.h"
#include "audio_queue.h"
#include "sdu_queue.h"
#include "gapi_isooshm.h"
#include "audio_encoder.h"

LOG_MODULE_REGISTER(audio_encoder, CONFIG_BLE_AUDIO_LOG_LEVEL);

#define AUDIO_ENCODER_MAX_CHANNELS 2

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
	lc3_encoder_t *lc3_encoder[AUDIO_ENCODER_MAX_CHANNELS];
	int32_t *lc3_scratch;

	/* Linked list of registered callbacks */
	struct cb_list *cb_list;

	/* Input and output queues */
	struct sdu_queue *sdu_queue[AUDIO_ENCODER_MAX_CHANNELS];
	struct audio_queue *audio_queue;

	/* Sequence number applied to each outging SDU */
	uint16_t sdu_seq;
};

static void audio_encoder_thread_func(void *p1, void *p2, void *p3)
{
	struct audio_encoder *enc = (struct audio_encoder *)p1;
	(void)p2;
	(void)p3;

	int ret;

	while (!enc->thread_abort) {
		/* Get the next audio block */
		struct audio_block *audio = NULL;

		ret = k_msgq_get(&enc->audio_queue->msgq, &audio, K_FOREVER);
		__ASSERT(ret == 0, "msgq get failed");

		/* A NULL audio block can be sent to the queue to wake up and abort thread. Continue
		 * and check thread abort flag.
		 */
		if (!audio) {
			continue;
		}

		/* Allocate left SDU (always present) and encode audio into it */
		gapi_isooshm_sdu_buf_t *p_sdu_l;

		ret = k_mem_slab_alloc(&enc->sdu_queue[0]->slab, (void *)&p_sdu_l, K_FOREVER);
		__ASSERT(ret == 0, "Failed to get memory from slab");

		ret = lc3_api_encode_frame(&enc->lc3_cfg, enc->lc3_encoder[0], audio->buf,
					   p_sdu_l->data, enc->sdu_queue[0]->payload_size,
					   enc->lc3_scratch);
		__ASSERT(ret == 0, "LC3 encoding failed");

		p_sdu_l->sdu_len = enc->sdu_queue[0]->payload_size;
		p_sdu_l->has_timestamp = false;
		p_sdu_l->seq_num = enc->sdu_seq;

		/* If only left channel */
		if (!enc->sdu_queue[1]) {
			goto send_left;
		}

		/* When a right ISO stream exists, allocate an SDU, fill with data and send */
		gapi_isooshm_sdu_buf_t *p_sdu_r;

		ret = k_mem_slab_alloc(&enc->sdu_queue[1]->slab, (void *)&p_sdu_r, K_FOREVER);
		__ASSERT(ret == 0, "Failed to get memory from slab");

		if (enc->mono) {
			/* In mono mode, copy the data from the left channel */
			memcpy(p_sdu_r->data, p_sdu_l->data, enc->sdu_queue[1]->payload_size);
		} else {
			/* In stereo mode, encode the right channel */
			int16_t *audio_data =
				audio->buf + enc->audio_queue->audio_block_samples / 2;
			ret = lc3_api_encode_frame(&enc->lc3_cfg, enc->lc3_encoder[1], audio_data,
						   p_sdu_r->data, enc->sdu_queue[1]->payload_size,
						   enc->lc3_scratch);
			__ASSERT(ret == 0, "LC3 encoding failed");
		}

		p_sdu_r->sdu_len = enc->sdu_queue[1]->payload_size;
		p_sdu_r->has_timestamp = false;
		p_sdu_r->seq_num = enc->sdu_seq;

		ret = k_msgq_put(&enc->sdu_queue[1]->msgq, &p_sdu_r, K_FOREVER);
		__ASSERT(ret == 0, "msgq put failed");

send_left:
		/* Send left SDU now that it is no longer needed */
		ret = k_msgq_put(&enc->sdu_queue[0]->msgq, &p_sdu_l, K_FOREVER);
		__ASSERT(ret == 0, "msgq put failed");

		/* Temporarily store capture timestamp for callbacks */
		uint32_t capture_timestamp = audio->timestamp;

		/* Free the completed audio block */
		k_mem_slab_free(&enc->audio_queue->slab, audio);

		/* Notify listeners that a block is completed */
		struct cb_list *cb_item = enc->cb_list;

		while (cb_item) {
			if (cb_item->cb) {
				cb_item->cb(cb_item->context, capture_timestamp, enc->sdu_seq);
			}
			cb_item = cb_item->next;
		}

		/* Increment sequence number for next SDU */
		enc->sdu_seq++;
	}
}

struct audio_encoder *audio_encoder_create(bool mono, uint32_t sampling_frequency,
					   k_thread_stack_t *stack, size_t stacksize,
					   struct sdu_queue *sdu_queue_l,
					   struct sdu_queue *sdu_queue_r,
					   struct audio_queue *audio_queue)
{
	if (!stack) {
		LOG_ERR("Thread stack must be provided");
		return NULL;
	}

	if (!audio_queue) {
		LOG_ERR("Audio queue must be provided");
		return NULL;
	}

	if (!sdu_queue_l) {
		LOG_ERR("At least one SDU queue must be provided");
		return NULL;
	}

	if (!sdu_queue_r && !mono) {
		LOG_ERR("One SDU queue must be provided per channel");
		return NULL;
	}

	struct audio_encoder *enc =
		(struct audio_encoder *)calloc(sizeof(struct audio_encoder), 1);

	if (!enc) {
		LOG_ERR("Failed to allocate audio encoder");
		return NULL;
	}

	enc->mono = mono;
	enc->sdu_queue[0] = sdu_queue_l;
	enc->sdu_queue[1] = sdu_queue_r;
	enc->audio_queue = audio_queue;

	/* Configure LC3 codec and allocate required memory */
	int ret =
		lc3_api_configure(&enc->lc3_cfg, (int32_t)sampling_frequency, FRAME_DURATION_10_MS);

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

	for (int i = 0; i < 2U - enc->mono; i++) {
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

	for (int i = 0; i < AUDIO_ENCODER_MAX_CHANNELS; i++) {
		if (encoder->lc3_encoder[i]) {
			free(encoder->lc3_encoder[i]);
		}
	}

	if (encoder->lc3_scratch) {
		free(encoder->lc3_scratch);
	}

	/* Free linked list of callbacks */
	while (encoder->cb_list) {
		struct cb_list *tmp = encoder->cb_list;

		encoder->cb_list = tmp->next;
		free(tmp);
	}

	free(encoder);

	return 0;
}
