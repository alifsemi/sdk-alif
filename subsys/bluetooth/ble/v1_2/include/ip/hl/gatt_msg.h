/**
 ****************************************************************************************
 *
 * @file gatt_msg.h
 *
 * @brief Header file - GATT Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 ****************************************************************************************
 */

#ifndef GATT_MSG_H_
#define GATT_MSG_H_


#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GATT_MSG_API Message API
 * @ingroup GATT_API
 * @brief Handles ALL messages to/from GATT block.
 *
 * @defgroup GATT_MSG_ID_API Message Identifiers
 * @ingroup GATT_MSG_API
 *
 * @defgroup GATT_MSG_STRUCT_API Message Structures
 * @ingroup GATT_MSG_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "gatt_db.h"
#include "gatt_cli.h"
#include "gatt_srv.h"
#include "compiler.h"
#include <stdbool.h>

/*
 * MESSAGES
 ****************************************************************************************
 */

/// @addtogroup GATT_MSG_ID_API
/// @{

/// Message API of the GATT task
/*@TRACE*/
enum gatt_msg_id
{
    /// GATT command Message (see #gatt_cmd_t). Specialization of the structure depends on value of #gatt_cmd_t.cmd_code.
    /// More information available in #gatt_cmd_code enumeration. A command message is completed once #GATT_CMP_EVT message
    /// is received
    GATT_CMD = MSG_ID(GATT, 0x00),
    /// GATT command complete event message (see #gatt_cmp_evt_t). Specialization of depends on value of #gatt_cmp_evt_t.cmd_code.
    /// More information available in #gatt_cmd_code enumeration
    GATT_CMP_EVT = MSG_ID(GATT, 0x01),
    /// GATT Indication message (see #gatt_ind_t). Specialization of the structure depends on value of #gatt_ind_t.ind_code.
    /// More information available in #gatt_ind_code enumeration
    GATT_IND = MSG_ID(GATT, 0x02),
    /// GATT request indication message (see #gatt_req_ind_t). Specialization of the structure depends on value of #gatt_req_ind_t.req_ind_code.
    /// More information available in #gatt_req_ind_code enumeration. A #GATT_REQ_IND must be confirm by API client using #GATT_CFM message.
    GATT_REQ_IND = MSG_ID(GATT, 0x03),
    /// GATT confirmation message (see #gatt_req_ind_t). Specialization of the structure depends on value of #gatt_cfm_t.req_ind_code.
    /// More information available in #gatt_req_ind_code enumeration
    GATT_CFM = MSG_ID(GATT, 0x04),
};

/// #GATT_CMD command codes
/*@TRACE*/
enum gatt_cmd_code
{
    /* --- GATT User Management --- */
    /// Command used to register a GATT user. This must be done prior to any GATT procedure execution.
    /// #gatt_user_register_cmd_t structure must be used to send command, #gatt_user_register_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_user_cli_register and #gatt_user_srv_register)
    GATT_USER_REGISTER = 0x0000,
    /// Command used to unregister a GATT user (client or server).
    /// #gatt_user_unregister_cmd_t structure must be used to send command, #gatt_user_unregister_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_user_cli_register and #gatt_user_unregister)
    GATT_USER_UNREGISTER = 0x0001,

    /* --- GATT Server Database Management --- */
    /// Command used to add a service into local attribute database.
    /// #gatt_db_svc_add_cmd_t structure must be used to send command, #gatt_db_svc_add_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_db_svc_add)
    GATT_DB_SVC_ADD = 0x0100,
    /// Command used to remove a service from local attribute database.
    /// #gatt_db_svc_remove_cmd_t structure must be used to send command, #gatt_db_svc_remove_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_db_svc_remove)
    GATT_DB_SVC_REMOVE = 0x0101,
    /// Command used to control visibility and usage authorization of a local service.
    /// A hidden service is present in database but cannot be discovered or manipulated by a peer device.
    /// A disabled service can be discovered by a peer device but it is not authorized to use it.
    /// #gatt_db_svc_ctrl_cmd_t structure must be used to send command, #gatt_db_svc_ctrl_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_db_svc_ctrl)
    GATT_DB_SVC_CTRL = 0x0102,
    #if (GATT_CACHING_SUPPORT)
    /// Command used to retrieve (or compute) the local database hash value.
    /// #gatt_db_hash_get_cmd_t structure must be used to send command, #gatt_db_hash_get_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_db_hash_get)
    GATT_DB_HASH_GET = 0x0103,
    #endif // (GATT_CACHING_SUPPORT)

    /* --- GATT Server initiated procedure --- */
    /// Command used by a GATT server user to send notifications or indications for some attributes values to peer device.
    /// Number of attributes must be set to one for #GATT_INDICATE event type.
    /// This command is consider reliable because GATT user is aware of maximum packet  size that can be transmitted
    /// over the air.
    /// #gatt_srv_event_reliable_send_cmd_t structure must be used to send command, #gatt_srv_event_reliable_send_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_srv_event_reliable_send)
    GATT_SRV_EVENT_RELIABLE_SEND = 0x0200,
    /// Command used by a GATT server user to send notifications or indications for one attribute value to peer device.
    /// #gatt_srv_event_send_cmd_t structure must be used to send command, #gatt_srv_event_send_cmp_evt_t received once
    /// command is executed.
    ///
    /// (see #gatt_srv_event_send)
    GATT_SRV_EVENT_SEND = 0x0201,
    /// Command used by a GATT server user to send notifications or indications for one attribute value to multiple peer devices.
    /// #gatt_srv_event_mtp_send_cmd_t structure must be used to send command, #gatt_srv_event_mtp_send_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_srv_event_mtp_send)
    GATT_SRV_EVENT_MTP_SEND = 0x0202,
    /// Command used by a GATT server to cancel an on-going multi-point event transmission
    /// #gatt_srv_event_mtp_cancel_cmd_t structure must be used to send command, #gatt_srv_event_mtp_cancel_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_srv_event_mtp_cancel)
    GATT_SRV_EVENT_MTP_CANCEL = 0x0203,

