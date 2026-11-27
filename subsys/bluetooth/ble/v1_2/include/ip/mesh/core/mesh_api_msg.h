/**
 ****************************************************************************************
 *
 * @file mesh_api_msg.h
 *
 * @brief Header file for Mesh Stack Message Application Programming Interface
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef MESH_API_MSG_
#define MESH_API_MSG_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup MESH_API_MSG Message API
 * @ingroup MESH
 * @brief  Mesh Stack Message Application Programming Interface
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_COMMON_MSG Common
 * @ingroup MESH_API_MSG
 * @brief Mesh Common Message Application Programming Interface
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_COMMON_GENERAL_MSG General
 * @ingroup MESH_COMMON_MSG
 * @brief Mesh General Common Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_COMMON_RUNTIME_MSG Run Time
 * @ingroup MESH_COMMON_MSG
 * @brief Mesh Common Run Time Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_COMMON_VERSION_MSG Version
 * @ingroup MESH_COMMON_MSG
 * @brief Mesh Common Version Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_COMMON_BUFFER_MSG Buffer
 * @ingroup MESH_COMMON_MSG
 * @brief Mesh Common Buffer Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_MSG Profile
 * @ingroup MESH_API_MSG
 * @brief Mesh Profile Message Application Programming Interface
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_GENERAL_MSG General
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh General Profile Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_LPN_MSG LPN
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh Profile Low Power Node Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_PROXY_MSG Proxy
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh Profile Proxy Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_KEY_MSG Key
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh Profile Key Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_PROV_MSG Provisioning
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh Profile Provisioning Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_CONFS_MSG Configuration Server
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh Profile Configuration Server Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_CONFC_MSG Configuration Client
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh Profile Configuration Client Model Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_FWDS_MSG Directed Forwarding Configuration Server
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh Profile Directed Forwarding Configuration Server Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_FWDC_MSG Directed Forwarding Configuration Client
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh Profile Directed Forwarding Configuration Client Model Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_HLTHS_MSG Health Server
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh Profile Health Server Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_PROFILE_HLTHC_MSG Health Client
 * @ingroup MESH_PROFILE_MSG
 * @brief Mesh Profile Health Client Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_MODEL_MSG Model
 * @ingroup MESH_API_MSG
 * @brief Mesh Model Message Application Programming Interface
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_MODEL_GENERAL_MSG General
 * @ingroup MESH_MODEL_MSG
 * @brief Mesh General Configuration Model Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_MODEL_SERVER_MSG Server
 * @ingroup MESH_MODEL_MSG
 * @brief Mesh Configuration Model Server Related Structure
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup MESH_MODEL_CLIENT_MSG Client
 * @ingroup MESH_MODEL_MSG
 * @brief Mesh Configuration Model Client Related Structure
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "mesh_defines.h"
#include "mesh_config.h"
#include "mesh_api.h"
#include "gap.h"


/// @addtogroup MESH_API_MSG
/// @{
/// @addtogroup MESH_COMMON_MSG
/// @{
/// @addtogroup MESH_COMMON_GENERAL_MSG
/// @{

/*
 * MACROS
 ****************************************************************************************
 */

/// Retrieve message value from its index
#define MESH_API_MSG_VAL(id)        (TASK_FIRST_MSG(TASK_ID_MESH) + (id))

/*
 * ENUMERATIONS FOR MESH STACK
 ****************************************************************************************
 */

/// Mesh Stack Message Indexes
enum mesh_api_msg_id
{
    /* ******************************** */
    /* *         MESH PROFILE         * */
    /* ******************************** */

    MESH_API_FIRST                           = MESH_API_MSG_VAL(0),
    /// Mesh Command
    MESH_API_CMD                             = MESH_API_FIRST,
    /// Command Complete event
    MESH_API_CMP_EVT                         = MESH_API_MSG_VAL(1),
    /// Mesh Request
    MESH_API_REQ                             = MESH_API_MSG_VAL(2),
    /// Mesh Response
    MESH_API_RSP                             = MESH_API_MSG_VAL(3),
    /// Mesh Indication
    MESH_API_IND                             = MESH_API_MSG_VAL(4),
    /// Mesh Request Indication
    MESH_API_REQ_IND                         = MESH_API_MSG_VAL(5),
    /// Used to answer if a model support or not a specific operation code.
    MESH_API_CFM                             = MESH_API_MSG_VAL(6),

    /// Request model if a specific operation code is supported (shall be confirmed with
    /// MESH_MODEL_OPCODE_CFM message)
    MESH_API_MODEL_OPCODE_REQ_IND            = MESH_API_MSG_VAL(10),
    /// Used to answer if a model support or not a specific operation code.
    MESH_API_MODEL_OPCODE_CFM                = MESH_API_MSG_VAL(11),
    /// Inform reception of a specific mesh message
    MESH_API_MODEL_MSG_IND                   = MESH_API_MSG_VAL(12),
    /// Inform transmission status of message issued by local model
    MESH_API_MODEL_MSG_SENT_IND              = MESH_API_MSG_VAL(13),
    /// Inform about new publication parameters for a model
    MESH_API_MODEL_PUBLISH_PARAM_IND         = MESH_API_MSG_VAL(14),

    /// Inform about updated attention state
    MESH_API_ATTENTION_UPDATE_IND            = MESH_API_MSG_VAL(20),
    /// Inform about requested node reset
    MESH_API_NODE_RESET_IND                  = MESH_API_MSG_VAL(21),
    /// Inform about new or deleted group (application key)
    MESH_API_GROUP_UPDATE_IND                = MESH_API_MSG_VAL(22),

    /// Request a page of the composition data
    MESH_API_COMPO_DATA_REQ_IND              = MESH_API_MSG_VAL(30),
    /// Response containing the required composition data page
    MESH_API_COMPO_DATA_CFM                  = MESH_API_MSG_VAL(31),
    /// Configuration update indication
    MESH_API_UPDATE_IND                      = MESH_API_MSG_VAL(32),

    /// Request to get the current Registered Fault state identified by Company ID for primary element
    MESH_API_FAULT_GET_REQ_IND               = MESH_API_MSG_VAL(40),
    /// Request to start a test procedure of primary element
    MESH_API_FAULT_TEST_REQ_IND              = MESH_API_MSG_VAL(41),
    /// Response containing current Registered Fault state for primary element
    MESH_API_FAULT_CFM                       = MESH_API_MSG_VAL(42),
    /// Inform that clear of Registered Fault state identified by Company ID has been received
    /// for primary element
    MESH_API_FAULT_CLEAR_IND                 = MESH_API_MSG_VAL(43),
    /// Inform about new publication period for Current Health state of primary element
    MESH_API_FAULT_PERIOD_IND                = MESH_API_MSG_VAL(44),

    /// Mesh Provisioning state change indication
    MESH_API_PROV_STATE_IND                  = MESH_API_MSG_VAL(50),
    /// Mesh Provisioning parameters request
    MESH_API_PROV_PARAM_REQ_IND              = MESH_API_MSG_VAL(51),
    /// Mesh Provisioning parameters response
    MESH_API_PROV_PARAM_CFM                  = MESH_API_MSG_VAL(52),
    /// Mesh Provisioning Authentication Data Request
    MESH_API_PROV_AUTH_DATA_REQ_IND          = MESH_API_MSG_VAL(53),
    /// Mesh Provisioning Authentication Data response
    MESH_API_PROV_AUTH_DATA_CFM              = MESH_API_MSG_VAL(54),

    /// Mesh Friend Offer reception indication
    MESH_API_LPN_OFFER_IND                   = MESH_API_MSG_VAL(60),
    /// Mesh Friendship status indication
    MESH_API_LPN_STATUS_IND                  = MESH_API_MSG_VAL(61),

    /// Mesh Proxy advertising state update indication
    MESH_API_PROXY_ADV_UPDATE_IND            = MESH_API_MSG_VAL(70),
    /// Mesh Proxy filter status indication
    MESH_API_PROXY_FILT_STATUS_IND           = MESH_API_MSG_VAL(71),
    MESH_API_LAST                            = MESH_API_PROXY_FILT_STATUS_IND,

    /* ******************************** */
    /* *       MESH PROFILE DEBUG     * */
    /* ******************************** */

    MESH_DBG_FIRST                           = MESH_API_MSG_VAL(90),
    /// Mesh Profile Debug Command
    MESH_DBG_CMD                             = MESH_DBG_FIRST,
    /// Mesh Profile Debug Command Complete Event
    MESH_DBG_CMP_EVT                         = MESH_API_MSG_VAL(91),
    /// Mesh Debug Data Received Indication
    MESH_DBG_DATA_RX_IND                     = MESH_API_MSG_VAL(92),
    /// Mesh Debug Data Send Indication
    MESH_DBG_DATA_SEND_IND                   = MESH_API_MSG_VAL(93),
    /// Mesh Debug Data Sent Indication
    MESH_DBG_DATA_SENT_IND                   = MESH_API_MSG_VAL(94),
    /// Mesh Debug Bearer Opened Indication
    MESH_DBG_BEARER_OPENED_IND               = MESH_API_MSG_VAL(95),
    /// Mesh Debug Bearer Close Indication
    MESH_DBG_BEARER_CLOSED_IND               = MESH_API_MSG_VAL(96),
    /// Mesh Debug Friendship Indication
    MESH_DBG_FRIEND_LPN_IND                  = MESH_API_MSG_VAL(97),
    MESH_DBG_LAST                            = MESH_DBG_FRIEND_LPN_IND,

    /* ******************************** */
    /* *         MESH STACK           * */
    /* ******************************** */

    MESH_COMMON_API_FIRST                    = MESH_API_MSG_VAL(110),
    /// Mesh Stack Command
    MESH_COMMON_API_CMD                      = MESH_COMMON_API_FIRST,
    /// Mesh Stack Command Complete Event
    MESH_COMMON_API_CMP_EVT                  = MESH_API_MSG_VAL(111),
    /// Mesh Stack Indication
    MESH_COMMON_API_IND                      = MESH_API_MSG_VAL(112),
    /// Mesh Stack Request Indication
    MESH_COMMON_API_REQ_IND                  = MESH_API_MSG_VAL(113),
    /// Mesh Stack Confirmation
    MESH_COMMON_API_CFM                      = MESH_API_MSG_VAL(114),
    MESH_COMMON_API_LAST                     = MESH_COMMON_API_CFM,

    /* ******************************** */
    /* *       MESH STACK DEBUG       * */
    /* ******************************** */

    MESH_COMMON_DBG_FIRST                    = MESH_API_MSG_VAL(120),
    /// Mesh Stack Debug Command
    MESH_COMMON_DBG_CMD                      = MESH_COMMON_DBG_FIRST,
    /// Mesh Stack Debug Command Complete Event
    MESH_COMMON_DBG_CMP_EVT                  = MESH_API_MSG_VAL(121),
    /// Mesh Stack Debug Indication
    MESH_COMMON_DBG_IND                      = MESH_API_MSG_VAL(122),
    MESH_COMMON_DBG_LAST                     = MESH_COMMON_DBG_IND,

    /* ******************************** */
    /* *           MESH MODEL         * */
    /* ******************************** */

    MESH_MDL_API_FIRST                       = MESH_API_MSG_VAL(130),
    /// Mesh Model Command
    MESH_MDL_API_CMD                         = MESH_MDL_API_FIRST,
    /// Mesh Model Command Complete Event
    MESH_MDL_API_CMP_EVT                     = MESH_API_MSG_VAL(131),
    /// Mesh Model Indication
    MESH_MDL_API_IND                         = MESH_API_MSG_VAL(132),
    /// Mesh Model Request Indication
    MESH_MDL_API_REQ_IND                     = MESH_API_MSG_VAL(133),
    /// Mesh Model Confirm
    MESH_MDL_API_CFM                         = MESH_API_MSG_VAL(134),
    MESH_MDL_API_LAST                        = MESH_MDL_API_CFM,

    /* ******************************** */
    /* *       MESH MODEL DEBUG       * */
    /* ******************************** */

