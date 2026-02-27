/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/* BLE Bundle: Multi-service peripheral exposes the following services simultaneously:
 *   - Heart Rate (HRPS)
 *   - Blood Pressure (BLPS)
 *   - Health Thermometer (HTPT)
 *   - Glucose (GLPS)
 *   - Cycling Speed & Cadence (CSCPS)
 *   - Running Speed & Cadence (RSCPS)
 *   - Link Loss (LLSS via PRXP)
 *   - Battery (BASS)
 *   - Blinky LED control (custom 128-bit GATT service)
 * All measurements are dummy values sent periodically, gated by CCCD subscription.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include "alif_ble.h"
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "address_verification.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"
#include "ble_gpio.h"

#include "shared_control.h"
#include "batt_svc.h"

#include "prf.h"
#include "bass.h"

/* Heart Rate Profile Server */
#include "hrp_common.h"
#include "hrps.h"

/* Blood Pressure Profile Server */
#include "blps.h"
#include "blps_msg.h"
#include "prf_types.h"
#include "rwprf_config.h"

/* Health Thermometer Profile Server */
#include "htpt.h"
#include "htpt_msg.h"

/* Glucose Profile Server */
#include "glps.h"
#include "glps_msg.h"

/* Cycling Speed & Cadence Profile Server */
#include "cscp_common.h"
#include "cscps.h"
#include "cscps_msg.h"

/* Running Speed & Cadence Profile Server */
#include "rscp_common.h"
#include "rscps.h"
#include "rscps_msg.h"

/* Proximity Profile (Link Loss only - IASS and TPSS removed to stay within profile task limit) */
#include "prxp_app.h"

/* Custom Blinky GATT service */
#include "gatt_db.h"
#include "gatt_srv.h"
#include "ke_mem.h"

/* LED0: controlled by BLE (blinky service)
 * LED2: connection status indicator (blinks while advertising)
 */
#define LED0_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led2)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);


void LedWorkerHandler(struct k_work *work);

static K_WORK_DELAYABLE_DEFINE(ledWork, LedWorkerHandler);
/* HRPS feature flags */
enum hrps_feat_bf {
	HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS = 0,
	HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT = CO_BIT(HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS),
	HRPS_ENGY_EXP_FEAT_SUP_POS = 1,
	HRPS_ENGY_EXP_FEAT_SUP_BIT = CO_BIT(HRPS_ENGY_EXP_FEAT_SUP_POS),
	HRPS_HR_MEAS_NTF_CFG_POS = 2,
	HRPS_HR_MEAS_NTF_CFG_BIT = CO_BIT(HRPS_HR_MEAS_NTF_CFG_POS),
};

/* HTPT stable measurement indication config value */
#define HTPT_CFG_STABLE_MEAS_IND_DIS 0

/* Dummy temperature state: oscillates between 35C and 40C */
static uint32_t ht_temp_value = 35;
static int8_t ht_direction = 1;

/* Body sensor location value used in HRPS configuration */
#define BODY_SENSOR_LOCATION_CHEST 0x01

/* Advertising address type, resolved at startup by address_verification() */
static uint8_t adv_type;

/* Shared connection state (referenced by battery and sub-services) */
struct shared_control ctrl = { false, 0, 0 };

/* Dummy sensor value cycling between 70-130 (shared by HR and BP) */
static uint16_t current_value = 70;

/* Per-service send gates.
 * Each flag is set when CCCD notifications are enabled AND the previous send completed.
 */
static bool hr_ready_to_send;
static bool bp_ready_to_send;
static bool ht_ready_to_send;
static bool gl_ready_to_send;
static bool gl_sent_once;
static bool cs_ready_to_send;
static bool rsc_ready_to_send;

static void send_glucose_once(void);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* BLINKY - Custom 128-bit GATT Service
 * Allows a BLE central (phone app) to toggle LED0 on the board.
 *
 * Slots used:
 *   0: bt_srv_hello (system, automatic)
 *   1: config_battery_service()
 *   2: blinky_init()
 *   3: (free - prxp uses prf_add_profile, no slot consumed)
 */

/* Service UUID */
#define BLINKY_UUID_SVC                                                                            \
	{0x5b, 0xda, 0x7d, 0x73, 0x0b, 0x5f, 0x52, 0x91,                                           \
	 0x58, 0x43, 0xc7, 0x8f, 0x31, 0xac, 0xbc, 0xb9}

