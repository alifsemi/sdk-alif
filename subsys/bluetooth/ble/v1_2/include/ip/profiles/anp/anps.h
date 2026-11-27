/**
 ****************************************************************************************
 *
 * @file anps_msg.h
 *
 * @brief Header file - Alert Notification Profile Server - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ANPS_H_
#define ANPS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup ANPS
 * @ingroup Profile
 * @brief Alert Notification Profile Server - Native API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "anps_msg.h"


/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Alert notification server callback set
typedef struct anps_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of New Alert information Update procedure
     *
     * @param[in] conidx Connection index
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_new_alert_upd_cmp)(uint8_t conidx, uint16_t status);
    /**
     ****************************************************************************************
     * @brief Completion of Unread Alert status Update procedure
     *
     * @param[in] conidx Connection index
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_unread_alert_status_upd_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief The peer device requests to be notified about new alert values
     *
     * @param[in] conidx        Connection index
     * @param[in] alert_type    Alert type (New Alert or Unread Alert Status)
     * @param[in] cat_id_mask_0 Category ID Bit Mask 0
     * @param[in] cat_id_mask_1 Category ID Bit Mask 1
     ****************************************************************************************
     */
    void (*cb_ntf_immediate_req)(uint8_t conidx, uint8_t alert_type, uint8_t cat_id_mask_0, uint8_t cat_id_mask_1);

    /**
     ****************************************************************************************
     * @brief  Indicate that the notification configuration has been modified by the peer device
     *         (must be considered as bond data)
     *
     * @param[in] conidx        Connection index
     * @param[in] alert_type    Alert type (New Alert or Unread Alert Status)
     * @param[in] ntf_ind_cfg   Client Characteristic Configuration Descriptor Status
     * @param[in] cat_id_mask_0 Category ID Bit Mask 0
     * @param[in] cat_id_mask_1 Category ID Bit Mask 1
     ****************************************************************************************
     */
    void (*cb_ntf_status_upd)(uint8_t conidx, uint8_t alert_type, uint16_t ntf_ccc_cfg,
                              uint8_t cat_id_mask_0, uint8_t cat_id_mask_1);
} anps_cb_t;


/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx                        Connection index
 * @param[in] new_alert_ntf_cfg             New Alert Characteristic - Saved Client Characteristic
 *                                          Configuration Descriptor Value
 * @param[in] unread_alert_status_ntf_cfg   Unread Alert Status Characteristic - Saved Client
 *                                          Characteristic Configuration Descriptor Value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t anps_enable(uint8_t conidx, uint16_t new_alert_ntf_cfg, uint16_t unread_alert_status_ntf_cfg);

/**
 ****************************************************************************************
 * @brief Update New Alert information
 *
 * Wait for #cb_new_alert_upd_cmp execution before sending new update
 *
 * @param[in] conidx        Connection index
 * @param[in] p_new_alert   Pointer to new alert information
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t anps_new_alert_upd(uint8_t conidx, const anp_new_alert_t* p_new_alert);

/**
 ****************************************************************************************
 * @brief Update Unread Alert status
 *
 * Wait for #cb_nunread_alert_status_upd_cmp execution before sending new update
 *
 * @param[in] conidx                Connection index
 * @param[in] p_unread_alert_status Pointer to Unread Alert Status information
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t anps_unread_alert_status_upd(uint8_t conidx, const anp_unread_alert_t* p_unread_alert_status);

/// @} ANPS

#endif //(ANPS_H_)
