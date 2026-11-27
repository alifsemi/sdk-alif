/**
 ****************************************************************************************
 *
 * @file udss_msg.h
 *
 * @brief Header file - User Data Service Profile - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _UDSS_MSG_H_
#define _UDSS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup UDSS
 * @ingroup Profile
 * @brief User Data Service Profile Task - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"      // Task definitions
#include "uds_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Messages for User Data Service Profile Sensor
/*@TRACE*/
enum udss_msg_id
{
    /// Enable the Service task - at connection
    UDSS_ENABLE_REQ                 = MSG_ID(UDSS, 0x00),
    /// Response to Enable the Service task - at connection
    UDSS_ENABLE_RSP                 = MSG_ID(UDSS, 0x01),

    /// Send String Characteristic write update to the APP
    UDSS_WR_CHAR_UTF8_REQ_IND       = MSG_ID(UDSS, 0x02),
    /// Send Rest Characteristic write update to the APP
    UDSS_WR_CHAR_REQ_IND            = MSG_ID(UDSS, 0x03),

    /// write confirmation Characteristic from the APP
    UDSS_WR_CHAR_CFM                = MSG_ID(UDSS, 0x04),

    /// Send Rest Characteristic Read Request to the APP
    UDSS_RD_CHAR_REQ_IND            = MSG_ID(UDSS, 0x05),

    /// Read confirmation String Characteristic from the APP
    UDSS_RD_CHAR_UTF8_CFM           = MSG_ID(UDSS, 0x06),
    /// Read confirmation Characteristic from the APP
    UDSS_RD_CHAR_CFM                = MSG_ID(UDSS, 0x07),

    /// Write DB_Change Inc Characteristic CCC update to the APP
    UDSS_WR_CHAR_CCC_IND            = MSG_ID(UDSS, 0x08),

    /// Inform that DB_Change Increment is updated from the APP
    UDSS_DB_CHG_INC_UPD_CMD         = MSG_ID(UDSS, 0x09),

    /// Send User Control_Point Characteristic write update to the APP
    UDSS_USER_CTRL_PT_REQ_RECV_IND  = MSG_ID(UDSS, 0x0A),
    /// Indication User Control_Point Characteristic from the APP
    UDSS_USER_CTRL_PT_RSP_SEND_CMD  = MSG_ID(UDSS, 0x0B),

    /// Send a complete event status to the application
    UDSS_CMP_EVT                    = MSG_ID(UDSS, 0x0C),
};