/* LED characteristic */
#define BLINKY_UUID_CHAR                                                                           \
	{0x5d, 0xda, 0x7d, 0x73, 0x0b, 0x5f, 0x52, 0x91,                                           \
	 0x58, 0x43, 0xc7, 0x8f, 0x31, 0xac, 0xbc, 0xb9}

static const uint8_t blinky_svc_uuid128[16] = BLINKY_UUID_SVC;
static const uint16_t lbs_service_uuid16[8] = {0xda5b, 0x737d, 0x5f0b, 0x9152,
					       0x4358, 0x8fc7, 0xac31, 0xb9bc};

/* Attribute indices into the blinky GATT database */
enum {
	BLINKY_IDX_SVC,        /* Primary service declaration */
	BLINKY_IDX_CHAR_DECL,  /* Characteristic declaration */
	BLINKY_IDX_CHAR_VAL,   /* Characteristic value (Read / Write / Notify) */
	BLINKY_IDX_CHAR_CCCD,  /* Client Characteristic Configuration Descriptor */
	BLINKY_IDX_NB,         /* Total attribute count */
};

/* Blinky runtime state */
struct blinky_env {
	uint16_t start_hdl;    /* First GATT handle, assigned during registration */
	uint8_t  user_lid;     /* GATT user local ID returned by gatt_user_srv_register */
	uint8_t  value;        /* Current LED state: 0 = off, 1 = on */
	uint16_t cccd;         /* Cached CCCD value (notification subscription state) */
	bool     ntf_ongoing;  /* True while a notification packet is in flight */
};

static struct blinky_env blinky;

/* GATT attribute table for the blinky service */
static const gatt_att_desc_t blinky_att_db[BLINKY_IDX_NB] = {
	[BLINKY_IDX_SVC] = { ATT_128_PRIMARY_SERVICE, ATT_UUID(16) | PROP(RD), 0 },
	[BLINKY_IDX_CHAR_DECL] = { ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0 },
	[BLINKY_IDX_CHAR_VAL] = { BLINKY_UUID_CHAR,
				   ATT_UUID(128) | PROP(RD) | PROP(WR) | PROP(N),
				   OPT(NO_OFFSET) | sizeof(uint8_t) },
	[BLINKY_IDX_CHAR_CCCD] = { ATT_128_CLIENT_CHAR_CFG,
				    ATT_UUID(16) | PROP(RD) | PROP(WR), 0 },
};

/* Bluetooth stack configuration */
static gapm_config_t gapm_cfg = {
	.role = GAP_ROLE_LE_PERIPHERAL,
	.pairing_mode = GAPM_PAIRING_DISABLE,
	.privacy_cfg = 0,
	.renew_dur = 1500,
	.private_identity.addr = {0},
	.irk.key = {0},
	.gap_start_hdl = 0,
	.gatt_start_hdl = 0,
	.att_cfg = 0,
	.sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
	.sugg_max_tx_time = GAP_LE_MIN_TIME,
	.tx_pref_phy = GAP_PHY_ANY,
	.rx_pref_phy = GAP_PHY_ANY,
	.tx_path_comp = 0,
	.rx_path_comp = 0,
	.class_of_device = 0,
	.dflt_link_policy = 0,
};

#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

/* Saved advertising activity index, used to restart advertising after disconnection */
static uint8_t adv_actv_idx;

/* CSCPS state */
static cscp_csc_meas_t cs_meas = {
	.flags = CSCP_MEAS_CRANK_REV_DATA_PRESENT_BIT,
};

static uint16_t cs_evt_time;

/* RSCPS state */
static uint32_t rsc_total_distance;
static uint16_t rsc_current_value = 1;

/* GAP connection callbacks */

static void on_disconnection(uint8_t conidx, uint16_t reason)
{
	/* Notify PRXP (triggers link loss alert if disconnection was not user-initiated) */
	prxp_disc_notify(reason);

	/* Turn off BLE-controlled LED on disconnect */
	ble_gpio_led_set(&led0, false);

	/* Reset all service send gates - phone must re-enable CCCDs after reconnect */
	hr_ready_to_send = false;
	bp_ready_to_send = false;
	ht_ready_to_send = false;
	gl_ready_to_send = false;
	gl_sent_once = false;
	cs_ready_to_send = false;
	rsc_ready_to_send = false;
}

/* HRPS callbacks */

