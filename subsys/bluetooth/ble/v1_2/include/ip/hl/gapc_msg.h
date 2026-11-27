/**
 ****************************************************************************************
 *
 * @file gapc_msg.h
 *
 * @brief Generic Access Profile Controller  Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPC_MSG_H_
#define GAPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup GAPC_MSG_API Message API
 * @ingroup GAPC_API
 * @brief Message API for GAP Client module
 * @details It handles messages from lower and higher layers related to an ongoing connection.
 * @{
 * @}
 ****************************************************************************************
 */

/// @addtogroup GAPC_MSG_ID_API Message Identifiers
/// @ingroup GAPC_MSG_API
/// @{
/// @}

/// @addtogroup GAPC_MSG_OPID_API Operation Identifiers
/// @ingroup GAPC_MSG_API
/// @{
/// @}

/// @addtogroup GAPC_MSG_ENUM_API Enumerations
/// @ingroup GAPC_MSG_API
/// @{
/// @}

/// @addtogroup GAPC_MSG_STRUCT_API Message Structures
/// @ingroup GAPC_MSG_API
/// @{
/// @}

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "gapc.h"
#include "gapc_sec.h"
#if (BLE_HOST_PRESENT)
#include "gapc_le.h"
#endif // (BLE_HOST_PRESENT)

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup GAPC_MSG_ID_API
/// @{

/// GAP Controller Task messages
/*@TRACE*/
enum gapc_msg_id
{
    /* Default event */
    /// Command Complete event
    /// see #gapc_proc_cmp_cb
    GAPC_CMP_EVT = MSG_ID(GAPC, 0x00u),
    /// Indication to the task that sends the unknown message
    GAPC_UNKNOWN_MSG_IND = MSG_ID(GAPC, 0x01u),

    /* Connection state information */
    /// Indicate that a LE connection has been established
    /// see #gapc_connection_req_cb.le_connection_req
    GAPC_LE_CONNECTION_REQ_IND = MSG_ID(GAPC, 0x02u),
    /// Indicate that a BT Classic connection has been established
    /// \if btdm
    /// see #gapc_connection_req_cb.bt_connection_req
    /// \endif
    GAPC_BT_CONNECTION_REQ_IND = MSG_ID(GAPC, 0x07u),
    /// Set specific link data configuration.
    GAPC_CONNECTION_CFM = MSG_ID(GAPC, 0x03u),
    /// Indicate that BT-Classic connection establishment on peripheral side is completed and L2CAP service can be used.
    /// \if btdm
    /// see #gapc_connection_req_cb.bt_periph_connection_estab
    /// \endif
    GAPC_BT_PERIPH_CONNECTION_ESTABLISHED_IND = MSG_ID(GAPC, 0x08u),
    /// Indicate that there is no more ATT bearer available
    /// see #gapc_connection_info_cb.no_more_att_bearer
    GAPC_NO_MORE_ATT_BEARER_IND = MSG_ID(GAPC, 0x04u),

    /* Link management command */
    /// Request disconnection of current link command.
    GAPC_DISCONNECT_CMD = MSG_ID(GAPC, 0x05u),
    /// Indicate that a link has been disconnected
    /// see #gapc_connection_info_cb.disconnected
    GAPC_DISCONNECT_IND = MSG_ID(GAPC, 0x06u),

    /* Connection info */
    /// Retrieve information command
    GAPC_GET_INFO_CMD = MSG_ID(GAPC, 0x10u),
    /// Peer device attribute DB info such as Device Name, Appearance or Slave Preferred Parameters
    GAPC_PEER_ATT_INFO_IND = MSG_ID(GAPC, 0x11u),
    /// Indication of peer version info
    GAPC_PEER_VERSION_IND = MSG_ID(GAPC, 0x12u),
    /// Indication of peer low energy features info
    GAPC_LE_PEER_FEATURES_IND = MSG_ID(GAPC, 0x13u),
    /// Indication of ongoing connection RSSI
    GAPC_RSSI_IND = MSG_ID(GAPC, 0x14u),
    /// Indication of ongoing connection Channel Map
    GAPC_LE_CHANNEL_MAP_IND = MSG_ID(GAPC, 0x15u),
    /// Peer device request local device info such as name, appearance or slave preferred parameters
    /// Application shall send #GAPC_GET_INFO_CFM message to accept or reject request
    /// (see #gapc_get_info_req_ind_t structure)
    GAPC_GET_INFO_REQ_IND = MSG_ID(GAPC, 0x16u),
    /// Message sent in response of #GAPC_GET_INFO_REQ_IND
    /// (see #gapc_get_info_cfm_t structure)
    GAPC_GET_INFO_CFM = MSG_ID(GAPC, 0x17u),
    /// Peer device request to modify local device info such as name or appearance
    GAPC_SET_INFO_REQ_IND = MSG_ID(GAPC, 0x18u),
    /// Local device accept or reject device info modification
    GAPC_SET_INFO_CFM = MSG_ID(GAPC, 0x19u),
    /// Indication of currently used channel selection algorithm
    /// see #gapc_le_channel_selection_algo_ind_t
    GAPC_LE_CHANNEL_SELECTION_ALGO_IND = MSG_ID(GAPC, 0x1Au),
    /// Indication of peer classic features info
    GAPC_BT_PEER_FEATURES_IND = MSG_ID(GAPC, 0x1Bu),
    #if (!HL_DEPRECATED_SERVICE_CHANGED)
    /// Send indication for Service Changed characteristic to a peer device\n
    /// (see #gapc_le_send_service_changed_cmd_t structure)
    GAPC_LE_SEND_SERVICE_CHANGED_CMD = MSG_ID(GAPC, 0x1Fu),
    #endif // (!HL_DEPRECATED_SERVICE_CHANGED)

