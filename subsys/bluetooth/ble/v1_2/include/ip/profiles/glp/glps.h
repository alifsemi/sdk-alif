/**
 ****************************************************************************************
 *
 * @file glps_msg.h
 *
 * @brief Header file - Glucose Profile Sensor - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _GLPS_H_
#define _GLPS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup GLPS
 * @ingroup GLP
 * @brief Glucose Profile Sensor - Native API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "glps_msg.h"


/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Glucose sensor server callback set
typedef struct glps_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of measurement transmission
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_meas_send_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Inform that bond data updated for the connection.
     *
     * @param[in] conidx        Connection index
     * @param[in] evt_cfg       Glucose indication/notification configuration (see enum #glps_evt_cfg_bf)
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t evt_cfg);

    /**
     ****************************************************************************************
     * @brief Inform that peer device requests an action using record access control point
     *
     * @note control point request must be answered using #glps_racp_rsp_send function
     *
     * @param[in] conidx        Connection index
     * @param[in] op_code       Operation Code (see #glp_racp_op_code)
     * @param[in] func_operator Function operator (see enum glp_racp_operator)
     * @param[in] filter_type   Filter type (see enum #glp_racp_filter)
     * @param[in] p_filter      Pointer to filter information
     ****************************************************************************************
     */
    void (*cb_racp_req)(uint8_t conidx, uint8_t op_code, uint8_t func_operator,
                        uint8_t filter_type, const union glp_filter* p_filter);

    /**
     ****************************************************************************************
     * @brief Completion of record access control point response send procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_racp_rsp_send_cmp)(uint8_t conidx, uint16_t status);

} glps_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx        Connection index
 * @param[in] evt_cfg       Glucose indication/notification configuration (see enum #glps_evt_cfg_bf)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t glps_enable(uint8_t conidx, uint8_t evt_cfg);

/**
 ****************************************************************************************
 * @brief Send a measurement to registered peer devices
 *
 * Wait for #cb_meas_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx           Connection index
 * @param[in] seq_num          Measurement sequence number
 * @param[in] p_meas           Pointer to glucose measurement information
 * @param[in] p_ctx            Pointer to glucose measurement information context (can be NULL)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t glps_meas_send(uint8_t conidx, uint16_t seq_num, const glp_meas_t* p_meas, const glp_meas_ctx_t* p_ctx);

/**
 ****************************************************************************************
 * @brief Send record access control point response.
 *
 * Wait for #cb_ctnl_pt_rsp_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] op_code       Requested Operation Code (see enum #glp_racp_op_code)
 * @param[in] racp_status   Record access control point execution status (see enum #glp_racp_status)
 * @param[in] num_of_record Number of record (meaningful for GLP_REQ_REP_NUM_OF_STRD_RECS operation)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t glps_racp_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t racp_status, uint16_t num_of_record);


/// @} GLPS

#endif /* _GLPS_H_ */
