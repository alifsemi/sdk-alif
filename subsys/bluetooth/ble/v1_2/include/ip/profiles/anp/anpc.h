/**
 ****************************************************************************************
 *
 * @file anpc_msg.h
 *
 * @brief Alert Notification Profile Client - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _ANPC_H_
#define _ANPC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup ANPC
 * @ingroup Profile
 * @brief Alert Notification Profile Client - Native API.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "anpc_msg.h"

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Alert notification client callback set
typedef struct anpc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion client enable
     *
     * @param[in] conidx Connection index
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     * @param[in] p_ans  Pointer to peer database description (Bond Data)
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const anpc_ans_content_t* p_ans);

    /**
     ****************************************************************************************
     * @brief Completion of Read procedure
     *
     * @param[in] conidx    Connection index
     * @param[in] status    Status of the procedure execution (see enum #hl_err)
     * @param[in] read_code Read code (see enum #anpc_rd_wr_ntf_codes)
     ****************************************************************************************
     */
    void (*cb_read_cmp)(uint8_t conidx, uint16_t status, uint8_t read_code);

    /**
     ****************************************************************************************
     * @brief Completion of Write procedure
     *
     * @param[in] conidx     Connection index
     * @param[in] status     Status of the procedure execution (see enum #hl_err)
     * @param[in] write_code Write code (see enum #anpc_rd_wr_ntf_codes)
     ****************************************************************************************
     */
    void (*cb_write_cmp)(uint8_t conidx, uint16_t status, uint8_t write_code);

    /**
     ****************************************************************************************
     * @brief Receive list of supported categories
     *
     * @param[in] conidx        Connection index
     * @param[in] char_code     Characteristic code (see enum #anpc_rd_wr_ntf_codes)
     * @param[in] cat_id_mask_0 Category ID Bit Mask 0
     * @param[in] cat_id_mask_1 Category ID Bit Mask 1
     ****************************************************************************************
     */
    void (*cb_supp_cat)(uint8_t conidx, uint8_t char_code, uint8_t cat_id_mask_0, uint8_t cat_id_mask_1);

    /**
     ****************************************************************************************
     * @brief Receive Update of new alert
     *
     * @param[in] conidx       Connection index
     * @param[in] cat_id       Category ID
     * @param[in] nb_alert     Number of alerts
     * @param[in] info_len     Text String Information length
     * @param[in] p_info       Text String Information
     ****************************************************************************************
     */
    void (*cb_new_alert_upd)(uint8_t conidx, uint8_t cat_id, uint8_t nb_alert, uint8_t info_len, const uint8_t* p_info);

    /**
     ****************************************************************************************
     * @brief Receive Update of Unread alert
     *
     * @param[in] conidx      Connection index
     * @param[in] cat_id      Category ID
     * @param[in] nb_alert    Number of alerts
     ****************************************************************************************
     */
    void (*cb_unread_alert_upd)(uint8_t conidx, uint8_t cat_id, uint8_t nb_alert);

    /**
     ****************************************************************************************
     * @brief Receive value of Notification configuration
     *
     * @param[in] conidx      Connection index
     * @param[in] char_code   Characteristic code (see enum #anpc_rd_wr_ntf_codes)
     * @param[in] ntf_cfg     Notification configuration value
     ****************************************************************************************
     */
    void (*cb_ntf_cfg)(uint8_t conidx, uint8_t char_code, uint16_t ntf_cfg);
} anpc_cb_t;


/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Enable Client - Perform a discovery or restore bond data
 *
 * @param[in] conidx                Connection index
 * @param[in] con_type              Connection type
 * @param[in] p_ans                 Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t anpc_enable(uint8_t conidx, uint8_t con_type, const anpc_ans_content_t* p_ans);

/**
 ****************************************************************************************
 * @brief Read information about peer device
 *
 * Wait for #cb_read_cmp before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] read_code     Read code (see enum #anpc_rd_wr_ntf_codes)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t anpc_read(uint8_t conidx, uint8_t read_code);

/**
 ****************************************************************************************
 * @brief Update notification configuration of New Alert
 *
 * Wait for #cb_write_cmp before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] ntf_cfg       Notification configuration
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t anpc_write_new_alert_ntf_cfg(uint8_t conidx, uint16_t ntf_cfg);

/**
 ****************************************************************************************
 * @brief Update notification configuration of Unread Alert
 *
 * Wait for #cb_write_cmp before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] ntf_cfg       Notification configuration
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t anpc_write_unread_alert_ntf_cfg(uint8_t conidx, uint16_t ntf_cfg);

/**
 ****************************************************************************************
 * @brief Write Alert Notification Control Point
 *
 * Wait for #cb_write_cmp before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] cmd_id        Control point Command ID
 * @param[in] cat_id        Control point Category ID
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t anpc_write_ctnl_pt(uint8_t conidx, uint8_t cmd_id, uint8_t cat_id);

/// @} ANPC

#endif //(_ANPC_H_)