    /* Connection parameters update */
    /// Perform update of connection parameters command
    GAPC_LE_UPDATE_PARAMS_CMD = MSG_ID(GAPC, 0x40u),
    /// Request of updating connection parameters indication
    GAPC_LE_UPDATE_PARAMS_REQ_IND = MSG_ID(GAPC, 0x41u),
    /// Master confirm or not that parameters proposed by slave are accepted or not
    GAPC_LE_UPDATE_PARAMS_CFM = MSG_ID(GAPC, 0x42u),
    /// Connection parameters updated indication
    GAPC_LE_PARAMS_UPDATED_IND = MSG_ID(GAPC, 0x43u),
    /// LE Set Data Length Command
    GAPC_LE_SET_PACKET_SIZE_CMD = MSG_ID(GAPC, 0x44u),
    /// LE Set Data Length Indication
    GAPC_LE_PACKET_SIZE_IND = MSG_ID(GAPC, 0x45u),
    /// Update LE Ping timeout value
    GAPC_SET_PING_TO_CMD = MSG_ID(GAPC, 0x46u),
    /// LE Ping timeout indication
    GAPC_PING_TO_VALUE_IND = MSG_ID(GAPC, 0x47u),
    /// LE Ping timeout expires indication
    GAPC_PING_TO_IND = MSG_ID(GAPC, 0x48u),
    /// Set the PHY configuration for current active link
    GAPC_LE_SET_PHY_CMD = MSG_ID(GAPC, 0x49u),
    /// Active link PHY configuration. Triggered when configuration is read or during an update.
    GAPC_LE_PHY_IND = MSG_ID(GAPC, 0x4Au),
    /// Set the preferred slave latency (for slave only, with RW controller)
    GAPC_LE_SET_PREFERRED_PERIPH_LATENCY_CMD = MSG_ID(GAPC, 0x4Bu),
    /// Set the preferred slave event duration (for slave only, with RW controller)
    GAPC_LE_SET_PREFERRED_PERIPH_EVENT_DURATION_CMD = MSG_ID(GAPC, 0x4Cu),
    /// Set the maximum reception and size and time using DLE negotiation
    GAPC_LE_SET_MAX_RX_SIZE_AND_TIME_CMD = MSG_ID(GAPC, 0x4Du),
    /// Request update of the subrating parameters applied on a connection
    GAPC_LE_UPDATE_SUBRATE_CMD = MSG_ID(GAPC, 0x4Eu),
    /// Indicate subrating parameters applied on a given connection
    GAPC_LE_SUBRATE_IND = MSG_ID(GAPC, 0x4Fu),
    #if(GATT_INDICATE_LEGACY_MTU_CHANGED)
    /// Indicate that MTU change on Legacy Attribute bearer
    GAPC_LE_ATT_LEGACY_BEARER_MTU_CHANGED_IND = MSG_ID(GAPC, 0x20u),
    #endif // (GATT_INDICATE_LEGACY_MTU_CHANGED)

    /// Set priority elevation level of a LE connection
    GAPC_LE_SET_PRIORITY_ELEVATION_CMD = MSG_ID(GAPC, 0x90u),
    /// Change supported packet types command
    GAPC_BT_CHANGE_PACKET_TYPE_CMD = MSG_ID(GAPC, 0x91u),
    /// Supported packet types changed indication
    GAPC_BT_PACKET_TYPE_CHANGED_IND = MSG_ID(GAPC, 0x92u),
    /// Switch role command
    GAPC_BT_SWITCH_ROLE_CMD = MSG_ID(GAPC, 0x93u),
    /// Role changed indication
    GAPC_BT_ROLE_CHANGED_IND = MSG_ID(GAPC, 0x94u),
    /// Configure Sniff Mode command
    GAPC_BT_CONFIGURE_SNIFF_MODE_CMD = MSG_ID(GAPC, 0x95u),
    /// Mode changed indication
    GAPC_BT_MODE_CHANGED_IND = MSG_ID(GAPC, 0x96u),
    /// Setup QoS command
    GAPC_BT_SETUP_QOS_CMD = MSG_ID(GAPC, 0x97u),
    /// QoS Setup indication
    GAPC_BT_QOS_SETUP_IND = MSG_ID(GAPC, 0x98u),
    /// Set Tx Power command
    GAPC_LE_SET_TX_POWER_CMD = MSG_ID(GAPC, 0x99u),
    /// Set Tx Power indication
    GAPC_LE_SET_TX_POWER_IND = MSG_ID(GAPC, 0x9Au),
    /// Write link policy settings command
    GAPC_BT_WRITE_LINK_POLICY_SETTINGS_CMD = MSG_ID(GAPC, 0x9Bu),

    /* Bonding procedure */
    /// Start Bonding command procedure
    GAPC_LE_BOND_CMD = MSG_ID(GAPC, 0x50u),
    /// Bonding requested by peer device indication message.
    GAPC_BOND_REQ_IND = MSG_ID(GAPC, 0x51u),
    /// Confirm requested bond information.
    GAPC_BOND_CFM = MSG_ID(GAPC, 0x52u),
    /// Bonding information indication message
    GAPC_BOND_IND = MSG_ID(GAPC, 0x53u),
    /// Request to inform the remote device when keys have been entered or erased
    GAPC_LE_NOTIFY_KEY_PRESSED_CMD = MSG_ID(GAPC, 0x54u),
    /// Indication that a KeyPress has been performed on the peer device.
    GAPC_LE_KEY_PRESSED_IND = MSG_ID(GAPC, 0x55u),

