/**
 ****************************************************************************************
 *
 * @file cscps_msg.h
 *
 * @brief Header file - Cycling Speed and Cadence Profile Sensor - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _CSCPS_MSG_H_
#define _CSCPS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup CSCPS
 * @ingroup Profile
 * @brief Cycling Speed and Cadence Profile - Message API.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "cscp_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Cycling Speed and Cadence Profile Sensor
/*@TRACE*/
enum cscps_msg_id
{
    /// Enable the CSCP Sensor task for a connection
    CSCPS_ENABLE_REQ                = MSG_ID(CSCPS, 0x00),
    /// Enable the CSCP Sensor task for a connection
    CSCPS_ENABLE_RSP                = MSG_ID(CSCPS, 0x01),

    /// Send a CSC Measurement to the peer device (Notification)
    CSCPS_NTF_CSC_MEAS_CMD          = MSG_ID(CSCPS, 0x02),

    /// Indicate that the SC Control Point characteristic value has been written
    CSCPS_SC_CTNL_PT_REQ_RECV_IND   = MSG_ID(CSCPS, 0x03),
    /// Application response after receiving a CSCPS_SC_CTNL_PT_REQ_RECV_IND message
    CSCPS_SC_CTNL_PT_RSP_SEND_CMD   = MSG_ID(CSCPS, 0x04),

    /// Indicate a new NTF/IND configuration to the application
    CSCPS_CFG_NTFIND_IND            = MSG_ID(CSCPS, 0x05),

    /// Send a complete event status to the application
    CSCPS_CMP_EVT                   = MSG_ID(CSCPS, 0x06),
};

/// Operation Code used in the profile state machine
enum cscps_op_code
{
    /// Reserved Operation Code
    CSCPS_RESERVED_OP_CODE          = 0x00,
    /// Send CSC Measurement Operation Code
    CSCPS_SEND_CSC_MEAS_OP_CODE     = 0x01,
    /// Send SC Control point response
    CSCPS_CTNL_PT_RSP_OP_CODE       = 0x02,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the initialization function
struct cscps_db_cfg
{
    /**
     * CSC Feature Value - Not supposed to be modified during the lifetime of the device
     * This value is used to decide if the SC Control Point Characteristic is part of the
     * Cycling Speed and Cadence service.
     */
    uint16_t    csc_feature;
    /**
     * Indicate if the Sensor Location characteristic is supported. Note that if the Multiple
     * Sensor Location feature is set has supported in the csc_feature parameter, the
     * characteristic will be added (mandatory).
     */
    uint8_t     sensor_loc_supp;
    /// Sensor location
    uint8_t     sensor_loc;
    /// Wheel revolution
    uint32_t    wheel_rev;
};

/// Parameters of the @ref CSCPS_ENABLE_REQ message
struct cscps_enable_req
{
    /// Connection index
    uint8_t  conidx;
    /// Stored CSC Measurement Char. Client Characteristic Configuration
    uint16_t csc_meas_ntf_cfg;
    /// Stored SC Control Point Char. Client Characteristic Configuration
    uint16_t sc_ctnl_pt_ntf_cfg;
};

/// Parameters of the @ref CSCPS_ENABLE_RSP message
struct cscps_enable_rsp
{
    /// Connection index
    uint8_t  conidx;
    /// Status
    uint16_t status;
};

/// Parameters of the @ref cscps_ntf_csc_meas_cmd message
struct cscps_ntf_csc_meas_cmd
{
    /// Bit field of connection index on which measurement will be send
    /// will be mask according to client that supports measurement reception
    uint32_t        conidx_bf;
    /// Wheel Revolution since the last wheel event time
    int16_t         wheel_rev;
    /// CSC Measurement information
    cscp_csc_meas_t csc_meas;
};


/// Parameters of the @ref CSCPS_SC_CTNL_PT_REQ_RECV_IND message
struct cscps_sc_ctnl_pt_req_recv_ind
{
    /// Connection index
    uint8_t                         conidx;
    /// Operation Code (see enum #cscp_sc_ctnl_pt_op_code)
    uint8_t                         op_code;
    /// Value
    union cscp_sc_ctnl_pt_req_val   value;
};

/// Parameters of the @ref CSCPS_SC_CTNL_PT_RSP_SEND_CMD message
struct cscps_sc_ctnl_pt_rsp_send_cmd
{
    /// Connection index
    uint8_t                         conidx;
    /// Operation Code (see #cscp_sc_ctnl_pt_op_code)
    uint8_t                         op_code;
    /// Response Value (see enum #cscp_ctnl_pt_resp_val)
    uint8_t                         resp_value;
    /// Value
    union cscp_sc_ctnl_pt_rsp_val  value;
};

/// Parameters of the @ref CSCPS_CFG_NTFIND_IND message
struct cscps_cfg_ntfind_ind
{
    /// Connection handle
    uint8_t  conidx;
    /// Characteristic Code (CSC Measurement or SC Control Point)
    uint8_t  char_code;
    /// Char. Client Characteristic Configuration
    uint16_t ntf_cfg;
};

/// Parameters of the @ref CSCPS_CMP_EVT message
struct cscps_cmp_evt
{
    /// Connection index - 0xFF if meaningless (CSCPS_NTF_CSC_MEAS_CMD)
    uint8_t  conidx;
    /// Operation Code
    uint8_t  operation;
    /// Operation Status
    uint16_t status;
};

/// @} CSCPS

#endif //(_CSCPS_MSG_H_)
