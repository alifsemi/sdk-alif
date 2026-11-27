/**
 ****************************************************************************************
 *
 * @file bmss.h
 *
 * @brief Header file - Bond Management Service Server - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BMSS_H_
#define BMSS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BMSS_API Bond Management Service Server
 * @ingroup BMS_API
 * @brief Description of API for Bond Management Service Server
 * @details Note - Reliable Write property is not supported for Bond Management Control Point characteristic
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BMSS_API_COMMON Common
 * @ingroup BMSS_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BMSS_API_NATIVE Native API
 * @ingroup BMSS_API
 * @brief Description of Native API for Bond Management Service Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bms.h"

/// @addtogroup BMSS_API_COMMON
/// @{

/// @} BMSS_API_COMMON

/// @addtogroup BMSS_API_NATIVE
/// @{

/*
 * TYPES DEFINITIONS
 ****************************************************************************************
 */

/// Bond Data structure
typedef struct bmss_bond_data
{
    /// Indicate if sending of indications is enabled for Bond Data Management Features characteristic
    bool ind_enabled;
} bmss_bond_data_t;

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for Bond Management Service Server
typedef struct bmss_cb
{
    /**
     ****************************************************************************************
     * @brief Provide bond data to upper layer.\n
     * The bond data must be stored and provided after establishment of a new connection with the server.
     *
     * @param[in] conidx                Connection index
     * @param[in] p_bond_data           Pointer to Bond Data structure
     ****************************************************************************************
     */
    void (*cb_bond_data)(uint8_t conidx, const bmss_bond_data_t* p_bond_data);

    /**
     ****************************************************************************************
     * @brief Inform upper layer that list of supported Bond Management Feature is required\n
     * The upper layer shall call bmss_feature_cfm function in order to provide the requested information.
     * Implementation allows call of the confirmation function directly from the callback function
     *
     * @param[in] conidx                Connection index
     * @param[in] token                 Token that shall be provided back in the confirmation
     ****************************************************************************************
     */
    void (*cb_features_req_ind)(uint8_t conidx, uint16_t token);

    /**
     ****************************************************************************************
     * @brief Inform upper layer about a received delete request\n
     * The upper layer shall call bmss_delete_cfm function once delete operation has been performed or if an error
     * has been detected.
     * Implementation allows call of the confirmation function directly from the callback function
     *
     * @param[in] conidx                Connection index
     * @param[in] token                 Token that shall be provided back in the confirmation
     * @param[in] opcode                Operation code (see #bms_opcode enumeration)
     * @param[in] auth_code_length      Length of Authorization Code
     * @param[in] p_auth_code           Pointer to Authorization Code
     ****************************************************************************************
     */
    void (*cb_delete_req_ind)(uint8_t conidx, uint16_t token, uint8_t opcode, uint16_t auth_code_length,
                              const uint8_t* p_auth_code);
} bmss_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Add support of Bond Management Service as Server
 *
 * @param[in] p_cbs                 Pointer to set of callback functions\n
 *                                  Shall not be NULL
 * @param[in,out] p_start_hdl       Pointer to required start handle of the service (input) and pointer at which
 *                                  effective start handle will be returned\n
 *                                  May be NULL
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t bmss_add(const bmss_cb_t* p_cbs, uint16_t* p_start_hdl);

/**
 ****************************************************************************************
 * @brief Restore bond data previously provided in cb_bond_data callback function\n
 * Should be used only if Bond Management Feature may change during lifetime of the device
 *
 * @param[in] conidx                Connection index
 * @param[in] p_bond_data           Pointer to Bond Data structure\n
 *                                  Shall not be NULL
 * @param[in] p_features_bf         Pointer to Bond Management Feature value if value has changed since last
 *                                  connection (see #bms_features_bf enumeration) else NULL\n
 *                                  Meaningful only if ind_enabled is true
 *                                  No more than #BMS_FEATURES_LENGTH_MAX bytes shall be provided\n
 *                                  Sent bit field is updated based on supported transport
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t bmss_restore_bond_data(uint8_t conidx, const bmss_bond_data_t* p_bond_data, const uint8_t* p_features_bf);

/**
 ****************************************************************************************
 * @brief Provide list of supported Bond Management Feature requested in cb_features_req_ind callback function.
 *
 * @param[in] conidx                Connection index
 * @param[in] token                 Token provided in the request callback function
 * @param[in] p_features_bf         Pointer to Features bit field value (see #bms_features_bf enumeration)\n
 *                                  No more than #BMS_FEATURES_LENGTH_MAX bytes shall be provided\n
 *                                  Sent bit field is updated based on supported transport
 ****************************************************************************************
 */
void bmss_features_cfm(uint8_t conidx, uint16_t token, const uint8_t* p_features_bf);

/**
 ****************************************************************************************
 * @brief Confirm execution of delete operation requested in cb_delete_req_ind callback function
 *
 * @param[in] conidx                Connection index
 * @param[in] status                Status (see #bms_error enumeration)
 * @param[in] token                 Token provided in the request callback function
 ****************************************************************************************
 */
void bmss_delete_cfm(uint8_t conidx, uint8_t status, uint16_t token);

/// @} BMSS_API_NATIVE

#endif // BMSS_H_
