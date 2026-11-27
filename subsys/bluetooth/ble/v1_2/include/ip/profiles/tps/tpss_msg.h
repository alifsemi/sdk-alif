/**
 ****************************************************************************************
 *
 * @file tpss_msg.h
 *
 * @brief Header file - Tx Power Service Server - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TPSS_MSG_H_
#define TPSS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TPSS_API_MSG Message API
 * @ingroup TPSS_API
 * @brief Description of Message API for Tx Power Service Server\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tpss.h"

/// @addtogroup TPSS_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Tx Power Service Server
enum tpss_msg_id
{
    /// Level request indication - See #tpss_level_req_ind_t
    TPSS_LEVEL_REQ_IND = MSG_ID(TPSS, 0x00u),
    /// Level confirmation - See #tpss_level_cfm_t
    TPSS_LEVEL_CFM = MSG_ID(TPSS, 0x01u),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #TPSS_LEVEL_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Token
    uint16_t token;
} tpss_level_req_ind_t;

/// Parameters of the #TPSS_LEVEL_CFM message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Tx Power Level
    int8_t level;
    /// Token
    uint16_t token;
} tpss_level_cfm_t;

/// @} TPSS_API_MSG

#endif // TPSS_MSG_H_
