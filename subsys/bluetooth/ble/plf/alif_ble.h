/*
 * Copyright (c) 2023 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ALIF_BLE_H
#define _ALIF_BLE_H

/**
 * @brief Enable the Alif BLE stack. This must be called before any other Alif BLE API calls.
 *
 * @param cb If non-NULL, the BLE stack initialisation will take place asynchronously and the
 *           provided callback function will be called when BLE is ready to use. If NULL, then
 *           initialisation takes place synchronously and this function call will block until BLE
 *           is ready.
 *
 * @return 0 on success, or error code
 */
int alif_ble_enable(void (*cb)(void));

/**
 ****************************************************************************************
 * @brief Stop BLE zephyr thread and es0.
 *
 * @return 0 on success, or negative error code
 ****************************************************************************************
 */
int alif_ble_disable(void);

/**
 * @brief Acquire mutex lock to BLE stack processing. This must be called before using any
 * Alif BLE APIs outside the callbacks provided by the stack. Corresponding call to
 * alif_ble_mutex_unlock must be done to unlock the mutex and to let the BLE stack continue
 * processing.
 *
 * @param timeout Maximum wait time to wait for mutex locking. Use K_NO_WAIT for non blocking
 * operations and K_FOREVER to wait indefinitely.
 *
 * @return 0 on success. -EBUSY if locking failed without waiting. -EAGAIN on timeout.
 */
int alif_ble_mutex_lock(k_timeout_t timeout);

/**
 * @brief Release BLE stack processing mutex. Must only be used after a successful call to
 * alif_ble_mutex_lock.
 *
 */
void alif_ble_mutex_unlock(void);


#endif /* _ALIF_BLE_H */
