/**
 ****************************************************************************************
 *
 * @file bmsc.h
 *
 * @brief Header file - Bond Management Service Client - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BMSC_H_
#define BMSC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BMSC_API Bond Management Service Client
 * @ingroup BMS_API
 * @brief Description of API for Bond Management Service Client
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BMSC_API_COMMON Common
 * @ingroup BMSC_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BMSC_API_NATIVE Native API
 * @ingroup BMSC_API
 * @brief Description of Native API for Bond Management Service Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bms.h"

/// @addtogroup BMSC_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Command type values
enum bmsc_cmd_type
{
    /// Discover
    BMSC_CMD_DISCOVER = 0,
    /// Delete
    BMSC_CMD_DELETE,
};

/// Bond Management Service characteristics
enum bmsc_char_type
{
    /// Bond Management Control Point characteristic
    BMSC_CHAR_CONTROL_POINT = 0,
    /// Bond Management Feature characteristic
    BMSC_CHAR_FEATURES,

    /// Maximum number of characteristic composing the service
    BMSC_CHAR_MAX,
};

/// Bond Management Service descriptors
enum bmsc_desc_type
{
    /// Client Characteristic Configuration descriptor for Bond Management Feature characteristic
    BMSC_DESC_FEATURES_CCC = 0,
    /// Characteristic Extended Properties descriptor for Bond Management Control Point characteristic
    BMSC_DESC_CP_CEP,

    /// Maximum number of descriptors that may be present in the service
    BMSC_DESC_MAX,
};

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// Structure containing the characteristics handles, value handles and descriptors
typedef struct bmsc_bms_content
{
    /// Service info
    prf_svc_t svc;
    /// Characteristic information (see #bmsc_char_type for indices)\n
    prf_char_t chars[BMSC_CHAR_MAX];
    /// Descriptor information\n
    prf_desc_t descs[BMSC_DESC_MAX];
} bmsc_bms_content_t;

/// @} BMSC_API_COMMON

/// @addtogroup BMSC_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for Bond Management Service Client
typedef struct bmsc_cb
{
    /**
     ****************************************************************************************
     * @brief Inform upper layer about completion of a command
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of procedure execution (see #hl_err enumeration)
     * @param[in] cmd_type      Command type (see #bmsc_cmd_type enumeration)
     ****************************************************************************************
     */
    void (*cb_cmp_evt)(uint8_t conidx, uint16_t status, uint8_t cmd_type);

    /**
     ****************************************************************************************
     * @brief Provide bond data to upper layer.\n
     * The bond data must be stored and provided after establishment of a new connection with the server.
     *
     * @param[in] conidx        Connection index
     * @param[in] p_bms         Pointer to structure describing Bond Management Service discovered in server's
     * attribute database
     ****************************************************************************************
     */
    void (*cb_bond_data)(uint8_t conidx, const bmsc_bms_content_t* p_bms);

    /**
     ****************************************************************************************
     * @brief Inform upper layer about Bond Management Feature supported by the server
     *
     * @param[in] conidx            Connection index
     * @param[in] length            Number of bytes composing the received Features bit field value
     * @param[in] p_features_bf     Pointer to Features bit field value (see #bms_features_bf enumeration)
     ****************************************************************************************
     */
    void (*cb_features)(uint8_t conidx, uint8_t length, const uint8_t* p_features_bf);
} bmsc_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Add support of Bond Management Service as Client
 *
 * @param[in] p_cbs                 Pointer to set of callback functions\n
 *                                  Shall not be NULL
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t bmsc_add(const bmsc_cb_t* p_cbs);

/**
 ****************************************************************************************
 * @brief Discover Bond Management Service
 * cb_bond_data callback function is called once mapping and content of the service have been discovered\n
 * cb_features callback function is called once list of supported Bond Management Feature have been received\n
 *
 * @param[in] conidx                Connection index
 *
 * @return Status of function execution (see #hl_err enumeration)\n
 * cb_cmp_evt callback function is called once discovery has been completed (cmd_type = #BMSC_CMD_DISCOVER)
 ****************************************************************************************
 */
uint16_t bmsc_discover(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Restore bond data previously provided in cb_bond_data callback function
 *
 * @param[in] conidx                Connection index
 * @param[in] p_bms                 Pointer to structure describing Bond Management Service discovered in server's
 * attribute database
 *
 * @return Status of function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t bmsc_restore_bond_data(uint8_t conidx, const bmsc_bms_content_t* p_bms);

/**
 ****************************************************************************************
 * @brief Require execution of a Bond Management delete procedure
 *
 * @param[in] conidx                Connection index
 * @param[in] opcode                Operation code (see #bms_opcode enumeration)
 * @param[in] auth_code_length      Length of Authorization Code (from 0 to #BMS_AUTH_CODE_LENGTH_MAX)
 * @param[in] p_auth_code           Pointer to Authorization Code\n
 *                                  Cannot be NULL if auth_code_length is set to 0
 *
 * @return Status of function execution (see #hl_err enumeration)\n
 * cb_cmp_evt callback function is called once execution has been required  (cmd_type = #BMSC_CMD_DELETE)
 ****************************************************************************************
 */
uint16_t bmsc_delete(uint8_t conidx, uint8_t opcode, uint16_t auth_code_length, const uint8_t* p_auth_code);

/// @} BMSC_API_NATIVE

#endif // BMSC_H_
