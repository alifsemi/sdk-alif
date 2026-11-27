/**
 ****************************************************************************************
 *
 * @file blpc_msg.h
 *
 * @brief Header file - Blood Pressure Profile Collector Role - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 *
 ****************************************************************************************
 */

#ifndef _BLPC_MSG_H_
#define _BLPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup BLPC
 * @ingroup Profile
 * @brief Blood Pressure Profile Collector  - Message API
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "blp_common.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*@TRACE*/
enum blpc_msg_id
{
    /// Start the blood pressure profile - at connection
    BLPC_ENABLE_REQ       = MSG_ID(BLPC, 0x00),
    /// Confirm that cfg connection has finished with discovery results, or that normal cnx started
    BLPC_ENABLE_RSP       = MSG_ID(BLPC, 0x01),
    /// Generic message to read a BPS or DIS characteristic value
    BLPC_RD_CHAR_CMD      = MSG_ID(BLPC, 0x02),
    ///Generic message for read responses for APP
    BLPC_RD_CHAR_IND      = MSG_ID(BLPC, 0x03),
    /// Generic message for configuring the 2 characteristics that can be handled
    BLPC_CFG_INDNTF_CMD   = MSG_ID(BLPC, 0x04),
    /// Blood pressure value send to APP
    BLPC_BP_MEAS_IND      = MSG_ID(BLPC, 0x05),
    /// Complete Event Information
    BLPC_CMP_EVT          = MSG_ID(BLPC, 0x06),
};

/// Characteristics
enum
{
    /// Blood Pressure Measurement
    BLPC_CHAR_BP_MEAS,
    /// Intermediate Cuff pressure
    BLPC_CHAR_CP_MEAS,
    /// Blood Pressure Feature
    BLPC_CHAR_BP_FEATURE,
    BLPC_CHAR_MAX,
};

/// Characteristic descriptors
enum
{
    /// Blood Pressure Measurement client config
    BLPC_DESC_BP_MEAS_CLI_CFG,
    /// Intermediate Cuff pressure client config
    BLPC_DESC_IC_MEAS_CLI_CFG,
    BLPC_DESC_MAX,
    BLPC_DESC_MASK = 0x10,
};

/// Operation codes
enum blpc_op_codes
{
    /// Database Creation Procedure
    BLPC_RESERVED_OP_CODE       = 0,
    /// Configure CCC Operation Code
    BLPC_CFG_INDNTF_CMD_OP_CODE = 1,
    /// Read char. Operation Code
    BLPC_RD_CHAR_CMD_OP_CODE    = 2,
};

///Structure containing the characteristics handles, value handles and descriptors
typedef struct bps_content
{
    /// service info
    prf_svc_t           svc;
    /// characteristic info:
    ///  - Blood Pressure Measurement
    ///  - Intermediate Cuff pressure
    ///  - Blood Pressure Feature
    prf_char_t      chars[BLPC_CHAR_MAX];
    /// Descriptor handles:
    ///  - Blood Pressure Measurement client cfg
    ///  - Intermediate Cuff pressure client cfg
    prf_desc_t descs[BLPC_DESC_MAX];
} bps_content_t;

/// Parameters of the @ref BLPC_ENABLE_REQ message
struct blpc_enable_req
{
    ///Connection index
    uint8_t       conidx;
    ///Connection type
    uint8_t       con_type;
    ///Existing handle values bps
    bps_content_t bps;
};

/// Parameters of the @ref BLPC_ENABLE_RSP message
struct blpc_enable_rsp
{
    ///Connection index
    uint8_t       conidx;
    ///status
    uint16_t      status;
    ///Existing handle values bps
    bps_content_t bps;
};

/// Parameters of the @ref BLPC_RD_CHAR_CMD message
struct blpc_rd_char_cmd
{
    ///Connection index
    uint8_t  conidx;
    ///Characteristic value code
    uint8_t  char_code;
};

/// Parameters of the @ref BLPC_RD_CHAR_IND message
struct blpc_rd_char_ind
{
    ///Connection index
    uint8_t             conidx;
    /// Characteristic value code
    uint8_t             char_code;
    /// Value length
    uint16_t            length;
    /// Value data
    uint8_t             value[__ARRAY_EMPTY];
};

/// Parameters of the @ref BLPC_CFG_INDNTF_CMD message
struct blpc_cfg_indntf_cmd
{
    ///Connection index
    uint8_t  conidx;
    ///Own code for differentiating between blood pressure and
    ///intermediate cuff pressure measurements
    uint8_t  char_code;
    ///Stop/notify/indicate value to configure into the peer characteristic
    uint16_t cfg_val;
};

/// Parameters of the @ref BLPC_BP_MEAS_IND message
struct blpc_bp_meas_ind
{
    ///Connection index
    uint8_t       conidx;
    /// Flag indicating if it is a intermediary cuff pressure measurement (1) or
    /// stable blood pressure measurement (0) (see enum #bps_meas_type).
    uint16_t      flag_interm_cp;
    ///Blood Pressure measurement
    bps_bp_meas_t meas_val;
};

///Parameters of the @ref BLPC_CMP_EVT message
struct blpc_cmp_evt
{
    ///Connection index
    uint8_t  conidx;
    /// Operation
    uint8_t  operation;
    /// Status
    uint16_t status;
};

/// @} BLPC

#endif /* _BLPC_MSG_H_ */
