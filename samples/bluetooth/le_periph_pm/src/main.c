/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/policy.h>
#include <zephyr/sys/poweroff.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/drivers/uart.h>
#include <cmsis_core.h>
#include <soc_common.h>
#include <se_service.h>
#include <es0_power_manager.h>

#include "alif_ble.h"
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "prf.h"
#include "gatt_db.h"
#include "gatt_srv.h"
#include "ke_mem.h"

#define DEBUG_PIN_NODE DT_ALIAS(debug_pin)

#if DT_NODE_EXISTS(DEBUG_PIN_NODE)
static const struct gpio_dt_spec debug_pin = GPIO_DT_SPEC_GET_OR(DEBUG_PIN_NODE, gpios, {0});
#endif

#if !defined(CONFIG_SOC_SERIES_B1)
#error "Application works only with B1 devices"
#endif

/**
 * As per the application requirements, it can remove the memory blocks which are not in use.
 */
#define APP_RET_MEM_BLOCKS                                                                         \
	SRAM4_1_MASK | SRAM4_2_MASK | SRAM4_3_MASK | SRAM4_4_MASK | SRAM5_1_MASK | SRAM5_2_MASK |  \
		SRAM5_3_MASK | SRAM5_4_MASK | SRAM5_5_MASK
#define SERAM_MEMORY_BLOCKS_IN_USE SERAM_1_MASK | SERAM_2_MASK | SERAM_3_MASK | SERAM_4_MASK

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(rtc0), snps_dw_apb_rtc, okay)
#define WAKEUP_SOURCE         DT_NODELABEL(rtc0)
#define SE_OFFP_EWIC_CFG      EWIC_RTC_A
#define SE_OFFP_WAKEUP_EVENTS WE_LPRTC
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(timer0), snps_dw_timers, okay)
#define WAKEUP_SOURCE         DT_NODELABEL(timer0)
#define SE_OFFP_EWIC_CFG      EWIC_VBAT_TIMER
#define SE_OFFP_WAKEUP_EVENTS WE_LPTIMER0
#else
#error "Wakeup Device not enabled in the dts"
#endif

#define WAKEUP_SOURCE_IRQ     DT_IRQ_BY_IDX(WAKEUP_SOURCE, 0, irq)

/**
 * Use the HFOSC clock for the UART console
 */
#if DT_SAME_NODE(DT_NODELABEL(uart4), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM 4
#define EARLY_BOOT_CONSOLE_INIT 1
#elif DT_SAME_NODE(DT_NODELABEL(uart3), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM 3
#define EARLY_BOOT_CONSOLE_INIT 1
#elif DT_SAME_NODE(DT_NODELABEL(uart2), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM 2
#define EARLY_BOOT_CONSOLE_INIT 1
#elif DT_SAME_NODE(DT_NODELABEL(uart1), DT_CHOSEN(zephyr_console))
#define CONSOLE_UART_NUM 1
#define EARLY_BOOT_CONSOLE_INIT 1
#else
#define EARLY_BOOT_CONSOLE_INIT 0
#endif

#if EARLY_BOOT_CONSOLE_INIT
#define UART_CTRL_CLK_SEL_POS 8
static int app_pre_console_init(void)
{
	/* Enable HFOSC in CGU */
	sys_set_bits(CGU_CLK_ENA, BIT(23));

	/* Enable HFOSC for the UART console */
	sys_clear_bits(EXPSLV_UART_CTRL, BIT((CONSOLE_UART_NUM + UART_CTRL_CLK_SEL_POS)));
	return 0;
}

/**
 * Console UART init is needed to adjust clocks properly.
 * This must be fixed into UART dirver and this code can be
 * removed when ready.
 */
SYS_INIT(app_pre_console_init, PRE_KERNEL_1, 50);
#endif

/* Configuration for different BLE and application timing parameters
 */
#ifdef WAKEUP_STRESS_TEST
int n __attribute__((noinit));
#define ADV_INT_MIN_SLOTS                100
#define ADV_INT_MAX_SLOTS                150
#define CONN_INT_MIN_SLOTS               20
#define CONN_INT_MAX_SLOTS               100
#define RTC_WAKEUP_INTERVAL_MS           (55 + (n++ % 50))
#define RTC_CONNECTED_WAKEUP_INTERVAL_MS (55 + (n++ % 50))
#define SERVICE_INTERVAL_MS              1000
#else
#define ADV_INT_MIN_SLOTS                1000
#define ADV_INT_MAX_SLOTS                1000
#define CONN_INT_MIN_SLOTS               800
#define CONN_INT_MAX_SLOTS               800
#define RTC_WAKEUP_INTERVAL_MS           5000
#define RTC_CONNECTED_WAKEUP_INTERVAL_MS 2150
#define SERVICE_INTERVAL_MS              RTC_CONNECTED_WAKEUP_INTERVAL_MS
#endif

static uint8_t hello_arr[] = "HelloHello";
static uint8_t hello_arr_index __attribute__((noinit));

