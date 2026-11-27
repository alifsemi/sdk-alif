/**
 ****************************************************************************************
 *
 * @file gmap_gmac.h
 *
 * @brief Gaming Audio Profile - Gaming Audio Service Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GMAP_GMAC_H_
#define GMAP_GMAC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GMAP_GMAC Gaming Audio Service Client
 * @ingroup GMAP
 * @brief Description of Gaming Audio Service Client module
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @defgroup GMAP_GMAC_ENUM Enumerations
 * @ingroup GMAP_GMAC
 * @brief Enumerations for Gaming Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GMAP_GMAC_STRUCT Structures
 * @ingroup GMAP_GMAC
 * @brief Structures for Gaming Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GMAP_GMAC_NATIVE_API Native API
 * @ingroup GMAP_GMAC
 * @brief Description of Native API for Gaming Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GMAP_GMAC_CALLBACK Callback Functions
 * @ingroup GMAP_GMAC_NATIVE_API
 * @brief Description of callback functions for Gaming Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GMAP_GMAC_FUNCTION Functions
 * @ingroup GMAP_GMAC_NATIVE_API
 * @brief Description of functions for Gaming Audio Service Client module
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gmap.h"               // Gaming Audio Profile Definitions
#include "prf_types.h"          // Profile common types
#include "gaf.h"                // Generic Audio Framework Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup GMAP_GMAC_ENUM
/// @{

/// List of command type values for Gaming Audio Profile Client module
enum gmap_gmac_cmd_type
{
    /// Discover (see #gmap_gmac_discover function)
    GMAP_GMAC_CMD_TYPE_DISCOVER = 0,
};

/// @} GMAP_GMAC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GMAP_GMAC_STRUCT
/// @{

/// Union for service values
typedef union union_gmap_gmac_value
{
    /// Role bit field (see #gmap_role_bf enumeration)
    /// Characteristic type = #GMAP_CHAR_TYPE_ROLE
    uint8_t role_bf;
    /// UGG Features bit field (see #gmap_ugg_features_bf enumeration)
    /// Characteristic type = #GMAP_CHAR_TYPE_UGG_FEATURES
    uint8_t ugg_features_bf;
    /// UGT Features bit field (see #gmap_ugt_features_bf enumeration)
    /// Characteristic type = #GMAP_CHAR_TYPE_UGT_FEATURES
    uint8_t ugt_features_bf;
    /// BGS Features bit field (see #gmap_bgs_features_bf enumeration)
    /// Characteristic type = #GMAP_CHAR_TYPE_BGS_FEATURES
    uint8_t bgs_features_bf;
    /// BGR Features bit field (see #gmap_bgr_features_bf enumeration)
    /// Characteristic type = #GMAP_CHAR_TYPE_BGR_FEATURES
    uint8_t bgr_features_bf;
} union_gmap_gmac_value_u;

/// @} GMAP_GMAC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup GMAP_GMAC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a Gaming Audio Client command has been completed
 *
 * @param[in] cmd_type      Command type (see #gmap_gmac_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] con_lid       Local index
 ****************************************************************************************
 */
typedef void (*gmap_gmac_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when Gaming Audio Service has been discovered
 *
 * @param[in] con_lid       Connection local index
 * @param[in] p_gmas_info   Pointer to Gaming Audio Service description structure
 ****************************************************************************************
 */
typedef void (*gmap_gmac_cb_bond_data)(uint8_t con_lid, const prf_svc_t* p_gmas_info);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication has been received from a Server device
 *
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*gmap_gmac_cb_svc_changed)(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when GMAP Role characteristic or UGT Features value has been received from a
 * Server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] char_type     Characteristic type (see #gmap_char_type enumeration)
 * @param[in] value         Received value
 ****************************************************************************************
 */
typedef void (*gmap_gmac_cb_value)(uint8_t con_lid, uint8_t char_type, union_gmap_gmac_value_u value);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Telephone and Media Audio Service Client module communication with upper layer
struct gmap_gmac_cb
{
    /// Callback function called when a command has been completed
    gmap_gmac_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when Gaming Audio Service has been discovered
    gmap_gmac_cb_bond_data cb_bond_data;
    /// Callback function called when a service changed indication has been received from a Server device
    gmap_gmac_cb_svc_changed cb_svc_changed;
    /// Callback function called when GMAP Role or UGT Features characteristic value has been received from a Server
    /// device
    gmap_gmac_cb_value cb_value;
};

/// @} GMAP_GMAC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_GMAP_GMAC)
/// @addtogroup GMAP_GMAC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of GMAP Client module
 *
 * @param[in] p_cb          Pointer to set to callback functions for communication with upper layer
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gmap_gmac_configure(const gmap_gmac_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Enable use of Gaming Audio Profile as Client when discovery has already been performed
 *
 * @param[in] con_lid           Connection local index
 * @param[in] p_gmas_info       Pointer to description of Gaming Audio Service
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gmap_gmac_restore_bond_data(uint8_t con_lid, const prf_svc_t* p_gmas_info);

/**
 ****************************************************************************************
 * @brief Enable use of Gaming Audio Profile as Client for a connected device with which no bonding has
 * been established during a previous connection.
 *
 * @param[in] con_lid           Connection local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gmap_gmac_discover(uint8_t con_lid);

/// @} GMAP_GMAC_FUNCTION

#endif //(GAF_GMAP_GMAC)

#endif // GMAP_GMAC_H_
