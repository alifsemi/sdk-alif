/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _BLE_STORAGE_H
#define _BLE_STORAGE_H

#include <stdint.h>

#define BLE_BOND_KEYS_NAME "bond_keys"
#define BLE_BOND_DATA_NAME "bond_data"
#define BLE_BOND_PEER_NAME "bond_peer"
#define BLE_PRIV_ID_NAME   "priv_id"

/**
 * @brief Stores data in the flash memory.
 *
 * @param key Key string to identify the data.
 * @param data Pointer to the data to store.
 * @param size Size of the data.
 *
 * @return 0 on success, error code otherwise.
 */
int ble_storage_init(void);

/**
 * @brief Stores data in the flash memory.
 *
 * @param key Key string to identify the data.
 * @param data Pointer to the data to store.
 * @param size Size of the data.
 *
 * @return 0 on success, error code otherwise.
 */
int ble_storage_save(const char *key, void *data, size_t size);

/**
 * @brief Stores data with index in the flash memory.
 *
 * @param key Key string to identify the data.
 * @param index index for data item
 * @param data Pointer to the data to store.
 * @param size Size of the data.
 *
 * @return 0 on success, error code otherwise.
 */
int ble_storage_save_with_index(const char *key, int index, void *data, size_t const size);

/**
 * @brief Loads data from the flash memory.
 *
 * @param key Key string to identify the data.
 * @param data Pointer to store the loaded data.
 * @param size Size of the data to load.
 *
 * @return 0 on success, error code otherwise.
 */
int ble_storage_load(const char *key, void *data, size_t size);

/**
 * @brief Loads data with index from the flash memory.
 *
 * @param key Key string to identify the data.
 * @param index index for data item
 * @param data Pointer to store the loaded data.
 * @param size Size of the data to load.
 *
 * @return 0 on success, error code otherwise.
 */
int ble_storage_load_with_index(const char *key, int index, void *data, size_t const size);

#endif /* _BLE_STORAGE_H */
