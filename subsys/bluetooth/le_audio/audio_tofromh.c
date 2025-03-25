/**
 * Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/logging/log.h>
#include <zephyr/types.h>
#include <zephyr/sys/__assert.h>

#include <stdint.h>
#include "audio_tofromh.h"
#include "gaf.h"
#include "bap.h"

LOG_MODULE_REGISTER(audio_tofromh, CONFIG_BLE_AUDIO_LOG_LEVEL);

/**
 * @brief Maps a GAF location to an audio location.
 *
 * @param[in] gaf_loc GAF location.
 *
 * @return Corresponding audio location, or AUDIO_LOCATION_MONO_AUDIO if the GAF location is not
 * specified.
 */
enum audio_location audio_gaf_loc_to_location(uint32_t gaf_loc)
{
	switch (gaf_loc) {
	case GAF_LOC_FRONT_LEFT_BIT:
		return AUDIO_LOCATION_FRONT_LEFT;
	case GAF_LOC_FRONT_RIGHT_BIT:
		return AUDIO_LOCATION_FRONT_RIGHT;
	case GAF_LOC_FRONT_CENTER_BIT:
		return AUDIO_LOCATION_FRONT_CENTER;
	case GAF_LOC_LFE1_BIT:
		return AUDIO_LOCATION_LFE1;
	case GAF_LOC_BACK_LEFT_BIT:
		return AUDIO_LOCATION_BACK_LEFT;
	case GAF_LOC_BACK_RIGHT_BIT:
		return AUDIO_LOCATION_BACK_RIGHT;
	case GAF_LOC_FRONT_LEFT_CENTER_BIT:
		return AUDIO_LOCATION_FRONT_LEFT_CENTER;
	case GAF_LOC_FRONT_RIGHT_CENTER_BIT:
		return AUDIO_LOCATION_FRONT_RIGHT_CENTER;
	case GAF_LOC_BACK_CENTER_BIT:
		return AUDIO_LOCATION_BACK_CENTER;
	case GAF_LOC_LFE2_BIT:
		return AUDIO_LOCATION_LFE2;
	case GAF_LOC_SIDE_LEFT_BIT:
		return AUDIO_LOCATION_SIDE_LEFT;
	case GAF_LOC_SIDE_RIGHT_BIT:
		return AUDIO_LOCATION_SIDE_RIGHT;
	case GAF_LOC_TOP_FRONT_LEFT_BIT:
		return AUDIO_LOCATION_TOP_FRONT_LEFT;
	case GAF_LOC_TOP_FRONT_RIGHT_BIT:
		return AUDIO_LOCATION_TOP_FRONT_RIGHT;
	case GAF_LOC_TOP_FRONT_CENTER_BIT:
		return AUDIO_LOCATION_TOP_FRONT_CENTER;
	case GAF_LOC_TOP_CENTER_BIT:
		return AUDIO_LOCATION_TOP_CENTER;
	case GAF_LOC_TOP_BACK_LEFT_BIT:
		return AUDIO_LOCATION_TOP_BACK_LEFT;
	case GAF_LOC_TOP_BACK_RIGHT_BIT:
		return AUDIO_LOCATION_TOP_BACK_RIGHT;
	case GAF_LOC_TOP_SIDE_LEFT_BIT:
		return AUDIO_LOCATION_TOP_SIDE_LEFT;
	case GAF_LOC_TOP_SIDE_RIGHT_BIT:
		return AUDIO_LOCATION_TOP_SIDE_RIGHT;
	case GAF_LOC_TOP_BACK_CENTER_BIT:
		return AUDIO_LOCATION_TOP_BACK_CENTER;
	case GAF_LOC_BOTTOM_FRONT_CENTER_BIT:
		return AUDIO_LOCATION_BOTTOM_FRONT_CENTER;
	case GAF_LOC_BOTTOM_FRONT_LEFT_BIT:
		return AUDIO_LOCATION_BOTTOM_FRONT_LEFT;
	case GAF_LOC_BOTTOM_FRONT_RIGHT_BIT:
		return AUDIO_LOCATION_BOTTOM_FRONT_RIGHT;
	case GAF_LOC_FRONT_LEFT_WIDE_BIT:
		return AUDIO_LOCATION_FRONT_LEFT_WIDE;
	case GAF_LOC_FRONT_RIGHT_WIDE_BIT:
		return AUDIO_LOCATION_FRONT_RIGHT_WIDE;
	case GAF_LOC_LEFT_SURROUND_BIT:
		return AUDIO_LOCATION_LEFT_SURROUND;
	case GAF_LOC_RIGHT_SURROUND_BIT:
		return AUDIO_LOCATION_RIGHT_SURROUND;
	default:
		LOG_INF("Defaulting to mono audio");
		return AUDIO_LOCATION_MONO_AUDIO;
	}
}

