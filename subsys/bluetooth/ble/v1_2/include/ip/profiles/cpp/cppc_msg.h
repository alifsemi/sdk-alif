/**
 ****************************************************************************************
 *
 * @file cppc_msg.h
 *
 * @brief Header file - Cycling Power Profile Collector Role - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _CPPC_MSG_H_
#define _CPPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup CPPC
 * @ingroup Profile
 * @brief Cycling Power Profile Collector - Message API
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */


#include "rwip_task.h" // Task definitions
#include "cpp_common.h"


/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Cycling Power Service Characteristic Descriptors
enum cppc_cps_descs
{
    /// CP Measurement Char. - Client Characteristic Configuration
    CPPC_DESC_CP_MEAS_CL_CFG,
    /// CP Measurement Char. - Server Characteristic Configuration
    CPPC_DESC_CP_MEAS_SV_CFG,
    /// CP Vector Char. - Client Characteristic Configuration
    CPPC_DESC_VECTOR_CL_CFG,
    /// Control Point Char. - Client Characteristic Configuration
    CPPC_DESC_CTNL_PT_CL_CFG,

    CPPC_DESC_MAX,

    CPPC_DESC_MASK = 0x10,
};

/// Message IDs
/*@TRACE*/
enum cppc_msg_ids
{
    /// Enable the Cycling Power Profile Collector task - at connection
    CPPC_ENABLE_REQ             = MSG_ID(CPPC, 0x00),
    /// Confirm that cfg connection has finished with discovery results, or that normal cnx started
    CPPC_ENABLE_RSP             = MSG_ID(CPPC, 0x01),

    /// Read the value of an attribute in the peer device database
    CPPC_READ_CMD               = MSG_ID(CPPC, 0x02),
    /// Configure sending of notification or indication
    CPPC_CFG_NTFIND_CMD         = MSG_ID(CPPC, 0x03),

    /// Configure the Cycling Power Control Point value
    CPPC_CTNL_PT_CFG_CMD        = MSG_ID(CPPC, 0x04),
    /// Indicate that an control point response has been triggered by peer device
    CPPC_CTNL_PT_IND            = MSG_ID(CPPC, 0x05),

    /// Indicate that an attribute value has been received either upon notification or read response
    CPPC_VALUE_IND              = MSG_ID(CPPC, 0x06),

    /// Complete Event Information
    CPPC_CMP_EVT                = MSG_ID(CPPC, 0x07),
};

/// Operation Codes
enum cppc_op_code
{
    /// Reserved operation code
    CPPC_RESERVED_OP_CODE           = 0x00,

    /// Read attribute value Procedure
    CPPC_READ_OP_CODE               = 0x01,

    /// Wait for the Write Response after having written a Client Char. Cfg. Descriptor.
    CPPC_CFG_NTF_IND_OP_CODE        = 0x02,

    /// Wait for the Write Response after having written the Control Point Char.
    CPPC_CTNL_PT_CFG_WR_OP_CODE     = 0x03,
    /// Wait for the Indication Response after having written the Control Point Char.
    CPPC_CTNL_PT_CFG_IND_OP_CODE    = 0x04,
};

/// Codes for reading/writing a CPS characteristic with one single request
enum cppc_code
{
    /// Notified CP Measurement
    CPPC_NTF_CP_MEAS          = CPP_CPS_MEAS_CHAR,
    /// Read CP Feature
    CPPC_RD_CP_FEAT           = CPP_CPS_FEAT_CHAR,
    /// Read Sensor Location
    CPPC_RD_SENSOR_LOC        = CPP_CPS_SENSOR_LOC_CHAR,
    /// Notified Vector
    CPPC_NTF_CP_VECTOR        = CPP_CPS_VECTOR_CHAR,
    /// Indicated SC Control Point
    CPPC_IND_CTNL_PT          = CPP_CPS_CTNL_PT_CHAR,

    /// Read/Write CP Measurement Client Char. Configuration Descriptor
    CPPC_RD_WR_CP_MEAS_CL_CFG    = (CPPC_DESC_CP_MEAS_CL_CFG   | CPPC_DESC_MASK),
    /// Read/Write CP Measurement Server Char. Configuration Descriptor
    CPPC_RD_WR_CP_MEAS_SV_CFG    = (CPPC_DESC_CP_MEAS_SV_CFG   | CPPC_DESC_MASK),

