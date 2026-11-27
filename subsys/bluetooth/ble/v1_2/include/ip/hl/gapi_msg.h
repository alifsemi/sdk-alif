/**
 ****************************************************************************************
 *
 * @file gapi_msg.h
 *
 * @brief GAP Isochronous - Definition of Kernel Messages
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPI_MSG_H_
#define GAPI_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GAPI_MSG Message API
 * @ingroup GAPI
 * @brief Description of Message API for GAP Isochronous block
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapi.h"            // GAPI Definitions
#include "rwip_task.h"       // RW Task Definitions

/// @addtogroup GAPI_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// GAP Isochronous Message Interface
enum gapi_msg_ids
{
    /* Default event */
    /// Command
    GAPI_CMD = MSG_ID(GAPI, 0x00u),
    /// Command Complete event
    GAPI_CMP_EVT = MSG_ID(GAPI, 0x01u),
    /// Request
    GAPI_REQ = MSG_ID(GAPI, 0x02u),
    /// Response
    GAPI_RSP = MSG_ID(GAPI, 0x03u),
    /// Indication
    GAPI_IND = MSG_ID(GAPI, 0x04u),
    /// Request indication
    GAPI_REQ_IND = MSG_ID(GAPI, 0x05u),
    /// Confurmation
    GAPI_CFM = MSG_ID(GAPI, 0x06u),
};

/// List of GAPI_REQ request code values for GAP Isochronous block
enum gapi_msg_req_code
{
    /// Common - Register Interface
    GAPI_INTF_REGISTER = GAPI_CODE(COMMON, 0u),

    /// Unicast - Add Group
    GAPI_UG_ADD = GAPI_CODE(UG, 0u),
    /// Unicast - Add Group with Test Parameters
    GAPI_UG_TEST_ADD = GAPI_CODE(UG, 1u),
    /// Unicast - Configure Stream
    GAPI_US_CONFIG = GAPI_CODE(UG, 2u),
    /// Unicast - Configure Stream with Test Parameters
    GAPI_US_TEST_CONFIG = GAPI_CODE(UG, 3u),
    /// Unicast - Bind Stream with a Connection
    GAPI_US_BIND = GAPI_CODE(UG, 5u),
    /// Unicast - Release Stream
    GAPI_US_RELEASE = GAPI_CODE(UG, 6u),

    /// Broadcast - Add Group
    GAPI_BG_ADD = GAPI_CODE(BG, 0u),
    /// Broadcast - Add Group with Test Parameters
    GAPI_BG_TEST_ADD = GAPI_CODE(BG, 1u),

    /// Data Path - Enable/disable ISO over HCI
    GAPI_DP_ENABLE_ISOOHCI = GAPI_CODE(DP, 0u),
    /// Data Path - Setup
    GAPI_DP_SETUP = GAPI_CODE(DP, 1u),
    /// Data Path - Remove
    GAPI_DP_REMOVE = GAPI_CODE(DP, 2u),

    /// Debug - Get Features
    GAPI_DBG_FEATURES = GAPI_CODE(DBG, 0u),
};

/// List of GAPI_IND indication codes
enum gapi_msg_ind_code
{
    /// Common - Unknown message
    GAPI_UNKNOWN_MSG = GAPI_CODE(COMMON, 0u),

    /// Unicast - Stream Enabled
    GAPI_US_ENABLED = GAPI_CODE(UG, 0u),
    /// Unicast - Stream Disabled
    GAPI_US_DISABLED = GAPI_CODE(UG, 1u),

    /// Broadcast - Synchronization Status
    GAPI_BG_SYNC_STATUS = GAPI_CODE(BG, 0u),
    /// Broadcast - Group Added for synchronization
    GAPI_BG_SYNC_ADDED = GAPI_CODE(BG, 1u),
    /// Broadcast - Group Created
    GAPI_BG_CREATED = GAPI_CODE(BG, 2u),

    /// Data Path - Update
    GAPI_DP_UPDATE = GAPI_CODE(DP, 0u),
    /// Data Path - ISO data received
    GAPI_DP_DATA_RX = GAPI_CODE(DP, 1u),

    #if (GAPI_TEST)
    /// Test Mode - Counters
    GAPI_TM_CNT = GAPI_CODE(TM, 0u),
    #endif // (GAPI_TEST)

    #if (AUDIO_SYNC_SUPPORT && BLE_BIS)
    /// AUSY mode
    GAPI_AUSY_ESTAB_BIS = GAPI_CODE(AUSY, 0u),
    #endif // (AUDIO_SYNC_SUPPORT && BLE_BIS)
};

