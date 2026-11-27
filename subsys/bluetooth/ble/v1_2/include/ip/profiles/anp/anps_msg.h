/**
 ****************************************************************************************
 *
 * @file anps_msg.h
 *
 * @brief Header file - Alert Notification Profile Server - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ANPS_MSG_H_
#define ANPS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup ANPS
 * @ingroup Profile
 * @brief Alert Notification Profile Server - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */


#include "rwip_task.h" // Task definitions
#include "anp_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Messages for Alert Notification Profile Server
/*@TRACE*/
enum anps_msg_ids
{
    /// Start the Alert Notification Profile Server Role
    ANPS_ENABLE_REQ             = MSG_ID(ANPS, 0x00),
    /// Start the Alert Notification Profile Server Role
    ANPS_ENABLE_RSP             = MSG_ID(ANPS, 0x01),

    /// Update the value of a characteristic
    ANPS_NTF_ALERT_CMD          = MSG_ID(ANPS, 0x02),
    /// The peer device requests to be notified about new alert values
    ANPS_NTF_IMMEDIATE_REQ_IND  = MSG_ID(ANPS, 0x03),
    /// Indicate that the notification configuration has been modified by the peer device
    ANPS_NTF_STATUS_UPDATE_IND  = MSG_ID(ANPS, 0x04),

    /// Complete Event Information
    ANPS_CMP_EVT                = MSG_ID(ANPS, 0x05),
};

/// Operation Codes
enum anps_op_codes
{
    ANPS_RESERVED_OP_CODE              = 0x00,

    /// Update New Alert Char. value
    ANPS_UPD_NEW_ALERT_OP_CODE,
    /// Update Unread Alert Status Char. value
    ANPS_UPD_UNREAD_ALERT_STATUS_OP_CODE,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref ANPS_CREATE_DB_REQ message
struct anps_db_cfg
{
    /// Supported New Alert Category Characteristic Value - Shall not be 0
    anp_cat_id_bit_mask_t supp_new_alert_cat;
    /// Supported Unread Alert Category Characteristic Value - Can be 0
    anp_cat_id_bit_mask_t supp_unread_alert_cat;
};

/// Parameters of the @ref ANPS_ENABLE_REQ message
struct anps_enable_req
{
    /// Connection index
    uint8_t  conidx;
    /// New Alert Characteristic - Saved Client Characteristic Configuration Descriptor Value
    uint16_t new_alert_ntf_cfg;
    /// Unread Alert Status Characteristic - Saved Client Characteristic Configuration Descriptor Value
    uint16_t unread_alert_status_ntf_cfg;
};

/// Parameters of the @ref ANPS_ENABLE_RSP message
struct anps_enable_rsp
{
    /// Connection index
    uint8_t  conidx;
    /// status
    uint16_t status;
};

///Parameters of the @ref ANPS_NTF_ALERT_CMD message
struct anps_ntf_alert_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// Operation Code (ANPS_UPD_NEW_ALERT_OP_CODE or ANPS_UPD_UNREAD_ALERT_STATUS_OP_CODE)
    uint8_t  operation;
    /// New Alert Characteristic Value or Unread Alert Status Characteristic Value
    union anps_value_tag
    {
        /// New Alert
        anp_new_alert_t new_alert;
        /// Unread Alert Status
        anp_unread_alert_t unread_alert_status;
    } value;
};

///Parameters of the @ref ANPS_NTF_IMMEDIATE_REQ_IND message
struct anps_ntf_immediate_req_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Alert type (New Alert or Unread Alert Status)
    uint8_t  alert_type;
    /// Status for each category
    anp_cat_id_bit_mask_t cat_ntf_cfg;
};

///Parameters of the @ref ANPS_NTF_STATUS_UPDATE_IND message
struct anps_ntf_status_update_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Alert type (New Alert or Unread Alert Status)
    uint8_t  alert_type;
    /// Client Characteristic Configuration Descriptor Status
    uint16_t ntf_ccc_cfg;
    /// Status for each category
    anp_cat_id_bit_mask_t cat_ntf_cfg;
};

///Parameters of the @ref ANPS_CMP_EVT message
struct anps_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} ANPS

#endif //(ANPS_MSG_H_)