#define BT_CONN_STATE_CONNECTED    0x00
#define BT_CONN_STATE_DISCONNECTED 0x01
/* Service Definitions */
#define ATT_128_PRIMARY_SERVICE    ATT_16_TO_128_ARRAY(GATT_DECL_PRIMARY_SERVICE)
#define ATT_128_INCLUDED_SERVICE   ATT_16_TO_128_ARRAY(GATT_DECL_INCLUDE)
#define ATT_128_CHARACTERISTIC     ATT_16_TO_128_ARRAY(GATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG    ATT_16_TO_128_ARRAY(GATT_DESC_CLIENT_CHAR_CFG)
/* HELLO SERVICE and attribute 128 bit UUIDs */
#define HELLO_UUID_128_SVC                                                                         \
	{0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34,                                           \
	 0x45, 0x56, 0x67, 0x78, 0x89, 0x90, 0x00, 0x00}
#define HELLO_UUID_128_CHAR0                                                                       \
	{0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34,                                           \
	 0x45, 0x56, 0x67, 0x78, 0x89, 0x15, 0x00, 0x00}
#define HELLO_UUID_128_CHAR1                                                                       \
	{0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x23, 0x34,                                           \
	 0x45, 0x56, 0x67, 0x78, 0x89, 0x16, 0x00, 0x00}
#define HELLO_METAINFO_CHAR0_NTF_SEND 0x4321
#define ATT_16_TO_128_ARRAY(uuid)                                                                  \
	{(uuid) & 0xFF, (uuid >> 8) & 0xFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

enum pm_state_mode_type {
	PM_STATE_MODE_IDLE,
	PM_STATE_MODE_STANDBY,
	PM_STATE_MODE_STOP
};

/* List of attributes in the service */
enum service_att_list {
	HELLO_IDX_SERVICE = 0,
	/* First characteristic is readable + supports notifications */
	HELLO_IDX_CHAR0_CHAR,
	HELLO_IDX_CHAR0_VAL,
	HELLO_IDX_CHAR0_NTF_CFG,
	/* Second characteristic is writable */
	HELLO_IDX_CHAR1_CHAR,
	HELLO_IDX_CHAR1_VAL,
	/* Number of items*/
	HELLO_IDX_NB,
};

static volatile uint8_t conn_status __attribute__((noinit));
/* Store advertising activity index for re-starting after disconnection */
static volatile uint8_t conn_idx __attribute__((noinit));
static uint8_t adv_actv_idx __attribute__((noinit));
static struct service_env env __attribute__((noinit));

static volatile bool wakeup_status;
static volatile int run_profile_error;
static uint32_t served_intervals_ms;

static const char device_name[] = CONFIG_BLE_DEVICE_NAME;

/* Service UUID to pass into gatt_db_svc_add */
static const uint8_t hello_service_uuid[] = HELLO_UUID_128_SVC;

/* GATT database for the service */
static const gatt_att_desc_t hello_att_db[HELLO_IDX_NB] = {
	[HELLO_IDX_SERVICE] = {ATT_128_PRIMARY_SERVICE, ATT_UUID(16) | PROP(RD), 0},

	[HELLO_IDX_CHAR0_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[HELLO_IDX_CHAR0_VAL] = {HELLO_UUID_128_CHAR0, ATT_UUID(128) | PROP(RD) | PROP(N),
				 OPT(NO_OFFSET)},
	[HELLO_IDX_CHAR0_NTF_CFG] = {ATT_128_CLIENT_CHAR_CFG, ATT_UUID(16) | PROP(RD) | PROP(WR),
				     0},

	[HELLO_IDX_CHAR1_CHAR] = {ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0},
	[HELLO_IDX_CHAR1_VAL] = {HELLO_UUID_128_CHAR1, ATT_UUID(128) | PROP(WR),
				 OPT(NO_OFFSET) | sizeof(uint16_t)},
};

K_SEM_DEFINE(init_sem, 0, 1);
K_SEM_DEFINE(conn_sem, 0, 1);

/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_DISABLE,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	.private_identity.addr = {0xCF, 0xFE, 0xFB, 0xDE, 0x11, 0x07},
	.irk.key = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	.gap_start_hdl = 0,
	.gatt_start_hdl = 0,
	.att_cfg = 0,
	.sugg_max_tx_octets = GAP_LE_MAX_OCTETS,
	.sugg_max_tx_time = GAP_LE_MAX_TIME,
	.tx_pref_phy = GAP_PHY_ANY,
	.rx_pref_phy = GAP_PHY_ANY,
	.tx_path_comp = 0,
	.rx_path_comp = 0,
	.class_of_device = 0,  /* BT Classic only */
	.dflt_link_policy = 0, /* BT Classic only */
};

/* Environment for the service */
struct service_env {
	uint16_t start_hdl;
	uint8_t user_lid;
	uint8_t char0_val[250];
	uint8_t char1_val;
	volatile bool ntf_ongoing;
	volatile uint16_t ntf_cfg;
};

const gapc_le_con_param_nego_with_ce_len_t preferred_connection_param = {
	.ce_len_min = 5,
	.ce_len_max = 10,
	.hdr.interval_min = CONN_INT_MIN_SLOTS,
	.hdr.interval_max = CONN_INT_MAX_SLOTS,
	.hdr.latency = 0,
	.hdr.sup_to = 800};

/* Macros */
LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

/* function headers */
static uint16_t service_init(void);

/* Functions */
static uint16_t start_le_adv(uint8_t actv_idx)
{
	uint16_t err;

	gapm_le_adv_param_t adv_params = {
		.duration = 0, /* Advertise indefinitely */
	};

	err = gapm_le_start_adv(actv_idx, &adv_params);
	if (err) {
		LOG_ERR("Failed to start LE advertising with error %u", err);
	}
	return err;
}

/**
 * Bluetooth GAPM callbacks
 */
void on_gapc_proc_cmp_cb(uint8_t conidx, uint32_t metainfo, uint16_t status)
{
	LOG_INF("%s conn:%d status:%d\n", __func__, conidx, status);
}

static void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
				 const gap_bdaddr_t *p_peer_addr,
				 const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy)
{
	LOG_DBG("Connection request on index %u", conidx);
	gapc_le_connection_cfm(conidx, 0, NULL);

	LOG_INF("Connection parameters: interval %u, latency %u, supervision timeout %u",
		p_con_params->interval, p_con_params->latency, p_con_params->sup_to);

	LOG_DBG("Peer BD address %02X:%02X:%02X:%02X:%02X:%02X (conidx: %u)", p_peer_addr->addr[5],
		p_peer_addr->addr[4], p_peer_addr->addr[3], p_peer_addr->addr[2],
		p_peer_addr->addr[1], p_peer_addr->addr[0], conidx);

	conn_status = BT_CONN_STATE_CONNECTED;
	conn_idx = conidx;
	LOG_DBG("BLE Connected conn:%d", conidx);

	k_sem_give(&conn_sem);

	LOG_INF("Please enable notifications on peer device..");
}

