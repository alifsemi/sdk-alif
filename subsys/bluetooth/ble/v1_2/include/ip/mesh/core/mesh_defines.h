/**
 ****************************************************************************************
 *
 * @file mesh_defines.h
 *
 * @brief Header file for Mesh Stack Defines
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef MESH_DEFINES_
#define MESH_DEFINES_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup MESH_DEFINES Defines
 * @ingroup MESH
 * @brief  Mesh Defines
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDES FILES
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdbool.h>
#include "co_math.h"
#include "arch.h"
#include "mesh_config.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Invalid Local identifier
#define MESH_INVALID_LID               (0xFF)

/// Size of a key
#define MESH_KEY_LEN                   (16)
/// Size of value block when encrypting
#define MESH_ENC_VAL_LEN               (16)
/// Public Key X coordinate length
#define MESH_PUB_KEY_X_LEN             (32)
/// Public Key Y coordinate length
#define MESH_PUB_KEY_Y_LEN             (32)
/// Size of Device UUID
#define MESH_DEV_UUID_LEN              (16)
/// Length of the Private P-256 key
#define MESH_PRIVATE_KEY_LEN           (32)
/// ECDH Secret size length
#define MESH_ECDH_SECRET_LEN           (32)
/// Size of K1 result length value
#define MESH_K1_RES_LEN                (16)
/// Size of K2 result length value - (263 bits)
#define MESH_K2_RES_LEN                (33)
/// Size of K3 result length value - (64 bits)
#define MESH_K3_RES_LEN                (8)
/// Size of K4 result length value - (6 bits)
#define MESH_K4_RES_LEN                (1)
/// Size of the Nonce used for AES-CCM
#define MESH_NONCE_LEN                 (13)
/// Size of Label UUID
#define M_LABEL_UUID_LEN               (16)

/// Unassigned address value
#define MESH_UNASSIGNED_ADDR           (0x0000)

/// Mesh Error Protocol Group Code
#define MESH_ERR_PROTOCOL_CODE         (0x0080)
/// Mesh Error Provisioning Group Code
#define MESH_ERR_PROVISIONING_CODE     (0x0081)
/// Mesh Error Internal Group Code
#define MESH_ERR_INTERNAL_CODE         (0x0082)
/// Mesh Error Low Power Node Group Code
#define MESH_ERR_LPN_CODE              (0x0083)
/// Mesh Error Model Group Code
#define MESH_ERR_MDL_CODE              (0x0084)

/// Primary element index
#define MESH_PRIMARY_ELEM_IDX          (0)
#if (BLE_MESH_FND_SVR)
/// Configuration Server SIG Model ID
#define MESH_FND_CONFS_MODEL_ID        (0x0000)
/// Health Server SIG Model ID
#define MESH_FND_HLTHS_MODEL_ID        (0x0002)
#endif // (BLE_MESH_FND_SVR)
#if (BLE_MESH_FND_CLI)
/// Configuration Client SIG Model ID
#define MESH_FND_CONFC_MODEL_ID        (0x0001)
/// Health Client SIG Model ID
#define MESH_FND_HLTHC_MODEL_ID        (0x0003)
#endif // (BLE_MESH_FND_CLI)

/*
 * MACROS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Macros returning the mesh error code for a given mesh sub-error code.
 *
 * @param[in] grp        Mesh group code
 * @param[in] suberror   Mesh sub-error code
 *
 * @return Mesh error code
 ****************************************************************************************
 */
#define MESH_ERR_(grp, suberror)        ((MESH_ERR_##grp##_CODE) | (suberror << 8))

/**
 ****************************************************************************************
 * @brief Macros returning the mesh sub-error code for a given mesh error code.
 *
 * @param[in] error   Mesh error code
 *
 * @return Mesh sub-error code
 ****************************************************************************************
 */
#define MESH_SUBERR(error)             (error >> 8)

/**
 ****************************************************************************************
 * @brief Macros returning the mesh group code for a given mesh error code.
 *
 * @param[in] error   Mesh error code
 *
 * @return Mesh group error code
 ****************************************************************************************
 */
