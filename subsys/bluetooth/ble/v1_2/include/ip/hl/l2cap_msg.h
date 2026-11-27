/**
 ****************************************************************************************
 *
 * @file l2cap_msg.h
 *
 * @brief Header file - L2CAP Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 ****************************************************************************************
 */

#ifndef L2CAP_MSG_H_
#define L2CAP_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup L2CAP_MSG_API Message API
 * @ingroup L2CAP_API
 * @brief Handles ALL messages to/from L2CAP block.
 *
 * @defgroup L2CAP_MSG_ID_API Message Identifiers
 * @ingroup L2CAP_MSG_API
 *
 * @defgroup L2CAP_MSG_STRUCT_API Message Structures
 * @ingroup L2CAP_MSG_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "l2cap.h"
#include "compiler.h"
#include <stdbool.h>

/*
 * MESSAGES
 ****************************************************************************************
 */

/// @addtogroup L2CAP_MSG_ID_API
/// @{

/// Message API of the L2CAP task
/*@TRACE*/
enum l2cap_msg_id
{
    /// L2CAP command Message (see #l2cap_cmd_t). Specialization of the structure depends on value of #l2cap_cmd_t.cmd_code.
    /// More information available in #l2cap_cmd_code enumeration. A command message is completed once #L2CAP_CMP_EVT message
    /// is received
    L2CAP_CMD = MSG_ID(L2CAP, 0x00),
    /// L2CAP command complete event message (see #l2cap_cmp_evt_t). Specialization of depends on value of #l2cap_cmp_evt_t.cmd_code.
    /// More information available in #l2cap_cmd_code enumeration
    L2CAP_CMP_EVT = MSG_ID(L2CAP, 0x01),
    /// L2CAP Indication message (see #l2cap_ind_t). Specialization of the structure depends on value of #l2cap_ind_t.ind_code.
    /// More information available in #l2cap_ind_code enumeration
    L2CAP_IND = MSG_ID(L2CAP, 0x02),
    /// L2CAP request indication message (see #l2cap_req_ind_t). Specialization of the structure depends on value of #l2cap_req_ind_t.req_ind_code.
    /// More information available in #l2cap_req_ind_code enumeration. A #L2CAP_REQ_IND must be confirm by API client using #L2CAP_CFM message.
    L2CAP_REQ_IND = MSG_ID(L2CAP, 0x03),
    /// L2CAP confirmation message (see #l2cap_req_ind_t). Specialization of the structure depends on value of #l2cap_cfm_t.req_ind_code.
    /// More information available in #l2cap_req_ind_code enumeration
    L2CAP_CFM = MSG_ID(L2CAP, 0x04),
};

/// #L2CAP_CMD command codes
/*@TRACE*/
enum l2cap_cmd_code
{
    /* Connection oriented channel                      */
    /* ************************************************ */
    /// Command used to register a Simplified Protocol/Service Multiplexer in order to accept L2CAP connection oriented
    /// channel (COC) from a peer device. Security level parameter ensures that a certain level of security is
    /// met before requesting application to accept or reject channel establishment.
    /// #l2cap_coc_spsm_add_cmd_t structure must be used to send command, #l2cap_coc_spsm_add_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #l2cap_coc_spsm_add)
    L2CAP_COC_SPSM_ADD = 0x0100,
    /// Command used to unregister a Simplified Protocol/Service Multiplexer.
    /// #l2cap_coc_spsm_remove_cmd_t structure must be used to send command, #l2cap_coc_spsm_remove_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #l2cap_coc_spsm_remove)
    L2CAP_COC_SPSM_REMOVE = 0x0101,
    /// Command use to initiate creation of several L2CAP connection oriented channels.
    /// The operation completes when all requested L2CAP channels are created or when an error is detected.
    /// If all channels cannot be created, reason is provided in error code.
    /// #l2cap_coc_create_cmd_t structure must be used to send command, #l2cap_coc_create_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #l2cap_coc_create)
    L2CAP_COC_CREATE = 0x0102,
    #if (EATT_SUPPORT)
    /// Command used to change reception MTU size for given L2CAP channels. New MTU size must be greater or equals
    /// to the previous reception MTU size of any channels.
    /// The operation completes when all L2CAP channels reception MTU size is modified.
    /// If all channels cannot be reconfigured, reason is provided in error code.
    /// #l2cap_coc_reconfigure_cmd_t structure must be used to send command, #l2cap_coc_reconfigure_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #l2cap_coc_reconfigure)
    L2CAP_COC_RECONFIGURE = 0x0103,
    #endif // (EATT_SUPPORT)

