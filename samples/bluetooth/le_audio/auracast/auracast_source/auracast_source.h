/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _AURACAST_SOURCE_H
#define _AURACAST_SOURCE_H

/**
 * @brief Start the LE audio auracast source
 *
 * @param octets_per_frame Number of octets per audio frame
 * @param frame_rate_hz Frame rate in Hz
 * @param frame_duration_us Frame duration in microseconds
 *
 * @return 0 on success
 */
int auracast_source_start(uint32_t octets_per_frame, uint32_t frame_rate_hz,
			  uint32_t frame_duration_us);

/**
 * @brief Stop the LE audio auracast source
 *
 * @return 0 on success
 */
int auracast_source_stop(void);

#endif /* _AURACAST_SOURCE_H */
