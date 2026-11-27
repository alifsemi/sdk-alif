/**
 ****************************************************************************************
 *
 * @file cppc.h
 *
 * @brief Header file - Cycling Power Profile Collector Role - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _CPPC_H_
#define _CPPC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup CPPC
 * @ingroup Profile
 * @brief Cycling Power Profile Collector - Native API
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "cppc_msg.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Cycling Power service client callback set
typedef struct cppc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_cps         Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const cppc_cps_content_t* p_cps);

    /**
     ****************************************************************************************
     * @brief Completion of read sensor feature procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] sensor_feat   CP sensor feature
     *
     ****************************************************************************************
     */
    void (*cb_read_sensor_feat_cmp)(uint8_t conidx, uint16_t status, uint32_t sensor_feat);

    /**
     ****************************************************************************************
     * @brief Completion of read sensor location procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] sensor_loc    Sensor Location
     *
     ****************************************************************************************
     */
    void (*cb_read_sensor_loc_cmp)(uint8_t conidx, uint16_t status, uint8_t sensor_loc);

    /**
     ****************************************************************************************
     * @brief Completion of read Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] desc_code     Descriptor Code (see enum #cppc_code)
     *                              - CPPC_RD_WR_CP_MEAS_CL_CFG: CP Measurement Client Char. Configuration
     *                              - CPPC_RD_WR_CP_MEAS_SV_CFG: CP Measurement Server Char. Configuration
     *                              - CPPC_RD_WR_VECTOR_CFG:     Vector Client Char. Configuration
     *                              - CPPC_RD_WR_CTNL_PT_CFG:    CP Control Point Client Char. Configuration
     * @param[in] cfg_val       Configuration value
     *
     ****************************************************************************************
     */
    void (*cb_read_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t desc_code, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of write Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] desc_code     Descriptor Code (see enum #cppc_code)
     *                              - CPPC_RD_WR_CP_MEAS_CL_CFG: CP Measurement Client Char. Configuration
     *                              - CPPC_RD_WR_CP_MEAS_SV_CFG: CP Measurement Server Char. Configuration
     *                              - CPPC_RD_WR_VECTOR_CFG:     Vector Client Char. Configuration
     *                              - CPPC_RD_WR_CTNL_PT_CFG:    CP Control Point Client Char. Configuration
     *
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t desc_code);

    /**
     ****************************************************************************************
     * @brief Function called when CP measurement information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_meas         Pointer to CP measurement information
     ****************************************************************************************
     */
    void (*cb_meas)(uint8_t conidx, const cpp_cp_meas_t* p_meas);

    /**
     ****************************************************************************************
     * @brief Function called when CP vector information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_vector       Pointer to CP vector information
     ****************************************************************************************
     */
    void (*cb_vector)(uint8_t conidx, const cpp_cp_vector_t* p_vector);

    /**
     ****************************************************************************************
     * @brief Completion of control point request procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the Request Send (see enum #hl_err)
     * @param[in] req_op_code   Requested Operation Code see enum #cpp_ctnl_pt_code
     * @param[in] resp_value    Response Value see enum #cpp_ctnl_pt_resp_val
     * @param[in] p_value       Pointer to response data content
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req_cmp)(uint8_t conidx, uint16_t status, uint8_t req_op_code, uint8_t resp_value,
                               const union cpp_ctnl_pt_rsp_val* p_value);
} cppc_cb_t;

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
 * @param[in] p_cps         Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t cppc_enable(uint8_t conidx, uint8_t con_type, const cppc_cps_content_t* p_cps);

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
uint16_t cppc_read_sensor_feat(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read sensor location procedure.
 *
 * Wait for #cb_read_loc_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t cppc_read_sensor_loc(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read Characteristic Configuration procedure.
 *
 * Wait for #cb_read_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] desc_code     Descriptor Code (see enum #cppc_code)
 *                              - CPPC_RD_WR_CP_MEAS_CL_CFG: CP Measurement Client Char. Configuration
 *                              - CPPC_RD_WR_CP_MEAS_SV_CFG: CP Measurement Server Char. Configuration
 *                              - CPPC_RD_WR_VECTOR_CFG:     Vector Client Char. Configuration
 *                              - CPPC_RD_WR_CTNL_PT_CFG:    CP Control Point Client Char. Configuration
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t cppc_read_cfg(uint8_t conidx, uint8_t desc_code);

/**
 ****************************************************************************************
 * @brief Perform a write Characteristic Configuration procedure.
 *
 * Wait for #cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] desc_code     Descriptor Code (see enum #cppc_code)
 *                              - CPPC_RD_WR_CP_MEAS_CL_CFG: CP Measurement Client Char. Configuration
 *                              - CPPC_RD_WR_CP_MEAS_SV_CFG: CP Measurement Server Char. Configuration
 *                              - CPPC_RD_WR_VECTOR_CFG:     Vector Client Char. Configuration
 *                              - CPPC_RD_WR_CTNL_PT_CFG:    CP Control Point Client Char. Configuration
 * @param[in] cfg_val       Configuration value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t cppc_write_cfg(uint8_t conidx, uint8_t desc_code, uint16_t cfg_val);

/**
 ****************************************************************************************
 * @brief Function called to send a control point request
 *
 * Wait for #cb_ctnl_pt_req_cmp execution before starting a new procedure
 *
 * @param[in] conidx         Connection index
 * @param[in] req_op_code    Requested Operation Code see enum #cpp_ctnl_pt_code
 * @param[in] p_value        Pointer to request data content
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t cppc_ctnl_pt_req(uint8_t conidx, uint8_t req_op_code, const union cpp_ctnl_pt_req_val* p_value);

/// @} CPPC

#endif //(_CPPC_H_)
