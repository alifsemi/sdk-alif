/**
 ****************************************************************************************
 *
 * @file tpsc_msg.h
 *
 * @brief Header file - Tx Power Service Client - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TPSC_MSG_H_
#define TPSC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TPSC_API_MSG Message API
 * @ingroup TPSC_API
 * @brief Description of Message API for Tx Power Service Client\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tpsc.h"
#include "ke_msg.h"

/// @addtogroup TPSC_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Tx Power Service Client
enum tpsc_msg_id
{
    /// Discover TPS in peer device's database - See #tpsc_discover_cmd_t
    TPSC_DISCOVER_CMD = MSG_ID(TPSC, 0x00u),
    /// Command completed event for #TPSC_DISCOVER_CMD - See #tpsc_discover_cmp_evt_t
    TPSC_DISCOVER_CMP_EVT = MSG_ID(TPSC, 0x01u),
    /// Get Tx Power Level characteristic value  - See #tpsc_get_level_cmd_t
    TPSC_GET_LEVEL_CMD = MSG_ID(TPSC, 0x02u),
    /// Command completed event for #TPSC_GET_LEVEL_CMD - See #tpsc_get_cmp_evt_t
    TPSC_GET_CMP_EVT = MSG_ID(TPSC, 0x03u),
    /// Received Tx Power Level indication - See #tpsc_level_ind_t
    TPSC_LEVEL_IND = MSG_ID(TPSC, 0x04u),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #TPSC_DISCOVER_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
} tpsc_discover_cmd_t;

/// Parameters of the #TPSC_DISCOVER_CMP_EVT message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Status (see #hl_err enumeration)
    uint16_t status;
    /// TPS content
    tpsc_content_t content;
} tpsc_discover_cmp_evt_t;

/// Parameters of the #TPSC_GET_LEVEL_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
} tpsc_get_level_cmd_t;

/// Parameters of the #TPSC_GET_CMP_EVT message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Status (see #hl_err enumeration)
    uint16_t status;
} tpsc_get_cmp_evt_t;

/// Parameters of the #TPSC_LEVEL_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Tx Power Level
    int8_t level;
} tpsc_level_ind_t;

/// @} TPSC_API_MSG

#endif // TPSC_MSG_H_
