/**
 ****************************************************************************************
 *
 * @file arc_aicc_msg.h
 *
 * @brief Audio Rendering Control - Definition of Kernel Messages (Audio Input Control Client)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_AICC_MSG_H_
#define ARC_AICC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_AICC_MSG Message API
 * @ingroup ARC_AICC
 * @brief Description of Message API for Audio Input Control Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc_msg.h"        // Audio Rendering Control Kernel Messages Definitions
#include "arc_aicc.h"       // Audio Rendering Control - Audio Input Control Client Definitions

/// @addtogroup ARC_AICC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Audio Input Control Client module
enum arc_aicc_msg_cmd_code
{
    /// Discover (see #arc_aicc_discover_cmd_t)
    ARC_AICC_DISCOVER = GAF_CODE(ARC, AICC, ARC_AICC_CMD_TYPE_DISCOVER),
    /// Get (see #arc_aicc_get_cmd_t)
    ARC_AICC_GET = GAF_CODE(ARC, AICC, ARC_AICC_CMD_TYPE_GET),
    /// Control (see #arc_aicc_control_cmd_t)
    ARC_AICC_CONTROL = GAF_CODE(ARC, AICC, ARC_AICC_CMD_TYPE_CONTROL),
    /// Set Description (see #arc_aicc_set_description_cmd_t)
    ARC_AICC_SET_DESCRIPTION = GAF_CODE(ARC, AICC, ARC_AICC_CMD_TYPE_SET_DESCRIPTION),
    /// Set Configuration (see #arc_aicc_set_cfg_cmd_t)
    ARC_AICC_SET_CFG = GAF_CODE(ARC, AICC, ARC_AICC_CMD_TYPE_SET_CFG),
};

/// List of GAF_REQ request code values for Audio Input Control Client module
enum arc_aicc_msg_req_code
{
    /// Configure (see #arc_aicc_configure_req_t)
    ARC_AICC_CONFIGURE = GAF_CODE(ARC, AICC, 0),
    /// Restore Bond Data (see #arc_aicc_restore_bond_data_req_t)
    ARC_AICC_RESTORE_BOND_DATA = GAF_CODE(ARC, AICC, 1),
};

/// List of GAF_IND indication code values for Audio Input Control Client module
enum arc_aicc_msg_ind_code
{
    /// Bond Data (see #arc_aicc_bond_data_ind_t)
    ARC_AICC_BOND_DATA = GAF_CODE(ARC, AICC, 0),
    /// Gain (see #arc_aicc_gain_ind_t)
    ARC_AICC_GAIN = GAF_CODE(ARC, AICC, 1),
    /// Gain Properties (see #arc_aicc_gain_prop_ind_t)
    ARC_AICC_GAIN_PROP = GAF_CODE(ARC, AICC, 2),
    /// Value (see #arc_aicc_value_ind_t)
    ARC_AICC_VALUE = GAF_CODE(ARC, AICC, 3),
    /// Description (see #arc_aicc_description_ind_t)
    ARC_AICC_DESCRIPTION = GAF_CODE(ARC, AICC, 4),
    /// Service Changed (see #arc_aicc_svc_changed_ind_t)
    ARC_AICC_SVC_CHANGED = GAF_CODE(ARC, AICC, 6),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ARC_AICC_DISCOVER command message
typedef struct arc_aicc_discover_cmd
{
    /// Command code (shall be set to #ARC_AICC_DISCOVER)
    ///  - ARC_AICC_DISCOVER
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Number of inputs
    uint8_t nb_inputs;
    /// Service handles
    prf_svc_t svc_hdl[__ARRAY_EMPTY];
} arc_aicc_discover_cmd_t;

/// Structure for #ARC_AICC_CONTROL command message
typedef struct arc_aicc_control_cmd
{
    /// Command code (shall be set to #ARC_AICC_CONTROL)
    ///  - ARC_AICC_CONTROL
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Input local index
    uint8_t input_lid;
    /// Operation code
    uint8_t opcode;
    /// Gain
    int8_t gain;
} arc_aicc_control_cmd_t;

/// Structure for #ARC_AICC_GET command message
typedef struct arc_aicc_get_cmd
{
    /// Command code (shall be set to #ARC_AICC_GET)
    ///  - ARC_AICC_GET
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Input local index
    uint8_t input_lid;
    /// Characteristic type
    uint8_t char_type;
} arc_aicc_get_cmd_t;

/// Structure for #ARC_AICC_SET_DESCRIPTION command message
typedef struct arc_aicc_set_description_cmd
{
    /// Command code (shall be set to #ARC_AICC_SET_DESCRIPTION)
    ///  - ARC_AICC_SET_DESCRIPTION
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Input local index
    uint8_t input_lid;
    /// Audio input description length
    uint16_t desc_len;
    /// Audio input description
    uint8_t desc[__ARRAY_EMPTY];
} arc_aicc_set_description_cmd_t;

/// Structure for #ARC_AICC_SET_CFG command message
typedef struct arc_aicc_set_cfg_cmd
{
    /// Command code (shall be set to #ARC_AICC_SET_CFG)
    ///  - ARC_AICC_SET_CFG
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Input local index
    uint8_t input_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Enable or disable
    uint8_t enable;
} arc_aicc_set_cfg_cmd_t;

/// Structure for command complete event message
typedef struct arc_aicc_cmp_evt
{
    /// Command code (see #arc_aicc_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Input local index
    uint8_t input_lid;
    /// Union
    union
    {
        /// Value
        uint8_t value;
        /// Operation code
        uint8_t opcode;
        /// Characteristic type
        uint8_t char_type;
    } u;
} arc_aicc_cmp_evt_t;

/// Structure for #ARC_AICC_CONFIGURE request message
typedef struct arc_aicc_configure_req
{
    /// Request code (shall be set to #ARC_AICC_CONFIGURE)
    ///  - ARC_AICC_CONFIGURE
    uint16_t req_code;
    /// Preferred MTU
    /// Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
} arc_aicc_configure_req_t;

/// Structure for #ARC_AICC_RESTORE_BOND_DATA request message
typedef struct arc_aicc_restore_bond_data_req
{
    /// Request code (shall be set to #ARC_AICC_RESTORE_BOND_DATA)
    ///  - ARC_AICC_RESTORE_BOND_DATA
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Number of inputs
    uint8_t nb_inputs;
    /// Description of found Audio Input Control Service instances
    arc_aicc_aics_t aics_info[__ARRAY_EMPTY];
} arc_aicc_restore_bond_data_req_t;

/// Structure for response message
typedef struct arc_aicc_rsp
{
    /// Request code (see #arc_aicc_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} arc_aicc_rsp_t;

/// Structure for #ARC_AICC_BOND_DATA indication message
typedef struct arc_aicc_bond_data_ind
{
    /// Indication code (shall be set to #ARC_AICC_BOND_DATA)
    ///  - ARC_AICC_BOND_DATA
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Input local index
    uint8_t input_lid;
    /// Description of found Audio Input Control Service instance
    arc_aicc_aics_t aics_info;
} arc_aicc_bond_data_ind_t;

/// Structure for #ARC_AICC_GAIN indication message
typedef struct arc_aicc_gain_ind
{
    /// Indication code (shall be set to #ARC_AICC_GAIN)
    ///  - ARC_AICC_GAIN
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Input local index
    uint8_t input_lid;
    /// Gain
    int8_t gain;
    /// Mute
    uint8_t mute;
    /// Gain mode
    uint8_t gain_mode;
} arc_aicc_gain_ind_t;

/// Structure for #ARC_AICC_GAIN_PROP indication message
typedef struct arc_aicc_gain_prop_ind
{
    /// Indication code (shall be set to #ARC_AICC_GAIN_PROP)
    ///  - ARC_AICC_GAIN_PROP
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Input local index
    uint8_t input_lid;
    /// Gain Setting Units
    uint8_t units;
    /// Gain Setting Minimum
    int8_t min;
    /// Gain Setting Maximum
    int8_t max;
} arc_aicc_gain_prop_ind_t;

/// Structure for #ARC_AICC_DESCRIPTION indication message
typedef struct arc_aicc_description_ind
{
    /// Indication code (shall be set to #ARC_AICC_DESCRIPTION)
    ///  - ARC_AICC_DESCRIPTION
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Input local index
    uint8_t input_lid;
    /// Audio input description length
    uint16_t desc_len;
    /// Audio input description
    uint8_t desc[__ARRAY_EMPTY];
} arc_aicc_description_ind_t;

/// Structure for #ARC_AICC_VALUE indication message
typedef struct arc_aicc_value_ind
{
    /// Indication code (shall be set to #ARC_AICC_VALUE)
    ///  - ARC_AICC_VALUE
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Input local index
    uint8_t input_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Value
    uint8_t value;
} arc_aicc_value_ind_t;

/// Structure for #ARC_AICC_SVC_CHANGED indication message
typedef struct arc_aicc_svc_changed_ind
{
    /// Indication code (shall be set to #ARC_AICC_SVC_CHANGED)
    ///  - ARC_AICC_GAIN
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} arc_aicc_svc_changed_ind_t;

/// @} ARC_AICC_MSG

#endif // ARC_AICC_MSG_H_
