/**
 ****************************************************************************************
 *
 * @file basc.h
 *
 * @brief Header file - Battery Service Client - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BASC_H_
#define BASC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BASC_API Battery Service Client
 * @ingroup BAS_API
 * @brief Description of API for Battery Service Client\n See \ref bas_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BASC_API_COMMON Common
 * @ingroup BASC_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BASC_API_NATIVE Native API
 * @ingroup BASC_API
 * @brief Description of Native API for Battery Service Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bas.h"
#include "basc_cfg.h"

/// @addtogroup BASC_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Command code
enum basc_cmd_code
{
    /// Discover
    BASC_CMD_DISCOVER = 0u,
    /// Get
    BASC_CMD_GET,
    /// Set CCCD
    BASC_CMD_SET_CCCD,
    /// Get Characteristic Presentation Format for Battery Level characteristic
    BASC_CMD_GET_PRESENTATION_FORMAT,

    BASC_CMD_MAX,
};

/// Characteristic type
enum basc_char_type
{
    /// Battery Level characteristic
    BASC_CHAR_TYPE_LEVEL = 0u,
    #if (HOST_MSG_API || BASC_LEVEL_STATUS)
    /// Battery Level Status characteristic
    BASC_CHAR_TYPE_LEVEL_STATUS,
    #endif // (HOST_MSG_API || BASC_LEVEL_STATUS)
    #if (HOST_MSG_API || BASC_CRITICAL_STATUS)
    /// Battery Critical Status characteristic
    BASC_CHAR_TYPE_CRITICAL_STATUS,
    #endif // (HOST_MSG_API || BASC_CRITICAL_STATUS)
    #if (HOST_MSG_API || BASC_ENERGY_STATUS)
    /// Battery Energy Status characteristic
    BASC_CHAR_TYPE_ENERGY_STATUS,
    #endif // (HOST_MSG_API || BASC_ENERGY_STATUS)
    #if (HOST_MSG_API || BASC_TIME_STATUS)
    /// Battery Time Status characteristic
    BASC_CHAR_TYPE_TIME_STATUS,
    #endif // (HOST_MSG_API || BASC_TIME_STATUS)
    #if (HOST_MSG_API || BASC_ESTIMATED_SERVICE_DATE)
    /// Estimated Service Date characteristic
    BASC_CHAR_TYPE_ESTIMATED_SERVICE_DATE,
    #endif // (HOST_MSG_API || BASC_ESTIMATED_SERVICE_DATE)
    #if (HOST_MSG_API || BASC_HEALTH_STATUS)
    /// Battery Health Status characteristic
    BASC_CHAR_TYPE_HEALTH_STATUS,
    #endif // (HOST_MSG_API || BASC_HEALTH_STATUS)
    #if (HOST_MSG_API || BASC_HEALTH_INFORMATION)
    /// Battery Health Information characteristic
    BASC_CHAR_TYPE_HEALTH_INFO,
    #endif // (HOST_MSG_API || BASC_HEALTH_INFORMATION)
    #if (HOST_MSG_API || BASC_INFORMATION)
    /// Battery Information characteristic
    BASC_CHAR_TYPE_INFO,
    #endif // (HOST_MSG_API || BASC_INFORMATION)
    #if (HOST_MSG_API || BASC_MANUFACTURER_NAME)
    /// Manufacturer Name String characteristic
    BASC_CHAR_TYPE_MANUFACTURER_NAME,
    #endif // (HOST_MSG_API || BASC_MANUFACTURER_NAME)
    #if (HOST_MSG_API || BASC_MODEL_NUMBER)
    /// Model Number String characteristic
    BASC_CHAR_TYPE_MODEL_NUMBER,
    #endif // (HOST_MSG_API || BASC_MODEL_NUMBER)
    #if (HOST_MSG_API || BASC_SERIAL_NUMBER)
    /// Serial Number String characteristic
    BASC_CHAR_TYPE_SERIAL_NUMBER,
    #endif // (HOST_MSG_API || BASC_SERIAL_NUMBER)

    BASC_CHAR_TYPE_MAX,
};

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// Structure containing description of BAS discovered in peer device's database
typedef struct
{
    /// Service information
    prf_svc_t svc;
    /// Characteristic information
    prf_char_t chars[BASC_CHAR_TYPE_MAX];
    /// Client Characteristic Configuration descriptor information
    prf_desc_t cccd[BASC_CHAR_TYPE_MAX];
    #if (HOST_MSG_API || BASC_PRESENTATION_FORMAT)
    /// Characteristic presentation format descriptor information
    prf_desc_t desc_presentation_format;
    #endif // (HOST_MSG_API || BASC_PRESENTATION_FORMAT)
} basc_content_t;

/// @} BASC_API_COMMON

/// @addtogroup BASC_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback function for backward communication with upper layer
typedef struct
{
    /**
     ****************************************************************************************
     * @brief Command completed event
     *
     * @param[in] conidx            Connection index
     * @param[in] status            Status (see #hl_err enumeration)
     * @param[in] cmd_code          Command code (see #basc_cmd_code enumeration)
     * @param[in] instance_idx      Instance index
     * @param[in] char_type         Characteristic type (see #basc_char_type enumeration)
     ****************************************************************************************
     */
    void (*cb_cmp_evt)(uint8_t conidx, uint16_t status, uint16_t cmd_code, uint8_t instance_idx, uint8_t char_type);

    /**
     ****************************************************************************************
     * @brief Inform about an update of bond data to be stored
     *
     * @param[in] conidx            Connection index
     * @param[in] nb_instances      Number of discovered instances
     * @param[in] p_bond_data       Pointer to bond data
     ****************************************************************************************
     */
    void (*cb_bond_data)(uint8_t conidx, uint8_t nb_instances, const basc_content_t* p_bond_data);

    /**
     ****************************************************************************************
     * @brief Inform about received characteristic value
     *
     * @param[in] conidx            Connection index
     * @param[in] instance_idx      Instance index
     * @param[in] char_type         Characteristic type (see #basc_char_type enumeration)
     * @param[in] p_buf             Pointer to buffer containing received value
     * For more details about data composition:
     *  - Battery Level, see #bas_level_size enumeration
     *  - Battery Level Status, see #bas_level_status_size enumeration
     *  - Battery Critical Status, see #bas_critical_status_size enumeration
     *  - Battery Energy Status, see #bas_energy_status_size enumeration
     *  - Battery Time Status, see #bas_time_status_size enumeration
     *  - Battery Health Status, see #bas_health_status_size enumeration
     *  - Battery Health Information, see #bas_health_info_size enumeration
     *  - Battery Information, see #bas_info_size enumeration
     *  - Estimated Service Date, see #bas_service_date_size
     ****************************************************************************************
     */
    void (*cb_value)(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, co_buf_t* p_buf);

    #if (BASC_PRESENTATION_FORMAT)
    /**
     ****************************************************************************************
     * @brief Inform about received Characteristic Presentation Format for Battery Level characteristic
     *
     * @param[in] conidx            Connection index
     * @param[in] instance_idx      Instance index
     * @param[in] p_buf             Pointer to buffer containing received value
     ****************************************************************************************
     */
    void (*cb_presentation_format)(uint8_t conidx, uint8_t instance_idx, co_buf_t* p_buf);
    #endif // (BASC_PRESENTATION_FORMAT)
} basc_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Battery Service as Client
 *
 * @param[in] p_cbs         Pointer to set of callback functions for communication with upper layer
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t basc_add(const basc_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/**
 ****************************************************************************************
 * @brief Discover Battery Service instances in a peer device's database
 *
 * @param[in] conidx            Connection index
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t basc_discover(uint8_t conidx);

#if (HL_BONDABLE)
/**
 ****************************************************************************************
 * @brief Restore bond data
 *
 * @param[in] conidx            Connection index
 * @param[in] nb_instances      Number of instances
 * @param[in] p_bond_data       Pointer to bond data
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t basc_restore_bond_data(uint8_t conidx, uint8_t nb_instances, const basc_content_t* p_bond_data);
#endif // (HL_BONDABLE)

/**
 ****************************************************************************************
 * @brief Get value of a Battery Service characteristic
 *
 * @param[in] conidx            Connection index
 * @param[in] instance_idx      Instance index
 * @param[in] char_type         Characteristic type (see #basc_char_type enumeration)
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t basc_get(uint8_t conidx, uint8_t instance_idx, uint8_t char_type);

#if (BASC_PRESENTATION_FORMAT)
/**
 ****************************************************************************************
 * @brief Get value of Characteristic Presentation Format descriptor for Battery Level characteristic
 *
 * @param[in] conidx            Connection index
 * @param[in] instance_idx      Instance index
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t basc_get_presentation_format(uint8_t conidx, uint8_t instance_idx);
#endif // (BASC_PRESENTATION_FORMAT)

/**
 ****************************************************************************************
 * @brief Enable/disable sending of notifications/indications for a Battery Service characteristic
 *
 * @param[in] conidx            Connection index
 * @param[in] instance_idx      Instance index
 * @param[in] char_type         Characteristic type (see #basc_char_type enumeration)
 * @param[in] p_buf             Pointer to buffer
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t basc_set_cccd(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, co_buf_t* p_buf);

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @return Pointer to content structure
 *
 * @param[in] conidx            Connection index
 * @param[in] instance_idx      Instance index
 ****************************************************************************************
 */
const basc_content_t* basc_get_content(uint8_t conidx, uint8_t instance_idx);
#endif // (!HOST_MSG_API)

/// @} BASC_API_NATIVE

#endif // BASC_H_
