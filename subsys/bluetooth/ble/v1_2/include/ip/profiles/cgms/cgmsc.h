/**
 ****************************************************************************************
 *
 * @file cgmsc.h
 *
 * @brief Header file - Continuous Glucose Monitoring Service Client - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CGMSC_H_
#define CGMSC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CGMSC_API Continuous Glucose Monitoring Service Client
 * @ingroup CGMS_API
 * @brief Description of API for Continuous Glucose Monitoring Service Client\n See \ref cgms_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CGMSC_API_COMMON Common
 * @ingroup CGMSC_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CGMSC_API_NATIVE Native API
 * @ingroup CGMSC_API
 * @brief Description of Native API for Continuous Glucose Monitoring Service Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "cgms.h"

/// @addtogroup CGMSC_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Command code
enum cgmsc_cmd_code
{
    /// Discover
    CGMSC_CMD_DISCOVER = 0u,
    /// Get
    CGMSC_CMD_GET,
    /// Set CCCD
    CGMSC_CMD_SET_CCCD,
    /// Set Session Start Time
    CGMSC_CMD_SET_SESSION_START_TIME,
    /// Control
    CGMSC_CMD_CONTROL,
};

/// Client Characteristic Configuration descriptor type values
enum cgmsc_cccd_type
{
    /// CGM Measurement characteristic
    CGMSC_CCCD_TYPE_MEASUREMENT = 0u,
    /// CGM Feature characteristic
    CGMSC_CCCD_TYPE_FEATURE,
    /// Record Access Control Point characteristic
    CGMSC_CCCD_TYPE_RACP,
    /// CGM Specific Ops Control Point characteristic
    CGMSC_CCCD_TYPE_SOCP,

    CGMSC_CCCD_TYPE_MAX,
};

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// Structure containing description of CGMS discovered in peer device's database
typedef struct
{
    /// Service information
    prf_svc_t svc;
    /// Characteristic information
    prf_char_t chars[CGMS_CHAR_TYPE_MAX];
    /// Client Characteristic Configuration descriptor information
    prf_desc_t cccd[CGMSC_CCCD_TYPE_MAX];
} cgmsc_content_t;

/// Structure containing bond data
typedef struct
{
    /// Service content
    cgmsc_content_t cgms;
    /// Is E2E-CRC supported ?
    bool e2e_crc_supported;
} cgmsc_bond_data_t;

/// @} CGMSC_API_COMMON

/// @addtogroup CGMSC_API_NATIVE
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
     * @param[in] cmd_code          Command code (see #cgmsc_cmd_code enumeration)
     * @param[in] type              Characteristic type (see #cgms_char_type enumeration)
     *                              or Client Characteristic Configuration descriptor type
     *                              (see #cgmsc_cccd_type enumeration)
     ****************************************************************************************
     */
    void (*cb_cmp_evt)(uint8_t conidx, uint16_t status, uint16_t cmd_code, uint8_t type);

    /**
     ****************************************************************************************
     * @brief Inform about an update of bond data to be stored
     *
     * @param[in] conidx            Connection index
     * @param[in] p_bond_data       Pointer to bond data
     ****************************************************************************************
     */
    void (*cb_bond_data)(uint8_t conidx, const cgmsc_bond_data_t* p_bond_data);

    /**
     ****************************************************************************************
     * @brief Inform about received value for either CGM Feature characteristic or CGM Status characteristic or
     * CGM Session Start Time characteristic or CGM Session Run Time characteristic\n
     *
     * @param[in] conidx            Connection index
     * @param[in] char_type         Characteristic type (see #cgms_char_type enumeration)
     * @param[in] p_buf             Pointer to buffer containing received value\n
     *                              When applicable, E2E-CRC has already been checked and has been removed from
     *                              data part\n
     *                              It has already been checked that received length is at least larger than
     *                              minimum expected length
     ****************************************************************************************
     */
    void (*cb_value)(uint8_t conidx, uint8_t char_type, co_buf_t* p_buf);

    /**
     ****************************************************************************************
     * @brief Inform about received measurement
     *
     * @param[in] conidx            Connection index
     * @param[in] p_buf             Pointer to buffer containing received measurement\n
     *                              When applicable, E2E-CRC has already been checked and has been removed from
     *                              data part\n
     *                              It has already been checked that received length is at least larger than
     *                              minimum expected length
     ****************************************************************************************
     */
    void (*cb_measurement)(uint8_t conidx, co_buf_t* p_buf);
} cgmsc_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Continuous Glucose Monitoring Service as Client
 *
 * @param[in] p_cbs         Pointer to set of callback functions for communication with upper layer\n
 *                          Shall not be NULL
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmsc_add(const cgmsc_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/**
 ****************************************************************************************
 * @brief Discover Continuous Glucose Monitoring Service in a peer device's database
 *
 * @param[in] conidx            Connection index
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmsc_discover(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Restore bond data
 *
 * @param[in] conidx            Connection index
 * @param[in] p_bond_data       Pointer to bond data\n
 *                              Shall not be NULL
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmsc_restore_bond_data(uint8_t conidx, const cgmsc_bond_data_t* p_bond_data);

/**
 ****************************************************************************************
 * @brief Get value of either #CGMS_CHAR_TYPE_FEATURE or #CGMS_CHAR_TYPE_STATUS or
 * #CGMS_CHAR_TYPE_SESSION_START_TIME or #CGMS_CHAR_TYPE_SESSION_RUN_TIME characteristic
 *
 * @param[in] conidx            Connection index
 * @param[in] char_type         Characteristic type (see #cgms_char_type enumeration)\n
 *                              #CGMS_CHAR_TYPE_FEATURE or #CGMS_CHAR_TYPE_STATUS or
 *                              #CGMS_CHAR_TYPE_SESSION_START_TIME or #CGMS_CHAR_TYPE_SESSION_RUN_TIME
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmsc_get(uint8_t conidx, uint8_t char_type);

/**
 ****************************************************************************************
 * @brief Enable/disable sending of notifications/indications for one of Continuous Glucose Monitoring Service
 * characteristics
 *
 * @param[in] conidx            Connection index
 * @param[in] cccd_type         Client Characteristic Configuration descriptor type (see #cgmsc_cccd_type enumeration)
 * @param[in] p_buf             Pointer to buffer\n
 *                              Shall not be NULL
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmsc_set_cccd(uint8_t conidx, uint8_t cccd_type, co_buf_t* p_buf);

/**
 ****************************************************************************************
 * @brief Set CGM Session Start Time value\n
 *        When supported by peer device (CGM Feature characteristic shall have been read), E2E-CRC value is
 *        computed and added internally
 *
 * @param[in] conidx            Connection index
 * @param[in] p_buf             Pointer to buffer\n
 *                              Shall not be NULL
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmsc_set_session_start_time(uint8_t conidx, co_buf_t* p_buf);

/**
 ****************************************************************************************
 * @brief Request an operation available through either Record Access Control Point characteristic or CGM Specific
 *        Ops Control Point characteristic\n
 *        When supported by peer device (CGM Feature characteristic shall have been read), E2E-CRC value is
 *        computed and added internally (CGM Specific Ops Control Point characteristic only)
 *
 * @param[in] conidx            Connection index
 * @param[in] char_type         Characteristic type (see #cgms_char_type enumeration)\n
 *                              #CGMSC_CCCD_TYPE_RACP or #CGMSC_CCCD_TYPE_SOCP
 * @param[in] p_buf             Pointer to buffer\n
 *                              Shall not be NULL
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t cgmsc_control(uint8_t conidx, uint8_t char_type, co_buf_t* p_buf);

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @return Pointer to content structure
 *
 * @param[in] conidx            Connection index
 ****************************************************************************************
 */
const cgmsc_content_t* cgmsc_get_content(uint8_t conidx);
#endif // (!HOST_MSG_API)

/// @} CGMSC_API_NATIVE

#endif // CGMSC_H_
