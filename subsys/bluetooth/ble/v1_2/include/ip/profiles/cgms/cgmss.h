/**
 ****************************************************************************************
 *
 * @file cgmss.h
 *
 * @brief Header file - Continuous Glucose Monitoring Service Server - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CGMSS_H_
#define CGMSS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CGMSS_API Continuous Glucose Monitoring Service Server
 * @ingroup CGMS_API
 * @brief Description of API for Continuous Glucose Monitoring Service Server\n See \ref cgms_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CGMSS_API_NATIVE Native API
 * @ingroup CGMSS_API
 * @brief Description of Native API for Continuous Glucose Monitoring Service Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "cgms.h"
#include "cgmss_cfg.h"

/// @addtogroup CGMSS_API_NATIVE
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Service configuration bit field meaning
enum cgmss_config_bf
{
    /// Support of indications for CGM Feature characteristic\n
    /// Meaningful only if compiled with CGMSS_FEATURE_IND option
    CGMSS_CONFIG_FEATURE_IND_POS = 0u,
    CGMSS_CONFIG_FEATURE_IND_BIT = CO_BIT(CGMSS_CONFIG_FEATURE_IND_POS),

    /// Activate support of E2E-CRC\n
    /// Meaningful only if compiled with CGMSS_E2E_CRC option
    CGMSS_CONFIG_E2E_CRC_POS = 1u,
    CGMSS_CONFIG_E2E_CRC_BIT = CO_BIT(CGMSS_CONFIG_E2E_CRC_POS),
};

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for Continuous Glucose Monitoring Service (Server)
typedef struct
{
    /**
     ****************************************************************************************
     * @brief Inform about write of CGM Session Start Time characteristic\n
     *        #cgmss_set_value_cfm shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] token             Token
     * @param[in] p_buf             Pointer to buffer\n
     *                              When applicable, E2E-CRC has already been checked and has been removed from
     *                              data part\n
     *                              It has already been checked that length of written is at least larger than
     *                              minimum expected length
     ****************************************************************************************
     */
    void (*cb_set_session_start_time_req)(uint8_t conidx, uint16_t token, co_buf_t* p_buf);

    /**
     ****************************************************************************************
     * @brief Request value after read of one of Continuous Glucose Monitoring Service's characteristics\n
     *        #cgmss_value_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] char_type         Characteristic type (see #cgms_char_type enumeration)
     * @param[in] token             Token
     ****************************************************************************************
     */
    void (*cb_value_req)(uint8_t conidx, uint8_t char_type, uint16_t token);

    /**
     ****************************************************************************************
     * @brief Request execution of operation required through either Record Access Control Point characteristic or
     * CGM Specific Ops Control Point characteristic\n
     *        #cgmss_set_value_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] char_type         Characteristic type (see #cgms_char_type enumeration)
     * @param[in] token             Token
     * @param[in] p_buf             Pointer to buffer\n
     *                              When applicable, E2E-CRC has already been checked and has been removed from
     *                              data part\n
     *                              It has already been checked that length of written is at least larger than
     *                              minimum expected length
     ****************************************************************************************
     */
    void (*cb_control_req)(uint8_t conidx, uint8_t char_type, uint16_t token, co_buf_t* p_buf);

    /**
     ****************************************************************************************
     * @brief Request value after read of one of Continuous Glucose Monitoring Service's Client Characteristic
     * Configuration descriptor/n
     *        #cgmss_get_cccd_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] char_type         Characteristic type (see #cgms_char_type enumeration)
     * @param[in] token             Token
     ****************************************************************************************
     */
    void (*cb_get_cccd_req)(uint8_t conidx, uint8_t char_type, uint16_t token);

    /**
     ****************************************************************************************
     * @brief Provide value written in one of Continuous Glucose Monitoring Service's Client Characteristic
     * Configuration descriptor/n
     *        #cgmss_set_value_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] char_type         Characteristic type (see #cgms_char_type enumeration)
     * @param[in] token             Token
     * @param[in] p_buf             Pointer to buffer\n
     *                              Length of written data has already been checked
     ****************************************************************************************
     */
    void (*cb_set_cccd_req)(uint8_t conidx, uint8_t char_type, uint16_t token, co_buf_t* p_buf);

    /**
     ****************************************************************************************
     * @brief Inform about indication or notification transmission status
     *
     * @param[in] conidx            Connection index
     * @param[in] char_type         Characteristic type (see #cgms_char_type enumeration)
     * @param[in] status            Status (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_sent)(uint8_t conidx, uint8_t char_type, uint16_t status);
} cgmss_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Continuous Glucose Monitoring Service as Server
 *
 * @param[in] config_bf     Configuration bit field (see #cgmss_config_bf enumeration)\n
 *                          Meaningful only if compiled with CGMSS_FLEXIBLE option
 * @param[in] p_cbs         Pointer to set of callback functions for communication with upper layer
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmss_add(uint8_t config_bf, const cgmss_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/**
 ****************************************************************************************
 * @brief Send a measurement through CGM Measurement characteristic
 *
 * @param[in] conidx            Connection index
 * @param[in] p_buf             Pointer to buffer containing measurement\n
 *                              Shall not be NULL\n
 *                              When applicable, E2E-CRC value is added internally
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmss_send_measurement(uint8_t conidx, co_buf_t* p_buf);

#if (CGMSS_FEATURE_IND)
/**
 ****************************************************************************************
 * @brief Send update for CGM Feature characteristic value
 *
 * @param[in] conidx            Connection index
 * @param[in] p_buf             Pointer to buffer\n
 *                              Shall not be NULL\n
 *                              E2E-CRC value (or default value) is added internally
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmss_send_features(uint8_t conidx, co_buf_t* p_buf);
#endif // (CGMSS_FEATURE_IND)

/**
 ****************************************************************************************
 * @brief Send response after handling of value written in either Record Access Control Point characteristic or
 * CGM Specific Ops Control Point characteristic
 *
 * @param[in] conidx            Connection index
 * @param[in] char_type         Characteristic type (see #cgms_char_type enumeration)
 * @param[in] p_buf             Pointer to buffer\n
 *                              Shall not be NULL\n
 *                              E2E-CRC value is added internally
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmss_send_control_response(uint8_t conidx, uint8_t char_type, co_buf_t* p_buf);

/**
 ****************************************************************************************
 * @brief Provide value requested for either CGM Feature characteristic or CGM Status characteristic or
 * CGM Session Start Time characteristic or CGM Session Run Time characteristic
 *
 * @param[in] conidx            Connection index
 * @param[in] token             Token
 * @param[in] char_type         Characteristic type (see #cgms_char_type enumeration)
 * @param[in] p_buf             Pointer to buffer\n
 *                              Shall not be NULL\n
 *                              When applicable, E2E-CRC value is added internally
 ****************************************************************************************
 */
void cgmss_value_cfm(uint8_t conidx, uint16_t token, uint8_t char_type, co_buf_t* p_buf);

/**
 ****************************************************************************************
 * @brief Provide value requested for one of the Client Characteristic Configuration descriptors
 *
 * @param[in] conidx            Connection index
 * @param[in] token             Token
 * @param[in] p_buf             Pointer to buffer\n
 *                              Shall not be NULL
 ****************************************************************************************
 */
void cgmss_get_cccd_cfm(uint8_t conidx, uint16_t token, co_buf_t* p_buf);

/**
 ****************************************************************************************
 * @brief Provide feedback about value written in CGM Session Start Time characteristic or Record Access Control
 * Point characteristic or in CGM Specific Ops Control Point characteristic or in one of the Client Characteristic
 * Configuration descriptors present within the service
 *
 * @param[in] conidx            Connection index
 * @param[in] status            Status (see #hl_err enumeration)
 * @param[in] token             Token
 ****************************************************************************************
 */
void cgmss_set_value_cfm(uint8_t conidx, uint16_t status, uint16_t token);

/// @} CGMSS_API_NATIVE

#endif // CGMSS_H_