#define MESH_ERR_GRP(error)                (error & 0xFF)

/// Check if access opcode is a 1 octet value
#define MESH_IS_1_OCT_OPCODE(opcode)       (((opcode) & 0x80) == 0)
/// Check if access opcode is a 2 octets value
#define MESH_IS_2_OCT_OPCODE(opcode)       (((opcode) & 0xC0) == 0x80)
/// Check if access opcode is a 3 octets value
#define MESH_IS_3_OCT_OPCODE(opcode)       (((opcode) & 0xC0) == 0xC0)

/// Macro returning if a Model ID is a vendor Model ID
#define MESH_IS_VENDOR_MODEL(mdl_id)       ((mdl_id & 0xFFFF0000) != 0)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// State of the provisioning
enum m_prov_state
{
    /// Provisioning started - procedure started by a provisioner
    M_PROV_STARTED = 0,
    /// Provisioning succeed
    M_PROV_SUCCEED,
    /// Provisioning failed
    M_PROV_FAILED,
};

/// Provisioning information
enum m_prov_info
{
    /// URI Hash present or not in the unprovisioned device beacon
    M_PROV_INFO_URI_HASH_PRESENT = (1 << 0),
};

/// Algorithms bit field
enum m_prov_algo_bf
{
    /// Bit[0]: FIPS P-256 Elliptic Curve
    M_PROV_ALGO_FIPS_P256_ELLIPTIC_CURVE_POS = 0,
    M_PROV_ALGO_FIPS_P256_ELLIPTIC_CURVE_BIT = 0x0001,
    /// Bit[1-15]: Reserved for Future Use
    M_PROV_ALGO_LSB = 0,
    M_PROV_ALGO_MASK = 0x1,
};

/// Algorithms field values
enum m_prov_algo
{
    /// FIPS P-256 Elliptic Curve
    M_PROV_ALGO_FIPS_P256_ELLIPTIC_CURVE = 0,
    /// Value [other-0xFF]: Reserved for Future Use
    M_PROV_ALGO_MAX,
};

/// Public Key field values
enum m_prov_pub_key_type
{
    /// Bit[0]: Public Key OOB information available
    M_PROV_PUB_KEY_OOB_AVAILABLE = 0x01,
    /// Bit[1-7]: Prohibited
};

/// Public Key Type field values
enum m_prov_pub_key_val
{
    /// No OOB Public Key is used
    M_PROV_PUB_KEY_OOB_NOT_USED = 0x00,
    /// OOB Public Key is used
    M_PROV_PUB_KEY_OOB_USED = 0x01,

    M_PROV_PUB_KEY_OOB_MAX,
};

/// Authentication Method field values
enum m_prov_auth_method
{
    /// No OOB authentication is used
    M_PROV_AUTH_NO_OOB = 0,
    /// Static OOB authentication is used
    M_PROV_AUTH_STATIC_OOB,
    /// Output OOB authentication is used
    M_PROV_AUTH_OUTPUT_OOB,
    /// Input OOB authentication is used
    M_PROV_AUTH_INPUT_OOB,
};

/// OOB Type bit field
enum m_prov_oob_type_bf
{
    /// Bit[0]: Static OOB information available
    M_PROV_OOB_STATIC_AVAILABLE_BIT = 0x01,
    M_PROV_OOB_STATIC_AVAILABLE_POS = 0,
    /// Bit[1-7]: Prohibited
    M_PROV_OOB_LSB = 0,
    M_PROV_OOB_MASK = 0x1,
};

/// Output OOB Action field values
enum m_prov_out_oob
{
    /// Bit[0]: Blink
    M_PROV_OUT_OOB_BLINK = 0x0001,
    /// Bit[1]: Beep
    M_PROV_OUT_OOB_BEEP = 0x0002,
    /// Bit[2]: Vibrate
    M_PROV_OUT_OOB_VIBRATE = 0x0004,
    /// Bit[3]: Output Numeric
    M_PROV_OUT_OOB_NUMERIC = 0x0008,
    /// Bit[4]: Output Alphanumeric
    M_PROV_OUT_OOB_ALPHANUMERIC = 0x0010,
    /// Bit[5-15]: Reserved for Future Use
};

