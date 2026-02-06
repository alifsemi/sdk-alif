/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/shell/shell.h>
#include <stdio.h>
#include "gapm.h"
#include "main.h"
#include "auracast_source.h"
#include "auracast_sink.h"
#include "auracast_sd.h"

LOG_MODULE_REGISTER(auracast_shell, LOG_LEVEL_ERR);

static const char *param_get_str(size_t const argc, char **argv, char *p_param,
				 const char *p_def_value)
{
	if (p_param && argc > 1) {
		for (int n = 0; n < (argc - 1); n++) {
			if (strcmp(argv[n], p_param) == 0) {
				return argv[n + 1];
			}
		}
	}
	return p_def_value;
}

static int32_t param_get_int(size_t const argc, char **argv, char *p_param, int const def_value)
{
	const char *p_value = param_get_str(argc, argv, p_param, NULL);

	if (p_value) {
		return strtol(p_value, NULL, 0);
	}

	return def_value;
}

static int cmd_info(const struct shell *shell, size_t argc, char **argv)
{
	const enum role my_role = get_current_role();
	const char *info = get_device_name();
	gap_bdaddr_t identity;
	char addr_str[18];

	gapm_get_identity(&identity);

	snprintf(addr_str, sizeof(addr_str), "%02X:%02X:%02X:%02X:%02X:%02X", identity.addr[5],
		 identity.addr[4], identity.addr[3], identity.addr[2], identity.addr[1],
		 identity.addr[0]);

	shell_fprintf(shell, SHELL_VT100_COLOR_YELLOW, "Current config:\n");
	shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "  Device Name: ");
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "%s\n", info);
	shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "  Device Addr: ");
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "%s\n", addr_str);

	if (my_role >= ROLE_MAX) {
		shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "Device role is invalid\n");
		return 0;
	}

	const char *role_strs[ROLE_MAX] = {
		"None",
		"Auracast Source",
		"Auracast Sink",
		"Auracast Scan Delegator",
	};

	shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "  Role: ");
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "%s\n", role_strs[my_role]);

	info = get_stream_name();
	shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "  Stream name: ");
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "%s\n", info ? info : "<not set>");

	info = get_auracast_encryption_passwd();
	shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "  Stream encryption password: ");
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "%s\n", info ? info : "<not set>");

	return 0;
}

static int cmd_set_device_name(const struct shell *shell, size_t const argc, char **argv)
{
	if (argc < 2) {
		LOG_ERR("argument 'name' is missing");
		return -EINVAL;
	}

	const char *device_name = argv[1];

	return set_device_name(device_name);
}

static int cmd_start_source(const struct shell *shell, size_t const argc, char **argv)
{
	if (argc < 2) {
		LOG_ERR("argument 'name' is missing");
		return -EINVAL;
	}

	struct startup_params command = {
		.cmd = COMMAND_SOURCE,
	};

	const char *stream_name = argv[1];
	const char *passwd = param_get_str(argc, argv, "--passwd", NULL);
	const char *codec = param_get_str(argc, argv, "--codec", NULL);

	if (codec) {
		LOG_DBG("Using '%s' codec configuration", codec);

		if (strcmp(codec, "8_1") == 0) {
			command.source.frame_rate_hz = 8000;
			command.source.frame_duration_us = 7500;
			command.source.octets_per_frame = 26;
		} else if (strcmp(codec, "8_2") == 0) {
			command.source.frame_rate_hz = 8000;
			command.source.frame_duration_us = 10000;
			command.source.octets_per_frame = 30;
		} else if (strcmp(codec, "16_1") == 0) {
			command.source.frame_rate_hz = 16000;
			command.source.frame_duration_us = 7500;
			command.source.octets_per_frame = 30;
		} else if (strcmp(codec, "16_2") == 0) {
			command.source.frame_rate_hz = 16000;
			command.source.frame_duration_us = 10000;
			command.source.octets_per_frame = 40;
		} else if (strcmp(codec, "24_1") == 0) {
			command.source.frame_rate_hz = 24000;
			command.source.frame_duration_us = 7500;
			command.source.octets_per_frame = 45;
		} else if (strcmp(codec, "24_2") == 0) {
			command.source.frame_rate_hz = 24000;
			command.source.frame_duration_us = 10000;
			command.source.octets_per_frame = 60;
		} else if (strcmp(codec, "32_1") == 0) {
			command.source.frame_rate_hz = 32000;
			command.source.frame_duration_us = 7500;
			command.source.octets_per_frame = 60;
		} else if (strcmp(codec, "32_2") == 0) {
			command.source.frame_rate_hz = 32000;
			command.source.frame_duration_us = 10000;
			command.source.octets_per_frame = 80;
#if CODEC_44khz_SUPPORT_ENABLED
		/* 44.1 kHz is not fully functional and need to be fixed */
		} else if (strcmp(codec, "441_1") == 0) {
			command.source.frame_rate_hz = 44100;
			command.source.frame_duration_us = 7500;
			command.source.octets_per_frame = 97;
		} else if (strcmp(codec, "441_2") == 0) {
			command.source.frame_rate_hz = 44100;
			command.source.frame_duration_us = 10000;
			command.source.octets_per_frame = 130;
#endif
		} else if (strncmp(codec, "48_", 3) == 0) {
			command.source.frame_rate_hz = 48000;

			const uint_fast8_t sub_ver = codec[3] - '0';
			const uint8_t octets_map[] = {75, 100, 90, 120, 117, 155};

			if (sub_ver >= ARRAY_SIZE(octets_map)) {
				LOG_ERR("Invalid codec name '%s', use '48_0' to '48_5'", codec);
				return -EINVAL;
			}

			command.source.frame_duration_us = (sub_ver & 1) ? 7500 : 10000;
			command.source.octets_per_frame = octets_map[sub_ver];
		} else {
			LOG_ERR("Invalid codec name '%s', use 'standard' or 'high'", codec);
			return -EINVAL;
		}

	} else {
		const char *p_duration = param_get_str(argc, argv, "--ms", NULL);

		command.source.octets_per_frame = param_get_int(
			argc, argv, "--sdu", CONFIG_ALIF_BLE_AUDIO_OCTETS_PER_CODEC_FRAME);
		command.source.frame_rate_hz =
			param_get_int(argc, argv, "--rate", CONFIG_ALIF_BLE_AUDIO_FS_HZ);

		if (!p_duration) {
			/* If frame duration is not provided, use default based on config */
			command.source.frame_duration_us =
				IS_ENABLED(CONFIG_ALIF_BLE_AUDIO_FRAME_DURATION_10MS) ? 10000
										      : 7500;
		} else if (strncmp(p_duration, "10", 2) == 0) {
			command.source.frame_duration_us = 10000;
		} else if (strncmp(p_duration, "7.5", 2) == 0) {
			command.source.frame_duration_us = 7500;
		} else {
			LOG_ERR("Invalid frame duration '%s', use '10' or '7.5'", p_duration);
			return -EINVAL;
		}
	}

	set_device_name(stream_name);
	set_stream_name(stream_name);
	set_auracast_encryption_passwd(passwd);

	return execute_shell_command(command);
}

