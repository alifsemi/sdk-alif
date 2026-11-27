/**
 ****************************************************************************************
 *
 * @file wscc.h
 *
 * @brief Header file - Weight SCale Profile Collector/Client Role - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _WSCC_H_
#define _WSCC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup WSCC
 * @ingroup Profile
 * @brief  Weight SCale ProfileProfile Collector - Native API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "wscc_msg.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Weight scale client callback set
typedef struct wscc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_wss         Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const wscc_wss_content_t* p_wss);

    /**
     ****************************************************************************************
     * @brief Completion of read feature procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] feature       Weight Scale Feature read value
     *
     ****************************************************************************************
     */
    void (*cb_read_feat_cmp)(uint8_t conidx, uint16_t status, uint32_t feature);

    /**
     ****************************************************************************************
     * @brief Completion of read Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] cfg_val       Configuration value
     *
     ****************************************************************************************
     */
    void (*cb_read_cfg_cmp)(uint8_t conidx, uint16_t status, uint16_t cfg_val);

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
     * @brief Function called when weight measurement information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_meas         Pointer to weight measurement data
     * @param[in] p_meas_prop    Pointer to measurement properties
     ****************************************************************************************
     */
    void (*cb_meas)(uint8_t conidx, const wsc_meas_t* p_meas, const wsc_meas_prop_t* p_meas_prop);
} wscc_cb_t;

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
 * @param[in] p_wss         Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t wscc_enable(uint8_t conidx, uint8_t con_type, const wscc_wss_content_t* p_wss);

/**
 ****************************************************************************************
 * @brief Perform a read feature procedure.
 *
 * Wait for #cb_read_feat_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t wscc_read_feat(uint8_t conidx);

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
uint16_t wscc_read_cfg(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a write Characteristic Configuration procedure.
 *
 * Wait for #cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] cfg_val       Configuration value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t wscc_write_cfg(uint8_t conidx, uint16_t cfg_val);

/// @} WSCC

#endif //(_WSCC_H_)
