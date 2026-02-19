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

/* HRPS */
#include "hrp_common.h"
#include "hrps.h"

/* BLPS */
#include "blps.h"
#include "blps_msg.h"
#include "prf_types.h"
#include "rwprf_config.h"

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

K_SEM_DEFINE(init_sem, 0, 1);
K_SEM_DEFINE(conn_sem, 0, 1);

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

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

    ctrl.connected = false;

    /* Reset both service gates; phone must re-enable CCCDs after reconnect */
    hr_ready_to_send = false;
    bp_ready_to_send = false;
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

/*
static uint16_t set_advertising_data(uint8_t actv_idx)
{
    uint16_t err;

    // 16-bit Service UUIDs to advertise (HR + BP + Battery)
    uint16_t svc_hr   = GATT_SVC_HEART_RATE;
    uint16_t svc_bp   = GATT_SVC_BLOOD_PRESSURE;
    uint16_t svc_batt = get_batt_id();

    const uint8_t num_svc = 3;

    const size_t device_name_len = sizeof(device_name) - 1;
    const uint16_t adv_device_name = GATT_HANDLE_LEN + device_name_len;
    const uint16_t adv_uuid_svc = GATT_HANDLE_LEN + (GATT_UUID_16_LEN * num_svc); 


    // Keep adv small: Name + UUID list only (no manufacturer data) 
    const uint16_t adv_len = adv_device_name + adv_uuid_svc;

    co_buf_t *p_buf;
    err = co_buf_alloc(&p_buf, 0, adv_len, 0);
    __ASSERT(err == 0, "Buffer allocation failed");

    uint8_t *p_data = co_buf_data(p_buf);

    // Complete Local Name 
    p_data[0] = device_name_len + 1;
    p_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
    memcpy(p_data + 2, device_name, device_name_len);

    // 16-bit UUID list
    p_data += adv_device_name;
    p_data[0] = (GATT_UUID_16_LEN * num_svc) + 1;
    p_data[1] = GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID;

    memcpy(p_data + 2, (void *)&svc_hr, sizeof(svc_hr));
    memcpy(p_data + 4, (void *)&svc_bp, sizeof(svc_bp));
    memcpy(p_data + 6, (void *)&svc_batt, sizeof(svc_batt));

    err = gapm_le_set_adv_data(actv_idx, p_buf);
    co_buf_release(p_buf);

    if (err) {
        LOG_ERR("Failed to set advertising data with error %u", err);
    }

    return err;
} */

static uint16_t set_advertising_data(uint8_t actv_idx)
{
    uint16_t err;

    uint16_t svc_hr   = GATT_SVC_HEART_RATE;      // 0x180D
    uint16_t svc_bp   = GATT_SVC_BLOOD_PRESSURE;  // 0x1810
    uint16_t svc_batt = get_batt_id();            // 0x180F

    const uint8_t num_svc = 3;

    const uint16_t adv_uuid_svc = GATT_HANDLE_LEN + (GATT_UUID_16_LEN * num_svc);
    const uint16_t adv_len = adv_uuid_svc;

    co_buf_t *p_buf;
    err = co_buf_alloc(&p_buf, 0, adv_len, 0);
	 __ASSERT(err == 0, "ADV buffer alloc failed");

    uint8_t *p_data = co_buf_data(p_buf);

    // Length of UUID list
    p_data[0] = (GATT_UUID_16_LEN * num_svc) + 1;
    p_data[1] = GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID;

    memcpy(p_data + 2, &svc_hr,   sizeof(svc_hr));
    memcpy(p_data + 4, &svc_bp,   sizeof(svc_bp));
    memcpy(p_data + 6, &svc_batt, sizeof(svc_batt));

    err = gapm_le_set_adv_data(actv_idx, p_buf);
    co_buf_release(p_buf);

    return err;
}

/*static uint16_t set_scan_data(uint8_t actv_idx)
{
    co_buf_t *p_buf;
    uint16_t err = co_buf_alloc(&p_buf, 0, 0, 0);
    __ASSERT(err == 0, "Buffer allocation failed");

    err = gapm_le_set_scan_response_data(actv_idx, p_buf);
    co_buf_release(p_buf);

    if (err) {
        LOG_ERR("Failed to set scan data with error %u", err);
    }

    return err;
} */

static uint16_t set_scan_data(uint8_t actv_idx)
{
    co_buf_t *p_buf;
    uint16_t err;

    const size_t name_len = sizeof(device_name) - 1;
    const uint16_t scan_len = GATT_HANDLE_LEN + name_len;

    err = co_buf_alloc(&p_buf, 0, scan_len, 0);
    __ASSERT(err == 0, "SCAN buffer alloc failed");

    uint8_t *p_data = co_buf_data(p_buf);

    /* Complete device name */
    p_data[0] = name_len + 1;
    p_data[1] = GAP_AD_TYPE_COMPLETE_NAME;
    memcpy(p_data + 2, device_name, name_len);

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

/* ---------------- Add profiles ---------------- */

static void server_configure(void)
{
    uint16_t err;

    /* HRPS */
    uint16_t start_hdl_hr = 0;
    struct hrps_db_cfg hrps_cfg = {0};

    //hrps_cfg.features = HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT | HRPS_HR_MEAS_NTF_CFG_BIT;
	hrps_cfg.features = 0;
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

static void combined_process(void)
{
    update_sensor_value();

    if (!ctrl.connected) {
        LOG_DBG("Waiting for peer connection...\n");
        k_sem_take(&conn_sem, K_FOREVER);
        return;
    }

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

    err = gapm_configure(0, &gapm_cfg, &gapm_cbs, on_gapm_process_complete);
    if (err) {
        LOG_ERR("gapm_configure error %u", err);
        return -1;
    }

    LOG_DBG("Waiting for init...\n");
    k_sem_take(&init_sem, K_FOREVER);
    LOG_DBG("Init complete!\n");

    /* Share connection info */
    service_conn(&ctrl);

    /* Adding battery service */
    config_battery_service();

    /* Add both profiles */
    server_configure();

    /* Create an advertising activity */
    create_advertising();

    while (1) {
        k_sleep(K_SECONDS(1));
        combined_process();
        battery_process();
    }
}