static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
	LOG_WRN("Unexpected key received key on conidx %u", conidx);
}

static void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason)
{
	uint16_t err;

	LOG_DBG("Connection index %u disconnected for reason %u", conidx, reason);

	err = start_le_adv(adv_actv_idx);
	if (err) {
		LOG_ERR("Error restarting advertising: %u", err);
	} else {
		LOG_DBG("Restarting advertising");
	}

	conn_status = BT_CONN_STATE_DISCONNECTED;
	conn_idx = GAP_INVALID_CONIDX;
	LOG_INF("BLE disconnected conn:%d. Waiting new connection", conidx);
}

static void on_name_get(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t offset,
			uint16_t max_len)
{
	const size_t device_name_len = sizeof(device_name) - 1;
	const size_t short_len = (device_name_len > max_len ? max_len : device_name_len);

	LOG_DBG("%s", __func__);

	gapc_le_get_name_cfm(conidx, token, GAP_ERR_NO_ERROR, device_name_len, short_len,
			     (const uint8_t *)device_name);
}

static void on_appearance_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{
	/* Send 'unknown' appearance */
	LOG_DBG("%s", __func__);
	gapc_le_get_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR, 0);
}

static void on_pref_param_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{

	gapc_le_preferred_periph_param_t prefs = {
		.con_intv_min = preferred_connection_param.hdr.interval_min,
		.con_intv_max = preferred_connection_param.hdr.interval_max,
		.latency = preferred_connection_param.hdr.latency,
		.conn_timeout = 3200 * 2,
	};
	LOG_DBG("%s", __func__);

	gapc_le_get_preferred_periph_params_cfm(conidx, token, GAP_ERR_NO_ERROR, prefs);
}

void on_bond_data_updated(uint8_t conidx, uint32_t metainfo, const gapc_bond_data_updated_t *p_data)
{
	LOG_DBG("%s", __func__);
}
void on_auth_payload_timeout(uint8_t conidx, uint32_t metainfo)
{
	LOG_DBG("%s", __func__);
}
void on_no_more_att_bearer(uint8_t conidx, uint32_t metainfo)
{
	LOG_DBG("%s", __func__);
}
void on_cli_hash_info(uint8_t conidx, uint32_t metainfo, uint16_t handle, const uint8_t *p_hash)
{
	LOG_DBG("%s", __func__);
}
void on_name_set(uint8_t conidx, uint32_t metainfo, uint16_t token, co_buf_t *p_buf)
{
	LOG_DBG("%s", __func__);
	gapc_le_set_name_cfm(conidx, token, GAP_ERR_NO_ERROR);
}
void on_appearance_set(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t appearance)
{
	LOG_DBG("%s", __func__);
	gapc_le_set_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR);
}

static const gapc_connection_req_cb_t gapc_con_cbs = {
	.le_connection_req = on_le_connection_req,
};

static const gapc_security_cb_t gapc_sec_cbs = {
	.key_received = on_key_received,
	/* All other callbacks in this struct are optional */
};

static const gapc_connection_info_cb_t gapc_con_inf_cbs = {
	.disconnected = on_disconnection,
	.name_get = on_name_get,
	.appearance_get = on_appearance_get,
	.slave_pref_param_get = on_pref_param_get,
	/* Other callbacks in this struct are optional */
	.bond_data_updated = on_bond_data_updated,
	.auth_payload_timeout = on_auth_payload_timeout,
	.no_more_att_bearer = on_no_more_att_bearer,
	.cli_hash_info = on_cli_hash_info,
	.name_set = on_name_set,
	.appearance_set = on_appearance_set,
};

