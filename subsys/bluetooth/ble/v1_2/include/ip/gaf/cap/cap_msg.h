/**
 ****************************************************************************************
 *
 * @file cap_msg.h
 *
 * @brief Common Audio Profile - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CAP_MSG_H_
#define CAP_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CAP_MSG Message API
 * @ingroup CAP_COMMON
 * @brief Description of Message API for Common Audio Profile module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "cap.h"            // CAP Definitions
#include "gaf_msg.h"        // Generic Audio Framework API Message Definitions

/// @addtogroup CAP_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request code values for Common Audio Profile Common module
enum cap_msg_req_code
{
    /// Common - Configure (see #cap_configure_req_t)
    CAP_CONFIGURE = GAF_CODE(CAP, COMMON, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #CAP_CONFIGURE request message
typedef struct cap_configure_req
{
    /// Request code (shall be set to #CAP_CONFIGURE)
    uint16_t req_code;
    /// Configuration bit field (see #cap_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Configuration Parameters structure for Common Audio Service Server Role
    cap_cas_cfg_param_t cfg_param_cas;
} cap_configure_req_t;

/// @} CAP_COMMON_MSG

#endif // CAP_MSG_H_
