/**
 ****************************************************************************************
 *
 * @file mesh_api.h
 *
 * @brief Header file for Mesh Stack Application Programming Interface
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef MESH_API_
#define MESH_API_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup MESH_API Native API
 * @ingroup MESH
 * @brief  Mesh Stack Native Application Programming Interface
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_COMMON Common
 * @ingroup MESH_API
 * @brief Mesh Common Application Programming Interface
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_COMMON_GENERAL General
 * @ingroup MESH_COMMON
 * @brief Mesh General Common Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_COMMON_RUN_TIME Run Time
 * @ingroup MESH_COMMON
 * @brief Mesh Common Run Time Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_COMMON_VERSION Version
 * @ingroup MESH_COMMON
 * @brief Mesh Common Version Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_COMMON_BUFFER Buffer
 * @ingroup MESH_COMMON
 * @brief Mesh Common Buffer Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE Profile
 * @ingroup MESH_API
 * @brief Mesh Profile Application Programming Interface
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_GENERAL General
 * @ingroup MESH_PROFILE
 * @brief Mesh General Profile Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_LPN LPN
 * @ingroup MESH_PROFILE
 * @brief Mesh Profile Low Power Node Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_PROXY Proxy
 * @ingroup MESH_PROFILE
 * @brief Mesh Profile Proxy Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_KEY Key
 * @ingroup MESH_PROFILE
 * @brief Mesh Profile Key Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_PROV Provisioning
 * @ingroup MESH_PROFILE
 * @brief Mesh Profile Configuration Client Model Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_CONFC Configuration Client
 * @ingroup MESH_PROFILE
 * @brief Mesh Profile Configuration Client Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_CONFS Configuration Server
 * @ingroup MESH_PROFILE
 * @brief Mesh Profile Configuration Server Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_HLTHC Health Client
 * @ingroup MESH_PROFILE
 * @brief Mesh Profile Health Client Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_HLTHS Health Server
 * @ingroup MESH_PROFILE
 * @brief Mesh Profile Health Server Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_MODEL Model
 * @ingroup MESH_API
 * @brief Mesh Model Application Programming Interface
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_MODEL_GENERAL General
 * @ingroup MESH_MODEL
 * @brief Mesh General Configuration Model Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_MODEL_SERVER Server
 * @ingroup MESH_MODEL
 * @brief Mesh Configuration Model Server Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_MODEL_CLIENT Client
 * @ingroup MESH_MODEL
 * @brief Mesh Configuration Model Client Related Structure
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mesh_defines.h"
#include "mesh_config.h"
#include "mesh_api_store.h"

/// @addtogroup MESH_API
/// @{
/// @addtogroup MESH_PROFILE
/// @{
/// @addtogroup MESH_PROFILE_GENERAL
/// @{

/*
 * ENUMERATIONS FOR MESH PROFILE
 ****************************************************************************************
 */

/// Mesh Supported Features
enum m_api_feat
{
     /// ------ COMPOSITION DATA -----

    /// Relay Node
    M_FEAT_RELAY_NODE_SUP       = (1 << 0),
    /// Proxy Node
    M_FEAT_PROXY_NODE_SUP       = (1 << 1),
    /// Friend Node
    M_FEAT_FRIEND_NODE_SUP      = (1 << 2),
    /// Low Power Node
    M_FEAT_LOW_POWER_NODE_SUP   = (1 << 3),

    /// Bit 4-15 RFU
    M_FEAT_RFU_LSB              = (4),
    M_FEAT_RFU_MASK             = (0xFFF0),

    /// ---------- INTERNAL ----------

    /// Message API supported
    M_FEAT_MSG_API_SUP          = (1 << 16),
    /// Provisioning over GATT
    M_FEAT_PB_GATT_SUP          = (1 << 17),
    /// Dynamic beacon interval supported
    M_FEAT_DYN_BCN_INTV_SUP     = (1 << 18),
    /// Provisioner role supported
    M_FEAT_PROVER_SUP           = (1 << 19),
    /// Provisionee role supported
    M_FEAT_PROVEE_SUP           = (1 << 20),
    /// Directed Forwarding Node feature supported
    M_FEAT_FWD_SUP              = (1 << 21),
    /// Directed Forwarding Configuration feature supported
    M_FEAT_FWD_CFG_SUP          = (1 << 22),
};

/// Model configuration
/// 7     1            0
/// +-----+------------+
/// | RFU | Publi Auth |
/// +-----+------------+
enum m_mdl_config
{
    /// Indicate if sending of publications is authorized or not
    M_MDL_CONFIG_PUBLI_AUTH_POS = 0,
    M_MDL_CONFIG_PUBLI_AUTH_BIT = 0x01,
};

/*
 * TYPES DEFINITION FOR MESH PROFILE
 ****************************************************************************************
 */

/// Mesh Profile Configuration Structure
typedef struct m_cfg
{
    /// Mask of supported features (see #m_api_feat enumeration)
    uint32_t features;
    /// 16-bit company identifier assigned by the Bluetooth SIG
    uint16_t cid;
    /// 16-bit vendor-assigned product identifier
    uint16_t pid;
    /// 16-bit vendor-assigned product version identifier
    uint16_t vid;
    /// Localization descriptor
    uint16_t loc;
    /// Number of addresses that can be stored for Message Replay Protection
    uint16_t nb_addr_replay;
    /// Number of pages in the Composition Data
    uint8_t  nb_cdata_page;

    /// Receive window in milliseconds when Friend feature is supported
    uint8_t frd_rx_window_ms;
    /// Queue size when Friend feature is supported
    uint8_t frd_queue_size;
    /// Subscription list size when Friend feature is supported
    uint8_t frd_subs_list_size;
} m_cfg_t;

/// Provisioning parameters
typedef struct m_prov_param
{
    /// Device UUID
    uint8_t dev_uuid[MESH_DEV_UUID_LEN];
    /// URI hash
    uint32_t uri_hash;
    /// OOB information
    uint16_t oob_info;
    /// Public key OOB information available
    uint8_t pub_key_oob;
    /// Static OOB information available
    uint8_t static_oob;
    /// Maximum size of Output OOB supported
    uint8_t out_oob_size;
    /// Maximum size in octets of Input OOB supported
    uint8_t in_oob_size;
    /// Supported Output OOB Actions (see #m_prov_out_oob enumeration)
    uint16_t out_oob_action;
    /// Supported Input OOB Actions (see #m_prov_in_oob enumeration)
    uint16_t in_oob_action;
    /// Bit field providing additional information (see #m_prov_info enumeration)
    uint8_t info;
} m_prov_param_t;

/**
 ****************************************************************************************
 * @brief Mesh Buffer, the value of the pointer must not be changed.
 *
 * It must be:
 * - Allocated through m_api_buf_alloc()
 * - Released with m_api_buf_release()
 ****************************************************************************************
 */
typedef void m_api_buf_t;

/*
 * CALLBACKS DEFINITION FOR MESH PROFILE
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Definition of callback function to call upon reception of a PDU for specific model identifier
 *
 * @param[in] model_lid    Model Local Identifier
 * @param[in] opcode       Operation code
 * @param[in] p_buf        Pointer to the buffer containing the message PDU. - No need to release buffer.
 * @param[in] app_key_lid  Application Key Local identifier (Required for a response)
 * @param[in] src          Source address of the message (Required for a response)
 * @param[in] rssi         Measured RSSI level for the received PDU.
 * @param[in] not_relayed  True if message have been received by an immediate peer; False, it can have been relayed
 ****************************************************************************************
 */
typedef void (*m_api_model_rx_cb)(m_lid_t model_lid, uint32_t opcode, m_api_buf_t* p_buf, m_lid_t app_key_lid,
                                  uint16_t src, int8_t rssi, bool not_relayed);

/**
 ****************************************************************************************
 * @brief Definition of callback function to call upon reception of a PDU to check that model can handle it
 *
 * @note m_api_model_opcode_status function must be used to provide information about opcode support
 *
 * @param[in] model_lid Model Local Identifier
 * @param[in] opcode    Operation code to check
 ****************************************************************************************
 */
typedef void (*m_api_model_opcode_check_cb)(m_lid_t model_lid, uint32_t opcode);

/**
 ****************************************************************************************
 * @brief Definition of callback function to call when publication parameters are updated
 *
 * @param[in] model_lid    Model Local Identifier
 * @param[in] addr         Publication Address
 * @param[in] period_ms    Publish period in milliseconds
 ****************************************************************************************
 */
typedef void (*m_api_model_publish_param_cb)(m_lid_t model_lid, uint16_t addr, uint32_t period_ms);

/**
 ****************************************************************************************
 * @brief Definition of callback function to call once PDU has been sent.
 *
 * @param[in] model_lid Model Local Identifier
 * @param[in] tx_hdl    Handle value configured by model when message has been requested to be sent
 * @param[in] p_buf     Pointer to the buffer containing the transmitted PDU. - Buffer must be released by model.
 * @param[in] status    Transmission status.
 ****************************************************************************************
 */
typedef void (*m_api_model_sent_cb)(m_lid_t model_lid, uint8_t tx_hdl, m_api_buf_t* p_buf, uint16_t status);

/**
 ****************************************************************************************
 * @brief Callback executed when mesh profile has been enabled
 *
 * @param[in] status    Execution status.
 * @param[in] prov      Indicate if node is provisioned or not
 ****************************************************************************************
 */
typedef void (*m_api_enabled_cb)(uint16_t status, bool prov);

/**
 ****************************************************************************************
 * @brief Callback executed when simple execution performed
 *
 * @param[in] status    Execution status.
 ****************************************************************************************
 */
typedef void (*m_api_end_cb)(uint16_t status);

/**
 ****************************************************************************************
 * @brief Callback used to inform that stored information have been loaded
 ****************************************************************************************
 */
typedef void (*m_api_storage_loaded_cb)(uint16_t status);

/**
 ****************************************************************************************
 * @brief Callback used to inform that updated information are required to be stored
 *
 * @param upd_type      Update type
 * @param length        Entry length
 * @param p_data        Pointer to entry data
 ****************************************************************************************
 */
typedef void (*m_api_storage_update_cb)(uint8_t upd_type, uint16_t length, uint8_t *p_data);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about start/end of attention timer
 ****************************************************************************************
 */
typedef void (*m_api_attention_cb)(uint8_t attention_state);

/**
 ****************************************************************************************
 * @brief Callback used to request a page of composition data to the application
 ****************************************************************************************
 */
typedef void (*m_api_compo_data_cb)(uint8_t page);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about received node reset request
 ****************************************************************************************
 */
typedef void (*m_api_node_reset_cb)(void);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about new or deleted group (application key)
 ****************************************************************************************
 */
typedef void (*m_api_group_update_cb)(bool added, uint16_t appkey_id, m_lid_t app_key_lid);

/**
 ****************************************************************************************
 * @brief Callback used to send a basic request indication message to the application
 ****************************************************************************************
 */
typedef void (*m_api_basic_req_cb)(uint32_t req_ind_code);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about a completed key action
 ****************************************************************************************
 */
typedef void (*m_api_key_gen_cb)(uint16_t status, m_lid_t key_lid, uint32_t cmd_code);

/// @} MESH_PROFILE_GENERAL

/// @addtogroup MESH_PROFILE_LPN
/// @{

/**
 ****************************************************************************************
 * @brief Callback used to inform application of friendship update as low power node
 ****************************************************************************************
 */
typedef void (*m_api_lpn_status_cb)(uint16_t status, uint16_t friend_addr);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about reception of a Friend Offer message
 ****************************************************************************************
 */
typedef void (*m_api_lpn_offer_cb)(uint16_t friend_addr, uint8_t rx_window,
                                   uint8_t queue_size, uint8_t subs_list_size, int8_t rssi);

/// @} MESH_PROFILE_LPN

/// @addtogroup MESH_PROFILE_PROXY
/// @{

/**
 ****************************************************************************************
 * @brief Callback used to inform application about reception of a Friend Offer message
 ****************************************************************************************
 */
