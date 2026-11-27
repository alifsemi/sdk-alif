/**
 ****************************************************************************************
 *
 * @file bmsc_msg.h
 *
 * @brief Header file - Bond Management Service Client - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BMSC_MSG_H_
#define BMSC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BMSC_API_MSG Message API
 * @ingroup BMSC_API
 * @brief Description of Message API for Bond Management Service Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bmsc.h"

/// @addtogroup BMSC_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Bond Management Service Client
enum bmsc_msg_id
{
    /// Discover Bond Management Service\n
    /// See #bmsc_discover_cmd_t
    BMSC_DISCOVER_CMD = MSG_ID(BMSC, 0x00),
    /// Require execution of a Bond Management delete procedure\n
    /// See #bmsc_delete_cmd_t
    BMSC_DELETE_CMD = MSG_ID(BMSC, 0x01),
    /// Command completed event for #BMSC_DISCOVER_CMD and #BMSC_DELETE_CMD commands\n
    /// See #bmsc_cmp_evt_t
    BMSC_CMP_EVT = MSG_ID(BMSC, 0x02),
    /// Restore bond data\n
    /// See #bmsc_restore_bond_data_req_t
    BMSC_RESTORE_BOND_DATA_REQ = MSG_ID(BMSC, 0x03),
    /// Response for #BMSC_RESTORE_BOND_DATA_REQ request\n
    /// See #bmsc_rsp_t
    BMSC_RSP = MSG_ID(BMSC, 0x04),
    /// Provide bond data to upper layer\n
    /// See #bmsc_bond_data_ind_t
    BMSC_BOND_DATA_IND = MSG_ID(BMSC, 0x05),
    /// Inform upper layer about Bond Management Feature supported by the server\n
    /// See #bmsc_features_ind_t
    BMSC_FEATURES_IND = MSG_ID(BMSC, 0x06),
};

/// Request type values
enum bmsc_req_type
{
    /// Restore bond data
    BMSC_REQ_RESTORE_BOND_DATA = 0,
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #BMSC_DISCOVER_CMD message
typedef struct bmsc_discover_cmd
{
    /// Connection Index
    uint8_t conidx;
} bmsc_discover_cmd_t;

/// Parameters of the #BMSC_DELETE_CMD message
typedef struct bmsc_delete_cmd
{
    /// Connection Index
    uint8_t conidx;
    /// Operation code (see #bms_opcode enumeration)
    uint8_t opcode;
    /// Length of Authorization Code (from 0 to #BMS_AUTH_CODE_LENGTH_MAX)
    uint16_t auth_code_length;
    /// Authorization code
    uint8_t auth_code[__ARRAY_EMPTY];
} bmsc_delete_cmd_t;

/// Parameters of the #BMSC_CMP_EVT message
typedef struct bmsc_cmp_evt
{
    /// Connection Index
    uint8_t conidx;
    /// Status
    uint16_t status;
    /// Command type (see #bmsc_cmd_type enumeration)
    uint8_t cmd_type;
} bmsc_cmp_evt_t;

/// Parameters of the #BMSC_RESTORE_BOND_DATA_REQ message
typedef struct bmsc_restore_bond_data_req
{
    /// Connection Index
    uint8_t conidx;
    /// Description of Bond Management Service discovered in server's attribute database
    bmsc_bms_content_t bms;
} bmsc_restore_bond_data_req_t;

/// Parameters of the #BMSC_RSP message
typedef struct bmsc_rsp
{
    /// Connection Index
    uint8_t conidx;
    /// Status
    uint16_t status;
    /// Request type (see #bmsc_req_type enumeration)
    uint8_t req_type;
} bmsc_rsp_t;

/// Parameters of the #BMSC_BOND_DATA_IND message
typedef struct bmsc_bond_data_ind
{
    /// Connection Index
    uint8_t conidx;
    /// Description of Bond Management Service discovered in server's attribute database
    bmsc_bms_content_t bms;
} bmsc_bond_data_ind_t;

/// Parameters of the #BMSC_FEATURES_IND message
typedef struct bmsc_features_ind
{
    /// Connection Index
    uint8_t conidx;
    /// Number of bytes composing the received Features bit field value
    uint8_t length;
    /// Features bit field value (see #bms_features_bf enumeration)
    uint8_t features_bf[__ARRAY_EMPTY];
} bmsc_features_ind_t;

/// @} BMSC_API_MSG

#endif // BMSC_MSG_H_
