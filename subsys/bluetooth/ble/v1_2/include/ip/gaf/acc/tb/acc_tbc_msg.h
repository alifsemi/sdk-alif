/**
 ****************************************************************************************
 *
 * @file acc_tbc_msg.h
 *
 * @brief Audio Content Control - Definition of Kernel Messages (Telephone Bearer Client)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_TBC_MSG_H_
#define ACC_TBC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_TBC_MSG Message API
 * @ingroup ACC_TBC
 * @brief Description of Message API for Telephone Bearer Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "acc_msg.h"        // ACC Message Definitions
#include "rwip_task.h"      // RW IP Task Definition
#include "acc_tbc.h"        // Audio Content Control - Telephone Bearer Client Definitions

/// @addtogroup ACC_TBC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Telephone Bearer Service Client module
enum acc_tbc_msg_cmd_code
{
    /// Discover (see #acc_tbc_discover_cmd_t)
    ACC_TBC_DISCOVER = GAF_CODE(ACC, TBC, ACC_TBC_CMD_TYPE_DISCOVER),
    /// Get (see #acc_tbc_get_cmd_t)
    ACC_TBC_GET = GAF_CODE(ACC, TBC, ACC_TBC_CMD_TYPE_GET),
    /// Set Configuration (see #acc_tbc_set_cfg_cmd_t)
    ACC_TBC_SET_CFG = GAF_CODE(ACC, TBC, ACC_TBC_CMD_TYPE_SET_CFG),
    /// Set Report Interval (see #acc_tbc_set_report_intv_cmd_t)
    ACC_TBC_SET_REPORT_INTV = GAF_CODE(ACC, TBC, ACC_TBC_CMD_TYPE_SET_REPORT_INTV),
    /// Create Outgoing Call (see #acc_tbc_call_outgoing_cmd_t)
    ACC_TBC_CALL_OUTGOING = GAF_CODE(ACC, TBC, ACC_TBC_CMD_TYPE_CALL_OUTGOING),
    /// Action for a Call (see #acc_tbc_call_action_cmd_t)
    ACC_TBC_CALL_ACTION = GAF_CODE(ACC, TBC, ACC_TBC_CMD_TYPE_CALL_ACTION),
    /// Join Call (see #acc_tbc_call_join_cmd_t)
    ACC_TBC_CALL_JOIN = GAF_CODE(ACC, TBC, ACC_TBC_CMD_TYPE_CALL_JOIN),
};

/// List of GAF_REQ request code values for Telephone Bearer Service Client module
enum acc_tbc_msg_req_code
{
    /// Configure (see #acc_tbc_configure_req_t)
    ACC_TBC_CONFIGURE = GAF_CODE(ACC, TBC, 0),
    /// Restore Bond Data (see #acc_tbc_restore_bond_data_req_t)
    ACC_TBC_RESTORE_BOND_DATA = GAF_CODE(ACC, TBC, 1),
};

/// List of GAF_IND indication code values for Telephone Bearer Service Client module
enum acc_tbc_msg_ind_code
{
    /// Bond Data (see #acc_tbc_bond_data_ind_t)
    ACC_TBC_BOND_DATA = GAF_CODE(ACC, TBC, 0),
    /// Call State (see #acc_tbc_call_state_ind_t)
    ACC_TBC_CALL_STATE = GAF_CODE(ACC, TBC, 2),
    /// Call State Long (see #acc_tbc_call_state_long_ind_t)
    ACC_TBC_CALL_STATE_LONG = GAF_CODE(ACC, TBC, 3),
    /// Long Value (see #acc_tbc_value_long_ind_t)
    ACC_TBC_VALUE_LONG = GAF_CODE(ACC, TBC, 4),
    /// Value (see #acc_tbc_value_ind_t)
    ACC_TBC_VALUE = GAF_CODE(ACC, TBC, 5),
    /// Service Changed (see #acc_tbc_svc_changed_ind_t)
    ACC_TBC_SVC_CHANGED = GAF_CODE(ACC, TBC, 6),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ACC_TBC_DISCOVER command message
typedef struct acc_tbc_discover_cmd
{
    /// Command code (shall be set to #ACC_TBC_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum number of Telephone Bearer Service instance that can be found
    uint8_t nb_tbs_max;
    /// Bit field indicating characteristics (based on #acc_tb_char_type) that shall be ignored (value not read
    /// if applicable, sending of notifications not enabled if application)\n
    /// Use of following characteristic will be forced:
    ///     - Content Control ID characteristic
    ///     - Call State characteristic
    ///     - Call Control Point characteristic
    uint16_t ignored_char_bf;
    /// Start handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t shdl;
    /// End handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t ehdl;
} acc_tbc_discover_cmd_t;

/// Structure for #ACC_TBC_GET command message
typedef struct acc_tbc_get_cmd
{
    /// Command code (shall be set to #ACC_TBC_GET)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Characteristic type
    uint8_t char_type;
} acc_tbc_get_cmd_t;

/// Structure for #ACC_TBC_SET_CFG command message
typedef struct acc_tbc_set_cfg_cmd
{
    /// Command code (shall be set to #ACC_TBC_SET_CFG)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Indicate if sending of notifications must be enabled (!=0) or disabled
    uint8_t enable;
} acc_tbc_set_cfg_cmd_t;

/// Structure for #ACC_TBC_SET_REPORT_INTV command message
typedef struct acc_tbc_set_report_intv_cmd
{
    /// Command code (shall be set to #ACC_TBC_SET_REPORT_INTV)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Indicate if feedback from Server device is required (!= 0) or not
    uint8_t reliable;
    /// Signal Strength Reporting Interval in seconds
    uint8_t sign_strength_intv_s;
} acc_tbc_set_report_intv_cmd_t;

/// Structure for #ACC_TBC_CALL_OUTGOING command message
typedef struct acc_tbc_call_outgoing_cmd
{
    /// Command code (shall be set to #ACC_TBC_CALL_OUTGOING)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Indicate if feedback from Server device is required (!= 0) or not
    uint8_t reliable;
    /// Length of Outgoing URI value
    uint8_t uri_len;
    /// Outgoing URI value
    uint8_t uri[__ARRAY_EMPTY];
} acc_tbc_call_outgoing_cmd_t;

/// Structure for #ACC_TBC_CALL_ACTION command message
typedef struct acc_tbc_call_action_cmd
{
    /// Command code (shall be set to #ACC_TBC_CALL_ACTION)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Indicate if feedback from Server device is required (!= 0) or not
    uint8_t reliable;
    /// Call index
    uint8_t call_id;
    /// Operation code
    uint8_t opcode;
} acc_tbc_call_action_cmd_t;

/// Structure for #ACC_TBC_CALL_JOIN command message
typedef struct acc_tbc_call_join_cmd
{
    /// Command code (shall be set to #ACC_TBC_CALL_JOIN)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Indicate if feedback from Server device is required (!= 0) or not
    uint8_t reliable;
    /// Number of joined calls
    uint8_t nb_calls;
    /// Call index of calls to join
    uint8_t call_ids[__ARRAY_EMPTY];
} acc_tbc_call_join_cmd_t;

/// Structure for command complete event message
typedef struct acc_tbc_cmp_evt
{
    /// Command code (see #acc_tbc_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Union
    union
    {
        /// Value
        uint8_t val;
        /// Characteristic type
        uint8_t char_type;
        /// Operation code
        uint8_t opcode;
    } u;
    /// Call index
    uint8_t call_id;
    /// Result
    uint8_t result;
} acc_tbc_cmp_evt_t;

/// Structure for #ACC_TBC_CONFIGURE request message
typedef struct acc_tbc_configure_req
{
    /// Request code (shall be set to #ACC_TBC_CONFIGURE)
    uint16_t req_code;
    /// Preferred MTU
    uint16_t pref_mtu;
} acc_tbc_configure_req_t;

/// Structure for #ACC_TBC_RESTORE_BOND_DATA request message
typedef struct acc_tbc_restore_bond_data_req
{
    /// Request code (shall be set to #ACC_TBC_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Number of instances of the (Generic) Telephone Bearer Service discovered
    uint8_t nb_bearers;
    /// Content description of (Generic) Telephone Bearer Service instances
    /// Generic Telephone Bearer Service is first service
    acc_tbc_tbs_info_t tbs_info[__ARRAY_EMPTY];
} acc_tbc_restore_bond_data_req_t;

/// Structure for response message
typedef struct acc_tbc_rsp
{
    /// Request code (see #acc_tbc_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} acc_tbc_rsp_t;

/// Structure for #ACC_TBC_BOND_DATA indication message
typedef struct acc_tbc_bond_data_ind
{
    /// Indication code (shall be set to #ACC_TBC_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// UUID
    uint16_t uuid;
    /// Content description of (Generic) Telephone Bearer Service instance
    acc_tbc_tbs_info_t tbs_info;
} acc_tbc_bond_data_ind_t;

/// Structure for #ACC_TBC_CALL_STATE indication message
typedef struct acc_tbc_call_state_ind
{
    /// Indication code (shall be set to #ACC_TBC_CALL_STATE)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    uint8_t id;
    /// Call flags
    uint8_t flags;
    /// Call state
    uint8_t state;
} acc_tbc_call_state_ind_t;

/// Structure for #ACC_TBC_CALL_STATE_LONG indication message
typedef struct acc_tbc_call_state_long_ind
{
    /// Indication code (shall be set to #ACC_TBC_CALL_STATE_LONG)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    uint8_t id;
    /// Call flags
    uint8_t flags;
    /// Call state
    uint8_t state;
    /// Length of Incoming or Outgoing Call URI value
    uint8_t uri_len;
    /// Incoming or Outgoing Call URI value
    uint8_t uri[__ARRAY_EMPTY];
} acc_tbc_call_state_long_ind_t;

/// Structure for #ACC_TBC_VALUE_LONG indication message
typedef struct acc_tbc_value_long_ind
{
    /// Indication code (shall be set to #ACC_TBC_VALUE_LONG)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    uint8_t call_id;
    /// Characteristic type
    uint8_t char_type;
    /// Length of value
    uint16_t val_len;
    /// Value
    uint8_t val[__ARRAY_EMPTY];
} acc_tbc_value_long_ind_t;

/// Structure for #ACC_TBC_VALUE indication message
typedef struct acc_tbc_value_ind
{
    /// Indication code (shall be set to #ACC_TBC_VALUE)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    /// Meaningful only for Termination Reason characteristic
    uint8_t call_id;
    /// Characteristic type
    uint8_t char_type;
    /// Value
    union
    {
        /// Value
        uint16_t val;
        /// Bearer Technology
        uint8_t techno;
        /// Signal Strength
        uint8_t sign_strength;
        /// Signal Strength Reporting Interval in seconds
        uint8_t sign_strength_intv_s;
        /// Content Control ID
        uint8_t ccid;
        /// Status Flags bit field
        uint16_t status_flags_bf;
        /// Call Control Point Optional Opcodes bit field
        uint16_t opt_opcodes_bf;
        /// Termination Reason
        uint8_t term_reason;
    } val;
} acc_tbc_value_ind_t;

/// Structure for #ACC_TBC_SVC_CHANGED indication message
typedef struct acc_tbc_svc_changed_ind
{
    /// Indication code (shall be set to #ACC_TBC_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} acc_tbc_svc_changed_ind_t;

/// @} ACC_TBC_MSG

#endif // ACC_TBC_MSG_H_
