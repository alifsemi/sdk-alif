/**
 ****************************************************************************************
 *
 * @file arc_aics_msg.h
 *
 * @brief Audio Rendering Control - Definition of Kernel Messages (Audio Input Control Server)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_AICS_MSG_H_
#define ARC_AICS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_AICS_MSG Message API
 * @ingroup ARC_AICS
 * @brief Description of Message API for Audio Input Control Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"            // GAF Defines
#include "arc_msg.h"        // Audio Rendering Control Kernel Messages Definitions
#include "arc_aics.h"       // Audio Rendering Control - Audio Input Control Server Definitions

/// @addtogroup ARC_AICS_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request code values for Audio Input Control Service Server module
enum arc_aics_msg_req_code
{
    /// Configure (see #arc_aics_configure_req_t)
    ARC_AICS_CONFIGURE = GAF_CODE(ARC, AICS, 0),
    /// Add (see #arc_aics_add_req_t)
    ARC_AICS_ADD = GAF_CODE(ARC, AICS, 1),
    /// Restore Bond Data (see #arc_aics_restore_bond_data_req_t)
    ARC_AICS_RESTORE_BOND_DATA = GAF_CODE(ARC, AICS, 2),
    /// Set (see #arc_aics_set_req_t)
    ARC_AICS_SET = GAF_CODE(ARC, AICS, 3),
    /// Set Description (see #arc_aics_set_description_req_t)
    ARC_AICS_SET_DESCRIPTION = GAF_CODE(ARC, AICS, 4),
};

/// List of GAF_IND indication code values for Audio Input Control Service Server module
enum arc_aics_msg_ind_code
{
    /// State (see #arc_aics_state_ind_t)
    ARC_AICS_STATE = GAF_CODE(ARC, AICS, 0),
    /// Bond Data (see #arc_aics_bond_data_ind_t)
    ARC_AICS_BOND_DATA = GAF_CODE(ARC, AICS, 1),
};

/// List of GAF_REQ_IND indication code values for Audio Input Control Service Server module
enum arc_aics_msg_req_ind_code
{
    /// Set Description (see #arc_aics_set_description_req_ind_t)
    ARC_AICS_SET_DESCRIPTION_RI = GAF_CODE(ARC, AICS, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ARC_AICS_ADD request message
typedef struct arc_aics_add_req
{
    /// Request code (shall be set to #ARC_AICS_ADD)
    uint16_t req_code;
    /// Maximum length of Audio Input Description
    uint8_t desc_max_len;
    /// Gain Units
    uint8_t gain_units;
    /// Gain Minimum
    int8_t gain_min;
    /// Gain Maximum
    int8_t gain_max;
    /// Input Type
    uint8_t input_type;
    /// Service configuration bit field (see #arc_aics_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Required start handle
    /// If set to GATT_INVALID_HDL, the start handle will be automatically chosen
    uint16_t shdl;
} arc_aics_add_req_t;

/// Structure for #ARC_AICS_RESTORE_BOND_DATA request message
typedef struct arc_aics_restore_bond_data_req
{
    /// Request code (shall be set to #ARC_AICS_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Input local index
    uint8_t input_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
    /// Event configuration bit field
    uint8_t evt_cfg_bf;
} arc_aics_restore_bond_data_req_t;

/// Structure for #ARC_AICS_CONFIGURE request message
typedef struct arc_aics_configure_req
{
    /// Request code (shall be set to #ARC_AICS_CONFIGURE)
    uint16_t req_code;
    /// Number of inputs
    uint8_t nb_inputs;
    /// Preferred MTU
    /// Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
} arc_aics_configure_req_t;

/// Structure for #ARC_AICS_SET request message
typedef struct arc_aics_set_req
{
    /// Request code (shall be set to #ARC_AICS_SET)
    uint16_t req_code;
    /// Input local index
    uint8_t input_lid;
    /// Set type
    uint8_t set_type;
    /// Value
    uint32_t value;
} arc_aics_set_req_t;

/// Structure for #ARC_AICS_SET_DESCRIPTION request message
typedef struct arc_aics_set_description_req
{
    /// Request code (shall be set to #ARC_AICS_SET_DESCRIPTION)
    uint16_t req_code;
    /// Input local index
    uint8_t input_lid;
    /// Value length
    uint8_t desc_len;
    /// Value
    uint8_t desc[__ARRAY_EMPTY];
} arc_aics_set_description_req_t;

/// Structure for request message
typedef struct arc_aics_rsp
{
    /// Request code (see #arc_aics_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Input local index
    uint8_t input_lid;
    /// Union
    union
    {
        /// Value
        uint8_t value;
        /// Connection local index
        uint8_t con_lid;
        /// Set type
        uint8_t set_type;
    } u;
} arc_aics_rsp_t;

/// Structure for #ARC_AICS_STATE indication message
typedef struct arc_aics_state_ind
{
    /// Indication code (shall be set to #ARC_AICS_STATE)
    ///  - ARC_AICS_STATE
    uint16_t ind_code;
    /// Input local index
    uint8_t input_lid;
    /// Gain
    int8_t gain;
    /// Gain Mode
    uint8_t gain_mode;
    /// Mute
    uint8_t mute;
} arc_aics_state_ind_t;

/// Structure for #ARC_AICS_BOND_DATA indication message
typedef struct arc_aics_bond_data_ind
{
    /// Indication code (shall be set to #ARC_AICS_BOND_DATA)
    ///  - ARC_AICS_BOND_DATA
    uint16_t ind_code;
    /// Input local index
    uint8_t input_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
} arc_aics_bond_data_ind_t;

/// Structure for #ARC_AICS_SET_DESCRIPTION request indication message
typedef struct arc_aics_set_description_req_ind
{
    /// Request Indication code (shall be set to #ARC_AICS_SET_DESCRIPTION)
    ///  - ARC_AICS_SET_DESCRIPTION_RI
    uint16_t req_ind_code;
    /// Input local index
    uint8_t input_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Value length
    uint8_t desc_len;
    /// Value
    uint8_t desc[__ARRAY_EMPTY];
} arc_aics_set_description_req_ind_t;

/// Structure for #ARC_AICS_SET_DESCRIPTION confirmation message
typedef struct arc_aics_set_description_cfm
{
    /// Request Indication code (shall be set to #ARC_AICS_SET_DESCRIPTION_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Input local index
    uint8_t input_lid;
    /// Value length
    uint8_t desc_len;
    /// Value
    uint8_t desc[__ARRAY_EMPTY];
} arc_aics_set_description_cfm_t;

/// @} ARC_AICS_MSG

#endif // ARC_AICS_MSG_H_
