/**
 ****************************************************************************************
 *
 * @file gaf_cli_msg.h
 *
 * @brief Generic Audio Framework - Client - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAF_CLI_MSG_H_
#define GAF_CLI_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GAF_CLI_MSG Message API
 * @ingroup GAF_CLI
 * @brief Description of Message API for Generic Audio Framework Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf_msg.h"            // GAF Message API Definitions
#include "gaf_cli.h"            // GAF Client Definitions

/// @addtogroup GAF_CLI_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Generic Audio Framework Client module
enum gaf_cli_msg_cmd_code
{
    /// Discover (see #gaf_cli_discover_cmd_t)
    GAF_CLI_DISCOVER = GAF_CODE(GAF, CLI, GAF_CLI_CMD_TYPE_DISCOVER),
};

/// List of GAF_IND request code values for Generic Audio Framework Client module
enum gaf_cli_msg_ind_code
{
    /// Service found (see #gaf_cli_svc_found_ind_t)
    GAF_CLI_SVC_FOUND = GAF_CODE(GAF, CLI, 0),
};


/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #GAF_CLI_DISCOVER command message
typedef struct gaf_cli_discover_cmd
{
    /// Command code (shall be set to #GAF_CLI_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Configuration bit field (see #gaf_cli_disc_cfg_bf enumeration)
    uint8_t cfg_bf;
} gaf_cli_discover_cmd_t;

/// Structure for command complete event message
typedef struct gaf_cli_cmp_evt
{
    /// Command code (see #gaf_cli_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} gaf_cli_cmp_evt_t;

/// Structure for #GAF_CLI_SVC_FOUND indication message
typedef struct gaf_cli_svc_found_ind
{
    /// Indication code (shall be set to #GAF_CLI_SVC_FOUND)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// UUID
    uint16_t uuid;
    /// Start handle to be used for discovery of service content
    uint16_t shdl;
    /// End handle
    uint16_t ehdl;
} gaf_cli_svc_found_ind_t;

/// @} GAF_CLI_MSG

#endif // GAF_CLI_MSG_H_
