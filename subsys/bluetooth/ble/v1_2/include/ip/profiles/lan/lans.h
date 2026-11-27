/**
 ****************************************************************************************
 *
 * @file lans.h
 *
 * @brief Header file - Location and Navigation Profile Sensor - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _LANS_H_
#define _LANS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup LANS
 * @ingroup Profile
 * @brief Location and Navigation Profile - Native API.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "lans_msg.h"

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

/// Location and Navigation server callback set
typedef struct lans_cb
{

    /**
     ****************************************************************************************
     * @brief Inform that bond data updated for the connection.
     *
     * @param[in] conidx        Connection index
     * @param[in] char_code     Characteristic Code (Location and speed, Control Point or navigation)
     * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t char_code, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of location and speed transmission
     *
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_loc_speed_send_cmp)(uint16_t status);

    /**
     ****************************************************************************************
     * @brief Completion of navigation information transmission
     *
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_navigation_send_cmp)(uint16_t status);

    /**
     ****************************************************************************************
     * @brief Inform that peer device requests an action using control point
     *
     * @note control point request must be answered using #lans_ctnl_pt_rsp_send function
     *
     * @param[in] conidx        Connection index
     * @param[in] op_code       Operation Code (see #lanp_ln_ctnl_pt_code)
     * @param[in] p_value       Pointer to control point request value
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req)(uint8_t conidx, uint8_t op_code, const union lanp_ln_ctnl_pt_req_val* p_value);

    /**
     ****************************************************************************************
     * @brief Completion of control point response send procedure
     *
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_rsp_send_cmp)(uint8_t conidx, uint16_t status);

} lans_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx              Connection index
 * @param[in] csc_meas_ntf_cfg    Stored LAN Measurement Char. Client Characteristic Configuration
 * @param[in] prfl_ntf_ind_cfg    Profile characteristic configuration:
 *                                  - Bit 0: Location and speed Characteristic NTF config
 *                                  - Bit 1: LN Control Point Characteristic IND config
 *                                  - Bit 2: Navigation Characteristic NTF config
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lans_enable(uint8_t conidx, uint16_t prfl_ntf_ind_cfg);

/**
 ****************************************************************************************
 * @brief Send a Location and Speed to registered peer devices
 *
 * Wait for #cb_loc_speed_send execution before starting a new procedure
 *
 * @param[in] conidx_bf        Bit field of connection index on which measurement will be send
 *                             will be mask according to client that supports reception
 * @param[in] p_loc_speed      Pointer to location and speed parameters
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lans_loc_speed_send(uint32_t conidx_bf, const lanp_loc_speed_t* p_loc_speed);

/**
 ****************************************************************************************
 * @brief Send a navigation to registered peer devices
 *
 * Wait for #cb_navigation_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx_bf        Bit field of connection index on which measurement will be send
 *                             will be mask according to client that supports reception
 * @param[in] p_nav            Pointer to navigation parameters
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lans_navigation_send(uint32_t conidx_bf, const lanp_navigation_t* p_nav);

/**
 ****************************************************************************************
 * @brief Update position quality parameters
 *
 * @param[in] p_pos_q         Pointer to position quality  parameters
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lans_pos_q_upd(const lanp_posq_t* p_pos_q);

/**
 ****************************************************************************************
 * @brief Send control point response.
 *
 * Wait for #cb_ctnl_pt_rsp_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] op_code       Operation Code (see enum #lanp_ln_ctnl_pt_code)
 * @param[in] resp_val      Control point response value (see enum #lanp_ctnl_pt_resp_val)
 * @param[in] p_value       Pointer to control point response value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t lans_ctnl_pt_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t resp_val,
                                const union lanp_ctnl_pt_rsp_val* p_value);

/// @} LANS

#endif //(_LANS_MSG_H_)
