/**
 ****************************************************************************************
 *
 * @file paspc.h
 *
 * @brief Header file - Phone Alert Status Profile Client Role - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _PASPC_H_
#define _PASPC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup PASPC
 * @ingroup PASPC
 * @brief Phone Alert Status Profile Client - Native API.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "paspc_msg.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Phone Alert service client callback set
typedef struct paspc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_pass        Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const paspc_pass_content_t* p_pass);

    /**
     ****************************************************************************************
     * @brief Completion of read alert status procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] value  Current alert status value
     *
     ****************************************************************************************
     */
    void (*cb_read_alert_status_cmp)(uint8_t conidx, uint16_t status, uint8_t value);

    /**
     ****************************************************************************************
     * @brief Completion of read ringer setting procedure.
     *
     * @param[in] conidx         Connection index
     * @param[in] status         Status of the procedure execution (see enum #hl_err)
     * @param[in] value          Current ringer setting value
     *
     ****************************************************************************************
     */
    void (*cb_read_ringer_setting_cmp)(uint8_t conidx, uint16_t status, uint8_t value);

    /**
     ****************************************************************************************
     * @brief Completion of read Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] desc_code     Descriptor Code (see enum #paspc_rd_wr_codes)
     *                              - PASPC_RD_WR_ALERT_STATUS_CFG: PAS Alert Status CCC Descriptor
     *                              - PASPC_RD_WR_RINGER_SETTING_CFG: PAS Ringer Setting CCC Descriptor
     * @param[in] cfg_val       Configuration value
     *
     ****************************************************************************************
     */
    void (*cb_read_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t desc_code, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of write Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] desc_code     Descriptor Code (see enum #paspc_rd_wr_codes)
     *                              - PASPC_RD_WR_ALERT_STATUS_CFG: PAS Alert Status CCC Descriptor
     *                              - PASPC_RD_WR_RINGER_SETTING_CFG: PAS Ringer Setting CCC Descriptor
     *
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t desc_code);

    /**
     ****************************************************************************************
     * @brief Function called when alert status update is received
     *
     * @param[in] conidx         Connection index
     * @param[in] value          Current alert status value
     ****************************************************************************************
     */
    void (*cb_alert_status)(uint8_t conidx, uint8_t value);

    /**
     ****************************************************************************************
     * @brief Function called when ringer setting update is received
     *
     * @param[in] conidx         Connection index
     * @param[in] value          Current ringer setting value
     ****************************************************************************************
     */
    void (*cb_ringer_setting)(uint8_t conidx, uint8_t value);

    /**
     ****************************************************************************************
     * @brief Completion of control point request procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the Request Send (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req_cmp)(uint8_t conidx, uint16_t status);
} paspc_cb_t;

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
 * @param[in] p_pass        Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t paspc_enable(uint8_t conidx, uint8_t con_type, const paspc_pass_content_t* p_pass);

/**
 ****************************************************************************************
 * @brief Perform a read alert status procedure.
 *
 * Wait for #cb_read_alert_status_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t paspc_read_alert_status(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read ringer setting procedure.
 *
 * Wait for #cb_read_ringer_setting_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t paspc_read_ringer_setting(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read Characteristic Configuration procedure.
 *
 * Wait for #cb_read_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] desc_code     Descriptor Code (see enum #paspc_rd_wr_codes)
 *                              - PASPC_RD_WR_ALERT_STATUS_CFG: PAS Alert Status CCC Descriptor
 *                              - PASPC_RD_WR_RINGER_SETTING_CFG: PAS Ringer Setting CCC Descriptor
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t paspc_read_cfg(uint8_t conidx, uint8_t desc_code);

/**
 ****************************************************************************************
 * @brief Perform a write Characteristic Configuration procedure.
 *
 * Wait for #cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] desc_code     Descriptor Code (see enum #paspc_rd_wr_codes)
 *                              - PASPC_RD_WR_ALERT_STATUS_CFG: PAS Alert Status CCC Descriptor
 *                              - PASPC_RD_WR_RINGER_SETTING_CFG: PAS Ringer Setting CCC Descriptor
 * @param[in] cfg_val       Configuration value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t paspc_write_cfg(uint8_t conidx, uint8_t desc_code, uint16_t cfg_val);

/**
 ****************************************************************************************
 * @brief Function called to send a control point request
 *
 * Wait for #cb_ctnl_pt_req_cmp execution before starting a new procedure
 *
 * @param[in] conidx         Connection index
 * @param[in] ringer_ctnl_pt Ringer Control Point value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t paspc_ctnl_pt_req(uint8_t conidx, uint8_t ringer_ctnl_pt);

/// @} PASPC

#endif //(_PASPC_H_)