static void on_hrps_meas_send_complete(uint16_t status)
{
	hr_ready_to_send = true; /* Previous send finished, allow next measurement */
}

static void on_hr_bond_data_upd(uint8_t conidx, uint16_t cfg_val)
{
	switch (cfg_val) {
	case PRF_CLI_STOP_NTFIND:
		LOG_INF("HR: Client requested stop notification/indication (conidx: %u)", conidx);
		hr_ready_to_send = false;
		break;

	case PRF_CLI_START_NTF:
	case PRF_CLI_START_IND:
		LOG_INF("HR: Client requested start notification/indication (conidx: %u)", conidx);
		hr_ready_to_send = true;
		break;

	default:
		break;
	}
}

static void on_hr_energy_exp_reset(uint8_t conidx)
{
	ARG_UNUSED(conidx);
}

/* BLPS callbacks */

static void on_blps_meas_send_complete(uint8_t conidx, uint16_t status)
{
	ARG_UNUSED(conidx);
	ARG_UNUSED(status);
	bp_ready_to_send = true;
}

static void on_blps_bond_data_upd(uint8_t conidx, uint8_t char_code, uint16_t cfg_val)
{
	ARG_UNUSED(char_code);

	switch (cfg_val) {
	case PRF_CLI_STOP_NTFIND:
		LOG_INF("BLPS: Client requested stop notification/indication (conidx: %u)", conidx);
		bp_ready_to_send = false;
		break;

	case PRF_CLI_START_NTF:
	case PRF_CLI_START_IND:
		LOG_INF("BLPS: Client requested start notification/indication (conidx: %u)",
			conidx);
		bp_ready_to_send = true;
		break;

	default:
		break;
	}
}

/* HTPT callbacks */

static void on_htpt_meas_send_complete(uint16_t status)
{
	ARG_UNUSED(status);
	ht_ready_to_send = true;
}

static void on_htpt_bond_data_upd(uint8_t conidx, uint8_t cfg_val)
{
	switch (cfg_val) {
	case HTPT_CFG_STABLE_MEAS_IND:
		LOG_INF("HTPT: Client enabled indications");
		ht_ready_to_send = true;
		break;

	case 0:
		LOG_INF("HTPT: Client disabled indications");
		ht_ready_to_send = false;
		break;

	default:
		LOG_INF("HTPT: Unsupported cfg change");
		break;
	}
}

static void on_htpt_meas_intv_chg(uint8_t conidx, uint16_t interval)
{
	ARG_UNUSED(conidx);
	ARG_UNUSED(interval);
}

static const htpt_cb_t htpt_cb = {
	.cb_bond_data_upd = on_htpt_bond_data_upd,
	.cb_temp_send_cmp = on_htpt_meas_send_complete,
	.cb_meas_intv_chg_req = on_htpt_meas_intv_chg,
};

/* GLPS callbacks */

static void on_glps_bond_data_upd(uint8_t conidx, uint8_t evt_cfg)
{
	ARG_UNUSED(conidx);

	if (evt_cfg == PRF_CLI_START_NTF) {
		gl_ready_to_send = true;
		gl_sent_once = false;
	} else {
		gl_ready_to_send = false;
	}
}

static void on_glps_meas_send_complete(uint8_t conidx, uint16_t status)
{
	ARG_UNUSED(conidx);
	ARG_UNUSED(status);
}

/* Called when the app sends a RACP request (e.g. user taps reload button).
 * Send the dummy measurement first, then report 1 record so the app shows a value.
 */
static void on_glps_racp_req(uint8_t conidx, uint8_t op_code,
			     uint8_t func_operator,
			     uint8_t filter_type,
			     const union glp_filter *p_filter)
{
	send_glucose_once();
	glps_racp_rsp_send(conidx, op_code, GLP_RSP_SUCCESS, 1);
}

static void on_glps_racp_rsp_send_cmp(uint8_t conidx, uint16_t status)
{
	ARG_UNUSED(conidx);
	ARG_UNUSED(status);
}

static const glps_cb_t glps_cb = {
	.cb_bond_data_upd = on_glps_bond_data_upd,
	.cb_meas_send_cmp = on_glps_meas_send_complete,
	.cb_racp_req = on_glps_racp_req,
	.cb_racp_rsp_send_cmp = on_glps_racp_rsp_send_cmp,
};

/* GLPS helpers */

