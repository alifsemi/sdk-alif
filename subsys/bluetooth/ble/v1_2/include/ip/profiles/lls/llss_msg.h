/**
 ****************************************************************************************
 *
 * @file llss_msg.h
 *
 * @brief Header file - Link Loss Service Server - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef LLSS_MSG_H_
#define LLSS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup LLSS_API_MSG Message API
 * @ingroup LLSS_API
 * @brief Description of Message API for Link Loss Service Server\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "llss.h"

/// @addtogroup LLSS_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Battery Service Server
enum llss_msg_id
{
    /// Set level request indication - See #llss_set_level_req_ind_t
    LLSS_SET_LEVEL_REQ_IND = MSG_ID(LLSS, 0x00u),
    /// Set level confirmation - See #llss_set_level_cfm_t
    LLSS_SET_LEVEL_CFM = MSG_ID(LLSS, 0x01u),
    /// Get level request indication - See #llss_get_level_req_ind_t
    LLSS_GET_LEVEL_REQ_IND = MSG_ID(LLSS, 0x02u),
    /// Get level confirmation - See #llss_get_level_cfm_t
    LLSS_GET_LEVEL_CFM = MSG_ID(LLSS, 0x03u),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #LLSS_SET_LEVEL_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Alert level (see #lls_alert_level enumeration)
    uint8_t level;
    /// Token
    uint16_t token;
} llss_set_level_req_ind_t;

/// Parameters of the #LLSS_SET_LEVEL_CFM message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Status (see #hl_err enumeration)
    uint16_t status;
    /// Token
    uint16_t token;
} llss_set_level_cfm_t;

/// Parameters of the #LLSS_GET_LEVEL_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Token
    uint16_t token;
} llss_get_level_req_ind_t;

/// Parameters of the #LLSS_GET_LEVEL_CFM message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Alert level (see #lls_alert_level enumeration)
    uint8_t level;
    /// Token
    uint16_t token;
} llss_get_level_cfm_t;

/// @} LLSS_API_MSG

#endif // LLSS_MSG_H_