    /* --- GATT Client initiated procedure --- */
    /// Command used by a GATT client user to discover primary or secondary services, exposed by peer device in its
    /// attribute database. All services can be discovered or filtering services having a specific UUID.
    /// #gatt_cli_discover_svc_cmd_t structure must be used to send command, #gatt_cli_discover_svc_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_discover_svc)
    GATT_CLI_DISCOVER_SVC = 0x0300,
    /// Command used by a GATT client user to discover included services, exposed by peer device in its attribute database.
    /// #gatt_cli_discover_inc_svc_cmd_t structure must be used to send command, #gatt_cli_discover_inc_svc_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_discover_inc_svc)
    GATT_CLI_DISCOVER_INC_SVC = 0x0301,
    /// Command used by a GATT client user to discover all or according to a specific UUID characteristics exposed by
    /// peer device in its attribute database.
    /// #gatt_cli_discover_char_cmd_t structure must be used to send command, #gatt_cli_discover_char_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_discover_char)
    GATT_CLI_DISCOVER_CHAR = 0x0302,
    /// Command used by a GATT client user to discover characteristic descriptor exposed by peer device in its
    /// attribute database.
    /// #gatt_cli_discover_desc_cmd_t structure must be used to send command, #gatt_cli_discover_desc_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_discover_desc)
    GATT_CLI_DISCOVER_DESC = 0x0303,
    /// Command used by a GATT client user to cancel an on-going discovery procedure. The metainfo parameter in the
    /// request must be equals to metainfo parameter used for service discovery command.
    /// The discovery is aborted as soon as on-going discovery attribute transaction is over.
    /// #gatt_cli_discover_cancel_cmd_t structure must be used to send command, #gatt_cli_discover_cancel_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_discover_cancel)
    GATT_CLI_DISCOVER_CANCEL = 0x0304,
    /// Command used by a GATT client user to read value of an attribute (identified by its handle) present in peer
    /// database.
    /// #gatt_cli_read_cmd_t structure must be used to send command, #gatt_cli_read_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_read)
    GATT_CLI_READ = 0x0305,
    /// Command used by a GATT client user to read value of an attribute with a given UUID in peer database.
    /// #gatt_cli_read_by_uuid_cmd_t structure must be used to send command, #gatt_cli_read_by_uuid_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_read_by_uuid)
    GATT_CLI_READ_BY_UUID = 0x0306,
    /// Command used by a GATT client user to read multiple attribute at the same time. If one of attribute length is
    /// unknown, the read multiple variable length procedure is used.
    /// #gatt_cli_read_multiple_cmd_t structure must be used to send command, #gatt_cli_read_multiple_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_read_multiple)
    GATT_CLI_READ_MULTIPLE = 0x0307,
    /// Command used by a GATT client user to request to write value of an attribute in peer database.
    /// This command is consider reliable because GATT user is aware of maximum packet size that can be transmitted
    /// over the air.
    /// #gatt_cli_write_reliable_cmd_t structure must be used to send command, #gatt_cli_write_reliable_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_write_reliable)
    GATT_CLI_WRITE_RELIABLE = 0x0308,
    /// Command used by a GATT client user to request to write value of an attribute in peer database.
    /// Since user is not aware of MTU size of the bearer used for attribute transmission it cannot be considered reliable.
    /// For a #GATT_WRITE_NO_RESP if attribute bearer max transmission size isn't sufficient, a #GATT_WRITE (with
    /// response) procedure will be used.
    /// For a #GATT_WRITE_SIGNED, if attribute bearer max transmission size isn't sufficient, the procedure is aborted
    /// with L2CAP_ERR_INVALID_MTU error code.
    /// #gatt_cli_write_cmd_t structure must be used to send command, #gatt_cli_write_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_write)
    GATT_CLI_WRITE = 0x0309,
    /// Command used by a GATT client user to request peer server to execute prepare write queue.
    /// #gatt_cli_write_exe_cmd_t structure must be used to send command, #gatt_cli_write_exe_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_write_exe)
    GATT_CLI_WRITE_EXE = 0x030A,
    /// Command used by a GATT client user to register for reception of events (notification / indication) for a
    /// given handle range.
    /// #gatt_cli_event_register_cmd_t structure must be used to send command, #gatt_cli_event_register_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_event_register)
    GATT_CLI_EVENT_REGISTER = 0x030B,
    /// Command used by a GATT client user to stop reception of events (notification / indication) onto a specific
    /// handle range.
    /// #gatt_cli_event_unregister_cmd_t structure must be used to send command, #gatt_srv_event_mtp_cancel_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_event_unregister)
    GATT_CLI_EVENT_UNREGISTER = 0x030C,
    /// Command used to update MTU to a specific value on legacy attribute bearer.
    /// #gatt_cli_mtu_update_cmd_t structure must be used to send command, #gatt_cli_mtu_update_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_cli_mtu_exch)
    GATT_CLI_MTU_UPDATE = 0x030D,

