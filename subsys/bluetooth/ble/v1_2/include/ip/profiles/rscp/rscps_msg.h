/**
 ****************************************************************************************
 *
 * @file rscps_msg.h
 *
 * @brief Header file - Running Speed and Cadence Profile Sensor - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _RSCPS_MSG_H_
#define _RSCPS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup RSCPS
 * @ingroup Profile
 * @brief Running Speed and Cadence Profile - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "rscp_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Messages for Running Speed and Cadence Profile Sensor
/*@TRACE*/
enum rscps_msg_id
{
    /// Enable the RSCP Sensor task for a connection
    RSCPS_ENABLE_REQ              = MSG_ID(RSCPS, 0x00),
    /// Enable the RSCP Sensor task for a connection
    RSCPS_ENABLE_RSP              = MSG_ID(RSCPS, 0x01),

    /// Send a RSC Measurement to the peer device (Notification)
    RSCPS_NTF_RSC_MEAS_CMD        = MSG_ID(RSCPS, 0x02),

    /// Indicate that the SC Control Point characteristic value has been written
    RSCPS_SC_CTNL_PT_REQ_RECV_IND = MSG_ID(RSCPS, 0x03),
    /// Application response after receiving a RSCPS_SC_CTNL_PT_REQ_RECV_IND message
    RSCPS_SC_CTNL_PT_RSP_SEND_CMD = MSG_ID(RSCPS, 0x04),

    /// Indicate a new NTF/IND configuration to the application
    RSCPS_CFG_NTFIND_IND          = MSG_ID(RSCPS, 0x05),

    /// Send a complete event status to the application
    RSCPS_CMP_EVT                 = MSG_ID(RSCPS, 0x06),
};

/// Operation Code used in the profile state machine
enum rscps_op_code
{
    /// Reserved Operation Code
    RSCPS_RESERVED_OP_CODE          = 0x00,
    /// Send RSC Measurement Operation Code
    RSCPS_SEND_RSC_MEAS_OP_CODE     = 0x01,
    /// Send Control point response Operation Code
    RSCPS_CTNL_PT_RSP_SEND_OP_CODE  = 0x02,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the initialization function
struct rscps_db_cfg
{
    /**
     * RSC Feature Value - Not supposed to be modified during the lifetime of the device
     * This value is used to decide if the SC Control Point Characteristic is part of the
     * Running Speed and Cadence service.
     */
    uint16_t rsc_feature;
    /**
     * Indicate if the Sensor Location characteristic is supported. Note that if the Multiple
     * Sensor Location feature is set has supported in the rsc_feature parameter, the
     * characteristic will be added (mandatory).
     */
    uint8_t  sensor_loc_supp;
    /// Sensor location
    uint8_t  sensor_loc;
};

/// Parameters of the @ref RSCPS_ENABLE_REQ message
struct rscps_enable_req
{
    /// Connection handle
    uint16_t conidx;
    /// Stored RSC Measurement Char. Client Characteristic Configuration
    uint16_t rsc_meas_ntf_cfg;
    /// Stored SC Control Point Char. Client Characteristic Configuration
    uint16_t sc_ctnl_pt_ntf_cfg;
};

/// Parameters of the @ref RSCPS_ENABLE_RSP message
struct rscps_enable_rsp
{
    /// Connection index
    uint8_t conidx;
    /// Status
    uint16_t status;
};

/// Parameters of the @ref RSCPS_NTF_RSC_MEAS_CMD message
struct rscps_ntf_rsc_meas_cmd
{
    /// Bit field of connection index on which measurement will be send
    /// will be mask according to client that supports measurement reception
    uint32_t        conidx_bf;
    /// RSC Measurement information
    rscp_rsc_meas_t rsc_meas;
};

/// Parameters of the @ref RSCPS_SC_CTNL_PT_REQ_RECV_IND message
struct rscps_sc_ctnl_pt_req_recv_ind
{
    /// Connection index
    uint8_t                       conidx;
    /// Operation Code (see enum #rscp_sc_ctnl_pt_op_code)
    uint8_t                       op_code;
    /// SC Control Point Request value
    union rscp_sc_ctnl_pt_req_val value;
};

/// Parameters of the @ref RSCPS_SC_CTNL_PT_RSP_SEND_CMD message
struct rscps_sc_ctnl_pt_rsp_send_cmd
{
    /// Connection index
    uint8_t                       conidx;
    /// Requested Operation Code  (see enum #rscp_sc_ctnl_pt_op_code)
    uint8_t                       op_code;
    /// Response Value  (see enum #rscp_sc_ctnl_pt_resp_val)
    uint8_t                       resp_value;
    /// SC Control Point Response value
    union rscp_sc_ctnl_pt_rsp_val value;
};

/// Parameters of the @ref RSCPS_CFG_NTFIND_IND message
struct rscps_cfg_ntfind_ind
{
    /// Connection index
    uint8_t     conidx;
    /// Characteristic Code (RSC Measurement or SC Control Point) (see enum #rscp_rscs_char)
    uint8_t     char_code;
    /// Char. Client Characteristic Configuration
    uint16_t    ntf_cfg;
};

/// Parameters of the @ref RSCPS_CMP_EVT message
struct rscps_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation Code (see enum #rscps_op_code)
    uint8_t  operation;
    /// Operation Status
    uint16_t status;
};

/// @} RSCPS

#endif //(_RSCPS_MSG_H_)
