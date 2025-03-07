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
 * @brief Create the audio source datapath channel
 *
 * @param[in] octets_per_frame Number of data per frame
 * @param[in] stream_lid Stream local ID of the channel to be created
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_channel_create_source(size_t octets_per_frame, uint8_t stream_lid);

/**
 * @brief Create the audio source datapath
 *
 * This creates and configures all of the required elements to stream audio from Bluetooth LE to I2S
 *
 * @param cfg Desired configuration of the audio datapath
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_create_source(struct audio_datapath_config *cfg);

/**
 * @brief Start the audio source datapath
 *
 * This starts reception of the first SDU over the ISO datapath, which when received starts off the
 * operation of the rest of the datapath.
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_start_source(void);

/**
 * @brief Clean up the audio source datapath
 *
 * This stops the audio source datapath and cleans up all elements created by
 * audio_datapath_create_source, freeing any allocated memory if necessary. After calling this
 * function, it is possible to create a new audio source datapath again using
 * audio_datapath_create_source.
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_cleanup_source(void);

/**
 * @brief Create the audio sink datapath channel
 *
 * @param[in] octets_per_frame Number of data per frame
 * @param[in] ch_index Index of the channel to be created
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_channel_create_sink(size_t octets_per_frame, uint8_t ch_index);

/**
 * @brief Create the audio sink datapath
 *
 * This creates and configures all of the required elements to stream audio from I2S to Bluetooth LE
 *
 * @param cfg Desired configuration of the audio datapath
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_create_sink(struct audio_datapath_config *cfg);

/**
 * @brief Start the audio sink datapath
 *
 * This starts transmission of the first SDU over the ISO datapath, which when transmitted starts
 * off the operation of the rest of the datapath.
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_start_sink(void);

/**
 * @brief Clean up the audio sink datapath
 *
 * This stops the audio sink datapath and cleans up all elements created by
 * audio_datapath_create_sink, freeing any allocated memory if necessary. After calling this
 * function, it is possible to create a new audio sink datapath again using
 * audio_datapath_create_sink.
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int audio_datapath_cleanup_sink(void);

#endif /* _AUDIO_DATAPATH_H */
