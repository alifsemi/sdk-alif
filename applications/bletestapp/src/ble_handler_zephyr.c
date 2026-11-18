/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "ble_handler.h"

#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>

#include "appl_shell.h"
#include <zephyr/shell/shell.h>
#include <zephyr/shell/shell_uart.h>

LOG_MODULE_DECLARE(main, CONFIG_MAIN_LOG_LEVEL);

static uint8_t hello_arr[] = "HelloHello";
static uint8_t hello_arr_index __attribute__((noinit));

/* BLE configurations*/
char __attribute__((noinit)) app_shell_device_name[DEVICE_NAME_LEN];

uint16_t ble_adv_int_min __attribute__((noinit));
uint16_t ble_adv_int_max __attribute__((noinit));
uint16_t ble_conn_int_min __attribute__((noinit));
uint16_t ble_conn_int_max __attribute__((noinit));

/* HELLO SERVICE and attribute 128 bit UUIDs */
#define HELLO_UUID_128_SVC_BYTES \
	0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x90,       \
		0x00, 0x00
#define HELLO_UUID_128_CHAR0_BYTES \
	0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x15,       \
		0x00, 0x00
#define HELLO_UUID_128_CHAR1_BYTES \
	0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x16,       \
		0x00, 0x00

struct service_env {
	uint8_t char0_val[CONFIG_DATA_STRING_LENGTH];
	uint8_t char1_val;
	bool ntf_enabled;
};

static struct service_env env __attribute__((noinit));
static struct bt_conn *current_conn;
static bool conn_status __attribute__((noinit));
static bool ble_enabled;
static bool adv_restart_pending;

static struct bt_uuid_128 hello_svc_uuid = BT_UUID_INIT_128(HELLO_UUID_128_SVC_BYTES);
static struct bt_uuid_128 hello_char0_uuid = BT_UUID_INIT_128(HELLO_UUID_128_CHAR0_BYTES);
static struct bt_uuid_128 hello_char1_uuid = BT_UUID_INIT_128(HELLO_UUID_128_CHAR1_BYTES);

static ssize_t hello_read_char0(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
				uint16_t len, uint16_t offset)
{
	ARG_UNUSED(conn);

	uint8_t loop_count = (CONFIG_DATA_STRING_LENGTH / 5U);

	if (CONFIG_DATA_STRING_LENGTH % 5U) {
		loop_count += 1U;
	}

	for (uint8_t i = 0U; i < loop_count; i++) {
		memcpy(env.char0_val + i * 5U, &hello_arr[hello_arr_index], 5U);
	}

	return bt_gatt_attr_read(conn, attr, buf, len, offset, env.char0_val,
				 CONFIG_DATA_STRING_LENGTH);
}

static ssize_t hello_write_char1(struct bt_conn *conn, const struct bt_gatt_attr *attr,
				 const void *buf, uint16_t len, uint16_t offset,
				 uint8_t flags)
{
	ARG_UNUSED(conn);
	ARG_UNUSED(attr);
	ARG_UNUSED(flags);

	if (offset != 0U) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (len != sizeof(env.char1_val)) {
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	memcpy(&env.char1_val, buf, sizeof(env.char1_val));
	LOG_DBG("TOGGLE LED, state %d", env.char1_val);

	return len;
}

static void hello_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);
	env.ntf_enabled = (value == BT_GATT_CCC_NOTIFY);
}

BT_GATT_SERVICE_DEFINE(
	hello_svc, BT_GATT_PRIMARY_SERVICE(&hello_svc_uuid.uuid),
	BT_GATT_CHARACTERISTIC(&hello_char0_uuid.uuid, BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, hello_read_char0, NULL, NULL),
	BT_GATT_CCC(hello_ccc_cfg_changed, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(&hello_char1_uuid.uuid, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE,
			       NULL, hello_write_char1, &env.char1_val));

static int start_advertising(void);

static void adv_restart_work_handler(struct k_work *work)
{
	ARG_UNUSED(work);

	if (!ble_enabled || !adv_restart_pending || conn_status) {
		return;
	}

	int err = start_advertising();

	if (!err) {
		adv_restart_pending = false;
		return;
	}

	if (err == -ENOMEM) {
		LOG_WRN("Advertising restart deferred, waiting for conn recycle");
		return;
	}

	adv_restart_pending = false;
	LOG_ERR("restart advertising failed %d", err);
}

K_WORK_DEFINE(adv_restart_work, adv_restart_work_handler);

static int start_advertising(void)
{
	struct bt_le_adv_param adv_param = {
		.id = BT_ID_DEFAULT,
		.sid = 0,
		.secondary_max_skip = 0,
		.options = BT_LE_ADV_OPT_CONN | BT_LE_ADV_OPT_USE_IDENTITY,
		.interval_min = ble_adv_int_min,
		.interval_max = ble_adv_int_max,
		.peer = NULL,
	};
	struct bt_data ad[] = {
		BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
		BT_DATA_BYTES(BT_DATA_UUID128_ALL, HELLO_UUID_128_SVC_BYTES),
		BT_DATA(BT_DATA_NAME_COMPLETE, app_shell_device_name,
			strlen(app_shell_device_name)),
	};

	int err = bt_set_name(app_shell_device_name);

	if (err) {
		LOG_WRN("bt_set_name failed %d, continuing with default device name", err);
	}

	(void)bt_le_adv_stop();

	err = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("bt_le_adv_start error %d", err);
		return err;
	}

	LOG_INF("Advertising started name:%s int:[%u,%u]", app_shell_device_name,
		ble_adv_int_min, ble_adv_int_max);

	return 0;
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	const struct shell *shell = shell_backend_uart_get_ptr();

	if (err) {
		shell_print(shell, "BLE connect failed (%u)", err);
		return;
	}

	if (current_conn != NULL) {
		bt_conn_unref(current_conn);
	}
	current_conn = bt_conn_ref(conn);
	conn_status = true;
	adv_restart_pending = false;

	struct bt_le_conn_param param = {
		.interval_min = ble_conn_int_min,
		.interval_max = ble_conn_int_max,
		.latency = 0,
		.timeout = 800,
	};

	err = bt_conn_le_param_update(conn, &param);
	if (err) {
		LOG_WRN("bt_conn_le_param_update failed %u", err);
	}

	shell_print(shell, "BLE Connected");
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	ARG_UNUSED(conn);
	const struct shell *shell = shell_backend_uart_get_ptr();

	conn_status = false;
	if (current_conn != NULL) {
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}

	shell_print(shell, "BLE %s reason 0x%02x. Waiting new connection", __func__, reason);
	adv_restart_pending = true;
	k_work_submit(&adv_restart_work);
}

