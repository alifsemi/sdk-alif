/**
 ****************************************************************************************
 *
 * @file cscpc_msg.h
 *
 * @brief Header file - Cycling Speed and Cadence Profile Collector Role - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _CSCPC_MSG_H_
#define _CSCPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup CSCPC
 * @ingroup Profile
 * @brief Cycling Speed and Cadence Profile Collector - Message API
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

/// Message IDs
/*@TRACE*/
enum cscpc_msg_ids
{
    /// Enable the Cycling Speed and Cadence Profile Collector task - at connection
    CSCPC_ENABLE_REQ         = MSG_ID(CSCPC, 0x00),
    /// Enable the Cycling Speed and Cadence Profile Collector task - at connection
    CSCPC_ENABLE_RSP         = MSG_ID(CSCPC, 0x01),

    /// Read the value of an attribute in the peer device database
    CSCPC_READ_CMD           = MSG_ID(CSCPC, 0x02),

    /// Configure sending of notification or indication
    CSCPC_CFG_NTFIND_CMD     = MSG_ID(CSCPC, 0x03),

    /// Configure the SC Control Point value
    CSCPC_CTNL_PT_CFG_CMD    = MSG_ID(CSCPC, 0x04),
    /// SC Control Point value response
    CSCPC_CTNL_PT_IND        = MSG_ID(CSCPC, 0x05),

    /// Indicate that an attribute value has been received either upon notification or read response
    CSCPC_VALUE_IND          = MSG_ID(CSCPC, 0x06),

    /// Complete Event Information
    CSCPC_CMP_EVT            = MSG_ID(CSCPC, 0x07),
};

/// Operation Codes
enum cscpc_op_codes
{
    /// Reserved operation code
    CSCPC_RESERVED_OP_CODE         = 0x00,

    /// Read attribute value Procedure
    CSCPC_READ_OP_CODE             = 0x01,
    /// Wait for the Write Response after having written a Client Char. Cfg. Descriptor.
    CSCPC_CFG_NTF_IND_OP_CODE      = 0x02,
    /// Wait for the control point response operation
    CSCPC_CTNL_PT_OP_CODE          = 0x03,
};

/// Cycling Speed and Cadence Service Characteristic Descriptors
enum cscpc_cscs_descs
{
    /// CSC Measurement Char. - Client Characteristic Configuration
    CSCPC_DESC_CSC_MEAS_CL_CFG,
    /// SC Control Point Char. - Client Characteristic Configuration
    CSCPC_DESC_SC_CTNL_PT_CL_CFG,

    CSCPC_DESC_MAX,

    CSCPC_DESC_MASK = 0x10,
};

/// Codes for reading/writing a CSCS characteristic with one single request
enum cscpc_codes
{
    /// Notified CSC Measurement
    CSCPC_NTF_CSC_MEAS          = CSCP_CSCS_CSC_MEAS_CHAR,
    /// Read CSC Feature
    CSCPC_RD_CSC_FEAT           = CSCP_CSCS_CSC_FEAT_CHAR,
    /// Read Sensor Location
    CSCPC_RD_SENSOR_LOC         = CSCP_CSCS_SENSOR_LOC_CHAR,
    /// Indicated SC Control Point
    CSCPC_IND_SC_CTNL_PT        = CSCP_CSCS_SC_CTNL_PT_CHAR,

    /// Read/Write CSC Measurement Client Char. Configuration Descriptor
    CSCPC_RD_WR_CSC_MEAS_CFG    = (CSCPC_DESC_CSC_MEAS_CL_CFG   | CSCPC_DESC_MASK),
    /// Read SC Control Point Client Char. Configuration Descriptor
    CSCPC_RD_WR_SC_CTNL_PT_CFG  = (CSCPC_DESC_SC_CTNL_PT_CL_CFG | CSCPC_DESC_MASK),
};


/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Cycling Speed and Cadence Service
 */
typedef struct cscpc_cscs_content
{
    /// Service info
    prf_svc_t           svc;

    /// Characteristic info:
    ///  - CSC Measurement
    ///  - CSC Feature
    ///  - Sensor Location
    ///  - SC Control Point
    prf_char_t      chars[CSCP_CSCS_CHAR_MAX];

    /// Descriptor handles:
    ///  - CSC Measurement Client Cfg
    ///  - SC Control Point Client Cfg
    prf_desc_t descs[CSCPC_DESC_MAX];
} cscpc_cscs_content_t;

/// Parameters of the @ref CSCPC_ENABLE_REQ message
struct cscpc_enable_req
{
    /// Connection index
    uint8_t              conidx;
    /// Connection type
    uint8_t              con_type;
    /// Existing handle values CSCS
    cscpc_cscs_content_t cscs;
};

/// Parameters of the @ref CSCPC_ENABLE_RSP message
struct cscpc_enable_rsp
{
    /// Connection index
    uint8_t              conidx;
    /// status
    uint16_t             status;
    /// Existing handle values CPS
    cscpc_cscs_content_t cscs;
};

/// Parameters of the @ref CSCPC_READ_CMD message
struct cscpc_read_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Operation Code
    uint8_t operation;
    /// Read code
    uint8_t read_code;
};

/// Parameters of the @ref CSCPC_CFG_NTFIND_CMD message
struct cscpc_cfg_ntfind_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// Operation Code
    uint8_t  operation;
    /// Descriptor code
    uint8_t  desc_code;
    /// Ntf/Ind Configuration
    uint16_t ntfind_cfg;
};

/// Parameters of the @ref CSCPC_CTNL_PT_CFG_CMD message
struct cscpc_ctnl_pt_cfg_cmd
{
    /// Connection index
    uint8_t                       conidx;
    /// Operation Code
    uint8_t                       operation;
    /// Operation Code  (see #cscp_sc_ctnl_pt_op_code)
    uint8_t                       op_code;
    /// SC Control Point Request value
    union cscp_sc_ctnl_pt_req_val value;
};

/// Parameters of the @ref CSCPC_CTNL_PT_IND message
struct cscpc_ctnl_pt_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Control Point Op code  (see #cscp_sc_ctnl_pt_op_code) - 16
    uint8_t  cp_opcode;
    /// Requested Operation Code (see #cscp_sc_ctnl_pt_op_code)
    uint8_t  req_op_code;
    /// Response Value (see enum #cscp_ctnl_pt_resp_val)
    uint8_t  resp_value;
    /// List of supported locations (see enum #cscp_sensor_loc)
    uint16_t supp_sensor_loc;
};


/// Parameters of the @ref CSCPC_VALUE_IND message
struct cscpc_value_ind
{
    /// Connection index
    uint8_t conidx;
    /// Attribute Code
    uint8_t att_code;
    /// Value
    union cscpc_value_tag
    {
        /// CSC Measurement
        cscp_csc_meas_t csc_meas;
        /// CSC Feature
        uint16_t        sensor_feat;
        /// Sensor Location
        uint8_t         sensor_loc;
        /// Client Characteristic Configuration Descriptor Value
        uint16_t        ntf_cfg;
    } value;
};

/// Parameters of the @ref CSCPC_CMP_EVT message
struct cscpc_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation code
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} CSCPC

#endif //(_CSCPC_MSG_H_)