/// Input OOB Action field values
enum m_prov_in_oob
{
    /// Bit[0]: Push
    M_PROV_IN_OOB_PUSH = 0x0001,
    /// Bit[1]: Twist
    M_PROV_IN_OOB_TWIST = 0x0002,
    /// Bit[2]: Input Numeric
    M_PROV_IN_OOB_NUMERIC = 0x0004,
    /// Bit[3]: Input Alphanumeric
    M_PROV_IN_OOB_ALPHANUMERIC = 0x0008,
    /// Bit[4-15]: Reserved for Future Use
};

/// Scan configuration bit field
enum m_prover_scan_cfg_bf
{
    /// Accept connectable advertising
    M_PROVER_SCAN_CFG_CON_ADV_ACCEPT_POS = 0,
    M_PROVER_SCAN_CFG_CON_ADV_ACCEPT_BIT = CO_BIT(M_PROVER_SCAN_CFG_CON_ADV_ACCEPT_POS),
};

/// Scan stopped reason
enum m_prover_scan_stopped_reason
{
    /// Stop command requested by APP
    M_PROVER_SCAN_STOPPED_REQ_BY_APP = 0,
    /// Provisioning start (Invite command) requested by APP
    M_PROVER_SCAN_STOPPED_PROV_STARTED,
    /// Scan timeout
    M_PROVER_SCAN_STOPPED_TIMEOUT,
};

/// Get type values for Configuration Client model
enum m_fnd_confc_get_type
{
    /// Get Beacon state
    M_FND_CONFC_GET_TYPE_BEACON = 0,
    /// Get Default TTL
    M_FND_CONFC_GET_TYPE_DFLT_TTL,
    /// Get GATT Proxy state
    M_FND_CONFC_GET_TYPE_GATT_PROXY,
    /// Get Relay State
    M_FND_CONFC_GET_TYPE_RELAY,
    /// Get Friend state
    M_FND_CONFC_GET_TYPE_FRIEND,
    /// Get Heartbeat Publication parameters
    M_FND_CONFC_GET_TYPE_HB_PUBLI,
    /// Get Heartbeat Subscription parameters
    M_FND_CONFC_GET_TYPE_HB_SUBS,
    /// Get Network Transmit state
    M_FND_CONFC_GET_TYPE_NET_TRANSMIT,
    /// Get list of Network Keys
    M_FND_CONFC_GET_TYPE_NET_KEYS,
    /// Get one page of the Composition Data
    M_FND_CONFC_GET_TYPE_COMPO_DATA,
    /// Get the current value of PollTimeout timer of the Low Power node within a Friend node
    M_FND_CONFC_GET_TYPE_LPN_POLLTIEOUT,

    M_FND_CONFC_GET_TYPE_MAX,
};

/// Received value type for Configuration Client model
enum m_fnd_confc_value_type
{
    /// Beacon status
    M_FND_CONFC_VAL_TYPE_BEACON_STATUS = 0,
    /// Composition Data status
    M_FND_CONFC_VAL_TYPE_COMPO_DATA_STATUS,
    /// Default TTL status
    M_FND_CONFC_VAL_TYPE_DFLT_TTL_STATUS,
    /// GATT Proxy status
    M_FND_CONFC_VAL_TYPE_GATT_PROXY_STATUS,
    /// Relay status
    M_FND_CONFC_VAL_TYPE_RELAY_STATUS,
    /// Model Publication status
    M_FND_CONFC_VAL_TYPE_MDL_PUBLI_STATUS,
    /// Model Subscription status
    M_FND_CONFC_VAL_TYPE_MDL_SUBS_STATUS,
    /// Model Subscription list
    M_FND_CONFC_VAL_TYPE_MDL_SUBS_LIST,
    /// NetKey status
    M_FND_CONFC_VAL_TYPE_NETKEY_STATUS,
    /// NetKey list
    M_FND_CONFC_VAL_TYPE_NETKEY_LIST,
    /// AppKey status
    M_FND_CONFC_VAL_TYPE_APPKEY_STATUS,
    /// AppKey list
    M_FND_CONFC_VAL_TYPE_APPKEY_LIST,
    /// Node Identity status
    M_FND_CONFC_VAL_TYPE_NODE_IDENTITY_STATUS,
    /// Model App status
    M_FND_CONFC_VAL_TYPE_MDL_APP_STATUS,
    /// Model App list
    M_FND_CONFC_VAL_TYPE_MDL_APP_LIST,
    /// Node Reset status
    M_FND_CONFC_VAL_TYPE_NODE_RESET_STATUS,
    /// Friend status
    M_FND_CONFC_VAL_TYPE_FRIEND_STATUS,
    /// Key Refresh Phase status
    M_FND_CONFC_VAL_TYPE_PHASE_STATUS,
    /// Heartbeat Publication status
    M_FND_CONFC_VAL_TYPE_HB_PUBLI_STATUS,
    /// Heartbeat Subscription status
    M_FND_CONFC_VAL_TYPE_HB_SUBS_STATUS,
    /// Low Power Node PollTimeout status
    M_FND_CONFC_VAL_TYPE_LPN_POLLTIMEOUT_STATUS,
    /// Network Transmit status
    M_FND_CONFC_VAL_TYPE_NET_TX_STATUS,
};

