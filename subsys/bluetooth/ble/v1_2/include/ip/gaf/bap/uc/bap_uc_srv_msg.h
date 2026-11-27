/**
 ****************************************************************************************
 *
 * @file bap_uc_srv_msg.h
 *
 * @brief Basic Audio Profile - Unicast Server - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_UC_SRV_MSG_H_
#define BAP_UC_SRV_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_UC_SRV_MSG Message API
 * @ingroup BAP_UC_SRV
 * @brief Description of Message API for Unicast Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap_msg.h"        // Basic Audio Profile Message API Definitions
#include "bap_uc_srv.h"     // Basic Audio Profile - Unicast Server Definitions

/// @addtogroup BAP_UC_SRV_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Unicast Server module
enum bap_uc_srv_msg_cmd_code
{
    /// Disable ASE (see #bap_uc_srv_disable_cmd_t)
    BAP_UC_SRV_DISABLE = GAF_CODE(BAP, UC_SRV, BAP_UC_SRV_CMD_TYPE_DISABLE),
    /// Release ASE (see #bap_uc_srv_release_cmd_t)
    BAP_UC_SRV_RELEASE = GAF_CODE(BAP, UC_SRV, BAP_UC_SRV_CMD_TYPE_RELEASE),
    /// Get Quality (see #bap_uc_srv_get_quality_cmd_t)
    BAP_UC_SRV_GET_QUALITY = GAF_CODE(BAP, UC_SRV, BAP_UC_SRV_CMD_TYPE_GET_QUALITY),
};

/// List of GAF_REQ command code values for Unicast Server module
enum bap_uc_srv_msg_req_code
{
    /// Restore Bond Data (see #bap_uc_srv_restore_bond_data_req_t)
    BAP_UC_SRV_RESTORE_BOND_DATA = GAF_CODE(BAP, UC_SRV, 0),
    /// Restore Codec Configuration stored as Bond Data (see #bap_uc_srv_restore_bond_data_codec_req_t)
    BAP_UC_SRV_RESTORE_BOND_DATA_CODEC = GAF_CODE(BAP, UC_SRV, 1),
    /// Configure Codec for an ASE (see #bap_uc_srv_configure_codec_req_t)
    BAP_UC_SRV_CONFIGURE_CODEC = GAF_CODE(BAP, UC_SRV, 2),
    /// Update Metadata for an ASE (see #bap_uc_srv_update_metadata_req_t)
    BAP_UC_SRV_UPDATE_METADATA = GAF_CODE(BAP, UC_SRV, 3),
};

/// List of GAF_IND indication code values for Unicast Server module
enum bap_uc_srv_msg_ind_code
{
    /// Bond Data (see #bap_uc_srv_bond_data_ind_t)
    BAP_UC_SRV_BOND_DATA = GAF_CODE(BAP, UC_SRV, 0),
    /// ASE State (see #bap_uc_srv_ase_state_ind_t)
    BAP_UC_SRV_ASE_STATE = GAF_CODE(BAP, UC_SRV, 1),
    /// CIS State (see #bap_uc_srv_cis_state_ind_t)
    BAP_UC_SRV_CIS_STATE = GAF_CODE(BAP, UC_SRV, 2),
};

/// List of GAF_REQ_IND request indication code values for Unicast Server module
enum bap_uc_srv_msg_req_ind_code
{
    /// Configure Codec for an ASE (see #bap_uc_srv_configure_codec_req_ind_t)
    BAP_UC_SRV_CONFIGURE_CODEC_RI = GAF_CODE(BAP, UC_SRV, 0),
    /// Enable an ASE (see #bap_uc_srv_enable_req_ind_t)
    BAP_UC_SRV_ENABLE = GAF_CODE(BAP, UC_SRV, 1),
    /// Update Metadata for an ASE (see #bap_uc_srv_update_metadata_req_ind_t)
    BAP_UC_SRV_UPDATE_METADATA_RI = GAF_CODE(BAP, UC_SRV, 2),
    /// Release an ASE (see #bap_uc_srv_release_req_ind_t)
    BAP_UC_SRV_RELEASE_RI = GAF_CODE(BAP, UC_SRV, 3),
    /// Update Data Path (see #bap_uc_srv_dp_update_req_ind_t)
    BAP_UC_SRV_DP_UPDATE = GAF_CODE(BAP, UC_SRV, 4),
    /// Configure QoS for an ASE (see #bap_uc_srv_configure_qos_req_ind_t)
    BAP_UC_SRV_CONFIGURE_QOS_RI = GAF_CODE(BAP, UC_SRV, 5),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #BAP_UC_SRV_RESTORE_BOND_DATA request message (see \ref bap_msc_ascs_reconnection)
typedef struct bap_uc_srv_restore_bond_data_req
{
    /// Request code (shall be set to #BAP_UC_SRV_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field for Audio Stream Control Service\n
    /// Each bit correspond to a characteristic in the range [0, BAP_UC_CHAR_TYPE_ASE[
    uint8_t cli_cfg_bf;
    /// Client configuration bit field for instances of the ASE characteristics\n
    /// Each bit correspond to an instance of the ASE characteristic
    uint16_t ase_cli_cfg_bf;
    /// Event configuration bit field for instances of the ASE characteristics\n
    /// Each bit correspond to an instance of the ASE characteristic\n
    /// Allow to indicate that ASE has switched to Idle state.
    uint16_t ase_evt_cfg_bf;
} bap_uc_srv_restore_bond_data_req_t;

/// Structure for #BAP_UC_SRV_RESTORE_BOND_DATA_CODEC request message (see \ref bap_msc_ascs_reconnection)
typedef struct bap_uc_srv_restore_bond_data_codec_req
{
    /// Request code (shall be set to #BAP_UC_SRV_RESTORE_BOND_DATA_CODEC)
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
    /// QoS Requirements
    bap_qos_req_t qos_req;
    /// Controller delay in microseconds
    uint32_t ctl_delay_us;
    /// Codec Configuration structure
    bap_cfg_t cfg;
} bap_uc_srv_restore_bond_data_codec_req_t;

/// Structure for #BAP_UC_SRV_CONFIGURE_CODEC request message (see \ref bap_msc_ascs_configure_codec)
typedef struct bap_uc_srv_configure_codec_req
{
    /// Request code (shall be set to #BAP_UC_SRV_CONFIGURE_CODEC)
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
    /// QoS Requirements
    bap_qos_req_t qos_req;
    /// Controller delay in microseconds
    uint32_t ctl_delay_us;
    /// Codec Configuration structure
    bap_cfg_t cfg;
} bap_uc_srv_configure_codec_req_t;

/// Structure for #BAP_UC_SRV_UPDATE_METADATA request message (see \ref bap_msc_ascs_update_metadata)
typedef struct bap_uc_srv_update_metadata_req
{
    /// Request code (shall be set to #BAP_UC_SRV_UPDATE_METADATA)
    uint16_t req_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Metadata structure
    bap_cfg_metadata_t metadata;
} bap_uc_srv_update_metadata_req_t;

/// Structure for response message
typedef struct bap_uc_srv_rsp
{
    /// Request code (see #bap_uc_srv_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_srv_rsp_t;

/// Structure for #BAP_UC_SRV_DISABLE command message (see \ref bap_msc_ascs_disable)
typedef struct bap_uc_srv_disable_cmd
{
    /// Command code (shall be set to #BAP_UC_SRV_DISABLE)
    uint16_t cmd_code;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_srv_disable_cmd_t;

/// Structure for #BAP_UC_SRV_RELEASE command message (see \ref bap_msc_ascs_release)
typedef struct bap_uc_srv_release_cmd
{
    /// Command code (shall be set to #BAP_UC_SRV_RELEASE)
    uint16_t cmd_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Indicate if ASE next state will be Idle or Codec Configured
    uint8_t idle;
} bap_uc_srv_release_cmd_t;

/// Structure for #BAP_UC_SRV_GET_QUALITY command message (see \ref bap_msc_ascs_get_quality)
typedef struct bap_uc_srv_get_quality_cmd
{
    /// Command code (shall be set to #BAP_UC_SRV_GET_QUALITY)
    uint16_t cmd_code;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_srv_get_quality_cmd_t;

/// Structure for command complete event message
typedef struct bap_uc_srv_cmp_evt
{
    /// Command code (see #bap_uc_srv_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_srv_cmp_evt_t;

/// Structure for #BAP_UC_SRV_GET_QUALITY command complete event message
typedef struct bap_uc_srv_get_quality_cmp_evt
{
    /// Command code (set to #BAP_UC_SRV_GET_QUALITY)
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
} bap_uc_srv_get_quality_cmp_evt_t;

/// Structure for #BAP_UC_SRV_BOND_DATA indication message
typedef struct bap_uc_srv_bond_data_ind
{
    /// Indication code (set to #BAP_UC_SRV_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field for Audio Stream Control Service\n
    /// Each bit correspond to a characteristic in the range [0, BAP_UC_CHAR_TYPE_ASE[
    uint8_t cli_cfg_bf;
    /// Client configuration bit field for instances of the ASE characteristics\n
    /// Each bit correspond to an instance of the ASE characteristic
    uint16_t ase_cli_cfg_bf;
} bap_uc_srv_bond_data_ind_t;

/// Structure for #BAP_UC_SRV_ASE_STATE indication message
typedef struct bap_uc_srv_ase_state_ind
{
    /// Indication code (set to #BAP_UC_SRV_ASE_STATE)
    uint16_t ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Connection local index
    uint8_t con_lid;
    /// State (see #bap_uc_ase_state enumeration)\n
    /// Meaningful only if con_lid != GAF_INVALID_LID
    uint8_t state;
    /// QoS Configuration\n
    /// Meaningful only if state is meaningful and ASE state is QoS Configured, Enabling, Streaming or Disabling
    bap_qos_cfg_t qos_cfg;
} bap_uc_srv_ase_state_ind_t;

/// Structure for #BAP_UC_SRV_CIS_STATE indication message
typedef struct bap_uc_srv_cis_state_ind
{
    /// Indication code (set to #BAP_UC_SRV_CIS_STATE)
    uint16_t ind_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Connection local index of LE connection the CIS is bound with
    uint8_t con_lid;
    /// ASE local index for Sink direction
    uint8_t ase_lid_sink;
    /// ASE local index for Source direction
    uint8_t ase_lid_src;
    /// CIG ID
    uint8_t cig_id;
    /// CIS ID
    uint8_t cis_id;
    /// Connection handle allocated by the controller\n
    /// GAP_INVALID_CONHDL means that the CIS is not established
    uint16_t conhdl;
    /// Group configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    gapi_ug_config_t cig_config;
    /// Stream configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    gapi_us_config_t cis_config;
} bap_uc_srv_cis_state_ind_t;

/// Structure for #BAP_UC_SRV_CONFIGURE_CODEC_RI request indication message
typedef struct bap_uc_srv_configure_codec_req_ind
{
    /// Request indication code (set to #BAP_UC_SRV_CONFIGURE_CODEC_RI)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// ASE instance index
    uint8_t ase_instance_idx;
    /// ASE local index
    uint8_t ase_lid;
    /// Target Latency (see #bap_uc_tgt_latency enumeration)
    uint8_t tgt_latency;
    /// Target PHY (see #bap_uc_tgt_phy enumeration)
    uint8_t tgt_phy;
    /// Codec ID
    gaf_codec_id_t codec_id;
    /// Codec Configuration structure
    bap_cfg_t cfg;
} bap_uc_srv_configure_codec_req_ind_t;

/// Structure for #BAP_UC_SRV_CONFIGURE_QOS_RI request indication message
typedef struct bap_uc_srv_configure_qos_req_ind
{
    /// Request indication code (set to #BAP_UC_SRV_CONFIGURE_QOS_RI)
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Stream local index (#GAF_INVALID_LID if stream is not already used)
    uint8_t stream_lid;
    /// QoS Configuration structure
    bap_qos_cfg_t qos_cfg;
} bap_uc_srv_configure_qos_req_ind_t;

/// Structure for #BAP_UC_SRV_ENABLE request indication message
typedef struct bap_uc_srv_enable_req_ind
{
    /// Request indication code (set to #BAP_UC_SRV_ENABLE)
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Metadata structure
    bap_cfg_metadata_t metadata;
} bap_uc_srv_enable_req_ind_t;

/// Structure for #BAP_UC_SRV_UPDATE_METADATA request indication message
typedef struct bap_uc_srv_update_metadata_req_ind
{
    /// Request indication code (set to #BAP_UC_SRV_UPDATE_METADATA)
    ///  - BAP_UC_SRV_UPDATE_METADATA_RI
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Metadata structure
    bap_cfg_metadata_t metadata;
} bap_uc_srv_update_metadata_req_ind_t;

/// Structure for #BAP_UC_SRV_RELEASE_RI request indication message
typedef struct bap_uc_srv_release_req_ind
{
    /// Request indication code (set to #BAP_UC_SRV_RELEASE_RI)
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_srv_release_req_ind_t;

/// Structure for #BAP_UC_SRV_DP_UPDATE request indication message
typedef struct bap_uc_srv_dp_update_req_ind
{
    /// Request indication code (set to #BAP_UC_SRV_DP_UPDATE)
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Indicate if Data Path is about to be started or stopped for the ASE
    bool start;
} bap_uc_srv_dp_update_req_ind_t;

/// Structure for #BAP_UC_SRV_DP_UPDATE confirmation message
typedef struct bap_uc_srv_dp_update_cfm
{
    /// Request indication code (shall be set to #BAP_UC_SRV_DP_UPDATE)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
} bap_uc_srv_dp_update_cfm_t;

/// Structure for #BAP_UC_SRV_CONFIGURE_CODEC_RI confirmation message
typedef struct bap_uc_srv_configure_codec_cfm
{
    /// Request indication code (shall be set to #BAP_UC_SRV_CONFIGURE_CODEC_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Response code (see #bap_uc_cp_rsp_code enumeration)
    uint8_t rsp_code;
    /// Reason (see #bap_uc_cp_reason enumeration)
    uint8_t reason;
    /// Allocated ASE local index
    uint8_t ase_lid;
    /// Data Path Configuration bit field (see #bap_dp_cfg_bf enumeration)
    uint16_t dp_cfg_bf;
    /// Codec ID
    gaf_codec_id_t codec_id;
    /// QoS Requirements
    bap_qos_req_t qos_req;
    /// Controller Delay in microseconds
    uint32_t ctl_delay_us;
    /// Codec Configuration structure
    bap_cfg_t cfg;
} bap_uc_srv_configure_codec_cfm_t;

/// Structure for #BAP_UC_SRV_CONFIGURE_QOS_RI confirmation message
typedef struct bap_uc_srv_configure_qos_cfm
{
    /// Request indication code (shall be set to #BAP_UC_SRV_CONFIGURE_QOS_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
    /// Response code (see #bap_uc_cp_rsp_code enumeration)
    uint8_t rsp_code;
    /// Reason (see #bap_uc_cp_reason enumeration)
    uint8_t reason;
} bap_uc_srv_configure_qos_cfm_t;

/// Structure for #BAP_UC_SRV_ENABLE confirmation message
typedef struct bap_uc_srv_enable_cfm
{
    /// Request indication code (shall be set to #BAP_UC_SRV_ENABLE)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
    /// Response code (see #bap_uc_cp_rsp_code enumeration)
    uint8_t rsp_code;
    /// Reason (see #bap_uc_cp_reason enumeration)
    uint8_t reason;
    /// Metadata structure
    bap_cfg_metadata_t metadata;
} bap_uc_srv_enable_cfm_t;

/// Structure for #BAP_UC_SRV_UPDATE_METADATA confirmation message
typedef struct bap_uc_srv_update_metadata_cfm
{
    /// Request indication code (shall be set to #BAP_UC_SRV_UPDATE_METADATA)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
    /// Response code (see #bap_uc_cp_rsp_code enumeration)
    uint8_t rsp_code;
    /// Reason (see #bap_uc_cp_reason enumeration)
    uint8_t reason;
    /// Metadata structure
    bap_cfg_metadata_t metadata;
} bap_uc_srv_update_metadata_cfm_t;

/// Structure for #BAP_UC_SRV_RELEASE_RI confirmation message
typedef struct bap_uc_srv_release_cfm
{
    /// Request indication code (shall be set to #BAP_UC_SRV_RELEASE_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
    /// Response code (see #bap_uc_cp_rsp_code enumeration)
    uint8_t rsp_code;
    /// Reason (see #bap_uc_cp_reason enumeration)
    uint8_t reason;
    /// Indicate if ASE next state will be Idle or Codec Configured
    uint8_t idle;
} bap_uc_srv_release_cfm_t;

/// @} BAP_UC_SRV_MSG

#endif // BAP_UC_SRV_MSG_H_
