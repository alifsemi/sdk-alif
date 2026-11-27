/**
 ****************************************************************************************
 *
 * @file ots_msg.h
 *
 * @brief Object Transfer Server - Message API Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef OTS_MSG_H_
#define OTS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup OTS_MSG_API Message API
 * @ingroup OTS
 * @brief Description of Message API for Object Transfer Profile Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <ots.h>           // Object Transfer Server Definitions

#if (BLE_OT_SERVER)

/// @addtogroup OTS_MSG_API
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Object Transfer Server
enum ots_msg_id
{
    OTS_CMD = MSG_ID(OTS, 0x00),
    OTS_CMP_EVT = MSG_ID(OTS, 0x01),
    OTS_REQ = MSG_ID(OTS, 0x02),
    OTS_RSP = MSG_ID(OTS, 0x03),
    OTS_IND = MSG_ID(OTS, 0x04),
    OTS_REQ_IND = MSG_ID(OTS, 0x05),
    OTS_CFM = MSG_ID(OTS, 0x06),
};

/// List of OTS_REQ request codes
enum ots_msg_req_codes
{
    OTS_ADD = 0x0000,
    OTS_RESTORE_BOND_DATA = 0x0001,
    OTS_OBJECT_ADD = 0x0002,
    OTS_OBJECT_REMOVE = 0x0003,
    OTS_OBJECT_CHANGE = 0x0004,
    OTS_OBJECT_CHANGED = 0x0005,
    OTS_SET = 0x0006,
    OTS_SET_TIME = 0x0007,
};

/// List of OTS_IND indication codes
enum ots_msg_ind_codes
{
    OTS_UNKNOWN_MSG = 0x0000,
    OTS_BOND_DATA = 0x0001,
    OTS_COC_CONNECTED = 0x0002,
    OTS_COC_DISCONNECTED = 0x0003,
    OTS_COC_DATA = 0x0004,
};

/*
 * KERNEL MESSAGES
 ****************************************************************************************
 */

/// Basic structure for OTS_CMD message
typedef struct ots_cmd
{
    /// Command code (see enum #ots_cmd_codes)
    uint16_t cmd_code;
} ots_cmd_t;

/// Basic structure for OTS_REQ message
typedef struct ots_req
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
} ots_req_t;

/// Basic structure for OTS_IND message
typedef struct ots_ind
{
    /// Indication code (see enum #ots_msg_ind_codes)
    uint16_t ind_code;
} ots_ind_t;

/// Basic structure for OTS_CFM message
typedef struct ots_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
} ots_cfm_t;

/// Structure for OTS_COC_DISCONNECT command message
typedef struct ots_coc_disconnect_cmd
{
    /// Command code (see enum #ots_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
} ots_coc_disconnect_cmd_t;

/// Structure for OTS_COC_SEND command message
typedef struct ots_coc_send_cmd
{
    /// Command code (see enum #ots_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// SDU data length
    uint16_t length;
    /// SDU data to be tranferred to the peer device
    uint8_t sdu[__ARRAY_EMPTY];
} ots_coc_send_cmd_t;

/// Structure for OTS_COC_RELEASE command message
typedef struct ots_coc_release_cmd
{
    /// Command code (see enum #ots_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
} ots_coc_release_cmd_t;

/// Structure command complete event
typedef struct ots_cmp_evt
{
    /// Command code (see enum #ots_cmd_codes)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} ots_cmp_evt_t;

/// Structure for OTS_ADD request message
typedef struct ots_add_req
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
    /// Configuration bit field (see enum #ots_add_cfg_bf)
    uint16_t cfg_bf;
    /// Required start handle
    /// If set to GATT_INVALID_HDL, the start handle will be automatically chosen
    uint16_t shdl;
    /// Object Action Control Point features
    uint32_t oacp_features;
    /// Object List Control Point features
    uint32_t olcp_features;
} ots_add_req_t;

/// Structure for OTS_RESTORE_BOND_DATA request message
typedef struct ots_restore_bond_data_req
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Client configuration bit field (see enum #ots_cli_cfg_bf)
    uint8_t cli_cfg_bf;
    /// Event configuration bit field (see enum #ots_cli_cfg_bf)
    uint8_t evt_cfg_bf;
    /// Number of Object Changed indication to send
    uint8_t nb_changes;
    /// Object Changed Information
    ots_changed_info_t changed_info[__ARRAY_EMPTY];
} ots_restore_bond_data_req_t;

