/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _AUDIO_QUEUE_H
#define _AUDIO_QUEUE_H

#include <zephyr/kernel.h>

struct audio_block {
	uint32_t timestamp;
	int16_t buf[];
};

struct audio_queue {
	size_t item_count;
	size_t item_size;
	size_t audio_block_samples;
	struct k_mem_slab slab;
	struct k_msgq msgq;
	uint8_t buf[];
};

/**
 * @brief Dynamically allocate and initialise an audio queue
 *
 * For most use cases dynamic allocation is required since the size of each audio block is not known
 * until the parameters of the stream are agreed between this device and the peer device (broadcast
 * source use case is an exception where all parameters can be fixed at compile time).
 *
 * @param item_count Number of audio blocks in the queue
 * @param audio_block_samples Number of samples in each audio block (considering both channels if
 * stereo, so this parameter would be 200 for a stereo stream with 100 samples per channel per
 * block)
 *
 * @retval Pointer to created audio queue header if successful
 * @retval NULL if an error occurred
 */
struct audio_queue *audio_queue_create(size_t item_count, size_t audio_block_samples);

/**
 * @brief Delete an audio queue that was previously dynamically allocated
 *
 * @param queue Pointer to the queue to delete
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_queue_delete(struct audio_queue *queue);

#endif /* _AUDIO_QUEUE_H */
