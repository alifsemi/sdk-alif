/**
 ****************************************************************************************
 *
 * @file scppc_msg.h
 *
 * @brief Header file - Scan Parameters Profile Client Role - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _SCPPC_H_
#define _SCPPC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup SCPPC
 * @ingroup SCPPC
 * @brief Scan Parameters Profile Client - Native API.
 *
 * @ref SCPPC monitor block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "scppc_msg.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Scan parameters profile client callback set
typedef struct scppc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_scps        Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const scps_content_t* p_scps);

    /**
     ****************************************************************************************
     * @brief Completion of read Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] ntf_cfg       Notification Configuration Value
     ****************************************************************************************
     */
    void (*cb_read_cfg_cmp)(uint8_t conidx, uint16_t status, uint16_t ntf_cfg);

    /**
     ****************************************************************************************
     * @brief Completion of write Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     *
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Completion of scan interval and window update procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_scan_intv_wd_upd_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Function called when peer device asks for a scan parameter refresh
     *
     * @param[in] conidx        Connection index
     ****************************************************************************************
     */
    void (*cb_scan_refresh)(uint8_t conidx);
} scppc_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * Wait for #cb_enable_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] con_type      Connection type
 * @param[in] p_scps        Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t scppc_enable(uint8_t conidx, uint8_t con_type, const scps_content_t* p_scps);

/**
 ****************************************************************************************
 * @brief Perform a read Characteristic Configuration procedure.
 *
 * Wait for #cb_read_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t scppc_read_cfg(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a write Characteristic Configuration procedure.
 *
 * Wait for #cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] ntf_cfg       Notification Configuration Value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t scppc_write_cfg(uint8_t conidx, uint16_t ntf_cfg);

/**
 ****************************************************************************************
 * @brief Function called to update peer scan interval and window parameters
 *
 * Wait for #cb_scan_intv_wd_upd_cmp execution before starting a new procedure
 *
 * @param[in] conidx            Connection index
 * @param[in] p_scan_intv_wd    Pointer to scan interval and window parameters
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t scppc_scan_intv_wd_upd(uint8_t conidx, const scpp_scan_intv_wd_t* p_scan_intv_wd);

/// @} SCPPC

#endif /* _SCPPC_H_ */