    MESH_MDL_DBG_FIRST                       = MESH_API_MSG_VAL(140),
    /// Mesh Stack Debug Command
    MESH_MDL_DBG_CMD                         = MESH_MDL_DBG_FIRST,
    /// Mesh Stack Debug Command Complete Event
    MESH_MDL_DBG_CMP_EVT                     = MESH_API_MSG_VAL(141),
    /// Mesh Stack Debug Indication
    MESH_MDL_DBG_IND                         = MESH_API_MSG_VAL(142),
    MESH_MDL_DBG_LAST                        = MESH_MDL_DBG_IND,

    /* ******************************** */
    /* *            INTERNAL          * */
    /* ******************************** */

    /// Message ID for first internal message, all following IDs are reserved
    MESH_INT_MSG                             = MESH_API_MSG_VAL(200),
};

/// Command codes for MESH_COMMON_API_CMD and MESH_COMMON_API_CMP_EVT messages
enum mesh_api_cmd_code
{
    /// Get elapsed time since first device up time
    MESH_API_GET_RUN_TIME               = 0,
    /// Set elapsed time since first device up time
    MESH_API_SET_RUN_TIME               = 1,
    /// Get Mesh Stack version
    MESH_API_GET_VERSION                = 2,

    /// Allocate a new block of buffers
    MESH_API_BUF_ALLOC_BLOCK            = 10,
    /// Free a block of buffers
    MESH_API_BUF_FREE_BLOCK             = 11,
};

/*
 * MESSAGE DEFINITIONS FOR MESH STACK
 ****************************************************************************************
 */

/// Command required structure (without parameters)
typedef struct mesh_api_cmd
{
    /// Command code (see #mesh_api_cmd_code enumeration)
    uint32_t cmd_code;
} mesh_api_cmd_t;

/// Command complete event required structure (without parameters)
typedef struct mesh_api_cmp_evt
{
    /// Command code (see #mesh_api_cmd_code enumeration)
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
} mesh_api_cmp_evt_t;

/// @} MESH_COMMON_GENERAL_MSG

/// @addtogroup MESH_COMMON_RUNTIME_MSG
/// @{

/// Set run time command structure
typedef struct mesh_api_set_run_time_cmd
{
    /// Command code (see #mesh_api_cmd_code enumeration)
    /// - MESH_API_SET_RUN_TIME: Set run time
    uint32_t cmd_code;
    /// Current clock value in milliseconds
    uint32_t clock_ms;
    /// Number of time the clock has wrapped since up time
    uint16_t nb_wrap;
} mesh_api_set_run_time_cmd_t;

/// Get run time command complete event structure
typedef struct mesh_api_get_run_time_cmp_evt
{
    /// Command code (see #mesh_api_cmd_code enumeration)
    /// - MESH_API_GET_RUN_TIME: Get run time
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;

    /// Number of time the clock has wrapped since up time
    uint16_t nb_wrap;
    /// Current clock value in milliseconds
    uint32_t clock_ms;
} mesh_api_get_run_time_cmp_evt_t;

/// @} MESH_COMMON_RUNTIME_MSG

/// @addtogroup MESH_COMMON_VERSION_MSG
/// @{

/// Get version command complete event structure
typedef struct mesh_api_get_version_cmp_evt
{
    /// Command code (see #mesh_api_cmd_code enumeration)
    /// - MESH_API_GET_VERSION: Get version
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;

    /// Mesh Versions
    mesh_version_t version;
} mesh_api_get_version_cmp_evt_t;

/// @} MESH_COMMON_VERSION_MSG

/// @addtogroup MESH_COMMON_BUFFER_MSG
/// @{

/// Allocate block of buffers command structure
typedef struct mesh_api_buf_alloc_block_cmd
{
    /// Command code (see #mesh_api_cmd_code enumeration)
    /// - MESH_API_BUF_ALLOC_BLOCK: Allocate a new block of buffers
    uint32_t cmd_code;
    /// Number of buffers
    uint8_t nb_bufs;
    /// Small (1) or long buffers (0)
    uint8_t small;
} mesh_api_buf_alloc_block_cmd_t;

/// Allocate new block of buffers command complete event structure
typedef struct mesh_api_buf_alloc_block_cmp_evt
{
    /// Command code (see #mesh_api_cmd_code enumeration)
    /// - MESH_API_BUF_ALLOC_BLOCK: Allocate a new block of buffers
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;

    /// Block ID
    uint8_t block_id;
} mesh_api_buf_alloc_block_cmp_evt_t;

/// Free block of buffers command structure
typedef struct mesh_api_buf_free_block_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - MESH_API_BUF_FREE_BLOCK: Free a block of buffers
    uint32_t cmd_code;
    /// Block ID
    uint8_t block_id;
} mesh_api_buf_free_block_cmd_t;

/// Free block of buffers command complete event structure
typedef struct mesh_api_buf_free_block_cmp_evt
{
    /// Command code (see #mesh_api_cmd_code enumeration)
    /// - MESH_API_BUF_FREE_BLOCK: Free a block of buffers
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;

    /// Block ID
    uint8_t block_id;
} mesh_api_buf_free_block_cmp_evt_t;

/// @} MESH_COMMON_BUFFER_MSG
/// @} MESH_COMMON_MSG

/// @addtogroup MESH_PROFILE_MSG
/// @{
/// @addtogroup MESH_PROFILE_GENERAL_MSG
/// @{

/*
 * ENUMERATIONS FOR MESH PROFILE
 ****************************************************************************************
 */

/// Command codes for MESH_API_CMD and MESH_API_CMP_EVT messages
enum m_api_cmd_code
{
    /// Enable Mesh profile
    M_API_ENABLE                        = 0,
    /// Disable Mesh profile
    M_API_DISABLE                       = 1,
    /// Register a model instance
    M_API_REGISTER_MODEL                = 4,
    /// Bind the application key with the model
    M_API_BIND_APP_MODEL                = 5,
    /// Unbind the application key with the model
    M_API_UNBIND_APP_MODEL              = 6,

    /// Model request to publish a new message
    M_API_MODEL_PUBLISH                 = 20,
    /// Model send a response message
    M_API_MODEL_SEND                    = 21,

    /// Publish health current status of primary element
    M_API_HEALTH_STATUS_SEND            = 30,

    /// Read Device public key
    M_API_PROV_PUB_KEY_READ             = 40,

    /// IV Update test mode
    M_API_IV_UPD_TEST_MODE              = 50,

    /// Require to start use of Low Power Node feature
    M_API_LPN_START                     = 60,
    /// Require to stop use of Low Power Node feature
    M_API_LPN_STOP                      = 61,
    /// Require to establish a friendship with a given node
    M_API_LPN_SELECT_FRIEND             = 62,

    /// Control the Proxy Connectable mode for server role
    M_API_PROXY_CTL                     = 70,
    /// Enable Proxy client role
    M_API_PROXY_CLI_ENABLE              = 71,
    /// Set filter type
    M_API_PROXY_CLI_SET_FILT_TYPE       = 72,
    /// Action of addresses for proxy filter
    M_API_PROXY_CLI_ACT_ADDRESSES       = 73,

    /// Load stored information
    M_API_STORAGE_LOAD                  = 80,
    #if (!(0))
    /// Request to force storage of information
    M_API_STORAGE_SAVE                  = 81,
    #endif //(!(0))
    /// Configure storage module behavior
    M_API_STORAGE_CONFIG                = 82,

    /// Add a new network key
    M_API_KEY_NET_ADD                   = 92,
    /// Update an existing network key
    M_API_KEY_NET_UPDATE                = 93,
    /// Delete a network key
    M_API_KEY_NET_DELETE                = 94,
    /// Use new key for transmission
    M_API_KEY_USE_NEW                   = 95,
    /// Remove old key for reception
    M_API_KEY_REVOKE_OLD                = 96,

    /// Add a new application key
    M_API_KEY_APP_ADD                   = 97,
    /// Update an existing application key
    M_API_KEY_APP_UPDATE                = 98,
    /// Delete a application key
    M_API_KEY_APP_DELETE                = 99,

    /// Scan for unprovisioned device beacons
    M_API_PROVER_SCAN                   = 100,
    /// Invite an unprovisioned node
    M_API_PROVER_INVITE                 = 101,
    /// Stop a provisioner procedure
    M_API_PROVER_STOP                   = 104,

    /// Request current configuration state
    M_API_CONFC_GET                     = 110,
    /// Request Network actions
    M_API_CONFC_ACT_NET                 = 111,
    /// Request Application Key actions
    M_API_CONFC_ACT_APP                 = 112,
    /// Request Model Get actions
    M_API_CONFC_GET_MDL                 = 113,
    /// Request Model Subscription actions
    M_API_CONFC_ACT_MDL_SUBS            = 114,
    /// Request Set type actions
    M_API_CONFC_SET                     = 115,
    /// Request Heartbeat Publication Set type actions
    M_API_CONFC_SET_HB_PUBLI            = 116,
    /// Request Heartbeat Subscription Set type actions
    M_API_CONFC_SET_HB_SUBS             = 117,
    /// Request Model AppKey actions
    M_API_CONFC_ACT_MDL_APP             = 118,
    /// Request Model Publication Set actions
    M_API_CONFC_SET_MDL_PUBLI           = 119,

    /// Request Get type actions
    M_API_HLTHC_GET                     = 130,
    /// Request Set type actions
    M_API_HLTHC_SET                     = 131,
    /// Request Fault actions
    M_API_HLTHC_ACT_FAULT               = 132,

};

/// Command codes for MESH_API_REQ and MESH_API_RSP messages
enum m_api_req_code
{
    /// Add a new device key
    M_API_KEY_DEV_ADD                   = 90,
    /// Delete a device key
    M_API_KEY_DEV_DELETE                = 91,

    /// Register the Configuration Client model
    M_API_CONFC_REG_MDL                 = 100,
    /// Provide device configuration information
    M_API_CONFC_SET_DEV                 = 101,

    /// Register the Health Client model
    M_API_HLTHC_REG_MDL                 = 110,

};

/// Indication codes for MESH_API_IND message
enum m_api_ind_code
{
    /// Node has been found
    M_API_PROVER_NODE_FOUND              = 128,
    /// Node has been found (PB-GATT)
    M_API_PROVER_NODE_FOUND_GATT         = 129,
    /// Scan stopped
    M_API_PROVER_SCAN_STOPPED            = 130,
    /// Mesh Proxy Service discovered
    M_API_PROVER_PROXY_SVC               = 131,
    /// Indicate provisioner State
    M_API_PROVER_STATE                   = 132,

    /// Received value (less than or equal to 4 bytes)
    M_API_CONFC_VALUE                    = 140,
    /// Received NetKey List
    M_API_CONFC_NETKEY_LIST              = 141,
    /// Received Network TX Status
    M_API_CONFC_NET_TX_STATUS            = 142,
    /// Received Relay Status
    M_API_CONFC_RELAY_STATUS             = 143,
    /// Received Heartbeat Publication Status
    M_API_CONFC_HB_PUBLI_STATUS          = 144,
    /// Received Heartbeat Subscription Status
    M_API_CONFC_HB_SUBS_STATUS           = 145,
    /// Received NetKey Status
    M_API_CONFC_NETKEY_STATUS            = 146,
    /// Received AppKey List
    M_API_CONFC_APPKEY_LIST              = 147,
    /// Received Node Identity Status
    M_API_CONFC_NODE_ID_STATUS           = 148,
    /// Received Key Refresh Phase Status
    M_API_CONFC_KEY_REFRESH_PHASE_STATUS = 149,
    /// Received Application Key Status
    M_API_CONFC_APPKEY_STATUS            = 150,
    /// Received Model Publication Status
    M_API_CONFC_MDL_PUB_STATUS           = 151,
    /// Received Model Subscription List
    M_API_CONFC_MDL_SUBS_LIST            = 152,
    /// Received Model AppKey indexes List
    M_API_CONFC_MDL_APP_LIST             = 153,
    /// Received Model Subscription or AppKey status
    M_API_CONFC_MDL_SUBS_APP_STATUS      = 154,
    /// Received Node Reset status
    M_API_CONFC_NODE_RESET_STATUS        = 155,
    /// Received LPN Poll Timeout status
    M_API_CONFC_LPN_POLLTIMEOUT_STATUS   = 156,
    /// Received Composition Data (Page > 0)
    M_API_CONFC_COMPO_DATA               = 157,
    /// Received Composition Data (Page 0)
    M_API_CONFC_COMPO_DATA_PAGE0         = 158,
    /// Received Element field in Composition Data (Page 0)
    M_API_CONFC_COMPO_DATA_ELMT          = 159,