/**
 * @brief Maps an audio location to a GAF location.
 *
 * @param[in] audio_location Audio location.
 *
 * @return Corresponding GAF location, or 0 if the audio location is not specified.
 */
uint32_t audio_location_to_gaf_loc(enum audio_location audio_location)
{
	switch (audio_location) {
	case AUDIO_LOCATION_FRONT_LEFT:
		return GAF_LOC_FRONT_LEFT_BIT;
	case AUDIO_LOCATION_FRONT_RIGHT:
		return GAF_LOC_FRONT_RIGHT_BIT;
	case AUDIO_LOCATION_FRONT_CENTER:
		return GAF_LOC_FRONT_CENTER_BIT;
	case AUDIO_LOCATION_LFE1:
		return GAF_LOC_LFE1_BIT;
	case AUDIO_LOCATION_BACK_LEFT:
		return GAF_LOC_BACK_LEFT_BIT;
	case AUDIO_LOCATION_BACK_RIGHT:
		return GAF_LOC_BACK_RIGHT_BIT;
	case AUDIO_LOCATION_FRONT_LEFT_CENTER:
		return GAF_LOC_FRONT_LEFT_CENTER_BIT;
	case AUDIO_LOCATION_FRONT_RIGHT_CENTER:
		return GAF_LOC_FRONT_RIGHT_CENTER_BIT;
	case AUDIO_LOCATION_BACK_CENTER:
		return GAF_LOC_BACK_CENTER_BIT;
	case AUDIO_LOCATION_LFE2:
		return GAF_LOC_LFE2_BIT;
	case AUDIO_LOCATION_SIDE_LEFT:
		return GAF_LOC_SIDE_LEFT_BIT;
	case AUDIO_LOCATION_SIDE_RIGHT:
		return GAF_LOC_SIDE_RIGHT_BIT;
	case AUDIO_LOCATION_TOP_FRONT_LEFT:
		return GAF_LOC_TOP_FRONT_LEFT_BIT;
	case AUDIO_LOCATION_TOP_FRONT_RIGHT:
		return GAF_LOC_TOP_FRONT_RIGHT_BIT;
	case AUDIO_LOCATION_TOP_FRONT_CENTER:
		return GAF_LOC_TOP_FRONT_CENTER_BIT;
	case AUDIO_LOCATION_TOP_CENTER:
		return GAF_LOC_TOP_CENTER_BIT;
	case AUDIO_LOCATION_TOP_BACK_LEFT:
		return GAF_LOC_TOP_BACK_LEFT_BIT;
	case AUDIO_LOCATION_TOP_BACK_RIGHT:
		return GAF_LOC_TOP_BACK_RIGHT_BIT;
	case AUDIO_LOCATION_TOP_SIDE_LEFT:
		return GAF_LOC_TOP_SIDE_LEFT_BIT;
	case AUDIO_LOCATION_TOP_SIDE_RIGHT:
		return GAF_LOC_TOP_SIDE_RIGHT_BIT;
	case AUDIO_LOCATION_TOP_BACK_CENTER:
		return GAF_LOC_TOP_BACK_CENTER_BIT;
	case AUDIO_LOCATION_BOTTOM_FRONT_CENTER:
		return GAF_LOC_BOTTOM_FRONT_CENTER_BIT;
	case AUDIO_LOCATION_BOTTOM_FRONT_LEFT:
		return GAF_LOC_BOTTOM_FRONT_LEFT_BIT;
	case AUDIO_LOCATION_BOTTOM_FRONT_RIGHT:
		return GAF_LOC_BOTTOM_FRONT_RIGHT_BIT;
	case AUDIO_LOCATION_FRONT_LEFT_WIDE:
		return GAF_LOC_FRONT_LEFT_WIDE_BIT;
	case AUDIO_LOCATION_FRONT_RIGHT_WIDE:
		return GAF_LOC_FRONT_RIGHT_WIDE_BIT;
	case AUDIO_LOCATION_LEFT_SURROUND:
		return GAF_LOC_LEFT_SURROUND_BIT;
	case AUDIO_LOCATION_RIGHT_SURROUND:
		return GAF_LOC_RIGHT_SURROUND_BIT;
	default:
		LOG_INF("Defaulting to GAF mono audio");
#if !CONFIG_ALIF_BLE_ROM_IMAGE_V1_0 /* ROM version > 1.0 */
		return GAF_LOC_MONO_AUDIO;
#else
		return GAF_LOC_NOT_ALLOWED;
#endif
	}
}