/// Structure for OTS_OBJECT_ADD request message
typedef struct ots_object_add_req
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
    /// Object ID
    ot_object_id_t object_id;
    /// Current Size field represents the actual number of octets of the object
    uint32_t current_size;
    /// Allocated Size field represents the number of octets allocated for the object
    uint32_t allocated_size;
    /// First-Created time
    prf_date_time_t first_created_time;
    /// Last-Modified time
    prf_date_time_t last_modified_time;
    /// Object Properties field
    uint32_t properties;
    /// Indicate if UUID of the newly created object is 128-bit (!=0) or 16-bit
    uint8_t uuid_type;
    /// UUID
    union
    {
        /// 16-bit UUID
        uint16_t uuid;
        /// 128-bit UUID
        uint8_t long_uuid[GATT_UUID_128_LEN];
    } uuid;
} ots_object_add_req_t;

/// Structure for OTS_OBJECT_REMOVE request message
typedef struct ots_object_remove_req
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
    /// Object local index
    uint8_t object_lid;
} ots_object_remove_req_t;

/// Structure for OTS_OBJECT_CHANGE request message
typedef struct ots_object_change_req
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Object local index
    uint8_t object_lid;
} ots_object_change_req_t;

/// Structure for OTS_OBJECT_CHANGED request message
typedef struct ots_object_changed_req
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
    /// Flags field (see enum #otp_changed_flags_bf)
    uint8_t flags;
    /// Object ID
    ot_object_id_t object_id;
} ots_object_changed_req_t;

/// Structure for OTS_SET request message
typedef struct ots_set_req
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
    /// Object local index
    uint8_t object_lid;
    /// Set type (see enum #ots_set_type)
    uint8_t set_type;
    /// Value
    union
    {
        /// Current Size field represents the actual number of octets of the object
        uint32_t current_size;
        /// Object Properties field (see enum #otp_prop_bf)
        uint32_t properties;
        /// Number of objects in the list
        uint32_t nb_object;
    } value;
} ots_set_req_t;

/// Structure for OTS_SET_TIME request message
typedef struct ots_set_time_req
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
    /// Object local index
    uint8_t object_lid;
    /// Last-Modified time
    prf_date_time_t time;
} ots_set_time_req_t;

/// Structure for response message
typedef struct ots_rsp
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Object local index
    uint8_t object_lid;
} ots_rsp_t;

/// Structure for OTS_ADD response message
typedef struct ots_add_rsp
{
    /// Request code (see enum #ots_msg_req_codes)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Start handle
    uint16_t start_handle;
} ots_add_rsp_t;

/// Structure for OTS_UNKNOWN_MSG indication message
typedef struct ots_unknown_msg_ind
{
    /// Indication code (see enum #ots_msg_ind_codes)
    uint16_t ind_code;
    /// Message ID
    ke_msg_id_t msg_id;
} ots_unknown_msg_ind_t;

/// Structure for OTS_BOND_DATA indication message
typedef struct ots_bond_data_ind
{
    /// Indication code (see enum #ots_msg_ind_codes)
    uint16_t ind_code;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field (see enum #ots_cli_cfg_bf)
    uint8_t cli_cfg_bf;
} ots_bond_data_ind_t;

/// Structure for OTS_COC_CONNECTED indication message
typedef struct ots_coc_connected_ind
{
    /// Indication code (see enum #ots_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum SDU size that the peer on the link can receive
    uint16_t peer_max_sdu;
    /// Maximum SDU size that the local device can receive
    uint16_t local_max_sdu;
} ots_coc_connected_ind_t;

/// Structure for OTS_COC_DISCONNECTED indication message
typedef struct ots_coc_disconnected_ind
{
    /// Indication code (see enum #ots_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Disconnection reason
    uint16_t reason;
} ots_coc_disconnected_ind_t;

/// Structure for OTS_COC_DATA indication message
typedef struct ots_coc_data_ind
{
    /// Indication code (see enum #ots_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// SDU data length
    uint16_t length;
    /// SDU data
    uint8_t sdu[__ARRAY_EMPTY];
} ots_coc_data_ind_t;

/// Structure for OTS_GET_NAME request indication message
typedef struct ots_get_name_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Object local index
    uint8_t object_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Offset
    uint16_t offset;
    /// Maximum length
    uint16_t max_len;
} ots_get_name_req_ind_t;

/// Structure for OTS_SET_NAME request indication message
typedef struct ots_set_name_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Object local index
    uint8_t object_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Name length
    uint8_t name_len;
    /// Name
    uint8_t name[__ARRAY_EMPTY];
} ots_set_name_req_ind_t;

