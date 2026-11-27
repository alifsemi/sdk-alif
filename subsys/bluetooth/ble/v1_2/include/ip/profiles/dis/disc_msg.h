/**
 ****************************************************************************************
 *
 * @file disc_msg.h
 *
 * @brief Header file - Device Information Service Client - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef DISC_MSG_H_
#define DISC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup DISC_API_MSG Message API
 * @ingroup DISC_API
 * @brief Description of Message API for Device Information Service Client\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "disc.h"
#include "ke_msg.h"

/// @addtogroup DISC_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Device Information Service Client
enum disc_msg_id
{
    /// Discover DIS in peer device's database - See #disc_discover_cmd_t
    DISC_DISCOVER_CMD = MSG_ID(DISC, 0x00u),
    /// Command completed event for #DISC_DISCOVER_CMD - See #disc_discover_cmp_evt_t
    DISC_DISCOVER_CMP_EVT = MSG_ID(DISC, 0x01u),
    /// Get characteristic value - See #disc_get_cmd_t
    DISC_GET_CMD = MSG_ID(DISC, 0x02u),
    /// Command completed event for #DISC_GET_CMD - See #disc_get_cmp_evt_t
    DISC_GET_CMP_EVT = MSG_ID(DISC, 0x03u),
    /// Received value indication - See #disc_value_ind_t
    DISC_VALUE_IND = MSG_ID(DISC, 0x04u),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #DISC_DISCOVER_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
} disc_discover_cmd_t;

/// Parameters of the #DISC_DISCOVER_CMP_EVT message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Status (see #hl_err enumeration)
    uint16_t status;
    /// DIS content
    disc_content_t content;
} disc_discover_cmp_evt_t;

/// Parameters of the #DISC_GET_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #disc_char_type enumeration)
    uint8_t char_type;
} disc_get_cmd_t;

/// Parameters of the #DISC_GET_CMP_EVT message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Status (see #hl_err enumeration)
    uint16_t status;
    /// Characteristic type (see #disc_char_type enumeration)
    uint8_t char_type;
} disc_get_cmp_evt_t;

/// Parameters of the #DISC_VALUE_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #disc_char_type enumeration)
    uint8_t char_type;
    /// Length
    uint8_t length;
    /// Value
    uint8_t value[__ARRAY_EMPTY];
} disc_value_ind_t;

/// @} DISC_API_MSG

#endif // DISC_MSG_H_
