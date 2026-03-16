/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/logging/log.h>
#include <zephyr/settings/settings.h>
#include <stdio.h>
#include <stdlib.h>
#include "storage.h"

LOG_MODULE_REGISTER(storage, CONFIG_STORAGE_LOG_LEVEL);

struct storage_ctx {
	uint8_t *p_output;
	size_t size;
};

static int settings_direct_loader(const char *const key, size_t const len,
				  settings_read_cb const read_cb, void *cb_arg, void *param)
{
	struct storage_ctx *p_ctx = (struct storage_ctx *)param;

	/* Handle the subtree if it is an exact key match. */
	if (settings_name_next(key, NULL) == 0) {
		ssize_t const cb_len = read_cb(cb_arg, p_ctx->p_output, p_ctx->size);

		if (cb_len != p_ctx->size) {
			LOG_ERR("Unable to read bytes_written from storage");
			return cb_len;
		}
	}

	return 0;
}

static void *find_a_node_by_index(int32_t const index, sys_slist_t *list)
{
	struct peer_data *p_peer;
	sys_snode_t *node = NULL;

	SYS_SLIST_ITERATE_FROM_NODE(list, node)
	{
		p_peer = (struct peer_data *)node;
		if (p_peer->storage_index == index) {
			return p_peer;
		}
	}
	return NULL;
}

static int settings_load_all(const char *const key, size_t const len,
			     settings_read_cb const read_cb, void *cb_arg, void *param)
{
	const int32_t storage_index = strtol(key, NULL, 0);
	struct storage_ctx *p_ctx = (struct storage_ctx *)param;
	sys_slist_t *p_list = (sys_slist_t *)p_ctx->p_output;
	const char *next;

	LOG_DBG("Load %s len: %d (index %d)", key, len, storage_index);

	if (!settings_name_next(key, &next)) {
		/* Invalid key received, just continue */
		return 0;
	}

	if (!next) {
		/* sub key should exist, ignore if not */
		return 0;
	}

	struct peer_data *node = find_a_node_by_index(storage_index, p_list);

	if (!node) {
		node = calloc(1, sizeof(*node));
		if (!node) {
			LOG_ERR("Failed to allocate memory for peer data");
			return -ENOMEM;
		}
		node->storage_index = storage_index;
		sys_slist_append(p_list, &node->node);
	}

	struct bond_data *p_data = NULL;

	if (!strcmp(next, SETTINGS_NAME_KEYS)) {
		p_data = &node->keys;
	} else if (!strcmp(next, SETTINGS_NAME_CONNECTION)) {
		p_data = &node->connection;
	} else if (!strcmp(next, SETTINGS_NAME_VOLUME)) {
		p_data = &node->connection;
	} else if (!strcmp(next, SETTINGS_NAME_ACSC)) {
		p_data = &node->acsc;
	} else if (!strcmp(next, SETTINGS_NAME_PACS)) {
		p_data = &node->pacs;
	} else {
		LOG_ERR("Invalid storage key %s", key);
		return 0;
	}

	if (!p_data->data) {
		p_data->data = malloc(len);
		if (!p_data->data) {
			LOG_ERR("Failed to allocate memory for keys data");
			return -ENOMEM;
		}
	}

	p_data->size = len;

	ssize_t const cb_len = read_cb(cb_arg, p_data->data, len);

	if (cb_len != len) {
		LOG_ERR("Unable to read data from storage");
		return cb_len;
	}

	return 0;
}

int storage_init(void)
{
	static bool initialized;

	if (initialized) {
		return 0;
	}

	int err = settings_subsys_init();

	if (err) {
		LOG_ERR("settings_subsys_init() failed (err %d)", err);
		return err;
	}

	initialized = true;
	return 0;
}

int storage_load_all(sys_slist_t *const p_found_peers)
{
	struct storage_ctx ctx = {
		.p_output = (void *)p_found_peers,
		.size = 0,
	};
	const int err = settings_load_subtree_direct(SETTINGS_BASE, settings_load_all, &ctx);

	if (err) {
		LOG_ERR("Failed to load " SETTINGS_BASE " data (err %d)", err);
	}

	return err;
}

int storage_store(const char *key, const int index, void *data, size_t const size)
{
	int err;
	char key_str[64];

	snprintf(key_str, sizeof(key_str), SETTINGS_BASE "/%d/%s", index, key);
	err = settings_save_one(key_str, data, size);
	if (err) {
		LOG_ERR("Failed to store %s data (err %d)", key, err);
	}
	return err;
}

int storage_load(const char *key, const int index, void *data, size_t const size)
{
	struct storage_ctx ctx = {
		.p_output = data,
		.size = size,
	};
	char key_str[64];

	snprintf(key_str, sizeof(key_str), SETTINGS_BASE "/%d/%s", index, key);

	int err = settings_load_subtree_direct(key_str, settings_direct_loader, &ctx);

	if (err) {
		LOG_ERR("Failed to load %s data (err %d)", key, err);
	}

	return err;
}