void on_param_update_req(uint8_t conidx, uint32_t metainfo, const gapc_le_con_param_nego_t *p_param)
{
	LOG_DBG("%s:%d", __func__, conidx);
	gapc_le_update_params_cfm(conidx, true, preferred_connection_param.ce_len_min,
				  preferred_connection_param.ce_len_max);
}
void on_param_updated(uint8_t conidx, uint32_t metainfo, const gapc_le_con_param_t *p_param)
{
	LOG_DBG("%s conn:%d", __func__, conidx);
}
void on_packet_size_updated(uint8_t conidx, uint32_t metainfo, uint16_t max_tx_octets,
			    uint16_t max_tx_time, uint16_t max_rx_octets, uint16_t max_rx_time)
{
	LOG_DBG("%s conn:%d max_tx_octets:%d max_tx_time:%d  max_rx_octets:%d "
		"max_rx_time:%d",
		__func__, conidx, max_tx_octets, max_tx_time, max_rx_octets, max_rx_time);

	/* PeHo: Seppo why this is done here? */
	const uint16_t ret = gapc_le_update_params(conidx, 0, &preferred_connection_param,
					     on_gapc_proc_cmp_cb);

	LOG_INF("Update connection %u ret:%d\n", conidx, ret);
}

void on_phy_updated(uint8_t conidx, uint32_t metainfo, uint8_t tx_phy, uint8_t rx_phy)
{
	LOG_DBG("%s conn:%d tx_phy:%d rx_phy:%d", __func__, conidx, tx_phy, rx_phy);
}
void on_subrate_updated(uint8_t conidx, uint32_t metainfo,
			const gapc_le_subrate_t *p_subrate_params)
{
	LOG_DBG("%s conn:%d", __func__, conidx);
}
/* All callbacks in this struct are optional */
static const gapc_le_config_cb_t gapc_le_cfg_cbs = {
	.param_update_req = on_param_update_req,
	.param_updated = on_param_updated,
	.packet_size_updated = on_packet_size_updated,
	.phy_updated = on_phy_updated,
	.subrate_updated = on_subrate_updated,
};

static void on_gapm_err(uint32_t metainfo, uint8_t code)
{
	LOG_ERR("gapm error %d", code);
}
static const gapm_cb_t gapm_err_cbs = {
	.cb_hw_error = on_gapm_err,
};

static const gapm_callbacks_t gapm_cbs = {
	.p_con_req_cbs = &gapc_con_cbs,
	.p_sec_cbs = &gapc_sec_cbs,
	.p_info_cbs = &gapc_con_inf_cbs,
	.p_le_config_cbs = &gapc_le_cfg_cbs,
	.p_bt_config_cbs = NULL, /* BT classic so not required */
	.p_gapm_cbs = &gapm_err_cbs,
};

static uint16_t set_advertising_data(uint8_t actv_idx)
{
	uint16_t err;

	/* Create advertising data with necessary services */
	const uint16_t adv_len = 0;

	co_buf_t *p_buf;

	err = co_buf_alloc(&p_buf, 0, adv_len, 0);
	if (err) {
		LOG_ERR("Buffer allocation failed");
		return err;
	}

	err = gapm_le_set_adv_data(actv_idx, p_buf);
	co_buf_release(p_buf);
	if (err) {
		LOG_ERR("Failed to set advertising data with error %u", err);
	}

	return err;
}

static uint16_t set_scan_data(uint8_t actv_idx)
{
	co_buf_t *p_buf;

	/* gatt service identifier */
	uint16_t svc[8] = {0xd123, 0xeabc, 0x785f, 0x1523, 0xefde, 0x1212, 0x1523, 0x0000};
	const size_t device_name_len = sizeof(device_name) - 1;
	const uint16_t adv_device_name = GATT_HANDLE_LEN + device_name_len;
	const uint16_t adv_uuid_svc = GATT_HANDLE_LEN + GATT_UUID_128_LEN;
	const uint16_t adv_len = adv_uuid_svc + adv_device_name;
	uint16_t err = co_buf_alloc(&p_buf, 0, adv_len, 0);

	uint8_t *p_data = co_buf_data(p_buf);

	/* Device name data */
	p_data[0] = device_name_len + 1;
	p_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
	memcpy(p_data + 2, device_name, device_name_len);

	/* Update data pointer */
	p_data = p_data + adv_device_name;

	/* Service UUID data */
	p_data[0] = GATT_UUID_128_LEN + 1;
	p_data[1] = GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID;
	memcpy(p_data + 2, &svc, sizeof(svc));

	__ASSERT(err == 0, "Buffer allocation failed");
	if (err) {
		LOG_ERR("Scan data buffer allocation failed = %d", err);
	}

	err = gapm_le_set_scan_response_data(actv_idx, p_buf);
	/* Release ownership of buffer so stack can free it when done */
	co_buf_release(p_buf);

	if (err) {
		LOG_ERR("Failed to set scan data with error %u\n", err);
	}

	return err;
}

/**
 * Advertising callbacks
 */