typedef void (*m_api_proxy_adv_update_cb)(uint8_t state, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about status of proxy filter
 ****************************************************************************************
 */
typedef void (*m_api_proxy_filt_status_cb)(uint8_t filt_type, uint16_t list_size);

/// @} MESH_PROFILE_PROXY

/// @addtogroup MESH_PROFILE_PROV
/// @{

/**
 ****************************************************************************************
 * @brief Callback used to inform about a modification of the provisioning module state
 *
 * @param state    State of the provisioner   (see #m_prov_state enumeration)
 * @param status   Relevant only for provisioning failed (failed reason)
 ****************************************************************************************
 */
typedef void (*m_api_prov_state_cb)(uint8_t state, uint16_t status);

/**
 ****************************************************************************************
 * @brief Callback used to inform that provisioning parameters are required
 ****************************************************************************************
 */
typedef void (*m_api_prov_param_req_cb)(void);

/**
 ****************************************************************************************
 * @brief Callback used to inform that Out Of Band Authentication Data is required for provisioning
 *
 * @note Authentication data must be provided using #m_api_prov_oob_auth_rsp function
 *
 * @param auth_method  Authentication method (see #m_prov_auth_method enumeration)
 * @param auth_action  Authentication Action:
 *                     - M_PROV_AUTH_NO_OOB     = Prohibited
 *                     - M_PROV_AUTH_STATIC_OOB = 16 bytes LSB static out of band data required
 *                     - M_PROV_AUTH_OUTPUT_OOB = see #m_prov_out_oob enumeration, 1 bit set.
 *                     - M_PROV_AUTH_INPUT_OOB  = see #m_prov_in_oob enumeration, 1 bit set.
 * @param auth_size    expected authentication maximum data size
 ****************************************************************************************
 */
typedef void (*m_api_prov_oob_auth_req_cb)(uint8_t auth_method, uint16_t auth_action, uint8_t auth_size);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about reception of public keys
 ****************************************************************************************
 */
typedef void (*m_api_provee_pub_key_oob_cb)(uint8_t *pub_key_x, uint8_t *pub_key_y);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about reception of an unprovisioned device beacon
 ****************************************************************************************
 */
typedef void (*m_api_prover_node_found_cb)(uint32_t uri_hash, uint16_t oob_info,
        int8_t rssi, uint8_t* p_dev_uuid);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about reception of a connectable advertising packet
 ****************************************************************************************
 */
typedef void (*m_api_prover_node_found_gatt_cb)(uint16_t oob_info,
        int8_t rssi, uint8_t* p_dev_uuid, uint8_t* p_addr, uint8_t addr_type);

/**
 ****************************************************************************************
 * @brief Callback used to inform application that Scan procedure has been stopped
 ****************************************************************************************
 */
typedef void (*m_api_prover_scan_stopped_cb)(uint8_t reason);

/**
 ****************************************************************************************
 * @brief Callback used to inform application the Provisioning state
 ****************************************************************************************
 */
typedef void (*m_api_prover_state_cb)(uint8_t state, uint16_t status, uint16_t unicast_addr);

/**
 ****************************************************************************************
 * @brief Callback used to send a request indication of provisioner identity to application
 ****************************************************************************************
 */
typedef void (*m_api_prover_identify_cb)(uint8_t nb_elt, uint16_t algorithms, uint8_t pub_key_type,
    uint8_t static_oob_type, uint8_t out_oob_size, uint16_t out_oob_action, uint8_t in_oob_size, uint16_t in_oob_action);

/**
 ****************************************************************************************
 * @brief Callback used to inform application that a provisioner procedure is stopped
 ****************************************************************************************
 */
typedef void (*m_api_prover_stop_cb)(uint16_t status);

/// @} MESH_PROFILE_PROV

/// @addtogroup MESH_PROFILE_GENERAL
/// @{

/**
 ****************************************************************************************
 * @brief Callback used to request list of fault for primary element
 ****************************************************************************************
 */
typedef void (*m_api_fault_get_cb)(uint16_t comp_id);

/**
 ****************************************************************************************
 * @brief Callback used to request test of faults for primary element
 ****************************************************************************************
 */
typedef void (*m_api_fault_test_cb)(uint16_t comp_id, uint8_t test_id, bool cfm_needed);

/**
 ****************************************************************************************
 * @brief Callback used to inform application that fault status for primary element must be cleared
 ****************************************************************************************
 */
typedef void (*m_api_fault_clear_cb)(uint16_t comp_id);

/**
 ****************************************************************************************
 * @brief Callback used to inform application that fault period for primary element has been updated
 ****************************************************************************************
 */
typedef void (*m_api_fault_period_cb)(uint32_t period_ms, uint32_t period_fault_ms);

/// @} MESH_PROFILE_GENERAL

/// @addtogroup MESH_PROFILE_CONFC
/// @{

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received status value
 ****************************************************************************************
 */
typedef void (*m_api_confc_value_cb)(uint16_t addr, uint8_t value_code, uint8_t value);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received NetKey List
 ****************************************************************************************
 */
typedef void (*m_api_confc_netkey_list_cb)(uint16_t addr, uint8_t value_code, uint16_t* p_key_ids, uint16_t nb);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Network Transmit Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_net_tx_status_cb)(uint16_t addr, uint8_t value_code, uint8_t nb_tx, uint8_t intv_slots);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Relay Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_relay_status_cb)(uint16_t addr, uint8_t value_code, uint8_t relay, uint8_t nb_retx, uint8_t intv_slots);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Heartbeat Publication Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_hb_publi_status_cb)(uint16_t addr, uint8_t value_code, uint8_t status, uint16_t dst,
        uint8_t cnt_log, uint8_t period_log, uint8_t ttl, uint16_t features, uint16_t net_key_id);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Heartbeat Subscription Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_hb_subs_status_cb)(uint16_t addr, uint8_t value_code, uint8_t status, uint16_t src,
        uint16_t dst, uint8_t period_log, uint8_t cnt_log, uint8_t min_hops, uint8_t max_hops);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Network Key Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_netkey_status_cb)(uint16_t addr, uint8_t value_code, uint8_t status, uint16_t net_key_id);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Application Key List
 ****************************************************************************************
 */
typedef void (*m_api_confc_appkey_list_cb)(uint16_t addr, uint8_t value_code, uint8_t status, uint16_t net_key_id,
        uint16_t nb, uint16_t *p_appkey_ids);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Node Identity Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_node_id_status_cb)(uint16_t addr, uint8_t value_code, uint8_t status,
        uint16_t net_key_id, uint8_t node_id);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Node Reset Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_node_reset_status_cb)(uint16_t addr, uint8_t value_code);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Key Refresh Phase Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_key_refresh_phase_status_cb)(uint16_t addr, uint8_t value_code, uint8_t status,
        uint16_t net_key_id, uint8_t node_id);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Application Key Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_appkey_status_cb)(uint16_t addr, uint8_t value_code, uint8_t status, uint16_t net_key_id,
        uint16_t app_key_id);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Model Publication Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_mdl_pub_status_cb)(uint16_t addr, uint8_t value_code, uint8_t status, uint16_t elm_addr,
        uint16_t pub_addr, uint16_t app_key_id, bool cred_flag, uint8_t pub_ttl, uint32_t pub_period_ms,
        uint8_t pub_retx_cnt, uint8_t pub_retx_intv_slots, uint32_t mdl_id);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Model Subscription List
 ****************************************************************************************
 */
typedef void (*m_api_confc_mdl_subs_list_cb)(uint16_t addr, uint8_t value_code, uint8_t status, uint16_t elm_addr,
        uint32_t mdl_id, uint16_t nb, uint16_t* p_addr);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Model App List
 ****************************************************************************************
 */
typedef void (*m_api_confc_mdl_app_list_cb)(uint16_t addr, uint8_t value_code, uint8_t status, uint16_t elm_addr,
        uint32_t mdl_id, uint16_t nb, uint16_t* p_appkey_id);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Model Subscription status or Model App Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_mdl_subs_app_status_cb)(uint16_t addr, uint8_t value_code, uint8_t status, uint16_t elm_addr,
        uint16_t gaddr_appkey, uint32_t mdl_id);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received LPN Poll Timeout Status
 ****************************************************************************************
 */
typedef void (*m_api_confc_lpn_polltimeout_status_cb)(uint16_t addr, uint8_t value_code, uint16_t lpn_addr,
        uint32_t poll_timeout);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Composition Data (Page > 0)
 ****************************************************************************************
 */
typedef void (*m_api_confc_compo_data_cb)(uint16_t addr, uint8_t value_code, uint8_t page,
        uint16_t data_len, uint8_t* p_data);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Composition Data (Page 0)
 ****************************************************************************************
 */
typedef void (*m_api_confc_compo_data_page0_cb)(uint16_t addr, uint8_t value_code, uint16_t cid,
        uint16_t pid, uint16_t vid, uint16_t crpl, uint16_t features, uint8_t nb_elmts);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received Element field in Composition Data (Page 0)
 ****************************************************************************************
 */
typedef void (*m_api_confc_compo_data_elmt_cb)(uint16_t addr, uint8_t value_code, uint16_t loc,
        uint8_t nb_sig_mdls, uint8_t nb_ven_mdls, uint16_t* p_mdl_ids_sig, uint32_t* p_mdl_ids_ven);

/// @} MESH_PROFILE_CONFC

/// @addtogroup MESH_PROFILE_HLTHC
/// @{

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received status value
 ****************************************************************************************
 */
typedef void (*m_api_hlthc_value_cb)(uint16_t addr, uint8_t value_code, uint8_t value);

/**
 ****************************************************************************************
 * @brief Callback used to inform application about the received current status or fault status
 ****************************************************************************************
 */
typedef void (*m_api_hlthc_cur_fault_cb)(uint16_t addr, uint8_t value_code, uint8_t test_id, uint16_t comp_id,
        uint8_t nb_faults, uint8_t* p_faults);

/// @} MESH_PROFILE_HLTHC

/// @addtogroup MESH_PROFILE_FWDC
/// @{


/// @} MESH_PROFILE_FWDC

/// @addtogroup MESH_PROFILE_GENERAL
/// @{

/*
 * CALLBACK STRUCTURES FOR MESH PROFILE
 ****************************************************************************************
 */

/// Mesh Profile Callback Structure
typedef struct m_api_cb
{
    /// Callback executed at end of mesh enable request
    m_api_enabled_cb            cb_enabled;
    /// Callback executed at end of mesh disable request
    m_api_end_cb                cb_disabled;
    /// Callback used to inform application about provisioning state
    m_api_prov_state_cb         cb_prov_state;
    #if (BLE_MESH_PROVEE)
    /// Callback used to request provisioning parameters to the application
    m_api_prov_param_req_cb     cb_prov_param_req;
    #endif // (BLE_MESH_PROVEE)
    /// Callback used to request out of band authentication data
    m_api_prov_oob_auth_req_cb  cb_prov_auth_req;
    /// Callback used to inform that stored information have been loaded
    m_api_storage_loaded_cb     cb_loaded;
    /// Callback used to inform that updated information are required to be stored
    m_api_storage_update_cb     cb_storage_update;
    /// Callback used to inform application about start/end of attention timer
    m_api_attention_cb          cb_attention;
    /// Callback used to request a page of composition data to the application
    m_api_compo_data_cb         cb_compo_data;
    /// Callback used to inform application about received node reset request
    m_api_node_reset_cb         cb_node_reset;
    /// Callback used to inform application about new or deleted group (application key)
    m_api_group_update_cb       cb_group_update;
    #if (BLE_MESH_LPN)
    /// Callback used to inform application of friendship update as low power node
    m_api_lpn_status_cb         cb_lpn_status;
    /// Callback used to inform application about reception of a Friend Offer message
    m_api_lpn_offer_cb          cb_lpn_offer;
    #endif //(BLE_MESH_LPN)
    #if (BLE_MESH_GATT_PROXY)
    #if (BLE_MESH_GATT_PROXY_SVR)
    /// Callback used to inform application about state update of proxy advertising
    m_api_proxy_adv_update_cb   cb_proxy_adv_update;
    #endif //(BLE_MESH_GATT_PROXY_SVR)
    #if (BLE_MESH_GATT_PROXY_CLI)
    /// Callback used to inform application about status of the proxy filter
    m_api_proxy_filt_status_cb   cb_proxy_filt_status;
    #endif //(BLE_MESH_GATT_PROXY_CLI)
    #endif //(BLE_MESH_GATT_PROXY)
    #if (BLE_MESH_PROVEE)
    /// Callback used to inform application about reception of public keys
    m_api_provee_pub_key_oob_cb  cb_provee_pub_key_oob;
    #endif // (BLE_MESH_PROVEE)
    #if (BLE_MESH_PROVER)
    /// Callback used to inform application about reception of an unprovisioned device beacon
    m_api_prover_node_found_cb  cb_prover_scan_node_found;
    #if (BLE_MESH_GATT_PROV)
    /// Callback used to inform application about reception of a connectable advertising packet
    m_api_prover_node_found_gatt_cb  cb_prover_scan_node_found_gatt;
    #endif // (BLE_MESH_GATT_PROV)
    /// Callback used to inform application that a scan procedure is stopped
    m_api_prover_scan_stopped_cb cb_prover_scan_stopped;
    /// Callback used to inform application the Provisioning state
    m_api_prover_state_cb cb_prover_state;
    /// Callback used to send a basic request indication to application
    m_api_basic_req_cb cb_basic_req;
    /// Callback used to send a prover identify request indication to application
    m_api_prover_identify_cb cb_prover_identify;
    /// Callback used to inform application about a completed key action
    m_api_key_gen_cb cb_key_gen;
    #endif // (BLE_MESH_PROVER)
} m_api_cb_t;

