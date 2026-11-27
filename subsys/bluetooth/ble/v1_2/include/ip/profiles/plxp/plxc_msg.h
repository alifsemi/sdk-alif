/**
 ****************************************************************************************
 *
 * @file plxc_msg.h
 *
 * @brief Header file - Pulse Oximeter Service Collector - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _PLXC_MSG_H_
#define _PLXC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup PLXC_API_MSG Message API
 * @ingroup PLXC_API
 * @brief Description of Message API for Pulse Oximeter Service Collector
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "plxc.h"
#include "rwip_task.h"

/// @addtogroup PLXC_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Message IDs
enum plxc_msg_ids
{
    /// Enable the Profile Collector task - at connection
    PLXC_ENABLE_REQ = MSG_ID(PLXC, 0x00),
    /// Response to Enable the Profile Collector task - at connection
    PLXC_ENABLE_RSP = MSG_ID(PLXC, 0x01),
    /// Read Characteristic
    PLXC_READ_CMD = MSG_ID(PLXC, 0x02),
    /// Configure Characteristic's CCC descriptor
    PLXC_CFG_CCC_CMD = MSG_ID(PLXC, 0x03),
    /// Write Command to the Control Point
    PLXC_WRITE_RACP_CMD = MSG_ID(PLXC, 0x04),
    /// Receive the Spot-check Measurement or Measurement Record Indication SPOT_MEAS
    /// *   Receive Continuous Measurement Notification CONT_MEAS
    /// *   Receive Control Point Response Indication RACP_RESP
    PLXC_VALUE_IND = MSG_ID(PLXC, 0x05),
    /// Read CCC value of specific characteristic
    PLXC_RD_CHAR_CCC_IND = MSG_ID(PLXC, 0x06),
    /// Complete event for the Application commands
    PLXC_CMP_EVT = MSG_ID(PLXC, 0x07),
};

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #PLXC_ENABLE_REQ message
typedef struct plxc_enable_req
{
    /// Connection index
    uint8_t conidx;
    /// Connection type
    uint8_t con_type;
    /// Content of Pulse Oximeter Service
    plxc_plxp_content_t plx;
} plxc_enable_req_t;

/// Parameters of the #PLXC_ENABLE_RSP message
typedef struct plxc_enable_rsp
{
    /// Connection index
    uint8_t conidx;
    /// Status
    uint16_t status;
    /// Content of discovered Pulse Oximeter Service
    plxc_plxp_content_t plx;
} plxc_enable_rsp_t;

/// Parameters of the #PLXC_READ_CMD message
typedef struct plxc_read_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Value Identifier (see #plxc_val_id enumeration)
    uint8_t val_id;
} plxc_read_cmd_t;

/// Parameters of the #PLXC_CFG_CCC_CMD message
typedef struct plxc_cfg_ccc_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Value Identifier (see #plxc_val_id enumeration)
    ///     - #PLXC_VAL_SPOT_CHECK_MEAS_CFG
    ///     - #PLXC_VAL_CONTINUOUS_MEAS_CFG
    ///     - #PLXC_VAL_RACP_CFG
    ///     - #PLXC_VAL_FEATURES_CFG
    uint8_t val_id;
    /// The Client Characteristic Configuration Value
    uint16_t ccc;
} plxc_cfg_ccc_cmd_t;

/// Parameters of the #PLXC_WRITE_RACP_CMD message
typedef struct plxc_write_racp_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Control Point OpCode see enum #plxp_cp_opcodes_id
    uint8_t cp_opcode;
    /// Operator  see enum #plxp_cp_operator_id
    uint8_t cp_operator;
} plxc_write_racp_cmd_t;

/// Parameters of the #PLXC_VALUE_IND message
typedef struct plxc_value_ind
{
    /// Connection index
    uint8_t conidx;
    /// Value Identifier (see #plxc_val_id enumeration)
    uint8_t val_id;
    /// Value
    union
    {
        /// Spot-Check Measurement (val_id = #PLXC_VAL_SPOT_CHECK_MEAS)
        plxp_spot_meas_t spot_meas;
        /// Continuous Measurement (val_id = #PLXC_VAL_CONTINUOUS_MEAS)
        plxp_cont_meas_t cont_meas;
        /// Record Access Control Point response (val_id = #PLXC_VAL_RACP_RSP)
        plxp_racp_rsp_t racp_rsp;
        /// Read Features request command value (val_id = #PLXC_VAL_FEATURES)
        plxp_features_t features;
    } value;
} plxc_value_ind_t;

/// Parameters of the #PLXC_RD_CHAR_CCC_IND message
typedef struct plxc_rd_char_ccc_ind
{
    /// Connection index
    uint8_t conidx;
    /// Value Identifier (see #plxc_val_id enumeration)
    ///     - #PLXC_VAL_SPOT_CHECK_MEAS_CFG
    ///     - #PLXC_VAL_CONTINUOUS_MEAS_CFG
    ///     - #PLXC_VAL_RACP_CFG
    ///     - #PLXC_VAL_FEATURES_CFG
    uint8_t val_id;
    /// Client Characteristic Configuration
    uint16_t ind_cfg;
} plxc_rd_char_ccc_ind_t;

/// Parameters of the #PLXC_CMP_EVT message
typedef struct plxc_cmp_evt
{
    /// Connection index
    uint8_t conidx;
    /// Operation (see #plxc_op_codes enumeration)
    uint8_t operation;
    /// Status
    uint16_t status;
} plxc_cmp_evt_t;

/// @} PLXC_MSG_API

#endif // _PLXC_MSG_H_
