/**
 ****************************************************************************************
 *
 * @file arc_vcc_msg.h
 *
 * @brief Audio Rendering Control - Definition of Kernel Messages (Volume Control Client)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_VCC_MSG_H_
#define ARC_VCC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_VCC_MSG Message API
 * @ingroup ARC_VCC
 * @brief Description of Message API for Volume Control Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc_msg.h"        // Audio Rendering Control Kernel Messages Definitions
#include "arc_vcc.h"        // Audio Rendering Control - Volume Control Client Definitions

/// @addtogroup ARC_VCC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Volume Control Service Client module
enum arc_vcc_msg_cmd_code
{
    /// Discover (see #arc_vcc_discover_cmd_t)
    ARC_VCC_DISCOVER = GAF_CODE(ARC, VCC, ARC_VCC_CMD_TYPE_DISCOVER),
    /// Control (see #arc_vcc_control_cmd_t)
    ARC_VCC_CONTROL = GAF_CODE(ARC, VCC, ARC_VCC_CMD_TYPE_CONTROL),
    /// Get (see #arc_vcc_get_cmd_t)
    ARC_VCC_GET = GAF_CODE(ARC, VCC, ARC_VCC_CMD_TYPE_GET),
    /// Set Configuration (see #arc_vcc_set_cfg_cmd_t)
    ARC_VCC_SET_CFG = GAF_CODE(ARC, VCC, ARC_VCC_CMD_TYPE_SET_CFG),
};

/// List of GAF_REQ request code values for Volume Control Service Client module
enum arc_vcc_msg_req_code
{
    /// Configure (see #arc_vcc_configure_req_t)
    ARC_VCC_CONFIGURE = GAF_CODE(ARC, VCC, 0),
    /// Restore Bond Data (see #arc_vcc_restore_bond_data_req_t)
    ARC_VCC_RESTORE_BOND_DATA = GAF_CODE(ARC, VCC, 1),
};

/// List of GAF_IND indication code values for Volume Control Service Client module
enum arc_vcc_msg_ind_code
{
    /// Bond Data (see #arc_vcc_bond_data_ind_t)
    ARC_VCC_BOND_DATA = GAF_CODE(ARC, VCC, 0),
    /// Found included service (see #arc_vcc_included_svc_ind_t)
    ARC_VCC_INCLUDED_SVC = GAF_CODE(ARC, VCC, 1),
    /// Volume (see #arc_vcc_volume_ind_t)
    ARC_VCC_VOLUME = GAF_CODE(ARC, VCC, 2),
    /// Flags (see #arc_vcc_flags_ind_t)
    ARC_VCC_FLAGS = GAF_CODE(ARC, VCC, 3),
    /// Service Changed (see #arc_vcc_svc_changed_ind_t)
    ARC_VCC_SVC_CHANGED = GAF_CODE(ARC, VCC, 5),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ARC_VCC_DISCOVER command message
typedef struct arc_vcc_discover_cmd
{
    /// Command code (shall be set to #ARC_VCC_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Start handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t shdl;
    /// End handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t ehdl;
} arc_vcc_discover_cmd_t;

/// Structure for #ARC_VCC_CONTROL command message
typedef struct arc_vcc_control_cmd
{
    /// Command code (shall be set to #ARC_VCC_CONTROL)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Operation code
    uint8_t opcode;
    /// Volume
    uint8_t volume;
} arc_vcc_control_cmd_t;

/// Structure for #ARC_VCC_GET command message
typedef struct arc_vcc_get_cmd
{
    /// Command code (shall be set to #ARC_VCC_GET)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type
    uint8_t char_type;
} arc_vcc_get_cmd_t;

/// Structure for #ARC_VCC_SET_CFG command message
typedef struct arc_vcc_set_cfg_cmd
{
    /// Command code (shall be set to #ARC_VCC_SET_CFG)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Enable
    uint8_t enable;
} arc_vcc_set_cfg_cmd_t;

/// Structure for command complete event message
typedef struct arc_vcc_cmp_evt
{
    /// Command code (see #arc_vcc_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
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
} arc_vcc_cmp_evt_t;

/// Structure for #ARC_VCC_CONFIGURE request message
typedef struct arc_vcc_configure_req
{
    /// Request code (shall be set to #ARC_VCC_CONFIGURE)
    uint16_t req_code;
} arc_vcc_configure_req_t;

/// Structure for #ARC_VCC_RESTORE_BOND_DATA request message
typedef struct arc_vcc_restore_bond_data_req
{
    /// Request code (shall be set to #ARC_VCC_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Volume Control Service description
    arc_vcc_vcs_t vcs_info;
} arc_vcc_restore_bond_data_req_t;

/// Structure for response message
typedef struct arc_vcc_rsp
{
    /// Request code (see #arc_vcc_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} arc_vcc_rsp_t;

/// Structure for #ARC_VCC_BOND_DATA indication message
typedef struct arc_vcc_bond_data_ind
{
    /// Indication code (shall be set to #ARC_VCC_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Volume Control Service description
    arc_vcc_vcs_t vcs_info;
} arc_vcc_bond_data_ind_t;

/// Structure for #ARC_VCC_INCLUDED_SVC indication message
typedef struct arc_vcc_included_svc_ind
{
    /// Indication code (shall be set to #ARC_VCC_INCLUDED_SVC)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// UUID
    uint16_t uuid;
    /// Start handle
    uint16_t shdl;
    /// End handle
    uint16_t ehdl;
} arc_vcc_included_svc_ind_t;

/// Structure for #ARC_VCC_VOLUME indication message
typedef struct arc_vcc_volume_ind
{
    /// Indication code (shall be set to #ARC_VCC_VOLUME)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Volume
    uint8_t volume;
    /// Mute
    uint8_t mute;
} arc_vcc_volume_ind_t;

/// Structure for #ARC_VCC_FLAGS indication message
typedef struct arc_vcc_flags_ind
{
    /// Indication code (shall be set to #ARC_VCC_FLAGS)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Volume Flags
    uint8_t flags;
} arc_vcc_flags_ind_t;

/// Structure for #ARC_VCC_SVC_CHANGED indication message
typedef struct arc_vcc_svc_changed_ind
{
    /// Indication code (shall be set to #ARC_VCC_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} arc_vcc_svc_changed_ind_t;

/// @} ARC_VCC_MSG

#endif // ARC_VCC_MSG_H_
