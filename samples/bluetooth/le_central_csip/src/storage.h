/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _STORAGE_H
#define _STORAGE_H

#include <stdint.h>
#include "atc_csisc.h"

#define SETTINGS_BASE            "uc_central"
#define SETTINGS_NAME_KEYS       "keys"
#define SETTINGS_NAME_CONNECTION "connection"

/*
 * Storage format will be:
 *   uc_central/<storage index>/<data name>
 */

struct bond_data {
	void *data;
	size_t size;
};

struct peer_data {
	sys_snode_t node;
	uint16_t storage_index;
	uint8_t actv_idx;
	uint8_t conidx;
	struct bond_data keys;            /* gapc_pairing_keys_t */
	struct bond_data connection;      /* gapc_bond_data_t */
};

/**
 * @brief Stores data in the flash memory.
 *
 * @param key Key string to identify the data.
 * @param data Pointer to the data to store.
 * @param size Size of the data.
 *
 * @return 0 on success, error code otherwise.
 */
int storage_init(void);

/**
 * @brief Loads all stored data from the flash memory.
 *
 * @param p_found_peers Pointer to the list of bonded peers.
 *
 * @return 0 on success, error code otherwise.
 */
int storage_load_all(sys_slist_t *p_found_peers);

/**
 * @brief Stores data in the flash memory.
 *
 * @param key Key string to identify the data.
 * @param index Index of the data to store.
 * @param data Pointer to the data to store.
 * @param size Size of the data.
 *
 * @return 0 on success, error code otherwise.
 */
int storage_store(const char *key, int index, void *data, size_t size);

/**
 * @brief Deletes data from the flash memory.
 *
 * @param key Key string to identify the data.
 * @param index Index of the data to delete.
 *
 * @return 0 on success, error code otherwise.
 */
int storage_delete(const char *key, const int index);

/**
 * @brief Loads data from the flash memory.
 *
 * @param key Key string to identify the data.
 * @param index Index of the data to load.
 * @param data Pointer to store the loaded data.
 * @param size Size of the data to load.
 *
 * @return 0 on success, error code otherwise.
 */
int storage_load(const char *key, int index, void *data, size_t size);

/**
 * @brief Retrieves a free peer context from the pool.
 * @return Pointer to the free peer context, or NULL if no free context is available.
 */
struct peer_data *storage_allocate_peer_context(void);

/**
 * @brief Frees a peer context and returns it to the pool.
 * @param p_peer Pointer to the peer context to free.
 */
void storage_free_peer_context(struct peer_data *p_peer);

#endif /* _STORAGE_H */