    /* Security request procedure */
    /// Start Security Request command procedure
    GAPC_LE_REQUEST_SECURITY_CMD = MSG_ID(GAPC, 0x56u),
    /// Security requested by peer device indication message
    GAPC_LE_SECURITY_IND = MSG_ID(GAPC, 0x57u),

    /* Encryption procedure */
    /// Start LE Encryption command procedure
    /// see #gapc_le_encrypt
    GAPC_LE_ENCRYPT_CMD = MSG_ID(GAPC, 0x58u),
    /// LE Encryption requested by peer device indication message.
    /// see #gapc_security_cb.le_encrypt_req
    GAPC_LE_ENCRYPT_REQ_IND = MSG_ID(GAPC, 0x59u),
    /// Confirm requested LE Encryption information.
    /// see #gapc_le_encrypt_req_reply
    GAPC_LE_ENCRYPT_CFM = MSG_ID(GAPC, 0x5Au),


    /// Message triggered when LE or BT Classic link becomes encrypted
    /// see #gapc_security_cb.auth_info
    GAPC_ENCRYPT_IND = MSG_ID(GAPC, 0x5Bu),

    /* Bond Data information  */
    /// Indicate update of bond data information
    GAPC_BOND_DATA_UPDATE_IND = MSG_ID(GAPC, 0x5Cu),

    /* BT Classic Security  */
    /// Set BT classic connection required security level
    GAPC_BT_SET_REQUIRED_SECURITY_LEVEL_CMD = MSG_ID(GAPC, 0x5Du),

    /* Periodic Sync Transfer */
    /// Transfer periodic advertising sync information to peer device
    GAPC_LE_START_PAST_CMD = MSG_ID(GAPC, 0x60u),

    /* Client Features */
    /// Enable usage of supported client features
    GAPC_LE_ENABLE_CENTRAL_FEATURES_CMD = MSG_ID(GAPC, 0x61u),


    /* Constant Tone Extension */
    #if (BLE_CON_CTE_RSP)
    /// Constant Tone Extension Transmission configuration command
    GAPC_LE_CONFIGURE_CTE_TX_CMD = MSG_ID(GAPC, 0x70u),
    #endif // (BLE_CON_CTE_RSP)
    #if (BLE_CON_CTE_REQ)
    /// Constant Tone Extension Reception configuration command
    GAPC_LE_CONFIGURE_CTE_RX_CMD = MSG_ID(GAPC, 0x71u),
    /// Constant Tone Extension request control command (enable / disable)
    GAPC_LE_CONTROL_CTE_REQUEST_CMD = MSG_ID(GAPC, 0x72u),
    #endif // (BLE_CON_CTE_REQ)
    #if (BLE_CON_CTE_RSP)
    /// Constant Tone Extension Response control command (enable / disable)
    GAPC_LE_CONTROL_CTE_RESPONSE_CMD = MSG_ID(GAPC, 0x73u),
    #endif // (BLE_CON_CTE_RSP)
    #if (BLE_CON_CTE_REQ)
    /// Indicate reception of a IQ Report event over a ble connection
    GAPC_LE_CTE_IQ_REPORT_IND = MSG_ID(GAPC, 0x74u),
    /// Indicate that an IQ Request has been rejected or CTE data not present in LMP response
    GAPC_LE_CTE_REQUEST_FAILED_IND = MSG_ID(GAPC, 0x75u),
    #endif // (BLE_CON_CTE_REQ)

    /* LE Power Control */
    /// Local TX power indication
    GAPC_LE_LOCAL_TX_POWER_IND = MSG_ID(GAPC, 0x80u),
    /// Remote TX power indication
    GAPC_LE_PEER_TX_POWER_IND = MSG_ID(GAPC, 0x81u),
    /// Control TX Power Reports command
    GAPC_LE_CONTROL_TX_POWER_REPORT_CMD = MSG_ID(GAPC, 0x82u),
    /// Local TX power change report indication
    GAPC_LE_LOCAL_TX_POWER_REPORT_IND = MSG_ID(GAPC, 0x83u),
    /// Remote TX power change report indication
    GAPC_LE_PEER_TX_POWER_REPORT_IND = MSG_ID(GAPC, 0x84u),
    /// Control Path loss configuration
    GAPC_LE_CONTROL_PASS_LOSS_CMD = MSG_ID(GAPC, 0x85u),
    /// Path Loss Threshold Event Indication
    GAPC_LE_PATH_LOSS_THRESHOLD_IND = MSG_ID(GAPC, 0x86u),


};
/// @} GAPC_MSG_ID_API

/// @addtogroup GAPC_MSG_OPID_API
/// @{

/// request operation type - application interface
/*@TRACE*/
enum gapc_operation
{
    /*                 Operation Flags                  */
    /* No Operation (if nothing has been requested)     */
    /* ************************************************ */
    /// No operation
    GAPC_NO_OP = 0x00u,

    /* Connection management */
    /// Disconnect link
    GAPC_DISCONNECT = 0x01u,

