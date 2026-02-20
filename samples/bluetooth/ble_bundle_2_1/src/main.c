
/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Combined HRPS + BLPS example:
 * - Exposes Heart Rate + Blood Pressure + Battery services simultaneously
 * - Sends dummy measurements periodically, gated by CCCD enable from the phone
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

#include "shared_control.h"
#include "batt_svc.h"

#include "prf.h"
#include "bass.h"
/* HRPS */
#include "hrp_common.h"
#include "hrps.h"

/* BLPS */
#include "blps.h"
#include "blps_msg.h"
#include "prf_types.h"
#include "rwprf_config.h"

//HTPT
#include "htpt.h"
#include "htpt_msg.h"

//GLPS
#include "glps.h"
#include "glps_msg.h"

//CSCPS
#include "cscp_common.h"
#include "cscps.h"
#include "cscps_msg.h"

//RSCPS
#include "rscp_common.h"
#include "rscps.h"
#include "rscps_msg.h"

//PRXP
#include "prxp_app.h"

//Blinky
#include "gatt_db.h"
#include "gatt_srv.h"
#include "ke_mem.h"

#define LED0_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led2)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

enum hrps_feat_bf {
    HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS = 0,
    HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT = CO_BIT(HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS),
    HRPS_ENGY_EXP_FEAT_SUP_POS = 1,
    HRPS_ENGY_EXP_FEAT_SUP_BIT = CO_BIT(HRPS_ENGY_EXP_FEAT_SUP_POS),
    HRPS_HR_MEAS_NTF_CFG_POS = 2,
    HRPS_HR_MEAS_NTF_CFG_BIT = CO_BIT(HRPS_HR_MEAS_NTF_CFG_POS),
};

/* HTPT config values not exported by headers */
#define HTPT_CFG_STABLE_MEAS_IND_DIS 0
static uint32_t ht_temp_value = 35;   // dummy starting temp
static int8_t ht_direction = 1;

#define BODY_SENSOR_LOCATION_CHEST 0x01

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE    ALIF_STATIC_RAND_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

struct shared_control ctrl = { false, 0, 0 };

/* Initial dummy value */
static uint16_t current_value = 70;

/* Separate readiness flags for each service (set by CCCD enable + send complete) */
static bool hr_ready_to_send;
static bool bp_ready_to_send;
static bool ht_ready_to_send;
static bool gl_ready_to_send;
static bool gl_sent_once;
static bool cs_ready_to_send;
static bool rsc_ready_to_send;


K_SEM_DEFINE(init_sem, 0, 1);
K_SEM_DEFINE(conn_sem, 0, 1);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

/* ---------- BLINKY (Custom GATT Service) ---------- */


/* 128-bit UUIDs */
#define BLINKY_UUID_SVC   {0x23,0xD1,0xBC,0xEA,0x5F,0x78,0x23,0x15,0xDE,0xEF,0x12,0x12,0x23,0x15,0x00,0x00}
#define BLINKY_UUID_CHAR  {0x23,0xD1,0xBC,0xEA,0x5F,0x78,0x23,0x15,0xDE,0xEF,0x12,0x12,0x24,0x15,0x00,0x00}

static const uint8_t blinky_svc_uuid128[16] = BLINKY_UUID_SVC;
enum {
    BLINKY_IDX_SVC,
    BLINKY_IDX_CHAR_DECL,
    BLINKY_IDX_CHAR_VAL,
    BLINKY_IDX_CHAR_CCCD,
    BLINKY_IDX_NB,
};

struct blinky_env {
    uint16_t start_hdl;
    uint8_t  user_lid;
    uint8_t  value;
    uint16_t cccd;
    bool     ntf_ongoing;
};

static struct blinky_env blinky;
static bool blinky_active = false;

static const gatt_att_desc_t blinky_att_db[BLINKY_IDX_NB] = {
    [BLINKY_IDX_SVC] =
        { ATT_128_PRIMARY_SERVICE, ATT_UUID(16) | PROP(RD), 0 },

    [BLINKY_IDX_CHAR_DECL] =
        { ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0 },

    [BLINKY_IDX_CHAR_VAL] =
        { BLINKY_UUID_CHAR, ATT_UUID(128) | PROP(RD) | PROP(WR) | PROP(N), OPT(NO_OFFSET) },

    [BLINKY_IDX_CHAR_CCCD] =
        { ATT_128_CLIENT_CHAR_CFG, ATT_UUID(16) | PROP(RD) | PROP(WR), 0 },
};
/**
 * Bluetooth stack configuration
 */
