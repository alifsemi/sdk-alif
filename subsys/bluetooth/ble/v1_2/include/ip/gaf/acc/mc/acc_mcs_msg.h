/**
 ****************************************************************************************
 *
 * @file acc_mcs_msg.h
 *
 * @brief Audio Content Control - Definition of Kernel Messages (Media Control Server)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_MCS_MSG_H_
#define ACC_MCS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_MCS_MSG Message API
 * @ingroup ACC_MCS
 * @brief Description of Message API for Media Control Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "acc_msg.h"        // ACC Message Definitions
#include "rwip_task.h"      // RW IP Task Definition
#include "acc_mcs.h"        // Audio Content Control - Media Control Server Definitions

/// @addtogroup ACC_MCS_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request code values for Media Control Service Server module
enum acc_mcs_msg_req_code
{
    /// Configure (see #acc_mcs_configure_req_t)
    ACC_MCS_CONFIGURE = GAF_CODE(ACC, MCS, 0),
    /// Add (see #acc_mcs_add_req_t)
    ACC_MCS_ADD = GAF_CODE(ACC, MCS, 1),
    /// Restore Bond Data (see #acc_mcs_restore_bond_data_req_t)
    ACC_MCS_RESTORE_BOND_DATA = GAF_CODE(ACC, MCS, 2),
    /// Set (see #acc_mcs_set_req_t)
    ACC_MCS_SET = GAF_CODE(ACC, MCS, 3),
    /// Set Object ID (see #acc_mcs_set_object_id_req_t)
    ACC_MCS_SET_OBJECT_ID = GAF_CODE(ACC, MCS, 4),
    /// Set Player Name (see #acc_mcs_set_player_name_req_t)
    ACC_MCS_SET_PLAYER_NAME = GAF_CODE(ACC, MCS, 5),
    /// Action (see #acc_mcs_action_req_t)
    ACC_MCS_ACTION = GAF_CODE(ACC, MCS, 6),
    /// Track Changed (see #acc_mcs_track_change_req_t)
    ACC_MCS_TRACK_CHANGE = GAF_CODE(ACC, MCS, 7),
};

/// List of GAF_IND indication code values for Media Control Service Server module
enum acc_mcs_msg_ind_code
{
    /// Bond Data (see #acc_mcs_bond_data_ind_t)
    ACC_MCS_BOND_DATA = GAF_CODE(ACC, MCS, 0),
};

/// List of GAF_REQ_IND request indication code values for Media Control Service Server module
enum acc_mcs_msg_req_ind_code
{
    /// Set Object ID (see #acc_mcs_set_object_id_req_ind_t)
    ACC_MCS_SET_OBJECT_ID_RI = GAF_CODE(ACC, MCS, 0),
    /// Control (see #acc_mcs_control_req_ind_t)
    ACC_MCS_CONTROL = GAF_CODE(ACC, MCS, 1),
    /// Search (see #acc_mcs_search_req_ind_t)
    ACC_MCS_SEARCH = GAF_CODE(ACC, MCS, 2),
    /// Get (see #acc_mcs_get_req_ind_t)
    ACC_MCS_GET = GAF_CODE(ACC, MCS, 3),
    /// Get Position (see #acc_mcs_get_position_req_ind_t)
    ACC_MCS_GET_POSITION = GAF_CODE(ACC, MCS, 4),
    /// Set (see #acc_mcs_set_req_ind_t)
    ACC_MCS_SET_RI = GAF_CODE(ACC, MCS, 5),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ACC_MCS_CONFIGURE request message
typedef struct acc_mcs_configure_req
{
    /// Request code (shall be set to #ACC_MCS_CONFIGURE)
    uint16_t req_code;
    /// Number of instances of the (Generic) Media Control Service
    /// Value 0 is prohibited
    uint8_t nb_mcs;
    /// Preferred MTU
    /// Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
} acc_mcs_configure_req_t;

/// Structure for #ACC_MCS_ADD request message
typedef struct acc_mcs_add_req
{
    /// Request code (shall be set to #ACC_MCS_ADD)
    uint16_t req_code;
    /// Configuration bit field
    uint32_t cfg_bf;
    /// Supported playing order bit field
    uint32_t playing_order_supp_bf;
    /// Start handle of associated Object Transfer Service
    uint16_t ots_shdl;
    /// Content Control ID
    uint8_t ccid;
    /// Required start handle
    /// If set to GATT_INVALID_HDL, the start handle will be automatically chosen
    uint16_t shdl;
    /// Media Player Icon Object ID
    acc_mc_object_id_t icon_obj_id;
} acc_mcs_add_req_t;

/// Structure for #ACC_MCS_RESTORE_BOND_DATA request message
typedef struct acc_mcs_restore_bond_data_req
{
    /// Request code (shall be set to #ACC_MCS_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint32_t cli_cfg_bf;
    /// Event configuration bit field
    uint32_t evt_cfg_bf;
    /// Length of Media Player Name value
    /// Meaningful only if notification of Media Player Name characteristic is supported
    uint8_t name_len;
    /// Length of Track Title value
    /// Meaningful only if notification of Track Title characteristic is supported
    uint8_t title_len;
    /// Media Player Name value followed by Track Title value
    uint8_t val[__ARRAY_EMPTY];
} acc_mcs_restore_bond_data_req_t;

/// Structure for #ACC_MCS_SET request message
typedef struct acc_mcs_set_req
{
    /// Request code (shall be set to #ACC_MCS_SET)
    uint16_t req_code;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
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
        uint8_t play_order;
        /// Supported Media Control Operation Codes bit field
        uint32_t opcodes_supp_bf;
    } val;
} acc_mcs_set_req_t;

/// Structure for #ACC_MCS_SET_OBJECT_ID request message
typedef struct acc_mcs_set_object_id_req
{
    /// Request code (shall be set to #ACC_MCS_SET_OBJECT_ID)
    uint16_t req_code;
    /// Media local index
    uint8_t media_lid;
    /// Object ID
    acc_mc_object_id_t obj_id;
} acc_mcs_set_object_id_req_t;

/// Structure for #ACC_MCS_SET_PLAYER_NAME request message
typedef struct acc_mcs_set_player_name_req
{
    /// Request code (shall be set to #ACC_MCS_SET_PLAYER_NAME)
    uint16_t req_code;
    /// Media local index
    uint8_t media_lid;
    /// Length of Media Player Name value
    uint8_t name_len;
    /// Media Player Name value
    uint8_t name[__ARRAY_EMPTY];
} acc_mcs_set_player_name_req_t;

/// Structure for #ACC_MCS_ACTION request message
typedef struct acc_mcs_action_req
{
    /// Request code (shall be set to #ACC_MCS_ACTION)
    uint16_t req_code;
    /// Media local index
    uint8_t media_lid;
    /// Action
    uint8_t action;
    /// Track Position
    int32_t track_pos;
    /// Seeking speed
    int8_t seeking_speed;
} acc_mcs_action_req_t;


/// Structure for #ACC_MCS_TRACK_CHANGE request message
typedef struct acc_mcs_track_change_req
{
    /// Request code (shall be set to #ACC_MCS_TRACK_CHANGE)
    uint16_t req_code;
    /// Media local index
    uint8_t media_lid;
    /// Track duration in multiple of 0.01 seconds
    /// Shall be higher than 0
    int32_t track_dur;
    /// Current Track Segments Object ID
    acc_mc_object_id_t segments_obj_id;
    /// Current Track Object ID
    acc_mc_object_id_t current_obj_id;
    /// Next Track Object ID
    acc_mc_object_id_t next_obj_id;
    /// Current Group Object ID
    acc_mc_object_id_t group_obj_id;
    /// Parent Group Object ID
    acc_mc_object_id_t parent_obj_id;
    /// Length of Track Title value
    uint8_t title_len;
    /// Track title value
    uint8_t title[__ARRAY_EMPTY];
} acc_mcs_track_change_req_t;

/// Structure for response message
typedef struct acc_mcs_rsp
{
    /// Request code (see #acc_mcs_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Media local index
    uint8_t media_lid;
    /// Union
    union
    {
        /// Additional parameter
        uint8_t param;
        /// Connection local index
        uint8_t con_lid;
        /// Characteristic type
        uint8_t char_type;
        /// Action
        uint8_t action;
    } param;
} acc_mcs_rsp_t;

/// Structure for #ACC_MCS_BOND_DATA indication message
typedef struct acc_mcs_bond_data_ind
{
    /// Indication code (shall be set to #ACC_MCS_BOND_DATA)
    uint16_t ind_code;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint32_t cli_cfg_bf;
} acc_mcs_bond_data_ind_t;

/// Structure for #ACC_MCS_SET_OBJECT_ID request indication message
typedef struct acc_mcs_set_object_id_req_ind
{
    /// Request Indication code (shall be set to #ACC_MCS_SET_OBJECT_ID)
    uint16_t req_ind_code;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Object ID
    acc_mc_object_id_t obj_id;
} acc_mcs_set_object_id_req_ind_t;

/// Structure for #ACC_MCS_CONTROL request indication message
typedef struct acc_mcs_control_req_ind
{
    /// Request Indication code (shall be set to #ACC_MCS_CONTROL)
    uint16_t req_ind_code;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Operation code
    uint8_t opcode;
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
    } val;
} acc_mcs_control_req_ind_t;

/// Structure for #ACC_MCS_SEARCH request indication message
typedef struct acc_mcs_search_req_ind
{
    /// Request Indication code (shall be set to #ACC_MCS_SEARCH)
    uint16_t req_ind_code;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Length of Search Parameters value
    uint8_t param_len;
    /// Search Parameters value
    uint8_t param[__ARRAY_EMPTY];
} acc_mcs_search_req_ind_t;

/// Structure for #ACC_MCS_GET request indication message
typedef struct acc_mcs_get_req_ind
{
    /// Request Indication code (shall be set to #ACC_MCS_GET)
    uint16_t req_ind_code;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Token
    uint16_t token;
    /// Offset
    uint16_t offset;
    /// Maximum length
    uint16_t length;
} acc_mcs_get_req_ind_t;

/// Structure for #ACC_MCS_GET_POSITION request indication message
typedef struct acc_mcs_get_position_req_ind
{
    /// Request Indication code (shall be set to #ACC_MCS_GET_POSITION)
    uint16_t req_ind_code;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Token
    uint16_t token;
} acc_mcs_get_position_req_ind_t;

/// Structure for #ACC_MCS_SET request indication message
typedef struct acc_mcs_set_req_ind
{
    /// Request Indication code (shall be set to #ACC_MCS_SET)
    uint16_t req_ind_code;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Union
    union
    {
        /// Additional parameter
        uint32_t param;
        /// Track position offset
        int32_t track_pos_offset;
        /// Playback speed
        int8_t playback_speed;
        /// Playing order
        uint8_t playing_order;
    } param;
} acc_mcs_set_req_ind_t;

/// Structure for #ACC_MCS_SET_OBJECT_ID confirmation message
typedef struct acc_mcs_set_object_id_cfm
{
    /// Request Indication code (shall be set to #ACC_MCS_SET_OBJECT_ID)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Media local index
    uint8_t media_lid;
} acc_mcs_set_object_id_cfm_t;

/// Structure for #ACC_MCS_CONTROL confirmation message
typedef struct acc_mcs_control_cfm
{
    /// Request Indication code (shall be set to #ACC_MCS_CONTROL)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Media local index
    uint8_t media_lid;
    /// Result
    uint8_t result;
    /// Action
    uint8_t action;
    /// Track Position
    int32_t track_pos;
    /// Seeking speed
    int8_t seeking_speed;
} acc_mcs_control_cfm_t;

/// Structure for #ACC_MCS_SEARCH confirmation message
typedef struct acc_mcs_search_cfm
{
    /// Request Indication code (shall be set to #ACC_MCS_SEARCH)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Media local index
    uint8_t media_lid;
    /// Object ID
    acc_mc_object_id_t obj_id;
} acc_mcs_search_cfm_t;

/// Structure for #ACC_MCS_GET confirmation message
typedef struct acc_mcs_get_cfm
{
    /// Request Indication code (shall be set to #ACC_MCS_GET)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Token
    uint16_t token;
    /// Length
    uint16_t length;
    /// Requested value
    uint8_t val[__ARRAY_EMPTY];
} acc_mcs_get_cfm_t;

/// Structure for #ACC_MCS_GET_POSITION confirmation message
typedef struct acc_mcs_get_position_cfm
{
    /// Request Indication code (shall be set to #ACC_MCS_GET_POSITION)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Token
    uint16_t token;
    /// Track position
    int32_t track_pos;
} acc_mcs_get_position_cfm_t;

/// Structure for #ACC_MCS_SET confirmation message
typedef struct acc_mcs_set_cfm
{
    /// Request Indication code (shall be set to #ACC_MCS_SET)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Media local index
    uint8_t media_lid;
    /// Union
    union
    {
        /// Value
        uint32_t val;
        /// Track position
        int32_t track_pos;
        /// Playback speed
        int8_t playback_speed;
    } val;
} acc_mcs_set_cfm_t;

/// @} ACC_MCS_MSG

#endif // ACC_MCS_MSG_H_
