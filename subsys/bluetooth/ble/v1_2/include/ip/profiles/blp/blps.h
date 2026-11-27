/**
 ****************************************************************************************
 *
 * @file blps.h
 *
 * @brief Header file - Blood Pressure Profile Sensor - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _BLPS_H_
#define _BLPS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup BLPS
 * @ingroup Profile
 * @brief Blood Pressure Profile Sensor - Native API.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "blps_msg.h"

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

/// Blood pressure service server callback set
typedef struct blps_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of blood pressure measurement transmission
     *
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_meas_send_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Inform that bond data updated for the connection.
     *
     * @param[in] conidx        Connection index
     * @param[in] char_code     Own code for differentiating between Blood Pressure Measurement,
     *                          and Intermediate Cuff Pressure Measurement characteristics
     * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t char_code, uint16_t cfg_val);
} blps_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
****************************************************************************************
* @brief Restore bond data of a known peer device (at connection establishment)
*
* @param[in] conidx            Connection index
* @param[in] bp_meas_ind_en    Blood Pressure indication configuration
* @param[in] interm_cp_ntf_en  Intermediate Cuff Pressure Notification configuration
*
* @return Status of the function execution (see enum #hl_err)
****************************************************************************************
*/
uint16_t blps_enable(uint8_t conidx, uint16_t bp_meas_ind_en, uint16_t interm_cp_ntf_en);

/**
****************************************************************************************
* @brief Make BCS sensor indicate it's peer device with the measurement
*
* Wait for #cb_meas_send_cmp execution before starting a new procedure
*
* @param[in] conidx         Connection index
* @param[in] stable         Flag indicating if it is a intermediary cuff pressure measurement (false)
*                           or stable blood pressure measurement (true).
* @param[in] p_meas         Pointer to the blood pressure measurement
*
* @return Status of the function execution (see enum #hl_err)
****************************************************************************************
*/
uint16_t blps_meas_send(uint8_t conidx, bool stable, const bps_bp_meas_t* p_meas);

/// @} BLPS

#endif /* _BLPS_H_ */
