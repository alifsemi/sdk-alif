/**
 ****************************************************************************************
 *
 * @file arc_mics_msg.h
 *
 * @brief Audio Rendering Control - Microphone Control Server - Message API Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_MICS_MSG_H_
#define ARC_MICS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_MICS_MSG Message API
 * @ingroup ARC_MICS
 * @brief Description of Message API for Microphone Control Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"            // GAF Defines
#include "arc_msg.h"        // Message API Definitions
#include "arc_mics.h"       // Microphone Control Server Definitions

/// @addtogroup ARC_MICS_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request code values for Microphone Control Service Server module
enum arc_mics_msg_req_code
{
    /// Configure (see #arc_mics_configure_req_t)
    ARC_MICS_CONFIGURE = GAF_CODE(ARC, MICS, 0),
    /// Restore Bond Data (see #arc_mics_restore_bond_data_req_t)
    ARC_MICS_RESTORE_BOND_DATA = GAF_CODE(ARC, MICS, 1),
    /// Set Mute (see #arc_mics_set_mute_req_t)
    ARC_MICS_SET_MUTE = GAF_CODE(ARC, MICS, 2),
};

/// List of GAF_IND indication code values for Microphone Control Service Server module
enum arc_mics_msg_ind_code
{
    /// Bond Data (see #arc_mics_bond_data_ind_t)
    ARC_MICS_BOND_DATA = GAF_CODE(ARC, MICS, 0),
    /// Mute (see #arc_mics_mute_ind_t)
    ARC_MICS_MUTE = GAF_CODE(ARC, MICS, 1),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ARC_MICS_CONFIGURE request message
typedef struct arc_mics_configure_req
{
    /// Request code (shall be set to #ARC_MICS_CONFIGURE)
    uint16_t req_code;
    /// Required start handle
    /// If set to GATT_INVALID_HDL, the start handle will be automatically chosen
    uint16_t shdl;
    /// Mute value
    uint8_t mute;
    /// Configuration bit field (see #arc_mics_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Number of Audio Input Control Service instances to include in the service
    uint8_t nb_inputs;
    /// Local index of Audio Input Control Service instances to include in the service
    uint8_t input_lid[__ARRAY_EMPTY];
} arc_mics_configure_req_t;

/// Structure for #ARC_MICS_RESTORE_BOND_DATA request message
typedef struct arc_mics_restore_bond_data_req
{
    /// Request code (shall be set to #ARC_MICS_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
    /// Event configuration bit field
    uint8_t evt_cfg_bf;
} arc_mics_restore_bond_data_req_t;

/// Structure for #ARC_MICS_SET_MUTE command message
typedef struct arc_mics_set_mute_req
{
    /// Request code (shall be set to #ARC_MICS_SET_MUTE)
    uint16_t req_code;
    /// Mute value
    uint8_t mute;
} arc_mics_set_mute_req_t;

/// Structure for request message
typedef struct arc_mics_rsp
{
    /// Request code (see #arc_mics_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} arc_mics_rsp_t;

/// Structure for #ARC_MICS_BOND_DATA indication message
typedef struct arc_mics_bond_data_ind
{
    /// Indication code (shall be set to #ARC_MICS_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
} arc_mics_bond_data_ind_t;

/// Structure for #ARC_MICS_MUTE indication message
typedef struct arc_mics_mute_ind
{
    /// Indication code (shall be set to #ARC_MICS_MUTE)
    uint16_t ind_code;
    /// Mute value
    uint8_t mute;
} arc_mics_mute_ind_t;

/// @} ARC_MICS_MSG

#endif // ARC_MICS_MSG_H_
