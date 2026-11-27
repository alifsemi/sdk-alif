/**
 ****************************************************************************************
 *
 * @file wscs.h
 *
 * @brief Header file - Weight SCale Profile Sensor - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _WSCS_H_
#define _WSCS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup WSCS
 * @ingroup Profile
 * @brief Weight SCale Profile - Native API.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "wscs_msg.h"

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

/// Weight scale service server callback set
typedef struct wscs_cb
{
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
     * @brief Completion of measurement transmission
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_meas_send_cmp)(uint8_t conidx, uint16_t status);
} wscs_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx            Connection index
 * @param[in] ind_cfg           CCC for the current connection
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t wscs_enable(uint8_t conidx, uint16_t ind_cfg);

/**
 ****************************************************************************************
 * @brief Send a measurement data to peer device
 *
 * Wait for #cb_meas_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx           Connection index
 * @param[in] p_meas           Pointer to the weight measurement data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t wscs_meas_send(uint8_t conidx, const wsc_meas_t* p_meas);

/// @} WSCS

#endif //(_WSCS_H_)