    /* --- Debug only --- */
    /// Debug command used to remove all services from local attribute database.
    /// #gatt_dbg_db_svc_remove_all_cmd_t structure must be used to send command, #gatt_dbg_db_svc_remove_all_cmp_evt_t
    /// received once command is executed.
    GATT_DBG_DB_SVC_REMOVE_ALL = 0x0904,
    /// Debug command used to get information about all services present in attribute database.
    /// #gatt_dbg_db_svc_list_get_cmd_t structure must be used to send command, #gatt_dbg_db_svc_list_get_cmp_evt_t
    /// received once command is executed.
    GATT_DBG_DB_SVC_LIST_GET = 0x0905,
    /// Debug command used to update service security level permission or to enable/disable a service.
    /// The UUID length information cannot be updated.
    /// #gatt_dbg_db_svc_info_set_cmd_t structure must be used to send command, #gatt_dbg_db_svc_info_set_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_db_svc_ctrl)
    GATT_DBG_DB_SVC_INFO_SET = 0x0906,
    /// Debug command used to retrieve information of an attribute.
    /// #gatt_dbg_db_att_info_get_cmd_t structure must be used to send command, #gatt_dbg_db_att_info_get_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_db_att_info_get)
    GATT_DBG_DB_ATT_INFO_GET = 0x0907,
    /// Debug command used to set information of an attribute.
    /// #gatt_dbg_db_att_info_set_cmd_t structure must be used to send command, #gatt_dbg_db_att_info_set_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_db_att_info_set)
    GATT_DBG_DB_ATT_INFO_SET = 0x0908,
    /// Debug command used to get information about active bearer.
    /// #gatt_dbg_bearer_info_get_cmd_t structure must be used to send command, #gatt_dbg_bearer_info_get_cmp_evt_t
    /// received once command is executed.
    ///
    /// (see #gatt_bearer_mtu_min_get)
    GATT_DBG_BEARER_INFO_GET = 0x0909,
    /// Debug command used to close a specific bearer.
    /// #gatt_dbg_bearer_close_cmd_t structure must be used to send command, #gatt_dbg_bearer_close_cmp_evt_t
    /// received once command is executed.
    GATT_DBG_BEARER_CLOSE = 0x090A,
    /// Debug command used to force establishment of EATT bearers.
    /// #gatt_dbg_bearer_eatt_estab_cmd_t structure must be used to send command, #gatt_dbg_bearer_eatt_estab_cmp_evt_t
    /// received once command is executed.
    GATT_DBG_BEARER_EATT_ESTAB = 0x090B,
};

/// #GATT_IND indication codes
/*@TRACE*/
enum gatt_ind_code
{
    /// Event triggered when an unknown message has been received by GATT task.
    /// #gatt_unknown_msg_ind_t received when event is triggered
    GATT_UNKNOWN_MSG = 0x0000,

    /* --- Client Message --- */
    /// Event triggered when service discovery procedure has information to provide about a service.
    /// To prevent memory overflow during discovery procedure, service information can be split into several events.
    /// When receiving start or complete service information, this means new service, first attribute can be either
    /// #GATT_ATT_PRIMARY_SVC or #GATT_ATT_SECONDARY_SVC.
    /// If start receive discovery information is present in message, GATT user should wait for new #GATT_CLI_SVC
    /// message with a continue service or end service discovery information.
    /// #gatt_cli_svc_ind_t received when event is triggered.
    ///
    /// (see #gatt_cli_cb.cb_svc)
    GATT_CLI_SVC = 0x0300,
    /// Event triggered when a service is found during service discovery procedure - Only Service information.
    /// #gatt_cli_svc_info_ind_t received when event is triggered.
    ///
    /// (see #gatt_cli_cb.cb_svc_info)
    GATT_CLI_SVC_INFO = 0x0301,
    /// Event triggered when an included service is found during discovery procedure.
    /// #gatt_cli_inc_svc_ind_t received when event is triggered.
    ///
    /// (see #gatt_cli_cb.cb_inc_svc)
    GATT_CLI_INC_SVC = 0x0302,
    /// Event triggered when a characteristic is found during discovery procedure.
    /// #gatt_cli_char_ind_t received when event is triggered.
    ///
    /// (see #gatt_cli_cb.cb_char)
    GATT_CLI_CHAR = 0x0303,
    /// Event triggered when a characteristic descriptor is found during discovery procedure.
    /// #gatt_cli_desc_ind_t received when event is triggered.
    ///
    /// (see #gatt_cli_cb.cb_desc)
    GATT_CLI_DESC = 0x0304,
    /// Event triggered when an attribute value has been read.
    /// #gatt_cli_att_val_ind_t received when event is triggered.
    ///
    /// (see #gatt_cli_cb.cb_att_val)
    GATT_CLI_ATT_VAL = 0x0305,
    /// Event triggered when a service change has been received or if an attribute transaction triggers an
    /// out of sync error.
    /// #gatt_cli_svc_changed_ind_t received when event is triggered.
    ///
    /// (see #gatt_cli_cb.cb_svc_changed)
    GATT_CLI_SVC_CHANGED = 0x0306,
};

