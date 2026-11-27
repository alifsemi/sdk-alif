/**
 ****************************************************************************************
 *
 * @file arc_vcc.h
 *
 * @brief Audio Rendering Control - Volume Control Client - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_VCC_H_
#define ARC_VCC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_VCC Volume Control Service Client
 * @ingroup ARC_VC
 * @brief Description of Volume Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VCC_ENUM Enumerations
 * @ingroup ARC_VCC
 * @brief Enumerations for Volume Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VCC_STRUCT Structures
 * @ingroup ARC_VCC
 * @brief Structures for Volume Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VCC_NATIVE_API Native API
 * @ingroup ARC_VCC
 * @brief Description of Native API for Volume Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VCC_CALLBACK Callback Functions
 * @ingroup ARC_VCC_NATIVE_API
 * @brief Description of callback functions for Volume Control Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VCC_FUNCTION Functions
 * @ingroup ARC_VCC_NATIVE_API
 * @brief Description of functions for Volume Control Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"               // GAF Defines
#include "arc_vc.h"            // Audio Rendering Control - Volume Control Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_VCC_ENUM
/// @{

/// List of command type values for Volume Control Service Client module
enum arc_vcc_cmd_type
{
    /// Discover
    ARC_VCC_CMD_TYPE_DISCOVER = 0,
    /// Control
    ARC_VCC_CMD_TYPE_CONTROL,
    /// Get
    ARC_VCC_CMD_TYPE_GET,
    /// Set Configuration
    ARC_VCC_CMD_TYPE_SET_CFG = 4,
};

/// @} ARC_VCC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_VCC_STRUCT
/// @{

/// Volume Control Service content description structure
typedef struct arc_vcc_vcs
{
    /// Service description
    prf_svc_t svc_info;
    /// Characteristics description
    prf_char_t char_info[ARC_VC_CHAR_TYPE_MAX];
    /// Descriptors description
    prf_desc_t desc_info[ARC_VC_DESC_TYPE_MAX];
} arc_vcc_vcs_t;

/// @} ARC_VCC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ARC_VCC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called each time a command has been completed
 *
 * @param[in] cmd_type      Command type (see #arc_vcc_cmd_type enumeration)
 * @param[in] status        Status
 * @param[in] con_lid       Connection local index
 * @param[in] param         Additional parameter
 ****************************************************************************************
 */
typedef void (*arc_vcc_cb_cmp_evt)(uint8_t cmd_type, uint16_t status, uint8_t con_lid, uint8_t param);

/**
 ****************************************************************************************
 * @brief Callback function called when Volume Control Service instance has been discovered in a peer server database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] p_svc_info    Pointer to service content description
 ****************************************************************************************
 */
typedef void (*arc_vcc_cb_bond_data)(uint8_t con_lid, arc_vcc_vcs_t* p_svc_info);

/**
 ****************************************************************************************
 * @brief Callback function called when Volume State has been received from a peer server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] volume        Volume Setting value
 * @param[in] mute          Mute value
 ****************************************************************************************
 */
typedef void (*arc_vcc_cb_volume)(uint8_t con_lid, uint8_t volume, uint8_t mute);

/**
 ****************************************************************************************
 * @brief Callback function called when Volume Flags has been received from a peer server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] flags         Volume Flags characteristic value
 ****************************************************************************************
 */
typedef void (*arc_vcc_cb_flags)(uint8_t con_lid, uint8_t flags);

/**
 ****************************************************************************************
 * @brief Callback function called when a Volume Offset Control Service or an Audio Input Control Service instance has
 * been discovered in a peer server
 *
 * @param[in] con_lid       Connection local index
 * @param[in] uuid          UUID
 * @param[in] shdl          Start handle
 * @param[in] ehdl          End handle
 ****************************************************************************************
 */
