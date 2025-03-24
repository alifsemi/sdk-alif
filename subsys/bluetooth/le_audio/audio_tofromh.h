/**
 * Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * @details
 * This header file contains API for translation between the BLE host stack
 * and the audio datapath (audio queues and ISO data paths).
 * @note
 * This API is currently only available for LE Audio usage.
 *
 */

#ifndef AUDIO_TOFROMH_H__
#define AUDIO_TOFROMH_H__

#include <zephyr/types.h>
#include "bap.h"

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enumerated values for audio locations
 *
 * These are used to identify the positions of audio channels in a multi-channel
 * audio stream.
 */
enum audio_location {
	AUDIO_LOCATION_MONO_AUDIO = 0,

	AUDIO_LOCATION_FRONT_LEFT,
	AUDIO_LOCATION_FRONT_RIGHT,
	AUDIO_LOCATION_FRONT_CENTER,
	/**
	 * LFE1: Low Frequency Effects 1
	 */
	AUDIO_LOCATION_LFE1,
	AUDIO_LOCATION_BACK_LEFT,
	AUDIO_LOCATION_BACK_RIGHT,
	AUDIO_LOCATION_FRONT_LEFT_CENTER,
	AUDIO_LOCATION_FRONT_RIGHT_CENTER,
	AUDIO_LOCATION_BACK_CENTER,
	/**
	 * LFE2: Low Frequency Effects 2
	 */
	AUDIO_LOCATION_LFE2,
	AUDIO_LOCATION_SIDE_LEFT,
	AUDIO_LOCATION_SIDE_RIGHT,
	AUDIO_LOCATION_TOP_FRONT_LEFT,
	AUDIO_LOCATION_TOP_FRONT_RIGHT,
	AUDIO_LOCATION_TOP_FRONT_CENTER,
	AUDIO_LOCATION_TOP_CENTER,
	AUDIO_LOCATION_TOP_BACK_LEFT,
	AUDIO_LOCATION_TOP_BACK_RIGHT,
	AUDIO_LOCATION_TOP_SIDE_LEFT,
	AUDIO_LOCATION_TOP_SIDE_RIGHT,
	AUDIO_LOCATION_TOP_BACK_CENTER,
	AUDIO_LOCATION_BOTTOM_FRONT_CENTER,
	AUDIO_LOCATION_BOTTOM_FRONT_LEFT,
	AUDIO_LOCATION_BOTTOM_FRONT_RIGHT,
	AUDIO_LOCATION_FRONT_LEFT_WIDE,
	AUDIO_LOCATION_FRONT_RIGHT_WIDE,
	AUDIO_LOCATION_LEFT_SURROUND,
	AUDIO_LOCATION_RIGHT_SURROUND,
};

enum audio_frame_duration {
	AUDIO_FRAME_DURATION_7P5MS,
	AUDIO_FRAME_DURATION_10MS,
};

/**
 * @brief Maps a GAF location to an audio location.
 *
 * @param[in] gaf_loc GAF location.
 *
 * @return Corresponding audio location, or AUDIO_LOCATION_MONO_AUDIO if the GAF location is not
 * specified.
 */
enum audio_location audio_gaf_loc_to_location(uint32_t gaf_loc);

/**
 * @brief Maps an audio location to a GAF location.
 *
 * @param[in] audio_loc Audio location.
 *
 * @return Corresponding GAF location, or 0 if the audio location is not
 * specified.
 */
uint32_t audio_location_to_gaf_loc(enum audio_location audio_loc);

/**
 * @brief Converts from enum bap_frame_dur to enum audio_frame_duration
 *
 * @param[in] dur The enum bap_frame_dur value to convert
 *
 * @return The corresponding enum audio_frame_duration value.
 */
enum audio_frame_duration audio_bap_frame_dur_to_frame_dur(enum bap_frame_dur dur);

/**
 * @brief Converts from enum audio_frame_duration to enum bap_frame_dur
 *
 * @param[in] dur The enum audio_frame_duration value to convert
 *
 * @return The corresponding enum bap_frame_dur value.
 */
enum bap_frame_dur audio_frame_dur_to_bap_frame_dur(enum audio_frame_duration dur);

/**
 * @brief Converts from a uint32_t sampling rate to enum bap_sampling_freq
 *
 * @param[in] rate The sampling rate in Hz to convert
 *
 * @return The corresponding enum bap_sampling_freq value.
 */
enum bap_sampling_freq audio_hz_to_bap_sampling_freq(uint32_t rate);

/**
 * @brief Converts from enum bap_sampling_freq to a uint32_t sampling rate
 *
 * @param[in] freq The enum bap_sampling_freq value to convert
 *
 * @return The corresponding sampling rate in Hz.
 */
uint32_t audio_bap_sampling_freq_to_hz(enum bap_sampling_freq freq);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_TOFROMH_H__ */