    /// Command use to initiate disconnection of a L2CAP connection oriented channel.
    /// #l2cap_coc_terminate_cmd_t structure must be used to send command, #l2cap_coc_terminate_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #l2cap_coc_terminate)
    L2CAP_COC_TERMINATE = 0x0104,

    /* Send PDU                                         */
    /* ************************************************ */
    /// Command used to send a SDU onto a specific L2CAP fixed or dynamic channel.
    /// @note Upper layer shall wait for #L2CAP_CMP_EVT indication before sending other message to prevent
    ///       buffer overflow on the device.
    ///
    /// In debug mode, only L2CAP segment can be transmitted. This can be start segment or continuation segment.
    /// L2CAP length and channel ID (CID) fields must be present in SDU data.
    /// #l2cap_sdu_send_cmd_t structure must be used to send command, #l2cap_sdu_send_cmd_t
    /// received once command is executed.
    ///
    /// (see #l2cap_chan_sdu_send)
    L2CAP_SDU_SEND = 0x0200,

    /* Debug only - Channel management                  */
    /* ************************************************ */
    /// Debug command used to register a fixed L2CAP channel such as ATT, Signaling or SMP. This channel does not support
    /// segmentation and reassembly mechanism.
    /// #l2cap_dbg_chan_fix_register_cmd_t structure must be used to send command, #l2cap_dbg_chan_fix_register_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #l2cap_chan_fix_register)
    L2CAP_DBG_CHAN_FIX_REGISTER = 0x0A01,
    /// Debug command use to register a dynamic L2CAP channel. This channel supports credit management, segmentation
    /// and reassembly mechanisms. To create a L2CAP Credit Based connection, #L2CAP_COC_CREATE command should
    /// be used instead.
    /// #l2cap_dbg_chan_dyn_register_cmd_t structure must be used to send command, #l2cap_dbg_chan_dyn_register_cmp_evt_t
    /// received once command is executed.
    L2CAP_DBG_CHAN_DYN_REGISTER = 0x0A02,
    /// Debug command used to unregister a dynamic or a fixed channel. After sending this command, it's no more
    /// possible to receive or send SDUs onto the channel.
    /// #l2cap_dbg_chan_unregister_cmd_t structure must be used to send command, #l2cap_dbg_chan_unregister_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #l2cap_chan_unregister)
    L2CAP_DBG_CHAN_UNREGISTER = 0x0A03,
    /// Debug command used to increment number of dynamic L2CAP channel transmission credits.
    /// #l2cap_dbg_chan_tx_credit_add_cmd_t structure must be used to send command, #l2cap_dbg_chan_tx_credit_add_cmp_evt_t
    /// received once command is executed.
    L2CAP_DBG_CHAN_TX_CREDIT_ADD = 0x0A04,
    /// Debug command used to configure or reconfigure a registered channel.
    /// #l2cap_dbg_chan_config_set_cmd_t structure must be used to send command, #l2cap_dbg_chan_config_set_cmp_evt_t
    /// received once command is executed.
    L2CAP_DBG_CHAN_CONFIG_SET = 0x0A05,
    /// Debug command used to retrieve information about a fixed or a dynamic channel.
    /// #l2cap_dbg_chan_info_get_cmd_t structure must be used to send command, #l2cap_dbg_coc_enhanced_nego_ctrl_cmp_evt_t
    /// received once command is executed.
    L2CAP_DBG_CHAN_INFO_GET = 0x0A06,
    /// Command Used to control usage of Enhanced L2CAP COC negotiations.
    /// #l2cap_dbg_coc_enhanced_nego_ctrl_cmd_t structure must be used to send command, #l2cap_dbg_chan_info_get_cmp_evt_t
    /// received once command is executed.
    L2CAP_DBG_COC_ENHANCED_NEGO_CTRL = 0x0900,
    #if (EATT_SUPPORT)
    /// Command used to change reception MTU and MPS size for given L2CAP channels. New MTU size must be greater
    /// or equals to the previous reception MTU size of any channels.
    /// The operation completes when all L2CAP channels reception MTU size is modified.
    /// If all channels cannot be reconfigured, reason is provided in error code.
    /// #l2cap_dbg_coc_reconfigure_cmd_t structure must be used to send command, #l2cap_dbg_coc_reconfigure_cmp_evt_t
    /// received once command is executed.
    L2CAP_DBG_COC_RECONFIGURE = 0x0901,
    #endif // (EATT_SUPPORT)
    /// Debug command used to control reception of ACL packet at L2CAP level. When reception is off, received packet
    /// are put in a reception queue till reception is enabled again. When BLE connection is established without
    /// confirmation from application, RX is disabled by default.
    /// #l2cap_dbg_rx_ctrl_cmd_t structure must be used to send command, #l2cap_dbg_rx_ctrl_cmp_evt_t
    /// received once command is executed.
    L2CAP_DBG_RX_CTRL = 0X0800,
};