/// Model Get type values for Configuration Client model
enum m_fnd_confc_mdl_get_type
{
    /// Get Publication Parameters
    M_FND_CONFC_MDL_GET_TYPE_PUBLI = 0,
    /// Get Model Subscrition List content
    M_FND_CONFC_MDL_GET_TYPE_SUBS,
    /// Get list of bound Application Keys bound to the Model
    M_FND_CONFC_MDL_GET_TYPE_APP,

    M_FND_CONFC_MDL_GET_TYPE_MAX
};

/// Model Subscription action type values for Configuration Client model
enum m_fnd_confc_mdl_subs_action_type
{
    /// Add an address in the subscription list
    M_FND_CONFC_MDL_SUBS_ACTION_TYPE_ADD = 0,
    /// Delete an address from the subscription list
    M_FND_CONFC_MDL_SUBS_ACTION_TYPE_DELETE,
    /// Overwrite content of subscription list
    M_FND_CONFC_MDL_SUBS_ACTION_TYPE_OVERWRITE,
    /// Delete content of subscription list
    M_FND_CONFC_MDL_SUBS_ACTION_TYPE_DELETE_ALL,

    M_FND_CONFC_MDL_SUBS_ACTION_TYPE_MAX,
};

/// Set type values for Configuration Client model
enum m_fnd_confc_set_type
{
    /// Set Beacon state
    M_FND_CONFC_SET_TYPE_BEACON = 0,
    /// Set Default TTL
    M_FND_CONFC_SET_TYPE_DFLT_TTL,
    /// Set GATT Proxy state
    M_FND_CONFC_SET_TYPE_GATT_PROXY,
    /// Set Friend state
    M_FND_CONFC_SET_TYPE_FRIEND,
    /// Node Reset
    M_FND_CONFC_SET_TYPE_RESET,
    /// Set Network Transmit state
    M_FND_CONFC_SET_TYPE_NET_TX,
    /// Set Relay and Relay Retransmit
    M_FND_CONFC_SET_TYPE_RELAY,

    M_FND_CONFC_SET_TYPE_MAX,
};

/// Model AppKey action type values for Configuration Client model
enum m_fnd_confc_mdl_app_action_type
{
    /// Bind an AppKey to a model
    M_FND_CONFC_MDL_APP_ACTION_TYPE_BIND,
    /// Remove the binding between an AppKey and a model
    M_FND_CONFC_MDL_APP_ACTION_TYPE_UNBIND,

    M_FND_CONFC_MDL_APP_ACTION_TYPE_MAX,
};

/// Model Set type for Configuration Client model
enum m_fnd_confc_mdl_publi_set_type
{
    /// Set Model Publication state
    M_FND_CONFC_MDL_PUBLI_SET_TYPE_ADDR,
    /// Set Model Publication Virtual Address
    M_FND_CONFC_MDL_PUBLI_SET_TYPE_VADDR,