    /// Received value (1 byte status)
    M_API_HLTHC_VALUE                    = 160,
    /// Received Current Status or Fault Status
    M_API_HLTHC_CUR_FAULT_STATUS         = 161,

    /// Indicate device public key
    M_API_PROVEE_PUB_KEY_OOB             = 170,

    /// Indicate proxy filter status
    M_API_PROXY_FILT_STATUS              = 180,

};

/// Request Indication codes for MESH_API_REQ_IND message
enum m_api_req_ind_code
{
    /// Request for provisioner identify
    M_API_PROVER_IDENTIFY                = 200,
    /// Request for device public key
    M_API_PROVER_PUB_KEY_OOB             = 201,
};


/*
 * MESSAGE DEFINITIONS FOR MESH PROFILE
 ****************************************************************************************
 */

/// Command required structure (without parameters)
typedef struct m_api_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    uint32_t cmd_code;
} m_api_cmd_t;

/// Command complete event required structure (without parameters)
typedef struct m_api_cmp_evt
{
    /// Command code (see #m_api_cmd_code enumeration)
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
} m_api_cmp_evt_t;

/// Request required structure (without parameters)
typedef struct m_api_req
{
    /// Request code (see #m_api_req_code enumeration)
    uint32_t req_code;
} m_api_req_t;

/// Response required structure (without parameters)
typedef struct m_api_rsp
{
    /// Request code (see #m_api_req_code enumeration)
    uint32_t req_code;
    /// Status of the command execution
    uint16_t status;
} m_api_rsp_t;

/// Mesh indication (without parameters)
typedef struct m_api_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    uint32_t ind_code;
} m_api_ind_t;

/// Mesh Request indication (without parameters)
typedef struct m_api_req_ind
{
    /// Request indication code (see #m_api_req_ind_code enumeration)
    uint32_t req_ind_code;
} m_api_req_ind_t;

/// Mesh Confirmation (without parameters)
typedef struct m_api_cfm
{
    /// Request indication code (see #m_api_req_ind_code enumeration)
    uint32_t req_ind_code;
    /// Status
    uint16_t status;
} m_api_cfm_t;

/// Enable mesh profile Command complete event
typedef struct m_api_enable_cmp_evt
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_ENABLE: Enable mesh profile
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
    /// Indicate if node is provisioned or not
    bool prov;
} m_api_enable_cmp_evt_t;

/// Register a model instance command structure
typedef struct m_api_register_model_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_REGISTER_MODEL: Register a model instance
    uint32_t cmd_code;

    /// Model ID
    uint32_t model_id;
    /// Index of element the models belongs to
    uint8_t elmt_idx;
    /// Configuration
    uint8_t config;
} m_api_register_model_cmd_t;

/// Register a model instance command complete event structure
typedef struct m_api_register_model_cmp_evt
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_REGISTER_MODEL: Register a model instance
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;

    /// Model LID
    m_lid_t model_lid;
} m_api_register_model_cmp_evt_t;

/// Bind Application Key with the Model
typedef struct m_api_bind_app_model_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_BIND_APP_MODEL: Bind Application Key with the Model
    uint32_t cmd_code;
    /// Application key local index
    m_lid_t  app_key_lid;
    /// Model local index
    m_lid_t mdl_lid;
} m_api_bind_app_model_cmd_t;

/// Unbind Application Key with the Model
typedef struct m_api_unbind_app_model_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_UNBIND_APP_MODEL: Unbind Application Key with the Model
    uint32_t cmd_code;
    /// Application key local index
    m_lid_t  app_key_lid;
    /// Model local index
    m_lid_t mdl_lid;
} m_api_unbind_app_model_cmd_t;

/// Load stored information command structure
typedef struct m_api_storage_load_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_STORAGE_LOAD: Load stored information
    uint32_t cmd_code;

    /// Length
    uint32_t length;
    /// Stored information
    uint8_t data[__ARRAY_EMPTY];
} m_api_storage_load_cmd_t;

/// Configure storage module command structure
typedef struct m_api_storage_config_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_STORAGE_CONFIG: Configure storage module
    uint32_t cmd_code;

    /// Configuration (meaning depends on compiled storage module)
    uint32_t config;
} m_api_storage_config_cmd_t;

/// Model request to publish a new message
typedef struct m_api_model_publish_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_MODEL_PUBLISH: Model request to publish a new message
    uint32_t cmd_code;
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Handle value used by model to retrieve which message has been sent
    uint8_t  tx_hdl;
    /// 1 = Segmented PDU force transport MIC to 64 bits ; 0 = 32 bits transport MIC
    uint8_t  trans_mic_64;
    /// Operation code of the message
    uint32_t opcode;
    /// Message length
    uint16_t len;
    /// Message content
    uint8_t  msg[__ARRAY_EMPTY];
} m_api_model_publish_cmd_t;

/// Model send a message
typedef struct m_api_model_send_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_MODEL_SEND: Model send a response message
    uint32_t cmd_code;
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Key information.
    /// If key_lid & 0x80 != 0, key_lid & 0x7F = network key local index
    /// else key_lid & 0x7F = application key local index.
    m_lid_t  key_lid;
    /// Handle value used by model to retrieve which message has been sent
    uint8_t  tx_hdl;
    /// 1 = Segmented PDU force transport MIC to 64 bits ; 0 = 32 bits transport MIC
    uint8_t  trans_mic_64;
    /// Operation code of the message
    uint32_t opcode;
    /// Destination address of the message
    uint16_t dst;
    /// 1, send message to an immediate peer; 0, accept message to be relayed
    uint8_t  not_relay;
    /// Message length
    uint16_t len;
    /// Message content
    uint8_t  msg[__ARRAY_EMPTY];

} m_api_model_send_cmd_t;

/// @} MESH_PROFILE_GENERAL_MSG

/// @addtogroup MESH_PROFILE_HLTHS_MSG
/// @{

/// Model send a response message
typedef struct m_api_health_status_send_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_SEND_HEALTH_STATUS: Model send a response message
    uint32_t cmd_code;
    /// Company ID
    uint16_t comp_id;
    /// Test ID
    uint8_t  test_id;
    /// Length of fault array
    uint8_t  length;
    /// Fault array
    uint8_t  fault_array[__ARRAY_EMPTY];
} m_api_health_status_send_cmd_t;

/// @} MESH_PROFILE_HLTHS_MSG

/// @addtogroup MESH_PROFILE_CONFC_MSG
/// @{

/// Register the model
typedef struct m_api_confc_reg_mdl_req
{
    /// Request code (see #m_api_req_code enumeration)
    /// - M_API_CONFC_REG_MDL: Register the model
    uint32_t req_code;
} m_api_confc_reg_mdl_req_t;

/// Provide device configuration information
typedef struct m_api_confc_set_dev_req
{
    /// Request code (see #m_api_req_code enumeration)
    /// - M_API_CONFC_SET_DEV: Provide device configuration information
    uint32_t req_code;
    /// Device key local index
    m_lid_t  dev_key_lid;
    /// Network key local index
    m_lid_t  net_key_lid;
    /// Device primary address
    uint16_t addr;
} m_api_confc_set_dev_req_t;

/// Request configuration state
typedef struct m_api_confc_get_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_CONFC_GET: Request configuration state
    uint32_t cmd_code;
    /// Type of state to request (see #m_fnd_confc_get_type enumeration)
    uint8_t get_type;
    /// Value of the LPN address/Composition Data page number
    uint16_t val;
} m_api_confc_get_cmd_t;

/// Request network actions
typedef struct m_api_confc_act_net_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_CONFC_ACT_NET: Request network actions
    uint32_t cmd_code;
    /// Type of network action to request
    uint8_t net_act_type;
    /// Network Key Index
    uint16_t net_key_id;
    /// Data (NetKey/Transition/Identity)
    uint8_t  data[__ARRAY_EMPTY];
} m_api_confc_act_net_cmd_t;

/// Request application key actions
typedef struct m_api_confc_act_app_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_CONFC_ACT_APP: Request application key actions
    uint32_t cmd_code;
    /// Type of application key action to request
    uint8_t app_act_type;
    /// Network Key Index
    uint16_t net_key_id;
    /// Application Key Index
    uint16_t app_key_id;
    /// Application Key
    uint8_t  app_key[__ARRAY_EMPTY];
} m_api_confc_act_app_cmd_t;

/// Request model get actions
typedef struct m_api_confc_get_mdl_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_CONFC_GET_MDL: Request model get actions
    uint32_t cmd_code;
    /// Type of model get action to request
    uint8_t mdl_get_type;
    /// Address of the element
    uint16_t elm_addr;
    /// SIG Model ID or Vendor Model ID
    uint32_t mdl_id;
} m_api_confc_get_mdl_cmd_t;

/// Request model subscription actions
typedef struct m_api_confc_act_mdl_subs_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_CONFC_ACT_MDL_SUBS: Request model subscription actions
    uint32_t cmd_code;
    /// Type of model subscription action to request
    uint8_t mdl_subs_act_type;
    /// Address of the element
    uint16_t elm_addr;
    /// SIG Model ID or Vendor Model ID
    uint32_t mdl_id;
    /// 0 for group address and 1 for Label UUID
    bool addr_type;
    /// Length of value
    uint16_t length;
    /// Group address or Label UUID
    uint8_t addr_uuid[__ARRAY_EMPTY];
} m_api_confc_act_mdl_subs_cmd_t;

/// Request Set type actions
typedef struct m_api_confc_set_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_CONFC_SET: Request Set type actions
    uint32_t cmd_code;
    /// Type of Set action to request
    uint8_t set_type;
    /// Value of Beacon/TTL/Proxy/Friend/Relay
    uint8_t value;
    /// Number of transmissions for each Network PDU originating from the node
    uint8_t tx_cnt;
    /// Number of 10-millisecond steps between transmissions
    uint8_t intv_slots;
} m_api_confc_set_cmd_t;

/// Request Heartbeat Publication Set type actions
typedef struct m_api_confc_set_hb_publi_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_CONFC_SET_HB_PUBLI: Request Heartbeat Publication Set type actions
    uint32_t cmd_code;
    /// Destination address for Heartbeat messages
    uint16_t dst;
    /// Number of Heartbeat messages to be sent
    uint16_t cnt;
    /// Period for sending Heartbeat messages
    uint16_t period_s;
    /// TTL to be used when sending Heartbeat messages
    uint8_t ttl;
    /// Bit field indicating features that trigger Heartbeat messages when changed
    uint16_t features;
    /// NetKey Index
    uint16_t netkey_id;
} m_api_confc_set_hb_publi_cmd_t;

/// Request Heartbeat Subscription Set type actions
typedef struct m_api_confc_set_hb_subs_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_CONFC_SET_HB_SUBS: Request Heartbeat Subscription Set type actions
    uint32_t cmd_code;
    /// Source address for Heartbeat messages
    uint16_t src;
    /// Destination address for Heartbeat messages
    uint16_t dst;
    /// Period for sending Heartbeat messages
    uint16_t period_s;
} m_api_confc_set_hb_subs_cmd_t;

