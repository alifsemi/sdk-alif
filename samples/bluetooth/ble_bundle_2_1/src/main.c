// Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
// Use, distribution and modification of this code is permitted under the
// terms stated in the Alif Semiconductor Software License Agreement
//
// You should have received a copy of the Alif Semiconductor Software
// License Agreement with this file. If not, please write to:
// contact@alifsemi.com, or visit: https://alifsemi.com/license

// BLE Bundle: Multi-service peripheral
// Exposes the following services simultaneously:
//   - Heart Rate (HRPS)
//   - Blood Pressure (BLPS)
//   - Health Thermometer (HTPT)
//   - Glucose (GLPS)
//   - Cycling Speed & Cadence (CSCPS)
//   - Running Speed & Cadence (RSCPS)
//   - Link Loss (LLSS via PRXP)
//   - Battery (BASS)
//   - Blinky LED control (custom 128-bit GATT service)
// All measurements are dummy values sent periodically, gated by CCCD subscription.

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

// Heart Rate Profile Server
#include "hrp_common.h"
#include "hrps.h"

// Blood Pressure Profile Server
#include "blps.h"
#include "blps_msg.h"
#include "prf_types.h"
#include "rwprf_config.h"

// Health Thermometer Profile Server
#include "htpt.h"
#include "htpt_msg.h"

// Glucose Profile Server
#include "glps.h"
#include "glps_msg.h"

// Cycling Speed & Cadence Profile Server
#include "cscp_common.h"
#include "cscps.h"
#include "cscps_msg.h"

// Running Speed & Cadence Profile Server
#include "rscp_common.h"
#include "rscps.h"
#include "rscps_msg.h"

// Proximity Profile (Link Loss only — IASS and TPSS removed to stay within profile task limit)
#include "prxp_app.h"

// Custom Blinky GATT service
#include "gatt_db.h"
#include "gatt_srv.h"
#include "ke_mem.h"

// ============================================================
// GPIO aliases
// LED0: controlled by BLE (blinky service)
// LED2: connection status indicator (blinks while advertising)
// ============================================================
#define LED0_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led2)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);

// ============================================================
// HRPS feature flags
// Not exported by the SDK headers, so defined locally here.
// ============================================================
enum hrps_feat_bf {
    HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS = 0,
    HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT = CO_BIT(HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS),
    HRPS_ENGY_EXP_FEAT_SUP_POS = 1,
    HRPS_ENGY_EXP_FEAT_SUP_BIT = CO_BIT(HRPS_ENGY_EXP_FEAT_SUP_POS),
    HRPS_HR_MEAS_NTF_CFG_POS = 2,
    HRPS_HR_MEAS_NTF_CFG_BIT = CO_BIT(HRPS_HR_MEAS_NTF_CFG_POS),
};

// HTPT stable measurement indication config value (not exported by SDK headers)
#define HTPT_CFG_STABLE_MEAS_IND_DIS 0

// Dummy temperature state: oscillates between 35C and 40C
static uint32_t ht_temp_value = 35;
static int8_t ht_direction = 1;

// Body sensor location value used in HRPS configuration
#define BODY_SENSOR_LOCATION_CHEST 0x01

// Use static random address for advertising
#define SAMPLE_ADDR_TYPE ALIF_STATIC_RAND_ADDR

// ============================================================
// Global state
// ============================================================

// Advertising address type, resolved at startup by address_verification()
static uint8_t adv_type;

// Shared connection state (referenced by battery and sub-services)
struct shared_control ctrl = { false, 0, 0 };

// Dummy sensor value cycling between 70-130 (shared by HR and BP)
static uint16_t current_value = 70;

// Per-service send gates.
// Each flag is set when CCCD notifications are enabled AND the previous send completed.
static bool hr_ready_to_send;
static bool bp_ready_to_send;
static bool ht_ready_to_send;
static bool gl_ready_to_send;
static bool gl_sent_once;   // GLPS sends a single record per connection session
static bool cs_ready_to_send;
static bool rsc_ready_to_send;

// Forward declaration: send_glucose_once is used in on_glps_racp_req
// which is defined before the function body appears in this file.
static void send_glucose_once(void);