static void on_adv_actv_stopped(uint32_t metainfo, uint8_t actv_idx, uint16_t reason)
{
	LOG_DBG("Advertising activity index %u stopped for reason %u", actv_idx, reason);
}

static void on_adv_actv_proc_cmp(uint32_t metainfo, uint8_t proc_id, uint8_t actv_idx,
				 uint16_t status)
{
	if (status) {
		LOG_ERR("Advertising activity process completed with error %u", status);
		return;
	}

	switch (proc_id) {
	case GAPM_ACTV_CREATE_LE_ADV:
		LOG_DBG("Advertising activity is created");
		adv_actv_idx = actv_idx;
		set_advertising_data(actv_idx);
		break;

	case GAPM_ACTV_SET_ADV_DATA:
		LOG_DBG("Advertising data is set");
		set_scan_data(actv_idx);
		break;

	case GAPM_ACTV_SET_SCAN_RSP_DATA:
		LOG_DBG("Scan data is set");
		start_le_adv(actv_idx);
		break;

	case GAPM_ACTV_START:
		LOG_DBG("Advertising was started");
		k_sem_give(&init_sem);
		break;

	default:
		LOG_WRN("Unexpected GAPM activity complete, proc_id %u", proc_id);
		break;
	}
}

static void on_adv_created(uint32_t metainfo, uint8_t actv_idx, int8_t tx_pwr)
{
	LOG_DBG("Advertising activity created, index %u, selected tx power %d", actv_idx, tx_pwr);
}

static const gapm_le_adv_cb_actv_t le_adv_cbs = {
	.hdr.actv.stopped = on_adv_actv_stopped,
	.hdr.actv.proc_cmp = on_adv_actv_proc_cmp,
	.created = on_adv_created,
};

static uint16_t create_advertising(void)
{
	uint16_t err;

	gapm_le_adv_create_param_t adv_create_params = {
		.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
		.disc_mode = GAPM_ADV_MODE_GEN_DISC,
		.tx_pwr = 0,
		.filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
		.prim_cfg = {
			.adv_intv_min = ADV_INT_MIN_SLOTS,
			.adv_intv_max = ADV_INT_MAX_SLOTS,
			.ch_map = ADV_ALL_CHNLS_EN,
			.phy = GAPM_PHY_TYPE_LE_1M,
		},
	};

	err = gapm_le_create_adv_legacy(0, GAPM_STATIC_ADDR, &adv_create_params, &le_adv_cbs);
	if (err) {
		LOG_ERR("Error %u creating advertising activity", err);
	}

	return err;
}

/* Add service to the stack */
static void server_configure(void)
{
	uint16_t err;

	err = service_init();

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}
}

void on_gapm_process_complete(uint32_t metainfo, uint16_t status)
{
	if (status) {
		LOG_ERR("gapm process completed with error %u", status);
		return;
	}

	server_configure();

	LOG_DBG("gapm process completed successfully");

	create_advertising();
}

/* Service callbacks */
static void on_att_read_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
			    uint16_t offset, uint16_t max_length)
{
	co_buf_t *p_buf = NULL;
	uint16_t status = GAP_ERR_NO_ERROR;
	uint16_t att_val_len = 0;
	void *att_val = NULL;

	do {
		if (offset != 0) {
			/* Long read not supported for any characteristics within this service */
			status = ATT_ERR_INVALID_OFFSET;
			break;
		}

		uint8_t att_idx = hdl - env.start_hdl;

		switch (att_idx) {
		case HELLO_IDX_CHAR0_VAL:
			att_val_len = CONFIG_DATA_STRING_LENGTH;
			uint8_t loop_count = (CONFIG_DATA_STRING_LENGTH / 5);

			if (CONFIG_DATA_STRING_LENGTH % 5) {
				loop_count += 1;
			}
			for (int i = 0; i < loop_count; i++) {
				memcpy(env.char0_val + i * 5, &hello_arr[hello_arr_index], 5);
			}
			att_val = env.char0_val;
			LOG_DBG("read hello text");
			break;

		case HELLO_IDX_CHAR0_NTF_CFG:
			att_val_len = sizeof(env.ntf_cfg);
			att_val = (void *)&env.ntf_cfg;
			break;

		default:
			break;
		}

		if (att_val == NULL) {
			status = ATT_ERR_REQUEST_NOT_SUPPORTED;
			break;
		}

		status = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, att_val_len,
				      GATT_BUFFER_TAIL_LEN);
		if (status != CO_BUF_ERR_NO_ERROR) {
			status = ATT_ERR_INSUFF_RESOURCE;
			break;
		}

		memcpy(co_buf_data(p_buf), att_val, att_val_len);
	} while (0);

	/* Send the GATT response */
	gatt_srv_att_read_get_cfm(conidx, user_lid, token, status, att_val_len, p_buf);
	if (p_buf != NULL) {
		co_buf_release(p_buf);
	}
}