/// Request Model AppKey actions
typedef struct m_api_confc_act_mdl_app_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_CONFC_ACT_MDL_APP: Request Model AppKey actions
    uint32_t cmd_code;
    /// Type of Model AppKey action to request
    uint8_t mdl_app_act_type;
    /// Address of the element
    uint16_t elm_addr;
    /// Index of AppKey
    uint16_t app_key_id;
    /// SIG Model ID or Vendor Model ID
    uint32_t mdl_id;
} m_api_confc_act_mdl_app_cmd_t;

/// Request Model Publication Set actions
typedef struct m_api_confc_set_mdl_publi_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_CONFC_SET_MDL_PUBLI: Request Model Publication Set actions
    uint32_t cmd_code;
    /// Type of Model Publication Set address
    uint8_t addr_type;
    /// Address of the element
    uint16_t elm_addr;
    /// Index of the application key
    uint16_t app_key_id;
    /// Value of the Friendship Credential Flag
    bool cred_flag;
    /// Default TTL value for the outgoing messages
    uint8_t pub_ttl;
    /// Period for periodic status publishing
    uint8_t pub_period;
    /// Number of retransmissions for each published message
    uint8_t retx_cnt;
    /// Number of 50-millisecond steps between retransmissions
    uint8_t retx_intv_slots;
    /// SIG Model ID or Vendor Model ID
    uint32_t mdl_id;
    /// Length of value
    uint16_t length;
    /// Value of the Label UUID/Publish address
    uint8_t val[__ARRAY_EMPTY];
} m_api_confc_set_mdl_publi_cmd_t;

/// @} MESH_PROFILE_CONFC_MSG

/// @addtogroup MESH_PROFILE_HLTHC_MSG
/// @{

/// Register the model
typedef struct m_api_hlthc_reg_mdl_req
{
    /// Request code (see #m_api_req_code enumeration)
    /// - M_API_HLTHC_REG_MDL: Register the model
    uint32_t req_code;
} m_api_hlthc_reg_mdl_req_t;

/// Register the model response structure
typedef struct m_api_hlthc_reg_mdl_rsp
{
    ///  Request code (see #m_api_req_code enumeration)
    /// - M_API_HLTHC_REG_MDL: Register the model
    uint32_t req_code;
    /// Status of the command execution
    uint16_t status;
    /// Model Local index registered
    m_lid_t  mdl_lid;
} m_api_hlthc_reg_mdl_rsp_t;

/// Request Get type actions
typedef struct m_api_hlthc_get_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_HLTHC_GET: Request Get type actions
    uint32_t cmd_code;

    /// Destination address of this message
    uint16_t addr;
     /// Application Key bound with the model
    m_lid_t app_key_lid;
    /// Type of Get action to request (see #m_fnd_hlthc_get_type enumeration)
    uint8_t get_type;
    /// Company identifier
    uint16_t comp_id;
} m_api_hlthc_get_cmd_t;

/// Request Set type actions
typedef struct m_api_hlthc_set_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_HLTHC_SET: Request Set type actions
    uint32_t cmd_code;

    /// Destination address of this message
    uint16_t addr;
     /// Application Key bound with the model
    m_lid_t app_key_lid;
    /// Type of Set action to request (see #m_fnd_hlthc_set_type enumeration)
    uint8_t set_type;
    /// Set Configuration bit field to indicate if it's acknowledged (see #m_fnd_hlthc_set_cfg_bf enumeration)
    uint8_t set_cfg;
    /// Fast Period Divisor or Attention value
    uint8_t val;
} m_api_hlthc_set_cmd_t;

/// Request Fault actions
typedef struct m_api_hlthc_act_fault_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_HLTHC_ACT_FAULT: Request Fault actions
    uint32_t cmd_code;

    /// Destination address of this message
    uint16_t addr;
     /// Application Key bound with the model
    m_lid_t app_key_lid;
    /// Type of Fault action to request (see #m_fnd_hlthc_action_type enumeration)
    uint8_t action_type;
    /// Fault action Configuration bit field to indicate if it's acknowledged
    uint8_t action_cfg;
    /// Identifier of a specific test to be performed (not used for Fault Clear)
    uint8_t test_id;
    /// Company identifier
    uint16_t comp_id;
} m_api_hlthc_act_fault_cmd_t;

/// @} MESH_PROFILE_HLTHC_MSG

/// @addtogroup MESH_PROFILE_FWDC_MSG
/// @{


/// @} MESH_PROFILE_FWDC_MSG

/// @addtogroup MESH_PROFILE_GENERAL_MSG
/// @{

/// Request model if a specific operation code is supported (shall be confirmed with MESH_MODEL_OPCODE_CFM message)
typedef struct m_api_model_opcode_req_ind
{
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Mesh message operation code (can be 1, 2 or 3 octet operation code)
    uint32_t opcode;
} m_api_model_opcode_req_ind_t;

/// Used to answer if a model support or not a specific operation code.
typedef struct m_api_model_opcode_cfm
{
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// MESH_ERR_OR_NO_ERROR if supported, else rejected
    uint16_t status;
    /// Mesh message operation code (can be 1, 2 or 3 octet operation code)
    uint32_t opcode;
} m_api_model_opcode_cfm_t;

/// Inform reception of a specific mesh message
typedef struct m_api_model_msg_ind
{
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Application Key Local identifier (Required for a response)
    m_lid_t  app_key_lid;
    /// Measured RSSI level for the received PDU.
    int8_t   rssi;
    /// 1 = if message have been received by an immediate peer; 0 = it can have been relayed
    uint8_t  not_relayed;
    /// Mesh message operation code (can be 1, 2 or 3 octet operation code)
    uint32_t opcode;
    /// Source address of the message (Required for a response)
    uint16_t src;
    /// Message length
    uint16_t msg_len;
    /// Message content
    uint8_t  msg[__ARRAY_EMPTY];
} m_api_model_msg_ind_t;

/// Inform transmission status of message issued by local model
typedef struct m_api_model_msg_sent_ind
{
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Handle value configured by model when message has been requested to be sent
    uint8_t  tx_hdl;
    /// Transmission status
    uint16_t status;
} m_api_model_msg_sent_ind_t;

/// Inform about new publication parameters for a model
typedef struct m_api_model_publish_param_ind
{
    /// Model Local Identifier
    m_lid_t  model_lid;
    /// Publish period in milliseconds
    uint32_t period_ms;
    /// Publication address
    uint16_t addr;
} m_api_model_publish_param_ind_t;

/// Inform about attention state update
typedef struct m_api_attention_update_ind
{
    /// Attention state
    uint8_t attention_state;
} m_api_attention_update_ind_t;

/// Structure for MESH_API_GROUP_UPDATE_IND message
typedef struct m_api_group_update_ind
{
    /// AppKey ID
    uint16_t appkey_id;
    /// Application key local index
    m_lid_t app_key_lid;
    /// Added (> 0) or deleted (= 0)
    uint8_t added;
} m_api_group_update_ind_t;

/// @} MESH_PROFILE_GENERAL_MSG

/// @addtogroup MESH_PROFILE_CONFS_MSG
/// @{

/// Request a page of the composition data
typedef struct m_api_compo_data_req_ind
{
    /// Page
    uint8_t page;
} m_api_compo_data_req_ind_t;

/// Response containing the required composition data page
typedef struct m_api_compo_data_cfm
{
    /// Page
    uint8_t page;
    /// Length
    uint8_t length;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} m_api_compo_data_cfm_t;

/// @} MESH_PROFILE_CONFS_MSG

/// @addtogroup MESH_PROFILE_GENERAL_MSG
/// @{

/// Configuration update indication message structure
typedef struct m_api_update_ind
{
    /// Update type
    uint8_t upd_type;
    /// Entry length
    uint8_t length;
    /// Entry value
    uint8_t data[__ARRAY_EMPTY];
} m_api_update_ind_t;

/// Request to get the current Registered Fault state identified by Company ID for primary element
typedef struct m_api_fault_get_req_ind
{
    /// Company ID
    uint16_t comp_id;
} m_api_fault_get_req_ind_t;

/// Request to start a test procedure of primary element
typedef struct m_api_fault_test_req_ind
{
    /// Company ID
    uint16_t comp_id;
    /// Test ID
    uint8_t  test_id;
    /// Indicate if MESH_FAULT_CFM message is expected
    bool     cfm_needed;
} m_api_fault_test_req_ind_t;

/// Response containing current Registered Fault state for primary element
typedef struct m_api_fault_cfm
{
    /// Accept (true) or reject (false) the request
    bool     accept;
    /// Company ID
    uint16_t comp_id;
    /// Test ID
    uint8_t  test_id;
    /// Length of fault array
    uint8_t  length;
    /// Fault array
    uint8_t  fault_array[__ARRAY_EMPTY];
} m_api_fault_cfm_t;

/// Inform that clear of Registered Fault state identified by Company ID has been received for primary element
typedef struct m_api_fault_clear_ind
{
    /// Company ID
    uint16_t comp_id;
} m_api_fault_clear_ind_t;

/// Inform about new publication period for Current Health state of primary element
typedef struct m_api_fault_period_ind
{
    /// Publication period in milliseconds when no fault is known
    uint32_t period_ms;
    /// Publication period in milliseconds when one or several fault are known
    uint32_t period_fault_ms;
} m_api_fault_period_ind_t;

/// IV Update test mode command
typedef struct m_api_iv_upd_test_mode_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_IV_UPD_TEST_MODE: Configure IV Update test mode
    uint32_t cmd_code;
    /// Transit to IV Update in Progress operation (true) or to Normal operation (false)
    bool     update;
} m_api_iv_upd_test_mode_cmd_t;

/// @} MESH_PROFILE_GENERAL_MSG

/// @addtogroup MESH_PROFILE_PROV_MSG
/// @{

/// Mesh Provisioning state change indication
typedef struct m_api_prov_state_ind
{
    /// Provisioning procedure state (see #m_prov_state enumeration)
    uint8_t state;
    /// Relevant only for provisioning failed (failed reason)
    uint16_t status;
} m_api_prov_state_ind_t;

/// Mesh Provisioning Parameters Response
typedef struct m_api_prov_param_cfm
{
    /// Device UUID
    uint8_t  dev_uuid[MESH_DEV_UUID_LEN];
    /// URI hash
    uint32_t uri_hash;
    /// OOB information
    uint16_t oob_info;
    /// Public key OOB information available
    uint8_t  pub_key_oob;
    /// Static OOB information available
    uint8_t  static_oob;
    /// Maximum size of Output OOB supported
    uint8_t  out_oob_size;
    /// Maximum size in octets of Input OOB supported
    uint8_t  in_oob_size;
    /// Supported Output OOB Actions (see #m_prov_out_oob enumeration)
    uint16_t out_oob_action;
    /// Supported Input OOB Actions (see #m_prov_in_oob enumeration)
    uint16_t in_oob_action;
    /// Bit field providing additional information (see #m_prov_info enumeration)
    uint8_t  info;
} m_api_prov_param_cfm_t;

/// Mesh Provisioning Authentication Data Request
typedef struct m_api_prov_auth_data_req_ind
{
    /// Authentication method (see #m_prov_auth_method enumeration)
    uint8_t  auth_method;
    /// expected authentication maximum data size
    uint8_t  auth_size;
    /// Authentication Action:
    ///   - M_PROV_AUTH_NO_OOB     = Prohibited
    ///   - M_PROV_AUTH_STATIC_OOB = 16 bytes LSB static out of band data required
    ///   - M_PROV_AUTH_OUTPUT_OOB = see #m_prov_out_oob enumeration, 1 bit set.
    ///   - M_PROV_AUTH_INPUT_OOB  = see #m_prov_in_oob enumeration, 1 bit set.
    uint16_t auth_action;
} m_api_prov_auth_data_req_ind_t;

/// Mesh Provisioning Authentication Data Response
typedef struct m_api_prov_auth_data_cfm
{
    /// 1, Accept pairing request, 0 reject
    uint8_t  accept;
    /// Authentication data size (<= requested size else pairing automatically rejected)
    uint8_t  auth_size;
    /// Authentication data (LSB for a number or array of bytes)
    uint8_t  auth_data[__ARRAY_EMPTY];
} m_api_prov_auth_data_cfm_t;

