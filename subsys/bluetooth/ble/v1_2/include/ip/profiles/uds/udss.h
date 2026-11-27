/**
 ****************************************************************************************
 *
 * @file udss.h
 *
 * @brief Header file - User Data Service Profile - Native API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _UDSS_H_
#define _UDSS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup UDSS
 * @ingroup Profile
 * @brief User Data Service Profile Task - Native API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "udss_msg.h"


/*
 * DEFINES
 ****************************************************************************************
 */

/// Buffer header size that must be present in buffer provided by application
#define UDSS_BUFFER_HEADER_LEN   (3 + GATT_BUFFER_HEADER_LEN)

/// Buffer tail size that must be present in buffer provided by application
#define UDSS_BUFFER_TAIL_LEN     (GATT_BUFFER_TAIL_LEN)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// User data service server callback set
typedef struct udss_cb
{
    /**
     ****************************************************************************************
     * @brief Inform that bond data updated for the connection.
     *
     * @param[in] conidx        Connection index
     * @param[in] val_id        CCC value identifier (see enum #uds_val_id)
     *                              - UDS_CHAR_ID_DB_CHG_INC
     *                              - UDS_CHAR_ID_USER_CTRL_PT
     * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t val_id, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of Database change increment update procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_db_chg_inc_upd_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Function called when peer device wants to read user data information
     *
     * Read information must be returned by application using #udss_read_cfm
     *
     * @param[in] conidx        Connection index
     * @param[in] token         Token information that must be returned in confirmation
     * @param[in] val_id        Value identifier (see enum #uds_val_id)
     * @param[in] offset        Data offset (only valid for an UTF-8 string)
     * @param[in] max_len       Maximum string length to return (only valid for an UTF-8 string)
     ****************************************************************************************
     */
    void (*cb_read_req)(uint8_t conidx, uint32_t token, uint8_t val_id, uint16_t offset, uint16_t max_len);

    /**
     ****************************************************************************************
     * @brief Function called when peer device wants to modify user data information
     *
     * Write request must be confirmed by application using #udss_write_cfm
     *
     * @param[in] conidx        Connection index
     * @param[in] token         Token information that must be returned in confirmation
     * @param[in] val_id        Value identifier (see enum #uds_val_id)
     * @param[in] p_value       Pointer that contains non-UTF-8 value. (NULL for a UTF-8 value)
     * @param[in] p_utf8_name   Pointer to buffer that contains UTF-8 name. (NULL for an non-UTF-8 value)
     ****************************************************************************************
     */
    void (*cb_write_req)(uint8_t conidx, uint32_t token, uint8_t val_id, const union uds_value* p_value,
                         co_buf_t* p_utf8_name);

    /**
     ****************************************************************************************
     * @brief Inform that peer device requests an action using control point
     *
     * @note control point request must be answered using #udss_ctnl_pt_rsp_send function
     *
     * @param[in] conidx        Connection index
     * @param[in] op_code       Operation Code (see enum #uds_ctrl_pt_response)
     * @param[in] user_id       User index
     * @param[in] consent       Used for sending consent command
     * @param[in] p_param       Pointer buffer value that contains response parameters
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req)(uint8_t conidx, uint8_t op_code, uint8_t user_id, uint16_t consent, co_buf_t* p_param);

    /**
     ****************************************************************************************
     * @brief Completion of control point response send procedure
     *
     * @param[in] status Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_rsp_send_cmp)(uint8_t conidx, uint16_t status);

} udss_cb_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx            Connection index
 * @param[in] db_chg_inc_ccc    Database Change Increment CCC
 * @param[in] user_ctrl_pt_ccc  User Control Point CCC
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udss_enable(uint8_t conidx, uint16_t db_chg_inc_ccc, uint16_t user_ctrl_pt_ccc);

/**
 ****************************************************************************************
 * @brief Inform peer device about database change increment value
 *
 * Wait for #cb_db_chg_inc_upd execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] value            Database change increment value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udss_db_chg_inc_upd(uint8_t conidx, uint32_t value);

/**
 ****************************************************************************************
 * @brief Send back time information
 *
 * @note Minimum buffer head: #UDS_BUFFER_HEADER_LEN,
 *       and minimum buffer tail: #UDS_BUFFER_TAIL_LEN
 *
 * @param[in] conidx        Connection index
 * @param[in] token         Token information received in request
 * @param[in] val_id        Value identifier (see enum #uds_val_id)
 * @param[in] status        Status of the request at application level (see enum #hl_err)
 * @param[in] p_value       Pointer that contains non-UTF-8 value. (NULL for a UTF-8 value)
 * @param[in] total_len     Total size of UTF-8 string (without considering offset)
 * @param[in] p_utf8_name   Pointer to buffer that contains UTF-8 name starting from offset.
 *                          (NULL for an non-UTF-8 value)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udss_read_cfm(uint8_t conidx, uint32_t token, uint8_t val_id, uint16_t status, const union uds_value* p_value,
                       uint16_t total_len, co_buf_t* p_utf8_name);

/**
 ****************************************************************************************
 * @brief Send back time information
 *
 * @param[in] conidx        Connection index
 * @param[in] token         Token information received in request
 * @param[in] val_id        Value identifier (see enum #uds_val_id)
 * @param[in] status        Status of the request at application level (see enum #hl_err)
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udss_write_cfm(uint8_t conidx, uint32_t token, uint8_t val_id, uint16_t status);

/**
 ****************************************************************************************
 * @brief Send control point response.
 *
 * Wait for #cb_ctnl_pt_rsp_send_cmp execution before starting a new procedure
 *
 * @note Minimum buffer head: #UDS_BUFFER_HEADER_LEN,
 *       and minimum buffer tail: #UDS_BUFFER_TAIL_LEN
 *
 * @param[in] conidx        Connection index
 * @param[in] op_code       Operation Code (see enum #uds_ctrl_pt_response)
 * @param[in] resp_val      Control point response value (see enum #uds_ctrl_pt_response)
 * @param[in] p_param       Pointer buffer value that contains response parameters
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t udss_ctnl_pt_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t resp_val, co_buf_t* p_param);


/// @} UDSS

#endif //(_UDSS_H_)