    /* Connection information */
    /// Retrieve name of peer device.
    GAPC_GET_PEER_NAME = 0x10u,
    /// Retrieve peer device version info.
    GAPC_GET_PEER_VERSION = 0x11u,
    /// Retrieve peer device low energy features.
    GAPC_LE_GET_PEER_FEATURES = 0x12u,
    /// Get Peer device appearance
    GAPC_LE_GET_PEER_APPEARANCE = 0x13u,
    /// Get Peer device Peripheral Preferred Parameters
    GAPC_LE_GET_PEER_PERIPH_PREFERRED_PARAMS = 0x14u,
    /// Retrieve connection RSSI.
    GAPC_GET_RSSI = 0x15u,
    /// Retrieve Connection Channel MAP.
    GAPC_LE_GET_CHANNEL_MAP = 0x16u,
    /// Retrieve Channel Selection Algorithm
    GAPC_LE_GET_CHANNEL_SELECTION_ALGO = 0x17u,
    /// Get if Central Address resolution supported
    GAPC_LE_GET_PEER_ADDR_RESOLUTION_SUPPORTED = 0x18u,
    /// Retrieve Peer database Hash value
    GAPC_LE_GET_PEER_DB_HASH = 0x19u,
    /// get timer timeout value
    GAPC_GET_PING_TO = 0x1Au,
    /// Retrieve PHY configuration of active link
    GAPC_LE_GET_PHY = 0x1Bu,
    /// Read the local current and maximum transmit power levels for 1M PHY
    GAPC_LE_GET_LOCAL_TX_POWER_LEVEL_1M = 0x1Cu,
    /// Read the local current and maximum transmit power levels for 2M PHY
    GAPC_LE_GET_LOCAL_TX_POWER_LEVEL_2M = 0x1Du,
    /// Read the local current and maximum transmit power levels for LE CODED PHY with S=8 data coding
    GAPC_LE_GET_LOCAL_TX_POWER_LEVEL_LE_CODED_S8 = 0x1Eu,
    /// Read the local current and maximum transmit power levels for LE CODED PHY with S=2 data coding
    GAPC_LE_GET_LOCAL_TX_POWER_LEVEL_LE_CODED_S2 = 0x1Fu,
    /// Retrieve peer device classic features.
    GAPC_BT_GET_PEER_FEATURES = 0x90u,

    /// Read the transmit power level used by the remote Controller for 1M PHY
    GAPC_LE_GET_PEER_TX_POWER_LEVEL_1M = 0x20u,
    /// Read the transmit power level used by the remote Controller for 2M PHY
    GAPC_LE_GET_PEER_TX_POWER_LEVEL_2M = 0x21u,
    /// Read the transmit power level used by the remote Controller for LE CODED PHY with S=8 data coding
    GAPC_LE_GET_PEER_TX_POWER_LEVEL_LE_CODED_S8 = 0x22u,
    /// Read the transmit power level used by the remote Controller for LE CODED PHY with S=2 data coding
    GAPC_LE_GET_PEER_TX_POWER_LEVEL_LE_CODED_S2 = 0x23u,
    /// Resolvable Private Address Only declaration
    GAPC_LE_GET_PEER_RPA_ONLY = 0x24u,
    #if (!HL_DEPRECATED_SERVICE_CHANGED)
    /// Send indication for Service Changed characteristic to a peer device
    GAPC_LE_SEND_SERVICE_CHANGED = 0x27u,
    #endif // (!HL_DEPRECATED_SERVICE_CHANGED)

    /* Connection parameters update */
    /// Perform update of connection parameters.
    GAPC_LE_UPDATE_PARAMS = 0x40u,
    /// Set LE Ping timeout value or Authentication Payload timeout value
    GAPC_SET_PING_TO = 0x41u,
    /// LE Set Data Length
    GAPC_LE_SET_PACKET_SIZE = 0x42u,
    /// Set the PHY configuration for current active link
    GAPC_LE_SET_PHY = 0x43u,
    /// Set the preferred peripheral latency (for peripheral only, with RW controller)
    GAPC_LE_SET_PREFERRED_PERIPH_LATENCY = 0x44u,
    /// Set the preferred peripheral event duration (for peripheral only, with RW controller)
    GAPC_LE_SET_PREFERRED_PERIPH_EVENT_DURATION = 0x45u,
    /// Set maximum RX size and time using DLE negotiation
    GAPC_LE_SET_MAX_RX_SIZE_AND_TIME = 0x46u,
    /// Request update of the subrating parameters applied on a connection
    GAPC_LE_UPDATE_SUBRATE = 0x47u,
    /// Set priority elevation level of a LE connection
    GAPC_LE_SET_PRIORITY_ELEVATION = 0x48u,
    /// Change supported packet types
    GAPC_BT_CHANGE_PACKET_TYPE = 0x49u,
    /// Switch role
    GAPC_BT_SWITCH_ROLE = 0x4Au,
    /// Enter Sniff Mode
    GAPC_BT_SNIFF_MODE_ENTER = 0x4Bu,
    /// Exit Sniff Mode
    GAPC_BT_SNIFF_MODE_EXIT = 0x4Cu,
    /// Setuo QoS
    GAPC_BT_SETUP_QOS = 0x4Du,
    /// Set Tx Power command
    GAPC_LE_SET_TX_POWER = 0x4Eu,
    /// Write Link Policy Settings
    GAPC_BT_WRITE_LINK_POLICY_SETTINGS = 0x4Fu,

    /* Security procedures */
    /// Start bonding procedure.
    GAPC_LE_BOND = 0x50u,
    /// Start encryption procedure.
    GAPC_LE_ENCRYPT = 0x51u,
    /// Start security request procedure
    GAPC_LE_REQUEST_SECURITY = 0x52u,
    /// Request to inform the remote device when keys have been entered or erased
    GAPC_LE_NOTIFY_KEY_PRESSED = 0x53u,
    /// Set BT classic connection required security level operation
    GAPC_BT_SET_REQUIRED_SECURITY_LEVEL = 0x54u,

    #if (BLE_PAST)
    /* Periodic Sync Transfer */
    /// Transfer periodic advertising sync information to peer device
    GAPC_LE_START_PAST = 0x60u,
    #endif // (BLE_PAST)

