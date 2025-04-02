/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _AUDIO_DECODER_H
#define _AUDIO_DECODER_H

#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include "audio_queue.h"
#include "lc3_api.h"
#include "sdu_queue.h"

/**
 * @brief Callback function signature for SDU completion
 *
 * @param context User-defined context to be passed to callback
 * @param timestamp SDU ref anchor point, in ISO clock time
 * @param sdu_seq Sequence number of SDU
 */
typedef void (*audio_decoder_sdu_cb_t)(void *context, uint32_t timestamp, uint16_t sdu_seq);

/**
 * @brief Create and start an audio decoder instance
 *
 * The audio decoder instance waits on SDUs to be available in the provided SDU queue(s). When an
 * SDU is available, it is decoded using the LC3 codec and pushed into the provided audio queue.
 * Either single or dual channel decoding is supported.
 *
 * @note The stack for the decoder thread is currently passed in as a parameter since at the time of
 * writing, the targeted Zephyr version does not support dynamically allocating thread stacks. It
 * would be much better to dynamically allocate the stack, as this means the correct stacksize can
 * be set internally to the module and the user does not need to know this. This PR:
 * https://github.com/zephyrproject-rtos/zephyr/pull/44379 adds the functionality needed, so this
 * module should be update to use dynamically allocated stacks at the point where the targeted
 * Zephyr version is updated to include this feature.
 *
 * @param stack Stack memory area for decoder thread. Must be statically allocated using
 * K_THREAD_STACK_DEFINE.
 * @param stacksize Size in bytes of stack memory area
 * @param p_sdu_queues Pointer to list of SDU queues for channels
 * @param num_queues SDU queue count on the give list
 * @param audio_queue Audio queue to push decoded audio data to
 * @param frame_duration Frame duration @ref enum audio_decoder_frame_duration
 *
 * @retval Created audio decoder instance if successful
 * @retval NULL on failure
 */
struct audio_decoder *audio_decoder_create(uint32_t sampling_frequency, k_thread_stack_t *stack,
					   size_t stacksize, struct sdu_queue *p_sdu_queues[],
					   size_t num_queues, struct audio_queue *audio_queue,
					   lc3_frame_duration_t frame_duration);

/**
 * @brief Register a callback to be called on completion of each decoded frame
 *
 * @param encoder Audio decoder instance to register with
 * @param cb Callback function
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_decoder_register_cb(struct audio_decoder *decoder, audio_decoder_sdu_cb_t cb,
			      void *context);

/**
 * @brief Stop and delete an audio decoder instance
 *
 * The decoder thread is stopped, and all memory allocated by audio_decoder_create is freed.
 *
 * @param decoder The audio decoder instance to delete
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_decoder_delete(struct audio_decoder *decoder);

#endif /* _AUDIO_DECODER_H */
