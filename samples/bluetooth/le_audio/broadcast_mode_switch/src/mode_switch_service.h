/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef MODE_SWITCH_SERVICE_H_
#define MODE_SWITCH_SERVICE_H_

#include <zephyr/types.h>

/* Mode definitions */
#define MODE_SWITCH_IDLE    0  /* Device in idle mode - not active */
#define MODE_SWITCH_SINK    1  /* Device in broadcast sink mode */
#define MODE_SWITCH_SOURCE  2  /* Device in broadcast source mode */

/**
 * @brief Start the mode switch service
 *
 * This creates and registers the custom GATT service that allows clients
 * to switch between broadcast sink and source modes.
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int mode_switch_service_start(void);

/**
 * @brief Stop the mode switch service
 *
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int mode_switch_service_stop(void);

/**
 * @brief Get the current mode
 *
 * @return Current mode (0 = idle, 1 = sink, 2 = source)
 */
uint8_t mode_switch_service_get_mode(void);

/**
 * @brief Set the current mode
 *
 * @param mode New mode (0 = idle, 1 = sink, 2 = source)
 * @retval 0 if successful
 * @retval Negative error code on failure
 */
int mode_switch_service_set_mode(uint8_t mode);



#endif /* MODE_SWITCH_SERVICE_H_ */
