/**
 ****************************************************************************************
 *
 * @file bap_bc_sink_msg.h
 *
 * @brief Basic Audio Profile - Broadcast Sink - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_SINK_MSG_H_
#define BAP_BC_SINK_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SINK_MSG Message API
 * @ingroup BAP_BC_SINK
 * @brief Description of Message API for Broadcast Sink module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap_bc_sink.h"     // Basic Audio Profile - Broadcast Sink Definitions
#include "bap_msg.h"         // Message API Definitions

/// @addtogroup BAP_BC_SINK_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command codes for Basic Audio Profile - Broadcast Sink
enum bap_bc_sink_msg_cmd_code
{
    /// Enable a Broadcast Group Sink - it creates a corresponding Broadcast group (see #bap_bc_sink_enable_cmd_t)
    BAP_BC_SINK_ENABLE = GAF_CODE(BAP, BC_SINK, BAP_BC_SINK_CMD_TYPE_ENABLE),
    /// Disable a Broadcast Group Sink - it removes Broadcast group and included streams
    /// (see #bap_bc_sink_disable_cmd_t)
    BAP_BC_SINK_DISABLE = GAF_CODE(BAP, BC_SINK, BAP_BC_SINK_CMD_TYPE_DISABLE),
    /// Start reception of audio data onto a Broadcast stream (see #bap_bc_sink_start_streaming_cmd_t)
    BAP_BC_SINK_START_STREAMING = GAF_CODE(BAP, BC_SINK, BAP_BC_SINK_CMD_TYPE_START_STREAMING),
    /// Stop reception of audio data onto a Broadcast stream (see #bap_bc_sink_stop_streaming_cmd_t)
    BAP_BC_SINK_STOP_STREAMING = GAF_CODE(BAP, BC_SINK, BAP_BC_SINK_CMD_TYPE_STOP_STREAMING),
    /// Get reception quality for a Broadcast stream (see #bap_bc_sink_get_quality_cmd_t)
    BAP_BC_SINK_GET_QUALITY = GAF_CODE(BAP, BC_SINK, BAP_BC_SINK_CMD_TYPE_GET_QUALITY),
};

/// List of GAF_REQ_IND request indication codes for Basic Audio Profile - Broadcast Sink
enum bap_bc_sink_msg_req_ind_code
{
    /// Request to enable a Broadcast Group Sink - it creates a corresponding Broadcast group
    /// (see #bap_bc_sink_enable_req_ind_t)
    BAP_BC_SINK_ENABLE_RI = GAF_CODE(BAP, BC_SINK, 0),
    /// Request to disable a Broadcast Group Sink - it removes Broadcast group and included streams
    /// (see #bap_bc_sink_disable_req_ind_t)
    BAP_BC_SINK_DISABLE_RI = GAF_CODE(BAP, BC_SINK, 1),
};

/// List of GAF_IND request indication codes for Basic Audio Profile - Broadcast Sink
enum bap_bc_sink_msg_ind_code
{
    /// Inform upper layer about status of synchronization with a Broadcast Group (see #bap_bc_sink_status_ind_t)
    BAP_BC_SINK_STATUS = GAF_CODE(BAP, BC_SINK, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for command complete message
typedef struct bap_bc_sink_cmp_evt
{
    /// Command code (see #bap_bc_sink_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status (see #gaf_err enumeration)
    uint16_t status;
    /// Group local index
    uint8_t grp_lid;
    /// Stream position. Meaningful only for BAP_BC_SINK_START_STREAMING and BAP_BC_SINK_STOP_STREAMING commands
    uint8_t stream_pos;
} bap_bc_sink_cmp_evt_t;

/// Structure for #BAP_BC_SINK_GET_QUALITY command complete event message
typedef struct bap_bc_sink_get_quality_cmp_evt
{
    /// Command code (see #bap_bc_sink_msg_cmd_code enumeration)
    ///  - BAP_BC_SINK_GET_QUALITY
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Group local index
    uint8_t grp_lid;
    /// Stream position
    uint8_t stream_pos;
    /// Number of packets received with a CRC error
    uint32_t crc_error_packets;
    /// Number of unreceived packets
    uint32_t rx_unrx_packets;
    /// Number of duplicate packets received
    uint32_t duplicate_packets;
} bap_bc_sink_get_quality_cmp_evt_t;

/// Structure for #BAP_BC_SINK_ENABLE command message
typedef struct bap_bc_sink_enable_cmd
{
    /// Request code (see #bap_bc_sink_msg_cmd_code enumeration)
    /// - BAP_BC_SINK_ENABLE
    uint16_t cmd_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Broadcast ID
    bap_bcast_id_t bcast_id;
    /// Maximum number of subevents the controller should use to receive data payloads in each interval
    uint8_t mse;
    /// Stream position bit field indicating Streams to synchronize with
    uint32_t stream_pos_bf;
    /// Timeout duration (in multiple of 10ms) before considering synchronization has been lost
    /// From 100ms to 163.84s
    uint16_t timeout_10ms;
    /// Indicate if streams are encrypted (!= 0) or not
    uint8_t encrypted;
    /// Broadcast code
    /// Meaningful only if encrypted parameter indicates that streams are encrypted
    gaf_bcast_code_t bcast_code;
} bap_bc_sink_enable_cmd_t;

/// Structure for #BAP_BC_SINK_DISABLE command message
typedef struct bap_bc_sink_disable_cmd
{
    /// Request code (see #bap_bc_sink_msg_cmd_code enumeration)
    /// - BAP_BC_SINK_DISABLE
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
} bap_bc_sink_disable_cmd_t;

/// Structure for #BAP_BC_SINK_START_STREAMING command message
typedef struct bap_bc_sink_start_streaming_cmd
{
    /// Request code (see #bap_bc_sink_msg_cmd_code enumeration)
    /// - BAP_BC_SINK_START_STREAMING
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
    /// Position of the stream in the group
    /// From 1 to 32
    uint8_t stream_pos;
    /// Controller delay in microseconds
    uint32_t ctl_delay_us;
    /// Codec ID value
    gaf_codec_id_t codec_id;
    /// Data Path ID
    uint8_t dp_id;
    /// Codec Configuration value (in LTV format)
    gaf_ltv_t cfg;
} bap_bc_sink_start_streaming_cmd_t;

/// Structure for #BAP_BC_SINK_STOP_STREAMING command message
typedef struct bap_bc_sink_stop_streaming_cmd
{
    /// Request code (see #bap_bc_sink_msg_cmd_code enumeration)
    /// - BAP_BC_SINK_STOP_STREAMING
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
    /// Position of the stream in the group
    /// From 1 to 32
    uint8_t stream_pos;
} bap_bc_sink_stop_streaming_cmd_t;

/// Structure for #BAP_BC_SINK_GET_QUALITY command message
typedef struct bap_bc_sink_get_quality_cmd
{
    /// Request code (see #bap_bc_sink_msg_cmd_code enumeration)
    /// - BAP_BC_SINK_GET_QUALITY
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
    /// Position of the stream in the group
    /// From 1 to 32
    uint8_t stream_pos;
} bap_bc_sink_get_quality_cmd_t;

/// Structure for #BAP_BC_SINK_STATUS indication message
typedef struct bap_bc_sink_status_ind
{
    /// Indication code (see #bap_bc_sink_msg_ind_code enumeration)
    /// - BAP_BC_SINK_STATUS
    uint16_t ind_code;
    /// Group local index
    uint8_t grp_lid;
    /// Broadcast Sink state (see #bap_bc_sink_state enumeration)
    uint8_t state;
    /// Stream position bit field indicating Stream with which synchronization is established
    /// Meaningful only if synchronization has been established
    uint32_t stream_pos_bf;
    /// Broadcast Group synchronization configuration
    /// Meaningful only if synchronization has been established
    gapi_bg_sync_config_t bg_cfg;
    /// Number of BISes synchronization has been established with
    /// Meaningful only if synchronization has been established
    uint8_t nb_bis;
    /// List of Connection Handle values provided by the Controller (nb_bis elements)
    uint16_t conhdl[__ARRAY_EMPTY];
} bap_bc_sink_status_ind_t;

/// Structure for #BAP_BC_SINK_ENABLE_RI request indication message
typedef struct bap_bc_sink_enable_req_ind
{
    /// Request Indication code (see #bap_bc_sink_msg_req_ind_code enumeration)
    /// -  BAP_BC_SINK_ENABLE_RI
    uint16_t req_ind_code;
    /// Group local index
    uint8_t grp_lid;
    /// Source local index
    uint8_t src_lid;
    /// Connection local index of Broadcast Assistant requiring synchronization
    uint8_t con_lid;
    /// Bit field indicating streams with which synchronization is mandatory
    uint32_t stream_pos_bf;
    /// Bit field indicating streams with which synchronization is optional\n
    /// List of selected streams to synchronize with must be provided in #BAP_BC_SINK_ENABLE_RI configuration message
    uint32_t stream_pos_bf_opt;
} bap_bc_sink_enable_req_ind_t;

/// Structure for #BAP_BC_SINK_ENABLE_RI confirm message
typedef struct bap_bc_sink_enable_cfm
{
    /// Request Indication code (see #bap_bc_sink_msg_req_ind_code enumeration)
    /// -  BAP_BC_SINK_ENABLE_RI
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Bit field indicating optional streams with which synchronization is requested
    uint32_t stream_sync_bf_opt;
    /// Group local index
    uint8_t grp_lid;
    /// Timeout duration (in multiple of 10ms) before considering synchronization has been lost
    /// From 100ms to 163.84s
    uint16_t timeout_10ms;
    /// Maximum number of subevents the controller should use to received data payloads in each interval
    uint8_t mse;
} bap_bc_sink_enable_cfm_t;

/// Structure for #BAP_BC_SINK_DISABLE_RI request indication message
typedef struct bap_bc_sink_disable_req_ind
{
    /// Request Indication code (see #bap_bc_sink_msg_req_ind_code enumeration)
    /// -  BAP_BC_SINK_DISABLE_RI
    uint16_t req_ind_code;
    /// Group local index
    uint8_t grp_lid;
    /// Connection local index of Broadcast Assistant requiring synchronization
    uint8_t con_lid;
} bap_bc_sink_disable_req_ind_t;

/// Structure for #BAP_BC_SINK_DISABLE_RI confirm message
typedef struct bap_bc_sink_disable_cfm
{
    /// Request Indication code (see #bap_bc_sink_msg_req_ind_code enumeration)
    /// -  BAP_BC_SINK_DISABLE_RI
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Group local index
    uint8_t grp_lid;
} bap_bc_sink_disable_cfm_t;

/// @} BAP_BC_SINK_MSG

#endif // BAP_BC_SINK_MSG_H_
