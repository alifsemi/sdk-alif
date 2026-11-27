/**
 ****************************************************************************************
 *
 * @file pasps.h
 *
 * @brief Phone Alert Status Profile Server - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef PASPS_H_
#define PASPS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup PASPS Task
 * @ingroup Profile
 * @brief Phone Alert Status Profile Server - Native API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "pasps_msg.h"

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

/// Phone Alert service server callback set
typedef struct pasps_cb
{
    /**
     ****************************************************************************************
     * @brief Inform that bond data updated for the connection.
     *
     * @param[in] conidx        Connection index
     * @param[in] char_code     Characteristic Code
     *                              - PASPS_ALERT_STATUS_NTF_CFG
     *                              - PASPS_RINGER_SETTING_NTF_CFG
     * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t char_code, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of alert status update (and registered peer device notification)
     *
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_alert_status_upd_cmp)(uint16_t status);

    /**
     ****************************************************************************************
     * @brief Completion of ringer setting update (and registered peer device notification)
     *
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_ringer_setting_upd_cmp)(uint16_t status);

    /**
     ****************************************************************************************
     * @brief Inform that peer device requests an action using control point
     *
     * @param[in] conidx         Connection index
     * @param[in] ringer_ctnl_pt Ringer Control Point value
     ****************************************************************************************
     */
    void (*cb_ctnl_pt)(uint8_t conidx, uint8_t ringer_ctnl_pt);

} pasps_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx                  Connection index
 * @param[in] alert_status_ntf_cfg    Alert Status Characteristic Notification Configuration
 * @param[in] ringer_setting_ntf_cfg  Ringer Setting Characteristic Notification Configuration
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t pasps_enable(uint8_t conidx, uint16_t alert_status_ntf_cfg, uint16_t ringer_setting_ntf_cfg);

/**
 ****************************************************************************************
 * @brief Update alert status and notify registered clients
 *
 * Wait for #cb_alert_status_upd_cmp execution before starting a new procedure
 *
 * @param[in] conidx_bf        Bit field of connection index on which notification will be send
 *                             will be mask according to client that supports reception
 * @param[in] value            New alert status
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t pasps_alert_status_upd(uint32_t conidx_bf, uint8_t value);

/**
 ****************************************************************************************
 * @brief Update ringer setting and notify registered clients
 *
 * Wait for #cb_ringer_setting_upd_cmp execution before starting a new procedure
 *
 * @param[in] conidx_bf      Bit field of connection index on which notification will be send
 *                           will be mask according to client that supports reception
 * @param[in] value          New ringer setting value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t pasps_ringer_setting_upd(uint32_t conidx_bf, uint8_t value);


/// @} PASPS

#endif // PASPS_H_
