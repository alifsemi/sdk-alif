/**
 ****************************************************************************************
 *
 * @file csism_msg.h
 *
 * @brief Coordinated Set Identification Service Set Member - Message API Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CSISM_MSG_H_
#define CSISM_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CSISM_MSG_API Message API
 * @ingroup CSISM
 * @brief Description of Message API for Coordinated Set Identification Profile Set Member module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "csism.h"           // Coordinated Set Identification Service Set Member Definitions

#if (BLE_CSIS_MEMBER)

/// @addtogroup CSISM_MSG_API
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Coordinated Set Identification Profile Set Member
enum csism_msg_id
{
    CSISM_CMD = MSG_ID(CSISM, 0x00),
    CSISM_CMP_EVT = MSG_ID(CSISM, 0x01),
    CSISM_REQ = MSG_ID(CSISM, 0x02),
    CSISM_RSP = MSG_ID(CSISM, 0x03),
    CSISM_IND = MSG_ID(CSISM, 0x04),
    CSISM_REQ_IND = MSG_ID(CSISM, 0x05),
    CSISM_CFM = MSG_ID(CSISM, 0x06),
};

/// List of CSISM_REQ request codes
enum csism_msg_req_codes
{
    CSISM_RESTORE_BOND_DATA = 0x0000,
    CSISM_SET_SIZE = 0x0001,
};

/// List of CSISM_IND indication codes
enum csism_msg_ind_codes
{
    CSISM_LOCK = 0x0000,
    CSISM_BOND_DATA = 0x0001,
    CSISM_RSI = 0x0002,
    CSISM_UNKNOWN_MSG = 0x0003,
};

/// List of CSISM_REQ_IND request indication codes
enum csism_msg_req_ind_codes
{
    CSISM_LTK = 0x0000,
};

/*
 * KERNEL MESSAGES
 ****************************************************************************************
 */

/// Basic structure for CSISM_CMD message
typedef struct csism_cmd
{
    /// Command code
    uint16_t cmd_code;
} csism_cmd_t;

/// Basic structure for CSISM_REQ message
typedef struct csism_req
{
    /// Request code
    uint16_t req_code;
} csism_req_t;

/// Basic structure for CSISM_IND message
typedef struct csism_ind
{
    /// Indication code
    uint16_t ind_code;
} csism_ind_t;

/// Basic structure for CSISM_REQ_IND message
typedef struct csism_req_ind
{
    /// Request indication code
    uint16_t req_ind_code;
} csism_req_ind_t;

/// Basic structure for CSISM_CFM message
typedef struct csism_cfm
{
    /// Request indication code
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
} csism_cfm_t;

/// Structure for CSISM_ADD command message
typedef struct csism_add_cmd
{
    /// Command code
    uint16_t cmd_code;
    /// Configuration bit field
    uint8_t cfg_bf;
    /// Number of members in the added Coordinated Set
    uint8_t size;
    /// Rank
    uint8_t rank;
    /// Lock timeout duration in seconds
    uint8_t lock_timeout_s;
    /// Required start handle
    /// If set to GATT_INVALID_HDL, the start handle will be automatically chosen
    uint16_t shdl;
    /// SIRK value
    csis_sirk_t sirk;
} csism_add_cmd_t;

/// Structure for CSISM_SET_SIRK command message
typedef struct csism_set_sirk_cmd
{
    /// Command code
    uint16_t cmd_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// SIRK value
    csis_sirk_t sirk;
} csism_set_sirk_cmd_t;

/// Structure for CSISM_UPDATE_RSI command message
typedef struct csism_update_rsi_cmd
{
    /// Command code
    uint16_t cmd_code;
    /// Coordinated Set local index
    uint8_t set_lid;
} csism_update_rsi_cmd_t;

/// Structure command complete event
typedef struct csism_cmp_evt
{
    /// Command code
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Coordinated Set local index
    uint8_t set_lid;
} csism_cmp_evt_t;

/// Structure for CSISM_RESTORE_BOND_DATA request message
typedef struct csism_restore_bond_data_req
{
    /// Request code
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Indicate if the peer device is the device for which lock has been granted
    uint8_t is_locked;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
    /// Event configuration bit field
    uint8_t evt_cfg_bf;
} csism_restore_bond_data_req_t;

/// Structure for CSISM_SET_SIZE request message
typedef struct csism_set_size_req
{
    /// Request code
    uint16_t req_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Number of members in the Coordinated Set
    uint8_t size;
} csism_set_size_req_t;

/// Structure for response message
typedef struct csism_rsp
{
    /// Request code
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Connection local index for CSISM_RESTORE_BOND_DATA request
    uint8_t con_lid;
} csism_rsp_t;

/// Structure for CSISM_LOCK indication message
typedef struct csism_lock_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// New lock state
    uint8_t lock;
    /// Connection local index of connection for which Coordinated Set has been locked
    /// or was locked
    uint8_t con_lid;
    /// Reason why Coordinated Set is not locked anymore
    uint8_t reason;
} csism_lock_ind_t;

/// Structure for CSISM_BOND_DATA indication message
typedef struct csism_bond_data_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
} csism_bond_data_ind_t;

/// Structure for CSISM_UNKNOWN_MSG indication message
typedef struct csism_unknown_msg_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Message ID
    ke_msg_id_t msg_id;
} csism_unknown_msg_ind_t;

/// Structure for CSISM_RSI indication message
typedef struct csism_rsi_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// RSI value
    csis_rsi_t rsi;
} csism_rsi_ind_t;

/// Structure for CSISM_LTK request indication message
typedef struct csism_ltk_req_ind
{
    /// Request indication code
    uint16_t req_ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Connection local index
    uint8_t con_lid;
} csism_ltk_req_ind_t;

/// Structure for CSISM_LTK confirmation message
typedef struct csism_ltk_cfm
{
    /// Request indication code
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// LTK
    uint8_t ltk[GAP_KEY_LEN];
} csism_ltk_cfm_t;

/// @} CSISM_MSG_API

#endif //(BLE_CSIS_MEMBER)

#endif // CSISM_MSG_H_
