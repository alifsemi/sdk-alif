/**
 ****************************************************************************************
 *
 * @file csism.h
 *
 * @brief Coordinated Set Identification Service Set Member - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CSISM_H_
#define CSISM_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CSISM Coordinated Set Identification Profile Set Member
 * @ingroup CSIP_API
 * @brief Description of Coordinated Set Identification Profile Set Member module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSISM_ENUM Enumerations
 * @ingroup CSISM
 * @brief Enumerations for Coordinated Set Identification Profile Set Member module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSISM_STRUCT Structures
 * @ingroup CSISM
 * @brief Structures for Coordinated Set Identification Profile Set Member module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSISM_NATIVE_API Native API
 * @ingroup CSISM
 * @brief Description of Native API for Coordinated Set Identification Profile Set Member module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSISM_CALLBACK Callback Functions
 * @ingroup CSISM_NATIVE_API
 * @brief Description of callback functions for Coordinated Set Identification Profile Set Member module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CSISM_FUNCTION Functions
 * @ingroup CSISM_NATIVE_API
 * @brief Description of functions for Coordinated Set Identification Profile Set Member module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "csis.h"            // Coordinated Set Identification Service Definitions

#if (BLE_CSIS_MEMBER)

#include "co_math.h"         // Common Math Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup CSISM_ENUM
/// @{

/// List of CSISM_CMD command codes
enum csism_cmd_codes
{
    /// Add Coordinated Set
    CSISM_ADD = 0x0000,
    /// Set Set Identity Resolving Key
    CSISM_SET_SIRK = 0x0001,
    /// Update Resolvable Set Identifier
    CSISM_UPDATE_RSI = 0x0002,
};

/// Configuration bit field meaning for CSISM ADD command
enum csism_add_cfg_bf
{
    /// Indicate if sending of notifications is supported or not for Set Identity Resolving Key characteristic - Position
    CSISM_ADD_CFG_SIRK_NTF_POS = 0,
    /// Indicate if sending of notifications is supported or not for Set Identity Resolving Key characteristic - Bit
    CSISM_ADD_CFG_SIRK_NTF_BIT = CO_BIT(CSISM_ADD_CFG_SIRK_NTF_POS),

    /// Indicate if Coordinated Set Size characteristic is supported - Position
    CSISM_ADD_CFG_SIZE_POS = 1,
    /// Indicate if Coordinated Set Size characteristic is supported - Bit
    CSISM_ADD_CFG_SIZE_BIT = CO_BIT(CSISM_ADD_CFG_SIZE_POS),

    /// Indicate if sending of notifications is supported or not for Coordinated Set Size characteristic - Position
    CSISM_ADD_CFG_SIZE_NTF_POS = 2,
    /// Indicate if sending of notifications is supported or not for Coordinated Set Size characteristic - Bit
    CSISM_ADD_CFG_SIZE_NTF_BIT = CO_BIT(CSISM_ADD_CFG_SIZE_NTF_POS),

    /// Indicate if Set Member Lock characteristic is supported - Position
    CSISM_ADD_CFG_LOCK_POS = 3,
    /// Indicate if Set Member Lock characteristic is supported - Bit
    CSISM_ADD_CFG_LOCK_BIT = CO_BIT(CSISM_ADD_CFG_LOCK_POS),

    /// Indicate if SIRK can be provided only using an OOB method - Position
    CSISM_ADD_CFG_SIRK_OOB_ONLY_POS = 4,
    /// Indicate if SIRK can be provided only using an OOB method - Bit
    CSISM_ADD_CFG_SIRK_OOB_ONLY_BIT = CO_BIT(CSISM_ADD_CFG_SIRK_OOB_ONLY_POS),

    /// Indicate if Set Member Rank characteristic is supported - Position
    CSISM_ADD_CFG_RANK_POS = 5,
    /// Indicate if Set Member Rank characteristic is supported - Bit
    CSISM_ADD_CFG_RANK_BIT = CO_BIT(CSISM_ADD_CFG_RANK_POS),

    /// Indicate if SIRK is encrypted (=1) or not - Position
    CSISM_ADD_CFG_SIRK_ENCRYPT_POS = 6,
    /// Indicate if SIRK is encrypted (=1) or not - Bit
    CSISM_ADD_CFG_SIRK_ENCRYPT_BIT = CO_BIT(CSISM_ADD_CFG_SIRK_ENCRYPT_POS),
};

/// Unlock reason values
enum csism_unlock_reason
{
    /// Lock released due to peer request
    CSISM_UNLOCK_REASON_PEER_REQ = 0,
    /// Lock released due to timeout
    CSISM_UNLOCK_REASON_TIMEOUT,
};

/// @} CSISM_ENUM

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// @addtogroup CSISM_STRUCT
/// @{

/// Configuration structure
typedef struct csism_cfg
{
    /// Number of Coordinated Sets the device may belongs to
    uint8_t nb_sets;
} csism_cfg_t;

/// @} CSISM_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup CSISM_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when lock state for a Coordinated Set has been updated
 *
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] lock          New lock state
 * @param[in] con_lid       Connection local index of connection for which Coordinated Set
 * has been locked or was locked
 * @param[in] reason        Reason why Coordinated Set is not locked anymore
 ****************************************************************************************
 */
