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

#include "bluetooth/le_audio/audio_encoder.h"

/**
 * @brief Configure microphone source using I2S
 *
 * @param i2s_mic_dev I2S device to use for microphone input
 * @param i2s_dev I2S device to use for audio output
 * @param audio_encoder Audio encoder that data will handle audio frames
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int mic_i2s_configure(const struct device *i2s_mic_dev, const struct device *i2s_dev,
		      struct audio_encoder *audio_encoder);

/**
 * @brief Start the microphone source stream
 */
void mic_i2s_start(void);

/**
 * @brief Stop the microphone source stream
 */
void mic_i2s_stop(void);

/**
 * @brief Control microphone source
 *
 * @param start true to start, false to stop
 */
void mic_i2s_control(bool const start);

#endif /* MIC_SOURCE_H */
