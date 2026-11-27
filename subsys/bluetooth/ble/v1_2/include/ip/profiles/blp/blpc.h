/**
 ****************************************************************************************
 *
 * @file blpc.h
 *
 * @brief Header file - Blood Pressure Profile Collector Role - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 *
 ****************************************************************************************
 */

#ifndef _BLPC_H_
#define _BLPC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup BLPC
 * @ingroup Profile
 * @brief Blood Pressure Profile Collector  - Native API
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "blpc_msg.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Blood pressure service client callback set
typedef struct blpc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_bas         Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const bps_content_t* p_bps);

    /**
     ****************************************************************************************
     * @brief Inform that characteristic read procedure is over
     *
     * @param[in] conidx      Connection index
     * @param[in] status      Status of the procedure execution (see enum #hl_err)
     * @param[in] char_code   Characteristic value code
     * @param[in] length      Value length
     * @param[in] p_value     Pointer to value data
     ****************************************************************************************
     */
    void (*cb_read_char_cmp)(uint8_t conidx, uint16_t status, uint8_t char_code, uint16_t length, const uint8_t* p_value);

    /**
     ****************************************************************************************
     * @brief Inform that indication configuration write procedure is over
     *
     * @param[in] conidx        Connection index
     * @param[in] char_code     Own code for differentiating between blood pressure and
     *                          intermediate cuff pressure measurements
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_write_ntf_ind_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t char_code);

    /**
     ****************************************************************************************
     * @brief Function called when new blood pressure measurement information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] stable         Flag indicating if it is an intermediary cuff pressure measurement (false)
     *                           or stable blood pressure measurement (true).
     * @param[in] p_meas         Pointer to blood pressure measurement information
     ****************************************************************************************
     */
    void (*cb_meas)(uint8_t conidx, bool stable, const bps_bp_meas_t* p_meas);
} blpc_cb_t;

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
 * @param[in] p_bps         Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t blpc_enable(uint8_t conidx, uint8_t con_type, const bps_content_t* p_bps);

/**
 ****************************************************************************************
 * @brief Perform a characteristic read procedure.
 *
 * Wait for #cb_read_char_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] char_code     Characteristic value code
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t blpc_read_char(uint8_t conidx, uint8_t char_code);

/**
 ****************************************************************************************
 * @brief Perform indication configuration write procedure
 *
 * Wait for #cb_write_ntf_ind_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] char_code     Own code for differentiating between blood pressure and
 *                          intermediate cuff pressure measurements
 * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t blpc_write_ntf_ind_cfg(uint8_t conidx, uint8_t char_code, uint16_t cfg_val);

/// @} BLPC

#endif /* _BLPC_H_ */
