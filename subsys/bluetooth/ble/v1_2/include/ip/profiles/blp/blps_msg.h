/**
 ****************************************************************************************
 *
 * @file blps_msg.h
 *
 * @brief Header file - Blood Pressure Profile Sensor - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _BLPS_MSG_H_
#define _BLPS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup BLPS
 * @ingroup Profile
 * @brief Blood Pressure Profile Sensor - Message API.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_task.h" // Task definitions
#include "blp_common.h"

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Messages for Blood Pressure Profile Sensor
/*@TRACE*/
enum blps_msg_id
{
    /// Start the Blood Pressure Profile Sensor - at connection
    BLPS_ENABLE_REQ      = MSG_ID(BLPS, 0x00),
    /// Start the Blood Pressure Profile Sensor - at connection
    BLPS_ENABLE_RSP      = MSG_ID(BLPS, 0x01),

    /// Send blood pressure measurement value from APP
    BLPS_MEAS_SEND_CMD   = MSG_ID(BLPS, 0x02),

    /// Inform APP of new configuration value
    BLPS_CFG_INDNTF_IND  = MSG_ID(BLPS, 0x03),

    /// Complete Event Information
    BLPS_CMP_EVT         = MSG_ID(BLPS, 0x04),
};

/// Operation codes
enum blps_op_codes
{
    /// Database Creation Procedure
    BLPS_RESERVED_OP_CODE      = 0,
    /// Indicate Measurement Operation Code
    BLPS_MEAS_SEND_CMD_OP_CODE = 1,
};

/// Parameters of the @ref BLPS_ENABLE_REQ message
struct blps_enable_req
{
    ///Connection index
    uint8_t  conidx;
    /// Blood Pressure indication configuration
    uint16_t bp_meas_ind_en;
    /// Intermediate Cuff Pressure Notification configuration
    uint16_t interm_cp_ntf_en;
};

/// Parameters of the @ref BLPS_ENABLE_RSP message
struct blps_enable_rsp
{
    ///Connection index
    uint8_t  conidx;
    ///Status
    uint16_t status;
};

///Parameters of the @ref BLPS_CFG_INDNTF_IND message
struct blps_cfg_indntf_ind
{
    ///Connection index
    uint8_t  conidx;
    /// Own code for differentiating between Blood Pressure Measurement, and Intermediate
    /// Cuff Pressure Measurement characteristics
    uint8_t  char_code;
    ///Stop/notify/indicate value to configure into the peer characteristic
    uint16_t cfg_val;
};

/////Parameters of the @ref BLPS_MEAS_SEND_CMD message
struct blps_meas_send_cmd
{
    ///Connection index
    uint8_t       conidx;
    /// Flag indicating if it is a intermediary cuff pressure measurement (1) or
    /// stable blood pressure measurement (0) (see enum #bps_meas_type).
    uint8_t       flag_interm_cp;
    ///Blood Pressure measurement
    bps_bp_meas_t meas_val;
};

///Parameters of the @ref BLPS_CMP_EVT message
struct blps_cmp_evt
{
    ///Connection index
    uint8_t  conidx;
    /// Operation
    uint8_t  operation;
    /// Status
    uint16_t status;
};

///Parameters of the @ref BLPS_CREATE_DB_REQ message
struct blps_db_cfg
{
    /// Supported features
    uint16_t features;
    /// Profile Configuration
    uint8_t  prfl_cfg;
};

/// @} BLPS

#endif /* _BLPS_MSG_H_ */