/// #GATT_REQ_IND request indication codes
/*@TRACE*/
enum gatt_req_ind_code
{
    /* --- GATT Server User --- */
    /// Event triggered by GATT when peer device requests to read an attribute.
    /// #gatt_srv_att_read_get_req_ind_t received when event is triggered, must be confirm using
    /// #gatt_srv_att_read_get_cfm_t message structure.
    ///
    /// (see #gatt_srv_cb.cb_att_read_get and #gatt_srv_att_read_get_cfm)
    GATT_SRV_ATT_READ_GET = 0x0200,
    /// Event triggered by GATT when user requires to send either a notification or an indication.
    /// #gatt_srv_att_event_get_req_ind_t received when event is triggered, must be confirm using
    /// #gatt_srv_att_event_get_cfm_t message structure.
    ///
    /// (see #gatt_srv_cb.cb_att_event_get  and #gatt_srv_att_event_get_cfm)
    GATT_SRV_ATT_EVENT_GET = 0x0201,
    /// Event triggered by GATT in order to ask GATT server user to provide information about a specific attribute value.
    /// #gatt_srv_att_info_get_req_ind_t received when event is triggered, must be confirm using
    /// #gatt_srv_att_info_get_cfm_t message structure.
    ///
    /// (see #gatt_srv_cb.cb_att_info_get and #gatt_srv_att_info_get_cfm)
    GATT_SRV_ATT_INFO_GET = 0x0202,
    /// Event triggered by GATT in order to inform GATT server user when an attribute value has been written by a
    /// peer device.
    /// #gatt_srv_att_val_set_req_ind_t received when event is triggered, must be confirm using
    /// #gatt_srv_att_val_set_cfm_t message structure.
    ///
    /// (see #gatt_srv_cb.cb_att_val_set and #gatt_srv_att_val_set_cfm)
    GATT_SRV_ATT_VAL_SET = 0x0203,
    /* --- GATT Client User --- */
    /// Event triggered by GATT in order to ask GATT client user to provide specific attribute value.
    /// This event can be trigger if GATT client user requests to update peer device attribute value using write
    /// procedure.
    /// #gatt_cli_att_val_get_req_ind_t received when event is triggered, must be confirm using
    /// #gatt_cli_att_val_get_cfm_t message structure.
    ///
    /// (see #gatt_cli_cb.cb_att_val_get and #gatt_cli_att_val_get_cfm)
    GATT_CLI_ATT_VAL_GET = 0x0300,
    /// Inform GATT client user about reception of either a notification or an indication from peer device.
    /// #gatt_cli_att_event_req_ind_t received when event is triggered, must be confirm using
    /// #gatt_cli_att_event_cfm_t message structure.
    ///
    /// (see #gatt_cli_cb.cb_att_val_evt and #gatt_cli_att_event_cfm)
    GATT_CLI_ATT_EVENT = 0x0301,
};

/// @} GATT_MSG_ID_API

/*
 * DEFAULT MESSAGE CONTENT
 ****************************************************************************************
 */

/// @addtogroup GATT_MSG_STRUCT_API
/// @{

/// Default #GATT_CMD message structure
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
} gatt_cmd_t;

/// Default #GATT_CMP_EVT message structure
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// GATT user local identifier
    uint8_t user_lid;
} gatt_cmp_evt_t;

/// Extension of #GATT_CMP_EVT message structure (gatt_cmp_evt_t) for procedure execution
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
} gatt_proc_cmp_evt_t;

/// Default #GATT_IND message structure
/*@TRACE*/
typedef struct
{
    /// Indication code (see #gatt_ind_code enumeration)
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO}), 0x0000 otherwise
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
} gatt_ind_t;

/// Default #GATT_REQ_IND message structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    uint16_t req_ind_code;
    /// Token provided by GATT module that must be used into the #GATT_CFM message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
} gatt_req_ind_t;

/// Default #GATT_CFM message structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    uint16_t req_ind_code;
    /// Token provided by GATT module into the #GATT_REQ_IND message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Status of the request by GATT user  (see #hl_err enumeration)
    uint16_t status;
} gatt_cfm_t;

/*
 * MESSAGE CONTENT
 ****************************************************************************************
 */

/// #GATT_USER_REGISTER Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_USER_REGISTER
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// Preferred MTU for attribute exchange.
    uint16_t pref_mtu;
    /// User attribute priority level
    uint8_t prio_level;
    /// Client or Server role (see #gatt_user_role enumeration)
    uint8_t role;
} gatt_user_register_cmd_t;

/// #GATT_USER_REGISTER uses default complete event structure
/*@TRACE*/
typedef gatt_cmp_evt_t gatt_user_register_cmp_evt_t;

/// #GATT_USER_UNREGISTER Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_USER_UNREGISTER
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT User Local identifier
    uint8_t user_lid;
} gatt_user_unregister_cmd_t;

/// #GATT_USER_UNREGISTER uses default complete event structure
/*@TRACE*/
typedef gatt_cmp_evt_t gatt_user_unregister_cmp_evt_t;

/// #GATT_DB_SVC_ADD Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DB_SVC_ADD
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Service Information bit field (see Table 18)
    uint8_t info;
    /// Service UUID (LSB first)
    uint8_t uuid[GATT_UUID_128_LEN];
    /// Attribute Start Handle (0 = chosen by GATT module)
    uint16_t start_hdl;
    /// Number of attribute(s) reserved for the service (shall be equals or greater nb_att)
    /// Prevent any services to be inserted between start_hdl and (start_hdl + nb_att_rsvd -1)
    uint8_t nb_att_rsvd;
    /// Number of attribute(s) in service
    uint8_t nb_att;
    /// List of attribute description present in service.
    /// @note First attribute in attribute array must be a Primary or a Secondary service
    gatt_att_desc_t atts[__ARRAY_EMPTY];
} gatt_db_svc_add_cmd_t;

/// #GATT_DB_SVC_ADD Command Complete Event structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    /// - #GATT_DB_SVC_ADD
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Service start handle associated to created service.
    uint16_t start_hdl;
} gatt_db_svc_add_cmp_evt_t;

/// #GATT_DB_SVC_REMOVE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DB_SVC_REMOVE
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Attribute Start Handle of the service to remove
    uint16_t start_hdl;
} gatt_db_svc_remove_cmd_t;

/// #GATT_DB_SVC_REMOVE uses default complete event structure
/*@TRACE*/
typedef gatt_cmp_evt_t gatt_db_svc_remove_cmp_evt_t;

/// #GATT_DB_SVC_CTRL Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DB_SVC_CTRL
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// 0: Authorize usage of the service
    /// 1: Reject usage of the service
    uint8_t enable;
    /// Service visibility (see #gatt_svc_visibility enumeration)
    uint8_t visible;
    /// Attribute Start Handle of the service to hide/show
    uint16_t start_hdl;
} gatt_db_svc_ctrl_cmd_t;