    M_FND_CONFC_MDL_PUBLI_SET_TYPE_MAX,
};

/// Get type values for Health Client model
enum m_fnd_hlthc_get_type
{
    /// Get Period Divider
    M_FND_HLTHC_GET_TYPE_PERIOD = 0,
    /// Get Attention Timer duration
    M_FND_HLTHC_GET_TYPE_ATTENTION,
    /// Get list of faults
    M_FND_HLTHC_GET_TYPE_FAULT,

    M_FND_HLTHC_GET_TYPE_MAX,
};

/// Set type values for Health Client model
enum m_fnd_hlthc_set_type
{
    /// Set Period Divider
    M_FND_HLTHC_SET_TYPE_PERIOD = 0,
    /// Set Attention Timer Duration
    M_FND_HLTHC_SET_TYPE_ATTENTION,

    M_FND_HLTHC_SET_TYPE_MAX,
};

/// Set configuration bit field
enum m_fnd_hlthc_set_cfg_bf
{
    /// Acknowledged Set type message
    M_FND_HLTHC_SET_CFG_ACK_POS = 0,
    M_FND_HLTHC_SET_CFG_ACK_BIT = CO_BIT(M_FND_HLTHC_SET_CFG_ACK_POS),

    /// Acknowledged Action type message
    M_FND_HLTHC_FAULT_ACTION_CFG_ACK_POS = 1,
    M_FND_HLTHC_FAULT_ACTION_CFG_ACK_BIT = CO_BIT(M_FND_HLTHC_FAULT_ACTION_CFG_ACK_POS),
};

/// Action type values for Health Client model
enum m_fnd_hlthc_action_type
{
    /// Clear registered faults
    M_FND_HLTHC_ACTION_TYPE_CLEAR = 0,
    /// Invoke a self-test procedure
    M_FND_HLTHC_ACTION_TYPE_TEST,

    M_FND_HLTHC_ACTION_TYPE_MAX,
};

/// Value type values for Health Client model
enum m_fnd_hlthc_value_type
{
    /// Current Status
    M_FND_HLTHC_VAL_TYPE_CUR_STATUS = 0,
    /// Fault Status
    M_FND_HLTHC_VAL_TYPE_FAULT_STATUS,
    /// Period Status
    M_FND_HLTHC_VAL_TYPE_PERIOD_STATUS,
    /// Attention Status
    M_FND_HLTHC_VAL_TYPE_ATTENTION_STATUS,
};

/// Bearer index bit field
enum mesh_bearer_index_bf
{
    /// Advertising bearer
    MESH_BEARER_INDEX_ADV_POS   = 0,
    MESH_BEARER_INDEX_ADV_BIT   = 0x1,
    /// Gatt bearer
    MESH_BEARER_INDEX_GATT_POS  = 1,
    MESH_BEARER_INDEX_GATT_BIT  = 0x2,

    /// RFU value
    MESH_BEARER_INDEX_RFU_LSB   = 2,
    MESH_BEARER_INDEX_RFU_MASK  = 0xFFFC,
};

/// bearer type value
enum mesh_bearer_type
{
    /// Advertising bearer
    MESH_BEARER_TYPE_ADV,
    /// GATT bearer
    MESH_BEARER_TYPE_GATT,

    MESH_BEARER_TYPE_MAX,
};

/// Connection packet type
enum mesh_bearer_con_type
{
    /// Network Message
    MESH_BEARER_CON_TYPE_NET    = (0x00),
    /// Mesh Beacon message
    MESH_BEARER_CON_TYPE_BEACON = (0x01),
    /// Proxy configuration message
    MESH_BEARER_CON_TYPE_PROXY  = (0x02),
    /// Provisioning PDU message
    MESH_BEARER_CON_TYPE_PROV   = (0x03),
};

/// Proxy connectable advertising control values
enum mesh_proxy_adv_ctl
{
    /// Stop connectable advertising
    MESH_PROXY_ADV_CTL_STOP = 0,
    /// Start connectable advertising with Node Identity (duration = 60s)
    MESH_PROXY_ADV_CTL_START_NODE,
    /// Start connectable advertising with Network ID (duration = 60s)
    MESH_PROXY_ADV_CTL_START_NET,
};