static gapm_config_t gapm_cfg = {
    .role = GAP_ROLE_LE_PERIPHERAL,
    .pairing_mode = GAPM_PAIRING_DISABLE,
    .privacy_cfg = 0,
    .renew_dur = 1500,
    .private_identity.addr = {0xCA, 0xFE, 0xFB, 0xDE, 0x11, 0x07},
    .irk.key = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    .gap_start_hdl = 0,
    .gatt_start_hdl = 0,
    .att_cfg = 0,
    .sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
    .sugg_max_tx_time = GAP_LE_MIN_TIME,
    .tx_pref_phy = GAP_PHY_ANY,
    .rx_pref_phy = GAP_PHY_ANY,
    .tx_path_comp = 0,
    .rx_path_comp = 0,
    .class_of_device = 0,  /* BT Classic only */
    .dflt_link_policy = 0, /* BT Classic only */
};

/* Load name from configuration file */
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME
static const char device_name[] = DEVICE_NAME;

/* Store advertising activity index for re-starting after disconnection */
static uint8_t adv_actv_idx;


static cscp_csc_meas_t cs_meas = {
    .flags = CSCP_MEAS_CRANK_REV_DATA_PRESENT_BIT,
};

static uint16_t cs_evt_time = 0;
static uint32_t rsc_total_distance = 0;
static uint16_t rsc_current_value = 1;
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
static void on_le_connection_req(uint8_t conidx, uint32_t metainfo, uint8_t actv_idx, uint8_t role,
                                 const gap_bdaddr_t *p_peer_addr,
                                 const gapc_le_con_param_t *p_con_params, uint8_t clk_accuracy)
{
    LOG_INF("Connection request on index %u", conidx);
    gapc_le_connection_cfm(conidx, 0, NULL);

    LOG_DBG("Connection parameters: interval %u, latency %u, supervision timeout %u",
            p_con_params->interval, p_con_params->latency, p_con_params->sup_to);

    LOG_HEXDUMP_DBG(p_peer_addr->addr, GAP_BD_ADDR_LEN, "Peer BD address");

    ctrl.connected = true;

    k_sem_give(&conn_sem);

    LOG_DBG("Please enable notifications/indications on peer device..");
}

static void on_key_received(uint8_t conidx, uint32_t metainfo, const gapc_pairing_keys_t *p_keys)
{
    LOG_WRN("Unexpected key received on conidx %u", conidx);
}

static void on_disconnection(uint8_t conidx, uint32_t metainfo, uint16_t reason)
{
    uint16_t err;

    LOG_INF("Connection index %u disconnected for reason %u", conidx, reason);

    err = start_le_adv(adv_actv_idx);
    if (err) {
        LOG_ERR("Error restarting advertising: %u", err);
    } else {
        LOG_DBG("Restarting advertising");
    }

    prxp_disc_notify(reason);
    ctrl.connected = false;
    blinky_active = false;
    gpio_pin_set_dt(&led0, 0);

    /* Reset both service gates; phone must re-enable CCCDs after reconnect */
    hr_ready_to_send = false;
    bp_ready_to_send = false;
    ht_ready_to_send = false;
    gl_ready_to_send = false;
    gl_sent_once = false;
    cs_ready_to_send = false;
    rsc_ready_to_send = false;


}

static void on_name_get(uint8_t conidx, uint32_t metainfo, uint16_t token, uint16_t offset,
                        uint16_t max_len)
{
    const size_t device_name_len = sizeof(device_name) - 1;
    const size_t short_len = (device_name_len > max_len ? max_len : device_name_len);

    gapc_le_get_name_cfm(conidx, token, GAP_ERR_NO_ERROR, device_name_len, short_len,
                         (const uint8_t *)device_name);
}

