/**
 ****************************************************************************************
 *
 * @file acc_mcc_msg.h
 *
 * @brief Audio Content Control - Definition of Kernel Messages (Media Control Client)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_MCC_MSG_H_
#define ACC_MCC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_MCC_MSG Message API
 * @ingroup ACC_MCC
 * @brief Description of Message API for Media Control Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "acc_msg.h"        // ACC Message Definitions
#include "rwip_task.h"      // RW IP Task Definition
#include "acc_mcc.h"        // Audio Content Control - Media Control Client Definitions

/// @addtogroup ACC_MCC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Media Control Service Client module
enum acc_mcc_msg_cmd_code
{
    /// Discover (see #acc_mcc_discover_cmd_t)
    ACC_MCC_DISCOVER = GAF_CODE(ACC, MCC, ACC_MCC_CMD_TYPE_DISCOVER),
    /// Get (see #acc_mcc_get_cmd_t)
    ACC_MCC_GET = GAF_CODE(ACC, MCC, ACC_MCC_CMD_TYPE_GET),
    /// Set Configuration (see #acc_mcc_set_cfg_cmd_t)
    ACC_MCC_SET_CFG = GAF_CODE(ACC, MCC, ACC_MCC_CMD_TYPE_SET_CFG),
    /// Set (see #acc_mcc_set_cmd_t)
    ACC_MCC_SET = GAF_CODE(ACC, MCC, ACC_MCC_CMD_TYPE_SET),
    /// Set Object ID (see #acc_mcc_set_object_id_cmd_t)
    ACC_MCC_SET_OBJECT_ID = GAF_CODE(ACC, MCC, ACC_MCC_CMD_TYPE_SET_OBJECT_ID),
    /// Control (see #acc_mcc_control_cmd_t)
    ACC_MCC_CONTROL = GAF_CODE(ACC, MCC, ACC_MCC_CMD_TYPE_CONTROL),
    /// Search (see #acc_mcc_search_cmd_t)
    ACC_MCC_SEARCH = GAF_CODE(ACC, MCC, ACC_MCC_CMD_TYPE_SEARCH),
};

/// List of GAF_REQ request code values for Media Control Service Client module
enum acc_mcc_msg_req_code
{
    /// Configure (see #acc_mcc_configure_req_t)
    ACC_MCC_CONFIGURE = GAF_CODE(ACC, MCC, 0),
    /// Restore Bond Data (see #acc_mcc_restore_bond_data_req_t)
    ACC_MCC_RESTORE_BOND_DATA = GAF_CODE(ACC, MCC, 1),
};

/// List of GAF_IND indication code values for Media Control Service Client module
enum acc_mcc_msg_ind_code
{
    /// Bond Data (see #acc_mcc_bond_data_ind_t)
    ACC_MCC_BOND_DATA = GAF_CODE(ACC, MCC, 0),
    /// Long Value (see #acc_mcc_value_long_ind_t)
    ACC_MCC_VALUE_LONG = GAF_CODE(ACC, MCC, 2),
    /// Value (see #acc_mcc_value_ind_t)
    ACC_MCC_VALUE = GAF_CODE(ACC, MCC, 3),
    /// Object ID (see #acc_mcc_object_id_ind_t)
    ACC_MCC_OBJECT_ID = GAF_CODE(ACC, MCC, 4),
    /// Track Changed (see #acc_mcc_track_changed_ind_t)
    ACC_MCC_TRACK_CHANGED = GAF_CODE(ACC, MCC, 5),
    /// Included Service Found (see #acc_mcc_included_svc_ind_t)
    ACC_MCC_INCLUDED_SVC = GAF_CODE(ACC, MCC, 6),
    /// Service Changed (see #acc_mcc_svc_changed_ind_t)
    ACC_MCC_SVC_CHANGED = GAF_CODE(ACC, MCC, 7),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ACC_MCC_DISCOVER command message
typedef struct acc_mcc_discover_cmd
{
    /// Command code (shall be set to #ACC_MCC_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum number of (Generic) Media Control Service instance that can be found
    uint8_t nb_mcs_max;
    /// Start handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t shdl;
    /// End handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t ehdl;
} acc_mcc_discover_cmd_t;

/// Structure for #ACC_MCC_GET command message
typedef struct acc_mcc_get_cmd
{
    /// Command code (shall be set to #ACC_MCC_GET)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
} acc_mcc_get_cmd_t;

/// Structure for #ACC_MCC_SET_CFG command message
typedef struct acc_mcc_set_cfg_cmd
{
    /// Command code (shall be set to #ACC_MCC_SET_CFG)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Indicate if sending of notifications must be enabled (!=0) or disabled
    uint8_t enable;
} acc_mcc_set_cfg_cmd_t;

/// Structure for #ACC_MCC_SET command message
typedef struct acc_mcc_set_cmd
{
    /// Command code (shall be set to #ACC_MCC_SET)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Indicate if feedback from Server device is required (!= 0) or not
    uint8_t reliable;
    /// Union
    union
    {
        /// Value
        uint32_t val;
        /// Track position
        int32_t track_pos;
        /// Playback speed
        int8_t playback_speed;
        /// Playing order
        uint8_t playing_order;
    } u;
} acc_mcc_set_cmd_t;

/// Structure for #ACC_MCC_SET_OBJECT_ID command message
typedef struct acc_mcc_set_object_id_cmd
{
    /// Command code (shall be set to #ACC_MCC_SET_OBJECT_ID)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Indicate if feedback from Server device is required (!= 0) or not
    uint8_t reliable;
    /// Object ID
    acc_mc_object_id_t obj_id;
} acc_mcc_set_object_id_cmd_t;

/// Structure for #ACC_MCC_CONTROL command message
typedef struct acc_mcc_control_cmd
{
    /// Command code (shall be set to #ACC_MCC_CONTROL)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Operation code
    uint8_t opcode;
    /// Indicate if feedback from Server device is required (!= 0) or not
    uint8_t reliable;
    /// Union
    union
    {
        /// Value
        int32_t val;
        /// Parameter for Goto Segment operation code
        int32_t segment;
        /// Parameter for Move Relative operation code
        int32_t offset;
        /// Parameter for Goto Track operation code
        int32_t track;
        /// Parameter for Goto Group operation code
        int32_t group;
    } u;
} acc_mcc_control_cmd_t;

/// Structure for #ACC_MCC_SEARCH command message
typedef struct acc_mcc_search_cmd
{
    /// Command code (shall be set to #ACC_MCC_SEARCH)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Indicate if feedback from Server device is required (!= 0) or not
    uint8_t reliable;
    /// Length of parameter value
    uint8_t param_len;
    /// Pointer to parameter value
    uint8_t param[__ARRAY_EMPTY];
} acc_mcc_search_cmd_t;

/// Structure for command complete event message
typedef struct acc_mcc_cmp_evt
{
    /// Command code (see #acc_mcc_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Union
    union
    {
        /// Additional parameter
        uint8_t param;
        /// Characteristic type
        uint8_t char_type;
        /// Operation code
        uint8_t opcode;
    } u;
    /// Result
    uint8_t result;
} acc_mcc_cmp_evt_t;

/// Structure for #ACC_MCC_CONFIGURE request message
typedef struct acc_mcc_configure_req
{
    /// Request code (shall be set to #ACC_MCC_CONFIGURE)
    uint16_t req_code;
    /// Preferred MTU
    uint16_t pref_mtu;
} acc_mcc_configure_req_t;

/// Structure for #ACC_MCC_RESTORE_BOND_DATA request message
typedef struct acc_mcc_restore_bond_data_req
{
    /// Request code (shall be set to #ACC_MCC_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Number of instances of the (Generic) Media Control Service discovered
    uint8_t nb_media;
    /// Content description of (Generic) Media Control Service instances
    acc_mcc_mcs_info_t mcs_info[__ARRAY_EMPTY];
} acc_mcc_restore_bond_data_req_t;

/// Structure for response message
typedef struct acc_mcc_rsp
{
    /// Request code (see #acc_mcc_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} acc_mcc_rsp_t;

/// Structure for #ACC_MCC_BOND_DATA indication message
typedef struct acc_mcc_bond_data_ind
{
    /// Indication code (shall be set to #ACC_MCC_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Content description of (Generic) Media Control Service instance
    acc_mcc_mcs_info_t mcs_info;
} acc_mcc_bond_data_ind_t;

/// Structure for #ACC_MCC_OBJECT_ID indication message
typedef struct acc_mcc_object_id_ind
{
    /// Indication code (shall be set to #ACC_MCC_OBJECT_ID)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Object ID
    acc_mc_object_id_t obj_id;
} acc_mcc_object_id_ind_t;

/// Structure for #ACC_MCC_INCLUDED_SVC indication message
typedef struct acc_mcc_included_svc_ind
{
    /// Indication code (shall be set to #ACC_MCC_INCLUDED_SVC)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Start handle
    uint16_t shdl;
    /// End handle
    uint16_t ehdl;
} acc_mcc_included_svc_ind_t;

/// Structure for #ACC_MCC_TRACK_CHANGED indication message
typedef struct acc_mcc_track_changed_ind
{
    /// Indication code (shall be set to #ACC_MCC_TRACK_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
} acc_mcc_track_changed_ind_t;

/// Structure for #ACC_MCC_VALUE_LONG indication message
typedef struct acc_mcc_value_long_ind
{
    /// Indication code (shall be set to #ACC_MCC_VALUE_LONG)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Length of value
    uint16_t val_len;
    /// Value
    uint8_t val[__ARRAY_EMPTY];
} acc_mcc_value_long_ind_t;

/// Structure for #ACC_MCC_VALUE indication message
typedef struct acc_mcc_value_ind
{
    /// Indication code (shall be set to #ACC_MCC_VALUE)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Union
    union
    {
        /// Value
        uint32_t val;
        /// Playback speed
        int8_t playback_speed;
        /// Seeking speed
        int8_t seeking_speed;
        /// Media state
        uint8_t state;
        /// Supported media control operation codes bit field
        uint32_t opcodes_supp_bf;
        /// Playing Order
        uint8_t playing_order;
        /// Supported Playing Order bit field
        uint32_t playing_order_supp_bf;
        /// Track duration of the current track in 0.01 second resolution
        int32_t track_dur;
        /// Track position of the current track in 0.01 second resolution
        int32_t track_pos;
        /// Content Control ID
        uint8_t ccid;
    } val;
} acc_mcc_value_ind_t;

/// Structure for #ACC_MCC_SVC_CHANGED indication message
typedef struct acc_mcc_svc_changed_ind
{
    /// Indication code (shall be set to #ACC_MCC_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} acc_mcc_svc_changed_ind_t;

/// @} ACC_MCC_MSG

#endif // ACC_MCC_MSG_H_