static void on_att_val_set(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
			   uint16_t offset, co_buf_t *p_data)
{
	uint16_t status = GAP_ERR_NO_ERROR;

	do {
		if (offset != 0) {
			/* Long write not supported for any characteristics in this service */
			status = ATT_ERR_INVALID_OFFSET;
			break;
		}

		uint8_t att_idx = hdl - env.start_hdl;

		switch (att_idx) {
		case HELLO_IDX_CHAR1_VAL: {
			if (sizeof(env.char1_val) != co_buf_data_len(p_data)) {
				LOG_DBG("Incorrect buffer size");
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
			} else {
				memcpy(&env.char1_val, co_buf_data(p_data), sizeof(env.char1_val));
				LOG_DBG("TOGGLE LED, state %d", env.char1_val);
			}
			break;
		}

		case HELLO_IDX_CHAR0_NTF_CFG: {
			if (sizeof(uint16_t) != co_buf_data_len(p_data)) {
				LOG_DBG("Incorrect buffer size");
				status = ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
			} else {
				uint16_t cfg;

				memcpy(&cfg, co_buf_data(p_data), sizeof(uint16_t));
				if (PRF_CLI_START_NTF == cfg || PRF_CLI_STOP_NTFIND == cfg) {
					env.ntf_cfg = cfg;
				} else {
					/* Indications not supported */
					status = ATT_ERR_REQUEST_NOT_SUPPORTED;
				}
			}
			break;
		}

		default:
			status = ATT_ERR_REQUEST_NOT_SUPPORTED;
			break;
		}
	} while (0);

	/* Send the GATT write confirmation */
	gatt_srv_att_val_set_cfm(conidx, user_lid, token, status);
}

static void on_event_sent(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t status)
{
	if (metainfo == HELLO_METAINFO_CHAR0_NTF_SEND) {
		env.ntf_ongoing = false;
	}
}

static const gatt_srv_cb_t gatt_cbs = {
	.cb_att_event_get = NULL,
	.cb_att_info_get = NULL,
	.cb_att_read_get = on_att_read_get,
	.cb_att_val_set = on_att_val_set,
	.cb_event_sent = on_event_sent,
};

/*
 * Service functions
 */
static uint16_t service_init(void)
{
	uint16_t status;

	/* Register a GATT user */
	status = gatt_user_srv_register(CFG_MAX_LE_MTU, 0, &gatt_cbs, &env.user_lid);
	if (status != GAP_ERR_NO_ERROR) {
		return status;
	}

	/* Add the GATT service */
	status = gatt_db_svc_add(env.user_lid, SVC_UUID(128), hello_service_uuid, HELLO_IDX_NB,
				 NULL, hello_att_db, HELLO_IDX_NB, &env.start_hdl);
	if (status != GAP_ERR_NO_ERROR) {
		gatt_user_unregister(env.user_lid);
		return status;
	}

	return GAP_ERR_NO_ERROR;
}

static uint16_t service_notification_send(uint32_t conidx_mask)
{
	co_buf_t *p_buf;
	uint16_t status;
	uint8_t conidx = 0;

	ARG_UNUSED(conidx_mask);

	/* Cannot send another notification unless previous one is completed */
	if (env.ntf_ongoing) {
		return PRF_ERR_REQ_DISALLOWED;
	}

	/* Check notification subscription */
	if (env.ntf_cfg != PRF_CLI_START_NTF) {
		return PRF_ERR_NTF_DISABLED;
	}

	/* Get a buffer to put the notification data into */
	status = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, CONFIG_DATA_STRING_LENGTH,
			      GATT_BUFFER_TAIL_LEN);
	if (status != CO_BUF_ERR_NO_ERROR) {
		return GAP_ERR_INSUFF_RESOURCES;
	}

	uint8_t const loop_count = ((CONFIG_DATA_STRING_LENGTH + 4) / 5);

	for (int i = 0; i < loop_count; i++) {
		memcpy(env.char0_val + i * 5, &hello_arr[hello_arr_index], 5);
	}

	memcpy(co_buf_data(p_buf), env.char0_val, CONFIG_DATA_STRING_LENGTH);
	hello_arr_index++;
	if (hello_arr_index > 4) {
		hello_arr_index = 0;
	}

	status = gatt_srv_event_send(conidx, env.user_lid, HELLO_METAINFO_CHAR0_NTF_SEND,
				     GATT_NOTIFY, env.start_hdl + HELLO_IDX_CHAR0_VAL, p_buf);

	co_buf_release(p_buf);

	if (status == GAP_ERR_NO_ERROR) {
		env.ntf_ongoing = true;
	}

	return status;
}