/// Callback Structure for Health Model for primary element
typedef struct m_api_fault_cb
{
    /// Callback used to request list of fault for primary element
    m_api_fault_get_cb          cb_fault_get;
    /// Callback used to request test of faults for primary element
    m_api_fault_test_cb         cb_fault_test;
    /// Callback used to inform application that fault status for primary element must be cleared
    m_api_fault_clear_cb        cb_fault_clear;
    /// Callback used to inform application that fault period for primary element has been updated
    m_api_fault_period_cb       cb_fault_period;
} m_api_fault_cb_t;

/// Callback Structure for registered models
typedef struct m_api_model_cb
{
    /// Reception of a buffer for model
    m_api_model_rx_cb             cb_rx;
    /// Callback executed when a PDU is properly sent
    m_api_model_sent_cb           cb_sent;
    /// Check if model can handle operation code
    m_api_model_opcode_check_cb   cb_opcode_check;
    /// Callback function called when publication parameters are updated
    m_api_model_publish_param_cb  cb_publish_param;
} m_api_model_cb_t;

/// @} MESH_PROFILE_GENERAL

/// @addtogroup MESH_PROFILE_CONFC
/// @{

/// Callback Structure for Mesh Configuration Client Model
typedef struct m_api_confc_cb
{
    /// Reception of a 1 byte status value
    m_api_confc_value_cb cb_value;
    /// Reception of a NetKey List
    m_api_confc_netkey_list_cb cb_netkey_list;
    /// Reception of a Network Transmit Status
    m_api_confc_net_tx_status_cb cb_net_tx_status;
    /// Reception of a Relay Status
    m_api_confc_relay_status_cb cb_relay_status;
    /// Reception of a Heartbeat Publication Status
    m_api_confc_hb_publi_status_cb cb_hb_publi_status;
    /// Reception of a Heartbeat Subscription Status
    m_api_confc_hb_subs_status_cb cb_hb_subs_status;
    /// Reception of a Nerwork Key Status
    m_api_confc_netkey_status_cb cb_netkey_status;
    /// Reception of a Application Key List
    m_api_confc_appkey_list_cb cb_appkey_list;
    /// Reception of a Node identity Status
    m_api_confc_node_id_status_cb cb_node_id_status;
    /// Reception of a Node Reset Status
    m_api_confc_node_reset_status_cb cb_node_reset_status;
    /// Reception of a Key refresh phase Status
    m_api_confc_key_refresh_phase_status_cb cb_key_refresh_phase_status;
    /// Reception of a Application Key Status
    m_api_confc_appkey_status_cb cb_appkey_status;
    /// Reception of a Model Publication Status
    m_api_confc_mdl_pub_status_cb cb_mdl_pub_status;
    /// Reception of a Model Subscription List
    m_api_confc_mdl_subs_list_cb cb_mdl_subs_list;
    /// Reception of a Model App List
    m_api_confc_mdl_app_list_cb cb_mdl_app_list;
    /// Reception of a Model Subscription status or Model App status
    m_api_confc_mdl_subs_app_status_cb cb_mdl_subs_app_status;
    /// Reception of a LPN Poll Timeout Status
    m_api_confc_lpn_polltimeout_status_cb cb_lpn_polltimeout_status;
    /// Reception of a Composition Data status (Page > 0)
    m_api_confc_compo_data_cb cb_compo_data;
    /// Reception of a Composition Data status (Page 0)
    m_api_confc_compo_data_page0_cb cb_compo_data_page0;
    /// Reception of an Element field in Composition Data status (Page 0)
    m_api_confc_compo_data_elmt_cb cb_compo_data_elmt;
} m_api_confc_cb_t;

/// @} MESH_PROFILE_CONFC

/// @addtogroup MESH_PROFILE_HLTHC
/// @{

/// Callback Structure for Mesh Health Client Model
typedef struct m_api_hlthc_cb
{
    /// Reception of a 1 byte status value
    m_api_hlthc_value_cb cb_value;
    /// Reception of Health Current Status or Health Fault Status
    m_api_hlthc_cur_fault_cb cb_cur_fault;
} m_api_hlthc_cb_t;

/// @} MESH_PROFILE_HLTHC

/// @addtogroup MESH_PROFILE_FWDC
/// @{


/// @} MESH_PROFILE_FWDC

/// @addtogroup MESH_PROFILE_GENERAL
/// @{

/*
 * FUNCTIONS DEFINITION FOR MESH PROFILE
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Define the set of callback to communicate with mesh native API
 *
 * @param[in] p_cb_api          Native application callback set use to communicate with a native API
 * @param[in] p_fault_cb_api    Native application callback for Health Model in primary element
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t m_api_set(const m_api_cb_t *p_cb_api, const m_api_fault_cb_t *p_fault_cb_api);

/**
 ****************************************************************************************
 * @brief Enable Mesh profile
 *
 * @note cb_enabled() of m_api_cb_t called at end of enable execution
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t m_api_enable(void);

/**
 ****************************************************************************************
 * @brief Disable Mesh profile
 *
 * @note cb_disabled() of m_api_cb_t called at end of disable execution
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t m_api_disable(void);

/**
 ****************************************************************************************
 * @brief Allocate buffers command
 *
 * @param[out] pp_buf    Pointer to the buffer structure allocated
 * @param[in]  size      size of data required for the buffer
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t m_api_buf_alloc(m_api_buf_t** pp_buf, uint16_t size);

/**
 ****************************************************************************************
 * @brief Release allocate buffers. The buffer is free as soon as all reference to buffer are released.
 *
 * @param[in] p_buf    Pointer to the buffer structure
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t m_api_buf_release(m_api_buf_t* p_buf);

/**
 ****************************************************************************************
 * @brief Register a model
 *
 * @param[in] model_id          Model ID.
 * @param[in] elmt_idx          Index of element the model belongs to
 * @param[in] config            Configuration (see #m_mdl_config enumeration)
 * @param[in] p_cb              Pointer to callback functions defined by the model
 * @param[out] p_model_lid      Pointer to the variable that will contain the allocated Model LID.
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_register_model(uint32_t model_id, uint8_t elmt_idx, uint8_t config,
                              const m_api_model_cb_t *p_cb, m_lid_t *p_model_lid);

/**
 ****************************************************************************************
 * @brief Bind the application key with the model
 *
 * @param[in] app_key_lid    Application key local index
 * @param[in] mdl_lid        Model local index
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_bind_app_mdl(m_lid_t app_key_lid, m_lid_t mdl_lid);

/**
 ****************************************************************************************
 * @brief Unbind the application key with the model
 *
 * @param[in] app_key_lid    Application key local index
 * @param[in] mdl_lid        Model local index
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_unbind_app_mdl(m_lid_t app_key_lid, m_lid_t mdl_lid);

/**
 ****************************************************************************************
 * @brief Load stored information for storage manager
 *
 * @param[in] length    Length of stored information report
 * @param[in] p_data    Pointer to stored information report
 *
 * @return An handling status (see enum mesh_error)
 ****************************************************************************************
 */
uint16_t m_api_storage_load(uint16_t length, const uint8_t *p_data);

#if (!(0))
/**
 ****************************************************************************************
 * @brief Store information about mesh network
 *
 * @return              An handling status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_storage_save(void);
#endif //(!(0))

/**
 ****************************************************************************************
 * @brief Configure storage module
 *
 * @param[in] config    Config value depends on storage method
 * @return              An handling status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_storage_config(uint32_t config);

/**
 ****************************************************************************************
 * @brief Set IV update mode and ignore 96-hour limit
 *
 * @param[in] update    True if transition to IV Update in Progress state is required, False if
 * require to transit to Normal state
 *
 * @return An handling status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_iv_upd_test_mode(bool update);

/**
 ****************************************************************************************
 * @brief Let the model publish a message over mesh network
 *
 * @note Message status will be reported with model callback (see #m_api_model_sent_cb)
 *
 * @param[in] model_lid    Model Local ID
 * @param[in] opcode       Operation code of the message
 * @param[in] tx_hdl       Handle value used by model to retrieve which message has been sent
 * @param[in] p_buf        Pointer to the buffer structure that contains message to publish
 * @param[in] trans_mic_64 1 = Segmented PDU force transport MIC to 64 bits ; 0 = 32 bits transport MIC
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_model_publish( m_lid_t model_lid, uint32_t opcode, uint8_t tx_hdl, m_api_buf_t *p_buf, bool trans_mic_64);

/**
 ****************************************************************************************
 * @brief Let the model send a message over mesh network
 *
 * @note Message status will be reported with model callback (see #m_api_model_sent_cb)
 *
 * @param[in] model_lid    Model Local ID
 * @param[in] opcode       Operation code of the message
 * @param[in] tx_hdl       Handle value used by model to retrieve which message has been sent
 * @param[in] p_buf        Pointer to the buffer structure that contains message to publish
 * @param[in] key_lid      Key information.
 *                         If key_lid & 0x80 != 0, key_lid & 0x7F = network key local index
 *                         else key_lid & 0x7F = application key local index
 * @param[in] dst          Destination address of the message
 * @param[in] trans_mic_64 For a segmented PDU force transport mic to 64 bits
 * @param[in] not_relay    True, send message to an immediate peer; False, accept message to be relayed
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_model_send(m_lid_t model_lid, uint32_t opcode, uint8_t tx_hdl, m_api_buf_t *p_buf,
                          m_lid_t key_lid, uint16_t dst, bool trans_mic_64, bool not_relay);

/**
 ****************************************************************************************
 * @brief Reply to the Model operation code support (see #m_api_model_opcode_check_cb)
 *
 * @param[in] model_lid Model Local ID
 * @param[in] opcode    Operation code checked
 * @param[in] status    MESH_ERR_NO_ERROR if operation supported by model, other error code to reject
 *
 ****************************************************************************************
 */
void m_api_model_opcode_status(m_lid_t model_lid, uint32_t opcode, uint16_t status);

#if (BLE_MESH_FND_CLI)
/**
 ****************************************************************************************
 * @brief Let the Configuration Client model send a message over mesh network
 *
 * @note Message status will be reported with model callback (see #m_api_model_sent_cb)
 *
 * @param[in] model_lid    Model Lid
 * @param[in] opcode       Operation code of the message
 * @param[in] tx_hdl       Handle value used by model to retrieve which message has been sent
 * @param[in] p_buf        Pointer to the buffer structure that contains message to publish
 * @param[in] dev_key_lid  Device key local index.
 * @param[in] net_key_lid  Network key local index.
 * @param[in] dst          Destination address of the message
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_model_send_dev(m_lid_t model_lid, uint32_t opcode, uint8_t tx_hdl, m_api_buf_t *p_buf,
                          m_lid_t dev_key_lid, m_lid_t net_key_lid, uint16_t dst);

/// @} MESH_PROFILE_GENERAL

/// @addtogroup MESH_PROFILE_CONFC
/// @{

/**
 ****************************************************************************************
 * @brief Register the model
 *
 * @param[in] p_cb          Callback functions reporting received value to APP
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_confc_reg_mdl(const m_api_confc_cb_t * p_cb);

/**
 ****************************************************************************************
 * @brief Provide device configuration information
 *
 * @param[in] dev_key_lid          Device key local index
 * @param[in] net_key_lid          Network key local index
 * @param[in] addr                 Device primary address
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_confc_set_dev(m_lid_t dev_key_lid, m_lid_t net_key_lid, uint16_t addr);

/**
 ****************************************************************************************
 * @brief Configuration Client Model requests a Get action
 *
 * @param[in] get_type          Type of Get action request
 * @param[in] val               Value of the LPN address/Composition Data page number
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_confc_get(uint8_t get_type, uint16_t val);

/**
 ****************************************************************************************
 * @brief Configuration Client Model requests a network action
 *
 * @param[in] net_act_type             Type of network action to request
 * @param[in] net_key_id               Network key index
 * @param[in] p_data                   Network key information (NetKey/Transition/Identity)
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_confc_act_net(uint8_t net_act_type, uint16_t net_key_id, uint8_t* p_data);

/**
 ****************************************************************************************
 * @brief Configuration Client Model requests an application key action
 *
 * @param[in] app_act_type             Type of application key action to request
 * @param[in] net_key_id               Network key index
 * @param[in] app_key_id               Application key index
 * @param[in] p_app_key                Application key
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_confc_act_app(uint8_t app_act_type, uint16_t net_key_id, uint16_t app_key_id, uint8_t* p_app_key);

/**
 ****************************************************************************************
 * @brief Configuration Client Model requests a Model Get type action
 *
 * @param[in] mdl_get_type             Type of model get action to request
 * @param[in] elm_addr                 Address of the element
 * @param[in] mdl_id                   SIG Model ID or Vendor Model ID
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_confc_get_mdl(uint8_t mdl_get_type, uint16_t elm_addr, uint32_t mdl_id);

/**
 ****************************************************************************************
 * @brief Configuration Client Model requests a Model Subscription action
 *
 * @param[in] mdl_subs_act_type  Model Subscription action type value (see #m_fnd_confc_mdl_subs_action_type enumeration)
 * @param[in] elm_addr           Address of the element
 * @param[in] mdl_id             SIG Model ID (2 bytes) or Vendor Model ID (4 bytes)
 * @param[in] addr_type          0 for group address and 1 for Label UUID
 * @param[in] p_addr_uuid        Group address or Label UUID
 *
 * @return Execution status
 ****************************************************************************************
 */