// Semaphores for startup and connection sequencing
K_SEM_DEFINE(init_sem, 0, 1);
K_SEM_DEFINE(conn_sem, 0, 1);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

// ============================================================
// BLINKY — Custom 128-bit GATT Service
// Allows a BLE central (phone app) to toggle LED0 on the board.
// Compatible with the nRF Blinky app and Alif's own app.
//
// GATT user slot note: The Alif BLE ROM has a hardcoded limit of
// 4 GATT user slots. blinky_init() must be called early (before
// bundle_server_configure) to guarantee it gets a slot.
// Slots used:
//   0: bt_srv_hello (system, automatic)
//   1: config_battery_service()
//   2: blinky_init()
//   3: (free — prxp uses prf_add_profile, no slot consumed)
// ============================================================

// Service UUID:       00001523-1212-EFDE-1523-785FEABCD123
#define BLINKY_UUID_SVC  {0x23,0xD1,0xBC,0xEA,0x5F,0x78,0x23,0x15,0xDE,0xEF,0x12,0x12,0x23,0x15,0x00,0x00}

// LED characteristic: 00001525-1212-EFDE-1523-785FEABCD123
// Must be 0x1525 (not 0x1524) — this is what Alif's app looks for to show the toggle UI.
#define BLINKY_UUID_CHAR {0x23,0xD1,0xBC,0xEA,0x5F,0x78,0x23,0x15,0xDE,0xEF,0x12,0x12,0x25,0x15,0x00,0x00}

static const uint8_t blinky_svc_uuid128[16] = BLINKY_UUID_SVC;

// Attribute indices into the blinky GATT database
enum {
    BLINKY_IDX_SVC,        // Primary service declaration
    BLINKY_IDX_CHAR_DECL,  // Characteristic declaration
    BLINKY_IDX_CHAR_VAL,   // Characteristic value (Read / Write / Notify)
    BLINKY_IDX_CHAR_CCCD,  // Client Characteristic Configuration Descriptor
    BLINKY_IDX_NB,         // Total attribute count
};

// Blinky runtime state
struct blinky_env {
    uint16_t start_hdl;    // First GATT handle, assigned during registration
    uint8_t  user_lid;     // GATT user local ID returned by gatt_user_srv_register
    uint8_t  value;        // Current LED state: 0 = off, 1 = on
    uint16_t cccd;         // Cached CCCD value (notification subscription state)
    bool     ntf_ongoing;  // True while a notification packet is in flight
};

static struct blinky_env blinky;

// Set to true on the first write from the phone.
// While true, combined_process() skips all LED blinking logic.
static bool blinky_active = false;

// GATT attribute table for the blinky service.
// sizeof(uint8_t) in CHAR_VAL's ext field is required — without it the BLE
// stack rejects writes and the write callback is never invoked.
static const gatt_att_desc_t blinky_att_db[BLINKY_IDX_NB] = {
    [BLINKY_IDX_SVC] =
        { ATT_128_PRIMARY_SERVICE, ATT_UUID(16) | PROP(RD), 0 },

    [BLINKY_IDX_CHAR_DECL] =
        { ATT_128_CHARACTERISTIC, ATT_UUID(16) | PROP(RD), 0 },

    [BLINKY_IDX_CHAR_VAL] =
        { BLINKY_UUID_CHAR, ATT_UUID(128) | PROP(RD) | PROP(WR) | PROP(N), OPT(NO_OFFSET) | sizeof(uint8_t) },

    [BLINKY_IDX_CHAR_CCCD] =
        { ATT_128_CLIENT_CHAR_CFG, ATT_UUID(16) | PROP(RD) | PROP(WR), 0 },
};

// ============================================================
// Bluetooth stack configuration
// ============================================================
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
    .class_of_device = 0,  // BT Classic only, unused in LE peripheral role
    .dflt_link_policy = 0, // BT Classic only, unused in LE peripheral role
};

// Device name loaded from prj.conf (CONFIG_BLE_DEVICE_NAME)
#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME
static const char device_name[] = DEVICE_NAME;

// Saved advertising activity index, used to restart advertising after disconnection
static uint8_t adv_actv_idx;

