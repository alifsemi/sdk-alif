/**
 ****************************************************************************************
 *
 * @file llsc_msg.h
 *
 * @brief Header file - Link Loss Service Client - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef LLSC_MSG_H_
#define LLSC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup LLSC_API_MSG Message API
 * @ingroup LLSC_API
 * @brief Description of Message API for Link Loss Service Client\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "llsc.h"
#include "ke_msg.h"

/// @addtogroup LLSC_API_MSG
/// @{

/*
 * ENUMERARIONS
 ****************************************************************************************
 */

/// Message IDs for Link Loss Service Client
enum llsc_msg_id
{
    /// Discover LLS in peer device's database - See #llsc_discover_cmd_t
    LLSC_DISCOVER_CMD = MSG_ID(LLSC, 0x00u),
    /// Get Alert Level characteristic value  - See #llsc_get_level_cmd_t
    LLSC_GET_LEVEL_CMD = MSG_ID(LLSC, 0x01u),
    /// Set Alert Level characteristic value  - See #llsc_set_level_cmd_t
    LLSC_SET_LEVEL_CMD = MSG_ID(LLSC, 0x02u),
    /// Command completed event = See #llsc_cmp_evt_t
    LLSC_CMP_EVT = MSG_ID(LLSC, 0x03u),
    /// Restore bond data - See #llsc_restore_bond_data_req_t
    LLSC_RESTORE_BOND_DATA_REQ = MSG_ID(LLSC, 0x04u),
    /// Response - See #llsc_rsp_t
    LLSC_RSP = MSG_ID(LLSC, 0x05u),
    /// Bond data updated indication - See #llsc_bond_data_ind_t
    LLSC_BOND_DATA_IND = MSG_ID(LLSC, 0x06u),
    /// Received Alert Level value indication - See #llsc_level_ind_t
    LLSC_LEVEL_IND = MSG_ID(LLSC, 0x07u),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #LLSC_DISCOVER_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
} llsc_discover_cmd_t;

/// Parameters of the #LLSC_GET_LEVEL_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
} llsc_get_level_cmd_t;

/// Parameters of the #LLSC_SET_LEVEL_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Level (see #lls_alert_level enumeration)
    uint8_t level;
} llsc_set_level_cmd_t;

/// Parameters of the #LLSC_CMP_EVT message
typedef struct
{
    /// Command code (see #llsc_cmd_code enumeration)
    uint8_t cmd_code;
    /// Connection index
    uint8_t conidx;
    /// Status (see #hl_err enumeration)
    uint16_t status;
} llsc_cmp_evt_t;

/// Parameters of the #LLSC_RESTORE_BOND_DATA_REQ message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Bond data
    llsc_content_t bond_data;
} llsc_restore_bond_data_req_t;

/// Parameters of the #LLSC_RSP message
typedef struct
{
    /// Status (see #hl_err enumeration)
    uint16_t status;
} llsc_rsp_t;

/// Parameters of the #LLSC_BOND_DATA_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Bond data
    llsc_content_t bond_data;
} llsc_bond_data_ind_t;

/// Parameters of the #LLSC_LEVEL_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Level (see #lls_alert_level enumeration)
    uint8_t level;
} llsc_level_ind_t;

/// @} LLSC_API_MSG

#endif // LLSC_MSG_H_