/// Proxy connectable advertising state update types
enum mesh_proxy_adv_upd
{
    /// Advertising with Node Identity stopped
    MESH_PROXY_ADV_NODE_STOP = 0,
    /// Advertising with Node Identity started
    MESH_PROXY_ADV_NODE_START,
    /// Advertising with Network ID stopped
    MESH_PROXY_ADV_NET_STOP,
    /// Advertising with Network Identity started
    MESH_PROXY_ADV_NET_START,

    /// Maximum value
    MESH_PROXY_ADV_MAX,
};

/// Proxy connectable advertising state update reasons
enum mesh_proxy_adv_upd_reason
{
    /// Stopped due to timeout (60s)
    MESH_PROXY_ADV_UPD_REASON_TIMEOUT = 0,
    /// Stopped due to state update
    MESH_PROXY_ADV_UPD_REASON_STATE,
    /// User request
    MESH_PROXY_ADV_UPD_REASON_USER,
    /// Peer request
    MESH_PROXY_ADV_UPD_REASON_PEER,
    /// Started due to provisioning using PB-GATT
    MESH_PROXY_ADV_UPD_REASON_PROV,
    /// Disconnection
    MESH_PROXY_ADV_UPD_REASON_DISC,
};

/// Group events for communication between Binding Manager and a model
enum mesh_mdl_grp_event
{
    /// New transition has been rejected by the main model, sent to bound model
    MESH_MDL_GRP_EVENT_TRANS_REJECTED = 0,
    /// Delay period has expired, sent to main model
    MESH_MDL_GRP_EVENT_TRANS_DELAY_EXPIRED,
    /// New Transition has been started by the main model
    MESH_MDL_GRP_EVENT_TRANS_STARTED,
    /// Immediate transmission has been started by the main model
    MESH_MDL_GRP_EVENT_TRANS_IMMEDIATE,
    /// Transition stopped after transition time
    MESH_MDL_GRP_EVENT_TRANS_END,
    /// Transition aborted
    MESH_MDL_GRP_EVENT_TRANS_ABORTED,

    /// All bound models have been registered
    MESH_MDL_GRP_EVENT_GROUP_FULL,
};

/// Mesh Error Codes
enum mesh_error
{
    /// No Error
    MESH_ERR_NO_ERROR                             = 0x0000,

    /* **************************************************************** */
    /* *                     PROTOCOL ERROR CODES                     * */
    /* **************************************************************** */

    /// Invalid Address
    MESH_ERR_INVALID_ADDR                         = MESH_ERR_(PROTOCOL, 0x01),
    /// Invalid_Model
    MESH_ERR_INVALID_MODEL                        = MESH_ERR_(PROTOCOL, 0x02),
    /// Invalid AppKey Index
    MESH_ERR_INVALID_APPKEY_ID                    = MESH_ERR_(PROTOCOL, 0x03),
    /// Invalid NetKey Index
    MESH_ERR_INVALID_NETKEY_ID                    = MESH_ERR_(PROTOCOL, 0x04),
    /// Insufficient_Resources
    MESH_ERR_INSUFFICIENT_RESOURCES               = MESH_ERR_(PROTOCOL, 0x05),
    /// Key Index Already Stored
    MESH_ERR_KEY_ID_ALREADY_STORED                = MESH_ERR_(PROTOCOL, 0x06),
    /// Invalid Publish Parameters
    MESH_ERR_INVALID_PUBLISH_PARAMS               = MESH_ERR_(PROTOCOL, 0x07),
    /// Not a Subscribe Model
    MESH_ERR_NOT_A_SUBSCRIBE_MODEL                = MESH_ERR_(PROTOCOL, 0x08),
    /// Storage Failure
    MESH_ERR_STORAGE_FAILURE                      = MESH_ERR_(PROTOCOL, 0x09),
    /// Feature Not Supported
    MESH_ERR_NOT_SUPPORTED                        = MESH_ERR_(PROTOCOL, 0x0A),
    /// Cannot Update
    MESH_ERR_CANNOT_UPDATE                        = MESH_ERR_(PROTOCOL, 0x0B),
    /// Cannot Remove
    MESH_ERR_CANNOT_REMOVE                        = MESH_ERR_(PROTOCOL, 0x0C),
    /// Cannot Bind
    MESH_ERR_CANNOT_BIND                          = MESH_ERR_(PROTOCOL, 0x0D),
    /// Temporarily Unable to Change State
    MESH_ERR_TEMP_UNABLE_TO_CHANGE_STATE          = MESH_ERR_(PROTOCOL, 0x0E),
    /// Cannot Set
    MESH_ERR_CANNOT_SET                           = MESH_ERR_(PROTOCOL, 0x0F),
    /// Unspecified Error
    MESH_ERR_UNSPECIFIED_ERROR                    = MESH_ERR_(PROTOCOL, 0x10),
    /// Invalid Binding
    MESH_ERR_INVALID_BINDING                      = MESH_ERR_(PROTOCOL, 0x11),
    /// Invalid Path Entry
    MESH_ERR_INVALID_PATH_ENTRY                   = MESH_ERR_(PROTOCOL, 0x12),
    /// Cannot Get
    MESH_ERR_CANNOT_GET                           = MESH_ERR_(PROTOCOL, 0x13),
    /// Obsolete Information
    MESH_ERR_OBSOLETE_INFO                        = MESH_ERR_(PROTOCOL, 0x14),
    /// Invalid Bearer
    MESH_ERR_INVALID_BEARER                       = MESH_ERR_(PROTOCOL, 0x15),

