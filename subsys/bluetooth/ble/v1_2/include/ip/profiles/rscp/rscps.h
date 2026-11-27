/**
 ****************************************************************************************
 *
 * @file rscps.h
 *
 * @brief Header file - Running Speed and Cadence Profile Sensor - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _RSCPS_H_
#define _RSCPS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup RSCPS
 * @ingroup Profile
 * @brief Running Speed and Cadence Profile - Native API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rscps_msg.h"


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

/// Running speed and cadence server callback set
typedef struct rscps_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of measurement transmission
     *
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_meas_send_cmp)(uint16_t status);

    /**
     ****************************************************************************************
     * @brief Inform that bond data updated for the connection.
     *
     * @param[in] conidx        Connection index
     * @param[in] char_code     Characteristic Code (RSC Measurement or SC Control Point)
     * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t char_code, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Inform that peer device requests an action using control point
     *
     * @note control point request must be answered using #rscps_ctnl_pt_rsp_send function
     *
     * @param[in] conidx        Connection index
     * @param[in] op_code       Operation Code (see #rscp_sc_ctnl_pt_op_code)
     * @param[in] p_value       Pointer to control point request value
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req)(uint8_t conidx, uint8_t op_code, const union rscp_sc_ctnl_pt_req_val* p_value);

    /**
     ****************************************************************************************
     * @brief Completion of control point response send procedure
     *
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_rsp_send_cmp)(uint8_t conidx, uint16_t status);

} rscps_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx              Connection index
 * @param[in] rsc_meas_ntf_cfg    Stored RSC Measurement Char. Client Characteristic Configuration
 * @param[in] sc_ctnl_pt_ntf_cfg  Stored SC Control Point Char. Client Characteristic Configuration
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t rscps_enable(uint8_t conidx, uint16_t rsc_meas_ntf_cfg, uint16_t sc_ctnl_pt_ntf_cfg);

/**
 ****************************************************************************************
 * @brief Send a measurement to registered peer devices
 *
 * Wait for #cb_meas_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx_bf        Bit field of connection index on which measurement will be send
 *                             will be mask according to client that supports measurement reception
 * @param[in] p_meas           Pointer to the RSC measurement
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t rscps_meas_send(uint32_t conidx_bf, const rscp_rsc_meas_t* p_meas);

/**
 ****************************************************************************************
 * @brief Send control point response.
 *
 * Wait for #cb_ctnl_pt_rsp_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] op_code       Operation Code (see enum #rscp_sc_ctnl_pt_op_code)
 * @param[in] resp_val      Control point response value (see enum #rscp_sc_ctnl_pt_resp_val)
 * @param[in] p_value       Pointer to control point response value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t rscps_ctnl_pt_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t resp_val,
                                const union rscp_sc_ctnl_pt_rsp_val* p_value);



/// @} RSCPS

#endif //(_RSCPS_H_)