/// Used in Complete event to indicate the operation
enum udss_op_codes
{
    UDSS_RESERVED_OP_CODE         = 0,
    /// Send Control Point Response
    UDSS_CTRL_PT_RSP_SEND_OP_CODE = 1,
    /// Update Database Change Increment value
    UDSS_DB_CHG_INC_UPD_OP_CODE   = 2,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the initialization function
/// bitmask of enabled Characteristics ref@ uds_val_id
struct udss_db_cfg
{
    /// Database configuration 
    uint32_t char_mask;
};

/*
 * Messages Exchange Parameters
 ****************************************************************************************
 */

/// Parameters of the @ref UDSS_ENABLE_REQ message
struct udss_enable_req
{
    /// Connection index
    uint8_t  conidx;
    /// Database Change Increment CCC
    uint16_t db_chg_inc_ccc;
    /// User Control Point CCC
    uint16_t user_ctrl_pt_ccc;
};

/// Parameters of the @ref UDSS_ENABLE_RSP message
struct udss_enable_rsp
{
    /// Connection index
    uint8_t  conidx;
    /// Operation Status
    uint16_t status;
};

/// Parameters of the @ref UDSS_WR_CHAR_UTF8_REQ_IND message
struct udss_wr_char_utf8_req_ind
{
    /// Connection index
    uint8_t     conidx;
    /// characteristic id @ref uds_val_id
    uint8_t     char_idx;
    /// Token value to return in confirmation
    uint32_t    token;
    /// Actual data for the string utf8 characteristic
    prf_utf_8_t utf_name;
}; 

/// Parameters of the @ref UDSS_WR_CHAR_REQ_IND message
struct udss_wr_char_req_ind
{
    /// Connection index
    uint8_t            conidx;
    /// characteristic id @ref uds_val_id
    uint8_t            char_idx;
    /// Token value to return in confirmation
    uint32_t           token;
    /// data for the characteristic
    union uds_value    value;
};

/// Parameters of the @ref UDSS_WR_CHAR_CFM message
struct udss_wr_char_cfm
{
    /// Connection index
    uint8_t   conidx;
    /// characteristic id @ref uds_val_id
    uint8_t   char_idx;
    /// Operation Status
    uint16_t  status;
    /// Token value provided in request
    uint32_t  token;
};

/// Parameters of the @ref UDSS_RD_CHAR_REQ_IND message
struct udss_rd_char_req_ind
{
    /// Connection index
    uint8_t   conidx;
    /// characteristic id @ref uds_val_id
    uint8_t   char_idx;
    /// Token value to return in confirmation
    uint32_t  token;
    /// Data offset (only valid for an UTF-8 string)
    uint16_t  offset;
    /// Maximum string length to return (only valid for an UTF-8 string)
    uint16_t  max_len;
};

/// Parameters of the @ref UDSS_RD_CHAR_UTF8_CFM message
struct udss_rd_char_utf8_cfm
{
    /// Connection index
    uint8_t     conidx;
    /// characteristic id @ref uds_val_id
    uint8_t     char_idx;
    /// Operation Status
    uint16_t    status;
    /// Token value provided in request
    uint32_t    token;
    /// Total string length without offset
    uint16_t    total_len;
    /// Actual data for the string utf8 characteristic starting from offset
    prf_utf_8_t utf_name;
}; 

/// Parameters of the @ref UDSS_RD_CHAR_CFM message
struct udss_rd_char_cfm
{
    /// Connection index
    uint8_t            conidx;
    /// characteristic id @ref uds_val_id
    uint8_t            char_idx;
    /// Operation Status
    uint16_t           status;
    /// Token value provided in request
    uint32_t           token;
    /// data for the characteristic
    union uds_value    value;
}; 

/// Parameters of the @ref UDSS_WR_CHAR_CCC_IND message
struct udss_wr_char_ccc_ind
{
    /// Connection index
    uint8_t  conidx;
    /// characteristic id @ref uds_val_id
    uint8_t  char_idx;
    /// Characteristic CCC
    uint16_t ind_cfg;
};

/// Parameters of the @ref UDSS_DB_CHG_INC_UPD_CMD message
struct udss_db_chg_inc_upd_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// characteristic value
    uint32_t value;
}; 

/// Parameters of the @ref UDSS_USER_CTRL_PT_REQ_RECV_IND message
struct udss_user_ctrl_pt_req_recv_ind
{
    /// Connection index
    uint8_t  conidx;
    /// User Control Point Op Code see enum #uds_ctrl_pt_op_code
    uint8_t  op_code;
    /// User index
    uint8_t  user_id;
    /// Used for sending consent command
    uint16_t consent;
    /// Parameter length
    uint16_t length;
    /// for the future extension  to send additional commands
    uint8_t  parameter[__ARRAY_EMPTY];
}; 

/// Parameters of the @ref UDSS_USER_CTRL_PT_RSP_SEND_CMD message
struct udss_user_ctrl_pt_rsp_send_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// Request Op Code
    uint8_t  req_op_code;
    /// Response Value
    uint8_t  resp_value;
    /// Length of response parameter
    uint16_t length;
    /// Response parameter
    uint8_t  parameter[__ARRAY_EMPTY];
}; 

/*
 * Confirmation to the APP after Update, Indicate, Notify messages
 ****************************************************************************************
 */
/// Parameters of the @ref UDSS_CMP_EVT message
struct udss_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation Code
    uint8_t  operation;
    /// Operation Status
    uint16_t status;
};

/// @} UDSS

#endif //(_UDSS_MSG_H_)

