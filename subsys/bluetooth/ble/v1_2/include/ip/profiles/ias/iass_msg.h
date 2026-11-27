/**
 ****************************************************************************************
 *
 * @file iass_msg.h
 *
 * @brief Header file - Immediate Alert Service Server - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef IASS_MSG_H_
#define IASS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup IASS_API_MSG Message API
 * @ingroup IASS_API
 * @brief Description of Message API for Immediate Alert Service Server\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "iass.h"

/// @addtogroup IASS_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Immediate Alert Service Server
enum iass_msg_id
{
    /// Updated alert level indication - See #iass_level_ind_t
    IASS_LEVEL_IND = MSG_ID(IASS, 0x00u),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #IASS_LEVEL_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Alert level (see #ias_alert_level enumeration)
    uint8_t level;
} iass_level_ind_t;

/// @} IASS_API_MSG

#endif // IASS_MSG_H_
