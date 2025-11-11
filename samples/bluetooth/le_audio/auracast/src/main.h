/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef MAIN_H
#define MAIN_H

#include <zephyr/types.h>
#include <gaf/gaf.h>

#define DEVICE_NAME_PREFIX_DEFAULT "ALIF Auracast"

/* Appearance */
#define APPEARANCE_EARBUDS 0x0941
#define APPEARANCE_HEADSET 0x0942

/* TODO: Change appearance to Headset when bidir communication is implemented */
#define APPEARANCE APPEARANCE_EARBUDS

enum role {
	ROLE_NONE,
	ROLE_AURACAST_SOURCE,
	ROLE_AURACAST_SINK,
	ROLE_AURACAST_SCAN_DELEGATOR,
	ROLE_MAX,
};

enum command {
	COMMAND_STOP,
	COMMAND_SOURCE,
	COMMAND_SINK,
	COMMAND_SINK_SELECT_STREAM,
	COMMAND_SCAN_DELEGATOR,
	COMMAND_MAX,
};

struct startup_params {
	enum command cmd;
	union {
		struct {
			uint32_t octets_per_frame;
			uint32_t frame_rate_hz;
			uint32_t frame_duration_us;
		} source;
		struct {
			int stream_index;
		} sink;
	};
};

/**
 * @brief Set device name
 *
 * @param name Device name string
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int set_device_name(const char *name);

/**
 * @brief Get the current device name
 *
 * @retval Device name string
 */
const char *get_device_name(void);

/**
 * @brief Set stream name
 *
 * @param name Stream name string
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int set_stream_name(const char *name);

/**
 * @brief Get the current stream name
 *
 * @retval Stream name string
 */
const char *get_stream_name(void);

/**
 * @brief Set Auracast encryption password
 *
 * @param passwd Password string
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int set_auracast_encryption_passwd(const char *passwd);

/**
 * @brief Get Auracast encryption password
 *
 * @retval Password string
 */
const char *get_auracast_encryption_passwd(void);

/**
 * @brief Fill the Auracast encryption key
 *
 * @param key_buffer Buffer to fill with the encryption key
 * @retval password length on success
 * @retval Negative error code on failure
 */
int fill_auracast_encryption_key(gaf_bcast_code_t *p_code);

/**
 * @brief Configure a role
 *
 * @param role Desired role to configure
 * @retval 0 on success
 * @retval -1 on failure
 */
int configure_role(enum role role);

/**
 * @brief Get the current configured role
 *
 * @retval Current role
 */
enum role get_current_role(void);

/**
 * @brief Execute a shell command in the BLE worker thread
 *
 * @param cmd Command to execute \ref struct startup_params
 *
 * @retval 0 on success
 * @retval Negative error code on failure
 */
int execute_shell_command(struct startup_params cmd);

#endif /* MAIN_H */