/* Convert mg/dL float to the 16-bit SFLOAT format used in GLP measurements */
static prf_sfloat glucose_to_sfloat(float mg_dl)
{
	uint16_t mantissa = ((uint16_t)mg_dl) & 0x0FFF;
	uint16_t exponent = 0x0; /* simple */

	return (exponent << 12) | mantissa;
}

/* Send a single dummy glucose record (95 mg/dL, capillary whole blood, finger) */
static void send_glucose_once(void)
{
	glp_meas_t meas = {
		.flags = GLP_MEAS_GL_CTR_TYPE_AND_SPL_LOC_PRES_BIT,
		.concentration = glucose_to_sfloat(95.0f),
		.type = GLP_TYPE_CAPILLARY_WHOLE_BLOOD,
		.location = GLP_LOC_FINGER,
		.base_time = {
			.year = 2024,
			.month = 4,
			.day = 1,
			.hour = 12,
			.min = 0,
			.sec = 0,
		},
	};

	uint16_t err = glps_meas_send(0, 1, &meas, NULL);

	if (err) {
		LOG_ERR("GLPS send error %u", err);
	}
}

/* CSCPS callbacks */

static void on_csc_meas_send_complete(uint16_t status)
{
	ARG_UNUSED(status);
	cs_ready_to_send = true;
}

static void on_csc_bond_data_upd(uint8_t conidx, uint8_t char_code, uint16_t cfg_val)
{
	ARG_UNUSED(conidx);

	if (char_code == CSCP_CSCS_CSC_MEAS_CHAR) {
		switch (cfg_val) {
		case PRF_CLI_STOP_NTFIND:
			LOG_INF("CSCPS: Client requested stop notifications");
			cs_ready_to_send = false;
			break;

		case PRF_CLI_START_NTF:
		case PRF_CLI_START_IND:
			LOG_INF("CSCPS: Client requested start notifications");
			cs_ready_to_send = true;
			break;
		}
	}
}

static void on_csc_ctnl_pt_req(uint8_t conidx, uint8_t op_code,
				const union cscp_sc_ctnl_pt_req_val *p_value)
{
	ARG_UNUSED(conidx);
	ARG_UNUSED(op_code);
	ARG_UNUSED(p_value);
}

static void on_csc_ctnl_pt_rsp_send_cmp(uint8_t conidx, uint16_t status)
{
	ARG_UNUSED(conidx);
	ARG_UNUSED(status);
}

static const cscps_cb_t cscps_cb = {
	.cb_bond_data_upd = on_csc_bond_data_upd,
	.cb_meas_send_cmp = on_csc_meas_send_complete,
	.cb_ctnl_pt_req = on_csc_ctnl_pt_req,
	.cb_ctnl_pt_rsp_send_cmp = on_csc_ctnl_pt_rsp_send_cmp,
};

/* RSCPS callbacks */

static void on_rsc_meas_send_complete(uint16_t status)
{
	ARG_UNUSED(status);
	rsc_ready_to_send = true;
}

static void on_rsc_bond_data_upd(uint8_t conidx, uint8_t char_code, uint16_t cfg_val)
{
	ARG_UNUSED(conidx);

	switch (cfg_val) {
	case PRF_CLI_STOP_NTFIND:
		LOG_INF("RSCPS: Client requested stop notifications");
		rsc_ready_to_send = false;
		break;

	case PRF_CLI_START_NTF:
	case PRF_CLI_START_IND:
		LOG_INF("RSCPS: Client requested start notifications");
		rsc_ready_to_send = true;
		break;
	}
}

static void on_rsc_ctnl_pt_req(uint8_t conidx, uint8_t op_code,
				const union rscp_sc_ctnl_pt_req_val *p_value)
{
	ARG_UNUSED(conidx);
	ARG_UNUSED(op_code);
	ARG_UNUSED(p_value);
}

static void on_rsc_ctnl_pt_rsp_send_cmp(uint8_t conidx, uint16_t status)
{
	ARG_UNUSED(conidx);
	ARG_UNUSED(status);
}

static const rscps_cb_t rscps_cb = {
	.cb_bond_data_upd = on_rsc_bond_data_upd,
	.cb_meas_send_cmp = on_rsc_meas_send_complete,
	.cb_ctnl_pt_req = on_rsc_ctnl_pt_req,
	.cb_ctnl_pt_rsp_send_cmp = on_rsc_ctnl_pt_rsp_send_cmp,
};

/* Profile callback structs */

