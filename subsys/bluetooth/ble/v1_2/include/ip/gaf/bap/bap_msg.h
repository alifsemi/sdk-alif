/**
 ****************************************************************************************
 *
 * @file bap_msg.h
 *
 * @brief Basic Audio Profile - Definition of Kernel Messages
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_MSG_H_
#define BAP_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_MSG Message API
 * @ingroup BAP_COMMON
 * @brief Description of Message API for Basic Audio Profile block
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap.h"            // BAP Definitions
#include "gaf_msg.h"        // Generic Audio Framework API Message Definitions

/// @addtogroup BAP_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request codes
enum bap_msg_req_code
{
    /// Configure Basic Audio Profile (see #bap_configure_req_t)
    BAP_CONFIGURE = GAF_CODE(BAP, COMMON, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #BAP_CONFIGURE request message
typedef struct bap_configure_req
{
    /// Request code (see #bap_msg_req_code enumeration)
    ///  - BAP_CONFIGURE
    uint16_t req_code;
    /// Supported role bit field (see #bap_role_bf enumeration)
    uint32_t role_bf;
    /// Capabilities Server configuration
    bap_capa_srv_cfg_t capa_srv_cfg;
    /// Capabilities Client configuration
    bap_capa_cli_cfg_t capa_cli_cfg;
    /// Unicast Server configuration
    bap_uc_srv_cfg_t uc_srv_cfg;
    /// Unicast Client configuration
    bap_uc_cli_cfg_t uc_cli_cfg;
    /// Broadcast Delegator configuration
    bap_bc_deleg_cfg_t bc_deleg_cfg;
    /// Broadcast Assistant configuration
    bap_bc_assist_cfg_t bc_assist_cfg;
} bap_configure_req_t;

/// @} BAP_MSG

#endif // BAP_MSG_H_