// ============================================================
// CSCPS state (Cycling Speed & Cadence)
// ============================================================
static cscp_csc_meas_t cs_meas = {
    .flags = CSCP_MEAS_CRANK_REV_DATA_PRESENT_BIT,
};
static uint16_t cs_evt_time = 0;

// ============================================================
// RSCPS state (Running Speed & Cadence)
// ============================================================
static uint32_t rsc_total_distance = 0;
static uint16_t rsc_current_value = 1;

// ============================================================
// Advertising helper
// ============================================================

static uint16_t start_le_adv(uint8_t actv_idx)
{
    uint16_t err;

    gapm_le_adv_param_t adv_params = {
        .duration = 0, // Advertise indefinitely
    };

    err = gapm_le_start_adv(actv_idx, &adv_params);
    if (err) {
        LOG_ERR("Failed to start LE advertising with error %u", err);
    }
    return err;
}

// ============================================================
// GAP connection callbacks
// ============================================================

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

    // Restart advertising so new connections can be made
    err = start_le_adv(adv_actv_idx);
    if (err) {
        LOG_ERR("Error restarting advertising: %u", err);
    } else {
        LOG_DBG("Restarting advertising");
    }

    // Notify PRXP (triggers link loss alert if disconnection was not user-initiated)
    prxp_disc_notify(reason);

    ctrl.connected = false;
    blinky_active = false;
    gpio_pin_set_dt(&led0, 0); // Turn off BLE-controlled LED on disconnect

    // Reset all service send gates — phone must re-enable CCCDs after reconnect
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
    // Send 'unknown' appearance
    gapc_le_get_appearance_cfm(conidx, token, GAP_ERR_NO_ERROR, 0);
}

// ============================================================
// HRPS callbacks (Heart Rate Profile Server)
// ============================================================

static void on_hrps_meas_send_complete(uint16_t status)
{
    hr_ready_to_send = true; // Previous send finished, allow next measurement
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

// ============================================================
// BLPS callbacks (Blood Pressure Profile Server)
// ============================================================

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

// ============================================================
// HTPT callbacks (Health Thermometer Profile Server)
// ============================================================

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

// ============================================================
// GLPS callbacks (Glucose Profile Server)
// ============================================================

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

// Called when the app sends a RACP request (e.g. user taps reload button).
// We send the dummy measurement first, then report 1 record so the app shows a value.
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

// ============================================================
// GLPS helpers
// ============================================================

// Convert mg/dL float to the 16-bit SFLOAT format used in GLP measurements
static prf_sfloat glucose_to_sfloat(float mg_dl)
{
    uint16_t mantissa = ((uint16_t)mg_dl) & 0x0FFF;
    uint16_t exponent = 0x0; // simple
    return (exponent << 12) | mantissa;
}

// Send a single dummy glucose record (95 mg/dL, capillary whole blood, finger)
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

// ============================================================
// CSCPS callbacks (Cycling Speed & Cadence Profile Server)
// ============================================================

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

// ============================================================
// RSCPS callbacks (Running Speed & Cadence Profile Server)
// ============================================================

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

// ============================================================
// GAP callback structs
// ============================================================

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

// All callbacks in this struct are optional — left empty
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

// ============================================================
// Profile callback structs
// ============================================================

static const hrps_cb_t hrps_cb = {
    .cb_bond_data_upd = on_hr_bond_data_upd,
    .cb_meas_send_cmp = on_hrps_meas_send_complete,
    .cb_energy_exp_reset = on_hr_energy_exp_reset,
};

static const blps_cb_t blps_cb = {
    .cb_bond_data_upd = on_blps_bond_data_upd,
    .cb_meas_send_cmp = on_blps_meas_send_complete,
};

// ============================================================
// Advertising data setup
// ============================================================

// Build the primary advertising packet with all standard 16-bit service UUIDs.
// num_svc must exactly match the number of memcpy calls below, or the
// buffer allocation and packet length will be wrong.
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
    //uint16_t svc_ias  = GATT_SVC_IMMEDIATE_ALERT;  // removed: IASS not registered
    //uint16_t svc_tps  = GATT_SVC_TX_POWER;         // removed: TPSS not registered
    uint16_t svc_batt = get_batt_id();

    const uint8_t num_svc = 8; // Must match memcpy count below
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
    //memcpy(p, &svc_ias,  2); p += 2;
    //memcpy(p, &svc_tps,  2); p += 2;
    memcpy(p, &svc_batt, 2);

    err = gapm_le_set_adv_data(actv_idx, p_buf);
    co_buf_release(p_buf);
    return err;
}

