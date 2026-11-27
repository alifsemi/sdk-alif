/**
 ****************************************************************************************
 *
 * @file tips_msg.h
 *
 * @brief Header file - Time Profile Server - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TIPS_MSG_H_
#define TIPS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TIPS_API_MSG Message API
 * @ingroup TIPS_API
 * @brief Description of Message API for Time Profile Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tips.h"
#include "rwip_task.h"

/// @addtogroup TIPS_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs
enum tips_msg_id
{
    /// Start the Time Profile Server Role - at connection
    TIPS_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_TIPS),
    /// Inform the application about the task creation result
    TIPS_ENABLE_RSP,
    /// Update Current Time Request from APP
    TIPS_UPD_CURR_TIME_CMD,
    /// Inform APP about a requested read
    TIPS_RD_REQ_IND,
    /// Read request Response
    TIPS_RD_CFM,
    /// Inform APP about modification of Current Time Characteristic Client. Charact. Cfg
    TIPS_CURRENT_TIME_CCC_IND,
    /// Inform APP about modification of Time Update Control Point Characteristic Value
    TIPS_TIME_UPD_CTNL_PT_IND,
    /// Complete Event Information
    TIPS_CMP_EVT
};

/// Operation codes
enum tips_op_codes
{
    /// Database Creation Procedure
    TIPS_RESERVED_OP_CODE = 0,
    /// Current Time notify Operation Code
    TIPS_UPD_CURR_TIME_CMD_OP_CODE,
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #TIPS_ENABLE_REQ message
typedef struct tips_enable_req
{
    /// Connection index
    uint8_t conidx;
    /// Current Time notification configuration
    uint16_t current_time_ntf_en;
} tips_enable_req_t;

/// Parameters of the #TIPS_ENABLE_RSP message
typedef struct tips_enable_rsp
{
    /// Connection index
    uint8_t conidx;
    /// Status (see #hl_err enumeration)
    uint16_t status;
} tips_enable_rsp_t;

/// Parameters of the #TIPS_UPD_CURR_TIME_CMD message
typedef struct tips_upd_curr_time_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Current Time
    tip_curr_time_t current_time;
    /**
     * Indicate if the new Current Time value can be sent if the current_time_ntf_en parameter is
     * set to 1.
     * (0 - Disable; 1- Enable)
     *
     * If the time of the Current Time Server is changed because of reference time update (adjust reason)
     * then no notifications shall be sent to the Current Time Service Client within the 15 minutes from
     * the last notification, unless one of both of the two statements below are true :
     *         - The new time information differs by more than 1min from the Current Time Server
     *         time previous to the update
     *         - The update was caused by the client (interacting with another service)
     */
    uint8_t enable_ntf_send;
} tips_upd_curr_time_cmd_t;

/// Parameters of the #TIPS_RD_REQ_IND message
typedef struct tips_rd_req_ind
{
    /// Connection index
    uint8_t conidx;
    /// Value identifier (see #tip_value_id enumeration)
    uint8_t val_id;
    /// Token value that must be provided in confirmation
    uint32_t token;
} tips_rd_req_ind_t;

/// Parameters of the #TIPS_RD_CFM message
typedef struct tips_rd_cfm
{
    /// Connection index
    uint8_t conidx;
    /// Value identifier (see #tip_value_id enumeration)
    uint8_t val_id;
    /// Token value provided in request
    uint32_t token;
    /// Read value data
    union tip_value value;
} tips_rd_cfm_t;

/// Parameters of the #TIPS_CURRENT_TIME_CCC_IND message
typedef struct tips_current_time_ccc_ind
{
    /// Connection index
    uint8_t conidx;
    /// Configuration value
    uint16_t cfg_val;
} tips_current_time_ccc_ind_t;

/// Parameters of the #TIPS_TIME_UPD_CTNL_PT_IND message
typedef struct tips_time_upd_ctnl_pt_ind
{
    /// Connection index
    uint8_t conidx;
    /// Time Update Control Point value
    tip_time_upd_contr_pt_t value;
} tips_time_upd_ctnl_pt_ind_t;

/// Parameters of the #TIPS_CMP_EVT message
typedef struct tips_cmp_evt
{
    /// Connection index
    uint8_t conidx;
    /// Operation (see #tips_op_codes enumeration)
    uint8_t operation;
    /// Status (see #hl_err enumeration)
    uint16_t status;
} tips_cmp_evt_t;

/// @} TIPS_API_MSG

#endif // TIPS_MSG_H_
