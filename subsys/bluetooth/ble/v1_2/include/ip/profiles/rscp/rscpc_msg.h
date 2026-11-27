/**
 ****************************************************************************************
 *
 * @file rscpc_msg.h
 *
 * @brief Header file - Running Speed and Cadence Profile Collector Role - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _RSCPC_MSG_H_
#define _RSCPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup RSCPC
 * @ingroup Profile
 * @brief Running Speed and Cadence Profile Collector - Message API
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "rscp_common.h"
#include "prf_types.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Message IDs
/*@TRACE*/
enum rscpc_msg_ids
{
    /// Enable the Running Speed and Cadence Profile Collector task - at connection
    RSCPC_ENABLE_REQ        = MSG_ID(RSCPC, 0x00),
    /// Enable the Running Speed and Cadence Profile Collector task - at connection
    RSCPC_ENABLE_RSP        = MSG_ID(RSCPC, 0x01),
    /// Read the value of an attribute in the peer device database
    RSCPC_READ_CMD          = MSG_ID(RSCPC, 0x02),
    /// Configure sending of notification or indication
    RSCPC_CFG_NTFIND_CMD    = MSG_ID(RSCPC, 0x03),
    /// Configure the SC Control Point value
    RSCPC_CTNL_PT_CFG_CMD   = MSG_ID(RSCPC, 0x04),
    /// SC Control Point value response
    RSCPC_CTNL_PT_IND       = MSG_ID(RSCPC, 0x05),
    /// Indicate that an attribute value has been received either upon notification or read response
    RSCPC_VALUE_IND         = MSG_ID(RSCPC, 0x06),
    /// Complete Event Information
    RSCPC_CMP_EVT           = MSG_ID(RSCPC, 0x07),
};

/// Operation Codes
enum rscpc_op_codes
{
    /// Reserved operation code
    RSCPC_RESERVED_OP_CODE    = 0x00,
    /// Read attribute value Procedure
    RSCPC_READ_OP_CODE        = 0x01,
    /// Wait for the Write Response after having written a Client Char. Cfg. Descriptor.
    RSCPC_CFG_NTF_IND_OP_CODE = 0x02,
    /// Wait for the Write Response after having written the SC Control Point Char.
    RSCPC_CTNL_PT_OP_CODE     = 0x03,
};

/// Running Speed and Cadence Service Characteristic Descriptors
enum rscpc_rscs_descs
{
    /// RSC Measurement Char. - Client Characteristic Configuration
    RSCPC_DESC_RSC_MEAS_CL_CFG,
    /// SC Control Point Char. - Client Characteristic Configuration
    RSCPC_DESC_SC_CTNL_PT_CL_CFG,

    RSCPC_DESC_MAX,

    RSCPC_DESC_MASK = 0x10,
};

/// Internal codes for reading/writing a RSCS characteristic with one single request
enum rscpc_codes
{
    /// Notified RSC Measurement
    RSCPC_NTF_RSC_MEAS          = RSCP_RSCS_RSC_MEAS_CHAR,
    /// Read RSC Feature
    RSCPC_RD_RSC_FEAT           = RSCP_RSCS_RSC_FEAT_CHAR,
    /// Read Sensor Location
    RSCPC_RD_SENSOR_LOC         = RSCP_RSCS_SENSOR_LOC_CHAR,
    /// Indicated SC Control Point
    RSCPC_IND_SC_CTNL_PT        = RSCP_RSCS_SC_CTNL_PT_CHAR,

    /// Read/Write RSC Measurement Client Char. Configuration Descriptor
    RSCPC_RD_WR_RSC_MEAS_CFG    = (RSCPC_DESC_RSC_MEAS_CL_CFG   | RSCPC_DESC_MASK),
    /// Read SC Control Point Client Char. Configuration Descriptor
    RSCPC_RD_WR_SC_CTNL_PT_CFG  = (RSCPC_DESC_SC_CTNL_PT_CL_CFG | RSCPC_DESC_MASK),
};

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */


/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Running Speed and Cadence Service
 */
typedef struct rscpc_rscs_content
{
    /// Service info
    prf_svc_t           svc;

    /// Characteristic info:
    ///  - RSC Measurement
    ///  - RSC Feature
    ///  - Sensor Location
    ///  - SC Control Point
    prf_char_t      chars[RSCP_RSCS_CHAR_MAX];

    /// Descriptor handles:
    ///  - RSC Measurement Client Cfg
    ///  - SC Control Point Client Cfg
    prf_desc_t descs[RSCPC_DESC_MAX];
} rscpc_rscs_content_t;

/// Parameters of the @ref RSCPC_ENABLE_REQ message
struct rscpc_enable_req
{
    /// Connection index
    uint8_t              conidx;
    /// Connection type
    uint8_t              con_type;
    /// Existing handle values RSCS
    rscpc_rscs_content_t rscs;
};

/// Parameters of the @ref RSCPC_ENABLE_RSP message
struct rscpc_enable_rsp
{
    /// Connection index
    uint8_t              conidx;
    /// status
    uint16_t             status;
    /// Existing handle values RSCS
    rscpc_rscs_content_t rscs;
};

/// Parameters of the @ref RSCPC_READ_CMD message
struct rscpc_read_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Operation Code
    uint8_t operation;
    /// Read code (see enum #rscpc_codes)
    uint8_t read_code;
};

/// Parameters of the @ref RSCPC_CFG_NTFIND_CMD message
struct rscpc_cfg_ntfind_cmd
{
    /// Connection index
    uint8_t  conidx;
    /// Operation Code
    uint8_t  operation;
    /// Descriptor code (see enum #rscpc_codes)
    uint8_t  desc_code;
    /// Ntf/Ind Configuration
    uint16_t ntfind_cfg;
};

/// Parameters of the @ref RSCPC_CTNL_PT_CFG_CMD message
struct rscpc_ctnl_pt_cfg_cmd
{
    /// Connection index
    uint8_t                       conidx;
    /// Operation Code
    uint8_t                       operation;
    /// Operation Code (see enum #rscp_sc_ctnl_pt_op_code)
    uint8_t                       op_code;
    /// SC Control Point Request value
    union rscp_sc_ctnl_pt_req_val value;
};

/// Parameters of the @ref RSCPC_CTNL_PT_IND message
struct rscpc_ctnl_pt_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Control Point Op code  (see enum #rscp_sc_ctnl_pt_op_code) - 16
    uint8_t  cp_opcode;
    /// Requested Operation Code  (see enum #rscp_sc_ctnl_pt_op_code)
    uint8_t  req_op_code;
    /// Response Value  (see enum #rscp_sc_ctnl_pt_resp_val)
    uint8_t  resp_value;
    /// List of supported locations
    uint16_t supp_sensor_loc;
};

/// Parameters of the @ref RSCPC_VALUE_IND message
struct rscpc_value_ind
{
    /// Connection index
    uint8_t             conidx;
    /// Attribute Code (see enum #rscpc_codes)
    uint8_t             att_code;
    /// Value
    union rscpc_value_tag
    {
        /// RSC Measurement
        rscp_rsc_meas_t rsc_meas;
        /// RSC Feature
        uint16_t        sensor_feat;
        /// Sensor Location
        uint8_t         sensor_loc;
        /// Client Characteristic Configuration Descriptor Value
        uint16_t        ntf_cfg;
    } value;
};

/// Parameters of the @ref RSCPC_CMP_EVT message
struct rscpc_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation code
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} RSCPC

#endif //(_RSCPC_MSG_H_)