// Build the scan response packet with the device name and the blinky 128-bit UUID.
// Splitting across adv + scan response keeps both within the 31-byte BLE limit.
static uint16_t set_scan_data(uint8_t actv_idx)
{
    uint16_t err;
    co_buf_t *p_buf;

    const size_t name_len = sizeof(device_name) - 1;
    const uint16_t scan_len =
        1 + 1 + name_len +   // length byte + type byte + device name
        1 + 1 + 16;          // length byte + type byte + 128-bit UUID

    err = co_buf_alloc(&p_buf, 0, scan_len, 0);
    __ASSERT(err == 0, "SCAN buffer alloc failed");

    uint8_t *p = co_buf_data(p_buf);

    // Complete device name
    *p++ = name_len + 1;
    *p++ = GAP_AD_TYPE_COMPLETE_NAME;
    memcpy(p, device_name, name_len);
    p += name_len;

    // 128-bit Blinky service UUID
    *p++ = GATT_UUID_128_LEN + 1;
    *p++ = GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID;
    memcpy(p, blinky_svc_uuid128, 16);

    err = gapm_le_set_scan_response_data(actv_idx, p_buf);
    co_buf_release(p_buf);
    return err;
}

// ============================================================
// Advertising activity callbacks
// ============================================================

static void on_adv_actv_stopped(uint32_t metainfo, uint8_t actv_idx, uint16_t reason)
{
    LOG_DBG("Advertising activity index %u stopped for reason %u", actv_idx, reason);
}

// Advertising setup state machine: create -> set adv data -> set scan data -> start
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

// ============================================================
// Blinky GATT callbacks
// ============================================================

static void blinky_att_read(uint8_t conidx, uint8_t user_lid, uint16_t token,
                            uint16_t hdl, uint16_t offset, uint16_t max_len)
{
    co_buf_t *buf;
    co_buf_alloc(&buf, GATT_BUFFER_HEADER_LEN, 1, GATT_BUFFER_TAIL_LEN);
    *co_buf_data(buf) = blinky.value;
    gatt_srv_att_read_get_cfm(conidx, user_lid, token, GAP_ERR_NO_ERROR, 1, buf);
    co_buf_release(buf);
}

// Old single-attribute write handler kept for reference, superseded by the version below
// static void blinky_att_write(uint8_t conidx, uint8_t user_lid, uint16_t token,
//                              uint16_t hdl, uint16_t offset, co_buf_t *data)
// {
//     memcpy(&blinky.cccd, co_buf_data(data), sizeof(uint16_t));
//     gatt_srv_att_val_set_cfm(conidx, user_lid, token, GAP_ERR_NO_ERROR);
// }

