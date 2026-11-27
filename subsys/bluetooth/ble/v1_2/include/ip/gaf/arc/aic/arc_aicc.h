/**
 ****************************************************************************************
 *
 * @file arc_aicc.h
 *
 * @brief Audio Rendering Control - Audio Input Control Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_AICC_H_
#define ARC_AICC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_AICC Audio Input Control Service Client
 * @ingroup ARC_AIC
 * @brief Description of Audio Input Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AICC_ENUM Enumerations
 * @ingroup ARC_AICC
 * @brief Enumerations for Audio Input Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AICC_STRUCT Structures
 * @ingroup ARC_AICC
 * @brief Structures for Audio Input Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AICC_NATIVE_API Native API
 * @ingroup ARC_AICC
 * @brief Description of Native API for Audio Input Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AICC_CALLBACK Callback Functions
 * @ingroup ARC_AICC_NATIVE_API
 * @brief Description of callback functions for Audio Input Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AICC_FUNCTION Functions
 * @ingroup ARC_AICC_NATIVE_API
 * @brief Description of functions for Audio Input Control Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"                // GAF Defines
#include "arc_aic.h"            // Audio Rendering Control - Audio Input Control Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_AICC_ENUM
/// @{

/// List of command type values for Audio Input Control Client module
enum arc_aicc_cmd_type
{
    /// Discover
    ARC_AICC_CMD_TYPE_DISCOVER = 0,
    /// Get
    ARC_AICC_CMD_TYPE_GET,
    /// Control
    ARC_AICC_CMD_TYPE_CONTROL,
    /// Set Description
    ARC_AICC_CMD_TYPE_SET_DESCRIPTION,
    /// Set Configuration
    ARC_AICC_CMD_TYPE_SET_CFG = 5,
};

/// @} ARC_AICC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_AICC_STRUCT
/// @{

/// Audio Input Control Service content description structure
typedef struct arc_aicc_aics
{
    /// Service description
    prf_svc_t svc_info;
    /// Characteristics description
    prf_char_t char_info[ARC_AIC_CHAR_TYPE_MAX];
    /// Descriptors description
    prf_desc_t desc_info[ARC_AIC_DESC_TYPE_MAX];
} arc_aicc_aics_t;

/// @} ARC_AICC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ARC_AICC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a command has been completed
 *
 * @param[in] cmd_type      Command type (see #arc_aicc_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 * @param[in] param         Additional parameter
 ****************************************************************************************
 */
typedef void (*arc_aicc_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid, uint8_t input_lid,
                                    uint8_t param);

/**
 ****************************************************************************************
 * @brief Callback function called when Audio Input Control Service instance has been discovered in a peer server
 * database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 * @param[in] p_aics_info   Pointer to service content description
 ****************************************************************************************
 */
typedef void (*arc_aicc_cb_bond_data)(uint8_t con_lid, uint8_t input_lid, arc_aicc_aics_t* p_aics_info);

/**
 ****************************************************************************************
 * @brief Callback function called when value for Audio Input State characteristic has been received from a peer server
 * device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 * @param[in] gain          Gain
 * @param[in] mute          Mute
 * @param[in] mode          Gain Mode
 ****************************************************************************************
 */
typedef void (*arc_aicc_cb_gain)(uint8_t con_lid, uint8_t input_lid, int8_t gain, uint8_t mute, uint8_t mode);

/**
 ****************************************************************************************
 * @brief Callback function called when value for Gain Setting Properties characteristic has been received from a peer
 * server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 * @param[in] units         Gain Setting Units
 * @param[in] min           Gain Setting Minimum
 * @param[in] max           Gain Setting Maximum
 ****************************************************************************************
 */
typedef void (*arc_aicc_cb_gain_prop)(uint8_t con_lid, uint8_t input_lid, uint8_t units, int8_t min, int8_t max);

/**
 ****************************************************************************************
 * @brief Callback function called when value for Audio Input Description characteristic has been received from a peer
 * server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 * @param[in] desc_len      Audio input description length
 * @param[in] p_desc        Pointer to audio input description value
 ****************************************************************************************
 */
typedef void (*arc_aicc_cb_description)(uint8_t con_lid, uint8_t input_lid, uint16_t desc_len, const char* p_desc);

/**
 ****************************************************************************************
 * @brief Callback function called when value for Audio Input Type or Audio Audio Input Status characteristic has been
 * received from a peer server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 * @param[in] char_type     Characteristic type
 * @param[in] val           Value
 ****************************************************************************************
 */
typedef void (*arc_aicc_cb_value)(uint8_t con_lid, uint8_t input_lid, uint8_t char_type, uint8_t val);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication has been received from Server device
 *
 * @param[in] con_lid           Connection local index
 ****************************************************************************************
 */
