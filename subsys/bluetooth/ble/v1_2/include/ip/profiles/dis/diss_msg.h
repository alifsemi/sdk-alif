/**
 ****************************************************************************************
 *
 * @file diss_msg.h
 *
 * @brief Header file - Device Information Service Server - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef DISS_MSG_H_
#define DISS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup DISS_API_MSG Message API
 * @ingroup DISS_API
 * @brief Description of Message API for Device Information Service Server\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 *     - Configuration parameter is a uint16_t bit field whose each bit indicate support for a characteristic based
 * on #diss_char_type enumeration
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "diss.h"

/// @addtogroup DISS_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Device Information Service Server
enum diss_msg_id
{
    /// Value request indication - See #diss_value_req_ind_t
    DISS_VALUE_REQ_IND = MSG_ID(DISS, 0x00u),
    /// Value confirmation - See #diss_value_cfm_t
    DISS_VALUE_CFM = MSG_ID(DISS, 0x01u),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #DISS_VALUE_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #diss_char_type enumeration)
    uint8_t char_type;
    /// Token
    uint16_t token;
} diss_value_req_ind_t;

/// Parameters of the #DISS_VALUE_CFM message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #diss_char_type enumeration)
    uint8_t char_type;
    /// Token
    uint16_t token;
    /// Length
    uint8_t length;
    /// Value
    uint8_t value[__ARRAY_EMPTY];
} diss_value_cfm_t;

/// @} DISS_API_MSG

#endif // DISS_MSG_H_
