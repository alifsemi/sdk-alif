/**
 ****************************************************************************************
 *
 * @file csisc_msg.h
 *
 * @brief Coordinated Set Identification Service Set Member - Message API Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CSISC_MSG_H_
#define CSISC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CSISC_MSG_API Message API
 * @ingroup CSISC
 * @brief Description of Message API for Coordinated Set Identification Profile Set Coordinator module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "csisc.h"           // Coordinated Set Identification Service Set Member Definitions

#if (BLE_CSIS_COORD)

/// @addtogroup CSISC_MSG_API
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Coordinated Set Identification Profile Set Coordinator
enum csisc_msg_id
{
    CSISC_CMD = MSG_ID(CSISC, 0x00),
    CSISC_CMP_EVT = MSG_ID(CSISC, 0x01),
    CSISC_REQ = MSG_ID(CSISC, 0x02),
    CSISC_RSP = MSG_ID(CSISC, 0x03),
    CSISC_IND = MSG_ID(CSISC, 0x04),
    CSISC_REQ_IND = MSG_ID(CSISC, 0x05),
    CSISC_CFM = MSG_ID(CSISC, 0x06),
};

/// List of CSISC_REQ request codes
enum csisc_msg_req_codes
{
    CSISC_RESTORE_BOND_DATA = 0x0000,
    CSISC_ADD_SIRK = 0x0001,
    CSISC_REMOVE_SIRK = 0x0002,
};

/// List of CSISC_IND indication codes
enum csisc_msg_ind_codes
{
    CSISC_BOND_DATA = 0x0000,
    CSISC_SIRK = 0x0001,
    CSISC_INFO = 0x0002,
    CSISC_UNKNOWN_MSG = 0x0004,
    CSISC_SVC_CHANGED = 0x0005,
};

/// List of CSISC_REQ_IND request indication codes
enum csisc_msg_req_ind_codes
{
    CSISC_LTK = 0x0000,
};

/*
 * KERNEL MESSAGES
 ****************************************************************************************
 */

/// Basic structure for CSISC_CMD message
typedef struct csisc_cmd
{
    /// Command code
    uint16_t cmd_code;
} csisc_cmd_t;

/// Basic structure for CSISC_REQ message
typedef struct csisc_req
{
    /// Request code
    uint16_t req_code;
} csisc_req_t;

/// Basic structure for CSISC_IND message
typedef struct csisc_ind
{
    /// Indication code
    uint16_t ind_code;
} csisc_ind_t;

/// Basic structure for CSISC_REQ_IND message
typedef struct csisc_req_ind
{
    /// Request indication code
    uint16_t req_ind_code;
} csisc_req_ind_t;

/// Basic structure for CSISC_CFM message
typedef struct csisc_cfm
{
    /// Request indication code
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
} csisc_cfm_t;

/// Structure for CSISC_RESOLVE command message
typedef struct csisc_resolve_cmd
{
    /// Command code
    uint16_t cmd_code;
    /// Configuration bit field
    csis_rsi_t rsi;
} csisc_resolve_cmd_t;

/// Structure for CSISC_DISCOVER command message
typedef struct csisc_discover_cmd
{
    /// Command code
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum number of Coordinated Set Identification Service instances that
    /// can be found
    uint8_t nb_sets_max;
    /// Start handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t shdl;
    /// End handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t ehdl;
} csisc_discover_cmd_t;

/// Structure for CSISC_LOCK command message
typedef struct csisc_lock_cmd
{
    /// Command code
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Lock state
    uint8_t lock;
} csisc_lock_cmd_t;

/// Structure for CSISC_GET command message
typedef struct csisc_get_cmd
{
    /// Command code
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Characteristic type
    uint8_t char_type;
} csisc_get_cmd_t;

/// Structure for CSISC_SET_CFG command message
typedef struct csisc_set_cfg_cmd
{
    /// Command code
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Indicate if sending of notifications must be enabled or not for
    /// the indicated characteristic
    uint8_t enable;
} csisc_set_cfg_cmd_t;

/// Structure command complete event
typedef struct csisc_cmp_evt
{
    /// Command code
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Local index
    union
    {
        /// Local index
        uint8_t lid;
        /// Connection local index
        uint8_t con_lid;
        /// Key local index
        uint8_t key_lid;
    } lid;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Type
    union
    {
        /// Type
        uint8_t type;
        /// Characteristic type
        uint8_t char_type;
    } type;
} csisc_cmp_evt_t;

/// Structure for CSISC_RESTORE_BOND_DATA request message
typedef struct csisc_restore_bond_data_req
{
    /// Request code
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Number of Coordinated Set Identification Service instances discovered in peer
    /// device database
    uint8_t nb_sets;
    /// Content description of each Coordinated Set Identification Service instance
    csisc_csis_info_t csis_info[__ARRAY_EMPTY];
} csisc_restore_bond_data_req_t;

/// Structure for CSISC_ADD_SIRK request message
typedef struct csisc_add_sirk_req
{
    /// Request code
    uint16_t req_code;
    /// SIRK value
    csis_sirk_t sirk;
} csisc_add_sirk_req_t;

/// Structure for CSISC_REMOVE_SIRK request message
typedef struct csisc_remove_sirk_req
{
    /// Request code
    uint16_t req_code;
    /// Key local index
    uint8_t key_lid;
} csisc_remove_sirk_req_t;

/// Structure for response message
typedef struct csisc_rsp
{
    /// Request code
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Local index
    union
    {
        /// Local index
        uint8_t lid;
        /// Key local index
        uint8_t key_lid;
        /// Connection local index
        uint8_t con_lid;
    } lid;
} csisc_rsp_t;

/// Structure for CSISC_BOND_DATA indication message
typedef struct csisc_bond_data_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Set local index
    uint8_t set_lid;
    /// Content description of Coordinated Set Identification Service instance
    csisc_csis_info_t csis_info;
} csisc_bond_data_ind_t;

/// Structure for CSISC_SIRK indication message
typedef struct csisc_sirk_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Key local index
    uint8_t key_lid;
    /// SIRK value
    csis_sirk_t sirk;
} csisc_sirk_ind_t;

/// Structure for CSISC_INFO indication message
typedef struct csisc_info_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Value
    union
    {
        /// Value
        uint8_t val;
        /// Number of devices in the Coordinated Set the Service device belongs to
        uint8_t size;
        /// Current lock state for the device
        uint8_t lock;
        /// Rank
        uint8_t rank;
    } val;
} csisc_info_ind_t;

/// Structure for CSISC_SVC_CHANGED indication message
typedef struct csisc_svc_changed_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} csisc_svc_changed_ind_t;

/// Structure for CSISC_UNKNOWN_MSG indication message
typedef struct csisc_unknown_msg_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Message ID
    ke_msg_id_t msg_id;
} csisc_unknown_msg_ind_t;

/// Structure for CSISC_LTK request indication message
typedef struct csisc_ltk_req_ind
{
    /// Request indication code
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Coordinated Set local index
    uint8_t set_lid;
} csisc_ltk_req_ind_t;

/// Structure for CSISC_LTK confirmation message
typedef struct csisc_ltk_cfm
{
    /// Request indication code
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// LTK
    uint8_t ltk[GAP_KEY_LEN];
} csisc_ltk_cfm_t;

/// @} CSISC_MSG_API

#endif //(BLE_CSIS_COORD)

#endif // CSISC_MSG_H_
