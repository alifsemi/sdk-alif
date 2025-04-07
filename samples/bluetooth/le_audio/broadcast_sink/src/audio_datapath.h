/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
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
#include <zephyr/device.h>

struct audio_datapath_config {
	const struct device *i2s_dev;
	/* const struct device *mclk_dev; */
	uint32_t pres_delay_us;
	uint32_t sampling_rate_hz;
	uint16_t octets_per_frame;
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
