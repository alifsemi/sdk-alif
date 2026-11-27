/**
 ****************************************************************************************
 *
 * @file wscs_msg.h
 *
 * @brief Header file - Weight SCale Profile Sensor - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _WSCS_MSG_H_
#define _WSCS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup WSCS
 * @ingroup Profile
 * @brief Weight SCale Profile - Message API.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "wsc_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Weight SCale Profile Sensor
/*@TRACE*/
enum wscs_msg_id
{
    /// Enable the WSCP Sensor task for a connection
    WSCS_ENABLE_REQ         = MSG_ID(WSCS, 0x00),
    /// Enable the WSCP Sensor task for a connection
    WSCS_ENABLE_RSP         = MSG_ID(WSCS, 0x01),
    /// Indicate a new IND configuration to the application
    /// Send CCC descriptor write update to the APP
    WSCS_MEAS_CCC_IND       = MSG_ID(WSCS, 0x02),

    /// Send a WSC Measurement to the peer device (Indication)
    WSCS_MEAS_INDICATE_CMD  = MSG_ID(WSCS, 0x03),

    /// Send a complete event status to the application
    WSCS_CMP_EVT            = MSG_ID(WSCS, 0x04),
};

enum wscs_op_codes
{
    WSCS_MEAS_INDICATE_CMD_OP_CODE = 1,
};
/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the initialization function
struct wscs_db_cfg
{
    /// Weight SCale service Feature Value
    uint32_t feature;
    /// Body Composition started handle
    uint16_t bcs_start_hdl;

};

/// Parameters of the @ref WSCS_ENABLE_REQ message
struct wscs_enable_req
{
    /// Connection index
    uint8_t  conidx;
    /// CCC for the current connection
    uint16_t ind_cfg;
};

/// Parameters of the @ref WSCS_ENABLE_RSP message
struct wscs_enable_rsp
{
    /// Connection index
    uint8_t  conidx;
    /// Status
    uint16_t status;
};

/// Parameters of the @ref WSCS_MEAS_INDICATE_CMD message
struct wscs_meas_indicate_cmd
{
    /// Connection index
    uint8_t         conidx;
    /// Presence of optional fields and measurement units
    uint8_t         flags;
    /// Weight value
    uint16_t        weight;
    /// Time Stamp value
    prf_date_time_t time_stamp;
    /// User ID value
    uint8_t         user_id;
    /// Body Mass Index
    uint16_t        bmi;
    /// Height value
    uint16_t        height;
};

/// Parameters of the @ref WSCS_WR_CCC_IND message
struct wscs_meas_ccc_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Char. Client Characteristic Configuration
    uint16_t ind_cfg;
};

/// Parameters of the @ref WSCS_CMP_EVT message
struct wscs_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// operation
    uint8_t  operation;
    /// Operation Status
    uint16_t status;
};

/// @} WSCS

#endif //(_WSCS_MSG_H_)