    #if (!(0))
    /* Client Features */
    /// Enable usage of supported client features
    GAPC_LE_ENABLE_CENTRAL_FEATURES = 0x61u,
    #endif // (!(0))

    /* Constant Tone Extension */
    #if (BLE_CON_CTE_RSP)
    /// Constant Tone Extension Transmission configuration
    GAPC_LE_CONFIGURE_CTE_TX = 0x70u,
    #endif // (BLE_CON_CTE_RSP)
    #if (BLE_CON_CTE_REQ)
    /// Constant Tone Extension Reception configuration
    GAPC_LE_CONFIGURE_CTE_RX = 0x71u,
    /// Constant Tone Extension request control (enable / disable)
    GAPC_LE_CONTROL_CTE_REQUEST = 0x72u,
    #endif // (BLE_CON_CTE_REQ)
    #if (BLE_CON_CTE_RSP)
    /// Constant Tone Extension Response control (enable / disable)
    GAPC_LE_CONTROL_CTE_RESPONSE = 0x73u,
    #endif // (BLE_CON_CTE_RSP)

    /* LE Power Control */
    /// Enable or disable the reporting to the local Host of transmit power level
    /// changes in the local and remote Controllers for the ACL connection
    GAPC_LE_CONTROL_TX_POWER_REPORT = 0x80u,
    /// Command is used to enable/disable path loss reporting for the connection
    GAPC_LE_CONTROL_PATH_LOSS = 0x81u,




};
/// @} GAPC_MSG_OPID_API

/// @addtogroup GAPC_MSG_ENUM_API
/// @{

/// Bond event type.
/*@TRACE*/
enum gapc_bond
{
    /// Bond Pairing request
    GAPC_PAIRING_REQ = 0u,
    /// Respond to Pairing request
    GAPC_PAIRING_RSP,

    /// Pairing Finished information
    GAPC_PAIRING_SUCCEED,
    /// Pairing Failed information
    GAPC_PAIRING_FAILED,

    /// Used to retrieve pairing Temporary Key
    GAPC_TK_EXCH,
    /// Used for Identity Resolving Key exchange
    GAPC_IRK_EXCH,
    /// Used for Identity Resolving Key exchange to overwrite device identity
    GAPC_IRK_WITH_ID_EXCH,
    /// Used for Connection Signature Resolving Key exchange
    GAPC_CSRK_EXCH,
    /// Used for Long Term Key exchange
    GAPC_LTK_EXCH,

    /// Bond Pairing request issue, Repeated attempt
    GAPC_REPEATED_ATTEMPT,

    /// Out of Band - exchange of confirm and rand.
    GAPC_OOB_EXCH,

    /// Numeric Comparison - Exchange of Numeric Value -
    GAPC_NC_EXCH,

    /// BT Classic IO Capabilities
    GAPC_BT_IOCAP,
    /// BT Classic PIN Code
    GAPC_BT_PIN_CODE,
    /// BT Classic user value Confirm
    GAPC_BT_USER_VALUE_CFM,
    /// BT Classic passkey value
    GAPC_BT_PASSKEY,
    /// BT Classic pairing end status
    GAPC_BT_PAIRING_END,
    /// BT Classic link authentication information
    GAPC_BT_AUTH_INFO,
    /// BT Classic link key generated
    GAPC_BT_LINK_KEY,
};

/// List of available device information values
/*@TRACE*/
enum gapc_dev_info
{
    /// Device Name
    GAPC_DEV_NAME = 0u,
    /// Device Appearance
    GAPC_DEV_APPEARANCE = 1u,
    /// Device Slave preferred parameters
    GAPC_DEV_SLV_PREF_PARAMS = 2u,
    /// Device Central address resolution
    GAPC_DEV_CTL_ADDR_RESOL = 3u,
    /// Device database hash value
    GAPC_DEV_DB_HASH = 4,
    /// Resolvable Private address only after bond
    GAPC_DEV_RSLV_PRIV_ADDR_ONLY = 5u,
    /// Client supported features
    GAPC_DEV_CLI_SUP_FEAT = 8u,
    /// Server supported features
    GAPC_DEV_SRV_SUP_FEAT = 9u,

    /// Maximum device information value
    GAPC_DEV_INFO_MAX,
};

/// @} GAPC_MSG_ENUM_API

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_MSG_STRUCT_API
/// @{

/// Operation command structure in order to keep requested operation.
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type
    uint8_t operation;
} gapc_operation_cmd_t;

/// Command complete event data structure
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type
    uint8_t operation;
    /// Status of the request
    uint16_t status;
} gapc_cmp_evt_t;

/// Indicate that an unknown message has been received
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Unknown message id
    uint16_t unknown_msg_id;
} gapc_unknown_msg_ind_t;

/// Set specific link data configuration.
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Bond data
    gapc_bond_data_t bond_data;
} gapc_connection_cfm_t;

/// Parameters of #GAPC_NO_MORE_ATT_BEARER_IND message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
} gapc_no_more_att_bearer_ind_t;

/// Request disconnection of current link command.
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_DISCONNECT: Disconnect link.
    uint8_t operation;
    /// Reason of disconnection (see #hl_err)
    /// Only following error codes should be used
    /// - #LL_ERR_AUTH_FAILURE
    /// - #LL_ERR_REMOTE_USER_TERM_CON
    /// - #LL_ERR_REMOTE_DEV_TERM_LOW_RESOURCES
    /// - #LL_ERR_REMOTE_DEV_POWER_OFF
    /// - #LL_ERR_UNSUPPORTED_REMOTE_FEATURE
    /// - #LL_ERR_PAIRING_WITH_UNIT_KEY_NOT_SUP
    /// - #LL_ERR_UNACCEPTABLE_CONN_PARAM
    uint16_t reason;
} gapc_disconnect_cmd_t;