uint16_t m_api_confc_act_mdl_subs(uint8_t mdl_subs_act_type, uint16_t elm_addr, uint32_t mdl_id, bool addr_type,
        void* p_addr_uuid);

/**
 ****************************************************************************************
 * @brief Configuration Client Model requests a Model Set action
 *
 * @param[in] set_type                 Type of Set action to request (see #m_fnd_confc_set_type enumeration)
 * @param[in] value                    Value of Beacon/TTL/Proxy/Friend/Relay
 * @param[in] tx_cnt                   Number of transmissions for each Network PDU originating from the node
 * @param[in] intv_slots               Number of 10-millisecond steps between transmissions
 *
 * @return Execution status
 ****************************************************************************************
 */
uint16_t m_api_confc_set(uint8_t set_type, uint8_t value, uint8_t tx_cnt, uint8_t intv_slots);

/**
 ****************************************************************************************
 * @brief Configuration Client Model requests a Heartbeat Publication Set action
 *
 * @param[in] dst               Destination address for Heartbeat messages
 * @param[in] cnt               Number of Heartbeat messages to be sent
 * @param[in] period_s          Period for sending Heartbeat messages
 * @param[in] ttl               TTL to be used when sending Heartbeat messages
 * @param[in] features          Bit field indicating features that trigger Heartbeat messages when changed
 * @param[in] netkey_id         NetKey Index
 *
 * @return Execution status
 ****************************************************************************************
 */
uint16_t m_api_confc_set_hb_publi(uint16_t dst, uint16_t cnt, uint16_t period_s, uint8_t ttl,
        uint16_t features, uint16_t netkey_id);

/**
 ****************************************************************************************
 * @brief Configuration Client Model requests a Heartbeat Subscription Set action
 *
 * @param[in] src               Source address for Heartbeat messages
 * @param[in] dst               Destination address for Heartbeat messages
 * @param[in] period_s          Period for sending Heartbeat messages
 *
 * @return Execution status
 ****************************************************************************************
 */
uint16_t m_api_confc_set_hb_subs(uint16_t src, uint16_t dst, uint16_t period_s);

/**
 ****************************************************************************************
 * @brief Configuration Client Model requests a Model AppKey action
 *
 * @param[in] mdl_app_act_type     Model AppKey action type value (see #m_fnd_confc_mdl_app_action_type enumeration)
 * @param[in] elm_addr             Element address
 * @param[in] app_key_id           Index of the AppKey
 * @param[in] mdl_id               SIG Model ID or Vendor Model ID
 *
 * @return Execution status
 ****************************************************************************************
 */
uint16_t m_api_confc_act_mdl_app(uint8_t mdl_app_act_type, uint16_t elm_addr, uint16_t app_key_id, uint32_t mdl_id);

/**
 ****************************************************************************************
 * @brief Configuration Client Model requests a Model Publication Set action
 *
 * @param[in] addr_type            Model Publication Set type value (see #m_fnd_confc_mdl_publi_set_type enumeration)
 * @param[in] elm_addr             Element address
 * @param[in] p_pub_addr           Value of the Publish address/Label UUID
 * @param[in] app_key_id           Index of the application key
 * @param[in] cred_flag            Value of the Friendship Credential Flag
 * @param[in] pub_ttl              Default TTL value for the outgoing messages
 * @param[in] pub_period           Period for periodic status publishing
 * @param[in] retx_cnt             Number of retransmissions for each published message
 * @param[in] retx_intv_slots      Number of 50-millisecond steps between retransmissions
 * @param[in] mdl_id               SIG Model ID or Vendor Model ID
 *
 * @return Execution status
 ****************************************************************************************
 */
uint16_t m_api_confc_set_mdl_publi(uint8_t addr_type, uint16_t elm_addr, uint8_t* p_pub_addr, uint16_t app_key_id,
        bool cred_flag, uint8_t pub_ttl, uint8_t pub_period, uint8_t retx_cnt, uint8_t retx_intv_slots, uint32_t mdl_id);

/// @} MESH_PROFILE_CONFC

/// @addtogroup MESH_PROFILE_HLTHC
/// @{

/**
 ****************************************************************************************
 * @brief Register the model
 *
 * @param[in] p_cb          Callback functions reporting received value to APP
 * @param[in] p_mdl_lid     Pointer to generated model local index
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_hlthc_reg_mdl(const m_api_hlthc_cb_t * p_cb, m_lid_t* p_mdl_lid);

/**
 ****************************************************************************************
 * @brief Health Client Model requests a Get type action
 *
 * @param[in] addr                 Destination address of this message
 * @param[in] app_key_lid          Application Key bound with the model
 * @param[in] get_type             Get type value (see #m_fnd_hlthc_get_type enumeration)
 * @param[in] comp_id              Company identifier
 *
 * @return Execution status
 ****************************************************************************************
 */
uint16_t m_api_hlthc_get(uint16_t addr, m_lid_t app_key_lid, uint8_t get_type, uint16_t comp_id);

/**
 ****************************************************************************************
 * @brief Health Client Model requests a Set type action
 *
 * @param[in] addr                 Destination address of this message
 * @param[in] app_key_lid          Application Key bound with the model
 * @param[in] set_type             Type of Set action to request
 * @param[in] set_cfg              Set Configuration bit field to indicate if it's acknowledged
 * @param[in] val                  Fast Period Divisor or Attention value
 *
 * @return Execution status
 ****************************************************************************************
 */
uint16_t m_api_hlthc_set(uint16_t addr, m_lid_t app_key_lid, uint8_t set_type, uint8_t set_cfg, uint8_t val);

/**
 ****************************************************************************************
 * @brief Health Client Model requests a Fault action
 *
 * @param[in] addr                 Destination address of this message
 * @param[in] app_key_lid          Application Key bound with the model
 * @param[in] action_type          Type of Fault action to request
 * @param[in] action_cfg           Fault action Configuration bit field to indicate if it's acknowledged
 * @param[in] test_id              Identifier of a specific test to be performed (not used for Fault Clear)
 * @param[in] comp_id              Company identifier
 *
 * @return Execution status
 ****************************************************************************************
 */
uint16_t m_api_hlthc_act_fault(uint16_t addr, m_lid_t app_key_lid, uint8_t action_type, uint8_t action_cfg,
        uint8_t test_id, uint16_t comp_id);

/// @} MESH_PROFILE_HLTHC

/// @addtogroup MESH_PROFILE_FWDC
/// @{


/// @} MESH_PROFILE_FWDC

#endif // (BLE_MESH_FND_CLI)

/// @addtogroup MESH_PROFILE_CONFS
/// @{

/**
 ****************************************************************************************
 * @brief Provide composition data
 *
 * @param[in] page      Page of composition data
 * @param[in] length    Page length
 * @param[in] p_data    Pointer to page content
 ****************************************************************************************
 */
void m_api_compo_data_cfm(uint8_t page, uint8_t length, uint8_t *p_data);

/// @} MESH_PROFILE_CONFS

/// @addtogroup MESH_PROFILE_HLTHS
/// @{

/**
 ****************************************************************************************
 * @brief Provide fault status for primary element
 *
 * @param[in] comp_id           Company ID
 * @param[in] test_id           Test ID
 * @param[in] length            Length of fault array
 * @param[in] p_fault_array     Pointer to the fault array
 ****************************************************************************************
 */
void m_api_health_status_send(uint16_t comp_id, uint8_t test_id, uint8_t length,
                              uint8_t *p_fault_array);

/**
 ****************************************************************************************
 * @brief Provide fault status for primary element
 *
 * @param[in] accept            Request accept
 * @param[in] comp_id           Company ID
 * @param[in] test_id           Test ID
 * @param[in] length            Length of fault array
 * @param[in] p_fault_array     Pointer to the fault array
 ****************************************************************************************
 */
void m_api_health_cfm(bool accept, uint16_t comp_id, uint8_t test_id, uint8_t length,
                      uint8_t *p_fault_array);

/// @} MESH_PROFILE_HLTHS

/// @addtogroup MESH_PROFILE_PROV
/// @{

/**
 ****************************************************************************************
 * @brief Provide authentication data to the provisioning module
 *
 * @param[in] accept      True, Accept pairing request, False reject
 * @param[in] auth_size   Authentication data size (<= requested size else pairing automatically rejected)
 * @param[in] p_auth_data Authentication data (LSB for a number or array of bytes)
 ****************************************************************************************
 */
void m_api_prov_oob_auth_rsp(bool accept, uint8_t auth_size, const uint8_t* p_auth_data);

#if (BLE_MESH_PROVEE)
/**
 ****************************************************************************************
 * @brief Provide provisioning parameters to the provisioning module
 *
 * @param[in] p_param      Provisioning parameters
 ****************************************************************************************
 */
void m_api_prov_param_rsp(const m_prov_param_t *p_param);

/**
 ****************************************************************************************
 * @brief Get the local public key for out of band transmission of local public key
 *
 * @param[out] p_pub_key_x   X Coordinate of public Key (32 bytes LSB)
 * @param[out] p_pub_key_y   Y Coordinate of public Key (32 bytes LSB)
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_prov_pub_key_read(uint8_t* p_pub_key_x, uint8_t* p_pub_key_y);
#endif // (BLE_MESH_PROVEE)

/// @} MESH_PROFILE_PROV

/// @addtogroup MESH_PROFILE_LPN
/// @{

#if (BLE_MESH_LPN)
/**
 ****************************************************************************************
 * @brief Enable Low Power Node feature and start looking for an available Friend node in
 * the neighborhood.
 *
 * @param[in] poll_timeout          Initial value of PollTimeout timer
 * @param[in] poll_intv_ms          Poll interval in milliseconds
 * @param[in] prev_addr             Previous address
 * @param[in] rx_delay              Requested receive delay
 * @param[in] rssi_factor           RSSI factor
 * @param[in] rx_window_factor      Receive window factor
 * @param[in] min_queue_size_log    Requested minimum number of messages that the Friend node can store in its Friend Queue
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_lpn_start(uint32_t poll_timeout, uint32_t poll_intv_ms, uint16_t prev_addr, uint8_t rx_delay,
                         uint8_t rssi_factor, uint8_t rx_window_factor, uint8_t min_queue_size_log);

/**
 ****************************************************************************************
 * @brief Disable Low Power Node feature
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_lpn_stop(void);

/**
 ****************************************************************************************
 * @brief Select a friend after reception of one or several Friend Offer messages.
 *
 * @param[in] friend_addr       Address of the selected Friend node.
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_lpn_select_friend(uint16_t friend_addr);
#endif //(BLE_MESH_LPN)

/// @} MESH_PROFILE_LPN

/// @addtogroup MESH_PROFILE_PROXY
/// @{

#if (BLE_MESH_GATT_PROXY)
#if (BLE_MESH_GATT_PROXY_SVR)
/**
 ****************************************************************************************
 * @brief Control if Proxy service should start / stop advertising it's capabilities
 *
 * @param[in] enable  True to enable advertising for 60s, False to stop advertising
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_proxy_ctrl(uint8_t enable);
#endif // (BLE_MESH_GATT_PROXY_SVR)

#if (BLE_MESH_GATT_PROXY_CLI)
/**
 ****************************************************************************************
 * @brief Enable Proxy client role
 *
 * @param[in] conidx  Connection index
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_proxy_cli_enable(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Send Set Filter Type Message
 *
 * @param[in] conidx            Connection index
 * @param[in] filt_type         Proxy filter type
 * @param[in] net_key_lid       Network Key Local index used for encryption
 * @param[in] dst               Destination address of the message
 *
 * @return Execution status code
 ****************************************************************************************
 */
uint16_t m_api_proxy_cli_set_filt_type(uint8_t conidx, uint8_t filt_type, m_lid_t net_key_lid, uint16_t dst);

/**
 ****************************************************************************************
 * @brief Send Add Addresses or Remove Addresses messages
 *
 * @param[in] conidx            Connection index
 * @param[in] add_rem           True for add and false for remove
 * @param[in] nb_addr           Number of addresses to add or remove
 * @param[in] p_addr_list       Address list
 * @param[in] net_key_lid       Network Key Local index used for encryption
 * @param[in] dst               Destination address of the message
 *
 * @return Execution status, MESH_ERR_NO_ERROR if succeed
 ****************************************************************************************
 */
uint16_t m_api_proxy_cli_act_addresses(uint8_t conidx, bool add_rem, uint16_t nb_addr, uint16_t* p_addr_list,
        m_lid_t net_key_lid, uint16_t dst);
#endif // (BLE_MESH_GATT_PROXY_CLI)
#endif // (BLE_MESH_GATT_PROXY)

