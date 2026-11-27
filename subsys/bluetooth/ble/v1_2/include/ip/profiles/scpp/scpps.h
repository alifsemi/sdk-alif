/**
 ****************************************************************************************
 *
 * @file scpps.h
 *
 * @brief Header file - Scan Parameters Profile Server - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _SCPPS_H_
#define _SCPPS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup SCPPS
 * @ingroup Profile
 * @brief Scan Parameters Profile Server - Native API
 *
 * @ref SCPPS block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "scpps_msg.h"


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

/// Scan parameters profile server callback set
typedef struct scpps_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of scan refresh notification procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_scan_refresh_send_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Inform that bond data updated for the connection.
     *
     * @param[in] conidx        Connection index
     * @param[in] ntf_cfg             Scan Refresh Notification Configurations
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t ntf_cfg);

    /**
     ****************************************************************************************
     * @brief Inform that peer device has updated scan interval and scan window parameters
     *
     * @param[in] conidx         Connection index
     * @param[in] p_scan_intv_wd Pointer to Scan Interval Window parameters
     ****************************************************************************************
     */
    void (*cb_scan_intv_wd_upd)(uint8_t conidx, const scpp_scan_intv_wd_t* p_scan_intv_wd);
} scpps_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx              Connection index
 * @param[in] ntf_cfg             Scan Refresh Notification Configurations
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t scpps_enable(uint8_t conidx, uint8_t ntf_cfg);

/**
 ****************************************************************************************
 * @brief Send a scan refresh notification to peer device.
 *
 * Wait for #cb_scan_refresh_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx              Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t scpps_scan_refresh_send(uint8_t conidx);

/// @} SCPPS

#endif //(_SCPPS_H_)
