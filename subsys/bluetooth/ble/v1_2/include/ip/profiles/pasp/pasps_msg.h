/**
 ****************************************************************************************
 *
 * @file pasps_msg.h
 *
 * @brief Phone Alert Status Profile Server - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef PASPS_MSG_H_
#define PASPS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup PASPS Task
 * @ingroup Profile
 * @brief Phone Alert Status Profile Server - Message API
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


/// Messages for Phone Alert Status Profile Server
/*@TRACE*/
enum pasps_msg_ids
{
    /// Set Bonded Data
    PASPS_ENABLE_REQ             = MSG_ID(PASPS, 0x00),
    /// Bonded data confirmation
    PASPS_ENABLE_RSP             = MSG_ID(PASPS, 0x01),

    /// Update the value of a characteristic
    PASPS_UPDATE_CHAR_VAL_CMD    = MSG_ID(PASPS, 0x02),
    /// Indicate that an attribute value has been written
    PASPS_WRITTEN_CHAR_VAL_IND   = MSG_ID(PASPS, 0x03),

    /// Complete Event Information
    PASPS_CMP_EVT                = MSG_ID(PASPS, 0x04),
};

/// Operation Codes
enum pasps_op_codes
{
    PASPS_RESERVED_OP_CODE              = 0x00,
    /// Update Alert Status Char. value
    PASPS_UPD_ALERT_STATUS_OP_CODE,
    /// Update Ringer Setting Char. value
    PASPS_UPD_RINGER_SETTING_OP_CODE,
};

/// Attribute Codes
enum pasps_att_code
{
    /// Alert Status Characteristic
    PASPS_ALERT_STATUS_CHAR_VAL,
    /// Ringer Setting Characteristic
    PASPS_RINGER_SETTING_CHAR_VAL,
    /// Ringer Control Point Characteristic
    PASPS_RINGER_CTNL_PT_CHAR_VAL,

    /// Alert Status Characteristic - Notification Configuration
    PASPS_ALERT_STATUS_NTF_CFG,
    /// Ringer Setting Characteristic - Notification Configuration
    PASPS_RINGER_SETTING_NTF_CFG,
};


/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref PASPS_CREATE_DB_REQ message
struct pasps_db_cfg
{
    /// Alert Status Char. Value
    uint8_t alert_status;
    /// Ringer Settings Char. Value
    uint8_t ringer_setting;
};

/// Parameters of the @ref PASPS_ENABLE_REQ message
struct pasps_enable_req
{
    ///Connection index
    uint8_t  conidx;
    /// Alert Status Characteristic Notification Configuration
    uint16_t alert_status_ntf_cfg;
    /// Ringer Setting Characteristic Notification Configuration
    uint16_t ringer_setting_ntf_cfg;
};

/// Parameters of the @ref PASPS_ENABLE_RSP message
struct pasps_enable_rsp
{
    ///Connection index
    uint8_t  conidx;
    /// status
    uint16_t status;
};

///Parameters of the @ref PASPS_UPDATE_CHAR_VAL_CMD message
struct pasps_update_char_val_cmd
{
    /// Bit field of connection index on which notification will be send
    /// will be mask according to client that supports reception
    uint32_t conidx_bf;
    /// Operation Code (PASPS_UPD_ALERT_STATUS_OP_CODE or PASPS_UPD_RINGER_SETTING_OP_CODE)
    uint8_t  operation;
    /// Alert Status Characteristic Value or Ringer Setting Characteristic Value
    uint8_t  value;
};

///Parameters of the @ref PASPS_WRITTEN_CHAR_VAL_IND message
struct pasps_written_char_val_ind
{
    ///Connection index
    uint8_t conidx;
    /// Attribute Code see enum #pasps_att_code
    uint8_t att_code;
    /// Written value
    union written_value_tag
    {
        /// Ringer Control Point Char. Value
        uint8_t  ringer_ctnl_pt;
        /// Alert Status Notification Configuration
        uint16_t alert_status_ntf_cfg;
        /// Ringer Setting Notification Configuration
        uint16_t ringer_setting_ntf_cfg;
    } value;
};

///Parameters of the @ref PASPS_CMP_EVT message
struct pasps_cmp_evt
{
    /// Operation see enum #pasps_op_codes
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} PASPS

#endif // PASPS_MSG_H_
