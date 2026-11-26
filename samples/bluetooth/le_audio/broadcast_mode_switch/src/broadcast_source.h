/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _BROADCAST_SOURCE_H
#define _BROADCAST_SOURCE_H

#include <stdbool.h>

/**
 * @brief Configure the BAP BC source module
 *
 * This function should be called once during initialization.
 * Subsequent calls will be ignored if already configured.
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int broadcast_source_configure(void);

/**
 * @brief Start the LE audio broadcast source
 *
 * This function will configure the modules if not already done,
 * then start the broadcast source.
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int broadcast_source_start(void);

/**
 * @brief Start broadcasting (enable PA and start streaming)
 *
 * The user asked for a source. Create BIG waits while the mode-switch
 * link is up. The BAP BC source modules must already be configured.
 * A failed Create BIG stays inside the source.
 *
 * @retval 0 if the request was recorded
 * @retval Negative error code if the modules are not configured
 */
int broadcast_source_start_broadcasting(void);

/**
 * @brief The user asked for the source to stop
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int broadcast_source_stop(void);

/**
 * @brief Report whether the mode-switch link is up
 *
 * Create BIG waits while that link is up.
 */
void broadcast_source_set_connected(bool connected);

/**
 * @brief The mode-switch link dropped
 */
void broadcast_source_on_disconnected(void);

#endif /* _BROADCAST_SOURCE_H */