// Handles writes to both the LED characteristic value and the CCCD.
// att_idx is derived from the handle offset so we can dispatch correctly.
static void blinky_att_write(uint8_t conidx, uint8_t user_lid, uint16_t token,
                             uint16_t hdl, uint16_t offset, co_buf_t *data)
{
    uint16_t att_idx = hdl - blinky.start_hdl;

    if (att_idx == BLINKY_IDX_CHAR_VAL) {
        // Reject writes with wrong length before touching the GPIO
        if (co_buf_data_len(data) != sizeof(uint8_t)) {
            gatt_srv_att_val_set_cfm(conidx, user_lid, token, ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN);
            return;
        }
        blinky_active = true; // Hand LED control to the phone, stop blinking logic
        blinky.value = *co_buf_data(data);
        gpio_pin_set_dt(&led0, blinky.value ? 1 : 0);

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

// ============================================================
// Advertising activity creation
// ============================================================

static uint16_t create_advertising(void)
{
    uint16_t err;

    gapm_le_adv_create_param_t adv_create_params = {
        .prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
        .disc_mode = GAPM_ADV_MODE_GEN_DISC,
        .tx_pwr = 0,
        .filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
        .prim_cfg = {
            .adv_intv_min = 160, // 100 ms
            .adv_intv_max = 800, // 500 ms
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

// ============================================================
// Blinky service registration
// ============================================================

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

// ============================================================
// Standard BLE profile registration
// All profiles here use prf_add_profile() which shares the
// profile framework's GATT user and does NOT consume a ROM
// GATT user slot.
// ============================================================

static void bundle_server_configure(void)
{
    uint16_t err;

    // Heart Rate Profile Server
    uint16_t start_hdl_hr = 0;
    struct hrps_db_cfg hrps_cfg = {0};

    hrps_cfg.features = HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT | HRPS_HR_MEAS_NTF_CFG_BIT;
    hrps_cfg.body_sensor_loc = BODY_SENSOR_LOCATION_CHEST;

    err = prf_add_profile(TASK_ID_HRPS, 0, 0, &hrps_cfg, &hrps_cb, &start_hdl_hr);
    if (err) {
        LOG_ERR("Error %u adding HRPS profile", err);
    }

    // Blood Pressure Profile Server
    uint16_t start_hdl_bp = 0;
    struct blps_db_cfg blps_cfg = {0};

    blps_cfg.features = 0;
    blps_cfg.prfl_cfg = 0;

    err = prf_add_profile(TASK_ID_BLPS, 0, 0, &blps_cfg, &blps_cb, &start_hdl_bp);
    if (err) {
        LOG_ERR("Error %u adding BLPS profile", err);
    }

    // Health Thermometer Profile Server
    uint16_t start_hdl_ht = 0;
    struct htpt_db_cfg htpt_cfg = {
    .features = HTPT_TEMP_TYPE_CHAR_SUP_BIT,
    .temp_type = HTP_TYPE_BODY,
    };

    err = prf_add_profile(TASK_ID_HTPT, 0, 0, &htpt_cfg, &htpt_cb, &start_hdl_ht);
    if (err) {
    LOG_ERR("Error %u adding HTPT profile", err);
    }

    // Glucose Profile Server
    // meas_ctx_supported = 0: measurement context characteristic not included
    uint16_t start_hdl_gl = 0;
    //struct glps_db_cfg glps_cfg = {0};
    struct glps_db_cfg glps_cfg = {
    .meas_ctx_supported = 0,
    };
    err = prf_add_profile(
    TASK_ID_GLPS,
    0,      // No security required
    0,
    &glps_cfg,
    &glps_cb,
    &start_hdl_gl);

    if (err) {
    LOG_ERR("Error %u adding GLPS profile", err);
    }

    // Cycling Speed & Cadence Profile Server
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

    // Running Speed & Cadence Profile Server
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

// Called when gapm_configure() completes — unblocks main() to continue setup
void on_gapm_process_complete(uint32_t metainfo, uint16_t status)
{
    if (status) {
        LOG_ERR("gapm process completed with error %u", status);
        return;
    }

    LOG_DBG("gapm process completed successfully");
    k_sem_give(&init_sem);
}

// ============================================================
// Measurement send helpers
// ============================================================

// Send a dummy heart rate measurement to all subscribed connections
static void send_hr_measurement(uint16_t value)
{
    uint16_t err;

    hrs_hr_meas_t hr_meas = {
        .flags = HRS_FLAG_HR_VALUE_FORMAT_POS,
        .heart_rate = value,
        .nb_rr_interval = 0,
    };

    // Send to all subscribed connections
    uint32_t conidx_bf = UINT32_MAX;
    err = hrps_meas_send(conidx_bf, &hr_meas);

    if (err) {
        LOG_ERR("Error %u sending HR measurement", err);
    }
}

// Send a dummy blood pressure measurement to connection index 0
static void send_bp_measurement(uint16_t value)
{
    uint16_t err;

    // Dummy timestamp
    prf_date_time_t ts = {
        .year = 2024,
        .month = 0x04,
        .day = 0x01,
        .hour = 0x01,
        .min = 0x01,
        .sec = 0x01,
    };

    // Dummy BP measurement
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

    // Send to first connection only (matches BLPS sample behavior)
    err = blps_meas_send(0, true, &meas);

    if (err) {
        LOG_ERR("Error %u sending BP measurement", err);
    }
}

// Cycle dummy sensor value between 70 and 130 (shared by HR and BP)
static void update_sensor_value(void)
{
    // Generating dummy values between 70 and 130
    if (current_value >= 130) {
        current_value = 70;
    } else {
        current_value++;
    }
}

// Oscillate dummy temperature between 35C and 40C
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
    cs_evt_time += 2000;  // Dummy event time advance

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

// ============================================================
// Main periodic processing (called every 1 second from main loop)
// ============================================================

static void combined_process(void)
{
    static bool led2_state;

    // If blinky is active the phone owns LED0 — don't touch it here
    if (blinky_active) {
        return;
    }

    // Not connected: blink LED2 to indicate advertising
    if (!ctrl.connected) {
        led2_state = !led2_state;
        gpio_pin_set_dt(&led2, led2_state);
        return;
    }

    // Connected: keep LED2 steady off
    gpio_pin_set_dt(&led2, 0);
    update_sensor_value();

    // if (!ctrl.connected) {
    //     LOG_DBG("Waiting for peer connection...\n");
    //     k_sem_take(&conn_sem, K_FOREVER);
    //     return;
    // }

    // HR gated by CCCD enable + send complete
    if (hr_ready_to_send) {
        send_hr_measurement(current_value);
        hr_ready_to_send = false;
    }

    // BP gated by CCCD enable + indication/notification complete
    if (bp_ready_to_send) {
        send_bp_measurement(current_value);
        bp_ready_to_send = false;
    }

    update_ht_temp();
    if (ht_ready_to_send) {
    send_htpt_measurement();
    ht_ready_to_send = false;
    }

    // Glucose sends one record per connection (it's a historical log, not a stream)
    if (gl_ready_to_send && !gl_sent_once) {
    send_glucose_once();
    gl_sent_once = true;
    }

    // CSCPS gated by CCCD enable + send complete
    if (cs_ready_to_send) {
        send_csc_measurement();
        cs_ready_to_send = false;
    }

    // RSCPS gated by CCCD enable + send complete
    update_rsc_value();
    if (rsc_ready_to_send) {
        send_rsc_measurement();
        rsc_ready_to_send = false;
    }
}

// ============================================================
// Entry point
// ============================================================

int main(void)
{
    uint16_t err;

    // Start up bluetooth host stack
    alif_ble_enable(NULL);

    if (address_verification(SAMPLE_ADDR_TYPE, &adv_type, &gapm_cfg)) {
        LOG_ERR("Address verification failed");
        return -EADV;
    }

    // Merge PRXP callbacks into the main GAP callback struct before configuring
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

    // Configure LED0 (BLE controlled by blinky)
    if (!gpio_is_ready_dt(&led0)) {
    LOG_ERR("LED0 not ready");
    return 0;
    }
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

    // Configure LED2 (connection status indicator)
    if (!gpio_is_ready_dt(&led2)) {
    LOG_ERR("LED2 not ready");
    return 0;
    }
    gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);

    // Share connection state with battery and other sub-services
    service_conn(&ctrl);

    // Register services in GATT user slot order.
    // The Alif BLE ROM has a hardcoded limit of 4 GATT user slots:
    //   slot 0: bt_srv_hello (system, registered automatically)
    //   slot 1: config_battery_service()
    //   slot 2: blinky_init() — must come before bundle/prxp
    //   slot 3: (unused — prxp and bundle use prf_add_profile, no slot needed)
    config_battery_service();
    blinky_init();
    // LOG_INF("blinky_init result: %u, start_hdl: %u, user_lid: %u",
    //         berr, blinky.start_hdl, blinky.user_lid);

    // Add all standard BLE profiles (share the profile framework GATT user)
    bundle_server_configure();
    prxp_server_configure(); // Registers LLSS only

    // Create and start advertising
    create_advertising();

    // Main loop: process measurements and battery every second
    while (1) {
        k_sleep(K_SECONDS(1));
        combined_process();
        battery_process();
        //ias_process(); // removed: IASS not registered
    }
}