/**
 ****************************************************************************************
 *
 * @file tmap_tmac.h
 *
 * @brief Telephony and Media Audio Profile - Telephony and Media Audio Service Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TMAP_TMAC_H_
#define TMAP_TMAC_H_


#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TMAP_TMAC Telephony and Media Audio Service Client
 * @ingroup TMAP
 * @brief Description of Telephony and Media Audio Service Client module
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @defgroup TMAP_TMAC_ENUM Enumerations
 * @ingroup TMAP_TMAC
 * @brief Enumerations for Telephony and Media Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TMAP_TMAC_STRUCT Structures
 * @ingroup TMAP_TMAC
 * @brief Structures for Telephony and Media Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TMAP_TMAC_NATIVE_API Native API
 * @ingroup TMAP_TMAC
 * @brief Description of Native API for Telephony and Media Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TMAP_TMAC_CALLBACK Callback Functions
 * @ingroup TMAP_TMAC_NATIVE_API
 * @brief Description of callback functions for Telephony and Media Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TMAP_TMAC_FUNCTION Functions
 * @ingroup TMAP_TMAC_NATIVE_API
 * @brief Description of functions for Telephony and Media Service Client module
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tmap.h"               // Telephony and Media Audio Profile Definitions
#include "prf_types.h"          // Profile common types
#include "gaf.h"                // Generic Audio Framework Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup TMAP_TMAC_ENUM
/// @{

/// List of command type values for Telephony and Media Audio Profile Client module
enum tmap_tmac_cmd_type
{
    /// Discover (see #tmap_tmac_discover function)
    TMAP_TMAC_CMD_TYPE_DISCOVER = 0,
};

/// @} TMAP_TMAC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup TMAP_TMAC_STRUCT
/// @{

/// Telephony and Media Service content description structure
typedef struct tmap_tmac_tmas
{
    /// Service description
    prf_svc_t svc_info;
    /// Characteristics description
    prf_char_t char_info[TMAP_CHAR_TYPE_MAX];
} tmap_tmac_tmas_t;

/// @} TMAP_TMAC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup TMAP_TMAC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a Telephony and Media Audio Client command has been completed
 *
 * @param[in] cmd_type      Command type (see #tmap_tmac_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] con_lid       Local index
 ****************************************************************************************
 */
typedef void (*tmap_tmac_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when Telephony and Media Audio Service has been discovered
 *
 * @param[in] con_lid       Connection local index
 * @param[in] p_tmas_info   Pointer to Telephony and Media Audio Service content description
 ****************************************************************************************
 */
typedef void (*tmap_tmac_cb_bond_data)(uint8_t con_lid, const tmap_tmac_tmas_t* p_tmas_info);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication has been received from a Server device
 *
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*tmap_tmac_cb_svc_changed)(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when TMAP Role characteristic value has been received from a Server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] role_bf       Bitmap of supported TMAP role (see #tmap_role_bf enumeration)
 ****************************************************************************************
 */
typedef void (*tmap_tmac_cb_role)(uint8_t con_lid, uint16_t role_bf);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Telephone and Media Audio Service Client module communication with upper layer
struct tmap_tmac_cb
{
    /// Callback function called when a command has been completed
    tmap_tmac_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when Telephony and Media Audio Service has been discovered
    tmap_tmac_cb_bond_data cb_bond_data;
    /// Callback function called when a service changed indication has been received from a Server device
    tmap_tmac_cb_svc_changed cb_svc_changed;
    /// Callback function called when Role characteristic value has been received from a Server device
    tmap_tmac_cb_role cb_role;
};

/// @} TMAP_TMAC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_TMAP_TMAC)
/// @addtogroup TMAP_TMAC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of TMAP Client module
 *
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t tmap_tmac_configure(const tmap_tmac_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Enable use of Telephony and Media Audio Profile as Client when discovery has already been performed
 *
 * @param[in] con_lid           Connection local index
 * @param[in] p_tmas_info       Pointer to content description of Telephony and Media Audio Service
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t tmap_tmac_restore_bond_data(uint8_t con_lid, const tmap_tmac_tmas_t* p_tmas_info);

/**
 ****************************************************************************************
 * @brief Enable use of Telephony and Media Audio Profile as Client for a connected device with which no bonding has
 * been established during a previous connection.
 *
 * @param[in] con_lid           Connection local index
 * @param[in] shdl              Start handle for the discovery. Set GATT_INVALID_HDL if not provided
 * @param[in] ehdl              End handle for the discovery. Set GATT_INVALID_HDL if not provided
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t tmap_tmac_discover(uint8_t con_lid, uint16_t shdl, uint16_t ehdl);

/// @} TMAP_TMAC_FUNCTION
#endif //(GAF_TMAP_TMAC)

#endif // TMAP_TMAC_H_
