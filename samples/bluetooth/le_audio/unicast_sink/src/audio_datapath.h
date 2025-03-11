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

#define I2S_NODE   DT_ALIAS(i2s_bus)
#define CODEC_NODE DT_ALIAS(audio_codec)

/* Other sampling rates and frame rates are not supported */
#define AUDIO_FRAMES_PER_SECOND 100

struct audio_datapath_config {
	uint32_t pres_delay_us;
	uint16_t sampling_rate_hz;
	bool frame_duration_is_10ms;
};

/**
 * @brief Create the audio datapath channel
 *
 * @param[in] octets_per_frame Number of data per frame
 * @param[in] ch_index Index of the channel to be created
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_create_channel(size_t const octets_per_frame, uint8_t const ch_index);

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
int audio_datapath_create(struct audio_datapath_config *cfg);

/**
 * @brief Start the audio datapath
 *
 * This starts reception of the first SDU over the ISO datapath, which when received starts off the
 * operation of the rest of the datapath.
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_start(void);

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