/// Read Device public key command
typedef struct m_api_prov_pub_key_read_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_PROV_PUB_KEY_READ: Read Device public key
    uint32_t cmd_code;
} m_api_prov_pub_key_read_cmd_t;

/// Read Device public key Command complete event
typedef struct m_api_prov_pub_key_read_cmp_evt
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_PROV_PUB_KEY_READ: Read Device public key
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
    /// X Coordinate of public Key (32 bytes LSB)
    uint8_t  pub_key_x[MESH_PUB_KEY_X_LEN];
    /// Y Coordinate of public Key (32 bytes LSB)
    uint8_t  pub_key_y[MESH_PUB_KEY_Y_LEN];
} m_api_prov_pub_key_read_cmp_evt_t;

/// @} MESH_PROFILE_PROV_MSG

/// @addtogroup MESH_PROFILE_LPN_MSG
/// @{

/// Start Low Power Node feature command structure
typedef struct m_api_lpn_start_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_LPN_START: Start Low Power Node feature
    uint32_t cmd_code;

    /// Initial value of PollTimeout timer (in multiple of 100ms)
    uint32_t poll_timeout;
    /// Poll interval (in milliseconds)
    uint32_t poll_intv_ms;
    /// Unicast address of the primary element of the previous friend
    uint16_t prev_addr;
    /// Receive delay
    uint8_t  rx_delay;
    /// RSSI factor
    uint8_t  rssi_factor;
    /// Receive window factor
    uint8_t  rx_window_factor;
    /// Minimum queue size (log value)
    uint8_t  min_queue_size_log;
} m_api_lpn_start_cmd_t;

/// Select Friend Node command structure
typedef struct m_api_lpn_select_friend_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_LPN_SELECT_FRIEND: Select Friend node
    uint32_t cmd_code;

    /// Friend node address
    uint16_t friend_addr;
} m_api_lpn_select_friend_cmd_t;

/// Low Power Node offer reception indication structure
typedef struct m_api_lpn_offer_ind
{
    /// Address of Friend node that sent the Friend Offer message
    uint16_t addr;
    /// Receive window value supported by the Friend node
    uint8_t  rx_window;
    /// Queue size available on the Friend node
    uint8_t  queue_size;
    /// Size of the subscription list that can be supported by the Friend Node
    uint8_t  subs_list_size;
    /// RSSI measured by the Friend node
    int8_t   rssi;
} m_api_lpn_offer_ind_t;

/// Low Power Node status indication structure
typedef struct m_api_lpn_status_ind
{
    /// Status
    uint16_t status;
    /// Friend address
    uint16_t friend_addr;
} m_api_lpn_status_ind_t;

/// @} MESH_PROFILE_LPN_MSG

/// @addtogroup MESH_PROFILE_PROXY_MSG
/// @{

/// Control the Proxy Connectable mode for server role
typedef struct m_api_proxy_ctl_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_PROXY_CTL: Control proxy
    uint32_t cmd_code;

    /// Required update (see #mesh_proxy_adv_ctl enumeration)
    uint8_t enable;
} m_api_proxy_ctl_cmd_t;

/// Enable Proxy client role
typedef struct m_api_proxy_cli_enable_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_PROXY_CLI_ENABLE: Enable Proxy client role
    uint32_t cmd_code;

    /// Connection index
    uint8_t conidx;
} m_api_proxy_cli_enable_cmd_t;

/// Set filter type
typedef struct m_api_proxy_cli_set_filt_type_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_PROXY_CLI_SET_FILT_TYPE: Set filter type
    uint32_t cmd_code;

    /// Connection index
    uint8_t conidx;
    /// Filter type
    uint8_t filt_type;
    /// Network key local index
    m_lid_t net_key_lid;
    /// Destination address
    uint16_t dst;
} m_api_proxy_cli_set_filt_type_cmd_t;

/// Action of addresses for proxy filter
typedef struct m_api_proxy_cli_act_addresses_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_PROXY_CLI_ACT_ADDRESSES: Addresses action
    uint32_t cmd_code;

    /// Connection index
    uint8_t conidx;
    /// True for add and false for remove
    bool add_rem;
    /// Number of addresses
    uint16_t nb_addr;
    /// Network key local index
    m_lid_t net_key_lid;
    /// Destination address
    uint16_t dst;
    /// Addresses list
    uint16_t addr_list[__ARRAY_EMPTY];
} m_api_proxy_cli_act_addresses_cmd_t;

/// Indicate that proxy advertising has been started or stopped and the reason
typedef struct m_api_proxy_adv_update_ind
{
    /// State (see #mesh_proxy_adv_upd enumeration)
    uint8_t state;
    /// Reason (see #mesh_proxy_adv_upd_reason enumeration)
    uint8_t reason;
} m_api_proxy_adv_update_ind_t;

/// Indicate the proxy filter status
typedef struct m_api_proxy_filt_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_PROXY_FILT_STATUS: Proxy filter status
    uint32_t ind_code;

    /// Filter type: white list or black list
    uint8_t filt_type;
    /// Number of addresses in the proxy filter list
    uint16_t list_size;
} m_api_proxy_filt_status_ind_t;

/// @} MESH_PROFILE_PROXY_MSG

/// @addtogroup MESH_PROFILE_KEY_MSG
/// @{

/// Add a new device key request structure
typedef struct m_api_key_dev_add_req
{
    /// Request code (see #m_api_req_code enumeration)
    /// - M_API_KEY_DEV_ADD: Add a new device key
    uint32_t req_code;
    /// Key to add
    uint8_t  key[MESH_KEY_LEN];
    /// Address bound to the device key
    uint16_t addr;
} m_api_key_dev_add_req_t;

/// Add a new device key command response structure
typedef struct m_api_key_dev_add_rsp
{
    ///  Request code (see #m_api_req_code enumeration)
    /// - M_API_KEY_DEV_ADD: Add a new device key
    uint32_t req_code;
    /// Status of the command execution
    uint16_t status;
    /// Device key local identifier
    m_lid_t  dev_key_lid;
} m_api_key_dev_add_rsp_t;

/// Delete a device key request structure
typedef struct m_api_key_dev_delete_req
{
    /// Request code (see #m_api_req_code enumeration)
    /// - M_API_KEY_DEV_DELETE: Delete a device key
    uint32_t req_code;
    /// Device key index
    uint16_t dev_key_id;
} m_api_key_dev_delete_req_t;

/// Add a new network key Command structure
typedef struct m_api_key_net_add_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_NET_ADD: Add a new network key
    uint32_t cmd_code;
    /// Network key index
    uint16_t net_key_id;
    /// Key to add
    uint8_t  key[MESH_KEY_LEN];
} m_api_key_net_add_cmd_t;

/// Add a new network key command complete event structure
typedef struct m_api_key_net_add_cmp_evt
{
    ///  Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_NET_ADD: Add a new network key
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
    /// Network key local identifier
    m_lid_t  net_key_lid;
} m_api_key_net_add_cmp_evt_t;

/// Update an existing network key Command structure
typedef struct m_api_key_net_update_add_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_NET_UPDATE: Update an existing network key
    uint32_t cmd_code;
    /// Network key index
    uint16_t net_key_id;
    /// Key to update
    uint8_t  key[MESH_KEY_LEN];
} m_api_key_net_update_cmd_t;

/// Update an existing network key event structure
typedef struct m_api_key_net_update_cmp_evt
{
    ///  Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_NET_UPDATE: Update an existing network key
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
    /// Network key local identifier
    m_lid_t  net_key_lid;
} m_api_key_net_update_cmp_evt_t;

/// Update an existing application key Command structure
typedef struct m_api_key_app_update_add_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_APP_UPDATE: Update an existing application key
    uint32_t cmd_code;
    /// Network key index
    uint16_t net_key_id;
    /// Application key index
    uint16_t app_key_id;
    /// Key to add
    uint8_t  key[MESH_KEY_LEN];
} m_api_key_app_update_cmd_t;

/// Update an existing application key event structure
typedef struct m_api_key_app_update_cmp_evt
{
    ///  Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_APP_UPDATE: Update an existing application key
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
    /// Application key local identifier
    m_lid_t  app_key_lid;
} m_api_key_app_update_cmp_evt_t;

/// Delete a network key Command structure
typedef struct m_api_key_net_delete_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_NET_DELETE: Delete a network key
    uint32_t cmd_code;
    /// Network key index
    uint16_t net_key_id;
} m_api_key_net_delete_cmd_t;
/// Delete a network key event structure - use m_api_cmp_evt

/// Delete a application key Command structure
typedef struct m_api_key_app_delete_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_APP_DELETE: Delete a application key
    uint32_t cmd_code;
    /// Network key index
    uint16_t net_key_id;
    /// Application key index
    uint16_t app_key_id;
} m_api_key_app_delete_cmd_t;
/// Delete a network key event structure - use m_api_cmp_evt

/// Force usage of new key for transmission
typedef struct m_api_key_use_new_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_USE_NEW: Use new key for transmission
    uint32_t cmd_code;
    /// Network key index
    uint16_t net_key_id;
} m_api_key_use_new_cmd_t;
/// Force usage of new key for transmission event structure - use m_api_cmp_evt

/// Remove the old key - only new key will be used for reception
typedef struct m_api_key_revoke_old_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_REVOKE_OLD: Remove old key for reception
    uint32_t cmd_code;
    /// Network key index
    uint16_t net_key_id;
} m_api_key_revoke_old_cmd_t;
///  Remove the old key - only new key will be used for reception event structure - use m_api_cmp_evt

/// Add a new application key Command structure
typedef struct m_api_key_app_add_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_APP_ADD: Add a new application key
    uint32_t cmd_code;
    /// Network key index bound to the new application key
    uint16_t net_key_id;
    /// Application key index
    uint16_t app_key_id;
    /// Key to add
    uint8_t  key[MESH_KEY_LEN];
} m_api_key_app_add_cmd_t;

/// Add a new application key command complete event structure
typedef struct m_api_key_app_add_cmp_evt
{
    ///  Command code (see #m_api_cmd_code enumeration)
    /// - M_API_KEY_APP_ADD: Add a application key
    uint32_t cmd_code;
    /// Status of the command execution
    uint16_t status;
    /// Application key local identifier
    m_lid_t  app_key_lid;
} m_api_key_app_add_cmp_evt_t;

/// @} MESH_PROFILE_KEY_MSG

/// @addtogroup MESH_PROFILE_PROV_MSG
/// @{

/// Scan for unprovisioned device beacons command structure
typedef struct m_api_prover_scan_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_PROVER_SCAN: Scan for unprovisioned device beacons
    uint32_t cmd_code;

    /// Scan timeout in seconds
    uint16_t timeout_s;
    /// Configuration bit field (see #m_prover_scan_cfg_bf enumeration)
    uint8_t cfg_bf;
} m_api_prover_scan_cmd_t;

/// Invite an unprovisioned node command structure
typedef struct m_api_prover_invite_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_PROVER_INVITE: invite an unprovisioned node
    uint32_t cmd_code;

    /// Connection index
    uint8_t conidx;
    /// Unprovisioned Device UUID
    uint8_t dev_uuid[MESH_DEV_UUID_LEN];
    /// Attention duration in seconds
    uint8_t attention_dur_s;
} m_api_prover_invite_cmd_t;

/// Inform application about scan procedure is stopped
typedef struct m_api_prover_scan_stopped_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_PROVER_SCAN_STOPPED: scan stopped
    uint32_t ind_code;

    /// Stopped Reason (see #m_prover_scan_stopped_reason enumeration)
    uint8_t reason;
} m_api_prover_scan_stopped_ind_t;