/// List of GAPI_REQ_IND request indication codes
enum gapi_msg_req_ind_code
{
    /// Unicast - Establish Stream Request
    GAPI_US_ESTABLISH = GAPI_CODE(UG, 0u),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Basic structure for GAPI_CMD message
typedef struct
{
    /// Command code
    uint16_t cmd_code;
} gapi_cmd_t;

/// Structure for #GAPI_CMP_EVT message
typedef struct
{
    /// Command code (see #gapi_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Group local index
    uint8_t group_lid;
    /// Stream local index
    uint8_t stream_lid;
} gapi_cmp_evt_t;

/// Basic structure for GAF_REQ message
typedef struct
{
    /// Request code
    uint16_t req_code;
} gapi_req_t;

/// Structure for #GAPI_RSP message
typedef struct
{
    /// Request code (see #gapi_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
} gapi_rsp_t;

/// Basic structure for GAPI_IND message
typedef struct
{
    /// Indication code
    uint16_t ind_code;
} gapi_ind_t;

/// Basic structure for GAPI_REQ_IND message
typedef struct
{
    /// Request indication code
    uint16_t req_ind_code;
} gapi_req_ind_t;

/// Basic structure for GAPI_CFM message
typedef struct
{
    /// Request indication code
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
} gapi_cfm_t;

/// Structure for #GAPI_UG_ADD request message
typedef struct
{
    /// Request code (shall be set to #GAPI_UG_ADD)
    uint16_t req_code;
    /// Unicast group information
    gapi_ug_param_t group_info;
    /// CIG ID
    uint8_t cig_id;
} gapi_ug_add_req_t;

/// Structure for #GAPI_UG_ADD response message
typedef struct
{
    /// Request code (shall be set to #GAPI_UG_ADD)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// CIG ID
    uint8_t cig_id;
    /// Allocated Group local index
    uint8_t group_lid;
} gapi_ug_add_rsp_t;

/// Structure for #GAPI_US_CONFIG request message
typedef struct
{
    /// Request code (shall be set to #GAPI_US_CONFIG)
    uint16_t req_code;
    /// Group local index
    uint8_t group_lid;
    /// CIS IS
    uint8_t cis_id;
    /// Unicast stream information
    gapi_us_param_t stream_info;
} gapi_us_config_req_t;

/// Structure for #GAPI_US_CONFIG response message
typedef struct
{
    /// Request code (shall be set to #GAPI_US_CONFIG)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Group local index
    uint8_t group_lid;
    /// CIS IS
    uint8_t cis_id;
    /// Allocated Stream local index
    uint8_t stream_lid;
} gapi_us_config_rsp_t;

/// Structure for #GAPI_US_ESTABLISH request indication message
typedef struct
{
    /// Request indication code (shall be set to #GAPI_US_ESTABLISH)
    uint16_t req_ind_code;
    /// CIG ID
    uint8_t cig_id;
    /// CIS ID
    uint8_t cis_id;
    /// Connection index
    uint8_t conidx;
    /// Group local index
    uint8_t group_lid;
    /// Stream local index
    uint8_t stream_lid;
} gapi_us_establish_req_ind_t;

/// Structure for #GAPI_US_ESTABLISH confirmation message
typedef struct
{
    /// Request indication code (shall be set to #GAPI_US_ESTABLISH)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Stream local index
    uint8_t stream_lid;
} gapi_us_establish_cfm_t;

/// Structure for #GAPI_US_BIND request message
typedef struct
{
    /// Request code (shall be set to #GAPI_US_BIND)
    uint16_t req_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Connection index
    uint8_t conidx;
} gapi_us_bind_req_t;

/// Structure for #GAPI_UG_UPDATE command message
typedef struct
{
    /// Command code (shall be set to #GAPI_UG_UPDATE)
    uint16_t cmd_code;
    /// Group local index
    uint8_t group_lid;
} gapi_ug_update_cmd_t;

/// Structure for #GAPI_US_ENABLE command message
typedef struct
{
    /// Command code (shall be set to #GAPI_US_ENABLE)
    uint16_t cmd_code;
    /// Group local index
    uint8_t group_lid;
} gapi_us_enable_cmd_t;

/// Structure for #GAPI_US_DISABLE command message
typedef struct
{
    /// Command code (shall be set to #GAPI_US_DISABLE)
    uint16_t cmd_code;
    /// Stream local index
    uint8_t stream_lid;
} gapi_us_disable_cmd_t;

/// Structure for #GAPI_US_RELEASE request message
typedef struct
{
    /// Request code (shall be set to #GAPI_US_RELEASE)
    uint16_t req_code;
    /// Stream local index
    uint8_t stream_lid;
} gapi_us_release_req_t;

/// Structure for #GAPI_UG_REMOVE command message
typedef struct
{
    /// Command code (shall be set to #GAPI_UG_REMOVE)
    uint16_t cmd_code;
    /// Group local index
    uint8_t group_lid;
} gapi_ug_remove_cmd_t;

/// Structure for #GAPI_BG_ADD request message
typedef struct
{
    /// Request code (shall be set to #GAPI_BG_ADD)
    uint16_t req_code;
    /// Group information
    gapi_bg_param_t group_info;
    /// BIG Handle
    uint8_t big_handle;
    /// Number of streams in the group
    uint8_t nb_streams;
} gapi_bg_add_req_t;

/// Structure for #GAPI_BG_ADD response message
typedef struct
{
    /// Request code (shall be set to #GAPI_BG_ADD)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// BIG Handle
    uint8_t big_handle;
    /// Number of streams in the group
    uint8_t nb_streams;
    /// Allocated group local index
    uint8_t group_lid;
    /// List of allocated stream local indexes
    uint8_t stream_lids[__ARRAY_EMPTY];
} gapi_bg_add_rsp_t;

/// Structure for #GAPI_BG_ENABLE command message
typedef struct
{
    /// Command code (shall be set to #GAPI_BG_ENABLE)
    uint16_t cmd_code;
    /// Group local index
    uint8_t group_lid;
    /// Advertising activity local index
    uint8_t adv_actv_lid;
    /// Indicate if streams are encrypted (!=0) or not (=0)
    uint8_t encrypted;
    /// Code used to generate the encryption key for encrypting payloads of all streams
    uint8_t broadcast_code[16];
} gapi_bg_enable_cmd_t;

/// Structure for #GAPI_BG_SYNC command message
typedef struct
{
    /// Command code (shall be set to #GAPI_BG_SYNC)
    uint16_t cmd_code;
    /// Synchronization timeout in multiple of 10ms
    uint16_t sync_timeout_10ms;
    /// BIG handle
    uint8_t big_handle;
    /// Synchronization activity local index
    uint8_t sync_actv_lid;
    /// Maximum number of subevents the controller should use to receive data payloads in each interval
    uint8_t mse;
    /// Indicate if streams are encrypted (!=0) or not (=0)
    uint8_t encrypted;
    /// Code used to generate the encryption key for encrypting payloads of all streams
    uint8_t broadcast_code[16];
    /// Stream position bit field to synchronize with.
    uint32_t stream_pos_bf;
} gapi_bg_sync_cmd_t;

/// Structure for #GAPI_BG_SYNC_ADDED indication message
typedef struct
{
    /// Indication code (shall be set to #GAPI_BG_SYNC_ADDED)
    uint16_t ind_code;
    /// BIG handle
    uint8_t big_handle;
    /// Number of streams to synchronize with
    uint8_t nb_streams;
    /// Group local index
    uint8_t group_lid;
    /// List of stream local indexes
    uint8_t stream_lids[__ARRAY_EMPTY];
} gapi_bg_sync_added_ind_t;

/// Structure for #GAPI_BG_DISABLE command message
typedef struct
{
    /// Command code (shall be set to #GAPI_BG_DISABLE)
    uint16_t cmd_code;
    /// Group local index
    uint8_t group_lid;
} gapi_bg_disable_cmd_t;

/// Structure for #GAPI_BG_REMOVE command message
typedef struct
{
    /// Command code (shall be set to #GAPI_BG_REMOVE)
    uint16_t cmd_code;
    /// Group local index
    uint8_t group_lid;
} gapi_bg_remove_cmd_t;

/// Structure for #GAPI_UG_TEST_ADD request message
typedef struct
{
    /// Request code (shall be set to #GAPI_UG_TEST_ADD)
    uint16_t req_code;
    /// Unicast group test information structure
    gapi_ug_test_param_t group_info;
    /// CIG ID
    uint8_t cig_id;
} gapi_ug_test_add_req_t;

/// Structure for #GAPI_US_TEST_CONFIG request message
typedef struct
{
    /// Request code (shall be set to #GAPI_US_TEST_CONFIG)
    uint16_t req_code;
    /// Group local index
    uint8_t group_lid;
    /// CIS IS
    uint8_t cis_id;
    /// Unicast stream information
    gapi_us_test_param_t stream_info;
} gapi_us_test_config_req_t;

/// Structure for #GAPI_BG_TEST_ADD request message
typedef struct
{
    /// Request code (shall be set to #GAPI_BG_TEST_ADD)
    uint16_t req_code;
    /// Broadcast group test information structure
    gapi_bg_test_param_t group_info;
    /// BIG Handle
    uint8_t big_handle;
    /// Number of streams in the group
    uint8_t nb_streams;
    /// Indicate if streams are encrypted (!=0) or not (=0)
    uint8_t encrypted;
    /// Code used to generate encryption key used to encrypt payloads
    uint8_t broadcast_code[16];
} gapi_bg_test_add_req_t;

/// Structure for #GAPI_DP_ENABLE_ISOOHCI request message
typedef struct
{
    /// Request code (shall be set to #GAPI_DP_ENABLE_ISOOHCI)
    uint16_t req_code;
    /// Enable or disable ?
    bool enable;
} gapi_dp_enable_isoohci_req_t;

/// Structure for #GAPI_DP_SETUP request message
typedef struct
{
    /// Request code (shall be set to #GAPI_DP_SETUP)
    uint16_t req_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Direction
    uint8_t direction;
    /// Data path ID (see #data_path_id enumeration)
    uint8_t dp_id;
    /// Controller Delay in microseconds
    uint32_t ctl_delay_us;
    /// Codec ID (see #gapi_codec_format enumeration for Codec ID first byte values)
    uint8_t codec_id[CODEC_ID_LEN];
    /// Codec Configuration value\n
    /// Meaningful only if first byte of Codec ID is not #GAPI_CODEC_FORMAT_TRANSPARENT
    gapi_ltv_t codec_cfg;
} gapi_dp_setup_req_t;

/// Structure for #GAPI_DP_REMOVE request message
typedef struct
{
    /// Request code (shall be set to #GAPI_DP_REMOVE)
    uint16_t req_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Direction bit field
    uint8_t direction_bf;
} gapi_dp_remove_req_t;

#if (GAPI_TEST)
/// Structure for #GAPI_TM_START command message
typedef struct
{
    /// Command code (shall be set to #GAPI_TM_START)
    uint16_t cmd_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Transmit (!=0) or receive (=0) test payload
    uint8_t transmit;
    /// Payload type
    uint8_t payload_type;
} gapi_tm_start_cmd_t;

/// Structure for #GAPI_TM_CNT_GET command message
typedef struct
{
    /// Command code (shall be set to #GAPI_TM_CNT_GET)
    uint16_t cmd_code;
    /// Stream local index
    uint8_t stream_lid;
} gapi_tm_cnt_get_cmd_t;

/// Structure for #GAPI_TM_STOP command message
typedef struct
{
    /// Command code (shall be set to #GAPI_TM_STOP)
    uint16_t cmd_code;
    /// Stream local index
    uint8_t stream_lid;
} gapi_tm_stop_cmd_t;
#endif // (GAPI_TEST)

/// Structure for #GAPI_GET_QUALITY command  message
typedef struct
{
    /// Command code (shall be set to #GAPI_GET_QUALITY)
    uint16_t cmd_code;
    /// Stream local index
    uint8_t stream_lid;
} gapi_get_quality_cmd_t;

/// Structure for #GAPI_GET_QUALITY command complete event message
typedef struct
{
    /// Command code (shall be set to #GAPI_GET_QUALITY)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Stream local index
    uint8_t stream_lid;
    /// Number of unacked transmitted packets
    uint32_t tx_unacked_packets;
    /// Number of flushed transmitted packets
    uint32_t tx_flushed_packets;
    /// Number of packets transmitted during last subevent
    uint32_t tx_last_subevent_packets;
    /// Number of retransmitted packets
    uint32_t retransmitted_packets;
    /// Number of packets received with a CRC error
    uint32_t crc_error_packets;
    /// Number of unreceived packets
    uint32_t rx_unreceived_packets;
    /// Number of duplicate packets received
    uint32_t duplicate_packets;
} gapi_get_quality_cmp_evt_t;

/// Structure for #GAPI_INTF_REGISTER command complete event message
typedef struct
{
    /// Request code (shall be set to #GAPI_INTF_REGISTER)
    uint16_t req_code;
    /// Is Unicast supported ?
    uint8_t unicast_supported;
    /// Is Broadcast supported ?
    uint8_t broadcast_supported;
} gapi_register_req_t;

/// Structure for #GAPI_US_DISABLED indication message
typedef struct
{
    /// Indication code (shall be set to #GAPI_US_DISABLED)
    uint16_t ind_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Reason
    uint8_t reason;
} gapi_us_disabled_ind_t;

/// Structure for #GAPI_US_ENABLED indication message
typedef struct
{
    /// Indication code (shall be set to #GAPI_US_ENABLED)
    uint16_t ind_code;
    /// Group local index
    uint8_t group_lid;
    /// Stream local index
    uint8_t stream_lid;
    /// Group configuration
    gapi_ug_config_t ug_config;
    /// Stream configuration
    gapi_us_config_t us_config;
} gapi_us_enabled_ind_t;

/// Structure for #GAPI_BG_SYNC_STATUS indication message
typedef struct
{
    /// Indication code (shall be set to #GAPI_BG_SYNC_STATUS)
    uint16_t ind_code;
    /// Group local index
    uint8_t group_lid;
    /// Status (see #gapi_bg_sync_status enumeration)
    uint8_t status;
    /// Group configuration
    /// Meaningful only if Group status is #GAPI_BG_SYNC_STATUS_ESTABLISHED
    gapi_bg_sync_config_t config;
    /// Number of BISes
    /// Meaningful only if Group status is #GAPI_BG_SYNC_STATUS_ESTABLISHED
    uint8_t nb_bis;
    /// List of Connection Handle values provided by the Controller (nb_bis elements)
    /// Meaningful only if Group status is #GAPI_BG_SYNC_STATUS_ESTABLISHED state
    uint16_t conhdl[__ARRAY_EMPTY];
} gapi_bg_sync_status_ind_t;

/// Structure for #GAPI_BG_CREATED indication message
typedef struct
{
    /// Indication code (shall be set to #GAPI_BG_CREATED)
    uint16_t ind_code;
    /// Group local index
    uint8_t group_lid;
    /// Group configuration
    /// Meaningful only if Group status is #GAPI_BG_SYNC_STATUS_ESTABLISHED
    gapi_bg_config_t config;
    /// Number of BISes
    /// Meaningful only if Group status is #GAPI_BG_SYNC_STATUS_ESTABLISHED
    uint8_t nb_bis;
    /// List of Connection Handle values provided by the Controller (nb_bis elements)
    /// Meaningful only if Group status is #GAPI_BG_SYNC_STATUS_ESTABLISHED state
    uint16_t conhdl[__ARRAY_EMPTY];
} gapi_bg_created_ind_t;

/// Structure for #GAPI_DP_UPDATE indication message
typedef struct
{
    /// Indication code (shall be set to #GAPI_DP_UPDATE)
    uint16_t ind_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Data path update type
    uint8_t dp_update;
    /// Direction for setup update, direction bit field for remove update
    uint8_t direction;
    /// Status
    uint16_t status;
} gapi_dp_update_ind_t;

/// Structure for #GAPI_DP_DATA_RX indication message
typedef struct
{
    /// Indication code (shall be set to #GAPI_DP_DATA_RX)
    uint16_t ind_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Packet status
    uint8_t packet_status;
    /// Timestamp
    uint32_t timestamp;
    /// Sequence number
    uint16_t seq_nb;
    /// SDU length
    uint16_t sdu_length;
    /// SDU
    uint8_t sdu[__ARRAY_EMPTY];
} gapi_dp_data_rx_ind_t;

#if (GAPI_TEST)
/// Structure for #GAPI_TM_CNT indication message
typedef struct
{
    /// Indication code (shall be set to #GAPI_TM_CNT)
    uint16_t ind_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Number of received packets
    uint32_t nb_rx;
    /// Number of missed packets
    uint32_t nb_missed;
    /// Number of failed packets
    uint32_t nb_failed;
} gapi_tm_cnt_ind_t;
#endif // (GAPI_TEST)

/// Structure for #GAPI_DBG_FEATURES response message
typedef struct
{
    /// Request code (shall be set to #GAPI_DBG_FEATURES)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Number of streams supported
    uint8_t nb_streams;
    /// Unicast supported
    uint8_t unicast;
    /// Broadcast supported
    uint8_t broadcast;
} gapi_dbg_features_rsp_t;

/// Structure for #GAPI_UNKNOWN_MSG indication message
typedef struct
{
    /// Indication code (shall be set to #GAPI_UNKNOWN_MSG)
    uint16_t ind_code;
    /// Message ID
    uint16_t msg_id;
} gapi_unknown_msg_ind_t;

/// @} GAPI_MSG

#endif // GAPI_MSG_H_