static const hrps_cb_t hrps_cb = {
	.cb_bond_data_upd = on_hr_bond_data_upd,
	.cb_meas_send_cmp = on_hrps_meas_send_complete,
	.cb_energy_exp_reset = on_hr_energy_exp_reset,
};

static const blps_cb_t blps_cb = {
	.cb_bond_data_upd = on_blps_bond_data_upd,
	.cb_meas_send_cmp = on_blps_meas_send_complete,
};

/* Build the primary advertising packet with all standard 16-bit service UUIDs */
static uint16_t set_advertising_data(uint8_t actv_idx)
{
	int ret;
	uint16_t comp_id = CONFIG_BLE_COMPANY_ID;
	uint16_t svc[] = {
		GATT_SVC_HEART_RATE,
		GATT_SVC_BLOOD_PRESSURE,
		GATT_SVC_HEALTH_THERMOM,
		GATT_SVC_GLUCOSE,
		GATT_SVC_CYCLING_SPEED_CADENCE,
		GATT_SVC_RUNNING_SPEED_CADENCE,
		GATT_SVC_LINK_LOSS,
		get_batt_id(),
	};

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID, svc, sizeof(svc));
	if (ret) {
		LOG_ERR("AD profile set fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	/* This sample write AD Name, Profile list and Company ID */
	ret = bt_adv_data_set_manufacturer(comp_id, NULL, 0);

	if (ret) {
		LOG_ERR("AD manufacturer data fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	return bt_gapm_advertiment_data_set(actv_idx);
}

/* Blinky GATT callbacks */

static void blinky_att_read(uint8_t conidx, uint8_t user_lid, uint16_t token,
			    uint16_t hdl, uint16_t offset, uint16_t max_len)
{
	co_buf_t *buf;

	co_buf_alloc(&buf, GATT_BUFFER_HEADER_LEN, 1, GATT_BUFFER_TAIL_LEN);
	if (!buf) {
		/* Allocation failed: report insufficient resources and return */
		gatt_srv_att_read_get_cfm(conidx, user_lid, token,
					GAP_ERR_INSUFF_RESOURCES, 0, NULL);
		return;
	}
	*co_buf_data(buf) = blinky.value;
	gatt_srv_att_read_get_cfm(conidx, user_lid, token, GAP_ERR_NO_ERROR, 1, buf);
	co_buf_release(buf);
}

/* Handles writes to both the LED characteristic value and the CCCD.
 * att_idx is derived from the handle offset so we can dispatch correctly.
 */
static void blinky_att_write(uint8_t conidx, uint8_t user_lid, uint16_t token,
			     uint16_t hdl, uint16_t offset, co_buf_t *data)
{
	uint16_t att_idx = hdl - blinky.start_hdl;

	if (att_idx == BLINKY_IDX_CHAR_VAL) {
		/* Reject writes with wrong length before touching the GPIO */
		if (co_buf_data_len(data) != sizeof(uint8_t)) {
			gatt_srv_att_val_set_cfm(conidx, user_lid, token,
						 ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN);
			return;
		}
		/* Hand LED control to the phone */
		blinky.value = *co_buf_data(data);
		ble_gpio_led_set(&led0, blinky.value ? 1 : 0);

	} else if (att_idx == BLINKY_IDX_CHAR_CCCD) {
		memcpy(&blinky.cccd, co_buf_data(data), sizeof(uint16_t));
	}

	gatt_srv_att_val_set_cfm(conidx, user_lid, token, GAP_ERR_NO_ERROR);
}

static void blinky_evt_sent(uint8_t conidx, uint8_t user_lid,
			    uint16_t metainfo, uint16_t status)
{
	blinky.ntf_ongoing = false;
}

static const gatt_srv_cb_t blinky_gatt_cbs = {
	.cb_att_read_get = blinky_att_read,
	.cb_att_val_set  = blinky_att_write,
	.cb_event_sent   = blinky_evt_sent,
};

/* Build the scan response packet with the device name and the blinky 128-bit UUID.
 * Splitting across adv + scan response keeps both within the 31-byte BLE limit.
 */
static uint16_t set_scan_data(uint8_t actv_idx)
{
	int ret;

	ret = bt_scan_rsp_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID, lbs_service_uuid16,
				  sizeof(lbs_service_uuid16));
	if (ret) {
		LOG_ERR("Scan response UUID set fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_scan_rsp_data_set_name_auto(DEVICE_NAME, strlen(DEVICE_NAME));

	if (ret) {
		LOG_ERR("Scan response device name data fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	return bt_gapm_scan_response_set(actv_idx);
}

/* Advertising activity creation */
static uint16_t create_advertising(void)
{
	gapm_le_adv_create_param_t adv_create_params = {
		.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
		.disc_mode = GAPM_ADV_MODE_GEN_DISC,
		.tx_pwr = 0,
		.filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
		.prim_cfg = {
				.adv_intv_min = 160,
				.adv_intv_max = 800,
				.ch_map = ADV_ALL_CHNLS_EN,
				.phy = GAPM_PHY_TYPE_LE_1M,
			},
	};

	return bt_gapm_le_create_advertisement_service(adv_type, &adv_create_params, NULL,
						       &adv_actv_idx);
}

/* Blinky service registration */
static uint16_t blinky_init(void)
{
	uint16_t err;

	err = gatt_user_srv_register(L2CAP_LE_MTU_MIN, 0,
				     &blinky_gatt_cbs,
				     &blinky.user_lid);
	if (err) {
		return err;
	}

	err = gatt_db_svc_add(blinky.user_lid, SVC_UUID(128),
			      blinky_svc_uuid128,
			      BLINKY_IDX_NB,
			      NULL,
			      blinky_att_db,
			      BLINKY_IDX_NB,
			      &blinky.start_hdl);

	return err;
}

/* Standard BLE profile registration.
 * All profiles here use prf_add_profile() which shares the
 * profile framework's GATT user and does NOT consume a ROM GATT user slot.
 */
static void bundle_server_configure(void)
{
	uint16_t err;

	/* Heart Rate Profile Server */
	uint16_t start_hdl_hr = 0;
	struct hrps_db_cfg hrps_cfg = {0};

	hrps_cfg.features = HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT | HRPS_HR_MEAS_NTF_CFG_BIT;
	hrps_cfg.body_sensor_loc = BODY_SENSOR_LOCATION_CHEST;

	err = prf_add_profile(TASK_ID_HRPS, 0, 0, &hrps_cfg, &hrps_cb, &start_hdl_hr);
	if (err) {
		LOG_ERR("Error %u adding HRPS profile", err);
	}

	/* Blood Pressure Profile Server */
	uint16_t start_hdl_bp = 0;
	struct blps_db_cfg blps_cfg = {0};

	blps_cfg.features = 0;
	blps_cfg.prfl_cfg = 0;

	err = prf_add_profile(TASK_ID_BLPS, 0, 0, &blps_cfg, &blps_cb, &start_hdl_bp);
	if (err) {
		LOG_ERR("Error %u adding BLPS profile", err);
	}

	/* Health Thermometer Profile Server */
	uint16_t start_hdl_ht = 0;
	struct htpt_db_cfg htpt_cfg = {
		.features = HTPT_TEMP_TYPE_CHAR_SUP_BIT,
		.temp_type = HTP_TYPE_BODY,
	};

	err = prf_add_profile(TASK_ID_HTPT, 0, 0, &htpt_cfg, &htpt_cb, &start_hdl_ht);
	if (err) {
		LOG_ERR("Error %u adding HTPT profile", err);
	}

	/* Glucose Profile Server
	 * meas_ctx_supported = 0: measurement context characteristic not included
	 */
	uint16_t start_hdl_gl = 0;
	struct glps_db_cfg glps_cfg = {
		.meas_ctx_supported = 0,
	};

	err = prf_add_profile(TASK_ID_GLPS, 0, 0, &glps_cfg, &glps_cb, &start_hdl_gl);
	if (err) {
		LOG_ERR("Error %u adding GLPS profile", err);
	}

	/* Cycling Speed & Cadence Profile Server */
	uint16_t start_hdl_csc = 0;
	struct cscps_db_cfg cscps_cfg = {
		.csc_feature = CSCP_FEAT_CRANK_REV_DATA_SUPP_BIT,
		.sensor_loc = CSCP_LOC_FRONT_WHEEL,
		.sensor_loc_supp = 0x01,
	};

	err = prf_add_profile(TASK_ID_CSCPS, 0, 0, &cscps_cfg, &cscps_cb, &start_hdl_csc);
	if (err) {
		LOG_ERR("Error %u adding CSCPS profile", err);
	}

	/* Running Speed & Cadence Profile Server */
	uint16_t start_hdl_rsc = 0;
	struct rscps_db_cfg rsc_cfg = {
		.rsc_feature = RSCP_FEAT_INST_STRIDE_LEN_SUPP_BIT |
			       RSCP_FEAT_WALK_RUN_STATUS_SUPP_BIT |
			       RSCP_FEAT_TOTAL_DST_MEAS_SUPP_BIT,
		.sensor_loc_supp = 0x01,
		.sensor_loc = RSCP_LOC_CHEST,
	};

	err = prf_add_profile(TASK_ID_RSCPS, 0, 0, &rsc_cfg, &rscps_cb, &start_hdl_rsc);
	if (err) {
		LOG_ERR("Error %u adding RSCPS profile", err);
	}
}

/* Measurement send helpers */

/* Send a dummy heart rate measurement to all subscribed connections */
static void send_hr_measurement(uint16_t value)
{
	uint16_t err;

	hrs_hr_meas_t hr_meas = {
		.flags = HRS_FLAG_HR_VALUE_FORMAT_POS,
		.heart_rate = value,
		.nb_rr_interval = 0,
	};

	/* Send to all subscribed connections */
	uint32_t conidx_bf = UINT32_MAX;

	err = hrps_meas_send(conidx_bf, &hr_meas);
	if (err) {
		LOG_ERR("Error %u sending HR measurement", err);
	}
}

/* Send a dummy blood pressure measurement to connection index 0 */
static void send_bp_measurement(uint16_t value)
{
	uint16_t err;

	/* Dummy timestamp */
	prf_date_time_t ts = {
		.year = 2024,
		.month = 0x04,
		.day = 0x01,
		.hour = 0x01,
		.min = 0x01,
		.sec = 0x01,
	};

	/* Dummy BP measurement */
	bps_bp_meas_t meas = {
		.flags = BPS_MEAS_FLAG_TIME_STAMP_BIT | BPS_MEAS_PULSE_RATE_BIT,
		.user_id = 0,
		.systolic = value,
		.diastolic = value - 10,
		.mean_arterial_pressure = value - 5,
		.pulse_rate = 90,
		.meas_status = 0x01,
		.time_stamp = ts,
	};

	/* Send to first connection only (matches BLPS sample behavior) */
	err = blps_meas_send(0, true, &meas);
	if (err) {
		LOG_ERR("Error %u sending BP measurement", err);
	}
}

/* Cycle dummy sensor value between 70 and 130 (shared by HR and BP) */
static void update_sensor_value(void)
{
	if (current_value >= 130) {
		current_value = 70;
	} else {
		current_value++;
	}
}

/* Oscillate dummy temperature between 35C and 40C */
static void update_ht_temp(void)
{
	ht_temp_value += ht_direction;

	if (ht_temp_value == 40 || ht_temp_value == 35) {
		ht_direction = -ht_direction;
	}
}

static void send_htpt_measurement(void)
{
	uint16_t err;

	htp_temp_meas_t meas = {
		.flags = HTP_UNIT_CELCIUS,
		.temp = ht_temp_value,
	};

	err = htpt_temp_send(&meas, HTP_TEMP_STABLE);
	if (err) {
		LOG_ERR("HTP send error %u", err);
	}
}

static void send_csc_measurement(void)
{
	uint16_t err;

	cs_evt_time += 2000; /* Dummy event time advance */

	cs_meas.cumul_wheel_rev += 6;
	cs_meas.last_wheel_evt_time = cs_evt_time;

	cs_meas.cumul_crank_rev += 3;
	cs_meas.last_crank_evt_time = cs_evt_time;

	err = cscps_meas_send(UINT32_MAX, 0, &cs_meas);
	if (err) {
		LOG_ERR("Error %u sending CSCPS measurement", err);
	}
}

static void send_rsc_measurement(void)
{
	uint16_t err;

	rscp_rsc_meas_t meas = {
		.flags = RSCP_MEAS_ALL_PRESENT,
		.inst_speed = 0x1C2 - rsc_current_value,
		.inst_cad = 0xA0 - rsc_current_value,
		.inst_stride_len = 0x96 - rsc_current_value,
		.total_dist = rsc_total_distance,
	};

	rsc_total_distance += (meas.inst_speed * 0.0039111 * 10);

	err = rscps_meas_send(UINT32_MAX, &meas);
	if (err) {
		LOG_ERR("RSC send error %u", err);
	}
}

static void update_rsc_value(void)
{
	if (rsc_current_value >= 4) {
		rsc_current_value = 1;
	} else {
		rsc_current_value++;
	}
}

void LedWorkerHandler(struct k_work *work)
{
	int res_schedule_time = 0;

	if (ctrl.connected) {
		ble_gpio_led_set(&led2, false);
	} else {
		ble_gpio_led_toggle(&led2);
		res_schedule_time = 500;
	}

	if (res_schedule_time) {
		k_work_reschedule(&ledWork, K_MSEC(res_schedule_time));
	}
}

/* Main periodic processing (called every 1 second from main loop) */
static void combined_process(void)
{

	if (!ctrl.connected) {
		return;
	}

	update_sensor_value();

	/* HR gated by CCCD enable + send complete */
	if (hr_ready_to_send) {
		send_hr_measurement(current_value);
		hr_ready_to_send = false;
	}

	/* BP gated by CCCD enable + indication/notification complete */
	if (bp_ready_to_send) {
		send_bp_measurement(current_value);
		bp_ready_to_send = false;
	}

	update_ht_temp();
	if (ht_ready_to_send) {
		send_htpt_measurement();
		ht_ready_to_send = false;
	}

	/* Glucose sends one record per connection (it's a historical log, not a stream) */
	if (gl_ready_to_send && !gl_sent_once) {
		send_glucose_once();
		gl_sent_once = true;
	}

	/* CSCPS gated by CCCD enable + send complete */
	if (cs_ready_to_send) {
		send_csc_measurement();
		cs_ready_to_send = false;
	}

	/* RSCPS gated by CCCD enable + send complete */
	update_rsc_value();
	if (rsc_ready_to_send) {
		send_rsc_measurement();
		rsc_ready_to_send = false;
	}
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		ctrl.connected = true;
		LOG_INF("Connection index %u connected to known device", con_idx);
		break;
	case GAPM_API_DEV_CONNECTED:
		ctrl.connected = true;
		LOG_INF("Connection index %u connected to new device", con_idx);
		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Connection index %u disconnected for reason %u", con_idx, status);
		ctrl.connected = false;
		on_disconnection(con_idx, status); /* Notify PRXP and reset state */
		break;
	case GAPM_API_PAIRING_FAIL:
		LOG_INF("Connection pairing index %u fail for reason %u", con_idx, status);
		break;
	}
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = app_connection_status_update,
};

int main(void)
{
	uint16_t err;
	/* Configure LED0 (BLE controlled by blinky) & LED2 (connection status indicator) */
	err = ble_gpio_led_init();
	if (err) {
		LOG_ERR("Led Init fail %u", err);
		return -1;
	}

	/* Start up bluetooth host stack */
	alif_ble_enable(NULL);

	/* Use static random address for advertising */
	if (address_verification(ALIF_STATIC_RAND_ADDR, &adv_type, &gapm_cfg)) {
		LOG_ERR("Address verification failed");
		return -EADV;
	}

	/* Configure Bluetooth Stack */
	LOG_INF("Init gapm service");
	err = bt_gapm_init(&gapm_cfg, &gapm_user_cb, DEVICE_NAME, strlen(DEVICE_NAME));
	if (err) {
		LOG_ERR("gapm_configure error %u", err);
		return -1;
	}

	/* Share connection state with battery and other sub-services */
	service_conn(&ctrl);

	config_battery_service();
	blinky_init();

	/* Add all standard BLE profiles (share the profile framework GATT user) */
	bundle_server_configure();
	prxp_server_configure(); /* Registers LLSS only */

	/* Create and start advertising */
	err = create_advertising();
	if (err) {
		LOG_ERR("Advertisement create fail %u", err);
		return -1;
	}

	err = set_advertising_data(adv_actv_idx);
	if (err) {
		LOG_ERR("Advertising data set fail %u", err);
		return -1;
	}

	err = set_scan_data(adv_actv_idx);
	if (err) {
		LOG_ERR("Scan data set fail %u", err);
		return -1;
	}

	err = bt_gapm_advertisement_start(adv_actv_idx);
	if (err) {
		LOG_ERR("Advertisement start fail %u", err);
		return -1;
	}
	print_device_identity();
	/* Set a Led init state */
	k_work_reschedule(&ledWork, K_MSEC(1));

	/* Main loop: process measurements and battery every second */
	while (1) {
		k_sleep(K_SECONDS(1));
		combined_process();
		battery_process();
	}
}