static int set_off_profile(enum pm_state_mode_type const pm_mode)
{
	int ret;
	off_profile_t offp;

	/* Set default for stop mode with RTC wakeup support */
	offp.power_domains = PD_VBAT_AON_MASK;
/* If CONFIG_FLASH_BASE_ADDRESS is zero application run from itcm and no MRAM needed */
#if (CONFIG_FLASH_BASE_ADDRESS == 0)
	offp.memory_blocks = 0;
#else
	offp.memory_blocks = MRAM_MASK;
#endif
	offp.memory_blocks |= SERAM_MEMORY_BLOCKS_IN_USE;
	offp.memory_blocks |= APP_RET_MEM_BLOCKS;
	offp.dcdc_voltage = 775;

	switch (pm_mode) {
	case PM_STATE_MODE_IDLE:
	case PM_STATE_MODE_STANDBY:
		offp.power_domains |= PD_SSE700_AON_MASK;
		offp.ip_clock_gating = 0;
		offp.phy_pwr_gating = 0;
		offp.dcdc_mode = DCDC_MODE_PFM_AUTO;
		break;
	case PM_STATE_MODE_STOP:
		offp.ip_clock_gating = 0;
		offp.phy_pwr_gating = 0;
		offp.dcdc_mode = DCDC_MODE_OFF;
		break;
	}

	offp.aon_clk_src = CLK_SRC_LFXO;
	offp.stby_clk_src = CLK_SRC_HFRC;
	offp.stby_clk_freq = SCALED_FREQ_RC_STDBY_0_075_MHZ;
	offp.ewic_cfg = SE_OFFP_EWIC_CFG;
	offp.wakeup_events = SE_OFFP_WAKEUP_EVENTS;
	offp.vtor_address = SCB->VTOR;
	offp.vtor_address_ns = SCB->VTOR;

	ret = se_service_set_off_cfg(&offp);
	if (ret) {
		LOG_ERR("SE: set_off_cfg failed = %d", ret);
	}

	return ret;
}

/**
 * Set the RUN profile parameters for this application.
 */
static int app_set_run_params(void)
{
	run_profile_t runp;
	int ret;

	runp.power_domains = PD_VBAT_AON_MASK | PD_SYST_MASK | PD_SSE700_AON_MASK | PD_SESS_MASK;
	runp.dcdc_voltage = 775;
	runp.dcdc_mode = DCDC_MODE_PFM_FORCED;
	runp.aon_clk_src = CLK_SRC_LFXO;
	runp.run_clk_src = CLK_SRC_HFRC;
	runp.cpu_clk_freq = CLOCK_FREQUENCY_76_8_RC_MHZ;
	runp.phy_pwr_gating = 0;
	runp.ip_clock_gating = 0;
	runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
	runp.scaled_clk_freq = SCALED_FREQ_RC_ACTIVE_76_8_MHZ;

	runp.memory_blocks = MRAM_MASK;
	runp.memory_blocks |= SERAM_MEMORY_BLOCKS_IN_USE;
	runp.memory_blocks |= APP_RET_MEM_BLOCKS;

	if (IS_ENABLED(CONFIG_MIPI_DSI)) {
		runp.phy_pwr_gating |= MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK | MIPI_PLL_DPHY_MASK;
		runp.ip_clock_gating |= CDC200_MASK | MIPI_DSI_MASK | GPU_MASK;
	}

	ret = se_service_set_run_cfg(&runp);

	return ret;
}
/*
 * CRITICAL: Must run at PRE_KERNEL_1 to restore SYSTOP before peripherals initialize.
 *
 * On cold boot: SYSTOP is already ON by default, safe to call.
 * On SOFT_OFF wakeup: SYSTOP is OFF, must restore BEFORE peripherals access registers.
 */
SYS_INIT(app_set_run_params, PRE_KERNEL_1, 3);

static inline uint32_t get_wakeup_irq_status(void)
{
	return NVIC_GetPendingIRQ(WAKEUP_SOURCE_IRQ);
}

/**
 * PM Notifier callback for power state entry
 */
static void pm_notify_state_entry(enum pm_state const state)
{
	/* TODO: enable when this is needed */
	/*
	const struct pm_state_info *next_state = pm_state_next_get(0);
	uint8_t substate_id = next_state ? next_state->substate_id : 0;
	*/

	switch (state) {
	case PM_STATE_SUSPEND_TO_RAM:
	case PM_STATE_SOFT_OFF:
		break;
	default:
		__ASSERT(false, "Entering unknown power state %d", state);
		LOG_ERR("Entering unknown power state %d", state);
		break;
	}
}

/**
 * PM Notifier callback called BEFORE devices are resumed
 *
 * This restores SE run configuration when resuming from S2RAM states.
 * Note: For SOFT_OFF, the system resets completely and app_set_run_params()
 * runs during normal PRE_KERNEL_1 initialization, so this callback is not needed.
 */
static void pm_notify_pre_device_resume(enum pm_state const state)
{
	wakeup_status = get_wakeup_irq_status();

	switch (state) {
	case PM_STATE_SUSPEND_TO_RAM: {
		run_profile_error = app_set_run_params();
		break;
	}
	case PM_STATE_SOFT_OFF: {
		/* No action needed - SOFT_OFF causes reset, not resume */
		break;
	}
	default: {
		__ASSERT(false, "Pre-resume for unknown power state %d", state);
		LOG_ERR("Pre-resume for unknown power state %d", state);
		break;
	}
	}
}

/**
 * PM Notifier structure
 */
static struct pm_notifier app_pm_notifier = {
	.state_entry = pm_notify_state_entry,
	.pre_device_resume = pm_notify_pre_device_resume,
};

void app_ready_for_sleep(void)
{
	pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
}

/*
 * This function will be invoked in the PRE_KERNEL_1 phase of the init
 * routine to prevent sleep during startup.
 */
static int app_pre_kernel_init(void)
{
	/* Register PM notifier callbacks */
	pm_notifier_register(&app_pm_notifier);

	pm_policy_state_lock_get(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);

	return 0;
}
SYS_INIT(app_pre_kernel_init, PRE_KERNEL_1, 39);