/// Inform application about discovery of an unprovisioned node
typedef struct m_api_prover_node_found_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_PROVER_NODE_FOUND: discovery of an unprovisioned node
    uint32_t ind_code;

    /// Hash of the associated URI advertised with the URI AD type
    uint32_t uri_hash;
    /// OOB information
    uint16_t oob_info;
    /// RSSI
    int8_t rssi;
    /// Device UUID
    uint8_t dev_uuid[MESH_DEV_UUID_LEN];
} m_api_prover_node_found_ind_t;

/// Inform application about discovery of an unprovisioned node (PB-GATT)
typedef struct m_api_prover_node_found_gatt_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_PROVER_NODE_FOUND_GATT: discovery of an unprovisioned node (PB-GATT)
    uint32_t ind_code;

    /// OOB information
    uint16_t oob_info;
    /// RSSI
    int8_t rssi;
    /// Device UUID
    uint8_t dev_uuid[MESH_DEV_UUID_LEN];
    /// Transmitter address
    uint8_t addr[GAP_BD_ADDR_LEN];
    /// Transmitter address type
    uint8_t addr_type;
} m_api_prover_node_found_gatt_ind_t;

/// Inform application about provisioner state
typedef struct m_api_prover_state_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_PROVER_STATE: provisioner state
    uint32_t ind_code;

    /// Provisioning procedure state (see #m_prov_state enumeration)
    uint8_t state;
    /// Provisioning status, relevant only for provisioning failed (failed reason)
    uint16_t status;
    /// Provisioned Device Unicast address
    uint16_t unicast_addr;
} m_api_prover_state_ind_t;

/// Inform application about device public key
typedef struct m_api_provee_pub_key_oob_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_PROVEE_PUB_KEY_OOB: device public key
    uint32_t ind_code;

    /// The X component of public key for the FIPS P-256 algorithm
    uint8_t  pub_key_x[MESH_PUB_KEY_X_LEN];
    /// The Y component of public key for the FIPS P-256 algorithm
    uint8_t  pub_key_y[MESH_PUB_KEY_Y_LEN];
} m_api_provee_pub_key_oob_ind_t;

/// Inform application to configure provisioner for Start procedure
typedef struct m_api_prover_identify_req_ind
{
    /// Request Indication code (see #m_api_req_ind_code enumeration)
    /// M_API_PROVER_IDENTIFY: Provisioner configuration
    uint32_t req_ind_code;

    /// Number of elements supported by the device
    uint8_t  nb_elt;
    /// Supported algorithms and other capabilities
    uint16_t algorithms;
    /// Supported public key types
    uint8_t  pub_key_type;
    /// Supported static OOB Types
    uint8_t  static_oob_type;
    /// Maximum size of Output OOB supported
    uint8_t  out_oob_size;
    /// Supported Output OOB Actions
    uint16_t out_oob_action;
    /// Maximum size in octets of Input OOB supported
    uint8_t  in_oob_size;
    /// Supported Input OOB Actions
    uint16_t in_oob_action;
} m_api_prover_identify_req_ind_t;

/// Stop a provisioner procedure command structure
typedef struct m_api_prover_stop_cmd
{
    /// Command code (see #m_api_cmd_code enumeration)
    /// - M_API_PROVER_STOP: Stop a provisioner procedure
    uint32_t cmd_code;
} m_api_prover_stop_cmd_t;


/// Provisioner configuration confirmation structure
typedef struct m_api_prover_identify_cfm
{
    /// Request indication code (see #m_api_req_ind_code enumeration)
    /// - M_API_PROVER_IDENTIFY: Provisioner configuration
    uint32_t req_ind_code;

    /// Accept (true) or reject (false) the request
    bool accept;
    /// Network key local index
    m_lid_t net_key_lid;
    /// Unicast address
    uint16_t addr;
    /// The algorithm used for provisioning
    uint8_t algo;
    /// Public Key used
    uint8_t pub_key;
    /// Authentication Method used
    uint8_t auth_method;
    /// Selected Output OOB Action or Input OOB Action or 0x00
    uint8_t auth_action;
    /// Size of the Output OOB used or size of the Input OOB used or 0x00
    uint8_t auth_size;
} m_api_prover_identify_cfm_t;

/// Device Public Key confirmation structure
typedef struct m_api_prover_pub_key_oob_cfm
{
    /// Request indication code (see #m_api_req_ind_code enumeration)
    /// - M_API_PROVER_PUB_KEY_OOB: Device Public Key
    uint32_t req_ind_code;

    /// Accept (true) or reject (false) the request
    bool accept;
    /// The X component of public key for the FIPS P-256 algorithm
    uint8_t  pub_key_x[MESH_PUB_KEY_X_LEN];
    /// The Y component of public key for the FIPS P-256 algorithm
    uint8_t  pub_key_y[MESH_PUB_KEY_Y_LEN];
} m_api_prover_pub_key_oob_cfm_t;

/// @} MESH_PROFILE_PROV_MSG

/// @addtogroup MESH_PROFILE_CONFC_MSG
/// @{

/// Inform application about configuration client model received <=4 bytes value
typedef struct m_api_confc_value_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_VALUE: received value
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Received value
    union
    {
        /// Beacon
        uint8_t beacon;
        /// Default TTL
        uint8_t dflt_ttl;
        /// GATT Proxy
        uint8_t gatt_proxy;
        /// Friend
        uint8_t friend;
    } value;
} m_api_confc_value_ind_t;

/// Inform application about configuration client model received Network TX status
typedef struct m_api_confc_net_tx_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_NET_TX_STATUS: received Network TX status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Number of transmissions for each Network PDU originating from the node
    uint8_t nb_tx;
    /// Number of 10-millisecond steps between transmissions
    uint8_t intv_slots;
} m_api_confc_net_tx_status_ind_t;

/// Inform application about configuration client model received Relay status
typedef struct m_api_confc_relay_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_RELAY_STATUS: received Relay status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Relay state
    uint8_t relay;
    /// Number of retransmissions on advertising bearer for each Network PDU relayed by the node
    uint8_t nb_retx;
    /// Number of 10-millisecond steps between retransmissions
    uint8_t intv_slots;
} m_api_confc_relay_status_ind_t;

/// Inform application about configuration client model received Heartbeat Publication status
typedef struct m_api_confc_hb_publi_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_HB_PUBLI_STATUS: received Heartbeat Publication status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status Code
    uint8_t status;
    /// Destination address for Heartbeat messages
    uint16_t dst;
    /// Number of Heartbeat messages remaining to be sent
    uint8_t cnt_log;
    /// Period for sending Heartbeat messages
    uint8_t period_log;
    /// TTL to be used when sending Heartbeat messages
    uint8_t ttl;
    /// Bit field indicating features that trigger Heartbeat messages when changed
    uint16_t features;
    /// NetKey Index
    uint16_t net_key_id;
} m_api_confc_hb_publi_status_ind_t;

/// Inform application about configuration client model received Heartbeat Subscription status
typedef struct m_api_confc_hb_subs_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_HB_SUBS_STATUS: received Heartbeat Subscription status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status Code
    uint8_t status;
    /// Source address for Heartbeat messages
    uint16_t src;
    /// Destination address for Heartbeat messages
    uint16_t dst;
    /// Period for sending Heartbeat messages
    uint8_t period_log;
    /// Number of Heartbeat messages remaining to be sent
    uint8_t cnt_log;
    /// Minimum hops when receiving Heartbeat messages
    uint8_t min_hops;
    /// Maximum hops when receiving Heartbeat messages
    uint8_t max_hops;
} m_api_confc_hb_subs_status_ind_t;

/// Inform application about configuration client model received NetKey status
typedef struct m_api_confc_netkey_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_NETKEY_STATUS: received NetKey status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status code
    uint8_t status;
    /// Network Key index
    uint16_t net_key_id;
} m_api_confc_netkey_status_ind_t;

/// Inform application about configuration client model received AppKey List
typedef struct m_api_confc_appkey_list_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_APPKEY_LIST: received AppKey List
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status code
    uint8_t status;
    /// Network Key index
    uint16_t net_key_id;
    /// Number of Application Key indexes
    uint16_t nb;
    /// Application Key indexes
    uint16_t appkey_ids[__ARRAY_EMPTY];
} m_api_confc_appkey_list_ind_t;

/// Inform application about configuration client model received Node Identity Status
typedef struct m_api_confc_node_id_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_NODE_ID_STATUS: received Node Identity Status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status code
    uint8_t status;
    /// Network Key index
    uint16_t net_key_id;
    /// Node identity
    uint8_t node_id;
} m_api_confc_node_id_status_ind_t;

/// Inform application about configuration client model received Key Refresh Phase Status
typedef struct m_api_confc_key_refresh_phase_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_KEY_REFRESH_PHASE_STATUS: received Key Refresh Phase Status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status code
    uint8_t status;
    /// Network Key index
    uint16_t net_key_id;
    /// Key Refresh Phase
    uint8_t phase;
} m_api_confc_key_refresh_phase_status_ind_t;

/// Inform application about configuration client model received NetKey List
typedef struct m_api_confc_netkey_list_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_NETKEY_LIST: received NetKey List
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Number of Key indexes
    uint16_t nb;
    /// Key indexes
    uint16_t key_ids[__ARRAY_EMPTY];
} m_api_confc_netkey_list_ind_t;

/// Inform application about configuration client model received Application Key Status
typedef struct m_api_confc_appkey_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_APPKEY_STATUS: received Application Key Status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status code
    uint8_t status;
    /// Network Key index
    uint16_t net_key_id;
    /// Application Key index
    uint16_t app_key_id;
} m_api_confc_appkey_status_ind_t;

/// Inform application about configuration client model received Model Publication Status
typedef struct m_api_confc_mdl_pub_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_MDL_PUB_STATUS: received Model Publication Status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status code
    uint8_t status;
    /// Element address
    uint16_t elm_addr;
    /// Value of the publish address
    uint16_t pub_addr;
    /// Index of the application key
    uint16_t app_key_id;
    /// Value of the Friendship Credential Flag
    bool cred_flag;
    /// Default TTL value for the outgoing messages
    uint8_t pub_ttl;
    /// Period for periodic status publishing (in milliseconds)
    uint32_t pub_period_ms;
    /// Number of retransmissions for each published message
    uint8_t pub_retx_cnt;
    /// Number of 50-millisecond steps between retransmissions
    uint8_t pub_retx_intv_slots;
    /// SIG Model ID or Vendor Model ID
    uint32_t mdl_id;
} m_api_confc_mdl_pub_status_ind_t;

/// Inform application about configuration client model received Model Subscription List
typedef struct m_api_confc_mdl_subs_list_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_MDL_SUBS_LIST: received Model Subscription List
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status code
    uint8_t status;
    /// Element address
    uint16_t elm_addr;
    /// Model ID
    uint32_t mdl_id;
    /// Number of addresses
    uint16_t nb;
    /// Addresses
    uint16_t addrs[__ARRAY_EMPTY];
} m_api_confc_mdl_subs_list_ind_t;

/// Inform application about configuration client model received Model App List
typedef struct m_api_confc_mdl_app_list_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_MDL_APP_LIST: received Model App List
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status code
    uint8_t status;
    /// Element address
    uint16_t elm_addr;
    /// Vendor Model ID
    uint32_t mdl_id;
    /// Number of AppKey indexes
    uint16_t nb;
    /// AppKey indexes
    uint16_t appkey_ids[__ARRAY_EMPTY];
} m_api_confc_mdl_app_list_ind_t;

/// Inform application about configuration client model received Model Subscription status or Model App status
typedef struct m_api_confc_mdl_subs_app_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_MDL_SUBS_APP_STATUS: received Model Subscription/App Status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Status code
    uint8_t status;
    /// Element address
    uint16_t elm_addr;
    /// Model ID
    uint32_t mdl_id;
    /// Received value
    union
    {
        /// Group address
        uint16_t g_addr;
        /// AppKey index
        uint16_t appkey_id;
    } value;
} m_api_confc_mdl_subs_app_status_ind_t;

/// Inform application about configuration client model received Node Reset Status
typedef struct m_api_confc_node_reset_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_NODE_RESET_STATUS: received Node Reset Status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
} m_api_confc_node_reset_status_ind_t;