/// Structure for OTS_OBJECT_CREATE request indication message
typedef struct ots_object_create_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Minimum size of memory allocated for the object
    uint32_t size;
    /// Indicate if UUID of the newly created object is 128-bit (!=0) or 16-bit
    uint8_t uuid_type;
    /// UUID
    union
    {
        /// 16-bit UUID
        uint16_t uuid;
        /// 128-bit UUID
        uint8_t long_uuid[GATT_UUID_128_LEN];
    } uuid;
} ots_object_create_req_ind_t;

/// Structure for OTS_OBJECT_EXECUTE request indication message
typedef struct ots_object_execute_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Object local index
    uint8_t object_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Parameter length
    uint16_t param_len;
    /// Execution parameter
    uint8_t param[__ARRAY_EMPTY];
} ots_object_execute_req_ind_t;

/// Structure for OTS_OBJECT_MANIPULATE request indication message
typedef struct ots_object_manipulate_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Object local index
    uint8_t object_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Operation code value (see enum #otp_oacp_opcode)
    uint8_t opcode;
    /// Position of the first octet to calculate checksum/read/write/
    /// on the Current Object
    uint32_t offset;
    /// Total number of octets to calculate checksum/read/write
    /// on the Current Object
    uint32_t length;
    /// Mode bit field, only for write operation (see enum #otp_oacp_mode_bf)
    uint8_t mode;
} ots_object_manipulate_req_ind_t;

/// Structure for OTS_OBJECT_CONTROL request indication message
typedef struct ots_object_control_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Object local index
    uint8_t object_lid;
    /// Token
    uint16_t token;
    /// Operation code value (see enum #otp_oacp_opcode)
    uint8_t opcode;
} ots_object_control_req_ind_t;

/// Structure for OTS_FILTER_GET request indication message
typedef struct ots_filter_get_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Token value to return in the confirmation
    uint32_t ots_token;
    /// Offset
    uint16_t offset;
    /// Maximum length
    uint16_t max_len;
} ots_filter_get_req_ind_t;

/// Structure for OTS_LIST_CONTROL request indication message
typedef struct ots_list_control_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Operation code value (see enum #otp_olcp_opcode)
    uint8_t opcode;
    /// List Sort Order (see enum #otp_olcp_order)
    /// Meaningful only for Order opcode
    uint8_t order;
} ots_list_control_req_ind_t;

/// Structure for OTS_LIST_GOTO request indication message
typedef struct ots_list_goto_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Operation code value (see enum #otp_olcp_opcode)
    uint8_t opcode;
    /// Object ID
    /// Meaningful only for Order opcode
    ot_object_id_t object_id;
} ots_list_goto_req_ind_t;

/// Structure for OTS_FILTER_SET request indication message
typedef struct ots_filter_set_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
} ots_filter_set_req_ind_t;

/// Structure for OTS_FILTER_SET_TIME request indication message
typedef struct ots_filter_set_time_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Left boundary of the timestamp interval
    prf_date_time_t time_start;
    /// Right boundary of the timestamp interval
    prf_date_time_t time_end;
} ots_filter_set_time_req_ind_t;

/// Structure for OTS_FILTER_SET_SIZE request indication message
typedef struct ots_filter_set_size_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Left boundary of the size interval
    uint32_t size_min;
    /// Right boundary of the size interval
    uint32_t size_max;
} ots_filter_set_size_req_ind_t;

/// Structure for OTS_FILTER_SET_NAME request indication message
typedef struct ots_filter_set_name_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Name length
    uint8_t name_len;
    /// Name
    uint8_t name[__ARRAY_EMPTY];
} ots_filter_set_name_req_ind_t;

/// Structure for OTS_FILTER_SET_TYPE request indication message
typedef struct ots_filter_set_type_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Indicate if UUID of the newly created object is 128-bit (!=0) or 16-bit
    uint8_t uuid_type;
    /// UUID
    union
    {
        /// 16-bit UUID
        uint16_t uuid;
        /// 128-bit UUID
        uint8_t long_uuid[GATT_UUID_128_LEN];
    } uuid;
} ots_filter_set_type_req_ind_t;

/// Structure for OTS_COC_CONNECT request indication message
typedef struct ots_coc_connect_req_ind
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Maximum SDU size that the peer on the link can receive
    uint16_t peer_max_sdu;
} ots_coc_connect_req_ind_t;

