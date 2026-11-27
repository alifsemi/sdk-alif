/**
 ****************************************************************************************
 *
 * @file arc_vocs.h
 *
 * @brief Audio Rendering Control - Volume Offset Control Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_VOCS_H_
#define ARC_VOCS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_VOCS Volume Offset Control Service Server
 * @ingroup ARC_VOC
 * @brief Description of Volume Offset Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOCS_ENUM Enumerations
 * @ingroup ARC_VOCS
 * @brief Enumerations for Volume Offset Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOCS_NATIVE_API Native API
 * @ingroup ARC_VOCS
 * @brief Description of Native API for Volume Offset Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOCS_CALLBACK Callback Functions
 * @ingroup ARC_VOCS_NATIVE_API
 * @brief Description of callback functions for Volume Offset Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOCS_FUNCTION Functions
 * @ingroup ARC_VOCS_NATIVE_API
 * @brief Description of functions for Volume Offset Control Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc_voc.h"            // Audio Rendering Control - Volume Offset Control Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_VOCS_ENUM
/// @{

/// Output configuration bit field
enum arc_vocs_cfg_bf
{
    /// Set to 1 if Audio Location characteristic is notification-capable - Position
    ARC_VOCS_CFG_LOC_NTF_POS = 0,
    /// Set to 1 if Audio Location characteristic is notification-capable - Bit
    ARC_VOCS_CFG_LOC_NTF_BIT = CO_BIT(ARC_VOCS_CFG_LOC_NTF_POS),

    /// Set to 1 if Audio Location characteristic is writable - Position
    ARC_VOCS_CFG_LOC_WR_POS = 1,
    /// Set to 1 if Audio Location characteristic is writable - Bit
    ARC_VOCS_CFG_LOC_WR_BIT = CO_BIT(ARC_VOCS_CFG_LOC_WR_POS),

    /// Set to 1 if Audio Output Description characteristic is notification-capable - Position
    ARC_VOCS_CFG_DESC_NTF_POS = 2,
    /// Set to 1 if Audio Output Description characteristic is notification-capable - Bit
    ARC_VOCS_CFG_DESC_NTF_BIT = CO_BIT(ARC_VOCS_CFG_DESC_NTF_POS),

    /// Set to 1 if Audio Output Description characteristic is writable - Position
    ARC_VOCS_CFG_DESC_WR_POS = 3,
    /// Set to 1 if Audio Output Description characteristic is writable - Bit
    ARC_VOCS_CFG_DESC_WR_BIT = CO_BIT(ARC_VOCS_CFG_DESC_WR_POS),

    /// Set to 1 if it is required to check the Lock state - Position
    /// Meaningful only if Coordinated Set Identification Profile Set Member Role is supported
    ARC_VOCS_CFG_CHECK_LOCK_POS = 4,
    /// Set to 1 if it is required to check the Lock state - Bit
    /// Meaningful only if Coordinated Set Identification Profile Set Member Role is supported
    ARC_VOCS_CFG_CHECK_LOCK_BIT = CO_BIT(ARC_VOCS_CFG_CHECK_LOCK_POS),

};

/// @} ARC_VOCS_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ARC_VOCS_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when Offset State characteristic value has been updated
 *
 * @param[in] output_lid    Output local index
 * @param[in] offset        Offset value
 ****************************************************************************************
 */
typedef void (*arc_vocs_cb_offset)(uint8_t output_lid, int16_t offset);

/**
 ****************************************************************************************
 * @brief Callback function called when Client Characteristic Configuration of a notification-capable
 * characteristic has been updated by a peer client
 *
 * @param[in] output_lid    Output local index
 * @param[in] con_lid       Connection local index
 * @param[in] cli_cfg_bf    Client configuration bit field
 ****************************************************************************************
 */
