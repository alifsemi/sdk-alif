/**
 ****************************************************************************************
 *
 * @file plxs.h
 *
 * @brief Header file - Pulse Oximeter Service Sensor - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _PLXS_H_
#define _PLXS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup PLXS_API Pulse Oximeter Service Sensor
 * @ingroup PLX_API
 * @brief Description of API for Pulse Oximeter Service Sensor
 * @details
 * Note 1 - If PLX Spot-Check Measurement characteristic is supported, Record Access Control Point is supported\n
 * Note 2 - Sending of indications for PLX Features characteristic is always supported
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup PLXS_API_COMMON Common
 * @ingroup PLXS_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup PLXS_API_NATIVE Native API
 * @ingroup PLXS_API
 * @brief Description of Native API for Pulse Oximeter Service Sensor
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "plxp_common.h"

/// @addtogroup PLXS_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Type of operations that may be supported
enum plxs_optype_id
{
    /// Both Spot-check and Continuous measurements are supported\n
    /// Add support of following characteristics:
    ///    - PLX Spot-check Measurement
    ///    - PLX Continuous Measurement
    ///    - Record Access Control Point
    PLXS_OPTYPE_SPOT_CHECK_AND_CONTINUOUS = 0,
    /// Only Spot-check measurements are supported\n
    /// Add support of following characteristics:
    ///    - PLX Spot-check Measurement
    ///    - Record Access Control Point
    PLXS_OPTYPE_SPOT_CHECK_ONLY,
    /// Only Continuous measurements are supported\n
    /// Add support of following characteristics:
    ///    - PLX Continuous Measurement
    PLXS_OPTYPE_CONTINUOUS_ONLY,

    /// Maximum value
    PLXS_OPTYPE_MAX,
};

/// Command operation codes
enum plxs_op_codes
{
    /// Send a Spot-check measurement
    PLXS_SPOT_CHECK_MEAS_CMD_OP_CODE = 1,
    /// Send a Continuous measurement
    PLXS_CONTINUOUS_MEAS_CMD_OP_CODE,
    /// Send Control Point response
    PLXS_RACP_CMD_OP_CODE,
    /// Indicate that list of supported features has been updated
    PLXS_FEATURES_UPDATED_CMD_OP_CODE,
};

/// Characteristic type values
enum plxs_char_types
{
    /// PLX Features characteristic
    PLXS_FEATURES_CODE = 0,
    /// PLX Spot-check Measurement characteristic
    PLXS_SPOT_CHECK_MEAS_CODE,
    /// PLX Continuous Measurement characteristic
    PLXS_CONTINUOUS_MEAS_CODE,
    /// Record Access Control Point characteristic
    PLXS_RACP_CODE,

    /// Maximum number of characteristics
    PLXS_CHAR_TYPE_MAX,
};

/// Indication/notification configuration bit field meaning
enum plxs_evt_cfg_bf
{
    /// Bit used to know if sending of indications is enabled for PLX Features characteristic
    PLXS_FEATURES_IND_CFG_POS = PLXS_FEATURES_CODE,
    PLXS_FEATURES_IND_CFG_BIT = CO_BIT(PLXS_FEATURES_IND_CFG_POS),
    /// Bit used to know if sending of indications is enabled for PLX Spot-check Measurement characteristic
    PLXS_MEAS_SPOT_IND_CFG_POS = PLXS_SPOT_CHECK_MEAS_CODE,
    PLXS_MEAS_SPOT_IND_CFG_BIT = CO_BIT(PLXS_MEAS_SPOT_IND_CFG_POS),
    /// Bit used to know if sending of notifications is enabled for PLX Continuous Measurement characteristic
    PLXS_MEAS_CONT_NTF_CFG_POS = PLXS_CONTINUOUS_MEAS_CODE,
    PLXS_MEAS_CONT_NTF_CFG_BIT = CO_BIT(PLXS_MEAS_CONT_NTF_CFG_POS),
    /// Bit used to know if sending of indications is enabled for Record Access Control Point characteristic
    PLXS_RACP_IND_CFG_POS = PLXS_RACP_CODE,
    PLXS_RACP_IND_CFG_BIT = CO_BIT(PLXS_RACP_IND_CFG_POS),
};

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// Database configuration structure
typedef struct plxs_db_cfg
{
    /// Indicate kind of measurements that are supported (see #plxs_optype_id enumeration)
    uint8_t optype;
    /// Supported Features (see #plxp_sup_feat_bf enumeration)
    uint16_t sup_feat;
    /// Measurement Status Support (see #plxp_meas_status_sup_bf enumeration)\n
    /// Meaningful only if presence indicated in sup_feat value
    uint16_t meas_stat_sup;
    /// Device and Sensor Status Support (see #plxp_dev_sensor_status_bf enumeration)\n
    /// Meaningful only if presence indicated in sup_feat value
    uint32_t dev_stat_sup;
} plxs_db_cfg_t;

/// @} PLXS_API_COMMON

/// @addtogroup PLXS_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for communication with upper layer
typedef struct plxs_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of Spot-Check measurement transmission
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_spot_meas_send_cmp)(uint8_t conidx, uint16_t status);
    /**
     ****************************************************************************************
     * @brief Completion of Continuous measurement transmission
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_cont_meas_send_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Inform that bond data updated for the connection
     *
     * @param[in] conidx        Connection index
     * @param[in] evt_cfg       Indication/notification configuration (see #plxs_evt_cfg_bf enumeration)
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t evt_cfg);

    /**
     ****************************************************************************************
     * @brief Inform that peer device requests an action using record access control point
     *
     * @note control point request must be answered using #plxs_racp_rsp_send function
     *
     * @param[in] conidx        Connection index
     * @param[in] op_code       Operation Code (see #plxp_cp_opcodes_id enumeration)
     * @param[in] func_operator Function operator (see #plxp_cp_operator_id enumeration)
     ****************************************************************************************
     */
    void (*cb_racp_req)(uint8_t conidx, uint8_t op_code, uint8_t func_operator);

    /**
     ****************************************************************************************
     * @brief Completion of record access control point response send procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_racp_rsp_send_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Completion of #PLXS_FEATURES_UPDATED_CMD_OP_CODE procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     * @param[in] cmd_type      Command type (see #plxs_op_codes enumeration)
     ****************************************************************************************
     */
    void (*cb_cmp_evt)(uint8_t conidx, uint16_t status, uint8_t cmd_type);
} plxs_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Add support of Pulse Oximeter Service as Sensor
 *
 * @param[in] p_cfg             Pointer to configuration\n
 *                              Shall not be NULL
 * @param[in] p_cbs             Pointer to set of callback functions\n
 *                              Shall not be NULL
 * @param[in,out] p_start_hdl   Pointer to required start handle (input) and pointer at which effective start
 *                              handle will be returned\n
 *                              May be NULL
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxs_add(const plxs_db_cfg_t* p_cfg, const plxs_cb_t* p_cbs, uint16_t* p_start_hdl);

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)\n
 *
 * @param[in] conidx            Connection index
 * @param[in] evt_cfg           Indication/notification configuration (see #plxs_evt_cfg_bf enumeration)
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxs_enable(uint8_t conidx, uint8_t evt_cfg);

/**
 ****************************************************************************************
 * @brief Indicate to peer device that list of supported features has been updated since last connection\n
 * This function is intended to be called directly after successful call of plxs_enable function if:
 *     - List of supported features has been updated since last connection
 *     - Sending of indications for the PLX Features characteristic has been enabled by the peer device
 * Wait for cb_cmp_evt execution before starting a new procedure

 * @param[in] conidx            Connection index
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxs_features_updated(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Send a Spot-Check measurement to registered peer devices
 * Wait for #plxs_cb_t.cb_spot_meas_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx           Connection index
 * @param[in] p_spot_meas      Pointer to Spot-Check measurement information
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxs_spot_meas_send(uint8_t conidx, const plxp_spot_meas_t* p_spot_meas);

/**
 ****************************************************************************************
 * @brief Send a continuous measurement to registered peer devices
 * Wait for #plxs_cb_t.cb_cont_meas_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx           Connection index
 * @param[in] p_cont_meas      Pointer to Continuous Measurement information
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxs_cont_meas_send(uint8_t conidx, const plxp_cont_meas_t* p_cont_meas);

/**
 ****************************************************************************************
 * @brief Send record access control point response.
 * Wait for #plxs_cb_t.cb_racp_rsp_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx            Connection index
 * @param[in] op_code           Operation code provided in #plxs_cb_t.cb_racp_req callback function
 * @param[in] racp_status       Execution status (see #plxp_cp_resp_code_id enumeration)
 * @param[in] num_of_record     Number of records\n
 *                              Meaningful for #PLXP_OPCODE_REPORT_NUMBER_OF_STORED_RECORDS operation
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxs_racp_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t racp_status, uint16_t num_of_record);

/// @} PLXS_API_NATIVE

#endif // _PLXS_H_