/// Structure for OTS_GET_NAME confirmation message
typedef struct ots_get_name_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Name length
    uint8_t name_len;
    /// Name
    uint8_t name[__ARRAY_EMPTY];
} ots_get_name_cfm_t;

/// Structure for OTS_SET_NAME confirmation message
typedef struct ots_set_name_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Token value to return in the confirmation
    uint16_t token;
} ots_set_name_cfm_t;

/// Structure for OTS_OBJECT_CREATE/MANIPULATE/CONTROL confirmation messages
typedef struct ots_object_control_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Result code (see enum #otp_oacp_result_code)
    uint8_t result_code;
    /// Calculated checksum value included in the response
    /// if operation code is OTP_OACP_OPCODE_CALCULATE_CHECKSUM and
    /// result code is OTP_OACP_RESULT_SUCCESS
    uint32_t checksum;
} ots_object_control_cfm_t;

/// Structure for OTS_OBJECT_EXECUTE confirmation message
typedef struct ots_object_execute_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Result code (see enum #otp_oacp_result_code)
    uint8_t result_code;
    /// Response parameter length
    uint16_t rsp_len;
    /// Response parameter
    uint8_t rsp[__ARRAY_EMPTY];
} ots_object_execute_cfm_t;

/// Structure for OTS_FILTER_GET confirmation message
/// Used for OTP_FILTER_TYPE_NO_FILTER, OTP_FILTER_TYPE_MARKED_OBJECTS
/// filter values
typedef struct ots_filter_get_empty_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint32_t ots_token;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
} ots_filter_get_empty_cfm_t;

/// Structure for OTS_FILTER_GET confirmation message
/// Used for OTP_FILTER_TYPE_CREATED_BETW, OTP_FILTER_TYPE_MODIFIED_BETW
/// filter values
typedef struct ots_filter_get_time_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint32_t ots_token;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Left boundary of the timestamp interval
    prf_date_time_t time_start;
    /// Right boundary of the timestamp interval
    prf_date_time_t time_end;
} ots_filter_get_time_cfm_t;

/// Structure for OTS_FILTER_GET confirmation message
/// Used for OTP_FILTER_TYPE_CURRENT_SIZE_BETW, OTP_FILTER_TYPE_ALLOCATED_SIZE_BETW
/// filter values
typedef struct ots_filter_get_size_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint32_t ots_token;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Left boundary of the size interval
    uint32_t size_min;
    /// Right boundary of the size interval
    uint32_t size_max;
} ots_filter_get_size_cfm_t;

/// Structure for OTS_FILTER_GET confirmation message
/// Used for OTP_FILTER_TYPE_NAME_STARTS_WITH, OTP_FILTER_TYPE_NAME_ENDS_WITH
/// OTP_FILTER_TYPE_NAME_CONTAINS, OTP_FILTER_TYPE_NAME_IS_EXACTLY
/// filter values
typedef struct ots_filter_get_name_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint32_t ots_token;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Name length
    uint8_t name_len;
    /// Name
    uint8_t name[__ARRAY_EMPTY];
} ots_filter_get_name_cfm_t;

/// Structure for OTS_FILTER_GET confirmation message
/// Used for OTP_FILTER_TYPE_OBJECT_TYPE filter value
typedef struct ots_filter_get_type_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint32_t ots_token;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Indicate if UUID of the newly created object is 128-bit (!=0) or 16-bit
    uint8_t uuid_type;
    /// UUID
    union
    {
        /// 16-bit UUID
        uint16_t uuid;
        /// 128-bit UUID
        uint8_t long_uuid[GATT_UUID_128_LEN];
    } uuid;
} ots_filter_get_type_cfm_t;

/// Structure for OTS_LIST_CONTROL/GOTO confirmation messages
typedef struct ots_list_control_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Result code (see enum #otp_olcp_result_code)
    uint8_t result_code;
    /// Number of objects
    uint32_t nb_object;
} ots_list_control_cfm_t;

/// Structure for OTS_FILTER_SET/SET_NAME/SET_TIME/SET_SIZE/SET_TYPE confirmation messages
typedef struct ots_filter_set_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Token value to return in the confirmation
    uint16_t token;
} ots_filter_set_cfm_t;

/// Structure for OTS_COC_CONNECT confirmation messages
typedef struct ots_coc_connect_cfm
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Maximum SDU size that the local device can receive
    uint16_t local_max_sdu;
} ots_coc_connect_cfm_t;

/// @} OTS_MSG_API

#endif //(BLE_OT_SERVER)

#endif // OTS_MSG_H_