/// #L2CAP_IND indication codes
/*@TRACE*/
enum l2cap_ind_code
{
    /// Event triggered when an unknown message has been received by L2CAP layer  from an upper layer.
    /// #l2cap_unknown_msg_ind_t received when event is triggered
    L2CAP_UNKNOWN_MSG = 0x0000,
    /// Event triggered when a L2CAP connection oriented channel is created.
    /// #l2cap_coc_created_ind_t received when event is triggered
    ///
    /// (see #l2cap_chan_coc_cb_t.cb_coc_created)
    L2CAP_COC_CREATED = 0x0100,
    #if (EATT_SUPPORT)
    /// Event triggered when a L2CAP connection oriented channel local or peer device reception MTU size is updated.
    /// #l2cap_coc_mtu_changed_ind_t received when event is triggered
    ///
    /// (see #l2cap_chan_coc_cb_t.cb_coc_mtu_changed)
    L2CAP_COC_MTU_CHANGED = 0x0101,
    #endif // (EATT_SUPPORT)
    /// Event triggered when a L2CAP connection oriented channel is terminated.
    /// #l2cap_coc_terminated_ind_t received when event is triggered
    ///
    /// (see #l2cap_chan_coc_cb_t.cb_coc_terminated)
    L2CAP_COC_TERMINATED = 0x0102,


    /* Debug only - Channel management                  */
    /* ************************************************ */
    /// Event triggered when an error is detected onto a L2CAP dynamic channel.
    /// #l2cap_dbg_chan_error_ind_t received when event is triggered
    L2CAP_DBG_CHAN_ERROR = 0x0A00,
    /// Event triggered when new reception credits are available for a L2CAP dynamic channel.
    /// #l2cap_dbg_chan_rx_credit_added_ind_t received when event is triggered
    L2CAP_DBG_CHAN_RX_CREDIT_ADDED = 0x0A01,
    /// Event triggered when transmission flow is paused onto a specific channel.
    /// #l2cap_dbg_chan_tx_flow_off_ind_t received when event is triggered
    L2CAP_DBG_CHAN_TX_FLOW_OFF = 0x0A02,
};

/// #L2CAP_REQ_IND request indication codes
/*@TRACE*/
enum l2cap_req_ind_code
{
    /* Connection oriented channel                      */
    /* ************************************************ */
    /// Inform Upper Layer that a L2CAP credit oriented connection is initiated by peer device.
    /// #l2cap_coc_connect_req_ind_t received when event is triggered, must be confirm using
    /// #l2cap_coc_connect_cfm_t message structure.
    ///
    /// (see #l2cap_coc_spsm_cb_t.cb_coc_connect_req and #l2cap_coc_connect_cfm)
    L2CAP_COC_CONNECT = 0x0100,

    /* SDU Reception                                    */
    /* ************************************************ */
    /// Inform Upper Layer about reception of SDU data over a L2CAP fixed or dynamically allocated channel.
    /// #l2cap_sdu_rx_req_ind_t received when event is triggered, must be confirm using
    /// #l2cap_sdu_rx_cfm_t message structure.
    ///
    /// (see #l2cap_chan_coc_cb_t.cb_sdu_rx or #l2cap_chan_cb_t.cb_sdu_rx)
    /// (#L2CAP_CFM message correspond to #co_buf_release action on native API)
    L2CAP_SDU_RX = 0x0200,
};

