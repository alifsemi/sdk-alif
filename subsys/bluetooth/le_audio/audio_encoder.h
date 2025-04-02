/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _AUDIO_ENCODER_H
#define _AUDIO_ENCODER_H

#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include "audio_queue.h"
#include "sdu_queue.h"
#include "lc3_api.h"

/**
 * @brief Callback function signature for SDU completion
 *
 * @param context User-defined context to be passed to callback
 * @param capture_timestamp ISO clock time at which audio block contained in SDU was captured
 * @param sdu_seq Sequence number of SDU
 */
typedef void (*audio_encoder_sdu_cb_t)(void *context, uint32_t capture_timestamp, uint16_t sdu_seq);

/**
 * @brief Create and start an audio encoder instance
 *
 * The audio encoder instance waits on audio data to be available in the provided audio queue, and
 * then encodes the data using the LC3 codec before pushing it to the SDU queue(s). It supports
 * either single or dual channel encoding.
 *
 * @note The stack for the encoder thread is currently passed in as a parameter since at the time of
 * writing, the targeted Zephyr version does not support dynamically allocating thread stacks. It
 * would be much better to dynamically allocate the stack, as this means the correct stacksize can
 * be set internally to the module and the user does not need to know this. This PR:
 * https://github.com/zephyrproject-rtos/zephyr/pull/44379 adds the functionality needed, so this
 * module should be update to use dynamically allocated stacks at the point where the targeted
 * Zephyr version is updated to include this feature.
 *
 * @param mono Flag set to true if the audio input data contains only a single channel. In this
 * case, only a single channel is encoded but it may be sent out on either one or two channels,
 * depending on how many non-NULL SDU queues were provided. Providing two SDU queues may be
 * necessary for compatibility with sink devices which only support stereo audio.
 * @param stack Stack memory area for encoder thread. Must be statically allocated using
 * K_THREAD_STACK_DEFINE.
 * @param stacksize Size in bytes of stack memory area
 * @param p_sdu_queues Pointer to list of SDU queues for channels
 * @param num_queues SDU queue count on the give list
 * @param audio_queue Audio queue to take audio blocks from
 * @param frame_duration Frame duration @ref enum audio_encoder_frame_duration
 *
 * @retval Created audio encoder instance if successful
 * @retval NULL on failure
 */
struct audio_encoder *audio_encoder_create(uint32_t sampling_frequency, k_thread_stack_t *stack,
					   size_t stacksize, struct sdu_queue *p_sdu_queues[],
					   size_t num_queues, struct audio_queue *audio_queue,
					   lc3_frame_duration_t frame_duration);

/**
 * @brief Register a callback to be called on completion of each encoded frame
 *
 * @param encoder Audio encoder instance to register with
 * @param cb Callback function
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_encoder_register_cb(struct audio_encoder *encoder, audio_encoder_sdu_cb_t cb,
			      void *context);

/**
 * @brief Stop and delete an audio encoder instance
 *
 * The encoder thread is stopped, and all memory allocated by audio_encoder_create is freed.
 *
 * @param encoder The audio encoder instance to delete
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_encoder_delete(struct audio_encoder *encoder);

#endif /* _AUDIO_ENCODER_H */
