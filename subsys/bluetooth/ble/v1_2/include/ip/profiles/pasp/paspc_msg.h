/**
 ****************************************************************************************
 *
 * @file paspc_msg.h
 *
 * @brief Header file - Phone Alert Status Profile Client Role - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _PASPC_MSG_H_
#define _PASPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup PASPC
 * @ingroup Profile
 * @brief Phone Alert Status Profile Client  - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "pasp_common.h"
#include "rwip_task.h" // Task definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Message IDs
/*@TRACE*/
enum paspc_msg_ids
{
    /// Enable the Phone Alert Status Profile Client task - at connection
    PASPC_ENABLE_REQ  = MSG_ID(PASPC, 0x00),
    /// Enable the Phone Alert Status Profile Client task - at connection
    PASPC_ENABLE_RSP  = MSG_ID(PASPC, 0x01),

    /// Read the value of an attribute in the peer device database
    PASPC_READ_CMD    = MSG_ID(PASPC, 0x02),
    /// Write the value of an attribute in the peer device database
    PASPC_WRITE_CMD   = MSG_ID(PASPC, 0x03),

    /// Indicate that an attribute value has been received either upon notification or read response
    PASPC_VALUE_IND   = MSG_ID(PASPC, 0x04),

    /// Complete Event Information
    PASPC_CMP_EVT     = MSG_ID(PASPC, 0x05),
};

/// Operation Codes
enum paspc_op_codes
{
    /// Reserved operation code
    PASPC_RESERVED_OP_CODE  = 0x00,
    /// Read attribute value Procedure
    PASPC_READ_OP_CODE,
    /// Write attribute value Procedure
    PASPC_WRITE_OP_CODE,
};

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */

/// Internal codes for reading/writing a PAS characteristic with one single request
enum paspc_rd_wr_codes
{
    /// Read PAS Alert Status
    PASPC_RD_ALERT_STATUS              = 0x00,
    /// Read PAS Ringer Setting
    PASPC_RD_RINGER_SETTING            = 0x01,
    /// Write PAS Ringer Control Point
    PASPC_WR_RINGER_CTNL_PT            = 0x02,

    /// Read/Write PAS Alert Status Client Characteristic Configuration Descriptor
    PASPC_RD_WR_ALERT_STATUS_CFG       = 0x03,
    /// Read PAS Ringer Setting Client Characteristic Configuration Descriptor
    PASPC_RD_WR_RINGER_SETTING_CFG     = 0x04,
};

/// Phone Alert Status Service Characteristics
enum
{
    /// Alert Status
    PASPC_CHAR_ALERT_STATUS,
    /// Ringer Setting
    PASPC_CHAR_RINGER_SETTING,
    /// Ringer Control Point
    PASPC_CHAR_RINGER_CTNL_PT,

    PASPC_CHAR_MAX,
};

/// Phone Alert Status Service Characteristic Descriptors
enum
{
    /// Alert Status Char. - Client Characteristic Configuration
    PASPC_DESC_ALERT_STATUS_CL_CFG,
    /// Ringer Setting Char. - Client Characteristic Configuration
    PASPC_DESC_RINGER_SETTING_CL_CFG,

    PASPC_DESC_MAX,
};

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Phone Alert Status Service
 */
typedef struct paspc_pass_content
{
    /// Service info
    prf_svc_t           svc;

    /// Characteristic info:
    ///  - Alert Status
    ///  - Ringer Setting
    ///  - Ringer Control Point
    prf_char_t      chars[PASPC_CHAR_MAX];

    /// Descriptor handles:
    ///  - Alert Status Client Cfg
    ///  - Ringer Setting Client Cfg
    prf_desc_t descs[PASPC_DESC_MAX];
} paspc_pass_content_t;


/// Parameters of the @ref PASPC_ENABLE_REQ message
struct paspc_enable_req
{
    ///Connection index
    uint8_t                   conidx;
    /// Connection type
    uint8_t                   con_type;
    /// Existing handle values PASS
    paspc_pass_content_t pass;
};

/// Parameters of the @ref PASPC_ENABLE_RSP message
struct paspc_enable_rsp
{
    ///Connection index
    uint8_t                   conidx;
    /// status
    uint16_t                  status;
    /// Existing handle values PASS
    paspc_pass_content_t pass;
};

/// Parameters of the @ref PASPC_READ_CMD message
struct paspc_read_cmd
{
    ///Connection index
    uint8_t conidx;
    /// Read code
    uint8_t read_code;
};

/// Parameters of the @ref PASPC_WRITE_CMD message
struct paspc_write_cmd
{
    ///Connection index
    uint8_t conidx;
    /// Write code see enum #paspc_rd_wr_codes
    uint8_t write_code;
    /// Value
    union write_value_tag
    {
        /// Ringer Control Point characteristic value
        uint8_t  ringer_ctnl_pt;
        /// Alert Status characteristic client characteristic configuration value
        uint16_t alert_status_ntf_cfg;
        /// Ringer Setting characteristic client characteristic configuration value
        uint16_t ringer_setting_ntf_cfg;
    } value;
};

/// Parameters of the @ref PASPC_VALUE_IND message
struct paspc_value_ind
{
    ///Connection index
    uint8_t conidx;
    /// Attribute Code see enum #paspc_rd_wr_codes
    uint8_t att_code;
    /// Value
    union value_tag
    {
        /// Alert Status characteristic value
        uint8_t  alert_status;
        /// Ringer Setting characteristic value
        uint8_t  ringer_setting;
        /// Alert Status characteristic client characteristic configuration value
        uint16_t alert_status_ntf_cfg;
        /// Ringer Setting characteristic client characteristic configuration value
        uint16_t ringer_setting_ntf_cfg;
    } value;
};

/// Parameters of the @ref PASPC_CMP_EVT message
struct paspc_cmp_evt
{
    ///Connection index
    uint8_t  conidx;
    /// Operation code see enum #paspc_op_codes
    uint8_t  operation;
    /// Status
    uint16_t status;
};


/// @} PASPC

#endif //(_PASPC_MSG_H_)