/// @} L2CAP_MSG_ID_API

/*
 * DEFAULT MESSAGE CONTENT
 ****************************************************************************************
 */

/// @addtogroup L2CAP_MSG_STRUCT_API
/// @{

/// Default #L2CAP_CMD command message structure
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
} l2cap_cmd_t;

/// Default #L2CAP_CMP_EVT command complete message structure
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
} l2cap_cmp_evt_t;

/// Extension of #l2cap_cmp_evt_t for #L2CAP_CMP_EVT related to a channel event
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index.
    uint8_t chan_lid;
} l2cap_chan_cmp_evt_t;

/// Default #L2CAP_IND indication message structure
/*@TRACE*/
typedef struct
{
    /// Indication code (see #l2cap_ind_code enumeration)
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO}), 0x0000 otherwise
    uint16_t metainfo;
} l2cap_ind_t;

/// Default #L2CAP_REQ_IND request indication message structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #l2cap_req_ind_code enumeration)
    uint16_t req_ind_code;
    /// Token provided by L2CAP module that must be used into the #L2CAP_CFM message
    uint16_t token;
} l2cap_req_ind_t;

/// Default #L2CAP_CFM confirm message structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #l2cap_req_ind_code enumeration)
    uint16_t req_ind_code;
    /// Token provided by L2CAP module into the #L2CAP_REQ_IND message
    uint16_t token;
} l2cap_cfm_t;

/*
 * MESSAGE CONTENT
 ****************************************************************************************
 */

/// #L2CAP_COC_SPSM_ADD Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_COC_SPSM_ADD
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
    /// Security level bit field (see #l2cap_sec_lvl_bf enumeration)
    uint8_t sec_lvl_bf;
} l2cap_coc_spsm_add_cmd_t;

/// #L2CAP_COC_SPSM_ADD uses default complete event structure
/*@TRACE*/
typedef l2cap_cmp_evt_t l2cap_coc_spsm_add_cmp_evt_t;

/// #L2CAP_COC_SPSM_REMOVE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_COC_SPSM_REMOVE
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} l2cap_coc_spsm_remove_cmd_t;

/// #L2CAP_COC_SPSM_REMOVE uses default complete event structure
/*@TRACE*/
typedef l2cap_cmp_evt_t l2cap_coc_spsm_remove_cmp_evt_t;

/// #L2CAP_COC_CREATE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_COC_CREATE
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;

    /// Connection Index
    uint8_t conidx;
    /// Number of L2CAP channel to create in parallel
    uint8_t nb_chan;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
    /// Local reception Maximum Transmit Unit Size
    uint16_t local_rx_mtu;
} l2cap_coc_create_cmd_t;

/// #L2CAP_COC_CREATE Complete event structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_COC_CREATE
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// Connection Index
    uint8_t conidx;
    /// Number of L2CAP channel created.
    uint8_t nb_chan;
} l2cap_coc_create_cmp_evt_t;

#if (EATT_SUPPORT)
/// #L2CAP_COC_RECONFIGURE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_COC_RECONFIGURE
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    ///  New Maximum Transmit Size used for reception
    uint16_t local_rx_mtu;
    /// Number of L2CAP channel to reconfigure
    uint8_t nb_chan;
    /// Array that contains list of L2CAP channel identifier to reconfigure
    uint8_t chan_lid[__ARRAY_EMPTY];
} l2cap_coc_reconfigure_cmd_t;

/// #L2CAP_COC_RECONFIGURE uses command complete event structure
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// Connection Index
    uint8_t conidx;
} l2cap_coc_reconfigure_cmp_evt_t;
#endif // (EATT_SUPPORT)

/// #L2CAP_COC_TERMINATE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_COC_TERMINATE
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index
    uint8_t chan_lid;
} l2cap_coc_terminate_cmd_t;

/// #L2CAP_COC_TERMINATE uses default channel command complete event structure
/*@TRACE*/
typedef l2cap_chan_cmp_evt_t l2cap_coc_terminate_cmp_evt_t;

