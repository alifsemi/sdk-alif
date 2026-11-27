/**
 ****************************************************************************************
 *
 * @file arc_aics.h
 *
 * @brief Audio Rendering Control - Audio Input Control Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_AICS_H_
#define ARC_AICS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_AICS Audio Input Control Service Server
 * @ingroup ARC_AIC
 * @brief Description of Audio Input Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AICS_ENUM Enumerations
 * @ingroup ARC_AICS
 * @brief Enumerations for Audio Input Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AICS_NATIVE_API Native API
 * @ingroup ARC_AICS
 * @brief Description of Native API for Audio Input Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AICS_CALLBACK Callback Functions
 * @ingroup ARC_AICS_NATIVE_API
 * @brief Description of callback functions for Audio Input Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AICS_FUNCTION Functions
 * @ingroup ARC_AICS_NATIVE_API
 * @brief Description of functions for Audio Input Control Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc_aic.h"            // Audio Rendering Control - Audio Input Control Definition

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// @addtogroup ARC_AICS_ENUM
/// @{

/// Input configuration bit field
enum arc_aics_cfg_bf
{
    /// Set to 1 if Audio Input Description characteristic is writable
    ARC_AICS_CFG_DESC_WR_POS = 0,
    ARC_AICS_CFG_DESC_WR_BIT = CO_BIT(ARC_AICS_CFG_DESC_WR_POS),

    /// Set to 1 if Audio Input Description characteristic supports sending of notifications
    ARC_AICS_CFG_DESC_NTF_POS = 1,
    ARC_AICS_CFG_DESC_NTF_BIT = CO_BIT(ARC_AICS_CFG_DESC_NTF_POS),

    /// Set to 1 if it is required to check the Lock state
    /// Meaningful only if Coordinated Set Identification Profile Set Member Role is supported
    ARC_AICS_CFG_CHECK_LOCK_POS = 2,
    ARC_AICS_CFG_CHECK_LOCK_BIT = CO_BIT(ARC_AICS_CFG_CHECK_LOCK_POS),
};

/// Set type values
enum arc_aics_set_type
{
    /// Set Input Status
    ARC_AICS_SET_TYPE_INPUT_STATUS = 0,
    /// Set Gain Setting
    ARC_AICS_SET_TYPE_GAIN,
    /// Set Mute
    ARC_AICS_SET_TYPE_MUTE,
    /// Set Gain Mode
    ARC_AICS_SET_TYPE_GAIN_MODE,

    ARC_AICS_SET_TYPE_MAX,
};

/// @} ARC_AICS_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ARC_AICS_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when Client Characteristic Configuration of a notification-capable
 * characteristic has been updated by a peer client
 *
 * @param[in] input_lid     Input local index
 * @param[in] con_lid       Connection local index
 * @param[in] cli_cfg_bf    Client configuration bit field
 ****************************************************************************************
 */
