/**
 ****************************************************************************************
 *
 * @file htpt.h
 *
 * @brief Header file - Health Thermometer Profile Thermometer - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef HTPT_H_
#define HTPT_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup HTPT_API Health Thermometer Service Server
 * @ingroup HTP_API
 * @brief Description of API for Health Thermometer Service Server
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HTPT_API_COMMON Common
 * @ingroup HTPT_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HTPT_API_NATIVE Native API
 * @ingroup HTPT_API
 * @brief Description of Native API for Health Thermometer Service Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "htp.h"

/// @addtogroup HTPT_API_COMMON
/// @{

/*
 * DEFINES
 ****************************************************************************************
 */

/// All Features supported
#define HTPT_ALL_FEAT_SUP           (0x1F)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Database Feature Configuration Flags
enum htpt_features_bf
{
    /// Indicate if Temperature Type Char. is supported
    HTPT_TEMP_TYPE_CHAR_SUP_POS   = 0,
    HTPT_TEMP_TYPE_CHAR_SUP_BIT   = 0x0001,

    /// Indicate if Intermediate Temperature Char. is supported
    HTPT_INTERM_TEMP_CHAR_SUP_POS = 1,
    HTPT_INTERM_TEMP_CHAR_SUP_BIT = 0x0002,

    /// Indicate if Measurement Interval Char. is supported
    HTPT_MEAS_INTV_CHAR_SUP_POS   = 2,
    HTPT_MEAS_INTV_CHAR_SUP_BIT   = 0x0004,

    /// Indicate if Measurement Interval Char. supports indications
    HTPT_MEAS_INTV_IND_SUP_POS    = 3,
    HTPT_MEAS_INTV_IND_SUP_BIT    = 0x0008,

    /// Indicate if Measurement Interval Char. is writable
    HTPT_MEAS_INTV_WR_SUP_POS     = 4,
    HTPT_MEAS_INTV_WR_SUP_BIT     = 0x0010,
};

/// Notification and indication configuration
enum htpt_ntf_ind_cfg
{
    /// Stable measurement interval indication enabled
    HTPT_CFG_STABLE_MEAS_IND = (1 << 0),
    /// Intermediate measurement notification enabled
    HTPT_CFG_INTERM_MEAS_NTF = (1 << 1),
    /// Measurement interval indication
    HTPT_CFG_MEAS_INTV_IND   = (1 << 2),
};

/*
 * TYPES DEFINITIONS
 ****************************************************************************************
 */

/// Parameters of the Health thermometer service database
typedef struct htpt_db_cfg
{
    /// Health thermometer Feature (see #htpt_features_bf enumeration)
    uint8_t features;
    /// Temperature Type value (meaningful only if bit #HTPT_TEMP_TYPE_CHAR_SUP_POS set in #features bit field)
    uint8_t temp_type;
    /// Measurement Interval Valid Range - Minimal value
    uint16_t valid_range_min;
    /// Measurement Interval Valid Range - Maximal value
    uint16_t valid_range_max;
    /// Measurement interval (latest known interval range)
    uint16_t meas_intv;
} htpt_db_cfg_t;

/// @} HTPT_API_COMMON

/// @addtogroup HTPT_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Health Thermometer Profile server callback set
typedef struct htpt_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of Send temperature procedure
     *
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_temp_send_cmp)(uint16_t status);

    /**
     ****************************************************************************************
     * @brief Inform that peer device want to update of measurement interval value
     *
     * @param[in] conidx    Connection index
     * @param[in] meas_intv Measurement Interval value in seconds
     ****************************************************************************************
     */
    void (*cb_meas_intv_chg_req)(uint8_t conidx, uint16_t meas_intv);

    /**
     ****************************************************************************************
     * @brief Inform that Bond data updated for the connection.
     *
     * @param[in] conidx        Connection index
     * @param[in] ntf_ind_cfg   Notification Configuration (see enum #htpt_ntf_ind_cfg)
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t ntf_ind_cfg);
} htpt_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx        Connection index
 * @param[in] ntf_ind_cfg   Notification configuration (Bond Data to restore: see enum #htpt_ntf_ind_cfg)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t htpt_enable(uint8_t conidx, uint8_t ntf_ind_cfg);

/**
 ****************************************************************************************
 * @brief Send temperature value from APP
 *
 * Wait for #htpt_cb_t.cb_temp_send_cmp execution before sending a new measurement
 *
 * @param[in] p_temp_meas   Pointer to Temperature Measurement information
 * @param[in] stable_meas   Stable or intermediary type of temperature (True stable meas, else false)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t htpt_temp_send(const htp_temp_meas_t* p_temp_meas, bool stable_meas);

/**
 ****************************************************************************************
 * @brief Update the measurement interval value
 *
 * @param[in] meas_intv Measurement Interval value in seconds
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t htpt_meas_intv_upd(uint16_t meas_intv);

/**
 ****************************************************************************************
 * @brief Confirm or not update of measurement interval value
 *
 * @param[in] conidx  Connection index
 * @param[in] status  Status of the request
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t htpt_meas_intv_chg_cfm(uint8_t conidx, uint16_t status);

/// @} HTPT_API_NATIVE

#endif // HTPT_H_