/// Indicate that a link has been disconnected
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Connection handle
    uint16_t conhdl;
    /// Reason of disconnection (see #hl_err)
    uint16_t reason;
} gapc_disconnect_ind_t;

/// Retrieve information command
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_GET_PEER_VERSION: Retrieve peer device version info.
    ///   (correspond to #gapc_get_peer_version function)
    /// - #GAPC_LE_GET_PEER_FEATURES: Retrieve peer device low energy features.
    ///   (correspond to #gapc_le_get_peer_features function)
    /// - #GAPC_GET_RSSI: Retrieve connection RSSI.
    ///   (correspond to #gapc_get_rssi function)
    /// - #GAPC_LE_GET_CHANNEL_MAP: Retrieve Connection Channel MAP.
    ///   (correspond to #gapc_le_get_channel_map function)
    /// - #GAPC_LE_GET_CHANNEL_SELECTION_ALGO: Retrieve Channel Selection Algorithm
    ///   (correspond to #gapc_le_get_channel_selection_algo function)
    /// - #GAPC_GET_PING_TO: Retrieve LE Ping Timeout Value
    /// - #GAPC_LE_GET_PHY: Retrieve PHY configuration of active link
    ///   (correspond to #gapc_le_get_phy function)
    /// - #GAPC_GET_PEER_NAME: Retrieve name of peer device
    ///   (correspond to #gapc_get_peer_name function)
    /// - #GAPC_LE_GET_PEER_DB_HASH: Retrieve Peer database Hash value
    ///   (correspond to #gapc_le_get_peer_db_hash function)
    /// - #GAPC_LE_GET_PEER_APPEARANCE: Get Peer device appearance
    ///   (correspond to #gapc_le_get_peer_appearance function)
    /// - #GAPC_LE_GET_PEER_PERIPH_PREFERRED_PARAMS: Get Peer device Slaved Preferred Parameters
    ///   (correspond to #gapc_le_get_periph_preferred_params function)
    /// - #GAPC_LE_GET_PEER_ADDR_RESOLUTION_SUPPORTED: Address Resolution Supported
    ///   (correspond to #gapc_le_get_peer_addr_resolution_supported function)
    /// - #GAPC_LE_GET_PEER_RPA_ONLY: Resolvable Private Address Only declaration
    ///   (correspond to #gapc_le_get_peer_rpa_only function)
    /// - Get Local TX power level (correspond to #gapc_le_get_local_tx_power_level function)
    ///     - #GAPC_LE_GET_LOCAL_TX_POWER_LEVEL_1M: Read the current and maximum transmit power levels for 1M PHY
    ///     - #GAPC_LE_GET_LOCAL_TX_POWER_LEVEL_2M: Read the current and maximum transmit power levels for 2M PHY
    ///     - #GAPC_LE_GET_LOCAL_TX_POWER_LEVEL_LE_CODED_S8: Read the current and maximum transmit power levels for LE CODED PHY with S=8 data coding
    ///     - #GAPC_LE_GET_LOCAL_TX_POWER_LEVEL_LE_CODED_S2: Read the current and maximum transmit power levels for LE CODED PHY with S=2 data coding
    /// - Get Peer TX power level (correspond to #gapc_le_get_peer_tx_power_level function)
    ///     - #GAPC_LE_GET_PEER_TX_POWER_LEVEL_1M: Read the transmit power level used by the remote Controller for 1M PHY
    ///     - #GAPC_LE_GET_PEER_TX_POWER_LEVEL_2M: Read the transmit power level used by the remote Controller for 2M PHY
    ///     - #GAPC_LE_GET_PEER_TX_POWER_LEVEL_LE_CODED_S8: Read the transmit power level used by the remote Controller for LE CODED PHY with S=8 data coding
    ///     - #GAPC_LE_GET_PEER_TX_POWER_LEVEL_LE_CODED_S2: Read the transmit power level used by the remote Controller for LE CODED PHY with S=2 data coding
    uint8_t operation;
} gapc_get_info_cmd_t;

/// Device information value union
/*@TRACE
 @trc_ref gapc_dev_info*/
typedef union gapc_info_val
{
    /// Device name
    //@trc_union parent.req == GAPC_DEV_NAME
    gap_device_name_t name;
    /// Appearance
    //@trc_union parent.req == GAPC_DEV_APPEARANCE
    uint16_t appearance;
    #if (BLE_HOST_PRESENT)
    /// Slave preferred parameters
    //@trc_union parent.req == GAPC_DEV_SLV_PREF_PARAMS
    gapc_le_preferred_periph_param_t slv_pref_params;
    #endif // (BLE_HOST_PRESENT)
    /// Central address resolution
    //@trc_union parent.req == GAPC_DEV_CTL_ADDR_RESOL
    uint8_t ctl_addr_resol;
    /// Database Hash value
    //@trc_union parent.req == GAPC_DEV_DB_HASH
    uint8_t hash[GATT_DB_HASH_LEN];
    /// Resolvable Private address only
    //@trc_union parent.req == GAPC_DEV_RSLV_PRIV_ADDR_ONLY
    uint8_t rslv_priv_addr_only;
} gapc_info_val_u;

