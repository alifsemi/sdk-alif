/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _AURACAST_SINK_H
#define _AURACAST_SINK_H

/**
 * @brief Start the LE audio auracast sink
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int auracast_sink_start(void);

/**
 * @brief Stop the LE audio auracast sink
 */
void auracast_sink_stop(void);

/**
 * @brief Select a stream to be enabled
 *
 * @param stream_index The index of the stream to be enabled
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int auracast_sink_select_stream(int stream_index);

#endif /* _AURACAST_SINK_H */
