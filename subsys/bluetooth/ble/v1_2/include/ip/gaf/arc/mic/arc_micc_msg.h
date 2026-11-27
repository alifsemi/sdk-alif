/**
 ****************************************************************************************
 *
 * @file arc_micc_msg.h
 *
 * @brief Audio Rendering Control - Definition of Kernel Messages (Microphone Control Client)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_MICC_MSG_H_
#define ARC_MICC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_MICC_MSG Message API
 * @ingroup ARC_MICC
 * @brief Description of Message API for Microphone Control Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc_msg.h"        // Audio Rendering Control Kernel Messages Definitions
#include "arc_micc.h"       // Audio Rendering Control - Microphone Control Client Definitions

/// @addtogroup ARC_MICC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Microphone Control Service Client module
enum arc_micc_msg_cmd_code
{
    /// Discover (see #arc_micc_discover_cmd_t)
    ARC_MICC_DISCOVER = GAF_CODE(ARC, MICC, ARC_MICC_CMD_TYPE_DISCOVER),
    /// Get Mute (see #arc_micc_get_mute_cmd_t)
    ARC_MICC_GET_MUTE = GAF_CODE(ARC, MICC, ARC_MICC_CMD_TYPE_GET_MUTE),
    /// Set Mute (see #arc_micc_set_mute_cmd_t)
    ARC_MICC_SET_MUTE = GAF_CODE(ARC, MICC, ARC_MICC_CMD_TYPE_SET_MUTE),
    /// Set Configuration (see #arc_micc_set_cfg_cmd_t)
    ARC_MICC_SET_CFG = GAF_CODE(ARC, MICC, ARC_MICC_CMD_TYPE_SET_CFG),
};

/// List of GAF_REQ request code values for Microphone Control Service Client module
enum arc_micc_msg_req_code
{
    /// Configure (see #arc_micc_configure_req_t)
    ARC_MICC_CONFIGURE = GAF_CODE(ARC, MICC, 0),
    /// Restore Bond Data (see #arc_micc_restore_bond_data_req_t)
    ARC_MICC_RESTORE_BOND_DATA = GAF_CODE(ARC, MICC, 1),
};

/// List of GAF_IND indication code values for Microphone Control Service Client module
enum arc_micc_msg_ind_code
{
    /// Bond Data (see #arc_micc_bond_data_ind_t)
    ARC_MICC_BOND_DATA = GAF_CODE(ARC, MICC, 0),
    /// Included Service (see #arc_micc_included_svc_ind_t)
    ARC_MICC_INCLUDED_SVC = GAF_CODE(ARC, MICC, 1),
    /// Mute (see #arc_micc_mute_ind_t)
    ARC_MICC_MUTE = GAF_CODE(ARC, MICC, 2),
    /// Service Changed (see #arc_micc_svc_changed_ind_t)
    ARC_MICC_SVC_CHANGED = GAF_CODE(ARC, MICC, 4),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ARC_MICC_DISCOVER request message
typedef struct arc_micc_discover_cmd
{
    /// Command code (shall be set to #ARC_MICC_DISCOVER)
    ///  - ARC_MICC_DISCOVER
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Start handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t shdl;
    /// End handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t ehdl;
} arc_micc_discover_cmd_t;

/// Structure for #ARC_MICC_SET_MUTE command message
typedef struct arc_micc_set_mute_cmd
{
    /// Command code (shall be set to #ARC_MICC_SET_MUTE)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Mute
    uint8_t mute;
} arc_micc_set_mute_cmd_t;

/// Structure for #ARC_MICC_GET_MUTE command message
typedef struct arc_micc_get_mute_cmd
{
    /// Command code (shall be set to #ARC_MICC_GET_MUTE)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
} arc_micc_get_mute_cmd_t;

/// Structure for #ARC_MICC_SET_CFG command message
typedef struct arc_micc_set_cfg_cmd
{
    /// Command code (shall be set to #ARC_MICC_SET_CFG)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Indicate if sending of notifications must be enabled or not
    uint8_t enable;
} arc_micc_set_cfg_cmd_t;

/// Structure for command complete event message for ARC_MICC_DISCOVER, ARC_MICC_SET_MUTE, ARC_MICC_GET_MUTE,
/// ARC_MICC_SET_CFG commands
typedef struct arc_micc_cmp_evt
{
    /// Command code (see #arc_micc_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} arc_micc_cmp_evt_t;

/// Structure for #ARC_MICC_CONFIGURE request message
typedef struct arc_micc_configure_req
{
    /// Request code (shall be set to #ARC_MICC_CONFIGURE)
    uint16_t req_code;
} arc_micc_configure_req_t;

/// Structure for #ARC_MICC_RESTORE_BOND_DATA request message
typedef struct arc_micc_restore_bond_data_req
{
    /// Request code (shall be set to #ARC_MICC_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Microphone Control Service description
    arc_micc_mics_t mics_info;
} arc_micc_restore_bond_data_req_t;

/// Structure for response message
typedef struct arc_micc_rsp
{
    /// Request code (see #arc_micc_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} arc_micc_rsp_t;

/// Structure for #ARC_MICC_BOND_DATA indication message
typedef struct arc_micc_bond_data_ind
{
    /// Indication code (shall be set to #ARC_MICC_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Description of found Microphone Control Service instance
    arc_micc_mics_t mics_info;
} arc_micc_bond_data_ind_t;

/// Structure for #ARC_MICC_INCLUDED_SVC indication message
typedef struct arc_micc_included_svc_ind
{
    /// Indication code (shall be set to #ARC_MICC_INCLUDED_SVC)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Start handle
    uint16_t shdl;
    /// End handle
    uint16_t ehdl;
} arc_micc_included_svc_ind_t;

/// Structure for #ARC_MICC_MUTE indication message
typedef struct arc_micc_mute_ind
{
    /// Indication code (shall be set to #ARC_MICC_MUTE)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Mute
    uint8_t mute;
} arc_micc_mute_ind_t;

/// Structure for #ARC_MICC_SVC_CHANGED indication message
typedef struct arc_micc_svc_changed_ind
{
    /// Indication code (shall be set to #ARC_MICC_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} arc_micc_svc_changed_ind_t;

/// @} ARC_MICC_MSG

#endif // ARC_MICC_MSG_H_
