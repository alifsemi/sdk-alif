/**
 ****************************************************************************************
 *
 * @file otc_msg.h
 *
 * @brief Object Transfer Client - Message API Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef OTC_MSG_H_
#define OTC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup OTC_MSG_API Message API
 * @ingroup OTC
 * @brief Description of Message API for Object Transfer Profile Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "otc.h"           // Object Transfer Client Definitions

#if (BLE_OT_CLIENT)

/// @addtogroup OTC_MSG_API
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Object Transfer Client
enum otc_msg_id
{
    OTC_CMD = MSG_ID(OTC, 0x00),
    OTC_CMP_EVT = MSG_ID(OTC, 0x01),
    OTC_REQ = MSG_ID(OTC, 0x02),
    OTC_RSP = MSG_ID(OTC, 0x03),
    OTC_IND = MSG_ID(OTC, 0x04),
};

/// List of OTC_REQ request codes
enum otc_msg_req_codes
{
    OTC_RESTORE_BOND_DATA = 0x0000,
};

/// List of OTC_IND indication codes
enum otc_msg_ind_codes
{
    OTC_UNKNOWN_MSG = 0x0000,
    OTC_BOND_DATA = 0x0001,
    OTC_EXECUTE_RSP = 0x0002,
    OTC_VALUE = 0x0003,
    OTC_TIME = 0x0004,
    OTC_OBJECT_ID = 0x0005,
    OTC_TYPE = 0x0006,
    OTC_NAME = 0x0007,
    OTC_FILTER = 0x0008,
    OTC_FILTER_TIME = 0x0009,
    OTC_FILTER_SIZE = 0x000A,
    OTC_FILTER_NAME = 0x000B,
    OTC_FILTER_TYPE = 0x000C,
    OTC_CHANGED = 0x000E,
    OTC_COC_CONNECTED = 0x000F,
    OTC_COC_DISCONNECTED = 0x0010,
    OTC_COC_DATA = 0x0011,
    OTC_SVC_CHANGED = 0x0012,
};

/*
 * KERNEL MESSAGES
 ****************************************************************************************
 */

/// Basic structure for OTC_CMD message
typedef struct otc_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
} otc_cmd_t;

/// Basic structure for OTC_REQ message
typedef struct otc_req
{
    /// Request code (see enum #otc_msg_req_codes)
    uint16_t req_code;
} otc_req_t;

/// Basic structure for OTC_IND message
typedef struct otc_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
} otc_ind_t;

/// Structure for OTC_DISCOVER command message
typedef struct otc_discover_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum number of Object Transfer Service instances that
    /// can be found
    uint8_t nb_ots_max;
    /// Service type
    uint8_t svc_type;
    /// Start handle
    uint16_t shdl;
    /// End handle
    uint16_t ehdl;
} otc_discover_cmd_t;

/// Structure for OTC_GET command message
typedef struct otc_get_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Get type (see enum #otc_get_type)
    uint8_t get_type;
    /// Characteristic type (see enum #otp_char_type)
    /// Meaningful only for OTC_GET_TYPE_SINGLE get type
    uint8_t char_type;
} otc_get_cmd_t;

/// Structure for OTC_SET_NAME command message
typedef struct otc_set_name_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Name length
    uint8_t name_len;
    /// Name
    uint8_t name[__ARRAY_EMPTY];
} otc_set_name_cmd_t;

/// Structure for OTC_SET_TIME command message
typedef struct otc_set_time_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Characteristic type (see enum #otp_char_type)
    uint8_t char_type;
    /// First-Created time or Last-Modified time
    prf_date_time_t time;
} otc_set_time_cmd_t;

/// Structure for OTC_SET_PROPERTIES command message
typedef struct otc_set_properties_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Properties (see enum #otp_prop_bf)
    uint32_t properties;
} otc_set_properties_cmd_t;

/// Structure for OTC_SET_CFG command message
typedef struct otc_set_cfg_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Indication-capable characteristic type (see enum #otp_char_type)
    uint8_t char_type;
    /// Indicate if sending of indications must be enabled (!=0) or not for
    /// the indicated characteristic
    uint8_t enable;
} otc_set_cfg_cmd_t;

/// Structure for OTC_OBJECT_CREATE command message
typedef struct otc_object_create_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
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
} otc_object_create_cmd_t;

/// Structure for OTC_OBJECT_CONTROL command message
typedef struct otc_object_control_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Operation code value (see enum #otp_oacp_opcode)
    uint8_t opcode;
} otc_object_control_cmd_t;

/// Structure for OTC_OBJECT_MANIPULATE command message
typedef struct otc_object_manipulate_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Operation code value (see enum #otp_oacp_opcode)
    uint8_t opcode;
    /// Position of the first octet to calculate checksum/read/write/
    /// on the Current Object
    uint32_t offset;
    /// Total number of octets to calculate checksum/read/write
    /// on the Current Object
    uint32_t length;
    /// Mode bit field (see enum #otp_oacp_mode_bf)
    /// Meaningful only for write operation
    uint8_t mode;
} otc_object_manipulate_cmd_t;