/// #GATT_DB_SVC_CTRL uses default complete event structure
/*@TRACE*/
typedef gatt_cmp_evt_t gatt_db_svc_ctrl_cmp_evt_t;

#if (GATT_CACHING_SUPPORT)
/// #GATT_DB_HASH_GET Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DB_HASH_GET
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
} gatt_db_hash_get_cmd_t;

/// #GATT_DB_HASH_GET Command Complete Event structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Database Hash.
    uint8_t hash[GATT_DB_HASH_LEN];
} gatt_db_hash_get_cmp_evt_t;
#endif // (GATT_CACHING_SUPPORT)

/// #GATT_SRV_EVENT_RELIABLE_SEND Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_SRV_EVENT_RELIABLE_SEND
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Event type to trigger (see #gatt_evt_type enumeration)
    uint8_t evt_type;
    /// Number of attribute
    uint8_t nb_att;
    /// List of attribute
    gatt_att_t atts[__ARRAY_EMPTY];
} gatt_srv_event_reliable_send_cmd_t;

/// #GATT_SRV_EVENT_RELIABLE_SEND uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_srv_event_reliable_send_cmp_evt_t;

/// #GATT_SRV_EVENT_SEND Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_SRV_EVENT_SEND
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Event type to trigger (see #gatt_evt_type enumeration)
    uint8_t evt_type;
    /// Attribute handle
    uint16_t hdl;
    /// Value length
    uint16_t value_length;
    /// Value to transmit
    uint8_t value[__ARRAY_EMPTY];
} gatt_srv_event_send_cmd_t;

/// #GATT_SRV_EVENT_SEND uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_srv_event_send_cmp_evt_t;

/// #GATT_SRV_EVENT_MTP_SEND Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_SRV_EVENT_MTP_SEND
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index bit field
    uint32_t conidx_bf;
    /// Event type to trigger (see #gatt_evt_type enumeration)
    uint8_t evt_type;
    /// Attribute handle
    uint16_t hdl;
    /// Value length
    uint16_t value_length;
    /// Value to transmit
    uint8_t value[__ARRAY_EMPTY];
} gatt_srv_event_mtp_send_cmd_t;

/// #GATT_SRV_EVENT_MTP_SEND uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_srv_event_mtp_send_cmp_evt_t;

/// #GATT_SRV_EVENT_MTP_CANCEL Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_SRV_EVENT_MTP_CANCEL
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
} gatt_srv_event_mtp_cancel_cmd_t;

/// #GATT_SRV_EVENT_MTP_CANCEL uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_cmp_evt_t gatt_srv_event_mtp_cancel_cmp_evt_t;

/// #GATT_CLI_DISCOVER_SVC Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_DISCOVER_SVC
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// GATT Service discovery type (see #gatt_svc_discovery_type enumeration)
    uint8_t disc_type;
    /// Perform discovery of all information present in the service  (1: enable, 0: disable)
    uint8_t full;
    /// Search start handle
    uint16_t start_hdl;
    /// Search end handle
    uint16_t end_hdl;
    /// UUID Type (see #gatt_uuid_type enumeration)
    uint8_t uuid_type;
    /// Searched Service UUID (meaningful only for discovery by UUID)
    uint8_t uuid[GATT_UUID_128_LEN];
} gatt_cli_discover_svc_cmd_t;

/// #GATT_CLI_DISCOVER_SVC uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_discover_svc_cmp_evt_t;

/// #GATT_CLI_DISCOVER_INC_SVC Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_DISCOVER_INC_SVC
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Search start handle
    uint16_t start_hdl;
    /// Search end handle
    uint16_t end_hdl;
} gatt_cli_discover_inc_svc_cmd_t;

/// #GATT_CLI_DISCOVER_INC_SVC uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_discover_inc_svc_cmp_evt_t;

/// #GATT_CLI_DISCOVER_CHAR Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_DISCOVER_CHAR
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// GATT characteristic discovery type (see #gatt_char_discovery_type enumeration)
    uint8_t disc_type;
    /// Search start handle
    uint16_t start_hdl;
    /// Search end handle
    uint16_t end_hdl;
    /// UUID Type (see #gatt_uuid_type enumeration)
    uint8_t uuid_type;
    /// Searched UUID (meaningful only for discovery by UUID)
    uint8_t uuid[GATT_UUID_128_LEN];
} gatt_cli_discover_char_cmd_t;

/// #GATT_CLI_DISCOVER_CHAR uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_discover_char_cmp_evt_t;

/// #GATT_CLI_DISCOVER_DESC Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_DISCOVER_DESC
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Search start handle
    uint16_t start_hdl;
    /// Search end handle
    uint16_t end_hdl;
} gatt_cli_discover_desc_cmd_t;

/// #GATT_CLI_DISCOVER_DESC uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_discover_desc_cmp_evt_t;

/// #GATT_CLI_DISCOVER_CANCEL Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_DISCOVER_CANCEL
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
} gatt_cli_discover_cancel_cmd_t;

/// #GATT_CLI_DISCOVER_CANCEL uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_discover_cancel_cmp_evt_t;

/// #GATT_CLI_READ Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_READ
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Attribute handle
    uint16_t hdl;
    /// Value offset
    uint16_t offset;
    /// Value length to read (0 = read complete value)
    uint16_t length;
} gatt_cli_read_cmd_t;

/// #GATT_CLI_READ uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_read_cmp_evt_t;

/// #GATT_CLI_READ_BY_UUID Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_READ_BY_UUID
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Search start handle
    uint16_t start_hdl;
    /// Search end handle
    uint16_t end_hdl;
    /// UUID Type (see #gatt_uuid_type enumeration)
    uint8_t uuid_type;
    /// Searched UUID
    uint8_t uuid[GATT_UUID_128_LEN];
} gatt_cli_read_by_uuid_cmd_t;

