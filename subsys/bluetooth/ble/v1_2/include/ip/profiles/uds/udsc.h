/**
 ****************************************************************************************
 *
 * @file udsc_msg.h
 *
 * @brief Header file - User Data Service Collector/Client Role - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _UDSC_H_
#define _UDSC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup UDSC
 * @ingroup Profile
 * @brief  User Data Service Profile Collector - Native API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "udsc_msg.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Buffer header size that must be present in buffer provided by application
#define UDSC_BUFFER_HEADER_LEN   (4 + GATT_BUFFER_HEADER_LEN)

/// Buffer tail size that must be present in buffer provided by application
#define UDSC_BUFFER_TAIL_LEN     (GATT_BUFFER_TAIL_LEN)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// User data service client callback set
typedef struct udsc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] p_uds         Pointer to peer database description bond data
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status, const udsc_uds_content_t* p_uds);

    /**
     ****************************************************************************************
     * @brief Completion of read information procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] val_id        Value identifier (see enum #uds_val_id)
     * @param[in] p_value       Pointer that contains non-UTF-8 value.
     *                          (NULL for a UTF-8 value)
     * @param[in] p_utf8_name   Pointer to buffer that contains UTF-8 name.
     *                          (NULL for an non-UTF-8 value)
     *
     ****************************************************************************************
     */
    void (*cb_read_cmp)(uint8_t conidx, uint16_t status, uint8_t val_id, const union uds_value* p_value,
                        co_buf_t* p_utf8_name);


    /**
     ****************************************************************************************
     * @brief Completion of write information procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] val_id        Value identifier (see enum #uds_val_id)
     *
     ****************************************************************************************
     */
    void (*cb_write_cmp)(uint8_t conidx, uint16_t status, uint8_t val_id);

    /**
     ****************************************************************************************
     * @brief Completion of read Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] val_id        CCC value identifier (see enum #uds_val_id)
     *                              - UDS_CHAR_ID_DB_CHG_INC
     *                              - UDS_CHAR_ID_USER_CTRL_PT
     * @param[in] cfg_val       Configuration value
     *
     ****************************************************************************************
     */
    void (*cb_read_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t val_id, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of write Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] val_id        CCC value identifier (see enum #uds_val_id)
     *                              - UDS_CHAR_ID_DB_CHG_INC
     *                              - UDS_CHAR_ID_USER_CTRL_PT
     *
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t val_id);

    /**
     ****************************************************************************************
     * @brief Function called when Database change increment update is received
     *
     * @param[in] conidx         Connection index
     * @param[in] value          Database change increment value
     ****************************************************************************************
     */
    void (*cb_db_chg_inc)(uint8_t conidx, uint32_t value);

    /**
     ****************************************************************************************
     * @brief Completion of control point request procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the Request Send (see enum #hl_err)
     * @param[in] req_op_code   Requested Operation Code see enum #uds_ctrl_pt_response
     * @param[in] resp_value    Response Value see enum #uds_ctrl_pt_response
     * @param[in] user_id       User ID used for register new user command
     * @param[in] p_param       Pointer buffer value that contains response parameters
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req_cmp)(uint8_t conidx, uint16_t status, uint8_t req_op_code, uint8_t resp_value,
                               uint8_t user_id, co_buf_t* p_param);
} udsc_cb_t;

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
 * @param[in] p_uds         Pointer to peer database description bond data
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udsc_enable(uint8_t conidx, uint8_t con_type, const udsc_uds_content_t* p_uds);

/**
 ****************************************************************************************
 * @brief Perform a read information procedure.
 *
 * Wait for #cb_read_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] val_id        Value identifier (see enum #uds_val_id)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udsc_read(uint8_t conidx, uint8_t val_id);

/**
 ****************************************************************************************
 * @brief Perform an information update procedure.
 *
 * Wait for #cb_write_cmp execution before starting a new procedure
 *
 * @note Minimum buffer head: #UDS_BUFFER_HEADER_LEN,
 *       and minimum buffer tail: #UDS_BUFFER_TAIL_LEN
 *
 * @param[in] conidx        Connection index
 * @param[in] val_id        Value identifier (see enum #uds_val_id)
 * @param[in] p_value       Pointer that contains non-UTF-8 value.
 *                          (NULL for a UTF-8 value)
 * @param[in] p_utf8_name   Pointer to buffer that contains UTF-8 name.
 *                          (NULL for an non-UTF-8 value)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udsc_write(uint8_t conidx, uint8_t val_id, const union uds_value* p_value, co_buf_t* p_utf8_name);

/**
 ****************************************************************************************
 * @brief Perform a read Characteristic Configuration procedure.
 *
 * Wait for #cb_read_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] val_id        CCC value identifier (see enum #uds_val_id)
 *                              - UDS_CHAR_ID_DB_CHG_INC
 *                              - UDS_CHAR_ID_USER_CTRL_PT
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udsc_read_cfg(uint8_t conidx, uint8_t val_id);

/**
 ****************************************************************************************
 * @brief Perform a write Characteristic Configuration procedure.
 *
 * Wait for #cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] val_id        CCC value identifier (see enum #uds_val_id)
 *                              - UDS_CHAR_ID_DB_CHG_INC
 *                              - UDS_CHAR_ID_USER_CTRL_PT
 * @param[in] cfg_val       Configuration value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udsc_write_cfg(uint8_t conidx, uint8_t val_id, uint16_t cfg_val);

/**
 ****************************************************************************************
 * @brief Function called to send a control point request
 *
 * Wait for #cb_ctnl_pt_req_cmp execution before starting a new procedure
 *
 * @note Minimum buffer head: #UDS_BUFFER_HEADER_LEN,
 *       and minimum buffer tail: #UDS_BUFFER_TAIL_LEN
 *
 * @param[in] conidx         Connection index
 * @param[in] req_op_code    Requested Operation Code see enum #uds_ctrl_pt_response
 * @param[in] user_id        User index
 * @param[in] consent        Used for sending consent command
 * @param[in] p_param        Pointer buffer value that contains response parameters
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udsc_ctnl_pt_req(uint8_t conidx, uint8_t req_op_code, uint8_t user_id, uint16_t consent, co_buf_t* p_param);

/// @} UDSC

#endif //(_UDSC_H_)
