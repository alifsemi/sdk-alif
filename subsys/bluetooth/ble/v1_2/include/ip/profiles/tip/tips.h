/**
 ****************************************************************************************
 *
 * @file tips.h
 *
 * @brief Header file - Time Profile Server - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TIPS_H_
#define TIPS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TIPS_API Time Profile Server
 * @ingroup TIP_API
 * @brief Description of API for Time Profile Server
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TIPS_API_COMMON Common
 * @ingroup TIPS_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TIPS_API_NATIVE Native API
 * @ingroup TIPS_API
 * @brief Description of Native API for Time Profile Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tip_common.h"

/// @addtogroup TIPS_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Database Configuration bit field meaning
enum tips_db_config_bf
{
    /// Indicate if Local Time Information characteristic is supported (Current Time Service) - Position
    TIPS_CTS_LOC_TIME_INFO_SUP_POS = 0,
    /// Indicate if Local Time Information characteristic is supported (Current Time Service) - Bit
    TIPS_CTS_LOC_TIME_INFO_SUP_BIT = CO_BIT(TIPS_CTS_LOC_TIME_INFO_SUP_POS),

    /// Indicate if Reference Time Information characteristic is supported (Current Time Service) - Position
    TIPS_CTS_REF_TIME_INFO_SUP_POS = 1,
    /// Indicate if Reference Time Information characteristic is supported (Current Time Service) - Bit
    TIPS_CTS_REF_TIME_INFO_SUP_BIT = CO_BIT(TIPS_CTS_REF_TIME_INFO_SUP_POS),

    /// Indicate if Next DST Change Service is supported - Position
    TIPS_NDCS_SUP_POS = 2,
    /// Indicate if Next DST Change Service is supported - Bit
    TIPS_NDCS_SUP_BIT = CO_BIT(TIPS_NDCS_SUP_POS),

    /// Indicate if Reference Time Update Service is supported - Position
    TIPS_RTUS_SUP_POS = 3,
    /// Indicate if Reference Time Update Service is supported - Bit
    TIPS_RTUS_SUP_BIT = CO_BIT(TIPS_RTUS_SUP_POS),

    /// Current Time Service Current Time Configuration support - Position
    TIPS_CTS_CURRENT_TIME_CFG_POS = 4,
    /// Current Time Service Current Time Configuration support - Bit
    TIPS_CTS_CURRENT_TIME_CFG_BIT = CO_BIT(TIPS_CTS_CURRENT_TIME_CFG_POS),
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Profile configuration structure
typedef struct tips_db_cfg
{
    /// Database configuration (see #tips_db_config_bf enumeration)
    uint8_t features;
} tips_db_cfg_t;

/// @} TIPS_API_COMMON

/// @addtogroup TIPS_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for communication with upper layer
typedef struct tips_cb
{
    /**
     ****************************************************************************************
     * @brief Inform that bond data has been updated for a connection.\n
     * This information shall be stored and provided at reconnection through #tips_enable function
     *
     * @param[in] conidx        Connection index
     * @param[in] cfg_val       Event configuration value for Current Time characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of current time update procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_curr_time_upd_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Indicate that a time information is requested
     * Time information must be returned by application using #tips_rd_info_cfm
     *
     * @param[in] conidx        Connection index
     * @param[in] token         Token information that must be returned in confirmation
     * @param[in] val_id        Value identifier (see #tip_value_id enumeration)
     ****************************************************************************************
     */
    void (*cb_rd_info_req)(uint8_t conidx, uint32_t token, uint8_t val_id);

    /**
     ****************************************************************************************
     * @brief Indicate that a request has been received from peer device
     *
     * @param[in] conidx          Connection index
     * @param[in] value           Time Update Control Point value
     ****************************************************************************************
     */
    void (*cb_ctnl_pt)(uint8_t conidx, tip_time_upd_contr_pt_t value);
} tips_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Add support of Current Time Service and optionally support of Next Change Service and support of
 * Reference Time Update service as Server
 *
 * @param[in] p_cfg             Pointer to configuration structure\n
 *                              Shall not be NULL
 * @param[in] p_cbs             Pointer to set of callback functions\n
 *                              Shall not be NULL
 * @param[in,out] p_start_hdl   Pointer to expected service start handle (input) and pointer at which effective start
 *                              handle will be returned\n
 *                              May be NULL
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tips_add(const tips_db_cfg_t* p_cfg, const tips_cb_t* p_cbs, uint16_t* p_start_hdl);

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx       Connection index
 * @param[in] ntf_cfg      Current Time notification configuration
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tips_enable(uint8_t conidx, uint16_t ntf_cfg);

/**
 ****************************************************************************************
 * @brief Send Current time value information
 *
 * @param[in] conidx            Connection index
 * @param[in] enable_ntf_send   Indicate if the new Current Time value can be sent if the
 *                              current_time_ntf_en parameter is enabled
 * @param[in] p_current_time    Pointer to current time value
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tips_curr_time_upd(uint8_t conidx, bool enable_ntf_send, const tip_curr_time_t* p_current_time);

/**
 ****************************************************************************************
 * @brief Send back time information
 *
 * @param[in] conidx        Connection index
 * @param[in] token         Token information received in request
 * @param[in] val_id        Value identifier (see #tip_value_id enumeration)
 * @param[in] p_value       Pointer to time value information
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tips_rd_info_cfm(uint8_t conidx, uint32_t token, uint8_t val_id, const union tip_value* p_value);

/// @} TIPS_API_NATIVE

#endif // TIPS_H_
