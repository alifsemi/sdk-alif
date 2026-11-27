/**
 ****************************************************************************************
 *
 * @file glpc_msg.h
 *
 * @brief Header file - Glucose Profile Collector Role - Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _GLPC_MSG_H_
#define _GLPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup GLPC Glucose Profile Collector
 * @ingroup Profile
 * @brief Glucose Profile Collector - Message API
 *
 * The GLPC is responsible for providing Glucose Profile Collector functionalities
 * to upper layer module or application. The device using this profile takes the role
 * of Glucose Profile Collector.
 *
 * Glucose Profile Collector. (GLPC): A GLPC (e.g. PC, phone, etc)
 * is the term used by this profile to describe a device that can interpret Glucose
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
#include "glp_common.h"
#include "prf_types.h"

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*@TRACE*/
enum glpc_msg_id
{
    /// Start the Glucose Profile - at connection
    GLPC_ENABLE_REQ         = MSG_ID(GLPC, 0x00),
    /// Confirm that cfg connection has finished with discovery results, or that normal cnx started
    GLPC_ENABLE_RSP         = MSG_ID(GLPC, 0x01),

    /// Register to Glucose sensor notifications - request
    GLPC_REGISTER_CMD       = MSG_ID(GLPC, 0x02),

    /// Read Glucose sensor characteristic
    GLPC_READ_CMD           = MSG_ID(GLPC, 0x03),
    /// Read Glucose sensor characteristic indication
    GLPC_READ_IND           = MSG_ID(GLPC, 0x04),

    /// Record Access Control Point Command
    GLPC_RACP_CMD           = MSG_ID(GLPC, 0x05),
    /// Record Access Control Point - Resp
    GLPC_RACP_IND           = MSG_ID(GLPC, 0x06),

    /// Glucose measurement value received
    GLPC_MEAS_IND           = MSG_ID(GLPC, 0x07),

    /// Glucose measurement context received
    GLPC_MEAS_CTX_IND       = MSG_ID(GLPC, 0x08),

    /// Complete Event Information
    GLPC_CMP_EVT            = MSG_ID(GLPC, 0x09),
};

/// Characteristics
enum glpc_char_code
{
    /// Glucose Measurement
    GLPC_CHAR_MEAS,
    /// Glucose Measurement Context
    GLPC_CHAR_MEAS_CTX,
    /// Glucose Sensor Feature
    GLPC_CHAR_FEATURE,
    /// Record Access control point
    GLPC_CHAR_RACP,

    GLPC_CHAR_MAX,
};


/// Characteristic descriptors
enum glpc_desc_code
{
    /// Glucose Measurement client config
    GLPC_DESC_MEAS_CLI_CFG,
    /// Glucose Measurement context client config
    GLPC_DESC_MEAS_CTX_CLI_CFG,
    /// Record Access control point client config
    GLPC_DESC_RACP_CLI_CFG,

    GLPC_DESC_MAX,
};

/// Operation codes
enum glpc_op_codes
{
    /// Database Creation Procedure
    GLPC_RESERVED_OP_CODE       = 0,
    /// Configure CCC Operation Code
    GLPC_REGISTER_CMD_OP_CODE   = 1,
    /// Read Operation Code
    GLPC_READ_CMD_OP_CODE       = 2,
    /// RACP Operation Code
    GLPC_RACP_CMD_OP_CODE       = 3,
};

/// Internal codes for reading/writing a GLS characteristic with one single request
enum glpc_read_code
{
    /// Read GLP Feature
    GLPC_RD_FEAT                    = 0,
    /// Read Glucose Measurement Client Char. Configuration Descriptor
    GLPC_RD_DESC_MEAS_CLI_CFG       = 1,
    /// Read Glucose Measurement context Client Char. Configuration Descriptor
    GLPC_RD_DESC_MEAS_CTX_CLI_CFG   = 2,
    /// Read Record Access control point Client Char. Configuration Descriptor
    GLPC_RD_DESC_RACP_CLI           = 3,
};

///Structure containing the characteristics handles, value handles and descriptors
typedef struct gls_content
{
    /// service info
    prf_svc_t           svc;

    /// characteristic info:
    ///  - Glucose Measurement
    ///  - Glucose Measurement Context
    ///  - Glucose Feature
    ///  - Record Access Control Point
    prf_char_t      chars[GLPC_CHAR_MAX];

    /// Descriptor handles:info:
    ///  - Glucose Measurement client cfg
    ///  - Glucose Measurement Context client cfg
    ///  - Record Access Control Point client cfg
    prf_desc_t descs[GLPC_DESC_MAX];
} gls_content_t;

/// Parameters of the @ref GLPC_ENABLE_REQ message
struct glpc_enable_req
{
    /// Connection index
    uint8_t        conidx;
    /// Connection type
    uint8_t        con_type;
    /// Existing handle values GLS
    gls_content_t  gls;
};

/// Parameters of the @ref GLPC_ENABLE_RSP message
struct glpc_enable_rsp
{
    /// Connection index
    uint8_t       conidx;
    /// status
    uint16_t      status;
    /// Existing handle values GLS
    gls_content_t gls;
};

/// Parameters of the @ref GLPC_REGISTER_CMD message
struct glpc_register_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Register or not Glucose measurement context notifications
    bool    meas_ctx_en;
};

/// Parameters of the @ref GLPC_READ_CMD message
struct glpc_read_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Glucose sensor read characteristic code (see enum #glpc_read_code)
    uint8_t read_code;
};

/// Parameters of the @ref GLPC_READ_IND message
struct glpc_read_ind
{
    /// Connection index
    uint8_t conidx;
    /// Glucose sensor read characteristic code (see enum #glpc_read_code)
    uint8_t read_code;
    /// Read value
    union
    {
        /// Glucose sensor features
        uint16_t features;

        /// Client Characteristic Configuration Descriptor value
        uint16_t ind_cfg;
    } value;
};

/// Parameters of the @ref GLPC_RACP_CMD message
struct glpc_racp_cmd
{
    /// Connection index
    uint8_t          conidx;
    /// Record access control point request
    glp_racp_req_t   req;
};

/// Parameters of the @ref GLPC_RACP_IND message
struct glpc_racp_ind
{
    /// Connection index
    uint8_t        conidx;
    /// operation code  (see enum #glp_racp_op_code) GLP_REQ_NUM_OF_STRD_RECS_RSP or GLP_REQ_RSP_CODE
    uint8_t        op_code;
    /// Request Op Code (see enum #glp_racp_op_code)
    uint8_t        req_op_code;
    /// Response Code status (see enum #glp_racp_status)
    uint8_t        racp_status;
    /// Number of record (if op_code = GLP_REQ_NUM_OF_STRD_RECS_RSP)
    uint16_t       num_of_record;
};

/// Parameters of the @ref GLPC_MEAS_IND message
struct glpc_meas_ind
{
    /// Connection index
    uint8_t     conidx;
    /// Sequence Number
    uint16_t    seq_num;
    /// Glucose measurement
    glp_meas_t  meas_val;
};

/// Parameters of the @ref GLPC_MEAS_CTX_IND message
struct glpc_meas_ctx_ind
{
    /// Connection index
    uint8_t        conidx;
    /// Sequence Number
    uint16_t       seq_num;
    /// Glucose measurement
    glp_meas_ctx_t ctx;
};

/// Parameters of the @ref GLPC_CMP_EVT message
struct glpc_cmp_evt
{
    /// Connection index
    uint8_t  conidx;
    /// Operation
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} GLPC

#endif /* _GLPC_MSG_H_ */
