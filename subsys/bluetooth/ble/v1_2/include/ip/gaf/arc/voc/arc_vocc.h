/**
 ****************************************************************************************
 *
 * @file arc_vocc.h
 *
 * @brief Audio Rendering Control - Volume Offset Control Service Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_VOCC_H_
#define ARC_VOCC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_VOCC Volume Offset Control Service Client
 * @ingroup ARC_VOC
 * @brief Description of Volume Offset Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOCC_ENUM Enumerations
 * @ingroup ARC_VOCC
 * @brief Enumerations for Volume Offset Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOCC_STRUCT Structures
 * @ingroup ARC_VOCC
 * @brief Structures for Volume Offset Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOCC_NATIVE_API Native API
 * @ingroup ARC_VOCC
 * @brief Description of Native API for Volume Offset Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOCC_CALLBACK Callback Functions
 * @ingroup ARC_VOCC_NATIVE_API
 * @brief Description of callback functions for Volume Offset Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOCC_FUNCTION Functions
 * @ingroup ARC_VOCC_NATIVE_API
 * @brief Description of functions for Volume Offset Control Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"                // GAF Defines
#include "arc_voc.h"            // Audio Rendering Control - Volume Offset Control Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_VOCC_ENUM
/// @{

/// List of command type values for Volume Offset Control Service Client module
enum arc_vocc_cmd_type
{
    /// Discover
    ARC_VOCC_CMD_TYPE_DISCOVER = 0,
    /// Get
    ARC_VOCC_CMD_TYPE_GET,
    /// Set Audio Description
    ARC_VOCC_CMD_TYPE_SET_DESCRIPTION,
    /// Set
    ARC_VOCC_CMD_TYPE_SET,
    /// Set Configuration
    ARC_VOCC_CMD_TYPE_SET_CFG = 5,
};

/// @} ARC_VOCC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_VOCC_STRUCT
/// @{

/// Volume Offset Control Service content description structure
typedef struct arc_vocc_vocs
{
    /// Service description
    prf_svc_t svc_info;
    /// Characteristics description
    prf_char_t char_info[ARC_VOC_CHAR_TYPE_MAX];
    /// Descriptors description
    prf_desc_t desc_info[ARC_VOC_DESC_TYPE_MAX];
} arc_vocc_vocs_t;

/// @} ARC_VOCC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ARC_VOCC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a command has been completed
 *
 * @param[in] cmd_type      Command type (see #arc_vocc_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] con_lid       Connection local index
 * @param[in] output_lid    Output local index
 * @param[in] param         Additional parameter
 ****************************************************************************************
 */
typedef void (*arc_vocc_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid, uint8_t output_lid,
                                    uint8_t param);

/**
 ****************************************************************************************
 * @brief Callback function called when a Volume Offset Control Service instance has been discovered in a peer server
 * database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] output_lid    Output local index
 * @param[in] p_svc_info    Pointer to service content description
 ****************************************************************************************
 */
typedef void (*arc_vocc_cb_bond_data)(uint8_t con_lid, uint8_t output_lid, arc_vocc_vocs_t* p_svc_info);

/**
 ****************************************************************************************
 * @brief Callback function called when Audio Location or Offset State characteristic value has been received from a
 * peer server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] output_lid    Output local index
 * @param[in] char_type     Characteristic type
 * @param[in] value         Value
 ****************************************************************************************
 */
typedef void (*arc_vocc_cb_value)(uint8_t con_lid, uint8_t output_lid, uint8_t char_type, uint32_t value);

/**
 ****************************************************************************************
 * @brief Callback function called when Audio Output Description characteristic value has been received from a peer
 * server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] output_lid    Output local index
 * @param[in] desc_len      Audio output description length
 * @param[in] p_desc        Pointer to audio output description value
 ****************************************************************************************
 */
typedef void (*arc_vocc_cb_description)(uint8_t con_lid, uint8_t output_lid, uint16_t desc_len, const char* p_desc);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication is received from a Server device
 *
 * @param[in] con_lid           Connection local index
 ****************************************************************************************
 */
