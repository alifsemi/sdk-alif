/**
 ****************************************************************************************
 *
 * @file gmap_msg.h
 *
 * @brief Gaming Audio Profile - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GMAP_MSG_H_
#define GMAP_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GMAP_MSG Message API
 * @ingroup GMAP_COMMON
 * @brief Description of Message API for Gaming Audio Profile Common module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gmap.h"           // GMAP Definitions
#include "gaf_msg.h"        // Generic Audio Framework API Message Definitions

/// @addtogroup GMAP_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request code values for Gaming Audio Profile Common module
enum gmap_msg_req_code
{
    /// Configure use of Telephone and Media Audio Profile module (see #gmap_configure_req_t)
    GMAP_CONFIGURE = GAF_CODE(GMAP, COMMON, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #GMAP_CONFIGURE request message
typedef struct gmap_configure_req
{
    /// Request code (shall be set to #GMAP_CONFIGURE)
    uint16_t req_code;
    /// Configuration bit field (see #gmap_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Configuration Parameters structure for Gaming Audio Service Server Role
    gmap_gmas_cfg_param_t cfg_param_gmas;
} gmap_configure_req_t;

/// @} GMAP_MSG

#endif // GMAP_MSG_H_
