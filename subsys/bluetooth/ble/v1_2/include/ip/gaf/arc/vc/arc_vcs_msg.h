/**
 ****************************************************************************************
 *
 * @file arc_vcs_msg.h
 *
 * @brief Audio Rendering Control - Volume Control Server - Message API Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_VCS_MSG_H_
#define ARC_VCS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_VCS_MSG Message API
 * @ingroup ARC_VCS
 * @brief Description of Message API for Volume Control Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"            // GAF Defines
#include "arc_msg.h"        // Message API Definitions
#include "arc_vcs.h"        // Volume Control Server Definitions

/// @addtogroup ARC_VCS_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request code values for Volume Control Service Server module
enum arc_vcs_msg_req_code
{
    /// Configure (see #arc_vcs_configure_req_t)
    ARC_VCS_CONFIGURE = GAF_CODE(ARC, VCS, 0),
    /// Restore Bond Data (see #arc_vcs_restore_bond_data_req_t)
    ARC_VCS_RESTORE_BOND_DATA = GAF_CODE(ARC, VCS, 1),
    /// Control (see #arc_vcs_control_req_t)
    ARC_VCS_CONTROL = GAF_CODE(ARC, VCS, 2),
};

/// List of GAF_IND indication code values for Volume Control Service Server module
enum arc_vcs_msg_ind_code
{
    /// Volume (see #arc_vcs_volume_ind_t)
    ARC_VCS_VOLUME = GAF_CODE(ARC, VCS, 0),
    /// Bond Data (see #arc_vcs_bond_data_ind_t)
    ARC_VCS_BOND_DATA = GAF_CODE(ARC, VCS, 1),
    /// Flags (see #arc_vcs_flags_ind_t)
    ARC_VCS_FLAGS = GAF_CODE(ARC, VCS, 2),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ARC_VCS_CONFIGURE request message
typedef struct arc_vcs_configure_req
{
    /// Request code (shall be set to #ARC_VCS_CONFIGURE)
    uint16_t req_code;
    /// Step size
    uint8_t step_size;
    /// Volume Flags characteristic value
    uint8_t flags;
    /// Initial Volume Settings value
    uint8_t volume;
    /// Initial Mute value
    uint8_t mute;
    /// Required start handle
    /// If set to GATT_INVALID_HDL the start handle will be automatically chosen
    uint16_t shdl;
    /// Configuration bit field (see #arc_vcs_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Number of Audio Input Control Service instances to include in the service
    uint8_t nb_inputs;
    /// Local index of Audio Input Control Service instances to inclide in the service
    uint8_t input_lid[__ARRAY_EMPTY];
} arc_vcs_configure_req_t;

/// Structure for #ARC_VCS_RESTORE_BOND_DATA request message
typedef struct arc_vcs_restore_bond_data_req
{
    /// Request code (shall be set to #ARC_VCS_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
    /// Event configuration bit field
    uint8_t evt_cfg_bf;
} arc_vcs_restore_bond_data_req_t;

/// Structure for #ARC_VCS_CONTROL request message
typedef struct arc_vcs_control_req
{
    /// Request code (shall be set to #ARC_VCS_CONTROL)
    uint16_t req_code;
    /// Operation code
    uint8_t opcode;
    /// Volume
    uint8_t volume;
} arc_vcs_control_req_t;

/// Structure for request message
typedef struct arc_vcs_rsp
{
    /// Request code (see #arc_vcs_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Union
    union
    {
        /// Value
        uint8_t value;
        /// Connection local index
        uint8_t con_lid;
        /// Operation code
        uint8_t opcode;
    } u;
} arc_vcs_rsp_t;

/// Structure for #ARC_VCS_BOND_DATA indication message
typedef struct arc_vcs_bond_ind
{
    /// Indication code (shall be set to #ARC_VCS_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
} arc_vcs_bond_data_ind_t;

/// Structure for #ARC_VCS_VOLUME indication message
typedef struct arc_vcs_volume_ind
{
    /// Indication code (shall be set to #ARC_VCS_VOLUME)
    uint16_t ind_code;
    /// Volume
    uint8_t volume;
    /// Mute
    uint8_t mute;
    /// Indicate if update has been triggered locally or not
    bool local;
} arc_vcs_volume_ind_t;

/// Structure for #ARC_VCS_FLAGS indication message
typedef struct arc_vcs_flags_ind
{
    /// Indication code (shall be set to #ARC_VCS_FLAGS)
    uint16_t ind_code;
    /// Volume Flags
    uint8_t flags;
} arc_vcs_flags_ind_t;

/// @} ARC_VCS_MSG

#endif // ARC_VCS_MSG_H_