/// #L2CAP_SDU_SEND Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_SDU_SEND
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index
    uint8_t chan_lid;
    /// Debug bit field (see #l2cap_dbg_bf enumeration)
    uint8_t dbg_bf;
    /// SDU Length
    uint16_t length;
    /// SDU Data
    uint8_t data[__ARRAY_EMPTY];
} l2cap_sdu_send_cmd_t;

/// #L2CAP_SDU_SEND uses default channel command complete event structure
/*@TRACE*/
typedef l2cap_chan_cmp_evt_t l2cap_sdu_send_cmp_evt_t;

/// #L2CAP_DBG_CHAN_FIX_REGISTER Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_DBG_CHAN_FIX_REGISTER
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP Channel Identifier for both reception and transmission
    uint16_t cid;
    /// Maximum Transmit Unit Size for both reception and transmission
    uint16_t mtu;
} l2cap_dbg_chan_fix_register_cmd_t;

/// #L2CAP_DBG_CHAN_FIX_REGISTER uses default channel command complete event structure
/*@TRACE*/
typedef l2cap_chan_cmp_evt_t l2cap_dbg_chan_fix_register_cmp_evt_t;

/// #L2CAP_DBG_CHAN_DYN_REGISTER Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_DBG_CHAN_DYN_REGISTER
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// Reception L2CAP Channel Identifier
    uint16_t rx_cid;
    /// Transmission L2CAP Channel Identifier
    uint16_t tx_cid;
    /// Reception Maximum Transmit Unit Size
    uint16_t rx_mtu;
    /// Transmission Maximum Transmit Unit Size
    uint16_t tx_mtu;
    /// Reception Maximum Packet Size
    uint16_t rx_mps;
    /// Transmission Maximum Packet Size
    uint16_t tx_mps;
    /// Reception credit number
    uint16_t rx_credit;
    /// Transmission credit number
    uint16_t tx_credit;
} l2cap_dbg_chan_dyn_register_cmd_t;

/// #L2CAP_DBG_CHAN_DYN_REGISTER uses default channel command complete event structure
/*@TRACE*/
typedef l2cap_chan_cmp_evt_t l2cap_dbg_chan_dyn_register_cmp_evt_t;

/// #L2CAP_DBG_CHAN_UNREGISTER Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_DBG_CHAN_UNREGISTER
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index.
    uint8_t chan_lid;
} l2cap_dbg_chan_unregister_cmd_t;

/// #L2CAP_DBG_CHAN_UNREGISTER uses default channel command complete event structure
/*@TRACE*/
typedef l2cap_chan_cmp_evt_t l2cap_dbg_chan_unregister_cmp_evt_t;

/// #L2CAP_DBG_CHAN_TX_CREDIT_ADD Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_DBG_CHAN_TX_CREDIT_ADD
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index.
    uint8_t chan_lid;
    /// Number of credit to add for SDU transmission
    uint16_t credit;
} l2cap_dbg_chan_tx_credit_add_cmd_t;

/// #L2CAP_DBG_CHAN_TX_CREDIT_ADD uses default channel command complete event structure
/*@TRACE*/
typedef l2cap_chan_cmp_evt_t l2cap_dbg_chan_tx_credit_add_cmp_evt_t;

/// #L2CAP_DBG_CHAN_CONFIG_SET Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_DBG_CHAN_CONFIG_SET
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index.
    uint8_t chan_lid;
    /// Updated parameters Bit field  (see #l2cap_chan_upd_bf enumeration)
    uint8_t update_bf;
    /// Configuration bit field (see l2cap_chan_cfg_bf enumeration)
    uint8_t config_bf;
    /// New reception MTU
    uint16_t rx_mtu;
    /// New transmission MTU
    /// @note setting ignored for a fixed channel.
    uint16_t tx_mtu;
    /// New reception MPS
    /// @note setting ignored for a fixed channel.
    uint16_t rx_mps;
    /// New transmission MPS
    /// @note setting ignored for a fixed channel.
    uint16_t tx_mps;
} l2cap_dbg_chan_config_set_cmd_t;

/// #L2CAP_DBG_CHAN_CONFIG_SET uses channel command complete event structure
/*@TRACE*/
typedef l2cap_chan_cmp_evt_t l2cap_dbg_chan_config_set_cmp_evt_t;