    /* **************************************************************** */
    /* *                     PROVISIONING ERROR CODES                 * */
    /* **************************************************************** */

    /// Prohibited
    MESH_ERR_PROV_PROHIBITED                      = MESH_ERR_(PROVISIONING, 0x00),
    /// The provisioning protocol PDU is not recognized by the device
    MESH_ERR_PROV_INVALID_PDU                     = MESH_ERR_(PROVISIONING, 0x01),
    /// The arguments of the protocol PDUs are outside expected values or the length of the PDU is
    /// different than expected
    MESH_ERR_PROV_INVALID_FORMAT                  = MESH_ERR_(PROVISIONING, 0x02),
    /// The PDU received was not expected at this moment of the procedure
    MESH_ERR_PROV_UNEXPECTED_PDU                  = MESH_ERR_(PROVISIONING, 0x03),
    /// The computed confirmation value was not successfully verified
    MESH_ERR_PROV_CONFIRMATION_FAILED             = MESH_ERR_(PROVISIONING, 0x04),
    /// The provisioning protocol cannot be continued due to insufficient resources in the device
    MESH_ERR_PROV_OUT_OF_RESOURCES                = MESH_ERR_(PROVISIONING, 0x05),
    /// The Data block was not successfully decrypted
    MESH_ERR_PROV_DECRYPTION_FAILED               = MESH_ERR_(PROVISIONING, 0x06),
    /// An unexpected error occurred that may not be recoverable
    MESH_ERR_PROV_UNEXPECTED                      = MESH_ERR_(PROVISIONING, 0x07),
    /// The device cannot assign consecutive unicast addresses to all elements
    MESH_ERR_PROV_CANNOT_ASSIGN_ADDR              = MESH_ERR_(PROVISIONING, 0x08),

