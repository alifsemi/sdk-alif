/**
 ****************************************************************************************
 *
 * @file glps_msg.h
 *
 * @brief Header file - Glucose Profile Sensor - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _GLPS_MSG_H_
#define _GLPS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup GLPS Glucose Profile Sensor
 * @ingroup GLP
 * @brief Glucose Profile Sensor - Message API
 *
 * Glucose sensor (GLS) profile provides functionalities to upper layer module
 * application. The device using this profile takes the role of Glucose sensor.
 *
 * The interface of this role to the Application is:
 *  - Enable the profile role (from APP)
 *  - Disable the profile role (from APP)
 *  - Notify peer device during Glucose measurement (from APP)
 *  - Indicate measurements performed to peer device (from APP)
 *
 * Profile didn't manages multiple users configuration and storage of offline measurements.
 * This must be handled by application.
 *
 * Glucose Profile Sensor. (GLPS): A GLPS (e.g. PC, phone, etc)
 * is the term used by this profile to describe a device that can perform Glucose
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
#include "rwip_task.h" // Task definitions
#include "glp_common.h"

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Messages for Glucose Profile Sensor
/*@TRACE*/
enum glps_msg_id
{
    /// Start the Glucose Profile Sensor - at connection
    GLPS_ENABLE_REQ                 = MSG_ID(GLPS, 0x00),
    /// Confirm that Glucose Profile Sensor is started.
    GLPS_ENABLE_RSP                 = MSG_ID(GLPS, 0x01),

    ///Inform APP of new configuration value
    GLPS_CFG_INDNTF_IND             = MSG_ID(GLPS, 0x02),

    /// Record Access Control Point Request
    GLPS_RACP_REQ_RCV_IND           = MSG_ID(GLPS, 0x03),

    /// Record Access Control Point Resp
    GLPS_SEND_RACP_RSP_CMD          = MSG_ID(GLPS, 0x04),

    /// Send Glucose measurement with context information
    GLPS_SEND_MEAS_WITH_CTX_CMD     = MSG_ID(GLPS, 0x05),
    /// Send Glucose measurement without context information
    GLPS_SEND_MEAS_WITHOUT_CTX_CMD  = MSG_ID(GLPS, 0x06),

    /// Inform that requested action has been performed
    GLPS_CMP_EVT                    = MSG_ID(GLPS, 0x07),
};

/// Type of request completed
enum glps_request_type
{
    /// Glucose measurement notification sent completed
    GLPS_SEND_MEAS,
    /// Record Access Control Point Response Indication
    GLPS_SEND_RACP_RSP
};


/// Indication/notification configuration (put in feature flag to optimize memory usage)
enum glps_evt_cfg_bf
{
    /// Bit used to know if Glucose measurement notification is enabled
    GLPS_MEAS_NTF_CFG_BIT      = CO_BIT(0),
    GLPS_MEAS_NTF_CFG_POS      = 0,
    /// Bit used to know if Glucose measurement context notification is enabled
    GLPS_MEAS_CTX_NTF_CFG_BIT  = CO_BIT(1),
    GLPS_MEAS_CTX_NTF_CFG_POS  = 1,
    /// Bit used to know if Glucose measurement context indication is enabled
    GLPS_RACP_IND_CFG_BIT      = CO_BIT(3),
    GLPS_RACP_IND_CFG_POS      = 3,
};


/// Parameters of the Glucose service database
struct glps_db_cfg
{
    /// Glucose Feature (see enum #glps_feat_bf)
    uint16_t features;
    /// Measurement context supported
    uint8_t  meas_ctx_supported;
};


/// Parameters of the @ref GLPS_ENABLE_REQ message
struct glps_enable_req
{
    /// Connection index
    uint8_t  conidx;
    /// Glucose indication/notification configuration (see enum #glps_evt_cfg_bf)
    uint8_t evt_cfg;
};

/// Parameters of the @ref GLPS_ENABLE_RSP message
struct glps_enable_rsp
{
    /// Connection index
    uint8_t  conidx;
    /// Status
    uint16_t status;
};

/// Parameters of the @ref GLPS_CFG_INDNTF_IND message
struct glps_cfg_indntf_ind
{
    /// Connection index
    uint8_t conidx;
    /// Glucose indication/notification configuration (see enum #glps_evt_cfg_bf)
    uint8_t evt_cfg;
};

/// Parameters of the @ref GLPS_SEND_MEAS_WITH_CTX_CMD message
struct glps_send_meas_with_ctx_cmd
{
    /// Connection index
    uint8_t        conidx;
    /// Sequence Number
    uint16_t       seq_num;
    /// Glucose measurement
    glp_meas_t     meas;
    /// Glucose measurement context
    glp_meas_ctx_t ctx;
};


/// Parameters of the @ref GLPS_SEND_MEAS_WITHOUT_CTX_CMD message
struct glps_send_meas_without_ctx_cmd
{
    /// Connection index
    uint8_t    conidx;
    /// Sequence Number
    uint16_t   seq_num;
    /// Glucose measurement
    glp_meas_t meas;
};


/// Parameters of the @ref GLPS_RACP_REQ_RCV_IND message
struct glps_racp_req_rcv_ind
{
    /// Connection index
    uint8_t          conidx;
    /// Record access control point request
    glp_racp_req_t   req;
};

/// Parameters of the @ref GLPS_SEND_RACP_RSP_CMD message
struct glps_send_racp_rsp_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// Number of records.
    uint16_t num_of_record;
    /// operation code (see enum #glp_racp_op_code)
    uint8_t  op_code;
    /// Command status (see enum #glp_racp_status)
    uint8_t  racp_status;
};

/// Parameters of the @ref GLPS_CMP_EVT message
struct glps_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// completed request (see enum #glps_request_type)
    uint8_t  request;
    /// Command status
    uint16_t status;
};

/// @} GLPS

#endif /* _GLPS_MSG_H_ */
