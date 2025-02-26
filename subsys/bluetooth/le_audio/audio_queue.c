/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <stdlib.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/__assert.h>
#include "audio_queue.h"

LOG_MODULE_REGISTER(audio_queue, CONFIG_BLE_AUDIO_LOG_LEVEL);

struct audio_queue *audio_queue_create(size_t item_count, size_t audio_block_samples)
{
	/* Timestamp and the 16-bit PCM samples */
	size_t item_size = sizeof(struct audio_block) + (audio_block_samples * sizeof(int16_t));

	/* Each item must be 4-byte aligned */
	size_t padded_size = ROUND_UP(item_size, 4);

	size_t total_size = sizeof(struct audio_queue) + (item_count * padded_size) +
			    (item_count * sizeof(void *));

	struct audio_queue *hdr = (struct audio_queue *)malloc(total_size);

	if (hdr == NULL) {
		LOG_ERR("Failed to allocate audio queue");
		return NULL;
	}

	/* malloc should give a minimum of 4-byte alignment, but confirm this */
	__ASSERT(IS_PTR_ALIGNED(hdr->buf, 4), "Audio buffer is not 4-byte aligned");

	int ret = k_mem_slab_init(&hdr->slab, hdr->buf, padded_size, item_count);

	if (ret) {
		LOG_ERR("Failed to initialise audio queue mem slab");
		return NULL;
	}

	k_msgq_init(&hdr->msgq, hdr->buf + (item_count * padded_size), sizeof(void *), item_count);

	hdr->audio_block_samples = audio_block_samples;
	hdr->item_count = item_count;
	hdr->item_size = item_size;

	return hdr;
}

int audio_queue_delete(struct audio_queue *queue)
{
	if (queue == NULL) {
		return -EINVAL;
	}

	free(queue);
	return 0;
}