/// Parameters of #GAPC_PEER_ATT_INFO_IND message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Requested information (see #gapc_dev_info enumeration)
    /// - #GAPC_DEV_NAME: Device Name
    #if (!(0))
    /// - #GAPC_DEV_APPEARANCE: Device Appearance
    /// - #GAPC_DEV_SLV_PREF_PARAMS: Device Slave preferred parameters
    /// - #GAPC_DEV_CTL_ADDR_RESOL: Address resolution supported
    /// - #GAPC_DEV_DB_HASH: Device Database Hash value
    /// - #GAPC_DEV_RSLV_PRIV_ADDR_ONLY: Resolvable Private Address Only declaration
    /// - #GAPC_DEV_CLI_SUP_FEAT: Client supported features
    /// - #GAPC_DEV_SRV_SUP_FEAT: Server supported features
    #endif // (!(0))
    uint8_t req;
    /// Attribute handle - #GATT_INVALID_HDL if not relevant\n
    /// Meaningless for BT >= 54
    uint16_t handle;
    /// Information value
    gapc_info_val_u info;
} gapc_peer_att_info_ind_t;

/// Indication of peer version info
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Manufacturer name
    uint16_t compid;
    /// LMP subversion
    uint16_t lmp_subvers;
    /// LMP version
    uint8_t lmp_vers;
} gapc_peer_version_ind_t;

/// Indication of ongoing connection RSSI
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// RSSI value
    int8_t rssi;
} gapc_rssi_ind_t;

/// Indication of LE Ping
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    ///Authenticated payload timeout
    uint16_t timeout;
} gapc_ping_to_value_ind_t;

/// Parameters of #GAPC_GET_INFO_REQ_IND message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Requested information (see #gapc_dev_info enumeration)
    /// - #GAPC_DEV_NAME: Device Name
    ///   (correspond to #gapc_connection_info_cb.name_get callback)
    /// - #GAPC_DEV_APPEARANCE: Device Appearance
    ///   (correspond to #gapc_connection_info_cb.appearance_get callback)
    /// - #GAPC_DEV_SLV_PREF_PARAMS: Device Slave preferred parameters
    ///   (correspond to #gapc_connection_info_cb.slave_pref_param_get callback)
    uint8_t req;
    /// Token value that must be returned in confirmation
    uint16_t token;
    /// Device Name data offset - relevant only for #GAPC_DEV_NAME
    uint16_t name_offset;
    /// Maximum name length (starting from offset) - relevant only for #GAPC_DEV_NAME
    uint16_t max_name_length;
} gapc_get_info_req_ind_t;

/// Parameters of #GAPC_GET_INFO_CFM message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Requested information (see #gapc_dev_info enumeration)
    /// - #GAPC_DEV_NAME: Device Name
    ///   (correspond to #gapc_le_get_name_cfm function)
    /// - #GAPC_DEV_APPEARANCE: Device Appearance
    ///   (correspond to #gapc_le_get_appearance_cfm function)
    /// - #GAPC_DEV_SLV_PREF_PARAMS: Device Slave preferred parameters
    ///   (correspond to #gapc_le_get_preferred_periph_params_cfm function)
    uint8_t req;
    /// Status code used to know if requested has been accepted or not
    uint16_t status;
    /// Token value provided in request indication
    uint16_t token;
    /// Complete value length including offset - relevant only for #GAPC_DEV_NAME
    uint16_t complete_length;
    /// Information value
    gapc_info_val_u info;
} gapc_get_info_cfm_t;


/// Parameters of #GAPC_SET_INFO_REQ_IND message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Requested information (see #gapc_dev_info enumeration)
    /// - #GAPC_DEV_NAME: Device Name
    /// - #GAPC_DEV_APPEARANCE: Device Appearance
    uint8_t req;
    /// Token value that must be returned in confirmation
    uint16_t token;
    /// Information value
    gapc_info_val_u info;
} gapc_set_info_req_ind_t;

/// Parameters of #GAPC_SET_INFO_CFM message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Requested information (see #gapc_dev_info enumeration)
    /// - #GAPC_DEV_NAME: Device Name
    /// - #GAPC_DEV_APPEARANCE: Device Appearance
    uint8_t req;
    /// Status code used to know if requested has been accepted or not
    uint16_t status;
    /// Token value provided in request indication
    uint16_t token;
} gapc_set_info_cfm_t;


#if (!HL_DEPRECATED_SERVICE_CHANGED)
/// Parameters of the #GAPC_LE_SEND_SERVICE_CHANGED_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP command type:
    /// - #GAPC_LE_SEND_SERVICE_CHANGED
    uint8_t operation;
    /// Start handle
    uint16_t shdl;
    /// End handle
    uint16_t ehdl;
} gapc_le_send_service_changed_cmd_t;
#endif // (!HL_DEPRECATED_SERVICE_CHANGED)

/// Bond procedure requested information data
/*@TRACE
 @trc_ref gapc_bond*/
typedef union gapc_u_bond_req_data
{
    /// Authentication level (#gap_auth) (if request = #GAPC_PAIRING_REQ)
    //@trc_union parent.request == GAPC_PAIRING_REQ
    uint8_t auth_req;
    /// LTK Key Size (if request = #GAPC_LTK_EXCH)
    //@trc_union parent.request == GAPC_LTK_EXCH
    uint8_t key_size;
    /// Device IO used to get TK: (if request = #GAPC_TK_EXCH)
    ///  - #GAP_TK_OOB:       TK get from out of band method
    ///  - #GAP_TK_DISPLAY:   TK generated and shall be displayed by local device
    ///  - #GAP_TK_KEY_ENTRY: TK shall be entered by user using device keyboard
    //@trc_union parent.request == GAPC_TK_EXCH
    uint8_t tk_type;

    /// Numeric value
    //@trc_union parent.request == GAPC_BT_USER_VALUE_CFM
    //@trc_union parent.request == GAPC_NC_EXCH
    uint32_t numeric_value;
} gapc_u_bond_req_data_t;