/// @} MESH_PROFILE_PROXY

/// @addtogroup MESH_PROFILE_KEY
/// @{

#if (BLE_MESH_PROVER)
/**
 ****************************************************************************************
 * @brief Add a new device key
 *
 * @param[in] key  Device key to add
 * @param[in] addr Address bound to the device key
 * @param[out] p_dev_key_lid  Device key local identifier
 *
 * @return Execution status code (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_key_dev_add(uint8_t *key, uint16_t addr, m_lid_t *p_dev_key_lid);

/**
 ****************************************************************************************
 * @brief Delete a device key
 *
 * @param[in] dev_key_id     Device key index
 *
 * @return Execution status code (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_key_dev_delete(uint16_t dev_key_id);

/**
 ****************************************************************************************
 * @brief Delete a network key if found
 *
 * @param[in] net_key_id     Network key index
 *
 * @return Execution status code (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_key_net_delete(uint16_t net_key_id);

/**
 ****************************************************************************************
 * @brief Delete a application key if found
 *
 * @param[in] net_key_id     Network key index
 * @param[in] app_key_id     Application key index
 *
 * @return Execution status code (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_key_app_delete(uint16_t net_key_id, uint16_t app_key_id);

/**
 ****************************************************************************************
 * @brief Use new key for transmission
 *
 * @param[in] net_key_id     Network key index
 *
 * @return Execution status code (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_key_use_new(uint16_t net_key_id);

/**
 ****************************************************************************************
 * @brief Revoke the old key
 *
 * @param[in] net_key_id     Network key index
 *
 * @return Execution status code (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_key_revoke_old(uint16_t net_key_id);

/// @} MESH_PROFILE_KEY

/// @addtogroup MESH_PROFILE_PROV
/// @{

/**
 ****************************************************************************************
 * @brief Scan for unprovisioned device beacons
 *
 * @param[in] timeout_s         Scan timeout in seconds
 * @param[in] cfg_bf            Configuration bie field
 * @return Execution status code (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_prover_scan(uint16_t timeout_s, uint8_t cfg_bf);

/**
 ****************************************************************************************
 * @brief Stop a provisioner procedure
 * @param[in] end_cb            Callback function executed at end of stop procedure
 * @return Execution status code (see #mesh_error enumeration)
 ****************************************************************************************
 */

uint16_t m_api_prover_stop(m_api_prover_stop_cb end_cb);

/**
 ****************************************************************************************
 * @brief Invite an unprovisioned node
 *
 * @param[in] conidx        Connection index
 * @param[in] p_uuid            Pointer to unprovisioned device UUID
 * @param[in] attention_dur_s   Attention duration in seconds
 * @return Execution status code (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_prover_invite(uint8_t conidx, uint8_t* p_uuid, uint8_t attention_dur_s);

/**
 ****************************************************************************************
 * @brief Provide provisioner configuration for start procedure
 *
 * @param[in] accept        Accept (true) or reject (false) the request
 * @param[in] net_key_lid   Network key local index
 * @param[in] addr          Unicast address assigned for the node
 * @param[in] algo          The algorithm used for provisioning
 * @param[in] pub_key       Public Key used
 * @param[in] auth_method   Authentication Method used
 * @param[in] auth_action   Selected Output OOB Action or Input OOB Action or 0x00
 * @param[in] auth_size     Size of the Output OOB used or size of the Input OOB used or 0x00
 ****************************************************************************************
 */
void m_api_prover_identify(bool accept, m_lid_t net_key_lid, uint16_t addr, uint8_t algo, uint8_t pub_key,
                           uint8_t auth_method, uint8_t auth_action, uint8_t auth_size);

/**
 ****************************************************************************************
 * @brief Provide device public key
 *
 * @param[in] accept          Accept (true) or reject (false) the request
 * @param[in] p_pub_key_x     The X component of public key for the FIPS P-256 algorithm
 * @param[in] p_pub_key_y     The Y component of public key for the FIPS P-256 algorithm
 ****************************************************************************************
 */
void m_api_prover_pub_key_oob(bool accept, uint8_t *p_pub_key_x, uint8_t *p_pub_key_y);

#endif // (BLE_MESH_PROVER)

/// @} MESH_PROFILE_PROV

/// @addtogroup MESH_PROFILE_KEY
/// @{

/**
 ****************************************************************************************
 * @brief Add a network key
 *
 * @param[in] net_key_id          Network key index
 * @param[in] p_net_key           Pointer to the 128bits key
 * @param[in] flags               Flags value
 *
 * @return Execution status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_key_net_add(uint16_t net_key_id, const uint8_t *p_net_key, uint8_t flags);
/**
 ****************************************************************************************
 * @brief Add an application key
 *
 * @param[in] net_key_id  Network key index bind to the new application key
 * @param[in] app_key_id  Application key index to add
 * @param[in] p_app_key   Pointer to the 128bits key
 *
 * @return Execution status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_key_app_add(uint16_t net_key_id, uint16_t app_key_id, const uint8_t *p_app_key);
/**
 ****************************************************************************************
 * @brief Update a network key
 *
 * @param[in] net_key_id          Network key index
 * @param[in] p_net_key           Pointer to the 128bits key
 *
 * @return Execution status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_key_net_update(uint16_t net_key_id, const uint8_t *p_net_key);
/**
 ****************************************************************************************
 * @brief Update a application key
 *
 * @param[in] net_key_id    Network key index
 * @param[in] app_key_id    Application key index
 * @param[in] p_net_key     Pointer to the 128bits key
 *
 * @return Execution status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t m_api_key_app_update(uint16_t net_key_id, uint16_t app_key_id, const uint8_t *p_net_key);

/// @} MESH_PROFILE_KEY
/// @} MESH_PROFILE

/// @addtogroup MESH_MODEL
/// @{
/// @addtogroup MESH_MODEL_GENERAL
/// @{

/*
 * ENUMERATIONS FOR MESH MODELS
 ****************************************************************************************
 */

/// Model Configuration Index
enum mm_cfg_idx
{
    /// ************ Generic Server ************

    MM_CFG_IDX_GENS_MIN                 = 0,
    /// Generic OnOff Server
    MM_CFG_IDX_GENS_ONOFF               = MM_CFG_IDX_GENS_MIN,
    /// Generic Level Server
    MM_CFG_IDX_GENS_LEVEL,
    /// Generic Default Transition Time
    MM_CFG_IDX_GENS_DFT_TRANS_TIME,
    /// Generic Power OnOff
    MM_CFG_IDX_GENS_POWER_ONOFF,
    /// Generic Power Level
    MM_CFG_IDX_GENS_POWER_LEVEL,
    /// Generic Battery
    MM_CFG_IDX_GENS_BATTERY,
    /// Generic Location
    MM_CFG_IDX_GENS_LOCATION,
    MM_CFG_IDX_GENS_MAX                 = MM_CFG_IDX_GENS_LOCATION,

    /// ************ Lighting Server ***********

    MM_CFG_IDX_LIGHTS_MIN               = 50,
    /// Light Lightness Server
    MM_CFG_IDX_LIGHTS_LN                = MM_CFG_IDX_LIGHTS_MIN,
    /// Light CTL Server
    MM_CFG_IDX_LIGHTS_CTL,
    /// Light HSL Server
    MM_CFG_IDX_LIGHTS_HSL,
    MM_CFG_IDX_LIGHTS_MAX               = MM_CFG_IDX_LIGHTS_HSL,

    /// Number of known configuration
    MM_CFG_IDX_NB,
};

/// Client Model Index
enum mm_cmdl_idx
{
    /// ************ Generic ************

    MM_CMDL_IDX_GENC_MIN                 = 0,
    /// Generic OnOff Client
    MM_CMDL_IDX_GENC_ONOFF               = MM_CMDL_IDX_GENC_MIN,
    /// Generic Level Client
    MM_CMDL_IDX_GENC_LEVEL,
    /// Generic Default Transition Time Client
    MM_CMDL_IDX_GENC_DFT_TRANS_TIME,
    /// Generic Power OnOff Client
    MM_CMDL_IDX_GENC_POWER_ONOFF,
    /// Generic Power Level Client
    MM_CMDL_IDX_GENC_POWER_LEVEL,
    /// Generic Battery Client
    MM_CMDL_IDX_GENC_BATTERY,
    /// Generic Location Client
    MM_CMDL_IDX_GENC_LOCATION,
    /// Generic Property Client
    MM_CMDL_IDX_GENC_PROPERTY,
    MM_CMDL_IDX_GENC_MAX                 = MM_CMDL_IDX_GENC_PROPERTY,

    /// ************ Lighting ***********

    MM_CMDL_IDX_LIGHTC_MIN               = 50,
    /// Light Lightness Client
    MM_CMDL_IDX_LIGHTC_LN                = MM_CMDL_IDX_LIGHTC_MIN,
    /// Light CTL Client
    MM_CMDL_IDX_LIGHTC_CTL,
    /// Light HSL Client
    MM_CMDL_IDX_LIGHTC_HSL,
    /// Light xyL Client
    MM_CMDL_IDX_LIGHTC_XYL,
    MM_CMDL_IDX_LIGHTC_MAX               = MM_CMDL_IDX_LIGHTC_XYL,

    /// Number of known client models
    MM_CMDL_IDX_NB,
};

/// Transition type
enum mm_trans_type
{
    /// Classic Set
    MM_TRANS_TYPE_CLASSIC = 0,
    /// Delta Set
    MM_TRANS_TYPE_DELTA,
    /// Move Set
    MM_TRANS_TYPE_MOVE,
    /// No transition
    MM_TRANS_TYPE_NONE,
};

/// Set information bit field)
/// 8     7     0
/// +-----+------+
/// | Ack | Type |
/// +-----+------+
enum mm_set_info
{
    /// Type (value depends on model for which operation is required)
    MM_SET_INFO_TYPE_LSB = 0,
    MM_SET_INFO_TYPE_MASK = 0x7F,

    /// Set or Set Unacknowledged
    MM_SET_INFO_ACK_POS = 7,
    MM_SET_INFO_ACK_BIT = 0x80,
};

/// Transition information bit field)
/// 16    8     7      6     2      0
/// +-----+-----+------+-----+------+
/// | TID | Ack | Long | RFU | Type |
/// +-----+-----+------+-----+------+
enum mm_trans_info
{
    /// Type (value depends on model for which transition is required)
    /// see #mm_trans_type enumeration
    MM_TRANS_INFO_TYPE_LSB = 0,
    MM_TRANS_INFO_TYPE_MASK = 0x0003,

    /// Include transition time and delay
    MM_TRANS_INFO_LONG_POS = 6,
    MM_TRANS_INFO_LONG_BIT = 0x0040,

    /// Set or Set Unacknowledged
    MM_TRANS_INFO_ACK_POS = 7,
    MM_TRANS_INFO_ACK_BIT = 0x0080,

    /// TID
    MM_TRANS_INFO_TID_LSB = 8,
    MM_TRANS_INFO_TID_MASK = 0xFF00,
};

/// Request indication codes
enum mm_req_ind_code
{
    /// Request battery information for a given element
    MM_API_SRV_BAT_REQ_IND = 0,
    /// Request Generic Location state for a given element (global part)
    MM_API_SRV_LOCG_REQ_IND,
    /// Request Generic Location state for a given element (local part)
    MM_API_SRV_LOCL_REQ_IND,
    /// Request to get Generic Property value
    MM_API_SRV_PROP_GET_REQ_IND,
    /// Request to set Generic Property value
    MM_API_SRV_PROP_SET_REQ_IND,

    /// Request start of a new transition to the main model
    MM_API_GRP_TRANS_REQ_IND = 50,
};

/// Get type values for Generic Power Level Client model
enum mm_get_type_plvl
{
    /// Get Generic Power Actual state value
    MM_GET_TYPE_PLVL_ACTUAL = 0,
    /// Get Generic Power Last state value
    MM_GET_TYPE_PLVL_LAST,
    /// Get Generic Power Default state value
    MM_GET_TYPE_PLVL_DFLT,
    /// Get Generic Power Range state value
    MM_GET_TYPE_PLVL_RANGE,

    /// Last option value
    MM_GET_TYPE_PLVL_MAX = MM_GET_TYPE_PLVL_RANGE,
};

/// Get type values for Generic Location Client model
enum mm_get_type_loc
{
    /// Get Generic Location Global state value
    MM_GET_TYPE_LOC_GLOBAL = 0,
    /// Get Generic Power Last state value
    MM_GET_TYPE_LOC_LOCAL,

    /// Last option value
    MM_GET_TYPE_LOC_MAX = MM_GET_TYPE_LOC_LOCAL,
};

