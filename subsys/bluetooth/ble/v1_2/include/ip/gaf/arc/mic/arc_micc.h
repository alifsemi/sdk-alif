/**
 ****************************************************************************************
 *
 * @file arc_micc.h
 *
 * @brief Audio Rendering Control - Microphone Control Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_MICC_H_
#define ARC_MICC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_MICC Microphone Control Service Client
 * @ingroup ARC_MIC
 * @brief Description of Microphone Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_MICC_ENUM Enumerations
 * @ingroup ARC_MICC
 * @brief Enumerations for Microphone Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_MICC_STRUCT Structures
 * @ingroup ARC_MICC
 * @brief Structures for Microphone Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_MICC_NATIVE_API Native API
 * @ingroup ARC_MICC
 * @brief Description of Native API for Microphone Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_MICC_CALLBACK Callback Functions
 * @ingroup ARC_MICC_NATIVE_API
 * @brief Description of callback functions for Microphone Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_MICC_FUNCTION Functions
 * @ingroup ARC_MICC_NATIVE_API
 * @brief Description of functions for Microphone Control Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"               // GAF Defines
#include "arc_mic.h"           // Audio Rendering Control - Microphone Control Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_MICC_ENUM
/// @{

/// List of command types values for Microphone Control Service Client module
enum arc_micc_cmd_type
{
    /// Discover
    ARC_MICC_CMD_TYPE_DISCOVER = 0,
    /// Get Mute
    ARC_MICC_CMD_TYPE_GET_MUTE,
    /// Set Mute
    ARC_MICC_CMD_TYPE_SET_MUTE,
    /// Set Configuration
    ARC_MICC_CMD_TYPE_SET_CFG = 4,
};

/// @} ARC_MICC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_MICC_STRUCT
/// @{

/// Microphone Control Service content description structure
typedef struct arc_micc_mics
{
    /// Service description
    prf_svc_t svc_info;
    /// Characteristics description
    prf_char_t char_info[ARC_MIC_CHAR_TYPE_MAX];
    /// Descriptors description
    prf_desc_t desc_info[ARC_MIC_DESC_TYPE_MAX];
} arc_micc_mics_t;

/// @} ARC_MICC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ARC_MICC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a command has been completed
 *
 * @param[in] cmd_type      Command type (see #arc_micc_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*arc_micc_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when Microphone Control Service instance has been discovered in a peer server
 * database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] p_mics_info   Pointer to service content description
 ****************************************************************************************
 */
typedef void (*arc_micc_cb_bond_data)(uint8_t con_lid, arc_micc_mics_t* p_mics_info);

/**
 ****************************************************************************************
 * @brief Callback function called when Mute characteristic value has been received from a peer server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] mute          Mute value
 ****************************************************************************************
 */
typedef void (*arc_micc_cb_mute)(uint8_t con_lid, uint8_t mute);

/**
 ****************************************************************************************
 * @brief Callback function called when an Audio Input Control Service instance has been discovered in a peer server
 *
 * @param[in] con_lid       Connection local index
 * @param[in] shdl          Start handle
 * @param[in] ehdl          End handle
 ****************************************************************************************
 */
typedef void (*arc_micc_cb_included_svc)(uint8_t con_lid, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication is received from a Server device
 *
 * @param[in] con_lid       Connection local index
 ****************************************************************************************
 */
typedef void (*arc_micc_cb_svc_changed)(uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for microphone management (client)
typedef struct arc_micc_cb
{
    /// Callback function called when a command has been completed
    arc_micc_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when Mute characteristic value has been received from a peer server device
    arc_micc_cb_mute cb_mute;
    /// Callback function called when Microphone Control Service instance has been discovered in a peer server database
    arc_micc_cb_bond_data cb_bond_data;
    /// Callback function called when an Audio Input Control Service instance has been discovered in a peer server
    arc_micc_cb_included_svc cb_included_svc;
    /// Callback function called when a service changed indication is received from a Server device
    arc_micc_cb_svc_changed cb_svc_changed;
} arc_micc_cb_t;

/// @} ARC_MICC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_ARC_MICC)
/// @addtogroup ARC_MICC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Create and configure Microphone Control Client module
 *
 * @param[in] p_cb      Pointer to set of callback functions for communications with upper layers
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_micc_configure(const arc_micc_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Initiate discovery of Microphone Control Service in peer server device database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] shdl          Start handle for the discovery. Set GATT_INVALID_HDL if not provided
 * @param[in] ehdl          End handle for the discovery. Set GATT_INVALID_HDL if not provided
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_micc_discover(uint8_t con_lid, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Set bonding information related to Microphone Management after connection with a peer device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] p_mics_info   Pointer to Microphone Control Service description structures
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_micc_restore_bond_data(uint8_t con_lid, const arc_micc_mics_t* p_mics_info);

/**
 ****************************************************************************************
 * @brief Set value for Mute State characteristic of a peer server device output
 *
 * @param[in] con_lid       Connection local index
 * @param[in] mute          Mute value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_micc_set_mute(uint8_t con_lid, uint8_t mute);

/**
 ****************************************************************************************
 * @brief Mute microphone
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_micc_mute(uint8_t con_lid)
{
    return (arc_micc_set_mute(con_lid, ARC_MIC_MUTE_MUTED));
}

/**
 ****************************************************************************************
 * @brief Unmute microphone
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_micc_unmute(uint8_t con_lid)
{
    return (arc_micc_set_mute(con_lid, ARC_MIC_MUTE_NOT_MUTED));
}

#if (GAF_DBG)
/**
 ****************************************************************************************
 * @brief Get value for Mute State characteristic of a peer server device output
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_micc_get_mute(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Set Client Characteristic Configuration value for Mute characteristic.
 *
 * @param[in] con_lid           Connection local index
 * @param[in] enable            Indicate if sending of notifications must be enabled or disabled
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_micc_set_cfg(uint8_t con_lid, uint8_t enable);
#endif //(GAF_DBG)

/// @} ARC_MICC_FUNCTION
#endif //(GAF_ARC_MICC)

#endif // ARC_MICC_H_
