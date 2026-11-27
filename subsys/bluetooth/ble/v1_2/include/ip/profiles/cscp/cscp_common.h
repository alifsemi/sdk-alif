/**
 ****************************************************************************************
 *
 * @file cscp_common.h
 *
 * @brief Header File - Cycling Speed and Cadence Profile common types.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _CSCP_COMMON_H_
#define _CSCP_COMMON_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup CSCP Cycling Speed and Cadence Profile
 * @ingroup Profile
 * @brief Cycling Speed and Cadence Profile
 *
 * The Cycling Speed and Cadence profile enables a Collector device to connect and
 * interact with a Cycling Speed and Cadence Sensor for use in sports and fitness
 * applications.
 *
 * This file contains all definitions that are common for the server and the client parts
 * of the profile.
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */


#include <stdint.h>
#include "prf_types.h"
#include "co_math.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Procedure Already in Progress Error Code
#define CSCP_ERROR_PROC_IN_PROGRESS         (0x80)
/// Client Characteristic Configuration descriptor improperly configured Error Code
#define CSCP_ERROR_CCC_INVALID_PARAM        (0x81)

/// CSC Measurement Value Min Length
#define CSCP_CSC_MEAS_MIN_LEN               (1)
/// CSC Measurement Value Max Length
#define CSCP_CSC_MEAS_MAX_LEN               (11)
/// SC Control Point Request Value Min Length
#define CSCP_SC_CNTL_PT_REQ_MIN_LEN         (1)
/// SC Control Point Request Value Max Length
#define CSCP_SC_CNTL_PT_REQ_MAX_LEN         (5)
/// SC Control Point Response Value Min Length
#define CSCP_SC_CNTL_PT_RSP_MIN_LEN         (3)
/// SC Control Point Response Value Max Length
#define CSCP_SC_CNTL_PT_RSP_MAX_LEN         (CSCP_SC_CNTL_PT_RSP_MIN_LEN + CSCP_LOC_MAX)

/// CSC Measurement Flags All present
#define CSCP_MEAS_ALL_PRESENT               (0x03)

/// CSC Feature Flags All supported
#define CSCP_FEAT_ALL_SUPP                  (0x0007)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Cycling Speed and Cadence Service Characteristics
enum cscp_cscs_char
{
    /// CSC Measurement
    CSCP_CSCS_CSC_MEAS_CHAR,
    /// CSC Feature
    CSCP_CSCS_CSC_FEAT_CHAR,
    /// Sensor Location
    CSCP_CSCS_SENSOR_LOC_CHAR,
    /// SC Control Point
    CSCP_CSCS_SC_CTNL_PT_CHAR,

    CSCP_CSCS_CHAR_MAX,
};

/// CSC Measurement Flags bit field
enum cscp_meas_flags_bf
{
    /// Wheel Revolution Data Present
    CSCP_MEAS_WHEEL_REV_DATA_PRESENT_POS = 0,
    CSCP_MEAS_WHEEL_REV_DATA_PRESENT_BIT = CO_BIT(CSCP_MEAS_WHEEL_REV_DATA_PRESENT_POS),

    /// Crank Revolution Data Present
    CSCP_MEAS_CRANK_REV_DATA_PRESENT_POS = 1,
    CSCP_MEAS_CRANK_REV_DATA_PRESENT_BIT = CO_BIT(CSCP_MEAS_CRANK_REV_DATA_PRESENT_POS),
};

/// CSC Feature Flags bit field
enum cscp_feat_flags_bf
{
    /// Wheel Revolution Data Supported
    CSCP_FEAT_WHEEL_REV_DATA_SUPP_POS = 0,
    CSCP_FEAT_WHEEL_REV_DATA_SUPP_BIT = CO_BIT(CSCP_FEAT_WHEEL_REV_DATA_SUPP_POS),

