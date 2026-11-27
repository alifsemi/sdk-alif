/**
 ****************************************************************************************
 *
 * @file gaf_cli.h
 *
 * @brief Generic Audio Framework - Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAF_CLI_H_
#define GAF_CLI_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GAF_CLI Client module
 * @ingroup GAF_TB
 * @brief Description of Generic Audio Framework Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_CLI_ENUM Enumerations
 * @ingroup GAF_CLI
 * @brief Enumerations for Generic Audio Framework Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_CLI_NATIVE_API Native API
 * @ingroup GAF_CLI
 * @brief Description of Native API for Generic Audio Framework Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_CLI_CALLBACK Callback Functions
 * @ingroup GAF_CLI_NATIVE_API
 * @brief Description of callback functions for Generic Audio Framework Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_CLI_FUNCTION Functions
 * @ingroup GAF_CLI_NATIVE_API
 * @brief Description of functions for Generic Audio Framework Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"       // Generic Audio Framework Defines
#include "gaf_cfg.h"   // Generic Audio Framework Configuration

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup GAF_CLI_ENUM
/// @{

/// List of command type value for Generic Audio Framework Client module
enum gaf_cli_cmd_type
{
    /// Discover services
    GAF_CLI_CMD_TYPE_DISCOVER = 0,
};

/// Discovery configuration bit field meaning (see #gaf_cli_discover function)
enum gaf_cli_disc_cfg_bf
{
    /// Discover all primary services - Position\n
    /// If not set, only GAF-related services will be discovered
    GAF_CLI_DISC_CFG_ALL_POS = 0,
    /// Discover all services - Bit
    GAF_CLI_DISC_CFG_ALL_BIT = CO_BIT(GAF_CLI_DISC_CFG_ALL_POS),
};

/// @} GAF_CLI_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup GAF_CLI_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when handling of a command has been completed
 *
 * @param[in] cmd_type      Command type (see #gaf_cli_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*gaf_cli_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when a service has been discovered in peer device database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] uuid          Service UUID
 * @param[in] shdl          Service start handle
 * @param[in] ehdl          Service end handle
 ****************************************************************************************
 */
typedef void (*gaf_cli_cb_svc_found)(uint8_t con_lid, uint16_t uuid, uint16_t shdl, uint16_t ehdl);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Generic Audio Framework Client module
struct gaf_cli_cb
{
    /// Callback function called when handling of a command has been completed
    gaf_cli_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when a service has been discovered in peer device database
    gaf_cli_cb_svc_found cb_svc_found;
};

/// @} GAF_CLI_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_CLI)
/// @addtogroup GAF_CLI_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of GAF Client module
 *
 * @param[in] p_cb          Pointer to set of callback functions for communication with upper layers
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gaf_cli_configure(const gaf_cli_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Discover list of primary services supported by a peer device\n
 * #gaf_cli_cb_svc_found callback function called each time a service is discovered in peer device database\n
 * #gaf_cli_cb_cmp_evt callback function called once procedure has been completed with #GAF_CLI_CMD_TYPE_DISCOVER
 * command type value
 *
 * @param[in] con_lid       Connection local index
 * @param[in] cfg_bf        Configuration bit field (see #gaf_cli_disc_cfg_bf enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gaf_cli_discover(uint8_t con_lid, uint8_t cfg_bf);

/// @} GAF_CLI_FUNCTION
#endif //(GAF_CLI)

#endif // GAF_CLI_H_