/// #L2CAP_DBG_CHAN_INFO_GET Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_DBG_CHAN_INFO_GET
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index.
    uint8_t chan_lid;
} l2cap_dbg_chan_info_get_cmd_t;

/// #L2CAP_DBG_CHAN_INFO_GET Complete event structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index.
    uint8_t chan_lid;
    /// Channel type (see #l2cap_chan_type enumeration)
    uint8_t chan_type;
    /// Configuration bit field (see l2cap_chan_cfg_bf enumeration)
    uint8_t config_bf;
    /// Reception L2CAP Channel Identifier
    uint16_t rx_cid;
    /// Transmission L2CAP Channel Identifier
    uint16_t tx_cid;
    ///Reception Maximum Transmit Unit Size
    uint16_t rx_mtu;
    /// Transmission Maximum Transmit Unit Size
    uint16_t tx_mtu;
    /// Reception Maximum Packet Size.        (@note 0 for a fixed L2CAP channel)
    uint16_t rx_mps;
    /// Transmission Maximum Packet Size.     (@note 0 for a fixed L2CAP channel)
    uint16_t tx_mps;
    /// Reception credit number.              (@note 0 for a fixed L2CAP channel)
    uint16_t rx_credit;
    /// Transmission reception credit number. (@note 0 for a fixed L2CAP channel)
    uint16_t tx_credit;
} l2cap_dbg_chan_info_get_cmp_evt_t;

#if (EATT_SUPPORT)
/// #L2CAP_DBG_COC_RECONFIGURE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_DBG_COC_RECONFIGURE
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    ///  New Maximum Transmit Size used for reception
    uint16_t local_rx_mtu;
    ///  New Maximum Packet Size used for reception
    uint16_t local_rx_mps;
    /// Number of L2CAP channel to reconfigure
    uint8_t nb_chan;
    /// Array that contains list of L2CAP channel identifier to reconfigure
    uint8_t chan_lid[__ARRAY_EMPTY];
} l2cap_dbg_coc_reconfigure_cmd_t;

/// #L2CAP_COC_RECONFIGURE uses #L2CAP_COC_RECONFIGURE command complete event structure
/*@TRACE*/
typedef l2cap_coc_reconfigure_cmp_evt_t l2cap_dbg_coc_reconfigure_cmp_evt_t;
#endif // (EATT_SUPPORT)

/// #L2CAP_DBG_COC_ENHANCED_NEGO_CTRL command structure
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_DBG_COC_ENHANCED_NEGO_CTRL
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// 0: Disable enhanced COC negotiation
    /// 1: Enable enhanced COC negotiation
    uint8_t enable;
} l2cap_dbg_coc_enhanced_nego_ctrl_cmd_t;

/// #L2CAP_DBG_COC_ENHANCED_NEGO_CTRL complete event structure
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// Connection Index
    uint8_t conidx;
} l2cap_dbg_coc_enhanced_nego_ctrl_cmp_evt_t;

/// #L2CAP_DBG_RX_CTRL command structure
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    ///  - #L2CAP_DBG_RX_CTRL
    uint16_t cmd_code;
    /// Metadata information returned in command events  (see \glos{METAINFO})
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// 0: Reception disabled, received ACL packets are put in a queue
    /// 1: Reception enabled
    uint8_t enable;
} l2cap_dbg_rx_ctrl_cmd_t;

/// #L2CAP_DBG_RX_CTRL complete event structure
/*@TRACE*/
typedef struct
{
    /// Command code (see #l2cap_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// Connection Index
    uint8_t conidx;
} l2cap_dbg_rx_ctrl_cmp_evt_t;

/// #L2CAP_UNKNOWN_MSG Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #l2cap_ind_code enumeration)
    ///  - #L2CAP_UNKNOWN_MSG
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Message identifier
    uint16_t msg_id;
} l2cap_unknown_msg_ind_t;

/// #L2CAP_COC_CREATED Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #l2cap_ind_code enumeration)
    ///  - #L2CAP_COC_CREATED
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO}), 0x0000 otherwise
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// Created L2CAP channel local index
    uint8_t chan_lid;
    /// Local device reception Maximum Transmit Unit size
    uint16_t local_rx_mtu;
    /// Peer device reception Maximum Transmit Unit size
    uint16_t peer_rx_mtu;
} l2cap_coc_created_ind_t;

