/**
 ****************************************************************************************
 *
 * @file tipc_msg.h
 *
 * @brief Header file - Time Profile Client - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _TIPC_MSG_H_
#define _TIPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TIPC_API_MSG Message API
 * @ingroup TIPC_API
 * @brief Description of Message API for Time Profile Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tipc.h"
#include "rwip_task.h"

/// @addtogroup TIPC_API_MSG
/// @{

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Message IDs
/*@TRACE*/
enum tipc_msg_id
{
    /// Start the time profile - at connection
    TIPC_ENABLE_REQ = MSG_ID(TIPC, 0x00),
    /// Confirm that cfg connection has finished with discovery results, or that normal cnx started
    TIPC_ENABLE_RSP = MSG_ID(TIPC, 0x01),
    /// Generic message to read a CTS, NDCS or RTUS characteristic value
    TIPC_RD_CHAR_CMD = MSG_ID(TIPC, 0x02),
    /// Received read value
    TIPC_RD_CHAR_IND = MSG_ID(TIPC, 0x03),
    /// Generic message for configuring the Current Time Characteristic on the Server
    TIPC_CT_NTF_CFG_CMD = MSG_ID(TIPC, 0x04),
    /// Generic message for writing the Time Update Control Point Characteristic Value on a peer device
    TIPC_WR_TIME_UPD_CTNL_PT_CMD = MSG_ID(TIPC, 0x05),
    /// Received Current Time value (Notification)
    TIPC_CT_IND = MSG_ID(TIPC, 0x06),
    /// Complete Event Information
    TIPC_CMP_EVT = MSG_ID(TIPC, 0x07),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #TIPC_ENABLE_REQ message
typedef struct tipc_enable_req
{
    /// Connection index
    uint8_t conidx;
    /// Connection type
    uint8_t con_type;
    /// Existing handle values cts
    tipc_cts_content_t cts;
    /// Existing handle values ndcs
    tipc_ndcs_content_t ndcs;
    /// Existing handle values rtus
    tipc_rtus_content_t rtus;
} tipc_enable_req_t;

/// Parameters of the #TIPC_ENABLE_RSP message
typedef struct tipc_enable_rsp
{
    /// Connection index
    uint8_t conidx;
    /// Status
    uint16_t status;
    /// Existing handle values cts
    tipc_cts_content_t cts;
    /// Existing handle values ndcs
    tipc_ndcs_content_t ndcs;
    /// Existing handle values rtus
    tipc_rtus_content_t rtus;
} tipc_enable_rsp_t;

/// Parameters of the #TIPC_CT_NTF_CFG_CMD message
typedef struct tipc_ct_ntf_cfg_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// Event configuration for Current Time
    uint16_t cfg_val;
} tipc_ct_ntf_cfg_cmd_t;

/// Parameters of the #TIPC_RD_CHAR_CMD message
typedef struct tipc_rd_char_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Value identifier (see #tip_value_id enumeration)
    uint8_t val_id;
} tipc_rd_char_cmd_t;

/// Parameters of the #TIPC_WR_TIME_UPD_CTNL_PT_CMD message
typedef struct tipc_wr_time_upd_ctnl_pt_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Value
    tip_time_upd_contr_pt_t value;
} tipc_wr_time_upd_ctnl_pt_cmd_t;

/// Parameters of the #TIPC_RD_CHAR_IND message
typedef struct tipc_rd_char_ind
{
    /// Connection index
    uint8_t conidx;
    /// Value identifier (see #tip_value_id enumeration)
    uint8_t val_id;
    /// Read value
    union tip_value value;
} tipc_rd_char_ind_t;

/// Parameters of the #TIPC_CT_IND message
typedef struct tipc_ct_ind
{
    /// Connection index
    uint8_t conidx;
    /// Current Time value
    tip_curr_time_t ct_val;
} tipc_ct_ind_t;

/// Parameters of the #TIPC_CMP_EVT message
typedef struct tipc_cmp_evt
{
    /// Connection index
    uint8_t conidx;
    /// Operation
    uint8_t operation;
    /// Status
    uint16_t status;
} tipc_cmp_evt_t;

/// @} TIPC_API_MSG

#endif // _TIPC_MSG_H_
