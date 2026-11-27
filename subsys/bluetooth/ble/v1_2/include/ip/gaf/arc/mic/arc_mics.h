/**
 ****************************************************************************************
 *
 * @file arc_mics.h
 *
 * @brief Audio Rendering Control - Microphone Control Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_MICS_H_
#define ARC_MICS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_MICS Microphone Control Service Server
 * @ingroup ARC_MIC
 * @brief Description of Microphone Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_MICS_ENUM Enumerations
 * @ingroup ARC_MICS
 * @brief Enumerations for Microphone Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_MICS_NATIVE_API Native API
 * @ingroup ARC_MICS
 * @brief Description of Native API for Microphone Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_MICS_CALLBACK Callback Functions
 * @ingroup ARC_MICS_NATIVE_API
 * @brief Description of callback functions for Microphone Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_MICS_FUNCTION Functions
 * @ingroup ARC_MICS_NATIVE_API
 * @brief Description of functions for Microphone Control Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc_mic.h"           // Microphone Control Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_MICS_ENUM
/// @{

/// Microphone Control Configuration bit field
enum arc_mics_cfg_bf
{
    /// Indicate if it is required (= 1) to check the Lock state
    /// Meaningful only if Coordinated Set Identification Profile Set Member Role is supported
    ARC_MICS_CFG_CHECK_LOCK_POS = 0,
    ARC_MICS_CFG_CHECK_LOCK_BIT = CO_BIT(ARC_MICS_CFG_CHECK_LOCK_POS),
};

/// @} ARC_MICS_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ARC_MICS_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when Client Characteristic Configuration value has been updated by a peer client for
 * Mute characteristic
 *
 * @param[in] con_lid       Connection local index
 * @param[in] cli_cfg_bf    Client configuration bit field
 ****************************************************************************************
 */
typedef void (*arc_mics_cb_bond_data)(uint8_t con_lid, uint8_t cli_cfg_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when Mute characteristic value has been updated
 *
 * @param[in] mute         Mute value
 ****************************************************************************************
 */
typedef void (*arc_mics_cb_mute)(uint8_t mute);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Microphone Control Server
typedef struct arc_mics_cb
{
    /// Callback function called when Client Characteristic Configuration value has been updated by a peer client for
    /// Mute characteristic
    arc_mics_cb_bond_data cb_bond_data;
    /// Callback function called when Mute characteristic value has been updated
    arc_mics_cb_mute cb_mute;
} arc_mics_cb_t;

/// @} ARC_MICS_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_ARC_MICS)
/// @addtogroup ARC_MICS_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure Microphone Control Server module
 *
 * @param[in] p_cb          Pointer to set of callback functions for communication with upper layers
 * @param[in] shdl          Start handle. If set to GATT_INVALID_HDL, the start handle will be automatically chosen
 * @param[in] mute          Mute value
 * @param[in] cfg_bf        Configuration bit field (see #arc_mics_cfg_bf enumeration)
 * @param[in] nb_inputs     Number of Audio Input Control Service instances to include in the service
 * @param[in] p_input_lid   Pointer to list of local indices of Audio Input Control Service instances to include
 * in the service
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_mics_configure(const arc_mics_cb_t* p_cb, uint16_t shdl, uint8_t mute, uint8_t cfg_bf,
                            uint8_t nb_inputs, uint8_t* p_input_lid);

/**
 ****************************************************************************************
 * @brief Set bonding information related to Microphone Control after connection with a peer device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] cli_cfg_bf    Client configuration bit field
 * @param[in] evt_cfg_bf    Event configuration bit field
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_mics_restore_bond_data(uint8_t con_lid, uint8_t cli_cfg_bf, uint8_t evt_cfg_bf);

/**
 ****************************************************************************************
 * @brief Set value of Mute characteristic
 *
 * @param[in] mute          Mute value (see #arc_mic_mute enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_mics_set_mute(uint8_t mute);

/**
 ****************************************************************************************
 * @brief Mute all microphone inputs
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_mics_mute(void)
{
    return (arc_mics_set_mute(ARC_MIC_MUTE_MUTED));
}

/**
 ****************************************************************************************
 * @brief Unmute all microphone inputs
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_mics_unmute(void)
{
    return (arc_mics_set_mute(ARC_MIC_MUTE_NOT_MUTED));
}

/**
 ****************************************************************************************
 * @brief Disable all microphone inputs
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_mics_disable(void)
{
    return (arc_mics_set_mute(ARC_MIC_MUTE_DISABLED));
}

/// @} ARC_MICS_FUNCTION
#endif //(GAF_ARC_MICS)

#endif // ARC_MICS_H_
