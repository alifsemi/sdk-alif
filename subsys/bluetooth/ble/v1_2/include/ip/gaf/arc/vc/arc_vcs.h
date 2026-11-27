/**
 ****************************************************************************************
 *
 * @file arc_vcs.h
 *
 * @brief Audio Rendering Control - Volume Control Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_VCS_H_
#define ARC_VCS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_VCS Volume Control Service Server
 * @ingroup ARC_VC
 * @brief Description of Volume Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VCS_ENUM Enumerations
 * @ingroup ARC_VCS
 * @brief Enumerations for Volume Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VCS_NATIVE_API Native API
 * @ingroup ARC_VCS
 * @brief Description of Native API for Volume Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VCS_CALLBACK Callback Functions
 * @ingroup ARC_VCS_NATIVE_API
 * @brief Description of callback functions for Volume Control Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VCS_FUNCTION Functions
 * @ingroup ARC_VCS_NATIVE_API
 * @brief Description of functions for Volume Control Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc_vc.h"            // Volume Control Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_VCS_ENUM
/// @{

/// Volume Control Configuration bit field
enum arc_vcs_cfg_bf
{
    /// Indicate if sending of notifications is supported (= 1) for the Volume Flags
    /// characteristic
    ARC_VCS_CFG_FLAGS_NTF_POS = 0,
    ARC_VCS_CFG_FLAGS_NTF_BIT = CO_BIT(ARC_VCS_CFG_FLAGS_NTF_POS),
    /// Indicate if it is required (= 1) to check the Lock state
    /// Meaningful only if Coordinated Set Identification Profile Set Member Role is supported
    ARC_VCS_CFG_CHECK_LOCK_POS = 1,
    ARC_VCS_CFG_CHECK_LOCK_BIT = CO_BIT(ARC_VCS_CFG_CHECK_LOCK_POS),
};

/// @} ARC_VCS_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ARC_VCS_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when Client Characteristic Configuration of a notification-capable
 * characteristic has been updated by a peer client
 *
 * @param[in] con_lid       Connection local index
 * @param[in] cli_cfg_bf    Client configuration bit field
 ****************************************************************************************
 */
typedef void (*arc_vcs_cb_bond_data)(uint8_t con_lid, uint8_t cli_cfg_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when Volume State characteristic value has been updated
 *
 * @param[in] volume        Volume Setting value
 * @param[in] mute          Mute value
 * @param[in] local         Indicate if volume update has been triggered locally or not
 ****************************************************************************************
 */
typedef void (*arc_vcs_cb_volume)(uint8_t volume, uint8_t mute, bool local);

/**
 ****************************************************************************************
 * @brief Callback function called when Volume Flags characteristic value has been updated
 *
 * @param[in] flags         Volume Flags value
 ****************************************************************************************
 */
typedef void (*arc_vcs_cb_flags)(uint8_t flags);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for volume management (server)
typedef struct arc_vcs_cb
{
    /// Callback function called when Client Characteristic Configuration of a notification-capable characteristic has
    /// been updated by a peer client
    arc_vcs_cb_bond_data cb_bond_data;
    /// Callback function called when Volume State characteristic value has been updated
    arc_vcs_cb_volume cb_volume;
    /// Callback function called when Volume Flags characteristic value has been updated
    arc_vcs_cb_flags cb_flags;
} arc_vcs_cb_t;

/// @} ARC_VCS_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_ARC_VCS)
/// @addtogroup ARC_VCS_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure Volume Control Server module
 *
 * @param[in] p_cb          Pointer to set of callback functions
 * @param[in] step_size     Step Size value
 * @param[in] flags         Volume Flags value
 * @param[in] volume        Initial Volume Setting value
 * @param[in] mute          Initial Mute value
 * @param[in] shdl          Required start handle
 *                          If set to GATT_INVALID_HDL the start handle will be automatically chosen
 * @param[in] cfg_bf        Configuration bit field (see #arc_vcs_cfg_bf enumeration)
 * @param[in] nb_inputs     Number of Audio Input Control Service instances to include in the service
 * @param[in] p_input_lid   Pointer to list of local indices for instances of Audio Input Control Service to include
 *                          in the service
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vcs_configure(const arc_vcs_cb_t* p_cb, uint8_t step_size, uint8_t flags, uint8_t volume,
                           uint8_t mute, uint16_t shdl, uint8_t cfg_bf, uint8_t nb_inputs,
                           uint8_t* p_input_lid);

/**
 ****************************************************************************************
 * @brief Set bonding information related to Volume Control after connection with a peer device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] cli_cfg_bf    Client configuration bit field
 * @param[in] evt_cfg_bf    Event configuration bit field
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vcs_restore_bond_data(uint8_t con_lid, uint8_t cli_cfg_bf, uint8_t evt_cfg_bf);

/**
 ****************************************************************************************
 * @brief Control Volume State characteristic value
 *
 * @param[in] opcode        Operation code
 * @param[in] volume        Volume
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vcs_control(uint8_t opcode, uint8_t volume);

/**
 ****************************************************************************************
 * @brief Increase volume
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcs_volume_increase(void)
{
    return (arc_vcs_control(ARC_VC_OPCODE_VOL_UP, 0));
}

/**
 ****************************************************************************************
 * @brief Decrease volume
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcs_volume_decrease(void)
{
    return (arc_vcs_control(ARC_VC_OPCODE_VOL_DOWN, 0));
}

/**
 ****************************************************************************************
 * @brief Set volume
 *
 * @param[in] volume        Volume
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcs_volume_set(uint8_t volume)
{
    return (arc_vcs_control(ARC_VC_OPCODE_VOL_SET_ABS, volume));
}

/**
 ****************************************************************************************
 * @brief Mute
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcs_mute(void)
{
    return (arc_vcs_control(ARC_VC_OPCODE_VOL_MUTE, 0));
}

/**
 ****************************************************************************************
 * @brief Unmute
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcs_unmute(void)
{
    return (arc_vcs_control(ARC_VC_OPCODE_VOL_UNMUTE, 0));
}

/**
 ****************************************************************************************
 * @brief Increase volume and unmute
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcs_volume_increase_unmute(void)
{
    return (arc_vcs_control(ARC_VC_OPCODE_VOL_UP_UNMUTE, 0));
}

/**
 ****************************************************************************************
 * @brief Decrease volume and unmute
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcs_volume_decrease_unmute(void)
{
    return (arc_vcs_control(ARC_VC_OPCODE_VOL_DOWN_UNMUTE, 0));
}

/// @} ARC_VCS_FUNCTION
#endif //(GAF_ARC_VCS)

#endif // ARC_VCS_H_
