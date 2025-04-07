/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _AUDIO_DATAPATH_H
#define _AUDIO_DATAPATH_H

#include <zephyr/types.h>

#define I2S_NODE   DT_ALIAS(i2s_bus)
#define CODEC_NODE DT_ALIAS(audio_codec)

struct audio_datapath_config {
	uint32_t pres_delay_us;
	uint32_t sampling_rate_hz;
	bool frame_duration_is_10ms;
};

/**
 * @brief Create the audio datapath
 *
 * This creates and configures all of the required elements to stream audio from Bluetooth LE to I2S
 *
 * @param cfg Desired configuration of the audio datapath
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_create(struct audio_datapath_config const *cfg);

/**
 * @brief Create the audio datapath channel
 *
 * @param[in] octets_per_frame Number of data per frame
 * @param[in] ch_index Index of the channel to be created
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_create_channel(size_t octets_per_frame, uint8_t ch_index);

/**
 * @brief Start a channel
 *
 * @param ch_index Index of the channel to start
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_start_channel(uint8_t ch_index);

/**
 * @brief Stop a channel
 *
 * @param decoder Audio decoder instance to stop channel for
 * @param channel_id Channel ID to stop
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_stop_channel(uint8_t ch_index);

/**
 * @brief Clean up the audio datapath
 *
 * This stops the audio datapath and cleans up all elements created by audio_datapath_create,
 * freeing any allocated memory if necessary. After calling this function, it is possible to create
 * a new audio datapath again using audio_datapath_create.
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_cleanup(void);

#endif /* _AUDIO_DATAPATH_H */
