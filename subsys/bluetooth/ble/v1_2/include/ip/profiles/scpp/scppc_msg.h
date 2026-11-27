/**
 ****************************************************************************************
 *
 * @file scppc_msg.h
 *
 * @brief Header file - Scan Parameters Profile Client Role - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _SCPPC_MSG_H_
#define _SCPPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup SCPPC
 * @ingroup SCPPC
 * @brief Scan Parameters Profile Client - Message API.
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
#include "rwip_task.h" // Task definitions
#include "scpp_common.h"
/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*@TRACE*/
enum scppc_msg_id
{
    /// Start the Scan Parameters profile Client Role - at connection
    SCPPC_ENABLE_REQ                  = MSG_ID(SCPPC, 0x00),
    ///Confirm that cfg connection has finished with discovery results, or that normal cnx started
    SCPPC_ENABLE_RSP                  = MSG_ID(SCPPC, 0x01),

    /// Write Scan Interval Window Characteristic Value Request - Write Without Response
    SCPPC_SCAN_INTV_WD_WR_CMD         = MSG_ID(SCPPC, 0x02),
    /// Read Scan Refresh Notification Configuration Value request
    SCPPC_SCAN_REFRESH_NTF_CFG_RD_CMD = MSG_ID(SCPPC, 0x03),
    /// Inform APP that Scan Refresh Notification Configuration has been read
    SCPPC_SCAN_REFRESH_NTF_CFG_RD_IND = MSG_ID(SCPPC, 0x04),
    /// Write Scan Refresh Notification Configuration Value request
    SCPPC_SCAN_REFRESH_NTF_CFG_CMD    = MSG_ID(SCPPC, 0x05),
    /// Inform APP that peer service request a refresh of Scan Parameters values
    SCPPC_SCAN_REFRESH_IND            = MSG_ID(SCPPC, 0x06),
    /// Complete Event Information
    SCPPC_CMP_EVT                     = MSG_ID(SCPPC, 0x07),
};

/// Characteristics
enum scppc_char
{
    /// Scan Interval Window
    SCPPC_CHAR_SCAN_INTV_WD,
    /// Scan Refresh
    SCPPC_CHAR_SCAN_REFRESH,

    SCPPC_CHAR_MAX,
};


/// Characteristic descriptors
enum scppc_desc
{
    /// Scan Refresh Client Config
    SCPPC_DESC_SCAN_REFRESH_CFG,

    SCPPC_DESC_MAX,
};

/// Operation codes
enum scppc_op_codes
{
    /// Database Creation Procedure
    SCPPC_RESERVED_OP_CODE = 0,

    /// Update Scan Interval and Window
    SCPPC_SCAN_INTV_WD_WR_CMD_OP_CODE = 1,

    /// Read Scan Refresh CCC Operation Code
    SCPPC_SCAN_REFRESH_NTF_CFG_RD_CMD_OP_CODE = 2,

    /// Configure Scan Refresh CCC Operation Code
    SCPPC_SCAN_REFRESH_NTF_CFG_CMD_OP_CODE = 3,
};
/*
 * APIs Structure
 ****************************************************************************************
 */

///Structure containing the characteristics handles, value handles and descriptors
typedef struct scps_content
{
    /// service info
    prf_svc_t           svc;

    /// characteristic info:
    ///  - Scan Interval Window
    ///     - Scan Refresh
    prf_char_t      chars[SCPPC_CHAR_MAX];

    /// Descriptor handles:
    ///  - Scan Refresh Client Cfg
    prf_desc_t descs[SCPPC_DESC_MAX];
} scps_content_t;

/// Parameters of the @ref SCPPC_ENABLE_REQ message
struct scppc_enable_req
{
    /// Connection index
    uint8_t        conidx;
    ///Connection type
    uint8_t        con_type;
    ///Existing handle values scps
    scps_content_t scps;
};

/// Parameters of the @ref SCPPC_ENABLE_RSP message
struct scppc_enable_rsp
{
    /// Connection index
    uint8_t        conidx;
    ///status
    uint16_t       status;
    ///Existing handle values scps
    scps_content_t scps;
};

///Parameters of the @ref SCPPC_SCAN_INTV_WD_WR_CMD message
struct scppc_scan_intv_wd_wr_cmd
{
    /// Connection index
    uint8_t             conidx;
    ///Scan Interval Window
    scpp_scan_intv_wd_t scan_intv_wd;
};

///Parameters of the @ref SCPPC_SCAN_REFRESH_NTF_CFG_RD_CMD message
struct scppc_scan_refresh_ntf_cfg_rd_cmd
{
    /// Connection index
    uint8_t             conidx;
};

///Parameters of the @ref SCPPC_SCAN_REFRESH_NTF_CFG_RD_IND message
struct scppc_scan_refresh_ntf_cfg_rd_ind
{
    /// Connection index
    uint8_t  conidx;
    ///Notification Configuration Value
    uint16_t ntf_cfg;
};

///Parameters of the @ref SCPPC_SCAN_REFRESH_NTF_CFG_CMD message
struct scppc_scan_refresh_ntf_cfg_cmd
{
    /// Connection index
    uint8_t  conidx;
    ///Notification Configuration
    uint16_t ntf_cfg;
};

///Parameters of the @ref SCPPC_SCAN_REFRESH_IND message
struct scppc_scan_refresh_ind
{
    /// Connection index
    uint8_t  conidx;
};

///Parameters of the @ref SCPPC_CMP_EVT message
struct scppc_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} SCPPC

#endif /* _SCPPC_MSG_H_ */
