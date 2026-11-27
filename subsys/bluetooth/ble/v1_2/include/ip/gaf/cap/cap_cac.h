/**
 ****************************************************************************************
 *
 * @file cap_cac.h
 *
 * @brief Common Audio Profile - Common Audio Service Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CAP_CAC_H_
#define CAP_CAC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CAP_CAC Common Audio Service Client
 * @ingroup CAP
 * @brief Description of Common Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_CAC_ENUM Enumerations
 * @ingroup CAP_CAC
 * @brief Enumerations for Common Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_CAC_STRUCT Structures
 * @ingroup CAP_CAC
 * @brief Structures for Common Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_CAC_NATIVE_API Native API
 * @ingroup CAP_CAC
 * @brief Description of Native API for Common Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_CAC_CALLBACK Callback Functions
 * @ingroup CAP_CAC_NATIVE_API
 * @brief Description of callback functions for Common Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_CAC_FUNCTION Functions
 * @ingroup CAP_CAC_NATIVE_API
 * @brief Description of functions for Common Audio Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "cap.h"                // Common Audio Profile Definitions
#include "prf_types.h"          // Profile common types

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup CAP_CAC_ENUM
/// @{

/// List of command type values for Common Audio Service Client module
enum cap_cac_cmd_type
{
    /// Discover
    CAP_CAC_CMD_TYPE_DISCOVER = 0,
};

/// @} CAP_CAC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup CAP_CAC_STRUCT
/// @{

/// Common Audio Service content description structure
typedef struct cap_cac_cas
{
    /// Service description
    prf_svc_t svc_info;
} cap_cac_cas_t;

/// @} CAP_CAC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup CAP_CAC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a CAP_CAC command has been completed
 *
 * @param[in] cmd_type      Command type (see #cap_cac_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] con_lid       Local index
 ****************************************************************************************
 */
typedef void (*cap_cac_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when Common Audio Service has been discovered
 *
 * @param[in] con_lid       Connection local index
 * @param[in] p_cas_info    Pointer to Common Audio Service content description structure
 * @param[in] csis_shdl     Start handle of included Coordinated Set Identification Service
 * @param[in] csis_ehdl     End handle of included Coordinated Set Identification Service
 ****************************************************************************************
 */
typedef void (*cap_cac_cb_bond_data)(uint8_t con_lid, const cap_cac_cas_t* p_cas_info, uint16_t csis_shdl,
                                     uint16_t csis_ehdl);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication has been received from a Server device
 *
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*cap_cac_cb_svc_changed)(uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Common Audio Service Client module communication with upper layer
struct cap_cac_cb
{
    /// Callback function called when a command has been completed
    cap_cac_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when Common Audio Service has been discovered
    cap_cac_cb_bond_data cb_bond_data;
    /// Callback function called when a service changed indication has been received from a Server device
    cap_cac_cb_svc_changed cb_svc_changed;
};

/// @} CAP_CAC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_CAP_CAC)
/// @addtogroup CAP_CAC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of CAP Client module
 *
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t cap_cac_configure(const cap_cac_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Enable use of Common Audio Profile as Client as discover Common Audio Service in a Server device database
 *
 * @param[in] con_lid           Connection local index
 * @param[in] shdl              Start handle for the discovery. Set GATT_INVALID_HDL if not provided
 * @param[in] ehdl              End handle for the discovery. Set GATT_INVALID_HDL if not provided
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t cap_cac_discover(uint8_t con_lid, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Enable use of Common Audio Profile as Client when discovery has already been performed
 *
 * @param[in] con_lid           Connection local index
 * @param[in] p_cas_info        Pointer to Common Audio Service content description structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t cap_cac_restore_bond_data(uint8_t con_lid, const cap_cac_cas_t* p_cas_info);

/// @} CAP_CAC_FUNCTION
#endif //(GAF_CAP_CAC)

#endif // CAP_CAC_H_