/// Bonding requested by peer device indication message.
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Bond request type (see #gapc_bond)
    uint8_t request;

    /// Bond procedure requested information data
    union gapc_u_bond_req_data data;
} gapc_bond_req_ind_t;


/// Pairing features
/*@TRACE*/
typedef struct
{
    /// Pairing information
    gapc_pairing_t pairing_info;
    /// Deprecated parameter reserved for future use
    /// To configure required security level, use #GAPM_LE_CONFIGURE_SECURITY_LEVEL_CMD.
    uint8_t sec_req_level;
} gapc_pairing_feat_t;

/// Confirmation message bond data union
/*@TRACE
 @trc_ref gapc_bond
*/
union gapc_bond_cfm_data
{
    /// Pairing Features (request = #GAPC_PAIRING_RSP)
    //@trc_union parent.request == GAPC_PAIRING_RSP
    gapc_pairing_feat_t pairing_feat;
    #if (BLE_HOST_PRESENT)
    /// LTK (request = #GAPC_LTK_EXCH)
    //@trc_union parent.request == GAPC_LTK_EXCH
    gapc_ltk_t ltk;
    /// CSRK (request = #GAPC_CSRK_EXCH)
    //@trc_union parent.request == GAPC_CSRK_EXCH
    gap_sec_key_t csrk;
    /// TK (request = #GAPC_TK_EXCH)
    //@trc_union parent.request == GAPC_TK_EXCH
    gap_sec_key_t tk;
    /// IRK (request = #GAPC_IRK_EXCH)
    //@trc_union parent.request == GAPC_IRK_EXCH
    //@trc_union parent.request == GAPC_IRK_WITH_ID_EXCH
    gapc_irk_t irk;
    #endif // (BLE_HOST_PRESENT)

    /// OOB Confirm and Random from the peer (request = #GAPC_OOB_EXCH)
    //@trc_union parent.request == GAPC_OOB_EXCH
    gap_oob_t oob;

    /// BT Classic Passkey value (request = #GAPC_BT_PASSKEY)
    //@trc_union parent.request == GAPC_BT_PASSKEY
    uint32_t passkey;
};

/// Confirm requested bond information.
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Bond request type (see #gapc_bond)
    uint8_t request;
    /// Request accepted
    uint8_t accept;

    /// Bond procedure information data
    union gapc_bond_cfm_data data;
} gapc_bond_cfm_t;

/**
 *  Pairing information
 */
/*@TRACE*/
typedef struct
{
    /// Pairing level information (#gap_pairing_lvl)
    uint8_t level;
    /// LTK exchanged during pairing.
    bool ltk_present;
    /// Type of combination key used during pairing (see #gapc_key_type enumeration)
    uint8_t key_type;
} gapc_pairing_info_t;


/// Bond procedure information data
/*@TRACE
 @trc_ref gapc_bond*/
typedef union gapc_u_bond_data
{
    /// Pairing information
    /// (if info = #GAPC_PAIRING_SUCCEED)
    //@trc_union parent.info == GAPC_PAIRING_SUCCEED
    gapc_pairing_info_t pairing;
    /// Pairing failed reason  (if info = #GAPC_PAIRING_FAILED)
    //@trc_union parent.info == GAPC_PAIRING_FAILED
    uint16_t reason;
    #if (BLE_HOST_PRESENT)
    /// Long Term Key information (if info = #GAPC_LTK_EXCH)
    //@trc_union parent.info == GAPC_LTK_EXCH
    gapc_ltk_t ltk;
    /// Identity Resolving Key information (if info = #GAPC_IRK_EXCH)
    //@trc_union parent.info == GAPC_IRK_EXCH
    gapc_irk_t irk;
    /// Connection Signature Resolving Key information (if info = #GAPC_CSRK_EXCH)
    //@trc_union parent.info == GAPC_CSRK_EXCH
    gap_sec_key_t csrk;
    #endif // (BLE_HOST_PRESENT)
    /// BT Classic Passkey numeric value to display (if info = #GAPC_BT_PASSKEY)
    //@trc_union parent.info == GAPC_BT_PASSKEY
    uint32_t passkey;
} gapc_u_bond_data_t;

/// Bonding information indication message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Bond information type (see #gapc_bond)
    uint8_t info;

    /// Bond procedure information data
    union gapc_u_bond_data data;
} gapc_bond_ind_t;

/// Parameters of the #GAPC_BOND_DATA_UPDATE_IND message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Updated bond data
    gapc_bond_data_updated_t data;
} gapc_bond_data_update_ind_t;

/// Encryption information indication message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Pairing level (#gap_pairing_lvl)
    uint8_t pairing_lvl;
    ///  Size of the encryption key (range [7-16])
    uint8_t key_size;
} gapc_encrypt_ind_t;

/// Parameters of the #GAPC_SET_PING_TO_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_SET_PING_TO : Set the LE Ping timeout value
    uint8_t operation;
    /// Authenticated payload timeout
    uint16_t timeout;
} gapc_set_ping_to_cmd_t;

/// Parameters of the #GAPC_PING_TO_IND message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
} gapc_ping_to_ind_t;

/// Parameters of the @ref GAPC_LE_SET_PRIORITY_ELEVATION_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_SET_PRIORITY_ELEVATION: Set priority elevation level of a LE connection
    uint8_t operation;
    /// Priority elevation (in units of increment)
    uint8_t elevation;
} gapc_le_set_priority_elevation_cmd_t;


/// @} GAPC_MSG_STRUCT_API

#endif /* GAPC_MSG_H_ */