uint32_t audio_bap_sampling_freq_to_hz(enum bap_sampling_freq freq)
{
	switch (freq) {
	case BAP_SAMPLING_FREQ_8000HZ: /* BAP_SAMPLING_FREQ_MIN implicitly set to 8000Hz */
		return 8000;
	case BAP_SAMPLING_FREQ_11025HZ:
		return 11025;
	case BAP_SAMPLING_FREQ_16000HZ:
		return 16000;
	case BAP_SAMPLING_FREQ_22050HZ:
		return 22050;
	case BAP_SAMPLING_FREQ_24000HZ:
		return 24000;
	case BAP_SAMPLING_FREQ_32000HZ:
		return 32000;
	case BAP_SAMPLING_FREQ_44100HZ:
		return 44100;
	case BAP_SAMPLING_FREQ_48000HZ:
		return 48000;
	case BAP_SAMPLING_FREQ_88200HZ:
		return 88200;
	case BAP_SAMPLING_FREQ_96000HZ:
		return 96000;
	case BAP_SAMPLING_FREQ_176400HZ:
		return 176400;
	case BAP_SAMPLING_FREQ_192000HZ:
		return 192000;
	case BAP_SAMPLING_FREQ_384000HZ: /* BAP_SAMPLING_FREQ_MAX implicitly set to 384000Hz */
		return 384000;
	default:
		__ASSERT(false, "Unknown sampling frequency");
		LOG_ERR("Unknown sampling frequency");
		return 0;
	}
}

enum bap_sampling_freq audio_hz_to_bap_sampling_freq(uint32_t rate)
{
	switch (rate) {
	case 8000:
		return BAP_SAMPLING_FREQ_8000HZ;
	case 11025:
		return BAP_SAMPLING_FREQ_11025HZ;
	case 16000:
		return BAP_SAMPLING_FREQ_16000HZ;
	case 22050:
		return BAP_SAMPLING_FREQ_22050HZ;
	case 24000:
		return BAP_SAMPLING_FREQ_24000HZ;
	case 32000:
		return BAP_SAMPLING_FREQ_32000HZ;
	case 44100:
		return BAP_SAMPLING_FREQ_44100HZ;
	case 48000:
		return BAP_SAMPLING_FREQ_48000HZ;
	case 88200:
		return BAP_SAMPLING_FREQ_88200HZ;
	case 96000:
		return BAP_SAMPLING_FREQ_96000HZ;
	case 176400:
		return BAP_SAMPLING_FREQ_176400HZ;
	case 192000:
		return BAP_SAMPLING_FREQ_192000HZ;
	case 384000:
		return BAP_SAMPLING_FREQ_384000HZ;
	default:
		__ASSERT(false, "Unknown sampling frequency");
		LOG_ERR("Unknown sampling frequency");
		return BAP_SAMPLING_FREQ_UNKNOWN;
	}
}

enum audio_frame_duration audio_bap_frame_dur_to_frame_dur(enum bap_frame_dur dur)
{
	switch (dur) {
	case BAP_FRAME_DUR_7_5MS:
		return AUDIO_FRAME_DURATION_7P5MS;
	case BAP_FRAME_DUR_10MS:
		return AUDIO_FRAME_DURATION_10MS;
	default:
		__ASSERT(false, "Unknown frame duration");
		LOG_ERR("Unknown frame duration");
		return 0;
	}
}

enum bap_frame_dur audio_frame_dur_to_bap_frame_dur(enum audio_frame_duration dur)
{
	switch (dur) {
	case AUDIO_FRAME_DURATION_7P5MS:
		return BAP_FRAME_DUR_7_5MS;
	case AUDIO_FRAME_DURATION_10MS:
		return BAP_FRAME_DUR_10MS;
	default:
		__ASSERT(false, "Unknown frame duration");
		LOG_ERR("Unknown frame duration");
		return 0;
	}
}