static void on_appearance_get(uint8_t conidx, uint32_t metainfo, uint16_t token)
{
    /* Send 'unknown' appearance */
    gapc_le_get_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR, 0);
}

/* ---------------- HRPS callbacks ---------------- */

static void on_hrps_meas_send_complete(uint16_t status)
{
    hr_ready_to_send = true;
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

/* ---------------- BLPS callbacks ---------------- */

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
        LOG_INF("BP: Client requested stop notification/indication (conidx: %u)", conidx);
        bp_ready_to_send = false;
        break;

    case PRF_CLI_START_NTF:
    case PRF_CLI_START_IND:
        LOG_INF("BP: Client requested start notification/indication (conidx: %u)", conidx);
        bp_ready_to_send = true;
        break;

    default:
        break;
    }
}

/* ---------------- HTPT callbacks ---------------- */
static void on_htpt_meas_send_complete(uint16_t status)
{
    ARG_UNUSED(status);
    ht_ready_to_send = true;
}

static void on_htpt_bond_data_upd(uint8_t conidx, uint8_t cfg_val)
{
    switch (cfg_val)
    {
    case HTPT_CFG_STABLE_MEAS_IND:
        LOG_INF("HTP: Client enabled indications");
        ht_ready_to_send = true;
        break;

    case 0:
        LOG_INF("HTP: Client disabled indications");
        ht_ready_to_send = false;
        break;

    default:
        LOG_INF("HTP: Unsupported cfg change");
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

//GLPS callbacks
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

static void on_glps_racp_req(uint8_t conidx, uint8_t op_code,
                             uint8_t func_operator,
                             uint8_t filter_type,
                             const union glp_filter *p_filter)
{
    ARG_UNUSED(func_operator);
    ARG_UNUSED(filter_type);
    ARG_UNUSED(p_filter);

    // Dummy response: no records
    glps_racp_rsp_send(conidx, op_code, GLP_RSP_NO_RECS_FOUND, 0);
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



static prf_sfloat glucose_to_sfloat(float mg_dl)
{
    uint16_t mantissa = ((uint16_t)mg_dl) & 0x0FFF;
    uint16_t exponent = 0x0; // simple
    return (exponent << 12) | mantissa;
}

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

/* ---------------- CSCPS callbacks ---------------- */

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
            LOG_INF("CSC: Client requested stop notifications");
            cs_ready_to_send = false;
            break;

        case PRF_CLI_START_NTF:
        case PRF_CLI_START_IND:
            LOG_INF("CSC: Client requested start notifications");
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

/* ---------------- RSCPS callbacks ---------------- */

static void on_rsc_meas_send_complete(uint16_t status)
{
    ARG_UNUSED(status);
    rsc_ready_to_send = true;
}

static void on_rsc_bond_data_upd(uint8_t conidx, uint8_t char_code, uint16_t cfg_val)
{
    ARG_UNUSED(conidx);

    switch (cfg_val)
    {
    case PRF_CLI_STOP_NTFIND:
        LOG_INF("RSC: Client requested stop notifications");
        rsc_ready_to_send = false;
        break;

    case PRF_CLI_START_NTF:
    case PRF_CLI_START_IND:
        LOG_INF("RSC: Client requested start notifications");
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
/* ---------------- GAP callbacks wiring ---------------- */

static const gapc_connection_req_cb_t gapc_con_cbs = {
    .le_connection_req = on_le_connection_req,
};

static const gapc_security_cb_t gapc_sec_cbs = {
    .key_received = on_key_received,
};

static const gapc_connection_info_cb_t gapc_con_inf_cbs = {
    .disconnected = on_disconnection,
    .name_get = on_name_get,
    .appearance_get = on_appearance_get,
};

/* All callbacks in this struct are optional */
static const gapc_le_config_cb_t gapc_le_cfg_cbs;

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
    .p_bt_config_cbs = NULL,
    .p_gapm_cbs = &gapm_err_cbs,
};

/* ---------------- Profile callback structs ---------------- */

static const hrps_cb_t hrps_cb = {
    .cb_bond_data_upd = on_hr_bond_data_upd,
    .cb_meas_send_cmp = on_hrps_meas_send_complete,
    .cb_energy_exp_reset = on_hr_energy_exp_reset,
};

static const blps_cb_t blps_cb = {
    .cb_bond_data_upd = on_blps_bond_data_upd,
    .cb_meas_send_cmp = on_blps_meas_send_complete,
};

/* ---------------- Advertising data ---------------- */

static uint16_t set_advertising_data(uint8_t actv_idx)
{
    uint16_t err;

    uint16_t svc_hr   = GATT_SVC_HEART_RATE;
    uint16_t svc_bp   = GATT_SVC_BLOOD_PRESSURE;
    uint16_t svc_htp  = GATT_SVC_HEALTH_THERMOM;
    uint16_t svc_gl   = GATT_SVC_GLUCOSE;
    uint16_t svc_csc  = GATT_SVC_CYCLING_SPEED_CADENCE;
    uint16_t svc_rsc  = GATT_SVC_RUNNING_SPEED_CADENCE;
    uint16_t svc_lls  = GATT_SVC_LINK_LOSS;
    uint16_t svc_ias  = GATT_SVC_IMMEDIATE_ALERT;
    uint16_t svc_tps  = GATT_SVC_TX_POWER;
    uint16_t svc_batt = get_batt_id();

    const uint8_t num_svc = 10;
    const uint16_t adv_len = 1 + 1 + (num_svc * 2);

    co_buf_t *p_buf;
    err = co_buf_alloc(&p_buf, 0, adv_len, 0);
    __ASSERT(err == 0, "ADV buffer alloc failed");

    uint8_t *p = co_buf_data(p_buf);

    *p++ = (num_svc * 2) + 1;
    *p++ = GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID;

    memcpy(p, &svc_hr,   2); p += 2;
    memcpy(p, &svc_bp,   2); p += 2;
    memcpy(p, &svc_htp,  2); p += 2;
    memcpy(p, &svc_gl,   2); p += 2;
    memcpy(p, &svc_csc,  2); p += 2;
    memcpy(p, &svc_rsc,  2); p += 2;
    memcpy(p, &svc_lls,  2); p += 2;
    memcpy(p, &svc_ias,  2); p += 2;
    memcpy(p, &svc_tps,  2); p += 2;
    memcpy(p, &svc_batt, 2);

    err = gapm_le_set_adv_data(actv_idx, p_buf);
    co_buf_release(p_buf);
    return err;
}

static uint16_t set_scan_data(uint8_t actv_idx)
{
    uint16_t err;
    co_buf_t *p_buf;

    const size_t name_len = sizeof(device_name) - 1;
    const uint16_t scan_len =
        1 + 1 + name_len +        // device name
        1 + 1 + 16;               // 128-bit UUID

    err = co_buf_alloc(&p_buf, 0, scan_len, 0);
    __ASSERT(err == 0, "SCAN buffer alloc failed");

    uint8_t *p = co_buf_data(p_buf);

    /* Complete device name */
    *p++ = name_len + 1;
    *p++ = GAP_AD_TYPE_COMPLETE_NAME;
    memcpy(p, device_name, name_len);
    p += name_len;

    /* 128-bit Blinky service UUID */
    *p++ = GATT_UUID_128_LEN + 1;
    *p++ = GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID;
    memcpy(p, blinky_svc_uuid128, 16);

    err = gapm_le_set_scan_response_data(actv_idx, p_buf);
    co_buf_release(p_buf);
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
        print_device_identity();
        address_verification_log_advertising_address(actv_idx);
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

static void blinky_att_read(uint8_t conidx, uint8_t user_lid, uint16_t token,
                            uint16_t hdl, uint16_t offset, uint16_t max_len)
{
    co_buf_t *buf;
    co_buf_alloc(&buf, GATT_BUFFER_HEADER_LEN, 1, GATT_BUFFER_TAIL_LEN);
    *co_buf_data(buf) = blinky.value;
    gatt_srv_att_read_get_cfm(conidx, user_lid, token, GAP_ERR_NO_ERROR, 1, buf);
    co_buf_release(buf);
}

/* static void blinky_att_write(uint8_t conidx, uint8_t user_lid, uint16_t token,
                             uint16_t hdl, uint16_t offset, co_buf_t *data)
{
    memcpy(&blinky.cccd, co_buf_data(data), sizeof(uint16_t));
    gatt_srv_att_val_set_cfm(conidx, user_lid, token, GAP_ERR_NO_ERROR);
} */

static void blinky_att_write(uint8_t conidx, uint8_t user_lid, uint16_t token,
                             uint16_t hdl, uint16_t offset, co_buf_t *data)
{
    uint16_t att_idx = hdl - blinky.start_hdl;

    if (att_idx == BLINKY_IDX_CHAR_VAL) 
    {
        blinky_active = true; 
        blinky.value = *co_buf_data(data);
        gpio_pin_set_dt(&led0, blinky.value);
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
static uint16_t create_advertising(void)
{
    uint16_t err;

    gapm_le_adv_create_param_t adv_create_params = {
        .prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
        .disc_mode = GAPM_ADV_MODE_GEN_DISC,
        .tx_pwr = 0,
        .filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
        .prim_cfg = {
            .adv_intv_min = 160, /* 100 ms */
            .adv_intv_max = 800, /* 500 ms */
            .ch_map = ADV_ALL_CHNLS_EN,
            .phy = GAPM_PHY_TYPE_LE_1M,
        },
    };

    err = gapm_le_create_adv_legacy(0, adv_type, &adv_create_params, &le_adv_cbs);
    if (err) {
        LOG_ERR("Error %u creating advertising activity", err);
    }

    return err;
}

static uint16_t blinky_init(void)
{
    uint16_t err;

    err = gatt_user_srv_register(L2CAP_LE_MTU_MIN, 0,
                                 &blinky_gatt_cbs,
                                 &blinky.user_lid);
    if (err) return err;

    static const uint8_t svc_uuid[] = BLINKY_UUID_SVC;

    err = gatt_db_svc_add(blinky.user_lid, SVC_UUID(128),
                          svc_uuid,
                          BLINKY_IDX_NB,
                          NULL,
                          blinky_att_db,
                          BLINKY_IDX_NB,
                          &blinky.start_hdl);

    return err;
}
/* ---------------- Add profiles ---------------- */

static void bundle_server_configure(void)
{
    uint16_t err;

    /* HRPS */
    uint16_t start_hdl_hr = 0;
    struct hrps_db_cfg hrps_cfg = {0};

    hrps_cfg.features = HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT | HRPS_HR_MEAS_NTF_CFG_BIT;
    hrps_cfg.body_sensor_loc = BODY_SENSOR_LOCATION_CHEST;

    err = prf_add_profile(TASK_ID_HRPS, 0, 0, &hrps_cfg, &hrps_cb, &start_hdl_hr);
    if (err) {
        LOG_ERR("Error %u adding HRPS profile", err);
    }

    /* BLPS */
    uint16_t start_hdl_bp = 0;
    struct blps_db_cfg blps_cfg = {0};

    blps_cfg.features = 0;
    blps_cfg.prfl_cfg = 0;

    err = prf_add_profile(TASK_ID_BLPS, 0, 0, &blps_cfg, &blps_cb, &start_hdl_bp);
    if (err) {
        LOG_ERR("Error %u adding BLPS profile", err);
    }

    //HTPT
    uint16_t start_hdl_ht = 0;
    struct htpt_db_cfg htpt_cfg = {
    .features = HTPT_TEMP_TYPE_CHAR_SUP_BIT,
    .temp_type = HTP_TYPE_BODY,
    };

    err = prf_add_profile(TASK_ID_HTPT, 0, 0, &htpt_cfg, &htpt_cb, &start_hdl_ht);
    if (err) {
    LOG_ERR("Error %u adding HTPT profile", err);
    }


    //GLPS
    uint16_t start_hdl_gl = 0;
    struct glps_db_cfg glps_cfg = {0};
    err = prf_add_profile(
    TASK_ID_GLPS,
    0,      // NO security
    0,
    &glps_cfg,
    &glps_cb,
    &start_hdl_gl);

    if (err) {
    LOG_ERR("Error %u adding GLPS profile", err);
    }

    /* CSCPS */
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

     /* RSCPS */
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

void on_gapm_process_complete(uint32_t metainfo, uint16_t status)
{
    if (status) {
        LOG_ERR("gapm process completed with error %u", status);
        return;
    }

    LOG_DBG("gapm process completed successfully");
    k_sem_give(&init_sem);
}
/* ---------------- Measurement senders ---------------- */

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

    /* Send to first connection only (matches your BLPS sample behavior) */
    err = blps_meas_send(0, true, &meas);

    if (err) {
        LOG_ERR("Error %u sending BP measurement", err);
    }
}

static void update_sensor_value(void)
{
    /* Generating dummy values between 70 and 130 */
    if (current_value >= 130) {
        current_value = 70;
    } else {
        current_value++;
    }
}

static void update_ht_temp(void)
{
    ht_temp_value += ht_direction;

    if (ht_temp_value == 40 || ht_temp_value == 35)
        ht_direction = -ht_direction;
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
    cs_evt_time += 2000;  // dummy event time advance

    cs_meas.cumul_wheel_rev += 6;
    cs_meas.last_wheel_evt_time = cs_evt_time;

    cs_meas.cumul_crank_rev += 3;
    cs_meas.last_crank_evt_time = cs_evt_time;

    uint16_t err = cscps_meas_send(UINT32_MAX, 0, &cs_meas);
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
    if (rsc_current_value >= 4)
        rsc_current_value = 1;
    else
        rsc_current_value++;
    }

static void combined_process(void)
{
    static bool led2_state;

    /* Blinky owns the LED — do NOTHING here */
    if (blinky_active) {
        return;
    }

    /* Connection indicator logic */
    if (!ctrl.connected) {
        led2_state = !led2_state;
        gpio_pin_set_dt(&led2, led2_state);
        return;
    }

    gpio_pin_set_dt(&led2, 0);
    update_sensor_value();

    /* if (!ctrl.connected) {
        LOG_DBG("Waiting for peer connection...\n");
        k_sem_take(&conn_sem, K_FOREVER);
        return;
    } */

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

int main(void)
{
    uint16_t err;

    /* Start up bluetooth host stack */
    alif_ble_enable(NULL);

    if (address_verification(SAMPLE_ADDR_TYPE, &adv_type, &gapm_cfg)) {
        LOG_ERR("Address verification failed");
        return -EADV;
    }

    gapm_callbacks_t merged = gapm_cbs;
    merged = prxp_append_cbs(&merged);

    err = gapm_configure(0, &gapm_cfg, &merged, on_gapm_process_complete);
    if (err) {
        LOG_ERR("gapm_configure error %u", err);
        return -1;
    }

    LOG_DBG("Waiting for init...\n");
    k_sem_take(&init_sem, K_FOREVER);
    LOG_DBG("Init complete!\n");

    /* Configure LED0 (BLE controlled) */
    if (!gpio_is_ready_dt(&led0)) {
    LOG_ERR("LED0 not ready");
    return 0;
    }
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

    /* Configure LED2 (connection indicator) */
    if (!gpio_is_ready_dt(&led2)) {
    LOG_ERR("LED2 not ready");
    return 0;
    }
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);

    /* Share connection info */
    service_conn(&ctrl);

    /* Adding battery service */
    config_battery_service();

    /* Add both profiles */
    bundle_server_configure();
    prxp_server_configure();

    blinky_init();

    /* Create an advertising activity */
    create_advertising();

    while (1) {
        k_sleep(K_SECONDS(1));
        combined_process();
        battery_process();
        ias_process();

       /** if (ctrl.connected &&
        blinky.cccd == PRF_CLI_START_NTF &&
        !blinky.ntf_ongoing) {

        blinky.value ^= 1;

        co_buf_t *buf;
        co_buf_alloc(&buf, GATT_BUFFER_HEADER_LEN, 1, GATT_BUFFER_TAIL_LEN);
        *co_buf_data(buf) = blinky.value;

        gatt_srv_event_send(0, blinky.user_lid, 0,
                        GATT_NOTIFY,
                        blinky.start_hdl + BLINKY_IDX_CHAR_VAL,
                        buf);

        co_buf_release(buf);
        blinky.ntf_ongoing = true; } */
    }
}