/// #GATT_CLI_READ_BY_UUID uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_read_by_uuid_cmp_evt_t;

/// #GATT_CLI_READ_MULTIPLE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_READ_MULTIPLE
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Number of attribute
    uint8_t nb_att;
    /// List of attribute
    /// If Attribute length is zero (consider length unknown):
    ///     - Attribute protocol read multiple variable length procedure used
    gatt_att_t atts[__ARRAY_EMPTY];
} gatt_cli_read_multiple_cmd_t;

/// #GATT_CLI_READ_MULTIPLE uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_read_multiple_cmp_evt_t;

/// #GATT_CLI_WRITE_RELIABLE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_WRITE_RELIABLE
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// GATT write type (see #gatt_write_type enumeration)
    uint8_t write_type;
    /// Write execution mode (see #gatt_write_mode enumeration). Valid only for #GATT_WRITE.
    uint8_t write_mode;
    /// Attribute handle
    uint16_t hdl;
    /// Value offset, valid only for #GATT_WRITE
    uint16_t offset;
    /// Value length to write
    uint16_t length;
} gatt_cli_write_reliable_cmd_t;

/// #GATT_CLI_WRITE_RELIABLE uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_write_reliable_cmp_evt_t;

/// #GATT_CLI_WRITE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_WRITE
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// GATT write type (see #gatt_write_type enumeration)
    uint8_t write_type;
    /// Attribute handle
    uint16_t hdl;
    /// Value offset, valid only for #GATT_WRITE
    uint16_t offset;
    /// Value length to write
    uint16_t value_length;
    /// Attribute value
    uint8_t value[__ARRAY_EMPTY];
} gatt_cli_write_cmd_t;

/// #GATT_CLI_WRITE uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_write_cmp_evt_t;

/// #GATT_CLI_WRITE_EXE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_WRITE_EXE
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// 1: Perform pending write operations
    /// 0: Cancel pending write operations
    uint8_t execute;
} gatt_cli_write_exe_cmd_t;

/// #GATT_CLI_WRITE_EXE uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_write_exe_cmp_evt_t;

/// #GATT_CLI_EVENT_REGISTER Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_EVENT_REGISTER
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Attribute start handle
    uint16_t start_hdl;
    /// Attribute end handle
    uint16_t end_hdl;
} gatt_cli_event_register_cmd_t;

/// #GATT_CLI_EVENT_REGISTER uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_event_register_cmp_evt_t;

/// #GATT_CLI_EVENT_UNREGISTER Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_EVENT_UNREGISTER
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Attribute start handle
    uint16_t start_hdl;
    /// Attribute end handle
    uint16_t end_hdl;
} gatt_cli_event_unregister_cmd_t;

/// #GATT_CLI_EVENT_UNREGISTER uses default command procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_event_unregister_cmp_evt_t;
/// #GATT_CLI_MTU_UPDATE Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_CLI_MTU_UPDATE
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
} gatt_cli_mtu_update_cmd_t;

/// #GATT_CLI_MTU_UPDATE uses default procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_cli_mtu_update_cmp_evt_t;

/// #GATT_DBG_DB_SVC_REMOVE_ALL Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DBG_DB_SVC_REMOVE_ALL
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
} gatt_dbg_db_svc_remove_all_cmd_t;

/// #GATT_DBG_DB_SVC_REMOVE_ALL uses default complete event structure
/*@TRACE*/
typedef gatt_cmp_evt_t gatt_dbg_db_svc_remove_all_cmp_evt_t;

/// #GATT_DBG_DB_SVC_LIST_GET Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DBG_DB_SVC_LIST_GET
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
} gatt_dbg_db_svc_list_get_cmd_t;

/// #GATT_DBG_DB_SVC_LIST_GET Command Complete Event structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Number of services
    uint8_t nb_svc;
    /// List of service description
    gatt_svc_desc_t svcs[__ARRAY_EMPTY];
} gatt_dbg_db_svc_list_get_cmp_evt_t;

/// #GATT_DBG_DB_SVC_INFO_SET Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DBG_DB_SVC_INFO_SET
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// Start handle of service to update
    uint16_t hdl;
    /// Service Information bit field (see #gatt_svc_info_bf enumeration)
    uint8_t info;
} gatt_dbg_db_svc_info_set_cmd_t;

/// #GATT_DBG_DB_SVC_INFO_SET uses default complete event structure
/*@TRACE*/
typedef gatt_cmp_evt_t gatt_dbg_db_svc_info_set_cmp_evt_t;

/// #GATT_DBG_DB_ATT_INFO_GET Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DBG_DB_ATT_INFO_GET
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// Attribute Handle
    uint16_t hdl;
} gatt_dbg_db_att_info_get_cmd_t;

/// #GATT_DBG_DB_ATT_INFO_GET Command Complete Event structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Attribute Handle
    uint16_t hdl;
    /// Attribute description
    gatt_att_desc_t att;
} gatt_dbg_db_att_info_get_cmp_evt_t;

/// #GATT_DBG_DB_ATT_INFO_SET Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DBG_DB_ATT_INFO_SET
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// Attribute Handle
    uint16_t hdl;
    /// Attribute information bit field (see #gatt_att_info_bf enumeration)
    uint16_t info;
} gatt_dbg_db_att_info_set_cmd_t;

/// #GATT_DBG_DB_ATT_INFO_SET uses default complete event structure
/*@TRACE*/
typedef gatt_cmp_evt_t gatt_dbg_db_att_info_set_cmp_evt_t;

/// #GATT_DBG_BEARER_INFO_GET Command structure definition
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DBG_BEARER_INFO_GET
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
} gatt_dbg_bearer_info_get_cmd_t;

