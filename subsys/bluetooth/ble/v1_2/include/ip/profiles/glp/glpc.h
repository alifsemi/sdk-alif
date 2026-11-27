/**
 ****************************************************************************************
 *
 * @file glpc.h
 *
 * @brief Header file - Glucose Profile Collector Role - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _GLPC_H_
#define _GLPC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup GLPC
 * @ingroup Profile
 * @brief Glucose Profile Collector - Native API
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "glpc_msg.h"


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Glucose sensor client callback set
typedef struct glpc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_gls         Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const gls_content_t* p_gls);

    /**
     ****************************************************************************************
     * @brief Completion of read sensor feature procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] features      Glucose sensor features
     *
     ****************************************************************************************
     */
    void (*cb_read_sensor_feat_cmp)(uint8_t conidx, uint16_t status, uint16_t features);

    /**
     ****************************************************************************************
     * @brief Completion of read Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] read_code     Glucose sensor read characteristic code (see enum #glpc_read_code)
     * @param[in] cfg_val       Configuration value
     *
     ****************************************************************************************
     */
    void (*cb_read_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t read_code, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of glucose sensor notification register procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     *
     ****************************************************************************************
     */
    void (*cb_register_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Function called when glucose measurement information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] seq_num        Measurement sequence number
     * @param[in] p_meas         Pointer to glucose measurement information
     ****************************************************************************************
     */
    void (*cb_meas)(uint8_t conidx, uint16_t seq_num, const glp_meas_t* p_meas);

    /**
     ****************************************************************************************
     * @brief Function called when glucose measurement context information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] seq_num        Measurement sequence number
     * @param[in] p_ctx          Pointer to glucose measurement information context
     ****************************************************************************************
     */
    void (*cb_meas_ctx)(uint8_t conidx, uint16_t seq_num, const glp_meas_ctx_t* p_ctx);

    /**
     ****************************************************************************************
     * @brief Completion of record access control point request send
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the Request Send (see enum #hl_err)
     * @param[in] req_op_code   Requested Operation Code (see enum #glp_racp_op_code)
     ****************************************************************************************
     */
    void (*cb_racp_req_cmp)(uint8_t conidx, uint16_t status, uint8_t req_op_code);


    /**
     ****************************************************************************************
     * @brief Reception of record access point response.
     *
     * @param[in] conidx        Connection index
     * @param[in] req_op_code   Requested Operation Code (see enum #glp_racp_op_code)
     * @param[in] racp_status   Record access control point execution status (see enum #glp_racp_status)
     * @param[in] num_of_record Number of record (meaningful for GLP_REQ_REP_NUM_OF_STRD_RECS operation)
     ****************************************************************************************
     */
    void (*cb_racp_rsp_recv)(uint8_t conidx, uint8_t req_op_code, uint8_t racp_status, uint16_t num_of_record);
} glpc_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * Wait for #cb_enable_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] con_type      Connection type
 * @param[in] p_gls         Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t glpc_enable(uint8_t conidx, uint8_t con_type, const gls_content_t* p_gls);

/**
 ****************************************************************************************
 * @brief Perform a read sensor feature procedure.
 *
 * Wait for #cb_read_sensor_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t glpc_read_sensor_feat(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read Characteristic Configuration procedure.
 *
 * Wait for #cb_read_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] read_code     Glucose sensor read characteristic code (see enum #glpc_read_code)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t glpc_read_cfg(uint8_t conidx, uint8_t read_code);

/**
 ****************************************************************************************
 * @brief Register to Glucose sensor notifications.
 *
 * Wait for #cb_register_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] meas_ctx_en   Register or not Glucose measurement context notifications
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t glpc_register(uint8_t conidx, bool meas_ctx_en);

/**
 ****************************************************************************************
 * @brief Function called to send a record access control point request
 *
 * Wait for #cb_racp_req_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] req_op_code   Requested Operation Code (see #glp_racp_op_code)
 * @param[in] func_operator Function operator (see enum glp_racp_operator)
 * @param[in] filter_type   Filter type (see enum #glp_racp_filter)
 * @param[in] p_filter      Pointer to filter information
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t glpc_racp_req(uint8_t conidx, uint8_t req_op_code, uint8_t func_operator,
                       uint8_t filter_type, const union glp_filter* p_filter);

#endif /* _GLPC_H_ */
