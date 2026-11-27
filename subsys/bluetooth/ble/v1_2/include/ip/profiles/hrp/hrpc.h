/**
 ****************************************************************************************
 *
 * @file hrpc.h
 *
 * @brief Header file - Heart Rate Profile Collector Role - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _HRPC_H_
#define _HRPC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup HRPC
 * @ingroup Profile
 * @brief Heart Rate Profile Collector  - Native API
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "hrpc_msg.h"


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Heart rate sensor client callback set
typedef struct hrpc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_hrs         Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const hrs_content_t* p_hrs);

    /**
     ****************************************************************************************
     * @brief Completion of read sensor location procedure.
     *
     * Wait for #cb_read_loc_cmp execution before starting a new procedure
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
     * @param[in] cfg_val       Configuration value
     *
     ****************************************************************************************
     */
    void (*cb_read_cfg_cmp)(uint8_t conidx, uint16_t status, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of write Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     *
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Function called when heart rate measurement information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_meas         Pointer to heart rate measurement information
     ****************************************************************************************
     */
    void (*cb_meas)(uint8_t conidx, const hrs_hr_meas_t* p_meas);

    /**
     ****************************************************************************************
     * @brief Completion of control point request procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the Request Send (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req_cmp)(uint8_t conidx, uint16_t status);
} hrpc_cb_t;

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
 * @param[in] p_hrs         Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t hrpc_enable(uint8_t conidx, uint8_t con_type, const hrs_content_t* p_hrs);

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
uint16_t hrpc_read_sensor_loc(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read Characteristic Configuration procedure.
 *
 * Wait for #cb_read_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t hrpc_read_cfg(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a write Characteristic Configuration procedure.
 *
 * Wait for #cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] cfg_val       Configuration value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t hrpc_write_cfg(uint8_t conidx, uint16_t cfg_val);

/**
 ****************************************************************************************
 * @brief Function called to send a control point request
 *
 * Wait for #cb_ctnl_pt_req_cmp execution before starting a new procedure
 *
 * @param[in] conidx         Connection index
 * @param[in] value          Control point command value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t hrpc_ctnl_pt_req(uint8_t conidx, uint8_t value);


/// @} HRPC

#endif /* _HRPC_H_ */