/// Structure for OTC_OBJECT_EXECUTE command message
typedef struct otc_object_execute_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Parameter length
    uint16_t param_len;
    /// Execution parameter
    uint8_t param[__ARRAY_EMPTY];
} otc_object_execute_cmd_t;

/// Structure for OTC_LIST_CONTROL command message
typedef struct otc_list_control_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Operation code value (see enum #otp_olcp_opcode)
    uint8_t opcode;
    /// List Sort Order (see enum #otp_olcp_order)
    /// Meaningful only for Order operation code
    uint8_t order;
} otc_list_control_cmd_t;

/// Structure for OTC_LIST_GOTO command message
typedef struct otc_list_goto_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Operation code value (see enum #otp_olcp_opcode)
    uint8_t opcode;
    /// Object ID
    /// Meaningful only for Order operation code
    ot_object_id_t object_id;
} otc_list_goto_cmd_t;

/// Structure for OTC_FILTER_SET command message
typedef struct otc_filter_set_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
} otc_filter_set_cmd_t;

/// Structure for OTC_FILTER_SET_TIME command message
typedef struct otc_filter_set_time_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Left boundary of the timestamp interval
    prf_date_time_t time_start;
    /// Right boundary of the timestamp interval
    prf_date_time_t time_end;
} otc_filter_set_time_cmd_t;

/// Structure for OTC_FILTER_SET_SIZE command message
typedef struct otc_filter_set_size_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Left boundary of the size interval
    uint32_t size_min;
    /// Right boundary of the size interval
    uint32_t size_max;
} otc_filter_set_size_cmd_t;

/// Structure for OTC_FILTER_SET_NAME command message
typedef struct otc_filter_set_name_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Name length
    uint8_t name_len;
    /// Name
    uint8_t name[__ARRAY_EMPTY];
} otc_filter_set_name_cmd_t;

/// Structure for OTC_FILTER_SET_TYPE command message
typedef struct otc_filter_set_type_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
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
} otc_filter_set_type_cmd_t;

/// Structure for OTC_COC_CONNECT command message
typedef struct otc_coc_connect_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum SDU size that the local device can receive
    uint16_t local_max_sdu;
} otc_coc_connect_cmd_t;

/// Structure for OTC_COC_DISCONNECT command message
typedef struct otc_coc_disconnect_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
} otc_coc_disconnect_cmd_t;

/// Structure for OTC_COC_SEND command message
typedef struct otc_coc_send_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// SDU data length
    uint16_t length;
    /// SDU data to be tranferred to the peer device
    uint8_t sdu[__ARRAY_EMPTY];
} otc_coc_send_cmd_t;

/// Structure for OTC_COC_RELEASE command message
typedef struct otc_coc_release_cmd
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
} otc_coc_release_cmd_t;

/// Structure command complete event
typedef struct otc_cmp_evt
{
    /// Command code (see enum #otc_cmd_codes)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Value 1
    union
    {
        /// Indication-capable characteristic type (see enum #otp_char_type)
        /// For OTC_SET_CFG command code
        uint8_t char_type;
        /// Filter local index
        /// For OTC_FILTER_SET, OTC_FILTER_SET_TIME, OTC_FILTER_SET_SIZE,
        /// OTC_FILTER_SET_NAME, OTC_FILTER_SET_TYPE command codes
        uint8_t filter_lid;
        /// Operation code value for Object Action Control Point or Object List Control Point
        /// For OTC_OBJECT_CREATE, OTC_OBJECT_CONTROL, OTC_OBJECT_MANIPULATE,
        /// OTC_OBJECT_EXECUTE, OTC_LIST_CONTROL, OTC_LIST_GOTO command codes
        uint8_t opcode;
    } value_1;
    /// Value 2
    union
    {
        /// Get type (see enum #otc_get_type)
        /// For OTC_GET command code
        uint8_t get_type;
        /// Result code for Object Action Control Point or Object List Control Point
        /// For OTC_OBJECT_CREATE, OTC_OBJECT_CONTROL, OTC_OBJECT_MANIPULATE,
        /// OTC_OBJECT_EXECUTE, OTC_LIST_CONTROL, OTC_LIST_GOTO command codes
        uint8_t result_code;
    } value_2;
    /// Value 3
    union
    {
        /// Characteristic type (see enum #otp_char_type)
        /// For OTC_GET, OTC_SET_NAME, OTC_SET_TIME, OTC_SET_PROPERTIES command code
        uint8_t char_type;
        /// Total number of Objects value included in the response
        /// For OTC_LIST_CONTROL command code
        uint32_t nb_object;
        /// Calculated checksum value included in the response
        /// For OTC_OBJECT_MANIPULATE command code
        uint32_t checksum;
    } value_3;
} otc_cmp_evt_t;

/// Structure for OTC_RESTORE_BOND_DATA request message
typedef struct otc_restore_bond_data_req
{
    /// Request code (see enum #otc_msg_req_codes)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Number of Object Transfer Service instance discovered in peer device database
    uint8_t nb_ots;
    /// Content description of each Object Transfer Service instance
    otc_ots_info_t ots_info[__ARRAY_EMPTY];
} otc_restore_bond_data_req_t;