typedef void (*arc_vocc_cb_svc_changed)(uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Volume Offset Control Service Client
typedef struct arc_vocc_cb
{
    /// Callback function called when a command has been completed
    arc_vocc_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when Audio Location or Offset State characteristic value has been received from a
    /// peer server device
    arc_vocc_cb_value cb_value;
    /// Callback function called when Audio Output Description characteristic value has been received from a peer
    /// server device
    arc_vocc_cb_description cb_description;
    /// Callback function called when a Volume Offset Control Service instance has been discovered in a peer server
    /// database
    arc_vocc_cb_bond_data cb_bond_data;
    /// Callback function called when a service changed indication is received from a Server device
    arc_vocc_cb_svc_changed cb_svc_changed;
} arc_vocc_cb_t;

/// @} ARC_VOCC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_ARC_VOCC)
/// @addtogroup ARC_VOCC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of Volume Offset Control Service Client module
 *
 * @param[in] p_cb          Pointer to set of callback functions
 * @param[in] pref_mtu      Preferred MTU\n
 *                          Values from 0 to 63 are equivalent to 64
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocc_configure(const arc_vocc_cb_t* p_cb, uint16_t pref_mtu);

/**
 ****************************************************************************************
 * @brief Discover Volume Offset Control Service instances in peer server device database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] nb_outputs    Number of outputs
 * @param[in] p_svc_hdl     Pointer to service handles
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocc_discover(uint8_t con_lid, uint8_t nb_outputs, prf_svc_t* p_svc_hdl);

/**
 ****************************************************************************************
 * @brief Set bonding information related to Volume Offset Control after connection with a peer device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] nb_outputs    Number of outputs
 * @param[in] p_vocs_info   Pointer to Volume Offset Control Service description structures
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocc_restore_bond_data(uint8_t con_lid, uint8_t nb_outputs, const arc_vocc_vocs_t* p_vocs_info);

/**
 ****************************************************************************************
 * @brief Set value for Audio Location characteristic of a peer server device output
 *
 * @param[in] con_lid       Connection local index
 * @param[in] output_lid    Output local index
 * @param[in] set_type      Set type
 * @param[in] value         Value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocc_set(uint8_t con_lid, uint8_t output_lid, uint8_t set_type, uint32_t value);

/**
 ****************************************************************************************
 * @brief Set Volume Offset for a given Audio Output
 *
 * @param[in] con_lid       Connection local index
 * @param[in] output_lid    Output local index
 * @param[in] offset        Offset value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vocc_set_offset(uint8_t con_lid, uint8_t output_lid, int16_t offset)
{
    return (arc_vocc_set(con_lid, output_lid, ARC_VOC_SET_TYPE_OFFSET, offset));
}

/**
 ****************************************************************************************
 * @brief Set Audio Location for a given Audio Output
 *
 * @param[in] con_lid       Connection local index
 * @param[in] output_lid    Output local index
 * @param[in] location      Audio Location
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vocc_set_location(uint8_t con_lid, uint8_t output_lid, uint8_t location)
{
    return (arc_vocc_set(con_lid, output_lid, ARC_VOC_SET_TYPE_LOCATION, location));
}

/**
 ****************************************************************************************
 * @brief Set value for Audio Output Description characteristic of a peer server device output
 *
 * @param[in] con_lid       Connection local index
 * @param[in] output_lid    Output local index
 * @param[in] desc_len      Audio output description length
 * @param[in] p_desc        Pointer to audio output description value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocc_set_description(uint8_t con_lid, uint8_t output_lid, uint16_t desc_len, const uint8_t* p_desc);

#if (GAF_DBG)
/**
 ****************************************************************************************
 * @brief Get value for either Offset State or Audio Location or Audio Output Description characteristic of a peer
 * server device output
 *
 * @param[in] con_lid       Connection local index
 * @param[in] output_lid    Output local index
 * @param[in] char_type     Characteristic type
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocc_get(uint8_t con_lid, uint8_t output_lid, uint8_t char_type);

/**
 ****************************************************************************************
 * @brief Set Client Characteristic Configuration value for either Offset State or Audio Location or Audio Output
 * Description characteristic of a peer server device output
 *
 * @param[in] con_lid           Connection local index
 * @param[in] output_lid        Output local index
 * @param[in] char_type         Characteristic type
 * @param[in] enable            Indicate if sending of notifications must be enabled or disabled
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocc_set_cfg(uint8_t con_lid, uint8_t output_lid, uint8_t char_type, uint8_t enable);
#endif //(GAF_DBG)

/// @} ARC_VOCC_FUNCTION
#endif //(GAF_ARC_VOCC)

#endif // ARC_VOCC_H_
