/**
 ****************************************************************************************
 *
 * @file atc_csisc.h
 *
 * @brief Audio Topology Control - Coordinated Set Identification Set Coordinator - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ATC_CSISC_H_
#define ATC_CSISC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ATC_CSISC Coordinated Set Identification Service Set Coordinator
 * @ingroup ATC_CSIS
 * @brief Description of Coordinated Set Identification Service Set Coordinator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_CSISC_ENUM Enumerations
 * @ingroup ATC_CSISC
 * @brief Enumerations for Coordinated Set Identification Service Set Coordinator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_CSISC_NATIVE_API Native API
 * @ingroup ATC_CSISC
 * @brief Description of Native API for Coordinated Set Identification Service Set Coordinator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_CSISC_CALLBACK Callback Functions
 * @ingroup ATC_CSISC_NATIVE_API
 * @brief Description of callback functions for Coordinated Set Identification Service Set Coordinator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_CSISC_FUNCTION Functions
 * @ingroup ATC_CSISC_NATIVE_API
 * @brief Description of functions for Coordinated Set Identification Service Set Coordinator module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"                // GAF Defines

#if (GAF_ATC_CSISC)

#include "atc_csi.h"            // Audio Topology Control - Coordinated Set Identification Definitions
#include "csisc.h"              // Coordinated Set Identification Set Coordinator Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ATC_CSISC_ENUM
/// @{

/// List of command types for Coordinated Set Identification Service Set Coordinator module
enum atc_csisc_cmd_type
{
    /// Resolve
    ATC_CSISC_CMD_TYPE_RESOLVE = 0,
    /// Discover
    ATC_CSISC_CMD_TYPE_DISCOVER,
    /// Lock
    ATC_CSISC_CMD_TYPE_LOCK,
    /// Get
    ATC_CSISC_CMD_TYPE_GET,
    /// Set Configuration
    ATC_CSISC_CMD_TYPE_SET_CFG = 5,
};

/// @} ATC_CSISC_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ATC_CSISC_CALLBACK
/// @{

/// Set of callback functions for Coordinated Set Identification Set Coordinator
typedef csisc_cb_t atc_csisc_cb_t;

/// @} ATC_CSISC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup ATC_CSISC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Create and configure Coordinated Set Identification Set Coordinator module
 *
 * @param[in] nb_sirk   Number of SIRK value that can be stored
 * @param[in] p_cb      Pointer to set of callback functions for communications with
 * upper layers
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t atc_csisc_configure(uint8_t nb_sirk, const csisc_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Resolve a RSI value
 *
 * @param[in] p_rsi        Pointer to RSI value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csisc_resolve(const csis_rsi_t* p_rsi)
{
    return (csisc_resolve(p_rsi));
}

/**
 ****************************************************************************************
 * @brief Enable use of Coordinated Set Identification Service block as Set Coordinator
 * for a connected device with which no bonding has been established during a previous
 * connection.
 *
 * @param[in] con_lid       Connection local index
 * @param[in] nb_sets_max   Maximum number of instance of the Coordinated Set Identification
 * Service that may be found
 * @param[in] shdl          Start handle for the discovery. Set GATT_INVALID_HDL if not provided.
 * @param[in] ehdl          End handle for the discovery. Set GATT_INVALID_HDL if not provided.
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csisc_discover(uint8_t con_lid, uint8_t nb_sets_max, uint16_t shdl, uint16_t ehdl)
{
    return (csisc_discover(con_lid, nb_sets_max, shdl, ehdl));
}

/**
 ****************************************************************************************
 * @brief Set bonding information for an instance of the Coordinated Set Information Service
 * after connection with a Set Coordinator device with which a bonded relationship had been
 * established during a previous connection.
 *
 * @param[in] con_lid           Connection local index
 * @param[in] nb_sets           Number of Coordinated Sets
 * @param[in] p_csis_info       Pointer to Content description structure of each Coordinated
 * Set Identification Service instance
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csisc_restore_bond_data(uint8_t con_lid, uint8_t nb_sets, const csisc_csis_info_t* p_csis_info)
{
    return (csisc_restore_bond_data(con_lid, nb_sets, p_csis_info));
}

/**
 ****************************************************************************************
 * @brief Lock or unlock a Set Member device for execution of a procedure
 *
 * @param[in] con_lid       Connection local index
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] lock          Lock state
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csisc_lock(uint8_t con_lid, uint8_t set_lid, uint8_t lock)
{
    return (csisc_lock(con_lid, set_lid, lock));
}

#if (GAF_DBG)
/**
 ****************************************************************************************
 * @brief Get value for one of the following characteristics in an instance of the Coordinated
 * Set Identification Service discovered in a Set Member device database:
 *     - Set Identity Resolving Key characteristic
 *     - Coordinated Set Size characteristic (only if supported)
 *     - Set Member Lock characteristic (only if supported)
*      - Set Member Rank characteristic (only if supported)
 *
 * @param[in] con_lid       Connection local index
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] char_type     Characteristic type
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csisc_get(uint8_t con_lid, uint8_t set_lid, uint8_t char_type)
{
    return (csisc_get(con_lid, set_lid, char_type));
}

/**
 ****************************************************************************************
 * @brief Enable or disable sending of notifications for one of the following characteristics in an
 * instance of the Coordinated Set Identification Service discovered in a Set Member device database:
 *      - Set Identity Resolving Key characteristic
 *      - Coordinated Set Size characteristic (only if supported and if sending of notifications
 * is supported for this characteristic)
 *      - Set Member Lock characteristic (only if supported and if sending of notifications
 * is supported for this characteristic)
 *
 * @param[in] con_lid           Connection local index
 * @param[in] set_lid           Coordinated Set local index
 * @param[in] char_type         Characteristic type
 * @param[in] enable            Indicate if sending of notifications must be enabled (!= 0) or not for
 * the indicated characteristic
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csisc_set_cfg(uint8_t con_lid, uint8_t set_lid, uint8_t char_type, uint8_t enable)
{
    return (csisc_set_cfg(con_lid, set_lid, char_type, enable));
}
#endif //(GAF_DBG)

/**
 ****************************************************************************************
 * @brief Add a SIRK value
 *
 * @param[in] p_sirk        Pointer to SIRK value
 * @param[out] p_key_lid    Pointer at which allocated Key local index will be returned
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csisc_add_sirk(const csis_sirk_t* p_sirk, uint8_t* p_key_lid)
{
    return (csisc_add_sirk(p_sirk, p_key_lid));
}

/**
 ****************************************************************************************
 * @brief Remove a SIRK value
 *
 * @param[in] key_lid       Key local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csisc_remove_sirk(uint8_t key_lid)
{
    return (csisc_remove_sirk(key_lid));
}

/**
 ****************************************************************************************
 * @brief Request LTK from upper layer
 *
 * @param[in] p_ltk             Pointer to LTK
 ****************************************************************************************
 */
__STATIC_FORCEINLINE void atc_csisc_ltk_cfm(const uint8_t* p_ltk)
{
    csisc_ltk_cfm(p_ltk);
}

/// @} ATC_CSISC_FUNCTION
#endif //(GAF_ATC_CSISC)

#endif // ATC_CSISC_H_
