/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef AUDIOBACKEND_H
#define AUDIOBACKEND_H

#include <stdint.h>

/**
 * @brief Initialize the audio capture backend.
 *
 * Configures the microphone device (I2S or PDM, selected at build time) for the
 * requested sampling rate and starts a worker thread that captures audio in the
 * background. Must be called before any other function in this API and paired
 * with @ref audio_uninit when capture is no longer needed.
 *
 * @param sampling_rate Desired sampling rate in Hz. For PDM microphones only a
 *                      fixed set of rates is supported (8000, 16000, 32000,
 *                      48000, 96000).
 *
 * @return 0 on success, or a negative error code on failure (e.g. -ENODEV if
 *         the microphone device is not ready, or -1 for an unsupported PDM
 *         sampling rate).
 */
int audio_init(int sampling_rate);

/**
 * @brief Stop audio capture and release the backend.
 *
 * Signals the worker thread to stop, waits for it to exit, stops the
 * microphone and clears the internal session state. Safe to call after
 * @ref audio_init; afterwards @ref audio_init must be called again before
 * capturing more audio.
 */
void audio_uninit(void);

/**
 * @brief Request the next chunk of audio samples.
 *
 * Hands the destination buffer to the worker thread and returns immediately
 * without blocking. The buffer is filled asynchronously; call
 * @ref wait_for_audio to block until the requested samples are available. The
 * buffer must remain valid until @ref wait_for_audio returns.
 *
 * @param data Pointer to the buffer that receives the captured samples.
 * @param len  Number of int16_t samples to capture into @p data.
 *
 * @return 0 on success.
 */
int get_audio_data(int16_t *data, int len);

/**
 * @brief Wait for the pending audio capture to complete.
 *
 * Blocks until the chunk requested by the previous @ref get_audio_data call has
 * been written to the destination buffer, or until the worker thread stops.
 *
 * @return 0 when a chunk is ready, or a negative error code if the worker
 *         stopped due to a microphone start or read error.
 */
int wait_for_audio(void);

/**
 * @brief Apply gain preprocessing to captured audio samples in place.
 *
 * Scales each sample by the configured I2S gain. For PDM microphones this is a
 * no-op.
 *
 * @param data Pointer to the buffer of samples to process in place.
 * @param len  Number of int16_t samples in @p data.
 */
void audio_preprocessing(int16_t *data, int len);

#endif