static int cmd_start_sink(const struct shell *shell, size_t const argc, char **argv)
{
	const char *stream_name = (argc >= 2) ? argv[1] : NULL;
	const char *passwd = (argc >= 3) ? argv[2] : NULL;

	set_stream_name(stream_name);
	set_auracast_encryption_passwd(passwd);

	return execute_shell_command((struct startup_params){
		.cmd = COMMAND_SINK,
	});
}

static int cmd_select(const struct shell *shell, size_t const argc, char **argv)
{
	if (argc < 2) {
		LOG_ERR("argument 'index' is missing");
		return -EINVAL;
	}

	const int index = strtol(argv[1], NULL, 0);
	const char *passwd = (argc >= 3) ? argv[2] : NULL;

	set_auracast_encryption_passwd(passwd);

	return execute_shell_command((struct startup_params){
		.cmd = COMMAND_SINK_SELECT_STREAM,
		.sink.stream_index = index,
	});
}

static int cmd_start_scan_delegator(const struct shell *shell, size_t const argc, char **argv)
{
	const char *device_name = (argc > 1) ? argv[1] : DEVICE_NAME_PREFIX_DEFAULT " SD";

	if (set_device_name(device_name) < 0) {
		LOG_ERR("Failed to set device name");
		return -EINVAL;
	}

	return execute_shell_command((struct startup_params){
		.cmd = COMMAND_SCAN_DELEGATOR,
	});
}

static int cmd_stop(const struct shell *shell, size_t const argc, char **argv)
{
	return execute_shell_command((struct startup_params){
		.cmd = COMMAND_STOP,
	});
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_cfg, SHELL_CMD_ARG(info, NULL, "Print device info", cmd_info, 1, 10),
	SHELL_CMD_ARG(name, NULL, "Set device name <name>", cmd_set_device_name, 1, 10),
	SHELL_CMD_ARG(
		source, NULL,
		"Start Auracast transmitter <stream name> [--passwd <password>] "
		"[--codec <name>] [--sdu <octets_per_frame_in_bytes>] [--rate <frame_rate_hz>] "
		"[--ms <frame_duration_in_ms>]",
		cmd_start_source, 1, 10),
	SHELL_CMD_ARG(sink, NULL, "Start Auracast receiver [<stream name> [password]]",
		      cmd_start_sink, 1, 10),
	SHELL_CMD_ARG(delegator, NULL, "Start Auracast scan delegator [device name]",
		      cmd_start_scan_delegator, 1, 10),
	SHELL_CMD_ARG(select, NULL, "Select Auracast stream <index> [password]", cmd_select, 1, 10),
	SHELL_CMD_ARG(stop, NULL, "Stop Auracast", cmd_stop, 1, 10),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(auracast, &sub_cfg, "Auracast config", NULL);
