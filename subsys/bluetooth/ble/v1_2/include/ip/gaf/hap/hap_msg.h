/**
 ****************************************************************************************
 *
 * @file hap_msg.h
 *
 * @brief Hearing Access Profile - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef HAP_MSG_H_
#define HAP_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup HAP_COMMON_MSG Message API
 * @ingroup HAP_COMMON
 * @brief Description of Message API for Hearing Access Profile Common module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "hap.h"            // Hearing Access Profile Definitions
#include "gaf_msg.h"        // Generic Audio Framework API Message Definitions
#include "hap_has.h"        // Hearing Access Server Definitions
#include "hap_hac.h"        // Hearing Access Client Definitions

/// @addtogroup HAP_COMMON_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request code value for Hearing Access Profile Common module
enum hap_msg_req_code
{
    /// Configure (see #hap_configure_req_t)
    HAP_CONFIGURE = GAF_CODE(HAP, COMMON, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #HAP_CONFIGURE request message
typedef struct hap_configure_req
{
    /// Request code (shall be set to #HAP_CONFIGURE)
    uint16_t req_code;
    /// Configuration bit field (see #hap_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Configuration Parameters structure for Hearing Access Service Server Role
    hap_has_cfg_param_t cfg_param_has;
    /// Configuration Parameters structure for Hearing Access Service Client Role
    hap_hac_cfg_param_t cfg_param_hac;
} hap_configure_req_t;

/// @} HAP_COMMON_MSG

#endif // HAP_MSG_H_