/// Structure that contains bearer information
/*@TRACE*/
typedef struct
{
    /// Bearer Local Identifier
    uint8_t bearer_lid;
    /// L2CAP Channel Local identifier
    uint8_t chan_lid;
    /// 1 if Enhanced ATT bearer 0 if legacy ATT bearer
    uint8_t eatt;
    /// Bearer MTU size
    uint16_t mtu;
} gatt_dbg_bearer_info_t;

/// #GATT_DBG_BEARER_INFO_GET Command Complete Event structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    uint16_t cmd_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Status of the operation (see #hl_err enumeration)
    uint16_t status;
    /// GATT user local identifier - not used
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Number of active bearers
    uint8_t nb_bearer;
    /// list of available bearers
    gatt_dbg_bearer_info_t bearers[__ARRAY_EMPTY];
} gatt_dbg_bearer_info_get_cmp_evt_t;

/// #GATT_DBG_BEARER_CLOSE Command structure definition
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DBG_BEARER_CLOSE
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Bearer Local identifier
    uint8_t bearer_lid;
} gatt_dbg_bearer_close_cmd_t;

/// #GATT_DBG_BEARER_CLOSE uses default procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_dbg_bearer_close_cmp_evt_t;

/// #GATT_DBG_BEARER_EATT_ESTAB Command structure definition
/*@TRACE*/
typedef struct
{
    /// Command code (see #gatt_cmd_code enumeration)
    ///  - #GATT_DBG_BEARER_EATT_ESTAB
    uint16_t cmd_code;
    /// Metadata information returned in command events (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
} gatt_dbg_bearer_eatt_estab_cmd_t;

/// #GATT_DBG_BEARER_EATT_ESTAB uses default procedure complete event structure
/*@TRACE*/
typedef gatt_proc_cmp_evt_t gatt_dbg_bearer_eatt_estab_cmp_evt_t;

/// #GATT_UNKNOWN_MSG Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #gatt_ind_code enumeration)
    ///  - #GATT_UNKNOWN_MSG
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO}), 0x0000 otherwise
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Message identifier
    uint16_t msg_id;
} gatt_unknown_msg_ind_t;

/// #GATT_CLI_SVC Indication structure definition
typedef struct
{
    /// Indication code (see #gatt_ind_code enumeration)
    ///  - #GATT_CLI_SVC
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// First handle value of following list
    uint16_t hdl;
    /// Discovery information (see #gatt_svc_disc_info enumeration)
    uint8_t disc_info;
    /// Number of attributes
    uint8_t nb_att;
    /// Attribute information present in a service
    gatt_svc_att_t atts[__ARRAY_EMPTY];
} gatt_cli_svc_ind_t;

/// #GATT_CLI_SVC_INFO Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #gatt_ind_code enumeration)
    ///  - #GATT_CLI_SVC_INFO
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Service start handle
    uint16_t start_hdl;
    /// Service end handle
    uint16_t end_hdl;
    /// UUID Type (see #gatt_uuid_type enumeration)
    uint8_t uuid_type;
    /// Service UUID (LSB first)
    uint8_t uuid[GATT_UUID_128_LEN];
} gatt_cli_svc_info_ind_t;

/// #GATT_CLI_INC_SVC Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #gatt_ind_code enumeration)
    ///  - #GATT_CLI_INC_SVC
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Include service attribute handle
    uint16_t inc_svc_hdl;
    /// Service start handle
    uint16_t start_hdl;
    /// Service end handle
    uint16_t end_hdl;
    /// UUID Type (see #gatt_uuid_type enumeration)
    uint8_t uuid_type;
    /// Service UUID (LSB first)
    uint8_t uuid[GATT_UUID_128_LEN];
} gatt_cli_inc_svc_ind_t;

/// #GATT_CLI_CHAR Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #gatt_ind_code enumeration)
    ///  - #GATT_CLI_CHAR
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Characteristic attribute handle
    uint16_t char_hdl;
    /// Value handle
    uint16_t val_hdl;
    /// Characteristic properties (see #gatt_att_info_bf enumeration - bits [0-7])
    uint8_t prop;
    /// UUID Type (see #gatt_uuid_type enumeration)
    uint8_t uuid_type;
    /// Characteristic value UUID - LSB first
    uint8_t uuid[GATT_UUID_128_LEN];
} gatt_cli_char_ind_t;

/// #GATT_CLI_DESC Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #gatt_ind_code enumeration)
    ///  - #GATT_CLI_DESC
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Characteristic descriptor attribute handle
    uint16_t desc_hdl;
    /// UUID Type (see #gatt_uuid_type enumeration)
    uint8_t uuid_type;
    /// Attribute UUID - LSB first
    uint8_t uuid[GATT_UUID_128_LEN];
} gatt_cli_desc_ind_t;

/// #GATT_CLI_ATT_VAL Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #gatt_ind_code enumeration)
    ///  - #GATT_CLI_ATT_VAL
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Attribute handle
    uint16_t hdl;
    /// Data offset
    uint16_t offset;
    /// Value length
    uint16_t value_length;
    /// Attribute value starting from offset
    uint8_t value[__ARRAY_EMPTY];
} gatt_cli_att_val_ind_t;

/// #GATT_CLI_SVC_CHANGED Indication structure definition
/*@TRACE*/
typedef struct
{
    /// Indication code (see #gatt_ind_code enumeration)
    ///  - #GATT_CLI_SVC_CHANGED
    uint16_t ind_code;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// True if an out of sync error has been received
    uint8_t out_of_sync;
    /// Service start handle
    uint16_t start_hdl;
    /// Service end handle
    uint16_t end_hdl;
} gatt_cli_svc_changed_ind_t;

/// #GATT_SRV_ATT_READ_GET request indication structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_SRV_ATT_READ_GET
    uint16_t req_ind_code;
    /// Token provided by GATT module that must be used into the #GATT_CFM message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Attribute handle
    uint16_t hdl;
    /// Value offset
    uint16_t offset;
    /// Maximum value length to return
    uint16_t max_length;
} gatt_srv_att_read_get_req_ind_t;