static void recycled(void)
{
	if (adv_restart_pending) {
		k_work_submit(&adv_restart_work);
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
	.recycled = recycled,
};

int ble_shell_reset(void)
{
	ble_adv_int_min = 1000;
	ble_adv_int_max = 1000;
	ble_conn_int_min = 800;
	ble_conn_int_max = 800;
	strncpy(app_shell_device_name, "test_shl", DEVICE_NAME_LEN - 1);
	app_shell_device_name[8] = 0;
	return 0;
}

SYS_INIT(ble_shell_reset, POST_KERNEL, 91);

int ble_init(void)
{
	if (ble_enabled) {
		return 0;
	}

	int err = bt_enable(NULL);

	if (err && err != -EALREADY) {
		LOG_ERR("bt_enable error %d", err);
		return err;
	}

	if (!ble_enabled) {
		bt_conn_cb_register(&conn_callbacks);
	}

	ble_enabled = true;

	if (app_shell_device_name[0] != '\0') {
		err = bt_set_name(app_shell_device_name);
		if (err) {
			LOG_WRN("bt_set_name failed %d", err);
		}
	}

	return 0;
}

int ble_start(void)
{
	if (!ble_enabled) {
		LOG_ERR("Bluetooth stack not initialized");
		return -EACCES;
	}

	hello_arr_index = 0;
	conn_status = false;
	adv_restart_pending = false;
	memset(&env, 0, sizeof(struct service_env));

	int err = start_advertising();

	if (err) {
		return err;
	}

	LOG_INF("start complete!");
	return 0;
}

int ble_uninit(void)
{
	conn_status = false;
	adv_restart_pending = false;
	(void)k_work_cancel(&adv_restart_work);
	(void)bt_le_adv_stop();

	if (current_conn != NULL) {
		(void)bt_conn_disconnect(current_conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}

	return 0;
}

bool ble_is_connected(void)
{
	return conn_status;
}

static int cmd_bt_disable(const struct shell *shell, size_t argc, char **argv)
{
	int ret;

	ble_shell_reset();
	ret = ble_uninit();
	shell_print(shell, "BLE application is restarted %d", ret);
	return 0;
}

static int cmd_bt_init(const struct shell *shell, size_t argc, char **argv)
{
	int ret = ble_init();

	shell_print(shell, "BLE stack init %d", ret);
	return 0;
}

static int cmd_bt_name(const struct shell *shell, size_t argc, char **argv)
{
	/* app_shell_device_name has null-ending as per cold boot init */
	strncpy(app_shell_device_name, argv[1], DEVICE_NAME_LEN - 1);
	shell_print(shell, "BLE name set to %s", app_shell_device_name);
	return 0;
}

static int cmd_bt_advertise(const struct shell *shell, size_t argc, char **argv)
{
	int ret;

	ret = ble_start();
	shell_print(shell, "BLE advertisement started %d", ret);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_cmds,
	SHELL_CMD_ARG(init, NULL, "[none]", cmd_bt_init, 1, 10),
	SHELL_CMD_ARG(advertise, NULL, "<type: off, on, nconn>", cmd_bt_advertise, 1, 10),
	SHELL_CMD_ARG(disable, NULL, "[none]", cmd_bt_disable, 1, 10),
	SHELL_CMD_ARG(name, NULL, "[name]", cmd_bt_name, 2, 10),
	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(bt, &sub_cmds, "bt - Bluetooth shell commands", NULL);