/// Inform application about configuration client model received LPN Poll Timeout Status
typedef struct m_api_confc_lpn_polltimeout_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_LPN_POLLTIMEOUT_STATUS: received LPN Poll Timeout Status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// The unicast address of the Low Power node
    uint16_t lpn_addr;
    /// The current value of the PollTimeout timer of the Low Power node
    uint32_t poll_timeout;
} m_api_confc_lpn_polltimeout_status_ind_t;

/// Inform application about configuration client model received Composition Data (Page > 0)
typedef struct m_api_confc_compo_data_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_COMPO_DATA: received Composition Data (Page > 0)
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Page number
    uint8_t page;
    /// Data length
    uint16_t data_len;
    /// Composition data
    uint8_t data[__ARRAY_EMPTY];
} m_api_confc_compo_data_ind_t;

/// Inform application about configuration client model received Composition Data (Page 0)
typedef struct m_api_confc_compo_data_page0_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_COMPO_DATA_PAGE0: received Composition Data (Page 0)
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Company identifier assigned by the Bluetooth SIG
    uint16_t cid;
    /// Vendor-assigned product identifier
    uint16_t pid;
    /// Vendor-assigned product version identifier
    uint16_t vid;
    /// The minimum number of replay protection list entries in a device
    uint16_t crpl;
    /// Device features
    uint16_t features;
    /// Number of elements
    uint8_t nb_elmts;
} m_api_confc_compo_data_page0_ind_t;

/// Inform application about configuration client model received Element field in Composition Data (Page 0)
typedef struct m_api_confc_compo_data_elmt_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_CONFC_COMPO_DATA_ELMT: received Element field in Composition Data (Page 0)
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Location descriptor
    uint16_t loc;
    /// Number of Models in this element
    uint8_t nb_mdls;
    /// Model identifiers sequence
    uint32_t mdl_ids[__ARRAY_EMPTY];
} m_api_confc_compo_data_elmt_ind_t;

/// @} MESH_PROFILE_CONFC_MSG

/// @addtogroup MESH_PROFILE_HLTHC_MSG
/// @{

/// Inform application about Health client model received 1 byte status value
typedef struct m_api_hlthc_value_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_HLTHC_VALUE: received value
    uint32_t ind_code;

    /// Received value code for Health Client model (see #m_fnd_hlthc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Received value
    union
    {
        /// Fast Period Divider
        uint8_t fast_period_div;
        /// Attention timer state
        uint8_t attention;
    } value;
} m_api_hlthc_value_ind_t;

/// Inform application about health client model received Current Status or Fault Status
typedef struct m_api_hlthc_cur_fault_status_ind
{
    /// Indication code (see #m_api_ind_code enumeration)
    /// M_API_HLTHC_CUR_FAULT_STATUS: received Current Status or Fault Status
    uint32_t ind_code;

    /// Received value code for Configuration Client model (see #m_fnd_confc_value_type enumeration)
    uint8_t value_code;
    /// Source address
    uint16_t src_addr;
    /// Identifier of a most recently performed test
    uint8_t test_id;
    /// Company Identifier
    uint16_t comp_id;
    /// Number of values in the Fault Array
    uint8_t nb_faults;
    /// Fault Array
    uint8_t faults[__ARRAY_EMPTY];
} m_api_hlthc_cur_fault_status_ind_t;

/// @} MESH_PROFILE_HLTHC_MSG

/// @addtogroup MESH_PROFILE_FWDC_MSG
/// @{


/// @} MESH_PROFILE_FWDC_MSG
/// @} MESH_PROFILE_MSG

/// @addtogroup MESH_MODEL_MSG
/// @{

/// @addtogroup MESH_MODEL_GENERAL_MSG
/// @{

/*
 * MESSAGE DEFINITIONS FOR MESH MODELS
 ****************************************************************************************
 */

/// Command codes for MESH_MDL_API_CMD and MESH_MDL_API_CMP_EVT messages
enum mm_api_cmd_code
{
    /// Register server model configuration
    MM_API_REGISTER_SERVER = 0,
    /// Register client model
    MM_API_REGISTER_CLIENT,
    /// Register Generic User/Admin/Manufacturer/Client Property Server models
    MM_API_REGISTER_SERVER_PROP,

    /// Get state value - for client models
    MM_API_CLI_GET = 10,
    /// Set state value - for client models
    ///     - Generic Default Transition Time
    ///     - Generic Power OnOff
    ///     - Generic Power Range
    ///     - Generic Power Last
    ///     - Generic Power Default
    ///     - Generic Battery
    MM_API_CLI_SET,
    /// Start transition to a new state - for client models
    ///     - Generic OnOff
    ///     - Generic Level
    ///     - Generic Power Actual
    MM_API_CLI_TRANSITION,
    /// Set Generic Location state value (global part) - for Generic Location Client models
    MM_API_CLI_SET_LOCG,
    /// Set Generic Location state value (local part) - for Generic Location Client models
    MM_API_CLI_SET_LOCL,
    /// Get a Generic Property state value or list of Generic Property states
    MM_API_CLI_GET_PROP,
    /// Set a Generic Property state value
    MM_API_CLI_SET_PROP,

    /// Create a group of models that are bound together
    MM_API_GRP_CREATE = 100,
    /// Initiate a new transition
    MM_API_GRP_TRANS_NEW,
    /// Set state of bound models
    MM_API_GRP_SET_STATE,
    /// Start a transition
    MM_API_GRP_TRANS_START,
    /// Get transition information
    MM_API_GRP_GET_INFO,

    /// Set current state
    MM_API_SRV_SET = 200,
};

/// Indication codes for MESH_MDL_API_IND message
enum mm_api_ind_code
{
    /// Model registered indication
    MM_API_REGISTER_IND = 0,

    /// Received state indication for:
    ///    - Generic OnOff Client model
    ///    - Generic Default Transition Time Client model
    ///    - Generic Level Client model
    ///    - Generic Power OnOff Client model
    MM_API_CLI_STATE_IND    = 10,
    /// Received Generic Battery state indication for Generic Battery Client model
    MM_API_CLI_BAT_IND,
    /// Received Generic Location state indication (global part) for Generic Location Client model
    MM_API_CLI_LOCG_IND,
    /// Received Generic Location state indication (local part) for Generic Location Client model
    MM_API_CLI_LOCL_IND,
    /// Get a Generic Property state value or list of Generic Property states
    MM_API_CLI_PROP_IND,
    /// Received list of User or Admin or Manufacturer properties
    MM_API_CLI_PROP_LIST_IND,

    /// State update indication
    MM_API_SRV_STATE_UPD_IND = 100,
    /// Set Generic Location state indication (global part) for Generic Location Server model
    MM_API_SRV_LOCG_UPD_IND,
    /// Set Generic Location state indication (local part) for Generic Location Server model
    MM_API_SRV_LOCL_UPD_IND,

    /// Group event indication
    MM_API_GRP_EVENT_IND = 200,
    /// State set by main model indication
    MM_API_GRP_STATE_IND,
};



/// Minimal content for MESH_MDL_API_CMD message
typedef struct mm_api_cmd
{
    /// Command Code (depends on indicated API)
    uint32_t cmd_code;
} mm_api_cmd_t;

/// Minimal content for MESH_MDL_API_CMP_EVT message
typedef struct mm_api_cmp_evt
{
    /// Command Code (depends on indicated API)
    uint32_t cmd_code;
    /// Status of command execution
    uint16_t status;
} mm_api_cmp_evt_t;

/// Minimal content for MESH_MDL_API_IND message
typedef struct mm_api_ind
{
    /// Indication code (depends on indicated API)
    uint32_t ind_code;
} mm_api_ind_t;

/// Minimal content for MESH_MDL_API_REQ_IND message
typedef struct mm_api_req_ind
{
    /// Request indication code (depends on indicated API)
    uint32_t req_ind_code;
} mm_api_req_ind_t;

/// Minimal content for MESH_MDL_API_CFM message
typedef struct mm_api_cfm
{
    /// Request indication code (depends on indicated API)
    uint32_t req_ind_code;
    /// Status of command execution by application
    uint16_t status;
} mm_api_cfm_t;

/// Register server model configuration command structure
typedef struct mm_api_register_server_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_REGISTER_SERVER: Register server model configuration
    uint32_t cmd_code;

    /// Index of element the model belongs to (zero based)
    uint8_t elmt_idx;
    /// Model configuration index (see #mm_cfg_idx enumeration)
    uint8_t mdl_cfg_idx;
    /// Informations
    uint8_t info;
} mm_api_register_server_cmd_t;

/// Register client model command structure
typedef struct mm_api_register_client_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_REGISTER_CLIENT: Register client model
    uint32_t cmd_code;

    /// Client Model index (see #mm_cmdl_idx enumeration)
    uint8_t cmdl_idx;
} mm_api_register_client_cmd_t;

/// Register Generic User/Manufacturer/Admin/Client Property Server model command structure
typedef struct mm_api_register_server_prop_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_REGISTER_SERVER_PROP: Register Generic User/Manufacturer/Admin/Client Server model
    uint32_t cmd_code;

    /// Index of element the model belongs to (zero based)
    uint8_t elmt_idx;
    /// Size of stored message queue
    uint8_t req_queue_len;
    /// Number of Generic User Properties
    uint8_t nb_prop_user;
    /// Number of Generic Admin Properties
    uint8_t nb_prop_admin;
    /// Number of Generic Manufacturer Properties
    uint8_t nb_prop_manuf;
    /// Number of Generic Client Properties
    uint8_t nb_prop_cli;
    /// Property information
    mm_prop_t prop_info[__ARRAY_EMPTY];
} mm_api_register_server_prop_cmd_t;

/// Model registered indication structure
typedef struct mm_api_register_ind
{
    /// Indication code (see #mm_api_ind_code enumeration)
    /// - MM_API_REGISTER_IND: Model registered
    uint32_t ind_code;

    /// Model ID
    uint32_t model_id;
    /// Model local index
    m_lid_t mdl_lid;
    /// Element index
    uint8_t elmt_idx;
} mm_api_register_ind_t;

/// @} MESH_MODEL_GENERAL_MSG

/// @addtogroup MESH_MODEL_CLIENT_MSG
/// @{

/// Get state command structure - for client model
typedef struct mm_api_cli_get_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_CLI_GET: Get state
    uint32_t cmd_code;

    /// Destination
    uint16_t dst;
    /// Get information
    uint16_t get_info;
    /// Client Model local index
    m_lid_t mdl_lid;
    /// Application key local index
    m_lid_t app_key_lid;
} mm_api_cli_get_cmd_t;

/// Set state command structure - for client model
typedef struct mm_api_cli_set_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_CLI_SET: Set state
    uint32_t cmd_code;

    /// State value 1
    uint32_t state_1;
    /// State value 2
    uint32_t state_2;
    /// Destination
    uint16_t dst;
    /// Set information (see #mm_set_info enumeration)
    uint16_t set_info;
    /// Client Model local index
    m_lid_t mdl_lid;
    /// Application key local index
    m_lid_t app_key_lid;
} mm_api_cli_set_cmd_t;

///  Start transition to a new state command structure - for client model
typedef struct mm_api_cli_transition_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_CLI_TRANSITION: Start transition to a new state
    uint32_t cmd_code;

    /// State value 1
    uint32_t state_1;
    /// State value 2
    uint32_t state_2;
    /// Transition time in milliseconds
    uint32_t trans_time_ms;
    /// Transition information (see #mm_trans_info enumeration)
    uint16_t trans_info;
    /// Destination
    uint16_t dst;
    /// Delay in milliseconds
    uint16_t delay_ms;
    /// Client Model local index
    m_lid_t mdl_lid;
    /// Application key local index
    m_lid_t app_key_lid;
} mm_api_cli_transition_cmd_t;