typedef void (*arc_vocs_cb_bond_data)(uint8_t output_lid, uint8_t con_lid, uint8_t cli_cfg_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when value of Audio Output Description characteristic has been written so that it
 * can be confirmed by upper layers
 *
 * @param[in] output_lid    Output local index
 * @param[in] con_lid       Connection local index
 * @param[in] desc_len      Audio output description length
 * @param[in] p_desc        Pointer to audio output description value
 ****************************************************************************************
 */
typedef void (*arc_vocs_cb_description_req)(uint8_t output_lid, uint8_t con_lid, uint8_t desc_len,
                                            const char* p_desc);

/**
 ****************************************************************************************
 * @brief Callback function called when value of Audio Location characteristic has been written so that it can be
 * confirmed by upper layers
 *
 * @param[in] output_lid    Output local index
 * @param[in] con_lid       Connection local index
 * @param[in] location_bf   Audio location bit field (see #gaf_loc_bf enumeration)
 ****************************************************************************************
 */
typedef void (*arc_vocs_cb_location_req)(uint8_t output_lid, uint8_t con_lid, uint32_t location_bf);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for volume management (server)
typedef struct arc_vocs_cb
{
    /// Callback function called when Client Characteristic Configuration of a notification-capable
    /// characteristic has been updated by a peer client
    arc_vocs_cb_bond_data cb_bond_data;
    /// Callback function called when Offset State characteristic value has been updated
    arc_vocs_cb_offset cb_offset;
    /// Callback function called when value of Audio Output Description characteristic has been written so that it can
    /// be confirmed by upper layers
    arc_vocs_cb_description_req cb_description_req;
    /// Callback function called when value of Audio Location characteristic has been written so that it can be
    /// confirmed by upper layers
    arc_vocs_cb_location_req cb_location_req;
} arc_vocs_cb_t;

/// @} ARC_VOCS_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_ARC_VOCS)
/// @addtogroup ARC_VOCS_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure Volume Offset Control Server module
 *
 * @param[in] p_cb          Pointer to set of callback functions for communication with upper layers
 * @param[in] nb_outputs    Number of outputs
 * @param[in] pref_mtu      Preferred MTU
 *                          Values from 0 to 63 are equivalent to 64
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocs_configure(const arc_vocs_cb_t* p_cb, uint8_t nb_outputs, uint16_t pref_mtu);

/**
 ****************************************************************************************
 * @brief Add an output
 *
 * @param[in] desc_max_len      Audio Output Description maximum length
 * @param[in] cfg_bf            Configuration bit field (see #arc_vocs_cfg_bf enumeration)
 * @param[in] shdl              Required start handle
 * @param[out] p_output_lid     Pointer at which allocated output local index is returned
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocs_add(uint8_t desc_max_len, uint8_t cfg_bf, uint16_t shdl, uint8_t* p_output_lid);

/**
 ****************************************************************************************
 * @brief Set bonding information related to an input after establishment of a connection
 *
 * @param[in] output_lid    Output local index
 * @param[in] con_lid       Connection local index
 * @param[in] cli_cfg_bf    Client configuration bit field
 * @param[in] evt_bf        Event bit field
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocs_restore_bond_data(uint8_t output_lid, uint8_t con_lid, uint8_t cli_cfg_bf, uint8_t evt_bf);

/**
 ****************************************************************************************
 * @brief Set value of Audio Location or Offset State characteristic
 *
 * @param[in] output_lid    Output local index
 * @param[in] set_type      Set type (see #arc_voc_set_type enumeration)
 * @param[in] value         Value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocs_set(uint8_t output_lid, uint8_t set_type, uint32_t value);

/**
 ****************************************************************************************
 * @brief Set Volume Offset for a given Audio Output
 *
 * @param[in] output_lid    Output local index
 * @param[in] offset        Offset value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vocs_set_offset(uint8_t output_lid, int16_t offset)
{
    return (arc_vocs_set(output_lid, ARC_VOC_SET_TYPE_OFFSET, offset));
}

/**
 ****************************************************************************************
 * @brief Set Audio Location for a given Audio Output
 *
 * @param[in] output_lid    Output local index
 * @param[in] location      Audio location value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vocs_set_location(uint8_t output_lid, uint32_t location)
{
    return (arc_vocs_set(output_lid, ARC_VOC_SET_TYPE_LOCATION, location));
}

/**
 ****************************************************************************************
 * @brief Set value of Audio Output Description characteristic value
 *
 * @param[in] output_lid    Output local index
 * @param[in] desc_len      Audio output description length
 * @param[in] p_desc        Pointer to Audio output description value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vocs_set_description(uint8_t output_lid, uint8_t desc_len, const uint8_t *p_desc);

/**
 ****************************************************************************************
 * @brief Confirm or not value written for Audio Output Description characteristic
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] output_lid        Output local index
 * @param[in] desc_len          Audio output description length
 * @param[in] p_desc            Pointer to Audio output description value
 ****************************************************************************************
 */
void arc_vocs_set_description_cfm(bool accept, uint8_t output_lid, uint8_t desc_len, const uint8_t *p_desc);

/**
 ****************************************************************************************
 * @brief Confirm or not value written for Audio Location characteristic
 *
 * @param[in] accept            Indicate if request is accepted or not
 * @param[in] output_lid        Output local index
 * @param[in] location_bf       Audio location bit field (see #gaf_loc_bf enumeration)
 ****************************************************************************************
 */
void arc_vocs_set_location_cfm(bool accept, uint8_t output_lid, uint32_t location_bf);

/// @} ARC_VOCS_FUNCTION
#endif //(GAF_ARC_VOCS)

#endif // ARC_VOCS_H_
