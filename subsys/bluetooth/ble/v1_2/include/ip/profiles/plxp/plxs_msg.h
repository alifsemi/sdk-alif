/**
 ****************************************************************************************
 *
 * @file plxs_msg.h
 *
 * @brief Header file - Pulse Oximeter Service Sensor - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _PLXS_MSG_H_
#define _PLXS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup PLXS_API_MSG Message API
 * @ingroup PLXS_API
 * @brief Description of Message API for Pulse Oximeter Service Sensor
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "plxs.h"
#include "rwip_task.h"

/// @addtogroup PLXS_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs
enum plxs_msg_id
{
    /// Enable the PLXP Sensor task for a connection
    PLXS_ENABLE_REQ = MSG_ID(PLXS, 0x00),
    /// Enable the PLXP Sensor task for a connection
    PLXS_ENABLE_RSP = MSG_ID(PLXS, 0x01),
    /// Send the Spot-check Measurement or Measurement Record
    /// Send Continuous Measurement
    PLXS_MEAS_VALUE_CMD = MSG_ID(PLXS, 0x02),
    /// Send Control Point Response
    PLXS_RACP_RESP_SEND_CMD = MSG_ID(PLXS, 0x03),
    /// Inform Application on the Characteristics CCC descriptor changes
    PLXS_CFG_INDNTF_IND = MSG_ID(PLXS, 0x04),
    /// Write to the Control Point forward to Application
    PLXS_RACP_REQ_RECV_IND  = MSG_ID(PLXS, 0x05),
    /// Complete event for a command
    PLXS_CMP_EVT = MSG_ID(PLXS, 0x06),
    /// Indicate that list of supported features has been updated
    PLXS_FEATURES_UPDATED_CMD = MSG_ID(PLXS, 0x07),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #PLXS_ENABLE_REQ message
typedef struct plxs_enable_req
{
    /// Connection index
    uint8_t conidx;
    /// Indication/notification configuration bit field (see #plxs_evt_cfg_bf enumeration)
    uint8_t evt_cfg;
} plxs_enable_req_t;

/// Parameters of the #PLXS_ENABLE_RSP message
typedef struct plxs_enable_rsp
{
    /// Connection index
    uint8_t conidx;
    /// Status
    uint16_t status;
} plxs_enable_rsp_t;

/// Parameters of the #PLXS_MEAS_VALUE_CMD message
typedef struct plxs_meas_value_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Operation (see #plxs_op_codes enumeration)
    uint8_t operation;
    /// Measurement
    union
    {
        /// Spot-Check Measurement (if operation = #PLXS_SPOT_CHECK_MEAS_CMD_OP_CODE)
        plxp_spot_meas_t spot_meas;
        /// Continuous Measurement (if operation = #PLXS_CONTINUOUS_MEAS_CMD_OP_CODE)
        plxp_cont_meas_t cont_meas;
    } value;
} plxs_meas_value_cmd_t;

/// Parameters of the #PLXS_RACP_RESP_SEND_CMD message
typedef struct plxs_racp_rsp_send_cmd
{
    /// Connection index
    uint8_t conidx;
    /// Operation = PLXS_RASP_CMD_OP_CODE see enum #plxs_op_codes
    uint8_t operation;
    /// Request Control Point OpCode see enum #plxp_cp_opcodes_id
    uint8_t req_cp_opcode;
    /// Response Code see enum #plxp_cp_resp_code_id
    uint8_t rsp_code;
    /// Number of Records
    uint16_t rec_num;
} plxs_racp_rsp_send_cmd_t;

/// Parameters of the #PLXS_FEATURES_UPDATED_CMD message
typedef struct plxs_features_updated_cmd
{
    /// Connection index
    uint8_t conidx;
} plxs_features_updated_cmd_t;

/// Parameters of the #PLXS_RACP_REQ_RECV_IND message
typedef struct plxs_racp_req_recv_ind
{
    /// Connection index
    uint8_t conidx;
    /// Control Point OpCode see enum #plxp_cp_opcodes_id
    uint8_t cp_opcode;
    /// Operator  see enum #plxp_cp_operator_id
    uint8_t cp_operator;
} plxs_racp_req_recv_ind_t;

/// Parameters of the #PLXS_CFG_INDNTF_IND message
typedef struct plxs_cfg_indntf_ind
{
    /// Connection index
    uint8_t conidx;
    /// Indication/notification configuration (see #plxs_evt_cfg_bf enumeration)
    uint8_t evt_cfg;
} plxs_cfg_indntf_ind_t;

/// Parameters of the #PLXS_CMP_EVT message
typedef struct plxs_cmp_evt
{
    /// Connection index
    uint8_t conidx;
    /// Operation (see #plxs_op_codes enumeration)
    uint8_t operation;
    /// Status
    uint16_t status;
} plxs_cmp_evt_t;

/// @} PLXS_API_MSG

#endif // _PLXS_MSG_H_