    /* **************************************************************** */
    /* *                     INTERNAL ERROR CODES                     * */
    /* **************************************************************** */
    /// Invalid Parameter
    MESH_ERR_INVALID_PARAM                        = MESH_ERR_(INTERNAL, 0x01),
    /// Command Disallowed
    MESH_ERR_COMMAND_DISALLOWED                   = MESH_ERR_(INTERNAL, 0x02),
    /// MIC Error
    MESH_ERR_MIC_ERROR                            = MESH_ERR_(INTERNAL, 0x03),
    /// Resource requested is busy
    MESH_ERR_BUSY                                 = MESH_ERR_(INTERNAL, 0x04),
    /// Request time value is past
    MESH_ERR_TIME_PAST                            = MESH_ERR_(INTERNAL, 0x05),
    /// Resource requested not found
    MESH_ERR_NOT_FOUND                            = MESH_ERR_(INTERNAL, 0x06),
    /// Sequence number error
    MESH_ERR_SEQ_ERROR                            = MESH_ERR_(INTERNAL, 0x07),
    /// Bearer instance has been closed
    MESH_ERR_BEARER_CLOSED                        = MESH_ERR_(INTERNAL, 0x08),
    /// Provisioning Failed
    MESH_ERR_PROVISIONING_FAILED                  = MESH_ERR_(INTERNAL, 0x09),
    /// Provisioning timeout - Transaction or Link timeout
    MESH_ERR_PROVISIONING_TIMEOUT                 = MESH_ERR_(INTERNAL, 0x0A),
    /// Failed to access ECDH - Critical error
    MESH_ERR_ECDH_FAILED                          = MESH_ERR_(INTERNAL, 0x0B),
    /// Request has no effect
    MESH_ERR_NO_EFFECT                            = MESH_ERR_(INTERNAL, 0x0C),
    /// Cannot fragment message due to lack of ressources
    MESH_ERR_CANNOT_FRAGMENT                      = MESH_ERR_(INTERNAL, 0x0D),

    /* **************************************************************** */
    /* *                  LOW POWER NODE ERROR CODES                  * */
    /* **************************************************************** */
    /// Establishment failed after several attempts
    MESH_ERR_LPN_ESTAB_FAILED                     = MESH_ERR_(LPN, 0x01),
    /// Establishment failed due to failure during generation of friend keys
    MESH_ERR_LPN_ESTAB_FAILED_KEY                 = MESH_ERR_(LPN, 0x02),
    /// Establishment failed because Friend Update message not received after transmission of Friend Poll
    MESH_ERR_LPN_ESTAB_FAILED_UPD                 = MESH_ERR_(LPN, 0x03),
    /// Friendship stopped due to local request
    MESH_ERR_LPN_FRIEND_LOST_LOCAL                = MESH_ERR_(LPN, 0x04),
    /// Friendship lost due to request timeout
    MESH_ERR_LPN_FRIEND_LOST_TIMEOUT              = MESH_ERR_(LPN, 0x05),

    /* **************************************************************** */
    /* *                      MODEL ERROR CODES                       * */
    /* **************************************************************** */
    /// Invalid Model Configuration
    MESH_ERR_MDL_INVALID_CFG                      = MESH_ERR_(MDL, 0x01),
    /// Invalid Model Identifier
    MESH_ERR_MDL_INVALID_MDL_ID                   = MESH_ERR_(MDL, 0x02),
    /// Invalid Opcode
    MESH_ERR_MDL_INVALID_OPCODE                   = MESH_ERR_(MDL, 0x03),
    /// Invalid model group local index
    MESH_ERR_MDL_INVALID_GROUP                    = MESH_ERR_(MDL, 0x04),
    /// Unknown model group
    MESH_ERR_MDL_UNKNOWN_GROUP                    = MESH_ERR_(MDL, 0x05),
    /// Model already part of the group
    MESH_ERR_MDL_MDL_ALREADY_IN_GROUP             = MESH_ERR_(MDL, 0x06),
    /// Group of model is full
    MESH_ERR_MDL_GROUP_FULL                       = MESH_ERR_(MDL, 0x07),
    /// One of the models has already been registered
    MESH_ERR_MDL_ALREADY_REGISTERED               = MESH_ERR_(MDL, 0x08),
    /// Provided Model Local Index is not valid
    MESH_ERR_MDL_INVALID_MDL_LID                  = MESH_ERR_(MDL, 0x09),
    /// Invalid role
    MESH_ERR_MDL_INVALID_ROLE                     = MESH_ERR_(MDL, 0x0A),
    /// Command is not available for the model
    MESH_ERR_MDL_COMMAND_NOT_AVAILABLE            = MESH_ERR_(MDL, 0x0B),

};



/*
 * TYPES
 ****************************************************************************************
 */

/// Local Identifier
typedef uint8_t m_lid_t;

/// @} MESH_DEFINES

#endif /* MESH_DEFINES_ */