#if (EATT_SUPPORT)
/// #L2CAP_COC_MTU_CHANGED  Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #l2cap_ind_code enumeration)
    ///  - #L2CAP_COC_MTU_CHANGED
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO}), 0x0000 otherwise
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index
    uint8_t chan_lid;
    /// Local device reception Maximum Transmit Unit size
    uint16_t local_rx_mtu;
    /// Peer device reception Maximum Transmit Unit size
    uint16_t peer_rx_mtu;
} l2cap_coc_mtu_changed_ind_t;
#endif // (EATT_SUPPORT)

/// #L2CAP_COC_TERMINATED Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #l2cap_ind_code enumeration)
    ///  - #L2CAP_COC_TERMINATED
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO}), 0x0000 otherwise
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index
    uint8_t chan_lid;
    /// Termination Reason (see #hl_err enumeration)
    uint16_t reason;
} l2cap_coc_terminated_ind_t;

/// #L2CAP_DBG_CHAN_ERROR Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #l2cap_ind_code enumeration)
    ///  - #L2CAP_DBG_CHAN_ERROR
    uint16_t ind_code;
    /// 0x0000.
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index
    uint8_t chan_lid;
    /// Termination Reason (see #hl_err enumeration)
    uint16_t reason;
} l2cap_dbg_chan_error_ind_t;

/// #L2CAP_DBG_CHAN_TX_FLOW_OFF Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #l2cap_ind_code enumeration)
    ///  - #L2CAP_DBG_CHAN_TX_FLOW_OFF
    uint16_t ind_code;
    /// 0x0000.
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index
    uint8_t chan_lid;
} l2cap_dbg_chan_tx_flow_off_ind_t;

/// #L2CAP_DBG_CHAN_RX_CREDIT_ADDED Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #l2cap_ind_code enumeration)
    ///  - #L2CAP_DBG_CHAN_RX_CREDIT_ADDED
    uint16_t ind_code;
    /// 0x0000.
    uint16_t metainfo;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index
    uint8_t chan_lid;
    /// Number of credit added for PDU reception
    uint16_t credits;
} l2cap_dbg_chan_rx_credit_added_ind_t;

/// #L2CAP_COC_CONNECT request indication structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #l2cap_req_ind_code enumeration)
    uint16_t req_ind_code;
    /// Token provided by L2CAP module that must be used into the #L2CAP_CFM message
    uint16_t token;
    /// Connection Index
    uint8_t conidx;
    /// Number of L2CAP channel requested to be created in parallel
    uint8_t nb_chan;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
    /// Peer device Maximum Transmit Unit reception size
    uint16_t peer_rx_mtu;
} l2cap_coc_connect_req_ind_t;

/// #L2CAP_COC_CONNECT confirm structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #l2cap_req_ind_code enumeration)
    uint16_t req_ind_code;
    /// Token provided by L2CAP module into the #L2CAP_REQ_IND message
    uint16_t token;
    /// Connection Index
    uint8_t conidx;
    /// Number of L2CAP channel local device accepts to create in parallel
    /// 0: reject all channel creation
    uint8_t nb_chan;
    /// Local reception Maximum Transmit Unit Size
    uint16_t local_rx_mtu;
} l2cap_coc_connect_cfm_t;

/// #L2CAP_SDU_RX request indication structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #l2cap_req_ind_code enumeration)
    uint16_t req_ind_code;
    /// Token req_ind_code by L2CAP module that must be used into the #L2CAP_CFM message
    uint16_t token;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index
    uint8_t chan_lid;
    /// Reception status (see #hl_err enumeration)
    uint16_t status;
    /// SDU Length
    uint16_t length;
    /// SDU Data
    uint8_t data[__ARRAY_EMPTY];
} l2cap_sdu_rx_req_ind_t;

/// #L2CAP_SDU_RX confirm structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #l2cap_req_ind_code enumeration)
    uint16_t req_ind_code;
    /// Token provided by L2CAP module into the #L2CAP_REQ_IND message
    uint16_t token;
    /// Connection Index
    uint8_t conidx;
    /// L2CAP channel local index
    uint8_t chan_lid;
} l2cap_sdu_rx_cfm_t;

/// @} L2CAP_MSG_STRUCT_API

#endif // L2CAP_MSG_H_
