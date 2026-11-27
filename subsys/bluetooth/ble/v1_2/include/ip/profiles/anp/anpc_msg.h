/**
 ****************************************************************************************
 *
 * @file anpc_msg.h
 *
 * @brief Alert Notification Profile Client - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _ANPC_MSG_H_
#define _ANPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup ANPC
 * @ingroup Profile
 * @brief Alert Notification Profile Client - Message API.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "prf_types.h"

#include "anp_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Message IDs
/*@TRACE*/
enum anpc_msg_ids
{
    /// Enable the Alert Notification Profile Client task - at connection
    ANPC_ENABLE_REQ            = MSG_ID(ANPC, 0x00),
    /// Enable the Alert Notification Profile Client task - at connection
    ANPC_ENABLE_RSP            = MSG_ID(ANPC, 0x01),

    /// Read the value of an attribute in the peer device database
    ANPC_READ_CMD              = MSG_ID(ANPC, 0x02),
    /// Write the value of an attribute in the peer device database
    ANPC_WRITE_CMD             = MSG_ID(ANPC, 0x03),
    /// Indicate that an attribute value has been received either upon notification or read response
    ANPC_VALUE_IND             = MSG_ID(ANPC, 0x04),

    /// Complete Event Information
    ANPC_CMP_EVT               = MSG_ID(ANPC, 0x05),
};

/// Operation Codes
enum anpc_op_codes
{
    /// Reserved operation code
    ANPC_RESERVED_OP_CODE  = 0x00,

    /**
     * EXTERNAL OPERATION CODES
     */

    /// Discovery Procedure
    ANPC_ENABLE_OP_CODE,
    /// Read attribute value Procedure
    ANPC_READ_OP_CODE,
    /// Write attribute value Procedure
    ANPC_WRITE_OP_CODE,

};

/// Alert Notification Service Characteristics
enum anpc_ans_chars
{
    /// Supported New Alert Category
    ANPC_CHAR_SUP_NEW_ALERT_CAT,
    /// New Alert
    ANPC_CHAR_NEW_ALERT,
    /// Supported Unread Alert Category
    ANPC_CHAR_SUP_UNREAD_ALERT_CAT,
    /// Unread Alert Status
    ANPC_CHAR_UNREAD_ALERT_STATUS,
    /// Alert Notification Control Point
    ANPC_CHAR_ALERT_NTF_CTNL_PT,

    ANPC_CHAR_MAX,
};

/// Alert Notification Service Characteristic Descriptors
enum anpc_ans_descs
{
    /// New Alert Char. - Client Characteristic Configuration
    ANPC_DESC_NEW_ALERT_CL_CFG,
    /// Unread Alert Status Char. - Client Characteristic Configuration
    ANPC_DESC_UNREAD_ALERT_STATUS_CL_CFG,

    ANPC_DESC_MAX,

    ANPC_DESC_MASK = 0x10,
};

/// Codes for reading/writing a ANS characteristic with one single request
enum anpc_rd_wr_ntf_codes
{
    /// Read ANS Supported New Alert Category
    ANPC_RD_SUP_NEW_ALERT_CAT           = ANPC_CHAR_SUP_NEW_ALERT_CAT,
    /// NTF New Alert
    ANPC_NTF_NEW_ALERT                  = ANPC_CHAR_NEW_ALERT,
    /// Read ANS Supported Unread Alert Category
    ANPC_RD_SUP_UNREAD_ALERT_CAT        = ANPC_CHAR_SUP_UNREAD_ALERT_CAT,
    /// NTF Unread Alert Categories
    ANPC_NTF_UNREAD_ALERT               = ANPC_CHAR_UNREAD_ALERT_STATUS,
    /// Write ANS Alert Notification Control Point
    ANPC_WR_ALERT_NTF_CTNL_PT           = ANPC_CHAR_ALERT_NTF_CTNL_PT,

    /// Read/Write ANS New Alert Client Characteristic Configuration Descriptor
    ANPC_RD_WR_NEW_ALERT_CFG            = (ANPC_DESC_NEW_ALERT_CL_CFG | ANPC_DESC_MASK),
    /// Read ANS Unread Alert Status Client Characteristic Configuration Descriptor
    ANPC_RD_WR_UNREAD_ALERT_STATUS_CFG  = (ANPC_DESC_UNREAD_ALERT_STATUS_CL_CFG | ANPC_DESC_MASK),
};

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */
/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Alert Notification Service
 */
typedef struct anpc_ans_content
{
    /// Service info
    prf_svc_t           svc;

    /// Characteristic info:
    ///  - Supported New Alert Category
    ///  - New Alert
    ///  - Supported Unread Alert Category
    ///  - Unread Alert Status
    ///  - Alert Notification Control Point
    prf_char_t      chars[ANPC_CHAR_MAX];

    /// Descriptor handles:
    ///  - New Alert Client Cfg
    ///  - Unread Alert Status Client Cfg
    prf_desc_t descs[ANPC_DESC_MAX];
} anpc_ans_content_t;

/// Parameters of the @ref ANPC_ENABLE_REQ message
struct anpc_enable_req
{
    /// Connection Index
    uint8_t                 conidx;
    /// Connection type
    uint8_t                 con_type;
    /// Existing handle values ANS
    anpc_ans_content_t      ans;
};

/// Parameters of the @ref ANPC_ENABLE_RSP message
struct anpc_enable_rsp
{
    /// Connection Index
    uint8_t                 conidx;
    /// status
    uint16_t                status;
    /// Existing handle values ANS
    anpc_ans_content_t      ans;
};

/// Parameters of the @ref ANPC_READ_CMD message
struct anpc_read_cmd
{
    /// Connection Index
    uint8_t conidx;
    /// Operation Code
    uint8_t operation;
    /// Read code
    uint8_t read_code;
};

/// Parameters of the @ref ANPC_WRITE_CMD message
struct anpc_write_cmd
{
    /// Connection Index
    uint8_t conidx;
    /// Operation Code
    uint8_t operation;
    /// Write code
    uint8_t write_code;
    /// Value
    union anpc_write_value_tag
    {
        /// Alert Notification Control Point
        anp_ctnl_pt_t ctnl_pt;
        /// New Alert Notification Configuration
        uint16_t      new_alert_ntf_cfg;
        /// Unread Alert Status Notification Configuration
        uint16_t      unread_alert_status_ntf_cfg;
    } value;
};

/// Parameters of the @ref ANPC_VALUE_IND message
struct anpc_value_ind
{
    /// Connection Index
    uint8_t                   conidx;
    /// Attribute Code
    uint8_t                   att_code;
    /// Value
    union anpc_value_tag
    {
        /// List of supported categories
        anp_cat_id_bit_mask_t supp_cat;
        /// New Alert
        anp_new_alert_t       new_alert;
        /// Unread Alert
        anp_unread_alert_t    unread_alert;
        /// Client Characteristic Configuration Descriptor Value
        uint16_t              ntf_cfg;
    } value;
};

/// Parameters of the @ref ANPC_CMP_EVT message
struct anpc_cmp_evt
{
    /// Connection Index
    uint8_t  conidx;
    /// Operation code
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} ANPC

#endif //(_ANPC_MSG_H_)