typedef void (*arc_aics_cb_bond_data)(uint8_t input_lid, uint8_t con_lid, uint8_t cli_cfg_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when Audio Input State characteristic value has been updated
 *
 * @param[in] input_lid     Input local index
 * @param[in] p_state       Pointer to Input State value
 ****************************************************************************************
 */
typedef void (*arc_aics_cb_state)(uint8_t input_lid, arc_aic_state_t* p_state);

/**
 ****************************************************************************************
 * @brief Callback function called when Audio Input Description characteristic value has been updated
 *
 * @param[in] input_lid     Input local index
 * @param[in] con_lid       Connection local index
 * @param[in] desc_len      Value length
 * @param[in] p_desc        Pointer to value
 ****************************************************************************************
 */
typedef void (*arc_aics_cb_description_req)(uint8_t input_lid, uint8_t con_lid, uint8_t desc_len,
                                            const char* p_desc);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Audio Input Control Server
typedef struct arc_aics_cb
{
    /// Callback function called when Client Characteristic Configuration of a notification-capable characteristic has
    ///  been updated by a peer client
    arc_aics_cb_bond_data cb_bond_data;
    /// Callback function called when Audio Input State characteristic value has been updated
    arc_aics_cb_state cb_state;
    /// Callback function called when Audio Input Description characteristic value has been updated
    arc_aics_cb_description_req cb_description_req;
} arc_aics_cb_t;

/// @} ARC_AICS_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_ARC_AICS)
/// @addtogroup ARC_AICS_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure Audio Input Control Server module
 *
 * @param[in] p_cb          Pointer to set of callback functions
 * @param[in] nb_inputs     Number of inputs
 * @param[in] pref_mtu      Preferred MTU
 *                          Values from 0 to 63 are equivalent to 64
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aics_configure(const arc_aics_cb_t* p_cb, uint8_t nb_inputs, uint16_t pref_mtu);

/**
 ****************************************************************************************
 * @brief Add an input
 *
 * @param[in] p_gain_prop       Pointer to Gain Setting Properties initial value
 * @param[in] input_type        Input Type
 * @param[in] desc_max_len      Maximum length of Audio Input Description
 * @param[in] cfg_bf            Configuration bit field (see #arc_aics_cfg_bf enumeration)
 * @param[in] shdl              Required start handle.
 *                              If set to GATT_INVALID_HDL, the start handle will be automatically chosen
 * @param[out] p_input_lid      Pointer at which allocated input local index is returned
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aics_add(const arc_aic_gain_prop_t* p_gain_prop, uint8_t input_type,uint8_t desc_max_len,
                      uint8_t cfg_bf, uint16_t shdl, uint8_t* p_input_lid);

/**
 ****************************************************************************************
 * @brief Set bonding information related to an input after establishment of a connection
 *
 * @param[in] input_lid     Input local index
 * @param[in] con_lid       Connection local index
 * @param[in] cli_cfg_bf    Client configuration bit field
 * @param[in] evt_cfg_bf    Event configuration bit field
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aics_restore_bond_data(uint8_t input_lid, uint8_t con_lid, uint8_t cli_cfg_bf, uint8_t evt_cfg_bf);

/**
 ****************************************************************************************
 * @brief Set value of Audio Input Status characteristic
 *
 * @param[in] input_lid     Input local index
 * @param[in] set_type      Set type (see #arc_aics_set_type enumeration)
 * @param[in] value         Value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aics_set(uint8_t input_lid, uint8_t set_type, uint32_t value);

/**
 ****************************************************************************************
 * @brief Set input status of a given Audio Input
 *
 * @param[in] input_lid     Input local index
 * @param[in] status        Input status (see #arc_aic_input_status enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aics_set_status(uint8_t input_lid, uint8_t status)
{
    return (arc_aics_set(input_lid, ARC_AICS_SET_TYPE_INPUT_STATUS, status));
}

/**
 ****************************************************************************************
 * @brief Set gain value for a given Audio Input
 *
 * @param[in] input_lid     Input local index
 * @param[in] gain          Gain value (see #arc_aic_gain_mode enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aics_set_gain(uint8_t input_lid, uint8_t gain)
{
    return (arc_aics_set(input_lid, ARC_AICS_SET_TYPE_GAIN, gain));
}

/**
 ****************************************************************************************
 * @brief Set gain mode for a given Audio Input
 *
 * @param[in] input_lid     Input local index
 * @param[in] gain_mode     Gain mode value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aics_set_gain_mode(uint8_t input_lid, uint8_t gain_mode)
{
    return (arc_aics_set(input_lid, ARC_AICS_SET_TYPE_GAIN_MODE, gain_mode));
}

/**
 ****************************************************************************************
 * @brief Mute a given Audio Input
 *
 * @param[in] input_lid     Input local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aics_mute(uint8_t input_lid)
{
    return (arc_aics_set(input_lid, ARC_AICS_SET_TYPE_MUTE, ARC_AIC_MUTE_MUTED));
}

/**
 ****************************************************************************************
 * @brief Unmute a given Audio Input
 *
 * @param[in] input_lid     Input local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aics_unmute(uint8_t input_lid)
{
    return (arc_aics_set(input_lid, ARC_AICS_SET_TYPE_MUTE, ARC_AIC_MUTE_NOT_MUTED));
}

/**
 ****************************************************************************************
 * @brief Disable a given Audio Input
 *
 * @param[in] input_lid     Input local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aics_disable(uint8_t input_lid)
{
    return (arc_aics_set(input_lid, ARC_AICS_SET_TYPE_MUTE, ARC_AIC_MUTE_DISABLED));
}

/**
 ****************************************************************************************
 * @brief Set value of Audio Input Description characteristic value
 *
 * @param[in] input_lid      Input local index
 * @param[in] desc_len       Value length
 * @param[in] p_desc         Pointer to value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aics_set_description(uint8_t input_lid, uint8_t desc_len, const uint8_t *p_desc);

/**
 ****************************************************************************************
 * @brief Confirm or not value written for Audio Input Description characteristic
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] input_lid         Input local index
 * @param[in] desc_len          Audio input description length
 * @param[in] p_desc            Pointer to Audio input description value
 ****************************************************************************************
 */
void arc_aics_set_description_cfm(bool accept, uint8_t input_lid, uint8_t desc_len, const uint8_t *p_desc);

/// @} ARC_AICS_FUNCTION
#endif //(GAF_ARC_AICS)

#endif // ARC_AICS_H_
