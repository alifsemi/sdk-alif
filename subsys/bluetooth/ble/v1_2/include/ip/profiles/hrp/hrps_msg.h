/**
 ****************************************************************************************
 *
 * @file hrps_msg.h
 *
 * @brief Header file - Heart Rate Profile Sensor - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _HRPS_MSG_H_
#define _HRPS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup HRPS
 * @ingroup Profile
 * @brief Heart Rate Profile sensor - Message API.
 *
 * Heart Rate sensor (HRS) profile provides functionalities to upper layer module
 * application. The device using this profile takes the role of Heart Rate sensor.
 *
 * The interface of this role to the Application is:
 *  - Enable the profile role (from APP)
 *  - Disable the profile role (from APP)
 *  - Notify peer device during Heart Rate measurement (from APP)
 *  - Indicate measurements performed to peer device (from APP)
 *
 * Profile didn't manages multiple users configuration and storage of offline measurements.
 * This must be handled by application.
 *
 * Heart Rate Profile Sensor. (HRPS): A HRPS (e.g. PC, phone, etc)
 * is the term used by this profile to describe a device that can perform Heart Rate
 * measurement and notify about on-going measurement and indicate final result to a peer
 * BLE device.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "rwip_task.h" // Task definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/// Messages for Heart Rate Profile Sensor
/*@TRACE*/
enum hrps_msg_id
{
    /// Start the Heart Rate Profile Sensor - at connection
    HRPS_ENABLE_REQ             = MSG_ID(HRPS, 0x00),
    /// Disable confirmation with configuration to save after profile disabled
    HRPS_ENABLE_RSP             = MSG_ID(HRPS, 0x01),
    /// Send Heart Rate measurement value from APP
    HRPS_MEAS_SEND_CMD          = MSG_ID(HRPS, 0x02),
    /// Inform APP of new configuration value
    HRPS_CFG_INDNTF_IND         = MSG_ID(HRPS, 0x03),
    /// Inform APP that Energy Expanded must be reset value
    HRPS_ENERGY_EXP_RESET_IND   = MSG_ID(HRPS, 0x04),
    /// Complete Event Information
    HRPS_CMP_EVT                = MSG_ID(HRPS, 0x05),
};

/// Operation codes
enum hrps_op_codes
{
    /// Database Creation Procedure
    HRPS_RESERVED_OP_CODE      = 0,
    /// Send Measurement value Operation Code
    HRPS_MEAS_SEND_CMD_OP_CODE = 1,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// Parameters of the @ref HRPS_CREATE_DB_REQ message
struct hrps_db_cfg
{
    /// Database configuration
    uint8_t features;
    /// Body Sensor Location
    uint8_t body_sensor_loc;
};

/// Parameters of the @ref HRPS_ENABLE_REQ message
struct hrps_enable_req
{
    /// Connection index
    uint8_t  conidx;
    /// Heart Rate Notification configuration
    uint16_t hr_meas_ntf;
};

/// Parameters of the @ref HRPS_ENABLE_RSP message
struct hrps_enable_rsp
{
    /// Connection index
    uint8_t conidx;
    /// status
    uint16_t status;
};

/// Parameters of the @ref HRPS_CFG_INDNTF_IND message
struct hrps_cfg_indntf_ind
{
    /// Connection idx
    uint8_t  conidx;
    /// Stop/notify/indicate value to configure into the peer characteristic
    uint16_t cfg_val;
};

/// Parameters of the @ref HRPS_MEAS_SEND_CMD message
struct hrps_meas_send_cmd
{
    /// Bit field of connection index on which measurement will be send
    /// will be mask according to client that supports measurement reception
    uint32_t      conidx_bf;
    /// Heart Rate measurement
    hrs_hr_meas_t meas_val;
};

/// Parameters of the @ref HRPS_CMP_EVT message
struct hrps_cmp_evt
{
    /// Operation code
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// Parameters of the @ref HRPS_ENERGY_EXP_RESET_IND message
struct hrps_energy_exp_reset_ind
{
    /// Connection index
    uint8_t conidx;
};

/// @} HRPS

#endif /* _HRPS_MSG_H_ */