    /// Crank Revolution Data Supported
    CSCP_FEAT_CRANK_REV_DATA_SUPP_POS = 1,
    CSCP_FEAT_CRANK_REV_DATA_SUPP_BIT = CO_BIT(CSCP_FEAT_CRANK_REV_DATA_SUPP_POS),

    /// Multiple Sensor Location Supported
    CSCP_FEAT_MULT_SENSOR_LOC_SUPP_POS = 2,
    CSCP_FEAT_MULT_SENSOR_LOC_SUPP_BIT = CO_BIT(CSCP_FEAT_MULT_SENSOR_LOC_SUPP_POS),
};

/// Sensor Locations Keys
enum cscp_sensor_loc
{
    /// Other (0)
    CSCP_LOC_OTHER          = 0,
    /// Front Wheel (4)
    CSCP_LOC_FRONT_WHEEL    = 4,
    /// Left Crank (5)
    CSCP_LOC_LEFT_CRANK,
    /// Right Crank (6)
    CSCP_LOC_RIGHT_CRANK,
    /// Left Pedal (7)
    CSCP_LOC_LEFT_PEDAL,
    /// Right Pedal (8)
    CSCP_LOC_RIGHT_PEDAL,
    /// Rear Dropout (9)
    CSCP_LOC_REAR_DROPOUT,
    /// Chainstay (10)
    CSCP_LOC_CHAINSTAY,
    /// Front Hub (11)
    CSCP_LOC_FRONT_HUB,
    /// Rear Wheel (12)
    CSCP_LOC_REAR_WHEEL,
    /// Rear Hub (13)
    CSCP_LOC_REAR_HUB,

    CSCP_LOC_MAX,
};

/// Control Point Operation Code Keys
enum cscp_sc_ctnl_pt_op_code
{
    /// Reserved value
    CSCP_CTNL_PT_OP_RESERVED        = 0,

    /// Set Cumulative Value
    CSCP_CTNL_PT_OP_SET_CUMUL_VAL,
    /// Start Sensor Calibration
    CSCP_CTNL_PT_OP_START_CALIB,
    /// Update Sensor Location
    CSCP_CTNL_PT_OP_UPD_LOC,
    /// Request Supported Sensor Locations
    CSCP_CTNL_PT_OP_REQ_SUPP_LOC,

    /// Response Code
    CSCP_CTNL_PT_RSP_CODE           = 16,
};

/// Control Point Response Value
enum cscp_ctnl_pt_resp_val
{
    /// Reserved value
    CSCP_CTNL_PT_RESP_RESERVED      = 0,

    /// Success
    CSCP_CTNL_PT_RESP_SUCCESS,
    /// Operation Code Not Supported
    CSCP_CTNL_PT_RESP_NOT_SUPP,
    /// Invalid Parameter
    CSCP_CTNL_PT_RESP_INV_PARAM,
    /// Operation Failed
    CSCP_CTNL_PT_RESP_FAILED,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// CSC Measurement
typedef struct cscp_csc_meas
{
    /// Flags
    uint8_t  flags;
    /// Cumulative Crank Revolution
    uint16_t cumul_crank_rev;
    /// Last Crank Event Time
    uint16_t last_crank_evt_time;
    /// Last Wheel Event Time
    uint16_t last_wheel_evt_time;

    // meaningless on server side
    /// Cumulative Wheel Revolution
    uint32_t cumul_wheel_rev;
} cscp_csc_meas_t;

/// CP Request value definition
union cscp_sc_ctnl_pt_req_val
{
    /// Sensor Location
    uint8_t  sensor_loc;
    /// Cumulative value
    uint32_t cumul_val;
};

/// CP Response value definition
union cscp_sc_ctnl_pt_rsp_val
{
    /// Supported sensor locations
    uint16_t supp_sensor_loc;

    // Only available on server side
    /// Sensor Location
    uint8_t  sensor_loc;
    /// New Cumulative Wheel revolution Value
    uint32_t cumul_wheel_rev;
};


/// @} cscp_common

#endif //(_CSCP_COMMON_H_)
