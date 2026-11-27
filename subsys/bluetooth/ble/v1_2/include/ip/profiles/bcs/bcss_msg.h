/**
 ****************************************************************************************
 *
 * @file bcss_msg.h
 *
 * @brief Header file - Body Composition Service Sensor/Server Role - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _BCSS_MSG_H_
#define _BCSS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup BCSS
 * @ingroup Profile
 * @brief Body Composition Service Sensor/Server Role - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "bcs_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Body Composition Service Sensor
/*@TRACE*/
enum bcss_msg_id
{
    /// Enable the BCS Sensor task for a connection
    BCSS_ENABLE_REQ         = MSG_ID(BCSS, 0x00),
    /// Enable the BCS Sensor task for a connection
    BCSS_ENABLE_RSP         = MSG_ID(BCSS, 0x01),
    /// Indicate a new IND configuration to the application
    /// Send CCC descriptor write update to the APP
    BCSS_MEAS_CCC_IND       = MSG_ID(BCSS, 0x02),

    /// Make BCS Sensor indicate it's peer device with the measurement
    BCSS_MEAS_INDICATE_CMD  = MSG_ID(BCSS, 0x03),

    /// Send a complete event status to the application
    BCSS_CMP_EVT            = MSG_ID(BCSS, 0x04),
};

/// Operation codes
enum bcss_op_codes
{
    /// Indicate Measurement Operation Code
    BCSS_MEAS_INDICATE_CMD_OP_CODE = 1,
};
/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the initialization function
struct bcss_db_cfg
{
    /// Body Composition service Feature Value
    uint32_t feature;
    /// Secondary service value
    uint8_t  secondary_service;
};

/// Parameters of the @ref BCSS_ENABLE_REQ message
struct bcss_enable_req
{
    /// Connection Index
    uint8_t  conidx;
    /// CCC for the current connection
    uint16_t ind_cfg;
};

/// Parameters of the @ref BCSS_ENABLE_RSP message
struct bcss_enable_rsp
{
    /// Connection Index
    uint8_t  conidx;
    /// Status
    uint16_t status;
};

/// Parameters of the @ref BCSS_MEAS_INDICATE_CMD message
struct bcss_meas_indicate_cmd
{
    /// Connection Index
    uint8_t         conidx;
    /// Measurement value
    bcs_meas_t     meas;
};

/// Parameters of the @ref BCSS_MEAS_CCC_IND message
struct bcss_meas_ccc_ind
{
    /// Connection Index
    uint8_t  conidx;
    /// Char. Client Characteristic Configuration
    uint16_t ind_cfg;
};

/// Parameters of the @ref BCSS_CMP_EVT message
struct bcss_cmp_evt
{
    /// Connection Index
    uint8_t  conidx;
   /// operation
    uint8_t  operation;
    /// Operation Status
    uint16_t status;
};

/// @} BCSS

#endif //(_BCSS_MSG_H_)
