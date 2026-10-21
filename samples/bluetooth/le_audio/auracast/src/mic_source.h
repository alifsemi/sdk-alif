/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef MIC_SOURCE_H
#define MIC_SOURCE_H

#include "bluetooth.h"
#if __cplusplus
extern "C" {
#endif

#include "bluetooth/le_audio/audio_encoder.h"
#include <zephyr/toolchain.h>

typedef int (*mic_data_callback_t)(const void *data, size_t size, void *user_data);

#if CONFIG_AUDIO_DMIC

/**
 * @brief Configure microphone source
 *
 * @param mic_dev Device to use for microphone input
 * @param i2s_dev I2S device to use for audio input from audio jack
 * @param audio_encoder Audio encoder that data will handle audio frames
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int mic_configure(const struct device *mic_dev, const struct device *i2s_dev,
		  struct audio_encoder *audio_encoder);

/**
 * @brief Register a callback to receive microphone data
 *
 * @param callback Function to call with microphone data
 * @param user_data User data to pass to the callback
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int mic_register_callback(mic_data_callback_t callback, void *user_data);

/**
 * @brief Adjust the microphone gain
 *
 * @param increase true to increase gain, false to decrease
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int mic_adjust_gain(bool increase);

/**
 * @brief Start the microphone source stream
 */
void mic_start(void);

/**
 * @brief Stop the microphone source stream
 */
void mic_stop(void);

/**
 * @brief Control microphone source
 *
 * @param start true to start, false to stop
 */
void mic_control(bool start);

#else
static inline int mic_configure(const struct device *mic_dev, const struct device *i2s_dev,
				struct audio_encoder *audio_encoder)
{
	return 0;
}

static inline int mic_register_callback(mic_data_callback_t callback, void *user_data)
{
	return 0;
}

static inline int mic_adjust_gain(bool increase)
{
	return 0;
}

static inline void mic_start(void)
{
}

static inline void mic_stop(void)
{
}

static inline void mic_control(bool const start)
{
	ARG_UNUSED(start);
}
#endif

#if __cplusplus
}
#endif

#endif /* MIC_SOURCE_H */
