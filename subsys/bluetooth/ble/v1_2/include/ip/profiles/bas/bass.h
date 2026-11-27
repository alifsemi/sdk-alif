/**
 ****************************************************************************************
 *
 * @file bass.h
 *
 * @brief Header file - Battery Service Server - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BASS_H_
#define BASS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BASS_API Battery Service Server
 * @ingroup BAS_API
 * @brief Description of API for Battery Service Server\n See \ref bas_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BASS_API_NATIVE Native API
 * @ingroup BASS_API
 * @brief Description of Native API for Battery Service Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bas.h"
#include "bass_cfg.h"

/// @addtogroup BASS_API_NATIVE
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Characteristic type
enum bass_char_type
{
    /// Battery Level characteristic
    BASS_CHAR_TYPE_LEVEL = 0u,
    #if (HOST_MSG_API || BASS_LEVEL_STATUS)
    /// Battery Level Status characteristic
    BASS_CHAR_TYPE_LEVEL_STATUS,
    #endif // (HOST_MSG_API || BASS_LEVEL_STATUS)
    #if (HOST_MSG_API || BASS_CRITICAL_STATUS)
    /// Battery Critical Status characteristic
    BASS_CHAR_TYPE_CRITICAL_STATUS,
    #endif // (HOST_MSG_API || BASS_CRITICAL_STATUS)
    #if (HOST_MSG_API || BASS_ENERGY_STATUS)
    /// Battery Energy Status characteristic
    BASS_CHAR_TYPE_ENERGY_STATUS,
    #endif // (HOST_MSG_API || BASS_ENERGY_STATUS)
    #if (HOST_MSG_API || BASS_TIME_STATUS)
    /// Battery Time Status characteristic
    BASS_CHAR_TYPE_TIME_STATUS,
    #endif // (HOST_MSG_API || BASS_TIME_STATUS)
    #if (HOST_MSG_API || BASS_ESTIMATED_SERVICE_DATE)
    /// Estimated Service Date characteristic
    BASS_CHAR_TYPE_ESTIMATED_SERVICE_DATE,
    #endif // (HOST_MSG_API || BASS_ESTIMATED_SERVICE_DATE)
    #if (HOST_MSG_API || BASS_HEALTH_STATUS)
    /// Battery Health Status characteristic
    BASS_CHAR_TYPE_HEALTH_STATUS,
    #endif // (HOST_MSG_API || BASS_HEALTH_STATUS)
    #if (HOST_MSG_API || BASS_HEALTH_INFORMATION)
    /// Battery Health Information characteristic
    BASS_CHAR_TYPE_HEALTH_INFO,
    #endif // (HOST_MSG_API || BASS_HEALTH_INFORMATION)
    #if (HOST_MSG_API || BASS_INFORMATION)
    /// Battery Information characteristic
    BASS_CHAR_TYPE_INFO,
    #endif // (HOST_MSG_API || BASS_INFORMATION)
    #if (HOST_MSG_API || BASS_MANUFACTURER_NAME)
    /// Manufacturer Name String characteristic
    BASS_CHAR_TYPE_MANUFACTURER_NAME,
    #endif // (HOST_MSG_API || BASS_MANUFACTURER_NAME)
    #if (HOST_MSG_API || BASS_MODEL_NUMBER)
    /// Model Number String characteristic
    BASS_CHAR_TYPE_MODEL_NUMBER,
    #endif // (HOST_MSG_API || BASS_MODEL_NUMBER)
    #if (HOST_MSG_API || BASS_SERIAL_NUMBER)
    /// Serial Number String characteristic
    BASS_CHAR_TYPE_SERIAL_NUMBER,
    #endif // (HOST_MSG_API || BASS_SERIAL_NUMBER)

    BASS_CHAR_TYPE_MAX,
};

/// Service configuration bit field meaning
enum bass_config_bf
{
    /// Support Characteristic Presentation Format descriptor for Battery Level characteristic\n
    /// Meaningful only if compiled with BASS_PRESENTATION_FORMAT option
    BASS_CONFIG_PRESENTATION_FORMAT_POS = 0u,
    BASS_CONFIG_PRESENTATION_FORMAT_BIT = CO_BIT(BASS_CONFIG_PRESENTATION_FORMAT_POS),

    /// First optional characteristic (see #bass_char_type enumeration)
    BASS_CONFIG_FIRST_OPT_CHAR_POS = BASS_CHAR_TYPE_LEVEL + 1u,

    /// Activate support of E2E-CRC\n
    /// Meaningful only if compiled with CGMSS_E2E_CRC option
    BASS_CONFIG_CRITICAL_POS = 12u,
    BASS_CONFIG_CRITICAL_BIT = CO_BIT(BASS_CONFIG_CRITICAL_POS),
};

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for Battery Service (Server)
typedef struct
{
    /**
     ****************************************************************************************
     * @brief Request value after read of one of Battery Service's characteristics/n
     *        #bass_value_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] instance_idx      Instance index
     * @param[in] char_type         Characteristic type (see #bass_char_type enumeration)
     * @param[in] token             Token
     ****************************************************************************************
     */
    void (*cb_value_req)(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, uint16_t token);

    /**
     ****************************************************************************************
     * @brief Request value after read of one of Battery Service's Client Characteristic Configuration descriptor/n
     *        #bass_value_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] instance_idx      Instance index
     * @param[in] char_type         Characteristic type (see #bass_char_type enumeration)
     * @param[in] token             Token
     ****************************************************************************************
     */
    void (*cb_get_cccd_req)(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, uint16_t token);

    /**
     ****************************************************************************************
     * @brief Provide value written in one of Battery Service's Client Characteristic Configuration descriptor/n
     *        #bass_set_cccd_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] instance_idx      Instance index
     * @param[in] char_type         Characteristic type (see #bass_char_type enumeration)
     * @param[in] token             Token
     * @param[in] p_buf             Pointer to buffer
     ****************************************************************************************
     */
    void (*cb_set_cccd_req)(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, uint16_t token, co_buf_t* p_buf);

    #if (BASS_PRESENTATION_FORMAT)
    /**
     ****************************************************************************************
     * @brief Request value of Characteristic Presentation Format descriptor for Battery Level characteristic/n
     *        #bass_value_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] instance_idx      Instance index
     * @param[in] token             Token
     ****************************************************************************************
     */
    void (*cb_presentation_format_req)(uint8_t conidx, uint8_t instance_idx, uint16_t token);
    #endif // (BASS_PRESENTATION_FORMAT)

    /**
     ****************************************************************************************
     * @brief Inform about indication or notification transmission status
     *
     * @param[in] conidx            Connection index
     * @param[in] instance_idx      Instance index
     * @param[in] char_type         Characteristic type (see #bass_char_type enumeration)
     * @param[in] status            Status (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_sent)(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, uint16_t status);
} bass_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Battery Service as Server
 *
 * @param[in] config_bf     Configuration bit field (see #bass_config_bf enumeration)\n
 *                          Meaningful only if compiled with BASS_FLEXIBLE option\n
 * @param[in] p_cbs         Pointer to set of callback functions for communication with upper layer
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t bass_add(uint16_t config_bf, const bass_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

#if (BASS_MULTI > 1)
/**
 ****************************************************************************************
 * @brief Add an additional instance of Battery Service in the database
 *
 * @param[in] config_bf     Configuration bit field (see #bass_config_bf enumeration)\n
 *                          Meaningful only if compiled with BASS_FLEXIBLE option
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t bass_add_instance(uint16_t config_bf);
#endif // (BASS_MULTI > 1)

/**
 ****************************************************************************************
 * @brief Notify/indicate new value for a Battery Service characteristic
 *
 * @param[in] conidx            Connection index
 * @param[in] instance_idx      Instance index
 * @param[in] char_type         Characteristic type (see #bass_char_type enumeration)
 * @param[in] evt_type          Event type (see #gatt_evt_type enumeration)
 * @param[in] p_buf             Pointer to buffer containing value\n
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
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t bass_update_value(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, uint8_t evt_type,
                           co_buf_t* p_buf);

/**
 ****************************************************************************************
 * @brief Provide value requested by peer device
 *
 * @param[in] conidx            Connection index
 * @param[in] token             Token
 * @param[in] p_buf             Pointer to buffer containing value
 ****************************************************************************************
 */
void bass_value_cfm(uint8_t conidx, uint16_t token, co_buf_t* p_buf);

/**
 ****************************************************************************************
 * @brief Provide value requested by peer device
 *
 * @param[in] conidx            Connection index
 * @param[in] status            Status (see #hl_err enumeration)
 * @param[in] token             Token
 ****************************************************************************************
 */
void bass_set_cccd_cfm(uint8_t conidx, uint16_t status, uint16_t token);

/// @} BASS_API_NATIVE

#endif // BASS_H_