    /// Read/Write Vector Client Char. Configuration Descriptor
    CPPC_RD_WR_VECTOR_CFG    = (CPPC_DESC_VECTOR_CL_CFG   | CPPC_DESC_MASK),
    /// Read/Write CP Control Point Client Char. Configuration Descriptor
    CPPC_RD_WR_CTNL_PT_CFG  = (CPPC_DESC_CTNL_PT_CL_CFG | CPPC_DESC_MASK),
};

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Cycling Power Service
 */
typedef struct cppc_cps_content
{
    /// Service info
    prf_svc_t svc;

    /// Characteristic info:
    ///  - CP Measurement
    ///  - CP Feature
    ///  - Sensor Location
    ///  - Vector
    ///  - SC Control Point
    prf_char_t chars[CPP_CPS_CHAR_MAX];

    /// Descriptor handles:
    ///  - CP Measurement Client Cfg
    ///  - Vector Client Cfg
    ///  - Control Point Client Cfg
    prf_desc_t descs[CPPC_DESC_MAX];
} cppc_cps_content_t;

/// Parameters of the @ref CPPC_ENABLE_REQ message
struct cppc_enable_req
{
    ///Connection index
    uint8_t            conidx;
    /// Connection type
    uint8_t            con_type;
    /// Existing handle values CPS
    cppc_cps_content_t cps;
};

/// Parameters of the @ref CPPC_ENABLE_RSP message
struct cppc_enable_rsp
{
    ///Connection index
    uint8_t            conidx;
    /// status
    uint16_t           status;
    /// Existing handle values CPS
    cppc_cps_content_t cps;
};

/// Parameters of the @ref CPPC_READ_CMD message
struct cppc_read_cmd
{
    ///Connection index
    uint8_t  conidx;
    /// Operation Code
    uint8_t  operation;
    /// Read code
    uint8_t  read_code;
};

/// Parameters of the @ref CPPC_CFG_NTFIND_CMD message
struct cppc_cfg_ntfind_cmd
{
    ///Connection index
    uint8_t  conidx;
    /// Operation Code
    uint8_t  operation;
    /// Descriptor code
    uint8_t  desc_code;
    /// Ntf/Ind Configuration
    uint16_t ntfind_cfg;
};

/// Parameters of the @ref CPPC_CTNL_PT_CFG_CMD message
struct cppc_ctnl_pt_cfg_cmd
{
    ///Connection index
    uint8_t                   conidx;
    /// Operation Code
    uint8_t                   operation;
    /// Operation Code
    uint8_t                   op_code;
    /// Value
    union cpp_ctnl_pt_req_val value;
};

/// Parameters of the @ref CPPC_VALUE_IND message
struct cppc_value_ind
{
    ///Connection index
    uint8_t  conidx;
    /// Attribute Code
    uint8_t  att_code;
    /// Value
    union cppc_value_tag
    {
        /// CP Feature
        uint32_t        sensor_feat;
        /// CP Measurement
        cpp_cp_meas_t   cp_meas;
        /// Sensor Vector
        cpp_cp_vector_t cp_vector;
        /// Client Characteristic Configuration Descriptor Value
        uint16_t        ntf_cfg;
        /// Sensor Location
        uint8_t         sensor_loc;
    } value;
};

/// Parameters of the @ref CPPC_CTNL_PT_IND message
struct cppc_ctnl_pt_ind
{
    ///Connection index
    uint8_t                   conidx;
    /// Operation code - Response Code (0x20)
    uint8_t                   op_code;
    /// Requested Operation Code
    uint8_t                   req_op_code;
    /// Response Value
    uint8_t                   resp_value;
    ///Value
    union cpp_ctnl_pt_rsp_val value;
};

/// Parameters of the @ref CPPC_CMP_EVT message
struct cppc_cmp_evt
{
    ///Connection index
    uint8_t  conidx;
    /// Operation code
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} CPPC

#endif //(_CPPC_MSG_H_)
