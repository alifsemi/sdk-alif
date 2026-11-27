/**
 ****************************************************************************************
 *
 * @file hrpc_msg.h
 *
 * @brief Header file - Heart Rate Profile Collector Role - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _HRPC_MSG_H_
#define _HRPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup HRPC
 * @ingroup Profile
 * @brief Heart Rate Profile Collector  - Message API
 *
 * The HRPC is responsible for providing Heart Rate Profile Collector functionalities
 * to upper layer module or application. The device using this profile takes the role
 * of Heart Rate Profile Collector.
 *
 * Heart Rate Profile Collector. (HRPC): A HRPC (e.g. PC, phone, etc)
 * is the term used by this profile to describe a device that can interpret Heart Rate
 * measurement in a way suitable to the user application.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_task.h" // Task definitions
#include "hrp_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*@TRACE*/
enum hrpc_msg_id
{
    /// Start the Heart Rate profile - at connection
    HRPC_ENABLE_REQ         = MSG_ID(HRPC, 0x00),
    /// Confirm that cfg connection has finished with discovery results, or that normal cnx started
    HRPC_ENABLE_RSP         = MSG_ID(HRPC, 0x01),
    /// Generic message to read a HRS or DIS characteristic value
    HRPC_RD_CHAR_CMD        = MSG_ID(HRPC, 0x02),
    /// Generic message for read responses for APP
    HRPC_RD_CHAR_IND        = MSG_ID(HRPC, 0x03),
    /// Generic message for configuring the 2 characteristics that can be handled
    HRPC_CFG_INDNTF_CMD     = MSG_ID(HRPC, 0x04),
    /// APP request for control point write (to reset Energy Expanded)
    HRPC_WR_CNTL_POINT_CMD  = MSG_ID(HRPC, 0x05),
    /// Heart Rate value send to APP
    HRPC_HR_MEAS_IND        = MSG_ID(HRPC, 0x06),
    /// Complete Event Information
    HRPC_CMP_EVT            = MSG_ID(HRPC, 0x07),
};

/// Operation codes
enum hrpc_op_codes
{
    /// Database Creation Procedure
    HRPC_RESERVED_OP_CODE          = 0,
    /// Read char. Operation Code
    HRPC_RD_CHAR_CMD_OP_CODE       = 1,
    /// CCC Configuration Operation Code
    HRPC_CFG_INDNTF_CMD_OP_CODE    = 2,
    /// Write Control Point Operation Code
    HRPC_WR_CNTL_POINT_CMD_OP_CODE = 3,
};

/// Value identifier
enum hrpc_val_id
{
    /// Body Sensor Location
    HRPC_VAL_BODY_SENSOR_LOCATION,
    /// Heart Rate Measurement client config
    HRPC_VAL_HR_MEAS_CLI_CFG
};


/// Characteristics
enum
{
    /// Heart Rate Measurement
    HRPC_CHAR_HR_MEAS,
    /// Body Sensor Location
    HRPC_CHAR_BODY_SENSOR_LOCATION,
    /// Heart Rate Control Point
    HRPC_CHAR_HR_CNTL_POINT,

    HRPC_CHAR_MAX,
};

/// Characteristic descriptors
enum
{
    /// Heart Rate Measurement client config
    HRPC_DESC_HR_MEAS_CLI_CFG,
    HRPC_DESC_MAX,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

///Structure containing the characteristics handles, value handles and descriptors
typedef struct hrs_content
{
    /// service info
    prf_svc_t           svc;

    /// characteristic info:
    ///  - Heart Rate Measurement
    ///  - Body Sensor Location
    ///  - Heart Rate Control Point
    prf_char_t      chars[HRPC_CHAR_MAX];

    /// Descriptor handles:
    ///  - Heart Rate Measurement client cfg
    prf_desc_t descs[HRPC_DESC_MAX];
} hrs_content_t;

/// Parameters of the @ref HRPC_ENABLE_REQ message
struct hrpc_enable_req
{
    /// Connection index
    uint8_t       conidx;
    /// Connection type
    uint8_t       con_type;
    /// Existing handle values hrs
    hrs_content_t hrs;
};

/// Parameters of the @ref HRPC_ENABLE_RSP message
struct hrpc_enable_rsp
{
    /// Connection index
    uint8_t       conidx;
    /// status
    uint16_t      status;
    /// Existing handle values hrs
    hrs_content_t hrs;
};

/// Parameters of the @ref HRPC_CFG_INDNTF_CMD message
struct hrpc_cfg_indntf_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// Stop/notify/indicate value to configure into the peer characteristic
    uint16_t cfg_val;
};

/// Parameters of the @ref HRPC_RD_CHAR_CMD message
struct hrpc_rd_char_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Value Id (see enum #hrpc_val_id)
    uint8_t val_id;
};

/// Parameters of the @ref HRPC_RD_CHAR_IND message
struct hrpc_rd_char_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Value Id (see enum #hrpc_val_id)
    uint8_t  val_id;
    /// Characteristic value length
    uint16_t length;
    /// Characteristic value
    uint8_t  value[__ARRAY_EMPTY];
};

/// Parameters of the @ref HRPC_WR_CNTL_POINT_CMD message
struct hrpc_wr_cntl_point_cmd
{
    /// Connection index
    uint8_t conidx;
    /// value
    uint8_t val;
};

/// Parameters of the @ref HRPC_HR_MEAS_IND message
struct hrpc_hr_meas_ind
{
    /// Connection index
    uint8_t       conidx;
    /// Heart Rate measurement
    hrs_hr_meas_t meas_val;
};

/// Parameters of the @ref HRPC_CMP_EVT message
struct hrpc_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} HRPC

#endif /* _HRPC_MSG_H_ */