/// Get type values for Generic Property Client model
enum mm_get_type_prop
{
    /// Send Generic User Properties Get message
    MM_GET_TYPE_PROP_UPROPS = 0,
    /// Send Generic User Property Get message
    MM_GET_TYPE_PROP_UPROP,
    /// Send Generic Admin Properties Get message
    MM_GET_TYPE_PROP_APROPS,
    /// Send Generic Admin Property Get message
    MM_GET_TYPE_PROP_APROP,
    /// Send Generic Manufacturer Properties Get message
    MM_GET_TYPE_PROP_MPROPS,
    /// Send Generic Manufacturer Property Get message
    MM_GET_TYPE_PROP_MPROP,
    /// Send Generic Client Properties Get message
    MM_GET_TYPE_PROP_CPROPS,

    /// Last option value
    MM_GET_TYPE_PROP_MAX = MM_GET_TYPE_PROP_CPROPS,
};

/// Get type values for Light Lightness Client model
enum mm_get_type_light_ln
{
    /// Get Light Lightness state value
    MM_GET_TYPE_LIGHT_LN_ACTUAL = 0,
    /// Get Light Lightness Linear state value
    MM_GET_TYPE_LIGHT_LN_LINEAR,
    /// Get Light Lightness Default state value
    MM_GET_TYPE_LIGHT_LN_DFLT,
    /// Get Light Lightness Last state value
    MM_GET_TYPE_LIGHT_LN_LAST,
    /// Get Light Lightness Range state value
    MM_GET_TYPE_LIGHT_LN_RANGE,

    /// Last option value
    MM_GET_TYPE_LIGHT_LN_MAX = MM_GET_TYPE_LIGHT_LN_RANGE,
};

/// Get type values for Light CTL Client model
enum mm_get_type_light_ctl
{
    /// Get Light CTL Lightness and Light CTL Temperature state value
    MM_GET_TYPE_LIGHT_CTL = 0,
    /// Get Light CTL Temperature and Light CTL Delta UV state value
    MM_GET_TYPE_LIGHT_CTL_TEMP,
    /// Get Light CTL Temperature Range state value
    MM_GET_TYPE_LIGHT_CTL_TEMP_RANGE,
    /// Get Light Lightness Default and Light CTL Temperature Default and Light CTL
    /// Delta UV Default state values
    MM_GET_TYPE_LIGHT_CTL_DFLT,

    /// Last option value
    MM_GET_TYPE_LIGHT_CTL_MAX = MM_GET_TYPE_LIGHT_CTL_DFLT,
};

/// Get type values for Light HSL Client model
enum mm_get_type_light_hsl
{
    /// Get Light HSL Lightness and Light HSL Hue and Light HSL Saturation state values
    MM_GET_TYPE_LIGHT_HSL = 0,
    /// Get Light HSL Hue state value
    MM_GET_TYPE_LIGHT_HSL_HUE,
    /// Get Light HSL Saturation state value
    MM_GET_TYPE_LIGHT_HSL_SAT,
    /// Get Light HSL Lightness and Light HSL Hue and Light HSL Saturation target state values
    MM_GET_TYPE_LIGHT_HSL_TGT,
    /// Get Light Lightness and Light HSL Hue and Light HSL Saturation default state values
    MM_GET_TYPE_LIGHT_HSL_DFLT,
    /// Get Light HSL Hue and Light HSL Saturation state range values
    MM_GET_TYPE_LIGHT_HSL_RANGE,

    /// Last option value
    MM_GET_TYPE_LIGHT_HSL_MAX = MM_GET_TYPE_LIGHT_HSL_RANGE,
};

/// Get type values for Light xyL Client model
enum mm_get_type_light_xyl
{
    /// Get Light xyL Lightness and Light xyL x and Light xyL y state values
    MM_GET_TYPE_LIGHT_XYL = 0,
    /// Get Light xyL Lightness and Light xyL x and Light xyL y state target values
    MM_GET_TYPE_LIGHT_XYL_TGT,
    /// Get Light Lightness and Light xyL x and Light xyL y state default values
    MM_GET_TYPE_LIGHT_XYL_DFLT,
    /// Get Light xyL x and Light xyL y state range values
    MM_GET_TYPE_LIGHT_XYL_RANGE,

    /// Last option value
    MM_GET_TYPE_LIGHT_XYL_MAX = MM_GET_TYPE_LIGHT_XYL_RANGE,
};

/// Set type values for the Generic Power Level Client model
enum mm_set_type_plvl
{
    /// Set Generic Power Default state value
    MM_SET_TYPE_PLVL_DFLT = 0,
    /// Set Generic Power Range state value
    MM_SET_TYPE_PLVL_RANGE,

    /// Last option value
    MM_SET_TYPE_PLVL_MAX = MM_SET_TYPE_PLVL_RANGE,
};

/// Set type values for the Light Lightness Client model
enum mm_set_type_light_ln
{
    /// Set Light Lightness Default state value
    MM_SET_TYPE_LIGHT_LN_DFLT = 0,
    /// Set Light Lightness Range state value
    MM_SET_TYPE_LIGHT_LN_RANGE,

    /// Last option value
    MM_SET_TYPE_LIGHT_LN_MAX = MM_SET_TYPE_LIGHT_LN_RANGE,
};

/// Set type values for the Light CTL Client model
enum mm_set_type_light_ctl
{
    /// Set Light CTL Temperature Range state value
    MM_SET_TYPE_LIGHT_CTL_TEMP_RANGE = 0,
    /// Set Light CTL Default state value
    MM_SET_TYPE_LIGHT_CTL_DFLT,

    /// Last option value
    MM_SET_TYPE_LIGHT_CTL_MAX = MM_SET_TYPE_LIGHT_CTL_DFLT,
};

/// Set type values for the Light HSL Client model
enum mm_set_type_light_hsl
{
    /// Set Light HSL Hue and Light HSL Saturation state range values
    MM_SET_TYPE_LIGHT_HSL_RANGE = 0,
    /// Set Light Lightness and Light HSL Hue and Light HSL Saturation default state values
    MM_SET_TYPE_LIGHT_HSL_DFLT,

    /// Last option value
    MM_SET_TYPE_LIGHT_HSL_MAX = MM_SET_TYPE_LIGHT_HSL_DFLT,
};

/// Set type values for the Light xyL Client model
enum mm_set_type_light_xyl
{
    /// Set Light xyL x and Light xyL y state range values
    MM_SET_TYPE_LIGHT_XYL_RANGE = 0,
    /// Set Light Lightness and Light xyL x and Light xyL y state default values
    MM_SET_TYPE_LIGHT_XYL_DFLT,

    /// Last option value
    MM_SET_TYPE_LIGHT_XYL_MAX = MM_SET_TYPE_LIGHT_XYL_DFLT,
};

/// Transition type values for the Light Lightness Client model
enum mm_trans_type_light_ln
{
    /// Set Light Lightness state value
    MM_TRANS_TYPE_LIGHT_LN = 0,
    /// Set Light Lightness Linear state value
    MM_TRANS_TYPE_LIGHT_LN_LIN,

    /// Last option value
    MM_TRANS_TYPE_LIGHT_LN_MAX = MM_TRANS_TYPE_LIGHT_LN_LIN,
};

/// Transition type values for the Light CTL Client model
enum mm_trans_type_light_ctl
{
    /// Set Light CTL Lightness and Light CTL Temperature and Light CTL Delta UV state values
    MM_TRANS_TYPE_LIGHT_CTL = 0,
    /// Set Light CTL Temperature and Light CTL Delta UV state values
    MM_TRANS_TYPE_LIGHT_CTL_TEMP,

    /// Last option value
    MM_TRANS_TYPE_LIGHT_CTL_MAX = MM_TRANS_TYPE_LIGHT_CTL_TEMP,
};

/// Transition type values for the Light HSL Client model
enum mm_trans_type_light_hsl
{
    /// Set Light HSL Lightness and Light HSL Hue and Light HSL Saturation state values
    MM_TRANS_TYPE_LIGHT_HSL = 0,
    /// Set Light HSL Hue state value
    MM_TRANS_TYPE_LIGHT_HSL_HUE,
    /// Set Light HSL Saturation state value
    MM_TRANS_TYPE_LIGHT_HSL_SAT,

    /// Last option value
    MM_TRANS_TYPE_LIGHT_HSL_MAX = MM_TRANS_TYPE_LIGHT_HSL_SAT,
};

/// State identifier values
enum mm_state_idx
{
    /// Generic OnOff state
    MM_STATE_GEN_ONOFF = 0,
    /// Generic Level state
    MM_STATE_GEN_LVL,
    /// Generic Default Transition Time state
    MM_STATE_GEN_DTT,
    /// Generic Power Actual state
    MM_STATE_GEN_POWER_ACTUAL,
    /// Generic Power Last state
    MM_STATE_GEN_POWER_LAST,
    /// Generic Power Default state
    MM_STATE_GEN_POWER_DFLT,
    /// Generic Power Range state
    MM_STATE_GEN_POWER_RANGE,
    /// Generic OnPowerUp state
    MM_STATE_GEN_ONPOWERUP,

    /// Light Lightness
    MM_STATE_LIGHT_LN = 50,
    /// Light Lightness Linear
    MM_STATE_LIGHT_LN_LIN,
    /// Light Lightness Last
    MM_STATE_LIGHT_LN_LAST,
    /// Light Lightness Default
    MM_STATE_LIGHT_LN_DFLT,
    /// Light Lightness Range
    MM_STATE_LIGHT_LN_RANGE,
    /// Light Lightness Range Min
    MM_STATE_LIGHT_LN_RANGE_MIN,
    /// Light Lightness Range Max
    MM_STATE_LIGHT_LN_RANGE_MAX,

    /// Light CTL Lightness
    MM_STATE_LIGHT_CTL_LN = 100,
    /// Light CTL Temperature
    MM_STATE_LIGHT_CTL_TEMP,
    /// Light CTL Delta UV
    MM_STATE_LIGHT_CTL_DELTA_UV,
    /// Light CTL Temperature Default
    MM_STATE_LIGHT_CTL_TEMP_DFLT,
    /// Light CTL Temperature Range
    MM_STATE_LIGHT_CTL_TEMP_RANGE,
    /// Light CTL Delta UV Default
    MM_STATE_LIGHT_CTL_DELTA_UV_DFLT,

    /// Light HSL Lightness
    MM_STATE_LIGHT_HSL_LN = 150,
    /// Light HSL Hue
    MM_STATE_LIGHT_HSL_HUE,
    /// Light HSL Saturation
    MM_STATE_LIGHT_HSL_SAT,
    /// Light HSL Target
    MM_STATE_LIGHT_HSL_TGT,
    /// Light HSL Default (Lightness + Hue + Saturation)
    MM_STATE_LIGHT_HSL_DFLT,
    /// Light HSL Lightness Default
    MM_STATE_LIGHT_HSL_DFLT_LN,
    /// Light HSL Hue Default
    MM_STATE_LIGHT_HSL_DFLT_HUE,
    /// Light HSL Saturation Default
    MM_STATE_LIGHT_HSL_DFLT_SAT,
    /// Light HSL Hue Range
    MM_STATE_LIGHT_HSL_RANGE_HUE,
    /// Light HSL Saturation Range
    MM_STATE_LIGHT_HSL_RANGE_SAT,

    /// Light xyL Lightness
    MM_STATE_LIGHT_XYL_LN = 200,
    /// Light xyL x and y
    MM_STATE_LIGHT_XYL_XY,
    /// Light xyL Lightness Target
    MM_STATE_LIGHT_XYL_LN_TGT,
    /// Light xyL x and y Target
    MM_STATE_LIGHT_XYL_XY_TGT,
    /// Light xyL Lightness Default
    MM_STATE_LIGHT_XYL_LN_DFLT,
    /// Light xyL x and y Default
    MM_STATE_LIGHT_XYL_XY_DFLT,
    /// Light xyL x and y Range
    MM_STATE_LIGHT_XYL_XY_RANGE
};

/// Type of state that can be set
enum mm_state_type
{
    /// Current state value
    MM_STATE_TYPE_CURRENT = 0,
    /// Targeted state value
    MM_STATE_TYPE_TARGET,
    /// Targeted state value during move transition
    MM_STATE_TYPE_TARGET_MOVE,
};

/// Generic Property type
enum mm_prop_type
{
    /// User Property
    MM_PROP_TYPE_USER = 0,
    /// Admin Property
    MM_PROP_TYPE_ADMIN,
    /// Manufacturer Property
    MM_PROP_TYPE_MANUF,
    /// Client Property
    MM_PROP_TYPE_CLI,
};


/*
 * TYPE DEFINITIONS FOR MESH MODELS
 ****************************************************************************************
 */

/// Mesh Model Configuration Structure
typedef struct mm_cfg
{
    /// Number of replay elements to allocate as part of the Replay Manager
    uint8_t nb_replay;
} mm_cfg_t;


/// Structure for Generic Property state information
typedef struct mm_prop
{
    /// Property ID
    uint16_t prop_id;
    /// User Access
    uint8_t user_access;
} mm_prop_t;

#if (BLE_MESH_MDL)

