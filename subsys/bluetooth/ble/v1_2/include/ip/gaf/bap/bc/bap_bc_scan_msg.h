/**
 ****************************************************************************************
 *
 * @file bap_bc_scan_msg.h
 *
 * @brief Basic Audio Profile - Broadcast Scan - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_SCAN_MSG_H_
#define BAP_BC_SCAN_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SCAN_MSG Message API
 * @ingroup BAP_BC_SCAN
 * @brief Description of Message API for Broadcast Scan module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap_bc_scan.h"     // Basic Audio Profile - Broadcast Scan Definitions
#include "bap_msg.h"         // Message API Definitions

/// @addtogroup BAP_BC_SCAN_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Broadcast Scan module
enum bap_bc_scan_msg_cmd_code
{
    /// Start seeking for Broadcast Audio Announcements sent by Broadcast Source devices
    /// (see #bap_bc_scan_start_cmd_t)
    BAP_BC_SCAN_START = GAF_CODE(BAP, BC_SCAN, BAP_BC_SCAN_CMD_TYPE_START),
    /// Stop all scan procedures currently in progress (see #bap_bc_scan_stop_cmd_t)
    BAP_BC_SCAN_STOP = GAF_CODE(BAP, BC_SCAN, BAP_BC_SCAN_CMD_TYPE_STOP),
    /// Synchronize with a Periodic Advertising (see #bap_bc_scan_pa_synchronize_cmd_t)
    BAP_BC_SCAN_PA_SYNCHRONIZE = GAF_CODE(BAP, BC_SCAN, BAP_BC_SCAN_CMD_TYPE_PA_SYNCHRONIZE),
    /// Terminate synchronization or cancel synchronization establishment with Periodic Advertising
    /// (see #bap_bc_scan_pa_terminate_cmd_t)
    BAP_BC_SCAN_PA_TERMINATE = GAF_CODE(BAP, BC_SCAN, BAP_BC_SCAN_CMD_TYPE_PA_TERMINATE),
};

/// List of GAF_REQ request code values for Broadcast Scan module
enum bap_bc_scan_msg_req_code
{
    /// Set scan parameters to be used for all scan procedures (see #bap_bc_scan_set_params_req_t)
    BAP_BC_SCAN_SET_PARAMS = GAF_CODE(BAP, BC_SCAN, 0),
    /// Control reception of Periodic Advertising reports (see #bap_bc_scan_pa_report_ctrl_req_t)
    BAP_BC_SCAN_PA_REPORT_CTRL = GAF_CODE(BAP, BC_SCAN, 1),
};

/// List of GAF_IND indication code values for Broadcast Scan module
enum bap_bc_scan_msg_ind_code
{
    /// Inform upper layer that discovery of Broadcast Source devices has been stopped due to a timeout
    /// (see #bap_bc_scan_timeout_ind_t)
    BAP_BC_SCAN_TIMEOUT = GAF_CODE(BAP, BC_SCAN, 0),
    /// Inform upper layer that a Broadcast Source device has been discovered (see #bap_bc_scan_report_ind_t)
    BAP_BC_SCAN_REPORT = GAF_CODE(BAP, BC_SCAN, 1),
    /// Inform upper layer that synchronization with a Periodic Advertising has been established
    /// (see #bap_bc_scan_pa_established_ind_t)
    BAP_BC_SCAN_PA_ESTABLISHED = GAF_CODE(BAP, BC_SCAN, 2),
    /// Inform upper layer that synchronization with a Periodic Advertising has been terminated, lost or that
    /// synchronization establishment has been aborted (see #bap_bc_scan_pa_terminated_ind_t)
    BAP_BC_SCAN_PA_TERMINATED = GAF_CODE(BAP, BC_SCAN, 3),
    /// Inform upper layer that a Periodic Advertising report has been received (see #bap_bc_scan_pa_report_ind_t)
    BAP_BC_SCAN_PA_REPORT = GAF_CODE(BAP, BC_SCAN, 4),
    /// Inform upper layer that a BIG Info advertising report has been received
    /// (see #bap_bc_scan_big_info_report_ind_t)
    BAP_BC_SCAN_BIG_INFO_REPORT = GAF_CODE(BAP, BC_SCAN, 5),
    /// Inform upper layer about Broadcast Group related information extraction from a received Basic Audio
    /// Announcement (see #bap_bc_scan_group_report_ind_t)
    BAP_BC_SCAN_GROUP_REPORT = GAF_CODE(BAP, BC_SCAN, 6),
    /// Inform upper layer about Broadcast Subgroup related information extraction from a received Basic Audio
    /// Announcement (see #bap_bc_scan_subgroup_report_ind_t)
    BAP_BC_SCAN_SUBGROUP_REPORT = GAF_CODE(BAP, BC_SCAN, 7),
    /// Inform upper layer about Broadcast Streams related information extraction from a received Basic Audio
    /// Announcement (see #bap_bc_scan_stream_report_ind_t)
    BAP_BC_SCAN_STREAM_REPORT = GAF_CODE(BAP, BC_SCAN, 8),
    /// Inform upper layer that a Public Broadcast Source has been discovered
    BAP_BC_SCAN_PUBLIC_BCAST_SOURCE = GAF_CODE(BAP, BC_SCAN, 9),
};

/// List of GAF_REQ_IND request indication code values for Broadcast Scan module
enum bap_bc_scan_msg_req_ind_code
{
    /// Inform upper layer that a Broadcast Assistant has requested to establish synchronization with a Periodic
    /// Advertising (see #bap_bc_scan_pa_synchronize_req_ind_t)
    BAP_BC_SCAN_PA_SYNCHRONIZE_RI = GAF_CODE(BAP, BC_SCAN, 0),
    /// Inform upper layer that a Broadcast Assistant has requested to terminate synchronization with a Periodic
    /// Advertising (see #bap_bc_scan_pa_terminate_req_ind_t)
    BAP_BC_SCAN_PA_TERMINATE_RI = GAF_CODE(BAP, BC_SCAN, 1),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #BAP_BC_SCAN_START command message
typedef struct bap_bc_scan_start_cmd
{
    /// Command code (shall be set to #BAP_BC_SCAN_START)
    uint16_t cmd_code;
    /// Timeout duration in seconds
    uint16_t timeout_s;
} bap_bc_scan_start_cmd_t;

/// Structure for #BAP_BC_SCAN_STOP command message
typedef struct bap_bc_scan_stop_cmd
{
    /// Command code (shall be set to #BAP_BC_SCAN_STOP)
    uint16_t cmd_code;
} bap_bc_scan_stop_cmd_t;

/// Structure for #BAP_BC_SCAN_PA_SYNCHRONIZE command message
typedef struct bap_bc_scan_pa_synchronize_cmd
{
    /// Command code (shall be set to #BAP_BC_SCAN_PA_SYNCHRONIZE)
    uint16_t cmd_code;
    /// Periodic Advertising identification
    bap_adv_id_t adv_id;
    /// Number of Periodic Advertising that can be skipped after a successful reception\n
    /// Maximum authorized value is 499
    uint16_t skip;
    /// Report filtering bit field (see #bap_bc_scan_report_filter_bf enumeration)
    uint8_t report_filter_bf;
    /// Synchronization timeout for the Periodic Advertising (in unit of 10ms between 100ms and 163.84s)
    uint16_t sync_to_10ms;
    /// Scan Timeout in seconds
    uint16_t timeout_s;
} bap_bc_scan_pa_synchronize_cmd_t;

/// Structure for #BAP_BC_SCAN_PA_TERMINATE command message
typedef struct bap_bc_scan_pa_terminate_cmd
{
    /// Command code (shall be set to #BAP_BC_SCAN_PA_TERMINATE)
    uint16_t cmd_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
} bap_bc_scan_pa_terminate_cmd_t;

/// Structure for command complete event message
typedef struct bap_bc_scan_cmp_evt
{
    /// Command code (see #bap_bc_scan_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Command execution status code
    uint16_t status;
    /// Allocated Periodic Advertising local index
    /// Meaningful only if command succeed
    /// Meaningful only for BAP_BC_SCAN_PA_SYNCHRONIZE, BAP_BC_SCAN_PA_SYNCHRONIZE_CON and BAP_BC_SCAN_PA_TRANSFER
    /// commands
    uint8_t pa_lid;
} bap_bc_scan_cmp_evt_t;

/// Structure for #BAP_BC_SCAN_SET_PARAMS request message
typedef struct bap_bc_scan_set_params_req
{
    /// Request code (shall be set to #BAP_BC_SCAN_SET_PARAMS)
    uint16_t req_code;
    /// Scan parameters
    bap_bc_scan_param_t scan_param;
} bap_bc_scan_set_params_req_t;

/// Structure for #BAP_BC_SCAN_PA_REPORT_CTRL request message
typedef struct bap_bc_scan_pa_report_ctrl_req
{
    /// Request code (shall be set to #BAP_BC_SCAN_PA_REPORT_CTRL)
    uint16_t req_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Report filtering bit field (see #bap_bc_scan_report_filter_bf enumeration)
    uint8_t report_filter_bf;
} bap_bc_scan_pa_report_ctrl_req_t;

/// Structure for response message
typedef struct bap_bc_scan_rsp
{
    /// Request code (see #bap_bc_scan_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Periodic Advertising local index
    uint8_t pa_lid;
} bap_bc_scan_rsp_t;

/// Structure for #BAP_BC_SCAN_TIMEOUT indication message
typedef struct bap_bc_scan_timeout_ind
{
    /// Indication code (set to #BAP_BC_SCAN_TIMEOUT)
    uint16_t ind_code;
} bap_bc_scan_timeout_ind_t;

/// Structure for #BAP_BC_SCAN_REPORT indication message
typedef struct bap_bc_scan_report_ind
{
    /// Indication code (set to #BAP_BC_SCAN_REPORT)
    uint16_t ind_code;
    /// Periodic Advertising identification
    bap_adv_id_t adv_id;
    /// Broadcast ID
    bap_bcast_id_t bcast_id;
    /// Information bit field (see #bap_bc_scan_info_bf enumeration)\n
    uint8_t info_bf;
    /// Air information
    gaf_adv_report_air_info_t air_info;
    /// Length of complete advertising data
    uint16_t length;
    /// Complete advertising data containing the complete Broadcast Audio Announcement
    uint8_t data[__ARRAY_EMPTY];
} bap_bc_scan_report_ind_t;

/// Structure for #BAP_BC_SCAN_PA_ESTABLISHED indication message
typedef struct bap_bc_scan_pa_established_ind
{
    /// Indication code (set to #BAP_BC_SCAN_PA_ESTABLISHED)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Periodic Advertising identification
    bap_adv_id_t adv_id;
    /// PHY on which synchronization has been established (see #gap_le_phy_val enumeration)
    uint8_t phy;
    /// Periodic advertising interval (in unit of 1.25ms, min is 7.5ms)
    uint16_t interval_frames;
} bap_bc_scan_pa_established_ind_t;

/// Structure for #BAP_BC_SCAN_PA_TERMINATED indication message
typedef struct bap_bc_scan_pa_terminated_ind
{
    /// Indication code (set to #BAP_BC_SCAN_PA_TERMINATED)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Stop reason (see #bap_bc_scan_pa_term_reason enumeration)
    uint8_t reason;
} bap_bc_scan_pa_terminated_ind_t;

/// Structure for #BAP_BC_SCAN_PA_REPORT indication message
typedef struct bap_bc_scan_pa_report_ind
{
    /// Indication code (set to #BAP_BC_SCAN_PA_REPORT)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Air information
    gaf_adv_report_air_info_t air_info;
    /// Length of Periodic Advertising report
    uint16_t length;
    /// Periodic Advertising report
    uint8_t data[__ARRAY_EMPTY];
} bap_bc_scan_pa_report_ind_t;

/// Structure for #BAP_BC_SCAN_BIG_INFO_REPORT indication message
typedef struct bap_bc_scan_big_info_report_ind
{
    /// Indication code (set to #BAP_BC_SCAN_BIG_INFO_REPORT)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// BIG Info Report
    gapm_le_big_info_t report;
} bap_bc_scan_big_info_report_ind_t;

/// Structure for #BAP_BC_SCAN_GROUP_REPORT indication message
typedef struct bap_bc_scan_group_report_ind
{
    /// Indication code (set to #BAP_BC_SCAN_GROUP_REPORT)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Number of subgroups in the Broadcast Group
    uint8_t nb_subgroups;
    /// Total number of streams in the Broadcast Group
    uint8_t nb_streams;
    /// Presentation Delay in microseconds
    uint32_t pres_delay_us;
} bap_bc_scan_group_report_ind_t;

/// Structure for #BAP_BC_SCAN_SUBGROUP_REPORT indication message
typedef struct bap_bc_scan_subgroup_report_ind
{
    /// Indication code (set to #BAP_BC_SCAN_SUBGROUP_REPORT)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Subgroup identifier
    uint8_t sgrp_id;
    /// Codec ID
    gaf_codec_id_t codec_id;
    /// Stream position index bit field indicating which Streams are part of the Subgroup
    uint32_t stream_pos_bf;
    /// Codec Configuration parameters structure
    bap_cfg_param_t param;
    /// Codec Configuration Metadata parameters structure
    bap_cfg_metadata_param_t param_metadata;
    /// Length of additional Codec Configuration
    uint8_t add_cfg_len;
    /// Length of additional Metadata
    uint8_t add_metadata_len;
    /// Additional Codec Configuration (in LTV format) followed by additional Metadata (in LTV format)\n
    /// Length of array is add_cfg_len + add_metadata_len
    uint8_t val[__ARRAY_EMPTY];
} bap_bc_scan_subgroup_report_ind_t;

/// Structure for #BAP_BC_SCAN_STREAM_REPORT indication message
typedef struct bap_bc_scan_stream_report_ind
{
    /// Indication code (set to #BAP_BC_SCAN_STREAM_REPORT)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Subgroup identifier
    uint8_t sgrp_id;
    /// Stream position in group
    uint8_t stream_pos;
    /// Codec ID
    gaf_codec_id_t codec_id;
    /// Codec Configuration structure
    bap_cfg_t cfg;
} bap_bc_scan_stream_report_ind_t;

#if (GAF_PBP)
/// Structure for #BAP_BC_SCAN_PUBLIC_BCAST_SOURCE indication message
typedef struct bap_bc_scan_public_bcast_source_ind
{
    /// Indication code (set to #BAP_BC_SCAN_PUBLIC_BCAST_SOURCE)
    uint16_t ind_code;
    /// Periodic Advertising identification
    bap_adv_id_t adv_id;
    /// Broadcast ID
    bap_bcast_id_t bcast_id;
    /// PBP features bit field (see #bap_bc_pbp_features_bf enumeration)
    uint8_t pbp_features_bf;
    /// Public Broadcast name length
    uint8_t broadcast_name_len;
    /// Public Metadata length
    uint8_t metadata_len;
    /// Public Broadcast name followed by Public Metadata
    uint8_t data[__ARRAY_EMPTY];
} bap_bc_scan_public_bcast_source_ind_t;
#endif //(GAF_PBP)

/// Structure for #BAP_BC_SCAN_PA_SYNCHRONIZE_RI request indication message
typedef struct bap_bc_scan_pa_synchronize_req_ind
{
    /// Request Indication code (set to #BAP_BC_SCAN_PA_SYNCHRONIZE_RI)
    uint16_t req_ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Source local index
    uint8_t src_lid;
    /// Connection local index
    uint8_t con_lid;
} bap_bc_scan_pa_synchronize_req_ind_t;

/// Structure for #BAP_BC_SCAN_PA_TERMINATE_RI request indication message
typedef struct bap_bc_scan_pa_terminate_req_ind
{
    /// Request Indication code (to #BAP_BC_SCAN_PA_TERMINATE_RI)
    uint16_t req_ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Connection local index
    uint8_t con_lid;
} bap_bc_scan_pa_terminate_req_ind_t;

/// Structure for #BAP_BC_SCAN_PA_SYNCHRONIZE_RI confirmation message
typedef struct bap_bc_scan_pa_synchronize_cfm
{
    /// Request Indication code (shall be set to #BAP_BC_SCAN_PA_SYNCHRONIZE_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Report filtering bit field (see #bap_bc_scan_report_filter_bf enumeration)
    uint8_t report_filter_bf;
    /// Number of Periodic Advertising that can be skipped after a successful reception\n
    /// Maximum authorized value is 499
    uint16_t skip;
    /// Synchronization timeout for the Periodic Advertising (in unit of 10ms, between 100ms and 163.84s)
    uint16_t sync_to_10ms;
    /// Synchronization establishment timeout in seconds
    uint16_t timeout_s;
} bap_bc_scan_pa_synchronize_cfm_t;

/// Structure for #BAP_BC_SCAN_PA_TERMINATE_RI confirmation message
typedef struct bap_bc_scan_pa_terminate_cfm
{
    /// Request Indication code (shall be set to #BAP_BC_SCAN_PA_TERMINATE_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Periodic Advertising local index
    uint8_t pa_lid;
} bap_bc_scan_pa_terminate_cfm_t;

/// @} BAP_BC_SCAN_MSG

#endif // BAP_BC_SCAN_MSG_H_
