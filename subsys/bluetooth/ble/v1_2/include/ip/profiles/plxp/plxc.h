/**
 ****************************************************************************************
 *
 * @file plxc.h
 *
 * @brief Header file - Pulse Oximeter Service Collector - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _PLXC_H_
#define _PLXC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup PLXC_API Pulse Oximeter Service Collector
 * @ingroup PLX_API
 * @brief Description of API for Pulse Oximeter Service Collector
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup PLXC_API_COMMON Common
 * @ingroup PLXC_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup PLXC_API_NATIVE Native API
 * @ingroup PLXC_API
 * @brief Description of Native API for Pulse Oximeter Service Collector
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "plxp_common.h"

/// @addtogroup PLXC_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Pulse Oximeter Service characteristics
enum plxc_char_ids
{
    /// PLX Spot-check Measurement characteristic
    PLXC_CHAR_SPOT_MEASUREMENT = 0,
    /// PLX Continuous Measurement characteristic
    PLXC_CHAR_CONT_MEASUREMENT,
    /// PLX Features Characteristic
    PLXC_CHAR_FEATURES,
    /// Record Access Control Point characteristic
    PLXC_CHAR_RACP,

    /// Number of characteristics
    PLXC_CHAR_MAX,
};

/// Pulse Oximeter Service descriptors
enum plxc_desc_ids
{
    /// PLX Spot-check Measurement characteristic - Client Characteristic Configuration descriptor
    PLXC_DESC_SPOT_MEASUREMENT_CCC = 0,
    /// PLX Continuous Measurement characteristic - Client Characteristic Configuration descriptor
    PLXC_DESC_CONT_MEASUREMENT_CCC,
    /// Record Access Control Point characteristic - Client Characteristic Configuration descriptor
    PLXC_DESC_RACP_CCC,
    /// PLX Features characteristic - Client Characteristic Configuration descriptor
    PLXC_DESC_FEATURES_CCC,

    /// Number of descriptors
    PLXC_DESC_MAX,
};

/// Value identifiers
enum plxc_val_id
{
    /// Features
    PLXC_VAL_FEATURES = 0,
    /// Spot-Check Measurement
    PLXC_VAL_SPOT_CHECK_MEAS,
    /// Continuous Measurement
    PLXC_VAL_CONTINUOUS_MEAS,
    /// RACP response
    PLXC_VAL_RACP_RSP,
    /// Spot-Check Measurement CCC value
    PLXC_VAL_SPOT_CHECK_MEAS_CFG,
    /// Continuous Measurement CCC value
    PLXC_VAL_CONTINUOUS_MEAS_CFG,
    /// RACP CCC value
    PLXC_VAL_RACP_CFG,
    /// Features CCC value
    PLXC_VAL_FEATURES_CFG,
};

/// Command operation codes
enum plxc_op_codes
{
    /// No operation
    PLXC_NO_OP = 0,
    /// Read
    PLXC_OP_CODE_READ,
    /// Write Client Characteristic Configuration
    PLXC_OP_CODE_WRITE_CCC,
    /// Send RACP Request
    PLXC_OP_CODE_WRITE_RACP,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Pulse Oximeter Service content description structure
typedef struct plxc_plxp_content
{
    /// Service information
    prf_svc_t svc;
    /// Characteristic information
    prf_char_t chars[PLXC_CHAR_MAX];
    /// Descriptor information
    prf_desc_t descs[PLXC_DESC_MAX];
} plxc_plxp_content_t;

/// @} PLXC_API_COMMON

/// @addtogroup PLXC_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for communication with upper layer
typedef struct plxc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     * @param[in] p_plx         Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const plxc_plxp_content_t* p_plx);

    /**
     ****************************************************************************************
     * @brief Completion of read feature procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     * @param[in] p_features    Pointer to sensor features information
     ****************************************************************************************
     */
    void (*cb_read_features_cmp)(uint8_t conidx, uint16_t status, const plxp_features_t* p_features);

    /**
     ****************************************************************************************
     * @brief Completion of read Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     * @param[in] val_id        Value identifier (see #plxc_val_id enumeration)
     *                               - #PLXC_VAL_SPOT_CHECK_MEAS_CFG
     *                               - #PLXC_VAL_CONTINUOUS_MEAS_CFG
     *                               - #PLXC_VAL_RACP_CFG
     *                               - #PLXC_VAL_FEATURES_CFG
     * @param[in] cfg_val       Configuration value
     ****************************************************************************************
     */
    void (*cb_read_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t val_id, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of sensor notification / indication configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see #hl_err enumeration)
     * @param[in] val_id        Value identifier (see #plxc_val_id enumeration)
     *                               - #PLXC_VAL_SPOT_CHECK_MEAS_CFG
     *                               - #PLXC_VAL_CONTINUOUS_MEAS_CFG
     *                               - #PLXC_VAL_RACP_CFG
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t val_id);

    /**
     ****************************************************************************************
     * @brief Function called when Spot-Check measurement information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_spot_meas    Pointer to Spot-Check measurement information
     ****************************************************************************************
     */
    void (*cb_spot_meas)(uint8_t conidx, const plxp_spot_meas_t* p_spot_meas);

    /**
     ****************************************************************************************
     * @brief Function called when Continuous measurement information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_cont_meas    Pointer to continuous measurement information
     ****************************************************************************************
     */
    void (*cb_cont_meas)(uint8_t conidx, const plxp_cont_meas_t* p_cont_meas);

    /**
     ****************************************************************************************
     * @brief Completion of record access control point request send
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the Request Send (see #hl_err enumeration)
     * @param[in] req_op_code   Requested Operation Code (see #plxp_cp_operator_id enumeration)
     ****************************************************************************************
     */
    void (*cb_racp_req_cmp)(uint8_t conidx, uint16_t status, uint8_t req_op_code);

    /**
     ****************************************************************************************
     * @brief Function called when record access point response is received
     *
     * @param[in] conidx            Connection index
     * @param[in] req_op_code       Requested Operation Code (see #plxp_cp_operator_id enumeration)
     * @param[in] racp_status       Record access control point execution status
     *                              (see #plxp_cp_resp_code_id enumeration)
     * @param[in] num_of_record     Number of records
     ****************************************************************************************
     */
    void (*cb_racp_rsp_recv)(uint8_t conidx, uint8_t req_op_code, uint8_t racp_status, uint16_t num_of_record);

    /**
     ****************************************************************************************
     * @brief Forward received list of supported features to upper layer\n
     * Optional for backward compatibility reason
     *
     * @param[in] conidx        Connection index
     * @param[in] p_features    Pointer to sensor features information
     ****************************************************************************************
     */
    void (*cb_features)(uint8_t conidx, const plxp_features_t* p_features);
} plxc_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Add support of Pulse Oximeter Service as Collector
 *
 * @param[in] p_cbs             Pointer to set of callback functions\n
 *                              Shall not be NULL
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxc_add(const plxc_cb_t* p_cbs);