/// #GATT_SRV_ATT_READ_GET confirm structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_SRV_ATT_READ_GET
    uint16_t req_ind_code;
    /// Token provided by GATT module into the #GATT_REQ_IND message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Status of the request by GATT user (see #hl_err enumeration)
    uint16_t status;
    /// Complete Length of the attribute value
    uint16_t att_length;
    /// Value length
    uint16_t value_length;
    /// Attribute value (starting from data offset)
    uint8_t value[__ARRAY_EMPTY];
} gatt_srv_att_read_get_cfm_t;

/// #GATT_SRV_ATT_EVENT_GET request indication structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_SRV_ATT_EVENT_GET
    uint16_t req_ind_code;
    /// Token provided by GATT module that must be used into the #GATT_CFM message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Metadata information provided by API user (see \glos{METAINFO})
    uint16_t metainfo;
    /// Attribute handle
    uint16_t hdl;
    /// Maximum value length to return
    uint16_t max_length;
} gatt_srv_att_event_get_req_ind_t;

/// #GATT_SRV_ATT_EVENT_GET confirm structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_SRV_ATT_EVENT_GET
    uint16_t req_ind_code;
    /// Token provided by GATT module into the #GATT_REQ_IND message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Status of the request by GATT user (see #hl_err enumeration)
    uint16_t status;
    /// Complete Length of the attribute value
    uint16_t att_length;
    /// Value length
    uint16_t value_length;
    /// Attribute value (starting from data offset)
    uint8_t value[__ARRAY_EMPTY];
} gatt_srv_att_event_get_cfm_t;

/// #GATT_SRV_ATT_INFO_GET request indication structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_SRV_ATT_INFO_GET
    uint16_t req_ind_code;
    /// Token provided by GATT module that must be used into the #GATT_CFM message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Attribute handle
    uint16_t hdl;
} gatt_srv_att_info_get_req_ind_t;

/// #GATT_SRV_ATT_INFO_GET confirm structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_SRV_ATT_INFO_GET
    uint16_t req_ind_code;
    /// Token provided by GATT module into the #GATT_REQ_IND message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Status of the request by GATT user (see #hl_err enumeration)
    uint16_t status;
    /// Attribute Value length
    uint16_t att_length;
} gatt_srv_att_info_get_cfm_t;

/// #GATT_SRV_ATT_VAL_SET request indication structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_SRV_ATT_VAL_SET
    uint16_t req_ind_code;
    /// Token provided by GATT module that must be used into the #GATT_CFM message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Attribute handle
    uint16_t hdl;
    /// Value offset
    uint16_t offset;
    /// Value length to write
    uint16_t value_length;
    /// Attribute value to update (starting from offset)
    uint8_t value[__ARRAY_EMPTY];
} gatt_srv_att_val_set_req_ind_t;

/// #GATT_SRV_ATT_VAL_SET confirm structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_SRV_ATT_VAL_SET
    uint16_t req_ind_code;
    /// Token provided by GATT module into the #GATT_REQ_IND message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Status of the request by GATT user (see #hl_err enumeration)
    uint16_t status;
} gatt_srv_att_val_set_cfm_t;

/// #GATT_CLI_ATT_VAL_GET request indication structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_CLI_ATT_VAL_GET
    uint16_t req_ind_code;
    /// Token provided by GATT module that must be used into the #GATT_CFM message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Metadata information provided by API user (see \glos{METAINFO}), 0x0000 otherwise
    uint16_t metainfo;
    /// Attribute handle
    uint16_t hdl;
    /// Data offset
    uint16_t offset;
    /// Maximum value length to return
    uint16_t max_length;
} gatt_cli_att_val_get_req_ind_t;

/// #GATT_CLI_ATT_VAL_GET confirm structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_CLI_ATT_VAL_GET
    uint16_t req_ind_code;
    /// Token provided by GATT module into the #GATT_REQ_IND message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Status of the request by GATT user (see #hl_err enumeration)
    uint16_t status;
    /// Value length
    uint16_t value_length;
    /// Attribute value (starting from data offset)
    uint8_t value[__ARRAY_EMPTY];
} gatt_cli_att_val_get_cfm_t;

/// #GATT_CLI_ATT_EVENT request indication structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_CLI_ATT_EVENT
    uint16_t req_ind_code;
    /// Token provided by GATT module that must be used into the #GATT_CFM message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Event type triggered (see #gatt_evt_type enumeration)
    uint8_t evt_type;
    /// 1: if event value if complete value has been received
    /// 0: if data received is equals to maximum attribute protocol value. In such case GATT Client User should
    ///    perform a read procedure.
    uint8_t complete;
    /// Attribute handle
    uint16_t hdl;
    /// Value length
    uint16_t value_length;
    /// Attribute value
    uint8_t value[__ARRAY_EMPTY];
} gatt_cli_att_event_req_ind_t;

/// #GATT_CLI_ATT_EVENT confirm structure
/*@TRACE*/
typedef struct
{
    /// Request Indication code (see #gatt_req_ind_code enumeration)
    /// - #GATT_CLI_ATT_EVENT
    uint16_t req_ind_code;
    /// Token provided by GATT module into the #GATT_REQ_IND message
    uint16_t token;
    /// GATT user local identifier
    uint8_t user_lid;
    /// Connection index
    uint8_t conidx;
    /// Status of the request by GATT user (see #hl_err enumeration) - Ignored
    uint16_t status;
} gatt_cli_att_event_cfm_t;

/// @} GATT_MSG_STRUCT_API

#endif // GATT_MSG_H_