/// Set Generic Location state value (global part)
typedef struct mm_api_cli_set_locg_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_CLI_SET_LOCG: Set Generic Location state value (global part)
    uint32_t cmd_code;

    /// Destination
    uint16_t dst;
    /// Client Model local index
    m_lid_t mdl_lid;
    /// Application key local index
    m_lid_t app_key_lid;
    /// Set information (see #mm_set_info enumeration)
    uint8_t set_info;

    /// Global Latitude
    int32_t latitude;
    /// Global Longitude
    int32_t longitude;
    /// Global Altitude
    int16_t altitude;
} mm_api_cli_set_locg_cmd_t;

/// Set Generic Location state value (global part)
typedef struct mm_api_cli_set_locl_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_CLI_SET_LOCL: Set Generic Location state value (local part)
    uint32_t cmd_code;

    /// Destination
    uint16_t dst;
    /// Client Model local index
    m_lid_t mdl_lid;
    /// Application key local index
    m_lid_t app_key_lid;
    /// Set information (see #mm_set_info enumeration)
    uint8_t set_info;

    /// Local North
    int16_t north;
    /// Local East
    int16_t east;
    /// Local Altitude
    int16_t altitude;
    /// Uncertainty
    uint16_t uncertainty;
    /// Floor
    uint8_t floor;
} mm_api_cli_set_locl_cmd_t;

/// Get a Generic Property state value or list of Generic Property states
typedef struct mm_api_cli_get_prop_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_CLI_GET_PROP: Get a Generic Property state value or list of Generic Property states
    uint32_t cmd_code;

    /// Destination
    uint16_t dst;
    /// Property ID
    uint16_t prop_id;
    /// Client Model local index
    m_lid_t mdl_lid;
    /// Application key local index
    m_lid_t app_key_lid;
    /// Get type
    uint8_t get_type;
} mm_api_cli_get_prop_cmd_t;

/// Received state indication structure
typedef struct mm_api_cli_state_ind
{
    /// Indication code (see #mm_api_ind_code enumeration)
    /// - MM_API_CLI_STATE_IND: Received state
    uint32_t ind_code;

    /// Source address
    uint16_t src;
    /// State Identifier
    uint16_t state_id;
    /// State 1
    uint32_t state_1;
    /// State 2
    uint32_t state_2;
    /// Remaining time in milliseconds
    uint32_t rem_time_ms;
} mm_api_cli_state_ind_t;

/// Received Generic Battery state indication structure
typedef struct mm_api_cli_battery_ind
{
    /// Indication code (see #mm_api_ind_code enumeration)
    /// - MM_API_CLI_BAT_IND: Received Generic Battery State
    uint32_t ind_code;

    /// Time to discharge in minutes
    uint32_t time_discharge;
    /// Time to charge in minutes
    uint32_t time_charge;
    /// Source address
    uint16_t src;
    /// Battery level
    uint8_t bat_lvl;
    /// Flags
    uint8_t flags;
} mm_api_cli_battery_ind_t;

/// Received Generic Location state (global part) indication structure
typedef struct mm_api_cli_locg_ind
{
    /// Indication code (see #mm_api_ind_code enumeration)
    /// - MM_API_CLI_LOCG_IND: Received Generic Location state indication (global part)
    uint32_t ind_code;

    /// Global Latitude
    int32_t latitude;
    /// Global Longitude
    int32_t longitude;
    /// Global Altitude
    int16_t altitude;

    /// Source address
    uint16_t src;
} mm_api_cli_locg_ind_t;

/// Received Generic Location state (local part) indication structure
typedef struct mm_api_cli_locl_ind
{
    /// Indication code (see #mm_api_ind_code enumeration)
    /// - MM_API_CLI_LOCL_IND: Received Generic Location state indication (local part)
    uint32_t ind_code;

    /// Source address
    uint16_t src;

    /// Local North
    int16_t north;
    /// Local East
    int16_t east;
    /// Local Altitude
    int16_t altitude;
    /// Uncertainty
    uint16_t uncertainty;
    /// Floor
    uint8_t floor;
} mm_api_cli_locl_ind_t;

/// Received User or Admin or Manufacturer Property value indication structure
typedef struct mm_api_cli_prop_ind
{
    /// Indication code (see #mm_api_ind_code enumeration)
    /// - MM_API_CLI_PROP_IND: Received User or Admin or Manufacturer Property value
    uint32_t ind_code;

    /// Source address
    uint16_t src;

    /// Property ID
    uint16_t prop_id;
    /// Length
    uint16_t length;
    /// User access
    uint8_t user_access;
    /// Value
    uint8_t val[__ARRAY_EMPTY];
} mm_api_cli_prop_ind_t;

/// Received list of User or Admin or Manufacturer properties indication structure
typedef struct mm_api_cli_prop_list_ind
{
    /// Indication code (see #mm_api_ind_code enumeration)
    /// - MM_API_CLI_PROP_LIST_IND: Received list of User or Admin or Manufacturer properties
    uint32_t ind_code;

    /// Source address
    uint16_t src;

    /// Number of properties
    uint16_t nb_prop;
    /// Property type
    uint8_t prop_type;
    /// Property IDs
    uint16_t prop_ids[__ARRAY_EMPTY];
} mm_api_cli_prop_list_ind_t;

/// Set a Generic Property state value
typedef struct mm_api_cli_set_prop_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_CLI_SET_PROP: Get a Generic Property state value or list of Generic Property states
    uint32_t cmd_code;

    /// Destination
    uint16_t dst;
    /// State value length
    uint16_t length;
    /// Property ID
    uint16_t prop_id;
    /// Client Model local index
    m_lid_t mdl_lid;
    /// Application key local index
    m_lid_t app_key_lid;
    /// Set information
    uint8_t set_info;
    /// User access
    uint8_t user_access;
    /// State value
    uint8_t val[__ARRAY_EMPTY];
} mm_api_cli_set_prop_cmd_t;

/// @} MESH_MODEL_CLIENT_MSG

/// @addtogroup MESH_MODEL_SERVER_MSG
/// @{

/// Set state value
typedef struct mm_api_srv_set_cmd
{
    /// Command code (see #mm_api_cmd_code enumeration)
    /// - MM_API_SRV_SET: Set state value
    uint32_t cmd_code;

    /// State value
    uint32_t state;
    /// State identifier
    uint16_t state_id;
    /// Model local index
    m_lid_t mdl_lid;
} mm_api_srv_set_cmd_t;

/// State update indication structure
typedef struct mm_api_srv_state_upd_ind
{
    /// Indication code (see #mm_api_ind_code enumeration)
    /// - MM_API_SRV_STATE_UPD_IND: Inform application about state update
    uint32_t ind_code;

    /// New state value or targeted state value depending on transition time
    uint32_t state;
    /// Transition time in milliseconds
    uint32_t trans_time_ms;
    /// State identifier (see #mm_state_idx))
    uint16_t state_id;
    /// Element index
    uint8_t elmt_idx;
} mm_api_srv_state_upd_ind_t;

/// Set Generic Location state (global part) indication structure
typedef struct mm_api_srv_locg_upd_ind
{
    /// Indication code (see #mm_api_ind_code enumeration)
    /// - MM_API_SRV_LOCG_UPD_IND: Set Generic Location state indication (global part)
    uint32_t ind_code;

    /// Global Latitude
    int32_t latitude;
    /// Global Longitude
    int32_t longitude;
    /// Global Altitude
    int16_t altitude;

    /// Element index
    uint8_t elmt_idx;
} mm_api_srv_locg_upd_ind_t;

/// Set Generic Location state (local part) indication structure
typedef struct mm_api_srv_locl_upd_ind
{
    /// Indication code (see #mm_api_ind_code enumeration)
    /// - MM_API_SRV_LOCL_UPD_IND: Set Generic Location state indication (local part)
    uint32_t ind_code;

    /// Local North
    int16_t north;
    /// Local East
    int16_t east;
    /// Local Altitude
    int16_t altitude;
    /// Uncertainty
    uint16_t uncertainty;
    /// Floor
    uint8_t floor;

    /// Element index
    uint8_t elmt_idx;
} mm_api_srv_locl_upd_ind_t;

/// Get element state value request indication structure
typedef struct mm_api_srv_state_req_ind
{
    /// Request indication code (see #mm_req_ind_code enumeration)
    uint32_t req_ind_code;

    /// Element index
    uint8_t elmt_idx;
} mm_api_srv_state_req_ind_t;

/// Request to get Generic Property value
typedef struct mm_api_srv_prop_get_req_ind
{
    /// Request indication code
    /// MM_API_SRV_PROP_GET_REQ_IND : Get Generic Property value
    uint32_t req_ind_code;

    /// Element index
    uint8_t elmt_idx;
    /// Property type
    uint8_t prop_type;
    /// Property ID
    uint16_t prop_id;
} mm_api_srv_prop_get_req_ind_t;

/// Request to set Generic Property value
typedef struct mm_api_srv_prop_set_req_ind
{
    /// Request indication code
    /// MM_API_SRV_PROP_SET_REQ_IND : Set Generic Property value
    uint32_t req_ind_code;

    /// Element index
    uint8_t elmt_idx;
    /// Property type
    uint8_t prop_type;
    /// Property ID
    uint16_t prop_id;
    /// Value length
    uint16_t length;
    /// Value
    uint8_t val[__ARRAY_EMPTY];
} mm_api_srv_prop_set_req_ind_t;

/// Message sent by application to return a requested Generic Battery state for an element
typedef struct mm_api_srv_bat_cfm
{
    /// Request indication code (see #mm_req_ind_code enumeration)
    /// MM_API_SRV_BAT_REQ_IND: Generic Battery state
    uint32_t req_ind_code;
    /// Status
    uint16_t status;

    /// Battery level
    uint8_t bat_lvl;
    /// Flags
    uint8_t flags;
    /// Time to discharge
    uint32_t time_discharge;
    /// Time to charge
    uint32_t time_charge;
    /// Element index
    uint8_t elmt_idx;
} mm_api_srv_bat_cfm_t;

/// Message sent by application to return a requested Generic Location state for an element (global part)
typedef struct mm_api_srv_locg_cfm
{
    /// Request indication code (see #mm_req_ind_code enumeration)
    /// MM_API_SRV_LOCG_REQ_IND: Generic Location state (global part)
    uint32_t req_ind_code;
    /// Status
    uint16_t status;

    /// Global Altitude
    uint16_t altitude;
    /// Global Latitude
    int32_t latitude;
    /// Global Longitude
    int32_t longitude;
    /// Element index
    uint8_t elmt_idx;
} mm_api_srv_locg_cfm_t;

/// Message sent by application to return a requested Generic Location state for an element (local part)
typedef struct mm_api_srv_locl_cfm
{
    /// Request indication code (see #mm_req_ind_code enumeration)
    /// MM_API_SRV_LOCL_REQ_IND: Generic Location state (local part)
    uint32_t req_ind_code;
    /// Status
    uint16_t status;

    /// Local North
    int16_t north;
    /// Local East
    int16_t east;
    /// Local Altitude
    int16_t altitude;
    /// Uncertainty
    uint16_t uncertainty;
    /// Floor
    uint8_t floor;
    /// Element index
    uint8_t elmt_idx;
} mm_api_srv_locl_cfm_t;

/// Confirmation for get/set Generic Property value request
typedef struct mm_api_srv_prop_cfm
{
    /// Request indication code
    /// MM_API_SRV_PROP_GET : Get Generic Property value
    /// MM_API_SRV_PROP_SET : Set Generic Property value
    uint32_t req_ind_code;
    /// Status
    uint16_t status;

    /// Element index
    uint8_t elmt_idx;
    /// Property type
    uint8_t prop_type;
    /// Property ID
    uint16_t prop_id;
    /// Value length
    uint16_t length;
    /// Value
    uint8_t val[__ARRAY_EMPTY];
} mm_api_srv_prop_cfm_t;

/// @} MESH_MODEL_SERVER_MSG
/// @} MESH_MODEL_MSG
/// @} MESH_API_MSG

#endif /* MESH_API_MSG_ */
