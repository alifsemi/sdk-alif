/**
 ****************************************************************************************
 *
 * @file iasc_msg.h
 *
 * @brief Header file - Immediate Alert Service Client - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef IASC_MSG_H_
#define IASC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup IASC_API_MSG Message API
 * @ingroup IASC_API
 * @brief Description of Message API for Immediate Alert Service Client\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "iasc.h"
#include "ke_msg.h"

/// @addtogroup IASC_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Immediate Alert Service Client
enum iasc_msg_id
{
    /// Discover IAS in peer device's database - See #iasc_discover_cmd_t
    IASC_DISCOVER_CMD = MSG_ID(IASC, 0x00u),
    /// Set Alert Level characteristic value  - See #iasc_set_level_cmd_t
    IASC_SET_LEVEL_CMD = MSG_ID(IASC, 0x01u),
    /// Command completed event = See #iasc_cmp_evt_t
    IASC_CMP_EVT = MSG_ID(IASC, 0x02u),
    /// Restore bond data - See #iasc_restore_bond_data_req_t
    IASC_RESTORE_BOND_DATA_REQ = MSG_ID(IASC, 0x03u),
    /// Response - See #iasc_rsp_t
    IASC_RSP = MSG_ID(IASC, 0x04u),
    /// Bond data updated indication - See #iasc_bond_data_ind_t
    IASC_BOND_DATA_IND = MSG_ID(IASC, 0x05u),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #IASC_DISCOVER_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
} iasc_discover_cmd_t;

/// Parameters of the #IASC_SET_LEVEL_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Level (see #ias_alert_level enumeration)
    uint8_t level;
} iasc_set_level_cmd_t;

/// Parameters of the #IASC_CMP_EVT message
typedef struct
{
    /// Command code (see #iasc_cmd_code enumeration)
    uint8_t cmd_code;
    /// Connection index
    uint8_t conidx;
    /// Status (see #hl_err enumeration)
    uint16_t status;
} iasc_cmp_evt_t;

/// Parameters of the #IASC_RESTORE_BOND_DATA_REQ message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Bond data
    iasc_content_t bond_data;
} iasc_restore_bond_data_req_t;

/// Parameters of the #IASC_RSP message
typedef struct
{
    /// Status (see #hl_err enumeration)
    uint16_t status;
} iasc_rsp_t;

/// Parameters of the #IASC_BOND_DATA_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Bond data
    iasc_content_t bond_data;
} iasc_bond_data_ind_t;

/// @} IASC_API_MSG

#endif // IASC_MSG_H_