typedef void (*csism_cb_lock)(uint8_t set_lid, uint8_t lock, uint8_t con_lid, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Callback function called when lock state for a Coordinated Set has been updated
 *
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] con_lid       Connection local index of connection for which Coordinated Set
 * has been locked or was locked
 * @param[in] cli_cfg_bf    Client configuration bit field
 ****************************************************************************************
 */
typedef void (*csism_cb_bond_data)(uint8_t set_lid, uint8_t con_lid, uint8_t cli_cfg_bf);

/**
 ****************************************************************************************
 * @brief Callback function called when upper layer is requested for sharing LTK
 *
 * @param[in] set_lid           Coordinated Set local index
 * @param[in] con_lid           Connection local index
 ****************************************************************************************
 */
typedef void (*csism_cb_ltk_req)(uint8_t set_lid, uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Callback function called when a new RSI has been generated
 *
 * @param[in] set_lid           Coordinated Set local index
 * @param[in] p_rsi             Pointer to generated RSI value
 ****************************************************************************************
 */
typedef void (*csism_cb_rsi)(uint8_t set_lid, const csis_rsi_t* p_rsi);

/**
 ****************************************************************************************
 * @brief Callback function called when a command has been completed
 *
 * @param[in] cmd_code      Command code
 * @param[in] status        Status
 * @param[in] set_lid       Coordinated Set local index
 ****************************************************************************************
 */
typedef void (*csism_cb_cmp_evt)(uint16_t cmd_code, uint16_t status, uint8_t set_lid);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Coordinated Set Identification Service Set Member
typedef struct csism_cb
{
    /// Callback function called when lock state for a Coordinated Set has been updated
    csism_cb_lock cb_lock;
    /// Callback function called when client configuration for an instance of the Coordinated Set
    /// Identification Service has been updated
    csism_cb_bond_data cb_bond_data;
    /// Callback function called when upper layer is requested for sharing LTK
    csism_cb_ltk_req cb_ltk_req;
    /// Callback function called when a new RSI has been generated
    csism_cb_rsi cb_rsi;
    /// Callback function called when a command has been completed
    csism_cb_cmp_evt cb_cmp_evt;
} csism_cb_t;

/// @} CSISM_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

/// @addtogroup CSISM_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Add an instance of the Coordinated Set Identification Service. First RSI value for the provided
 * SIRK is generated and reported using cb_rsi callback function.
 *
 * @param[in] cfg_bf            Configuration bit field
 * @param[in] size              Number of members in the added Coordinated Set\n
 *                              Meaningful only if #CSISM_ADD_CFG_SIZE_BIT bit set in cfg_bf
 * @param[in] rank              Rank\n
 *                              Meaningful only if #CSISM_ADD_CFG_RANK_BIT or #CSISM_ADD_CFG_LOCK_BIT bit set in
 *                              cfg_bf\n
 * @param[in] lock_timeout_s    Lock timeout duration in seconds\n
 *                              Meaningful only if #CSISM_ADD_CFG_LOCK_BIT bit set in cfg_bf
 * @param[in] shdl              Required start handle
 *                              If set to GATT_INVALID_HDL, the start handle will be automatically chosen
 * @param[in] p_sirk            Pointer to SIRK value. Shall not be NULL and shall not be a all-zero value
 *
 * @return An error status\n
 * cb_cmp_evt callback function is called once procedure is finished and first RSI value has been generated.
 * The upper layer should wait for reception of this event before starting any advertising activity.
 ****************************************************************************************
 */
uint16_t csism_add(uint8_t cfg_bf, uint8_t size, uint8_t rank, uint8_t lock_timeout_s,
                   uint16_t shdl, const csis_sirk_t* p_sirk);

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
 * @return An error status
 ****************************************************************************************
 */
uint16_t csism_restore_bond_data(uint8_t con_lid, uint8_t set_lid, bool is_locked,
                                 uint8_t cli_cfg_bf, uint8_t evt_cfg_bf);

/**
 ****************************************************************************************
 * @brief Update SIRK value for a Coordinated Set
 *
 * @param[in] set_lid           Coordinated Set local index
 * @param[in] p_sirk            Pointer to SIRK value
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t csism_set_sirk(uint8_t set_lid, const csis_sirk_t* p_sirk);

/**
 ****************************************************************************************
 * @brief Update RSI value for a Coordinated Set
 *
 * @param[in] set_lid           Coordinated Set local index
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t csism_update_rsi(uint8_t set_lid);

/**
 ****************************************************************************************
 * @brief Update number of devices belonging to a Coordinated Set
 *
 * @param[in] set_lid           Coordinated Set local index
 * @param[in] size              Number of members in the Coordinated Set
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t csism_set_size(uint8_t set_lid, uint8_t size);

/**
 ****************************************************************************************
 * @brief Request LTK from upper layer
 *
 * @param[in] p_ltk             Pointer to LTK
 ****************************************************************************************
 */
void csism_ltk_cfm(const uint8_t* p_ltk);

/**
 ****************************************************************************************
 * @brief Get start handle of a Coordinated Set Identification Service instance
 *
 * @param[in] set_lid           Coordinated Set local index
 * @param[in] p_shdl            Pointer at which request start handle value will be returned
 *
 * @return An error status
 ****************************************************************************************
 */
uint16_t csism_get_shdl(uint8_t set_lid, uint16_t* p_shdl);

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
bool csism_is_proc_allowed(uint8_t con_lid, uint8_t set_lid);

/// @} CSISM_FUNCTION

#endif //(BLE_CSIS_MEMBER)

#endif // CSISM_H_
