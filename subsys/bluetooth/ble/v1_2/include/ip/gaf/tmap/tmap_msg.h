/**
 ****************************************************************************************
 *
 * @file tmap_msg.h
 *
 * @brief Telephony and Media Audio Profile - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TMAP_MSG_H_
#define TMAP_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TMAP_MSG Message API
 * @ingroup TMAP_COMMON
 * @brief Description of Message API for Telephony and Media Audio Profile Common module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tmap.h"           // TMAP Definitions
#include "gaf_msg.h"        // Generic Audio Framework API Message Definitions

/// @addtogroup TMAP_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request code values for Telephony and Media Audio Profile Common module
enum tmap_msg_req_code
{
    /// Configure use of Telephone and Media Audio Profile module (see #tmap_configure_req_t)
    TMAP_CONFIGURE = GAF_CODE(TMAP, COMMON, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #TMAP_CONFIGURE request message
typedef struct tmap_configure_req
{
    /// Request code (shall be set to #TMAP_CONFIGURE)
    uint16_t req_code;
    /// Configuration bit field (see #tmap_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Configuration Parameters structure for Telephony and Media Audio Service Server Role
    tmap_tmas_cfg_param_t cfg_param_tmas;
} tmap_configure_req_t;

/// @} TMAP_MSG

#endif // TMAP_MSG_H_