typedef void (*arc_vcc_cb_included_svc)(uint8_t con_lid, uint16_t uuid, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication is received from a Service device
 *
 * @param[in] con_lid           Connection local index
 ****************************************************************************************
 */
typedef void (*arc_vcc_cb_svc_changed)(uint8_t con_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for volume management (client)
typedef struct arc_vcc_cb
{
    /// Callback function called when a command has been completed
    arc_vcc_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when Volume State has been received from a peer server device
    arc_vcc_cb_volume cb_volume;
    /// Callback function called when Volume Flags have been received from a peer server device
    arc_vcc_cb_flags cb_flags;
    /// Callback function called when Volume Control Service instance has been discovered in a peer server database
    arc_vcc_cb_bond_data cb_bond_data;
    /// Callback function called when a Volume Offset Control Service or an Audio Input Control Service instance has
    /// been discovered in a peer server
    arc_vcc_cb_included_svc cb_included_svc;
    /// Callback function called when a service changed indication is received from a Service device
    arc_vcc_cb_svc_changed cb_svc_changed;
} arc_vcc_cb_t;

/// @} ARC_VCC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_ARC_VCC)
/// @addtogroup ARC_VCC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Create and configure Volume Control Client module
 *
 * @param[in] p_cb      Pointer to set of callback functions for communications with upper layers
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vcc_configure(const arc_vcc_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Initiate discovery of Volume Control Service in peer server device database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] shdl          Start handle for the discovery. Set GATT_INVALID_HDL if not provided
 * @param[in] ehdl          End handle for the discovery. Set GATT_INVALID_HDL if not provided
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vcc_discover(uint8_t con_lid, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Set bonding information related to Volume Control after connection with a peer device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] p_vcs_info    Pointer to Volume Control Service description structures
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vcc_restore_bond_data(uint8_t con_lid, const arc_vcc_vcs_t* p_vcs_info);

/**
 ****************************************************************************************
 * @brief Control peer server device's Volume State values
 *
 * @param[in] con_lid       Connection local index
 * @param[in] opcode        Operation code
 * @param[in] volume        Volume
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vcc_control(uint8_t con_lid, uint8_t opcode, uint8_t volume);

/**
 ****************************************************************************************
 * @brief Increase volume (by step size locally decided by Server)
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcc_volume_increase(uint8_t con_lid)
{
    return (arc_vcc_control(con_lid, ARC_VC_OPCODE_VOL_UP, 0));
}

/**
 ****************************************************************************************
 * @brief Decrease volume (by step size locally decided by Server)
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcc_volume_decrease(uint8_t con_lid)
{
    return (arc_vcc_control(con_lid, ARC_VC_OPCODE_VOL_DOWN, 0));
}

/**
 ****************************************************************************************
 * @brief Set volume
 *
 * @param[in] con_lid       Connection local index
 * @param[in] volume        Volume
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcc_volume_set(uint8_t con_lid, uint8_t volume)
{
    return (arc_vcc_control(con_lid, ARC_VC_OPCODE_VOL_SET_ABS, volume));
}

/**
 ****************************************************************************************
 * @brief Mute
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcc_mute(uint8_t con_lid)
{
    return (arc_vcc_control(con_lid, ARC_VC_OPCODE_VOL_MUTE, 0));
}


/**
 ****************************************************************************************
 * @brief Unmute
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcc_unmute(uint8_t con_lid)
{
    return (arc_vcc_control(con_lid, ARC_VC_OPCODE_VOL_UNMUTE, 0));
}

/**
 ****************************************************************************************
 * @brief Increase volume (by step size locally decided by Server) and unmute
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcc_volume_increase_unmute(uint8_t con_lid)
{
    return (arc_vcc_control(con_lid, ARC_VC_OPCODE_VOL_UP_UNMUTE, 0));
}

/**
 ****************************************************************************************
 * @brief Decrease volume (by step size locally decided by Server) and unmute
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t arc_vcc_volume_decrease_unmute(uint8_t con_lid)
{
    return (arc_vcc_control(con_lid, ARC_VC_OPCODE_VOL_DOWN_UNMUTE, 0));
}

#if (GAF_DBG)
/**
 ****************************************************************************************
 * @brief Get value for either Volume State or Volume Flags characteristic of a peer server device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] char_type     Characteristic type
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vcc_get(uint8_t con_lid, uint8_t char_type);

/**
 ****************************************************************************************
 * @brief Set Client Characteristic Configuration value for either Volume State or Volume Flags characteristic of a
 * peer server device
 *
 * @param[in] con_lid           Connection local index
 * @param[in] char_type         Characteristic type
 * @param[in] enable            Indicate if sending of notifications must be enabled or disabled
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t arc_vcc_set_cfg(uint8_t con_lid, uint8_t char_type, uint8_t enable);
#endif //(GAF_DBG)

/// @} ARC_VCC_FUNCTION
#endif //(GAF_ARC_VCC)

#endif // ARC_VCC_H_