/**
 ****************************************************************************************
 * @brief Either initiate discovery of Pulse Oxymeter Service or restore bond data of a known peer device
 * (at connection establishment)
 *
 * @param[in] conidx        Connection index
 * @param[in] con_type      Connection type
 *                              - #PRF_CON_DISCOVERY: Initiate discovery\n
 *                                  Wait for #plxc_cb_t.cb_enable_cmp execution before starting a new procedure
 *                              - #PRF_CON_NORMAL: Restore bond data\n
 *                                  No callback called in that case
 * @param[in] p_plx         Pointer to peer database description bond data\n
 *                          Meaningful only if con_type = #PRF_CON_NORMAL
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxc_enable(uint8_t conidx, uint8_t con_type, const plxc_plxp_content_t* p_plx);

/**
 ****************************************************************************************
 * @brief Perform a read sensor features procedure.
 * Wait for #plxc_cb_t.cb_read_features_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxc_read_features(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read Characteristic Configuration procedure.
 * Wait for #plxc_cb_t.cb_read_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] val_id        Value identifier (see #plxc_val_id enumeration)
 *                               - #PLXC_VAL_SPOT_CHECK_MEAS_CFG
 *                               - #PLXC_VAL_CONTINUOUS_MEAS_CFG
 *                               - #PLXC_VAL_RACP_CFG
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxc_read_cfg(uint8_t conidx, uint8_t val_id);

/**
 ****************************************************************************************
 * @brief Configure sensor notification and indication configuration.
 * Wait for #plxc_cb_t.cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] val_id        Value identifier (see #plxc_val_id enumeration)
 *                               - #PLXC_VAL_SPOT_CHECK_MEAS_CFG
 *                               - #PLXC_VAL_CONTINUOUS_MEAS_CFG
 *                               - #PLXC_VAL_RACP_CFG
 * @param[in] ccc           Client Characteristic Configuration value
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxc_write_cfg(uint8_t conidx, uint8_t val_id, uint16_t ccc);

/**
 ****************************************************************************************
 * @brief Function called to send a record access control point request
 * Wait for #plxc_cb_t.cb_racp_req_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] req_op_code   Requested Operation Code (see #plxp_cp_operator_id enumeration)
 * @param[in] func_operator Function operator
 *
 * @return Status of the function execution (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t plxc_racp_req(uint8_t conidx, uint8_t req_op_code, uint8_t func_operator);

/// @} PLXC_API_NATIVE

#endif // _PLXC_H_
