/**
 ****************************************************************************************
 *
 * @file atc_csism.h
 *
 * @brief Audio Topology Control - Coordinated Set Identification Set Member - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ATC_CSISM_H_
#define ATC_CSISM_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ATC_CSISM Coordinated Set Identification Service Set Member
 * @ingroup ATC_CSIS
 * @brief Description of Coordinated Set Identification Service Set Member module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_CSISM_ENUM Enumerations
 * @ingroup ATC_CSISM
 * @brief Enumerations for Coordinated Set Identification Service Set Member module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_CSISM_NATIVE_API Native API
 * @ingroup ATC_CSISM
 * @brief Description of Native API for Coordinated Set Identification Service Set Member module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_CSISM_CALLBACK Callback Functions
 * @ingroup ATC_CSISM_NATIVE_API
 * @brief Description of callback functions for Coordinated Set Identification Service Set Member module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_CSISM_FUNCTION Functions
 * @ingroup ATC_CSISM_NATIVE_API
 * @brief Description of functions for Coordinated Set Identification Service Set Member module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"                // GAF Defines

#if (GAF_ATC_CSISM)

#include "atc_csi.h"            // Audio Topology Control - Coordinated Set Identification Definitions
#include "csism.h"              // Coordinated Set Identification Set Member Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ATC_CSISM_ENUM
/// @{

/// List of command type values for Coordinated Set Identification Service Set Member module
enum atc_csism_cmd_type
{
    /// Add
    ATC_CSISM_CMD_TYPE_ADD = 0,
    /// Set SIRK
    ATC_CSISM_CMD_TYPE_SET_SIRK,
    /// Update RSI
    ATC_CSISM_CMD_TYPE_UPDATE_RSI,
};

/// @} ATC_CSISM_ENUM

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ATC_CSISM_CALLBACK
/// @{

/// Set of callback functions for Coordinated Set Identification Set Member
typedef csism_cb_t atc_csism_cb_t;

/// @} ATC_CSISM_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup ATC_CSISM_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Create and configure Coordinated Set Identification Set Member module
 *
 * @param[in] nb_sets   Number of Coordinated Set the device may belongs to
 * @param[in] p_cb      Pointer to set of callback functions for communications with
 * upper layers
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t atc_csism_configure(uint8_t nb_sets, const atc_csism_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Update SIRK for a given Coordinated Set
 *
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] p_sirk        Pointer to new SIRK
 *
 * @return An error value
 ****************************************************************************************
 */
uint16_t atc_csism_set_sirk(uint8_t set_lid, csis_sirk_t* p_sirk);

/**
 ****************************************************************************************
 * @brief Update RSI for a given Coordinated Set
 *
 * @param[in] set_lid       Coordinated Set local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t atc_csism_update_rsi(uint8_t set_lid);

/**
 ****************************************************************************************
 * @brief Return if Audio Topology Control Coordinated Set Identification Set Member module has been configured
 *
 * @return true if use of Coordinated Set Identification Service Set Member module has been configured else false
 ****************************************************************************************
 */
bool atc_csism_is_configured(void);

/**
 ****************************************************************************************
 * @brief Add an instance of the Coordinated Set Identification Service
 *
 * @param[in] cfg_bf            Configuration bit field
 * @param[in] size              Number of members in the added Coordinated Set
 * @param[in] rank              Rank
 * @param[in] lock_timeout_s    Lock timeout duration in seconds
 * @param[in] shdl              Required start handle
 *                              If set to GATT_INVALID_HDL, the start handle will be automatically chosen
 * @param[in] p_sirk            Pointer to SIRK value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csism_add(uint8_t cfg_bf, uint8_t size, uint8_t rank, uint8_t lock_timeout_s,
                                uint16_t shdl, const csis_sirk_t* p_sirk)
{
    return (csism_add(cfg_bf, size, rank, lock_timeout_s, shdl, p_sirk));
}

/**
 ****************************************************************************************
 * @brief Set bonding information for an instance of the Coordinated Set Information Service
 * after connection with a Set Coordinator device with which a bonded relationship had been
 * established during a previous connection.
 *
 * @param[in] con_lid           Connection local index
 * @param[in] set_lid           Set local index
 * @param[in] is_locked         Indicate if the peer device is the device for which lock has been granted
 * @param[in] cli_cfg_bf        Client configuration bit field
 * @param[in] evt_cfg_bf        Event configuration bit field
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csism_restore_bond_data(uint8_t con_lid, uint8_t set_lid, bool is_locked,
                                              uint8_t cli_cfg_bf, uint8_t evt_cfg_bf)
{
    return (csism_restore_bond_data(con_lid, set_lid, is_locked, cli_cfg_bf, evt_cfg_bf));
}

/**
 ****************************************************************************************
 * @brief Update number of devices belonging to a Coordinated Set
 *
 * @param[in] set_lid           Coordinated Set local index
 * @param[in] size              Number of members in the Coordinated Set
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csism_set_size(uint8_t set_lid, uint8_t size)
{
    return (csism_set_size(set_lid, size));
}

/**
 ****************************************************************************************
 * @brief Request LTK from upper layer
 *
 * @param[in] p_ltk             Pointer to LTK
 ****************************************************************************************
 */
__STATIC_FORCEINLINE void atc_csism_ltk_cfm(const uint8_t* p_ltk)
{
    csism_ltk_cfm(p_ltk);
}

/**
 ****************************************************************************************
 * @brief Check if procedure is allowed for the current Lock state
 *
 * @param[in] con_lid           Connection local index
 * @param[in] set_lid           Set local index
 *
 * @return True if procedure is allowed, false if not allowed
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t atc_csism_is_proc_allowed(uint8_t con_lid, uint8_t set_lid)
{
    return (csism_is_proc_allowed(con_lid, set_lid));
}

/// @} ATC_CSISM_FUNCTION
#endif //(GAF_ATC_CSISM)

#endif // ATC_CSISM_H_
