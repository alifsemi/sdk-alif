/**
 ****************************************************************************************
 *
 * @file hrps.h
 *
 * @brief Header file - Heart Rate Profile Sensor - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _HRPS_H_
#define _HRPS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup HRPS
 * @ingroup Profile
 * @brief Heart Rate Profile sensor - Native API.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "hrps_msg.h"


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

/// Heart rate sensor server callback set
typedef struct hrps_cb
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
     * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Inform APP that Energy Expanded must be reset value
     *
     * @param[in] conidx        Connection index
     ****************************************************************************************
     */
    void (*cb_energy_exp_reset)(uint8_t conidx);
} hrps_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx              Connection index
 * @param[in] hr_meas_ntf         Heart Rate Notification configuration
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t hrps_enable(uint8_t conidx, uint16_t hr_meas_ntf);

/**
 ****************************************************************************************
 * @brief Send a measurement to registered peer devices
 *
 * Wait for #cb_meas_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx_bf        Bit field of connection index on which measurement will be send
 *                             will be mask according to client that supports measurement reception
 * @param[in] p_meas           Pointer to the Heart Rate measurement
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t hrps_meas_send(uint32_t conidx_bf, const hrs_hr_meas_t* p_meas);


/// @} HRPS

#endif /* _HRPS_H_ */
