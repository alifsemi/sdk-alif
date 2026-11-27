/**
 ****************************************************************************************
 *
 * @file csisc.h
 *
 * @brief Coordinated Set Identification Service Set Coordinator - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CSISC_H_
#define CSISC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CSISC Coordinated Set Identification Profile Set Coordinator
 * @ingroup CSIP_API
 * @brief Description of Coordinated Set Identification Profile Set Coordinator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSISC_ENUM Enumerations
 * @ingroup CSISC
 * @brief Enumerations for Coordinated Set Identification Profile Set Coordinator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSISC_STRUCT Structures
 * @ingroup CSISC
 * @brief Structures for Coordinated Set Identification Profile Set Coordinator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSISC_NATIVE_API Native API
 * @ingroup CSISC
 * @brief Description of Native API for Coordinated Set Identification Profile Set Coordinator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSISC_CALLBACK Callback Functions
 * @ingroup CSISC_NATIVE_API
 * @brief Description of callback functions for Coordinated Set Identification Profile Set Coordinator module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSISC_FUNCTION Functions
 * @ingroup CSISC_NATIVE_API
 * @brief Description of functions for Coordinated Set Identification Profile Set Coordinator module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "csis.h"            // Coordinated Set Identification Service Definitions

#if (BLE_CSIS_COORD)

#include "co_math.h"         // Common Math Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup CSISC_ENUM
/// @{

/// List of CSISC_CMD command codes
enum csisc_cmd_codes
{
    CSISC_RESOLVE = 0x0000,
    CSISC_DISCOVER = 0x0001,
    CSISC_LOCK = 0x0002,
    CSISC_GET = 0x0003,
    CSISC_SET_CFG = 0x0005,
};

/// @} CSISC_ENUM

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// @addtogroup CSISC_STRUCT
/// @{

/// Configuration structure
typedef struct csisc_cfg
{
    /// Number of SIRK values that can be stored
    uint8_t nb_sirk;
} csisc_cfg_t;

/// Content description structure for Coordinated Set Identification Service
typedef struct csisc_csis_info
{
    /// Service description
    prf_svc_t svc_info;
    /// Characteristics description
    prf_char_t char_info[CSIS_CHAR_TYPE_MAX];
    /// Descriptors description
    prf_desc_t desc_info[CSIS_DESC_TYPE_MAX];
} csisc_csis_info_t;

/// @} CSISC_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup CSISC_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when an instance of the Coordinated Set Identification
 * Service has been found in Server device database
 *
 * @param[in] con_lid       Connection local index
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] p_csis_info   Pointer to Content description structure for the service
 ****************************************************************************************
 */
typedef void (*csisc_cb_bond_data)(uint8_t con_lid, uint8_t set_lid, const csisc_csis_info_t* p_csis_info);

/**
 ****************************************************************************************
 * @brief Callback function called when a SIRK value has been received
 *
 * @param[in] con_lid       Connection local index
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] key_lid       Key local index
 * @param[in] p_sirk        Pointer to SIRK value
 ****************************************************************************************
 */
typedef void (*csisc_cb_sirk)(uint8_t con_lid, uint8_t set_lid, uint8_t key_lid,
                              const csis_sirk_t* p_sirk);

/**
 ****************************************************************************************
 * @brief Callback function called when value for either Coordinated Set Size characteristic
 * or Set Member Lock characteristic or Set Member Rank characteristic has been received
 *
 * @param[in] con_lid       Connection local index
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] char_type     Characteristic type
 * @param[in] val           Value
 ****************************************************************************************
 */
typedef void (*csisc_cb_info)(uint8_t con_lid, uint8_t set_lid, uint8_t char_type, uint8_t val);

/**
 ****************************************************************************************
 * @brief Callback function called when a command has been completed
 *
 * @param[in] cmd_code      Command code
 * @param[in] status        Status
 * @param[in] lid           Connection local index or Key local index
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] char_type     Characteristic type for CSISC_GET command
 *                          and CSISC_SET_CFG commands
 ****************************************************************************************
 */
typedef void (*csisc_cb_cmp_evt)(uint16_t cmd_code, uint16_t status, uint8_t lid,
                                 uint8_t set_lid, uint8_t char_type);

/**
 ****************************************************************************************
 * @brief Callback function called when a service changed indication is received from a Set Member device
 *
 * @param[in] con_lid           Connection local index
 ****************************************************************************************
 */
typedef void (*csisc_cb_svc_changed)(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when upper layer is requested for sharing LTK
 *
 * @param[in] con_lid           Connection local index
 * @param[in] set_lid           Coordinated Set local index
 ****************************************************************************************
 */
typedef void (*csisc_cb_ltk_req)(uint8_t con_lid, uint8_t set_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Coordinated Set Identification Service Set Coordinator
typedef struct csisc_cb
{
    /// Callback function called when an instance of the Coordinated Set Identification
    /// Service has been found in Server device database
    csisc_cb_bond_data cb_bond_data;
    /// Callback function called when a SIRK value has been received
    csisc_cb_sirk cb_sirk;
    /// Callback function called when value for either Coordinated Set Size characteristic or
    /// Set Member Lock characteristic or Set Member Rank characteristic has been received
    csisc_cb_info cb_info;
    /// Callback function called when a command has been completed
    csisc_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when a service changed indication is received from a Set Member device
    csisc_cb_svc_changed cb_svc_changed;
    /// Callback function called when upper layer is requested for sharing LTK
    csisc_cb_ltk_req cb_ltk_req;
} csisc_cb_t;

/// @} CSISC_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup CSISC_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Resolve a RSI value
 *
 * @param[in] p_rsi        Pointer to RSI value
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t csisc_resolve(const csis_rsi_t* p_rsi);

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
 * @return An error status
 ****************************************************************************************
 */
uint16_t csisc_discover(uint8_t con_lid, uint8_t nb_sets_max, uint16_t shdl, uint16_t ehdl);

/**
 ****************************************************************************************
 * @brief Lock or unlock a Set Member device for execution of a procedure
 *
 * @param[in] con_lid       Connection local index
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] lock          Lock state
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t csisc_lock(uint8_t con_lid, uint8_t set_lid, uint8_t lock);

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
 * @return An error status
 ****************************************************************************************
 */
uint16_t csisc_get(uint8_t con_lid, uint8_t set_lid, uint8_t char_type);

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
 * @return An error status
 ****************************************************************************************
 */
uint16_t csisc_set_cfg(uint8_t con_lid, uint8_t set_lid, uint8_t char_type, uint8_t enable);

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
 * @return An error status
 ****************************************************************************************
 */
uint16_t csisc_restore_bond_data(uint8_t con_lid, uint8_t nb_sets, const csisc_csis_info_t* p_csis_info);

/**
 ****************************************************************************************
 * @brief Add a SIRK value
 *
 * @param[in] p_sirk        Pointer to SIRK value
 * @param[out] p_key_lid    Pointer at which allocated Key local index will be returned
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t csisc_add_sirk(const csis_sirk_t* p_sirk, uint8_t* p_key_lid);

/**
 ****************************************************************************************
 * @brief Remove a SIRK value
 *
 * @param[in] key_lid       Key local index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t csisc_remove_sirk(uint8_t key_lid);

/**
 ****************************************************************************************
 * @brief Request LTK from upper layer
 *
 * @param[in] p_ltk             Pointer to LTK
 ****************************************************************************************
 */
void csisc_ltk_cfm(const uint8_t* p_ltk);

/// @} CSISC_FUNCTION

#endif //(BLE_CSIS_COORD)

#endif // CSISC_H_
