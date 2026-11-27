/**
 ****************************************************************************************
 *
 * @file bcss_msg.h
 *
 * @brief Header file - Body Composition Service Sensor/Server Role - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _BCSS_H_
#define _BCSS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup BCSS
 * @ingroup Profile
 * @brief Body Composition Service Sensor/Server Role - Native API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bcss_msg.h"


/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Body composition service server callback set
typedef struct bcss_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of BCS sensor measurement transmission
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
     * @param[in] ind_cfg       Indication configuration
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t ind_cfg);
} bcss_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
****************************************************************************************
* @brief Restore bond data of a known peer device (at connection establishment)
*
* @param[in] conidx          Connection index
* @param[in] ind_cfg         CCC for the current connection
*
* @return Status of the function execution (see enum #hl_err)
****************************************************************************************
*/
uint16_t bcss_enable(uint8_t conidx, uint8_t ind_cfg);

/**
****************************************************************************************
* @brief Make BCS sensor indicate it's peer device with the measurement
*
* Wait for #cb_meas_send_cmp execution before starting a new procedure
*
* @param[in] conidx    Connection index
* @param[in] p_meas    Pointer to the measurement value
*
* @return Status of the function execution (see enum #hl_err)
****************************************************************************************
*/
uint16_t bcss_meas_send(uint8_t conidx, const bcs_meas_t* p_meas);

/// @} BCSS

#endif //(_BCSS_H_)