typedef void (*arc_aicc_cb_svc_changed)(uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Audio Input Control Client
typedef struct arc_aicc_cb
{
    /// Callback function called when a command has been completed
    arc_aicc_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when value for Audio Input State characteristic has been received from a peer server
    /// device
    arc_aicc_cb_gain cb_gain;
    /// Callback function called when value for Gain Setting Properties have been received from a peer server device
    arc_aicc_cb_gain_prop cb_gain_prop;
    /// Callback function called when value for Audio Input Description characteristic has been received from a peer
    /// server device
    arc_aicc_cb_description cb_description;
    /// Callback function called when value for Audio Input Type or Audio Input Status characteristic has been received
    /// from a peer server device
    arc_aicc_cb_value cb_value;
    /// Callback function called when Audio Input Control Service instance has been discovered in a peer server
    /// database
    arc_aicc_cb_bond_data cb_bond_data;
    /// Callback function called when a service changed indication has been received from Server device
    arc_aicc_cb_svc_changed cb_svc_changed;
} arc_aicc_cb_t;

/// @} ARC_AICC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_ARC_AICC)
/// @addtogroup ARC_AICC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure Audio Input Control Client module
 *
 * @param[in] p_cb          Pointer to set of callback functions for communication with upper layers
 * @param[in] pref_mtu      Preferred MTU
 *                          Values from 0 to 63 are equivalent to 64
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aicc_configure(const arc_aicc_cb_t* p_cb, uint16_t pref_mtu);

/**
 ****************************************************************************************
 * @brief Discover Audio Input Control Service Services in peer server device database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] nb_inputs     Number of inputs
 * @param[in] p_svc_hdl     Pointer to service handles
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aicc_discover(uint8_t con_lid, uint8_t nb_inputs, prf_svc_t* p_svc_hdl);

/**
 ****************************************************************************************
 * @brief Set bonding information related to Audio Input Control after connection with a peer device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] nb_inputs     Number of inputs
 * @param[in] p_aics_info   Pointer to Audio Input Control Service description structures
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aicc_restore_bond_data(uint8_t con_lid, uint8_t nb_inputs, const arc_aicc_aics_t* p_aics_info);

/**
 ****************************************************************************************
 * @brief Control peer server device's Gain value for one of its inputs
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 * @param[in] opcode        Operation code
 * @param[in] gain          Gain Setting value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aicc_control(uint8_t con_lid, uint8_t input_lid, uint8_t opcode, int8_t gain);

/**
 ****************************************************************************************
 * @brief Set Gain for a given Audio Input
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 * @param[in] gain          Gain Setting value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aicc_set_gain(uint8_t con_lid, uint8_t input_lid, uint8_t gain)
{
    return (arc_aicc_control(con_lid, input_lid, ARC_AIC_OPCODE_SET_GAIN, gain));
}

/**
 ****************************************************************************************
 * @brief Mute a given Audio Input
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aicc_mute(uint8_t con_lid, uint8_t input_lid)
{
    return (arc_aicc_control(con_lid, input_lid, ARC_AIC_OPCODE_MUTE, 0));
}

/**
 ****************************************************************************************
 * @brief Unmute a given Audio Input
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aicc_unmute(uint8_t con_lid, uint8_t input_lid)
{
    return (arc_aicc_control(con_lid, input_lid, ARC_AIC_OPCODE_UNMUTE, 0));
}

/**
 ****************************************************************************************
 * @brief Set Manual Gain Mode for a given Audio Input
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aicc_set_manual_mode(uint8_t con_lid, uint8_t input_lid)
{
    return (arc_aicc_control(con_lid, input_lid, ARC_AIC_OPCODE_SET_MANUAL_MODE, 0));
}


/**
 ****************************************************************************************
 * @brief Set Automatic Gain Mode for a given Audio Input
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_aicc_set_auto_mode(uint8_t con_lid, uint8_t input_lid)
{
    return (arc_aicc_control(con_lid, input_lid, ARC_AIC_OPCODE_SET_AUTO_MODE, 0));
}


/**
 ****************************************************************************************
 * @brief Set value for Audio Input Description characteristic of a peer server device input
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 * @param[in] desc_len      Audio input description length
 * @param[in] p_desc        Pointer to audio input description value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aicc_set_description(uint8_t con_lid, uint8_t input_lid, uint16_t desc_len, const uint8_t* p_desc);

#if (GAF_DBG)
/**
 ****************************************************************************************
 * @brief Get value for either Audio Input State or Gain Setting Properties or Audio Input Type or AudioInput Status
 * or Audio Input Description characteristic of a peer server device input
 *
 * @param[in] con_lid       Connection local index
 * @param[in] input_lid     Input local index
 * @param[in] char_type     Characteristic type
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aicc_get(uint8_t con_lid, uint8_t input_lid, uint8_t char_type);

/**
 ****************************************************************************************
 * @brief Set Client Characteristic Configuration value for either Input State or Input Status or Audio Input
 * Description characteristic of a peer server device input
 *
 * @param[in] con_lid           Connection local index
 * @param[in] input_lid         Input local index
 * @param[in] char_type         Characteristic type
 * @param[in] enable            Indicate if sending of notifications must be enabled or disabled
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_aicc_set_cfg(uint8_t con_lid, uint8_t input_lid, uint8_t char_type, uint8_t enable);
#endif //(GAF_DBG)

/// @} ARC_AICC_FUNCTION
#endif //(GAF_ARC_AICC)

#endif // ARC_AICC_H_