/*
 * CALLBACKS DEFINITION FOR MESH MODELS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Callback function called when a model has been registered.
 *
 * @param[in] model_id      Model Identifier
 * @param[in] elmt_idx      Element Index
 * @param[in] mdl_lid       Allocated model local index
 ****************************************************************************************
 */
typedef void (*mm_api_register_ind_cb)(uint32_t model_id, uint8_t elmt_idx, m_lid_t mdl_lid);

/// @} MESH_MODEL_GENERAL

/// @addtogroup MESH_MODEL_SERVER
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called to inform application about a state update
 *
 * @param[in] state_id          State identifier (see #mm_state_idx)
 * @param[in] elmt_idx          Element Index
 * @param[in] state             New state value or targeted state value depending on
 * transition time
 * @param[in] trans_time_ms     Transition time in milliseconds
 ****************************************************************************************
 */
typedef void (*mm_api_srv_state_upd_ind_cb)(uint16_t state_id, uint8_t elmt_idx, uint32_t state,
                                            uint32_t trans_time_ms);

/**
 ****************************************************************************************
 * @brief Callback function called when a state value not stored locally is required.
 *
 * @param[in] req_ind_code      Request indication code (see #mm_req_ind_code enumerations)
 * @param[in] elmt_idx          Element Index
 ****************************************************************************************
 */
typedef void (*mm_api_srv_state_req_ind_cb)(uint32_t req_ind_code, uint8_t elmt_idx);

/**
 ****************************************************************************************
 * @brief Callback function called when global part of Generic Location state has been
 * set.
 *
 * @param[in] elmt_idx      Element Index
 * @param[in] latitude      Global Latitude
 * @param[in] longitude     Global Longitude
 * @param[in] altitude      Global Altitude
 ****************************************************************************************
 */
typedef void (*mm_api_srv_locg_upd_ind_cb)(uint8_t elmt_idx, int32_t latitude, int32_t longitude,
                                           int16_t altitude);

/**
 ****************************************************************************************
 * @brief Callback function called when local part of Generic Location state has been
 * set.
 *
 * @param[in] elmt_idx      Element Index
 * @param[in] north         Local North
 * @param[in] east          Local East
 * @param[in] altitude      Local Altitude
 * @param[in] floor         Floor Number
 * @param[in] uncertainty   Uncertainty
 ****************************************************************************************
 */
typedef void (*mm_api_srv_locl_upd_ind_cb)(uint8_t elmt_idx, int16_t north, int16_t east,
                                           int16_t altitude, uint8_t floor, uint16_t uncertainty);

/**
 ****************************************************************************************
 * @brief Inform application about received get request for a Generic User/Admin/Manufacturer
 * Property. A confirmation is expected from the application.
 *
 * @param[in] elmt_idx      Index of element for which get request has been received
 * @param[in] prop_type     Property type (see #mm_prop_type enumeration)
 * @param[in] prop_id       Property ID
 ****************************************************************************************
 */
typedef void (*mm_api_srv_prop_get_req_ind_cb)(uint8_t elmt_idx, uint8_t prop_type, uint16_t prop_id);

/**
 ****************************************************************************************
 * @brief Inform application about received set request for a Generic User/Admin/Manufacturer
 * Property. A confirmation is expected from the application.
 *
 * @param[in] elmt_idx      Index of element for which set request has been received
 * @param[in] prop_type     Property type (see #mm_prop_type enumeration)
 * @param[in] prop_id       Property ID
 * @param[in] length        Property value length
 * @param[in] p_val         Pointer to the received property value
 ****************************************************************************************
 */
typedef void (*mm_api_srv_prop_set_req_ind_cb)(uint8_t elmt_idx, uint8_t prop_type, uint16_t prop_id,
                                               uint16_t length, uint8_t *p_val);

/// @} MESH_MODEL_SERVER

/// @addtogroup MESH_MODEL_CLIENT
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called upon reception of a status message in order to inform
 * upper application about received state value.
 *
 * @param[in] src           Address of node's element for which state value is reported
 * @param[in] state_id      State Identifier (see #mm_state_idx)
 * @param[in] state_1       State value 1
 * @param[in] state_2       State value 2
 * @param[in] rem_time_ms   Remaining time in milliseconds before end of transition
 ****************************************************************************************
 */
typedef void (*mm_api_cli_state_ind_cb)(uint16_t src, uint16_t state_id, uint32_t state_1,
                                        uint32_t state_2, uint32_t rem_time_ms);

/**
 ****************************************************************************************
 * @brief Callback function called upon reception of Generic Battery Status message in order
 * to inform application about received Generic Battery state value.
 *
 * @param[in] src               Address of node's element for which state value is reported
 * @param[in] bat_lvl           Battery Level value
 * @param[in] time_discharge    Time to discharge in minutes
 * @param[in] time_charge       Time to charge in minutes
 * @param[in] flags             Flags
 ****************************************************************************************
 */
typedef void (*mm_api_cli_bat_ind_cb)(uint16_t src, uint8_t bat_lvl, uint32_t time_discharge,
                                      uint32_t time_charge, uint8_t flags);

/**
 ****************************************************************************************
 * @brief Callback function called upon reception of Generic Location Global Status message
 * in order to inform application about received global part of Generic Location state value.
 *
 * @param[in] src               Address of node's element for which state value is reported
 * @param[in] latitude          Global Latitude
 * @param[in] longitude         Global Longitude
 * @param[in] altitude          Global Altitude
 ****************************************************************************************
 */
typedef void (*mm_api_cli_locg_ind_cb)(uint16_t src, int32_t latitude, int32_t longitude,
                                       int16_t altitude);

/**
 ****************************************************************************************
 * @brief Callback function called upon reception of Generic Location Global Status message
 * in order to inform application about received local part of Generic Location state value.
 *
 * @param[in] src               Address of node's element for which state value is reported
 * @param[in] north             Local North
 * @param[in] east              Local East
 * @param[in] altitude          Local Altitude
 * @param[in] floor             Floor Number
 * @param[in] uncertainty       Uncertainty
 ****************************************************************************************
 */
typedef void (*mm_api_cli_locl_ind_cb)(uint16_t src, int16_t north, int16_t east, int16_t altitude,
                                       uint8_t floor, uint16_t uncertainty);

/**
 ****************************************************************************************
 * @brief Inform application about reception of a Generic User/Admin/Manufacturer Property
 * value
 *
 * @param[in] src           Address of element for which property value has been received
 * @param[in] prop_id       Property ID
 * @param[in] user_access   User access
 * @param[in] length        Property value length
 * @param[in] p_val         Pointer to the property value
 ****************************************************************************************
 */
typedef void (*mm_api_cli_prop_ind_cb)(uint16_t src, uint16_t prop_id, uint8_t user_access,
                                       uint16_t length, uint8_t *p_val);

/**
 ****************************************************************************************
 * @brief Inform application about reception of list of Generic User/Admin/Manufacturer/Client
 * Properties supported by an element
 *
 * @param[in] src           Address of element for which list of properties has been received
 * @param[in] prop_type     Property type (see #mm_prop_type enumeration)
 * @param[in] nb_prop       Number of properties
 * @param[in] p_prop_ids    Pointer to received list of Property IDs
 ****************************************************************************************
 */
typedef void (*mm_api_cli_prop_list_ind_cb)(uint16_t src, uint8_t prop_type, uint16_t nb_prop,
                                            uint16_t *p_prop_ids);

/// @} MESH_MODEL_CLIENT

/*
 * CALLBACK STRUCTURES FOR MESH MODELS
 ****************************************************************************************
 */

/// @addtogroup MESH_MODEL_SERVER
/// @{

#if (BLE_MESH_MDL_SERVER)
/// Mesh Models Callback Structure (Server)
typedef struct mm_api_cb_server
{
    /// Callback used inform application about a state update
    mm_api_srv_state_upd_ind_cb cb_srv_state_upd_ind;
    /// Callback used to request state information from an application
    mm_api_srv_state_req_ind_cb cb_srv_state_req_ind;
    #if (BLE_MESH_MDL_GENS)
    /// Callback used to inform application about set Generic Location state value
    /// (global part)
    mm_api_srv_locg_upd_ind_cb cb_srv_locg_upd_ind;
    /// Callback used to inform application about set Generic Location state value
    /// (local part)
    mm_api_srv_locl_upd_ind_cb cb_srv_locl_upd_ind;
    /// Callback used to request Generic Property state value
    mm_api_srv_prop_get_req_ind_cb cb_srv_prop_get_req_ind;
    /// Callback used to inform application about set Generic Property state value
    mm_api_srv_prop_set_req_ind_cb cb_srv_prop_set_req_ind;
    #endif //(BLE_MESH_MDL_GENS)
} mm_api_cb_server_t;
#endif //(BLE_MESH_MDL_SERVER)

/// @} MESH_MODEL_SERVER

/// @addtogroup MESH_MODEL_CLIENT
/// @{

#if (BLE_MESH_MDL_CLIENT)
/// Mesh Models Callback Structure (Client)
typedef struct mm_api_cb_client
{
    /// Callback used to inform application about received state
    mm_api_cli_state_ind_cb cb_cli_state_ind;
    #if (BLE_MESH_MDL_GENC)
    /// Callback used to inform application about received Generic Battery state value
    mm_api_cli_bat_ind_cb cb_cli_bat_ind;
    /// Callback used to inform application about received Generic Location state value
    /// (global part)
    mm_api_cli_locg_ind_cb cb_cli_locg_ind;
    /// Callback used to inform application about received Generic Location state value
    /// (local part)
    mm_api_cli_locl_ind_cb cb_cli_locl_ind;
    /// Callback used to inform application about a received Generic Property state value
    mm_api_cli_prop_ind_cb cb_cli_prop_ind;
    /// Callback used to inform application about received list of Generic User/Admin/Manufacturer
    /// Properties
    mm_api_cli_prop_list_ind_cb cb_cli_prop_list_ind;
    #endif //(BLE_MESH_MDL_GENC)
} mm_api_cb_client_t;
#endif //(BLE_MESH_MDL_CLIENT)

/// @} MESH_MODEL_CLIENT

/*
 * FUNCTION DECLARATIONS FOR MESH MODELS
 ****************************************************************************************
 */

/// @addtogroup MESH_MODEL_SERVER
/// @{

#if (BLE_MESH_MDL_SERVER)
/**
 ****************************************************************************************
 * @brief Define the set of callback functions to communicate with Mesh Model block (Server part)
 *
 * @param[in] p_cb_api      Callback functions for communication with Mesh Model block
 *
 * @return An error code
 ****************************************************************************************
 */
uint16_t mm_api_set_server(const mm_api_cb_server_t *p_cb_api);

/**
 ****************************************************************************************
 * @brief Register Server model configuration for a local element
 *
 * @param[in] elmt_idx         Element Index
 * @param[in] mdl_cfg_idx      Server model configuration index (see #mm_cfg_idx enumeration)
 * @param[in] info             Information
 * @param[in] cb_register_ind  Callback function called each time a model is registered
 *
 * @return An error status (see #mesh_error)
 ****************************************************************************************
 */
uint16_t mm_api_register_server(uint8_t elmt_idx, uint8_t mdl_cfg_idx, uint8_t info,
                                mm_api_register_ind_cb cb_register_ind);

#if (BLE_MESH_MDL_GENS)
/**
 ****************************************************************************************
 * @brief Register Generic Property models on an element
 *
 * @param[in] elmt_idx          Index of element on which models must be registered
 * @param[in] req_queue_len     Number of received messages that can be queued when model
 * is waiting for application confirmation
 * @param[in] nb_prop_user      Number of Generic User Properties
 * @param[in] nb_prop_admin     Number of Generic Admin Properties
 * @param[in] nb_prop_manuf     Number of Generic Manufacturer Properties
 * @param[in] nb_prop_cli       Number of Generic Client Properties
 * @param[in] p_props           Pointer to list of Generic Property information
 * @param[in] cb_register_ind   Callback function called each time a model is registered
 *
 * @return An handling error status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t mm_api_register_server_prop(uint8_t elmt_idx, uint8_t req_queue_len,
                                     uint8_t nb_prop_user, uint8_t nb_prop_admin,
                                     uint8_t nb_prop_manuf, uint8_t nb_prop_cli,
                                     const mm_prop_t *p_props, mm_api_register_ind_cb cb_register_ind);
#endif //(BLE_MESH_MDL_GENS)

/**
 ****************************************************************************************
 * @brief Set state value
 *
 * @param[in] mdl_lid       Model lid
 * @param[in] state_id      State identifier
 * @param[in] state         State value
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t mm_api_srv_set(m_lid_t mdl_lid, uint16_t state_id, uint32_t state);

#if (BLE_MESH_MDL_GENS)
/**
 ****************************************************************************************
 * @brief Confirm value of Generic Battery state for a given local element after reception
 * of a request indication
 *
 * @param[in] status            Confirmation status
 * @param[in] elmt_idx          Element Index
 * @param[in] bat_lvl           Battery Level
 * @param[in] time_charge       Time to charge in minutes
 * @param[in] time_discharge    Time to discharge in minutes
 * @param[in] flags             Flags
 ****************************************************************************************
 */
void mm_api_srv_bat_cfm(uint16_t status, uint8_t elmt_idx, uint8_t bat_lvl, uint32_t time_charge,
                          uint32_t time_discharge, uint8_t flags);

/**
 ****************************************************************************************
 * @brief Confirm value global part of Generic Location state for a given local element
 * after reception of a request indication
 *
 * @param[in] status            Confirmation status
 * @param[in] elmt_idx          Element Index
 * @param[in] latitude          Global Latitude
 * @param[in] longitude         Global Longitude
 * @param[in] altitude          Global Altitude
 ****************************************************************************************
 */
void mm_api_srv_locg_cfm(uint16_t status, uint8_t elmt_idx, int32_t latitude, int32_t longitude,
                           int16_t altitude);

/**
 ****************************************************************************************
 * @brief Confirm value local part of Generic Location state for a given local element
 * after reception of a request indication
 *
 * @param[in] status            Confirmation status
 * @param[in] elmt_idx          Element Index
 * @param[in] north             Local North
 * @param[in] east              Local East
 * @param[in] altitude          Local Altitude
 * @param[in] floor             Floor number
 * @param[in] uncertainty       Uncertainty
 ****************************************************************************************
 */
void mm_api_srv_locl_cfm(uint16_t status, uint8_t elmt_idx, int16_t north, int16_t east,
                           int16_t altitude, uint8_t floor, uint16_t uncertainty);

/**
 ****************************************************************************************
 * @brief Confirmation functions for get and set request indication received by the application
 *
 * @param[in] status        Confirmation status
 * @param[in] elmt_idx      Element index
 * @param[in] prop_type     Property Type (see #mm_prop_type enumeration)
 * @param[in] prop_id       Property ID
 * @param[in] length        Property Length
 * @param[in] p_val         Pointer to the property value
 ****************************************************************************************
 */
void mm_api_srv_prop_cfm(uint16_t status, uint8_t elmt_idx, uint8_t prop_type, uint16_t prop_id,
                         uint16_t length, const uint8_t *p_val);
#endif //(BLE_MESH_MDL_GENS)
#endif //(BLE_MESH_MDL_SERVER)

/// @} MESH_MODEL_SERVER

/// @addtogroup MESH_MODEL_CLIENT
/// @{

#if (BLE_MESH_MDL_CLIENT)
/**
 ****************************************************************************************
 * @brief Define the set of callback functions to communicate with Mesh Model block (Client part)
 *
 * @param[in] p_cb_api      Callback functions for communication with Mesh Model block
 *
 * @return An error code
 ****************************************************************************************
 */
uint16_t mm_api_set_client(const mm_api_cb_client_t *p_cb_api);

/**
 ****************************************************************************************
 * @brief Register use of a client model
 *
 * @param[in] cmdl_idx         Client model index (see #mm_cmdl_idx enumeration)
 * @param[in] cb_register_ind  Callback function called each time a model is registered
 *
 * @return Handling status
 ****************************************************************************************
 */
uint16_t mm_api_register_client(uint16_t cmdl_idx, mm_api_register_ind_cb cb_register_ind);

/**
 ****************************************************************************************
 * @brief Get value of a state
 *
 * @param[in] mdl_lid       Local index for the client model used to get the needed state value
 * @param[in] app_key_lid   Application key local index
 * @param[in] dst           Element address to which the get message must be sent
 * @param[in] get_info      Get information
 *
 * @return An error status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t mm_api_cli_get(m_lid_t mdl_lid, m_lid_t app_key_lid, uint16_t dst, uint16_t get_info);

/**
 ****************************************************************************************
 * @brief Set value of a state
 *
 * @param[in] mdl_lid       Local index for the client model used to set the needed state value
 * @param[in] app_key_lid   Application key local index
 * @param[in] dst           Element address to which the set message must be sent
 * @param[in] state_1       State value 1
 * @param[in] state_2       State value 2
 * @param[in] set_info      Set information
 *
 * @return An error status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t mm_api_cli_set(m_lid_t mdl_lid, m_lid_t app_key_lid, uint16_t dst, uint32_t state_1,
                        uint32_t state_2, uint16_t set_info);

/**
 ****************************************************************************************
 * @brief Set value of a state using a transition
 *
 * @param[in] mdl_lid           Local index for the client model used to set the needed state value
 * @param[in] app_key_lid   Application key local index
 * @param[in] dst               Element address to which the set message must be sent
 * @param[in] state_1           State value 1
 * @param[in] state_2           State value 2
 * @param[in] trans_time_ms     Transition Time in milliseconds
 * @param[in] delay_ms          Delay in milliseconds
 * @param[in] trans_info        Transition Information (see #mm_trans_info enumeration)
 *
 * @return An error status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t mm_api_cli_transition(m_lid_t mdl_lid, m_lid_t app_key_lid, uint16_t dst, uint32_t state_1,
                               uint32_t state_2, uint32_t trans_time_ms, uint16_t delay_ms,
                               uint16_t trans_info);

#if (BLE_MESH_MDL_GENC)
/**
 ****************************************************************************************
 * @brief Set global part of Generic Location state
 *
 * @param[in] mdl_lid       Local index for the client model used to set the needed state value
 * @param[in] app_key_lid   Application key local index
 * @param[in] dst           Element address to which the set message must be sent
 * @param[in] set_info      Set information
 * @param[in] latitude      Global Latitude
 * @param[in] longitude     Global Longitude
 * @param[in] altitude      Global Altitude
 *
 * @return An error status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t mm_api_cli_set_locg(m_lid_t mdl_lid, m_lid_t app_key_lid, uint16_t dst, uint8_t set_info,
                             int32_t latitude, int32_t longitude, int16_t altitude);

/**
 ****************************************************************************************
 * @brief Set local part of Generic Location state
 *
 * @param[in] mdl_lid       Local index for the client model used to set the needed state value
 * @param[in] app_key_lid   Application key local index
 * @param[in] dst           Element address to which the set message must be sent
 * @param[in] set_info      Set information
 * @param[in] north         Local North
 * @param[in] east          Local East
 * @param[in] altitude      Local Altitude
 * @param[in] floor         Floor Number
 * @param[in] uncertainty   Uncertainty
 *
 * @return An error status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t mm_api_cli_set_locl(m_lid_t mdl_lid, m_lid_t app_key_lid, uint16_t dst, uint8_t set_info,
                             int16_t north, int16_t east, int16_t altitude, uint8_t floor,
                             uint16_t uncertainty);

/**
 ****************************************************************************************
 * @brief Request to send a Generic User/Admin/Manufacturer/Client Property(ies) Get message
 * to an element
 *
 * @param[in] mdl_lid       Model Local Index of Generic Property Client model
 * @param[in] app_key_lid   Application key local index
 * @param[in] dst           Address of element to which the message must be sent
 * @param[in] get_type      Get type
 * @param[in] prop_id       Property ID
 *
 * @return An handling status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t mm_api_cli_get_prop(m_lid_t mdl_lid, m_lid_t app_key_lid, uint16_t dst, uint8_t get_type,
                             uint16_t prop_id);

/**
 ****************************************************************************************
 * @brief Request to send a Generic User/Admin/Manufacturer Property Set message to an
 * element
 *
 * @param[in] mdl_lid       Model local Index of Generic Property Client model
 * @param[in] app_key_lid   Application key local index
 * @param[in] dst           Address of element to which the message must be sent
 * @param[in] set_info      Set information
 * @param[in] prop_id       Property ID
 * @param[in] user_access   User access
 * @param[in] length        Property value length
 * @param[in] p_val         Pointer to the property value
 *
 * @return An handling status (see #mesh_error enumeration)
 ****************************************************************************************
 */
uint16_t mm_api_cli_set_prop(m_lid_t mdl_lid, m_lid_t app_key_lid, uint16_t dst, uint8_t set_info,
                             uint16_t prop_id, uint8_t user_access, uint16_t length,
                             const uint8_t *p_val);
#endif //(BLE_MESH_MDL_GENC)
#endif //(BLE_MESH_MDL_CLIENT)
#endif //(BLE_MESH_MDL)

/// @} MESH_MODEL_CLIENT
/// @} MESH_MODEL

/// @addtogroup MESH_COMMON
/// @{
/// @addtogroup MESH_COMMON_GENERAL
/// @{

/*
 * TYPE DEFINITIONS FOR MESH STACK
 ****************************************************************************************
 */

/// Mesh Stack Configuration Structure
typedef struct mesh_cfg
{
    /// Mesh Profile Configuration
    m_cfg_t prf_cfg;
    #if (BLE_MESH_MDL)
    /// Mesh Model Configuration
    mm_cfg_t model_cfg;
    #endif //(BLE_MESH_MDL)
} mesh_cfg_t;

/// Mesh Stack Version Structure
typedef struct mesh_version
{
    /// Mesh Specification version (X.Y.Z)
    uint8_t mesh_version_x;
    /// Mesh Specification version (X.Y.Z)
    uint8_t mesh_version_y;
    /// Mesh Specification version (X.Y.Z)
    uint8_t mesh_version_z;

    /// Mesh Software version (X.Y)
    uint8_t mesh_sw_version_x;
    /// Mesh Software version (X.Y)
    uint8_t mesh_sw_version_y;
} mesh_version_t;

/*
 * CALLBACK DEFINITIONS FOR MESH STACK
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Callback executed when a buffer block has been properly released.
 *
 * @param[in] block_id  Buffer Block identifier
 ****************************************************************************************
 */
typedef void (*mesh_api_buf_block_released_cb)(uint8_t block_id);

/*
 * CALLBACK STRUCTURES FOR MESH STACK
 ****************************************************************************************
 */

/// Callbacks structure for Mesh Stack
typedef struct mesh_api_cb
{
    /// Callback used to inform that a buffer block has been properly released
    mesh_api_buf_block_released_cb cb_buf_block_freed;
} mesh_api_cb_t;

/*
 * FUNCTION DECLARATIONS FOR MESH STACK
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handler for messages targeting the mesh stack
 *
 * @param[in] msg_id        Message ID
 * @param[in] src_id        Source ID
 * @param[in] p_param       Pointer to command code
 *
 * @return A message status (see enum mal_msg_status)
 ****************************************************************************************
 */
uint8_t mesh_handler(uint16_t msg_id, uint16_t src_id, const void *p_param);

/**
 ****************************************************************************************
 * @brief Define the set of callback to communicate with mesh native API
 *
 * @param[in] p_cb_api Native application callback set use to communicate with a native API
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t mesh_api_set(const mesh_api_cb_t *p_cb_api);

/// @} MESH_COMMON_GENERAL

/// @addtogroup MESH_COMMON_RUN_TIME
/// @{

/**
 ****************************************************************************************
 * @brief Get device run time
 *
 * @param[out] p_clock_ms       Pointer to variable that will contain the current clock in milliseconds
 * @param[out] p_nb_wrap        Pointer to variable that will contain the number of wrap
 ****************************************************************************************
 */
void mesh_api_get_run_time(uint32_t *p_clock_ms, uint16_t *p_nb_wrap);

/**
 ****************************************************************************************
 * @brief Set device run time
 *
 * @param[out] clock_ms       Current clock in milliseconds
 * @param[out] nb_wrap        Number of wraps
 *
 * @return An handling status (see enum mesh_error)
 ****************************************************************************************
 */
uint16_t mesh_api_set_run_time(uint32_t clock_ms, uint16_t nb_wrap);

/// @} MESH_COMMON_RUN_TIME

/// @addtogroup MESH_COMMON_VERSION
/// @{

/**
 ****************************************************************************************
 * @brief Get Mesh Stack version
 *
 * @param[out] p_version       Pointer to structure in which version information will be provided
 ****************************************************************************************
 */
void mesh_api_get_version(mesh_version_t *p_version);

/// @} MESH_COMMON_VERSION

/// @addtogroup MESH_COMMON_BUFFER
/// @{

/**
 ****************************************************************************************
 * @brief Allocate block of buffers
 *
 * @param[out] p_block_id  Pointer to the variable that will contain the index of the allocated block.
 * @param[in]  nb_bufs     Number of buffers contained in the block.
 * @param[in]  small       Indicate if block contains small or long buffers.
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t mesh_api_buf_alloc_block(uint8_t *p_block_id, uint8_t nb_bufs, bool small);

/**
 ****************************************************************************************
 * @brief Free block of buffers
 *
 * @note cb_release() of m_api_cb_t called at end of disable execution
 *
 * @param[in] block_id   Index of the allocated block.
 *
 * @return Execution Status code
 ****************************************************************************************
 */
uint16_t mesh_api_buf_free_block(uint8_t block_id);

/// @} MESH_COMMON_BUFFER
/// @} MESH_COMMON
/// @} MESH_API

#endif /* MESH_API_ */