#if defined(CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_HOOKS)

static uint32_t idle_timer_pre_idle;

/* Idle timer used for timer while entering the idle state */
static const struct device *idle_timer = DEVICE_DT_GET(DT_CHOSEN(zephyr_cortex_m_idle_timer));
/**
 * To simplify the driver, implement the callout to Counter API
 * as hooks that would be provided by platform drivers if
 * CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_HOOKS was selected instead.
 */
void z_cms_lptim_hook_on_lpm_entry(uint64_t max_lpm_time_us)
{

	/* Store current value of the selected timer to calculate a
	 * difference in measurements after exiting the idle state.
	 */
	counter_get_value(idle_timer, &idle_timer_pre_idle);
	/**
	 * Disable the counter alarm in case it was already running.
	 */
	/* counter_cancel_channel_alarm(idle_timer, 0); */

	/* Set the alarm using timer that runs the idle.
	 * Needed rump-up/setting time, lower accurency etc. should be
	 * included in the exit-latency in the power state definition.
	 */

	struct counter_alarm_cfg cfg = {
		.callback = NULL,
		.ticks = counter_us_to_ticks(idle_timer, max_lpm_time_us) + idle_timer_pre_idle,
		.user_data = NULL,
		.flags = COUNTER_ALARM_CFG_ABSOLUTE,
	};
	counter_set_channel_alarm(idle_timer, 0, &cfg);
}

uint64_t z_cms_lptim_hook_on_lpm_exit(void)
{
	/**
	 * Calculate how much time elapsed according to counter.
	 */
	uint32_t idle_timer_post, idle_timer_diff;

	counter_get_value(idle_timer, &idle_timer_post);

	/**
	 * Check for counter timer overflow
	 * (TODO: this doesn't work for downcounting timers!)
	 */
	if (idle_timer_pre_idle > idle_timer_post) {
		idle_timer_diff = (counter_get_top_value(idle_timer) - idle_timer_pre_idle) +
				  idle_timer_post + 1;
	} else {
		idle_timer_diff = idle_timer_post - idle_timer_pre_idle;
	}

	return (uint64_t)counter_ticks_to_us(idle_timer, idle_timer_diff);
}
#endif /* CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER */

int main(void)
{
	const struct device *const wakeup_dev = DEVICE_DT_GET(WAKEUP_SOURCE);
	uint16_t ble_status;
	int ret;

#if DT_NODE_EXISTS(DEBUG_PIN_NODE)
	if (!gpio_is_ready_dt(&debug_pin)) {
		LOG_ERR("Led not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&debug_pin, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Led config failed\n");
		return 0;
	}
#endif

	if (!device_is_ready(wakeup_dev)) {
		printk("%s: device not ready", wakeup_dev->name);
		return -1;
	}

	ret = counter_start(wakeup_dev);

	printk("BLE Sleep demo\n");

	ret = set_off_profile(PM_STATE_MODE_STOP);

	if (ret) {
		LOG_ERR("off profile set failed. error: %d", ret);
		return ret;
	}

	/* Start up bluetooth host stack. */
	ble_status = alif_ble_enable(NULL);

	if (ble_status == 0) {
		/* BLE initialized first time */
		hello_arr_index = 0;
		conn_idx = GAP_INVALID_CONIDX;
		memset(&env, 0, sizeof(struct service_env));
		conn_status = BT_CONN_STATE_DISCONNECTED;

		/* Generate random address */
		se_service_get_rnd_num(&gapm_cfg.private_identity.addr[3], 3);
		ble_status = gapm_configure(0, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);

		if (ble_status) {
			LOG_ERR("gapm_configure error %u", ble_status);
			return -1;
		}

		LOG_DBG("Waiting for initial BLE init...");
		k_sem_take(&init_sem, K_FOREVER);
		LOG_INF("Init complete!");
	}

	app_ready_for_sleep();

	while (1) {
#if DT_NODE_EXISTS(DEBUG_PIN_NODE)
		gpio_pin_configure_dt(&debug_pin, GPIO_OUTPUT_ACTIVE);
		gpio_pin_toggle_dt(&debug_pin);
#endif

		if (conn_status != BT_CONN_STATE_CONNECTED) {
			k_sleep(K_MSEC(RTC_WAKEUP_INTERVAL_MS));
			continue;
		}

		k_sleep(K_MSEC(RTC_CONNECTED_WAKEUP_INTERVAL_MS));

		/* TODO: better error handling will be needed here! */
		if (run_profile_error) {
			LOG_ERR("app_set_run_params failed. error: %d", run_profile_error);
			return run_profile_error;
		}

		if (wakeup_status) {
			served_intervals_ms += RTC_CONNECTED_WAKEUP_INTERVAL_MS;

			if (served_intervals_ms >= SERVICE_INTERVAL_MS) {
				if ((env.ntf_cfg == PRF_CLI_START_NTF) &&
					(!env.ntf_ongoing)) {
					/* Update text at RTC periods */
					service_notification_send(UINT32_MAX);
				}
				served_intervals_ms = 0;
			}
		}
	}
	return 0;
}
