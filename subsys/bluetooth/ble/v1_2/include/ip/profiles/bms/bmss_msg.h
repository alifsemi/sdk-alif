/**
 ****************************************************************************************
 *
 * @file bmss_msg.h
 *
 * @brief Header file - Bond Management Service Server - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef BMSS_MSG_H_
#define BMSS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BMSS_API_MSG Message API
 * @ingroup BMSS_API
 * @brief Description of Message API for Bond Management Service Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bmss.h"

/// @addtogroup BMSS_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/*@TRACE*/
/// Messages IDs for Bond Management Service Server
enum bmss_msg_id
{
    /// Restore bond data previously provided with #BMSS_BOND_DATA_IND message\n
    /// See #bmss_restore_bond_data_req_t
    BMSS_RESTORE_BOND_DATA_REQ = MSG_ID(BMSS, 0x00),
    /// Response message for #BMSS_RESTORE_BOND_DATA_REQ message\n
    /// See #bmss_rsp_t
    BMSS_RSP = MSG_ID(BMSS, 0x01),
    /// Provide bond data to upper layer\n
    /// See #bmss_bond_data_ind_t
    BMSS_BOND_DATA_IND = MSG_ID(BMSS, 0x02),
    /// Inform upper layer that list of supported Bond Management Feature is required\n
    /// See #bmss_features_req_ind_t
    BMSS_FEATURES_REQ_IND = MSG_ID(BMSS, 0x03),
    /// Inform upper layer about a received delete request\n
    /// See #bmss_delete_req_ind_t
    BMSS_DELETE_REQ_IND = MSG_ID(BMSS, 0x04),
    /// Provide list of supported Bond Management Feature requested with #BMSS_FEATURES_REQ_IND message\n
    /// See #bmss_features_cfm_t
    BMSS_FEATURES_CFM = MSG_ID(BMSS, 0x05),
    /// Confirm execution of delete operation requested with #BMSS_DELETE_REQ_IND message\n
    /// See #bmss_delete_cfm_t
    BMSS_DELETE_CFM = MSG_ID(BMSS, 0x06),
};

/// Request type values
enum bmss_req_type
{
    /// Restore bond data
    BMSS_REQ_RESTORE_BOND_DATA = 0,
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #BMSS_RESTORE_BOND_DATA_REQ message
typedef struct bmss_restore_bond_data_req
{
    /// Connection index
    uint8_t conidx;
    /// Bond Data structure
    bmss_bond_data_t bond_data;
    /// Indicate if list of supported Bond Management Feature has been updated since disconnection
    bool features_updated;
    /// Features bit field
    uint8_t features_bf[BMS_FEATURES_LENGTH_MAX];
} bmss_restore_bond_data_req_t;

/// Parameters of the #BMSS_RSP message
typedef struct bmss_rsp
{
    /// Connection index
    uint8_t conidx;
    /// Status
    uint16_t status;
    /// Request type (see #bmss_req_type enumeration)
    uint8_t req_type;
} bmss_rsp_t;

/// Parameters of the #BMSS_BOND_DATA_IND message
typedef struct bmss_bond_data_ind
{
    /// Connection index
    uint8_t conidx;
    /// Bond Data structure
    bmss_bond_data_t bond_data;
} bmss_bond_data_ind_t;

/// Parameters of the #BMSS_FEATURES_REQ_IND message
typedef struct bmss_features_req_ind
{
    /// Connection index
    uint8_t conidx;
    /// Token that shall be provided back in the confirmation
    uint16_t token;
} bmss_features_req_ind_t;

/// Parameters of the #BMSS_DELETE_REQ_IND message
typedef struct bmss_delete_req_ind
{
    /// Connection index
    uint8_t conidx;
    /// Token that shall be provided back in the confirmation
    uint16_t token;
    /// Operation code (see #bms_opcode enumeration)
    uint8_t opcode;
    /// Length of Authorization Code
    uint16_t auth_code_length;
    /// Authorization Code
    uint8_t auth_code[__ARRAY_EMPTY];
} bmss_delete_req_ind_t;

/// Parameters of the #BMSS_FEATURES_CFM message
typedef struct bmss_features_cfm
{
    /// Connection index
    uint8_t conidx;
    /// Token provided in the request indication
    uint16_t token;
    /// Features bit field
    uint8_t features_bf[BMS_FEATURES_LENGTH_MAX];
} bmss_features_cfm_t;

/// Parameters of the #BMSS_DELETE_CFM message
typedef struct bmss_delete_cfm
{
    /// Connection index
    uint8_t conidx;
    /// Status (see #bms_error enumeration)
    uint8_t status;
    /// Token provided in the request indication
    uint16_t token;
} bmss_delete_cfm_t;

/// @} BMSS_API_MSG

#endif // BMSS_MSG_H_
