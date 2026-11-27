/**
 ****************************************************************************************
 *
 * @file bap_uc_cli_msg.h
 *
 * @brief Basic Audio Profile - Unicast Client - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_UC_CLI_MSG_H_
#define BAP_UC_CLI_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CLI_MSG Message API
 * @ingroup BAP_UC_CLI
 * @brief Description of Message API for Unicast Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap_msg.h"        // Basic Audio Profile Message API Definitions
#include "bap_uc_cli.h"     // Basic Audio Profile - Unicast Client Definitions

/// @addtogroup BAP_UC_CLI_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Unicast Client module
enum bap_uc_cli_msg_cmd_code
{
    /// Discover (see #bap_uc_cli_discover_cmd_t)
    BAP_UC_CLI_DISCOVER = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_DISCOVER),
    /// Configure Codec for an ASE (see #bap_uc_cli_configure_codec_cmd_t)
    BAP_UC_CLI_CONFIGURE_CODEC = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_CONFIGURE_CODEC),
    /// Configure QoS for an ASE (see #bap_uc_cli_configure_qos_cmd_t)
    BAP_UC_CLI_CONFIGURE_QOS = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_CONFIGURE_QOS),
    /// Enable an ASE (see #bap_uc_cli_enable_cmd_t)
    BAP_UC_CLI_ENABLE = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_ENABLE),
    /// Update Metadata for an ASE (see #bap_uc_cli_update_metadata_cmd_t)
    BAP_UC_CLI_UPDATE_METADATA = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_UPDATE_METADATA),
    /// Disable an ASE (see #bap_uc_cli_disable_cmd_t)
    BAP_UC_CLI_DISABLE = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_DISABLE),
    /// Release an ASE (see #bap_uc_cli_release_cmd_t)
    BAP_UC_CLI_RELEASE = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_RELEASE),
    /// Get Quality (see #bap_uc_cli_get_quality_cmd_t)
    BAP_UC_CLI_GET_QUALITY = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_GET_QUALITY),
    /// Set Configuration (see #bap_uc_cli_set_cfg_cmd_t)
    BAP_UC_CLI_SET_CFG = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_SET_CFG),
    /// Get State (see #bap_uc_cli_get_state_cmd_t)
    BAP_UC_CLI_GET_STATE = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_GET_STATE),
    /// Remove Group (see #bap_uc_cli_remove_group_cmd_t)
    BAP_UC_CLI_REMOVE_GROUP = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_REMOVE_GROUP),
    /// Control CIS (see #bap_uc_cli_cis_control_cmd_t)
    BAP_UC_CLI_CIS_CONTROL = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_CIS_CONTROL),
    /// Set Group parameters (see #bap_uc_cli_set_group_params_cmd_t)
    BAP_UC_CLI_SET_GROUP_PARAMS = GAF_CODE(BAP, UC_CLI, BAP_UC_CLI_CMD_TYPE_SET_GROUP_PARAMS),
};

/// List of GAF_REQ request code values for Unicast Client module
enum bap_uc_cli_msg_req_code
{
    /// Restore Bond Data (see #bap_uc_cli_restore_bond_data_req_t)
    BAP_UC_CLI_RESTORE_BOND_DATA = GAF_CODE(BAP, UC_CLI, 0),
    /// Restore Codec Configuration stored as Bond Data (see #bap_uc_cli_restore_bond_data_codec_req_t)
    BAP_UC_CLI_RESTORE_BOND_DATA_CODEC = GAF_CODE(BAP, UC_CLI, 1),
    /// Create a Group (see #bap_uc_cli_create_group_req_t)
    BAP_UC_CLI_CREATE_GROUP = GAF_CODE(BAP, UC_CLI, 2),
    /// Update write type used for ASE Control Point characteristic - Mainly for PTS testing purpose
    BAP_UC_CLI_SET_WRITE_TYPE = GAF_CODE(BAP, UC_CLI, 3),
    /// Create Stream
    BAP_UC_CLI_CREATE_STREAM = GAF_CODE(BAP, UC_CLI, 4),
};

/// List of GAF_IND indication code values for Unicast Client module
enum bap_uc_cli_msg_ind_code
{
    /// Bond Data (see #bap_uc_cli_bond_data_ind_t)
    BAP_UC_CLI_BOND_DATA = GAF_CODE(BAP, UC_CLI, 0),
    /// CIS State (see #bap_uc_cli_cis_state_ind_t)
    BAP_UC_CLI_CIS_STATE = GAF_CODE(BAP, UC_CLI, 2),
    /// State (Idle or Releasing) (see #bap_uc_cli_state_empty_ind_t)
    BAP_UC_CLI_STATE_EMPTY = GAF_CODE(BAP, UC_CLI, 3),
    /// State (Codec Configured) (see #bap_uc_cli_state_codec_ind_t)
    BAP_UC_CLI_STATE_CODEC = GAF_CODE(BAP, UC_CLI, 4),
    /// State (QoS Configured) (see #bap_uc_cli_state_qos_ind_t)
    BAP_UC_CLI_STATE_QOS = GAF_CODE(BAP, UC_CLI, 5),
    /// State (Enabling or Streaming or Disabling) (see #bap_uc_cli_state_metadata_ind_t)
    BAP_UC_CLI_STATE_METADATA = GAF_CODE(BAP, UC_CLI, 6),
    /// Error (see #bap_uc_cli_error_ind_t)
    BAP_UC_CLI_ERROR = GAF_CODE(BAP, UC_CLI, 7),
    /// Service Changed (see #bap_uc_cli_svc_changed_ind_t)
    BAP_UC_CLI_SVC_CHANGED = GAF_CODE(BAP, UC_CLI, 8),
};

/// List of GAF_REQ_IND request indication code values for Unicast Client module
enum bap_uc_cli_msg_req_ind_code
{
    /// Data Path Update (see #bap_uc_cli_dp_update_req_ind_t)
    BAP_UC_CLI_DP_UPDATE = GAF_CODE(BAP, UC_CLI, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #BAP_UC_CLI_RESTORE_BOND_DATA request message (see \ref bap_msc_ascs_reconnection)
typedef struct bap_uc_cli_restore_bond_data_req
{
    /// Request code (shall be set to #BAP_UC_CLI_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Audio Stream Control Service content description
    bap_uc_cli_ascs_t ascs_info;
} bap_uc_cli_restore_bond_data_req_t;

/// Structure for #BAP_UC_CLI_RESTORE_BOND_DATA_CODEC request message (see \ref bap_msc_ascs_reconnection)
typedef struct bap_uc_cli_restore_bond_data_codec_req
{
    /// Request code (shall be set to #BAP_UC_CLI_RESTORE_BOND_DATA_CODEC)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// ASE local index
    uint8_t ase_lid;
    /// ASE instance index
    uint8_t ase_instance_idx;
    /// Data Path Configuration bit field (see #bap_dp_cfg_bf enumeration)
    uint16_t dp_cfg_bf;
    /// Codec ID
    gaf_codec_id_t codec_id;
    /// Controller Delay in microseconds
    uint32_t ctl_delay_us;
    /// Codec Configuration
    bap_cfg_t cfg;
} bap_uc_cli_restore_bond_data_codec_req_t;

/// Structure for #BAP_UC_CLI_CREATE_GROUP request message
typedef struct bap_uc_cli_create_group_req
{
    /// Request code (shall be set to #BAP_UC_CLI_CREATE_GROUP)
    uint16_t req_code;
    /// CIG ID
    uint8_t cig_id;
    /// Group parameters
    bap_uc_cli_grp_param_t params;
} bap_uc_cli_create_group_req_t;

/// Structure for #BAP_UC_CLI_CREATE_STREAM request message
typedef struct
{
    /// Request code (shall be set to #BAP_UC_CLI_CREATE_STREAM)
    uint16_t req_code;
    /// Group local index
    uint8_t grp_lid;
    /// CIS ID
    uint8_t cis_id;
    /// Stream parameters
    gapi_us_param_t params;
} bap_uc_cli_create_stream_req_t;

/// Structure for #BAP_UC_CLI_SET_WRITE_TYPE request message
typedef struct bap_uc_cli_set_write_type_req
{
    /// Request code (shall be set to #BAP_UC_CLI_SET_WRITE_TYPE)
    uint16_t req_code;
    /// Indicate if used of reliable write is preferred
    bool reliable;
} bap_uc_cli_set_write_type_req_t;

/// Structure for response message
typedef struct bap_uc_cli_rsp
{
    /// Request code (see #bap_uc_cli_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Union
    union
    {
        /// Local index
        uint8_t lid;
        /// Connection local index
        uint8_t con_lid;
        /// Group local index
        uint8_t grp_lid;
    } lid;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_cli_rsp_t;

/// Structure for #BAP_UC_CLI_REMOVE_GROUP command message
typedef struct bap_uc_cli_remove_group_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_REMOVE_GROUP)
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
} bap_uc_cli_remove_group_cmd_t;

/// Structure for #BAP_UC_CLI_CIS_CONTROL command message
typedef struct bap_uc_cli_cis_control_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_CIS_CONTROL)
    uint16_t cmd_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Indicate if CIS bound with the indicated ASE must be established (!= 0) or disconnected
    bool establish;
} bap_uc_cli_cis_control_cmd_t;

/// Structure for #BAP_UC_CLI_DISCOVER command message (see \ref bap_msc_ascs_discovery)
typedef struct bap_uc_cli_discover_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Start handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t shdl;
    /// End handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t ehdl;
} bap_uc_cli_discover_cmd_t;

/// Structure for #BAP_UC_CLI_CONFIGURE_CODEC command message (see \ref bap_msc_ascs_configure_codec)
typedef struct bap_uc_cli_configure_codec_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_CONFIGURE_CODEC)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// ASE local index
    uint8_t ase_lid;
    /// ASE instance index
    uint8_t ase_instance_idx;
    /// Data Path Configuration bit field (see #bap_dp_cfg_bf enumeration)
    uint16_t dp_cfg_bf;
    /// Target Latency (see #bap_uc_tgt_latency enumeration)
    uint8_t tgt_latency;
    /// Target PHY (see #bap_uc_tgt_phy enumeration)
    uint8_t tgt_phy;
    /// Target PHY
    /// Codec ID
    gaf_codec_id_t codec_id;
    /// Controller Delay in microseconds
    uint32_t ctl_delay_us;
    /// Codec Configuration
    bap_cfg_t cfg;
} bap_uc_cli_configure_codec_cmd_t;

/// Structure for #BAP_UC_CLI_CONFIGURE_QOS command message (see \ref bap_msc_ascs_configure_qos)
typedef struct bap_uc_cli_configure_qos_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_CONFIGURE_QOS)
    uint16_t cmd_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Group local index
    uint8_t grp_lid;
    /// CIS ID
    uint8_t cis_id;
    /// QoS Configuration
    bap_uc_cli_qos_cfg_t qos_cfg;
} bap_uc_cli_configure_qos_cmd_t;

/// Structure for #BAP_UC_CLI_ENABLE command message (see \ref bap_msc_ascs_enable)
typedef struct bap_uc_cli_enable_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_ENABLE)
    uint16_t cmd_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Metadata
    /// List of CCIDs for the ASE can be provided as part of the additional Metadata
    bap_cfg_metadata_t metadata;
} bap_uc_cli_enable_cmd_t;

/// Structure for #BAP_UC_CLI_UPDATE_METADATA command message (see \ref bap_msc_ascs_update_metadata)
typedef struct bap_uc_cli_update_metadata_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_UPDATE_METADATA)
    uint16_t cmd_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Metadata
    /// List of CCIDs for the ASE can be provided as part of the additional Metadata
    bap_cfg_metadata_t metadata;
} bap_uc_cli_update_metadata_cmd_t;

/// Structure for #BAP_UC_CLI_DISABLE command message (see \ref bap_msc_ascs_disable)
typedef struct bap_uc_cli_disable_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_DISABLE)
    uint16_t cmd_code;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_cli_disable_cmd_t;

/// Structure for #BAP_UC_CLI_RELEASE command message (see \ref bap_msc_ascs_release)
typedef struct bap_uc_cli_release_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_RELEASE)
    uint16_t cmd_code;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_cli_release_cmd_t;

/// Structure for #BAP_UC_CLI_GET_QUALITY command message (see \ref bap_msc_ascs_get_quality)
typedef struct bap_uc_cli_get_quality_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_GET_QUALITY)
    uint16_t cmd_code;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_cli_get_quality_cmd_t;

/// Structure for #BAP_UC_CLI_SET_CFG command message (see \ref bap_msc_ascs_set_cfg)
typedef struct bap_uc_cli_set_cfg_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_SET_CFG)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type
    uint8_t char_type;
    /// ASE instance index
    /// Meaningful only if configuration for an instance of the ASE characteristic is requested
    uint8_t ase_instance_idx;
    /// Indicate if sending of notifications must be enabled (!= 0) or disabled for the indicated characteristic
    uint8_t enable;
} bap_uc_cli_set_cfg_cmd_t;

/// Structure for #BAP_UC_CLI_GET_STATE command message (see \ref bap_msc_ascs_get)
typedef struct bap_uc_cli_get_state_cmd
{
    /// Command code (shall be set to #BAP_UC_CLI_GET_STATE)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// ASE instance index
    /// Meaningful only if configuration for an instance of the ASE characteristic is requested
    uint8_t ase_instance_idx;
} bap_uc_cli_get_state_cmd_t;

/// Structure for #BAP_UC_CLI_SET_GROUP_PARAMS command message
typedef struct
{
    /// Command code (shall be set to #BAP_UC_CLI_SET_GROUP_PARAMS)
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
} bap_uc_cli_set_group_params_cmd_t;

/// Structure for command complete event message
typedef struct bap_uc_cli_cmp_evt
{
    /// Command code (see #bap_uc_cli_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Union
    union
    {
        /// Local index
        uint8_t lid;
        /// Connection local index
        uint8_t con_lid;
        /// Group local index
        uint8_t grp_lid;
    } u_lid;
    /// Union
    union
    {
        /// ASE parameter
        uint8_t ase_param;
        /// ASE instance index
        uint8_t ase_instance_idx;
        /// ASE local index
        uint8_t ase_lid;
    } u_ase_param;
    /// Characteristic type
    uint8_t char_type;
} bap_uc_cli_cmp_evt_t;

/// Structure for #BAP_UC_CLI_GET_QUALITY command complete event message
typedef struct bap_uc_cli_get_quality_cmp_evt
{
    /// Command code (set to #BAP_UC_CLI_GET_QUALITY)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
    /// Number of packets transmitted and unacked
    uint32_t tx_unacked_packets;
    /// Number of flushed transmitted packets
    uint32_t tx_flushed_packets;
    /// Number of packets transmitted during last subevent
    uint32_t tx_last_subevent_packets;
    /// Number of retransmitted packets
    uint32_t retx_packets;
    /// Number of packets received with a CRC error
    uint32_t crc_error_packets;
    /// Number of unreceived packets
    uint32_t rx_unrx_packets;
    /// Number of duplicate packets received
    uint32_t duplicate_packets;
} bap_uc_cli_get_quality_cmp_evt_t;

/// Structure for #BAP_UC_CLI_BOND_DATA indication message
typedef struct bap_uc_cli_bond_data_ind
{
    /// Indication code (set to #BAP_UC_CLI_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Audio Stream Control Service content description
    bap_uc_cli_ascs_t ascs_info;
} bap_uc_cli_bond_data_ind_t;

/// Structure for #BAP_UC_CLI_CIS_STATE indication message
typedef struct bap_uc_cli_cis_state_ind
{
    /// Indication code (set to #BAP_UC_CLI_CIS_STATE)
    uint16_t ind_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Connection local index of LE connection the CIS is bound with
    uint8_t con_lid;
    /// ASE local index for Sink direction
    uint8_t ase_lid_sink;
    /// ASE local index for Source direction
    uint8_t ase_lid_src;
    /// Group local index
    uint8_t grp_lid;
    /// CIS ID
    uint8_t cis_id;
    /// Connection handle allocated for the CIS by Controller\n
    /// GAP_INVALID_CONHDL indicates that CIS is not established
    uint16_t conhdl;
    /// Event that has triggered update of CIS state (see #bap_uc_cli_cis_event enumeration)
    uint8_t event;
    /// Group configuration
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    gapi_ug_config_t cig_config;
    /// Stream configuration
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    gapi_us_config_t cis_config;
} bap_uc_cli_cis_state_ind_t;

/// Structure for #BAP_UC_CLI_STATE_EMPTY indication message
typedef struct bap_uc_cli_state_empty_ind
{
    /// Indication code (set to #BAP_UC_CLI_STATE_EMPTY)
    uint16_t ind_code;
    /// Connection local index\n
    /// GAF_INVALID_LID indicates that the ASE is no more used
    uint8_t con_lid;
    /// ASE instance index
    /// Meaningful only if con_lid is not equal to GAF_INVALID_LID
    uint8_t ase_instance_idx;
    /// ASE local index
    uint8_t ase_lid;
    /// ASE State
    /// Meaningful only if con_lid is not equal to GAF_INVALID_LID
    uint8_t state;
} bap_uc_cli_state_empty_ind_t;

/// Structure for #BAP_UC_CLI_STATE_CODEC indication message
typedef struct bap_uc_cli_state_codec_ind
{
    /// Indication code (set to #BAP_UC_CLI_STATE_CODEC)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// ASE instance index
    uint8_t ase_instance_idx;
    /// ASE local index
    uint8_t ase_lid;
    /// Codec ID
    gaf_codec_id_t codec_id;
    /// QoS Requirements
    bap_qos_req_t qos_req;
    /// Codec Configuration
    bap_cfg_t cfg;
} bap_uc_cli_state_codec_ind_t;

/// Structure for #BAP_UC_CLI_STATE_QOS indication message
typedef struct bap_uc_cli_state_qos_ind
{
    /// Indication code (set to #BAP_UC_CLI_STATE_QOS)
    uint16_t ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// QoS Configuration
    bap_qos_cfg_t qos_cfg;
} bap_uc_cli_state_qos_ind_t;

/// Structure for #BAP_UC_CLI_STATE_METADATA indication message
typedef struct bap_uc_cli_state_metadata_ind
{
    /// Indication code (set to #BAP_UC_CLI_STATE_METADATA)
    uint16_t ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// State
    uint8_t state;
    /// Metadata
    bap_cfg_metadata_t metadata;
} bap_uc_cli_state_metadata_ind_t;

/// Structure for #BAP_UC_CLI_SVC_CHANGED indication message
typedef struct bap_uc_cli_svc_changed_ind
{
    /// Indication code (set to #BAP_UC_CLI_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} bap_uc_cli_svc_changed_ind_t;

/// Structure for #BAP_UC_CLI_ERROR indication message
typedef struct bap_uc_cli_error_ind
{
    /// Indication code (set to #BAP_UC_CLI_ERROR)
    uint16_t ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Operation code
    uint8_t opcode;
    /// Response code
    uint8_t rsp_code;
    /// Reason
    uint8_t reason;
} bap_uc_cli_error_ind_t;

/// Structure for #BAP_UC_CLI_DP_UPDATE request indication message
typedef struct bap_uc_cli_dp_update_req_ind
{
    /// Request indication code (set to #BAP_UC_CLI_DP_UPDATE)
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Indicate if Data Path is about to be started or stopped for the ASE
    bool start;
} bap_uc_cli_dp_update_req_ind_t;

/// Structure for #BAP_UC_CLI_DP_UPDATE confirmation message
typedef struct bap_uc_cli_dp_update_cfm
{
    /// Request indication code (shall be set to #BAP_UC_CLI_DP_UPDATE)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_cli_dp_update_cfm_t;

/// @} BAP_UC_CLI_MSG

#endif // BAP_UC_CLI_MSG_H_