/// Structure for response message
typedef struct otc_rsp
{
    /// Request code (see enum #otc_msg_req_codes)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} otc_rsp_t;

/// Structure for OTC_UNKNOWN_MSG indication message
typedef struct otc_unknown_msg_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Message ID
    ke_msg_id_t msg_id;
} otc_unknown_msg_ind_t;

/// Structure for OTC_BOND_DATA indication message
typedef struct otc_bond_data_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Content description of Object Transfer Service instance
    otc_ots_info_t ots_info;
} otc_bond_data_ind_t;

/// Structure for OTC_EXECUTE_RSP indication message
typedef struct otc_execute_rsp_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Response parameter length
    uint8_t rsp_len;
    /// Response parameter
    uint8_t rsp[__ARRAY_EMPTY];
} otc_execute_rsp_ind_t;

/// Structure for OTC_VALUE indication message
typedef struct otc_value_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Characteristic type (see enum #otp_char_type)
    uint8_t char_type;
    /// Value 1
    union
    {
        /// Object Action Control Point features (see enum #otp_oacp_feat_bf)
        uint32_t oacp_features;
        /// Current Size field represents the actual number of octets of the object
        uint32_t current_size;
        /// Object Properties field (see enum #otp_prop_bf)
        uint32_t properties;
    } value_1;
    /// Value 2
    union
    {
        /// Object List Control Point features (see enum #otp_olcp_feat_bf)
        uint32_t olcp_features;
        /// Allocated Size field represents the number of octets allocated for the object
        uint32_t allocated_size;
    } value_2;
} otc_value_ind_t;

/// Structure for OTC_TIME indication message
typedef struct otc_time_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Characteristic type (see enum #otp_char_type)
    uint8_t char_type;
    /// First-Created time or Last-Modified time
    prf_date_time_t time;
} otc_time_ind_t;

/// Structure for OTC_OBJECT_ID indication message
typedef struct otc_object_id_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Object ID
    ot_object_id_t object_id;
} otc_object_id_ind_t;

/// Structure for OTC_TYPE indication message
typedef struct otc_type_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
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
} otc_type_ind_t;

/// Structure for OTC_NAME indication message
typedef struct otc_name_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Name length
    uint8_t name_len;
    /// Name
    uint8_t name[__ARRAY_EMPTY];
} otc_name_ind_t;

/// Structure for OTC_FILTER indication message
typedef struct otc_filter_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
} otc_filter_ind_t;

/// Structure for OTC_FILTER_TIME indication message
typedef struct otc_filter_time_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Left boundary of the timestamp interval
    prf_date_time_t time_start;
    /// Right boundary of the timestamp interval
    prf_date_time_t time_end;
} otc_filter_time_ind_t;

/// Structure for OTC_FILTER_SIZE indication message
typedef struct otc_filter_size_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Left boundary of the size interval
    uint32_t size_min;
    /// Right boundary of the size interval
    uint32_t size_max;
} otc_filter_size_ind_t;

/// Structure for OTC_FILTER_NAME indication message
typedef struct otc_filter_name_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
    /// Filter value (see enum #otp_filter_type)
    uint8_t filter_val;
    /// Name length
    uint8_t name_len;
    /// Name
    uint8_t name[__ARRAY_EMPTY];
} otc_filter_name_ind_t;

/// Structure for OTC_FILTER_TYPE indication message
typedef struct otc_filter_type_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Filter local index
    uint8_t filter_lid;
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
} otc_filter_type_ind_t;

/// Structure for OTC_CHANGED indication message
typedef struct otc_changed_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Transfer local index
    uint8_t transfer_lid;
    /// Flags field (see enum #otp_changed_flags_bf)
    uint8_t flags;
    /// Object ID
    ot_object_id_t object_id;
} otc_changed_ind_t;

/// Structure for OTC_COC_CONNECTED indication message
typedef struct otc_coc_connected_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum SDU size that the peer on the link can receive
    uint16_t peer_max_sdu;
    /// Maximum SDU size that the local device can receive
    uint16_t local_max_sdu;
} otc_coc_connected_ind_t;

/// Structure for OTC_COC_DISCONNECTED indication message
typedef struct otc_coc_disconnected_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Disconnection reason
    uint16_t reason;
} otc_coc_disconnected_ind_t;

/// Structure for OTC_COC_DATA indication message
typedef struct otc_coc_data_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// SDU data length
    uint16_t length;
    /// SDU data
    uint8_t sdu[__ARRAY_EMPTY];
} otc_coc_data_ind_t;

/// Structure for OTC_SVC_CHANGED indication message
typedef struct otc_svc_changed_ind
{
    /// Indication code (see enum #otc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} otc_svc_changed_ind_t;

/// @} OTC_MSG_API

#endif //(BLE_OT_CLIENT)

#endif // OTC_MSG_H_
