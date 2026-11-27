/**
 ****************************************************************************************
 *
 * @file bap_bc_src_msg.h
 *
 * @brief Basic Audio Profile - Broadcast Source - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_SRC_MSG_H_
#define BAP_BC_SRC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC_SRC_MSG Message API
 * @ingroup BAP_BC_SRC
 * @brief Description of Message API for Broadcast Source module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap_bc_src.h"     // Basic Audio Profile - Broadcast Source Definitions
#include "bap_msg.h"        // Message API Definitions

/// @addtogroup BAP_BC_SRC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Broadcast Source module
enum bap_bc_src_msg_cmd_code
{
    /// Enable Periodic Advertising for a Broadcast Group (see #bap_bc_src_enable_pa_cmd_t)
    BAP_BC_SRC_ENABLE_PA = GAF_CODE(BAP, BC_SRC, BAP_BC_SRC_CMD_TYPE_ENABLE_PA),
    /// Enable a Broadcast Group (see #bap_bc_src_enable_cmd_t)
    BAP_BC_SRC_ENABLE = GAF_CODE(BAP, BC_SRC, BAP_BC_SRC_CMD_TYPE_ENABLE),
    /// Start transmission of audio data on either all streams or on a subset of streams within a Broadcast Group
    /// (see #bap_bc_src_start_streaming_cmd_t)
    BAP_BC_SRC_START_STREAMING = GAF_CODE(BAP, BC_SRC, BAP_BC_SRC_CMD_TYPE_START_STREAMING),
    /// Stop transmission of audio data for either all streams or for a subset of streams within a Broadcast Group
    /// (see #bap_bc_src_stop_streaming_cmd_t)
    BAP_BC_SRC_STOP_STREAMING = GAF_CODE(BAP, BC_SRC, BAP_BC_SRC_CMD_TYPE_STOP_STREAMING),
    /// Disable Periodic Advertising for a Broadcast Group
    /// (see #bap_bc_src_disable_pa_cmd_t)
    BAP_BC_SRC_DISABLE_PA = GAF_CODE(BAP, BC_SRC, BAP_BC_SRC_CMD_TYPE_DISABLE_PA),
    /// Disable a Broadcast Group (see #bap_bc_src_disable_cmd_t)
    BAP_BC_SRC_DISABLE = GAF_CODE(BAP, BC_SRC, BAP_BC_SRC_CMD_TYPE_DISABLE),
    /// Update Metadata (see #bap_bc_src_update_metadata_cmd_t)
    BAP_BC_SRC_UPDATE_METADATA = GAF_CODE(BAP, BC_SRC, BAP_BC_SRC_CMD_TYPE_UPDATE_METADATA),
    /// Remove a Broadcast Group (see #bap_bc_src_remove_group_cmd_t)
    BAP_BC_SRC_REMOVE_GROUP = GAF_CODE(BAP, BC_SRC, BAP_BC_SRC_CMD_TYPE_REMOVE_GROUP),
};

/// List of GAF_REQ request code values for Broadcast Source module
enum bap_bc_src_msg_req_code
{
    /// Add a Broadcast Group (see #bap_bc_src_add_group_req_t)
    BAP_BC_SRC_ADD_GROUP = GAF_CODE(BAP, BC_SRC, 0),
    /// Configure or reconfigure a Broadcast Subgroup (see #bap_bc_src_set_subgroup_req_t)
    BAP_BC_SRC_SET_SUBGROUP = GAF_CODE(BAP, BC_SRC, 1),
    /// Configure or reconfigure a Broadcast Stream and set the Broadcast Subgroup it belongs to
    /// (see #bap_bc_src_set_stream_req_t)
    BAP_BC_SRC_SET_STREAM = GAF_CODE(BAP, BC_SRC, 2),
};

/// List of GAF_IND indication code values for Broadcast Source module
enum bap_bc_src_msg_ind_code
{
    /// Group information once created at controller level (see #bap_bc_src_info_ind_t)
    BAP_BC_SRC_INFO = GAF_CODE(BAP, BC_SRC, 0),
};


/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for response message
typedef struct bap_bc_src_rsp
{
    /// Request code (see #bap_bc_src_msg_req_code enumeration)
    uint16_t req_code;
    /// Status of request execution
    uint16_t status;
    /// Group local index
    uint8_t grp_lid;
    /// Subgroup local index
    uint8_t sgrp_lid;
    /// Stream local index
    uint8_t stream_lid;
} bap_bc_src_rsp_t;

/// Structure for #BAP_BC_SRC_ADD_GROUP request message
typedef struct bap_bc_src_add_group_req
{
    /// Request code (shall be set to #BAP_BC_SRC_ADD_GROUP)
    uint16_t req_code;
    /// Broadcast ID
    bap_bcast_id_t bcast_id;
    /// Number of Streams in the Broadcast Group. Cannot be 0
    uint8_t nb_streams;
    /// Number of Subgroups in the Broadcast Group. Cannot be 0
    uint8_t nb_subgroups;
    /// Broadcast Group parameters
    bap_bc_grp_param_t grp_param;
    /// Advertising parameters
    bap_bc_adv_param_t adv_param;
    /// Periodic Advertising parameters
    bap_bc_per_adv_param_t per_adv_param;
    /// Presentation Delay in microseconds
    uint32_t pres_delay_us;
    /// Indicate if Streams are encrypted (!= 0) or not
    uint8_t encrypted;
    /// Broadcast code\n
    /// Meaningful only if Streams are encrypted
    gaf_bcast_code_t bcast_code;
} bap_bc_src_add_group_req_t;

/// Structure for #BAP_BC_SRC_SET_SUBGROUP request message
typedef struct bap_bc_src_set_subgroup_req
{
    /// Request code (shall be set to #BAP_BC_SRC_SET_SUBGROUP)
    uint16_t req_code;
    /// Group local index
    uint8_t grp_lid;
    /// Subgroup local index
    uint8_t sgrp_lid;
    /// Codec ID value
    gaf_codec_id_t codec_id;
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
} bap_bc_src_set_subgroup_req_t;

/// Structure for #BAP_BC_SRC_SET_STREAM request message
typedef struct bap_bc_src_set_stream_req
{
    /// Request code (shall be set to #BAP_BC_SRC_SET_STREAM)
    uint16_t req_code;
    /// Group local index
    uint8_t grp_lid;
    /// Subgroup local index of Subgroup the Stream belongs to
    uint8_t sgrp_lid;
    /// Stream local index
    uint8_t stream_lid;
    /// Controller delay in microseconds
    uint32_t ctl_delay_us;
    /// Data Path Configuration bit field (see #bap_dp_cfg_bf enumeration)
    uint16_t dp_cfg_bf;
    /// Codec Configuration structure
    bap_cfg_t cfg;
} bap_bc_src_set_stream_req_t;

/// Structure for command complete message
typedef struct bap_bc_src_cmp_evt
{
    /// Command code (see #bap_bc_src_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Group local index
    uint8_t grp_lid;
    /// Subgroup local index
    uint8_t sgrp_lid;
} bap_bc_src_cmp_evt_t;

/// Structure for #BAP_BC_SRC_ENABLE_PA command message
typedef struct bap_bc_src_enable_pa_cmd
{
    /// Command code (shall be set to #BAP_BC_SRC_ENABLE_PA)
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
    /// Length of additional advertising data
    uint16_t adv_data_len;
    /// Length of additional Periodic Advertising data
    uint16_t per_adv_data_len;
    /// Length of Public Broadcast Source Name
    uint8_t broadcast_name_len;
    /// Length of Public Broadcast Source Metadata
    uint8_t metadata_len;
    /// Additional advertising data followed by additional Periodic Advertising data followed by Public Broadcast
    /// Source Name followed by Public Broadcast Source Metadata
    uint8_t data[__ARRAY_EMPTY];
} bap_bc_src_enable_pa_cmd_t;

/// Structure for #BAP_BC_SRC_ENABLE command message
typedef struct bap_bc_src_enable_cmd
{
    /// Command code (shall be set to #BAP_BC_SRC_ENABLE)
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
} bap_bc_src_enable_cmd_t;

/// Structure for #BAP_BC_SRC_START_STREAMING command message
typedef struct bap_bc_src_start_streaming_cmd
{
    /// Command code (shall be set to #BAP_BC_SRC_START_STREAMING)
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
    /// Stream local index bit field indicating for which Streams streaming must be started\n
    /// 0xFFFFFFFF means that streaming must be started for all Streams
    uint32_t stream_lid_bf;
} bap_bc_src_start_streaming_cmd_t;

/// Structure for #BAP_BC_SRC_STOP_STREAMING command message
typedef struct bap_bc_src_stop_streaming_cmd
{
    /// Command code (shall be set to #BAP_BC_SRC_STOP_STREAMING)
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
    /// Stream local index bit field indicating for which Streams streaming must be stopped\n
    /// 0xFFFFFFFF means that streaming must be stopped for all Streams
    uint32_t stream_lid_bf;
} bap_bc_src_stop_streaming_cmd_t;

/// Structure for #BAP_BC_SRC_DISABLE_PA command message
typedef struct bap_bc_src_disable_pa_cmd
{
    /// Command code (shall be set to #BAP_BC_SRC_DISABLE_PA)
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
} bap_bc_src_disable_pa_cmd_t;

/// Structure for #BAP_BC_SRC_DISABLE command message
typedef struct bap_bc_src_disable_cmd
{
    /// Command code (shall be set to #BAP_BC_SRC_DISABLE)
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
    /// Indicate if Periodic Advertising must also be disabled
    uint8_t disable_pa;
} bap_bc_src_disable_cmd_t;

/// Structure for #BAP_BC_SRC_UPDATE_METADATA command message
typedef struct bap_bc_src_update_metadata_cmd
{
    /// Command code (shall be set to #BAP_BC_SRC_UPDATE_METADATA)
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
    /// Subgroup local index
    uint8_t sgrp_lid;
    /// Metadata
    bap_cfg_metadata_t metadata;
} bap_bc_src_update_metadata_cmd_t;

/// Structure for #BAP_BC_SRC_REMOVE_GROUP command message
typedef struct bap_bc_src_remove_group_cmd
{
    /// Command code (shall be set to #BAP_BC_SRC_REMOVE_GROUP)
    uint16_t cmd_code;
    /// Group local index
    uint8_t grp_lid;
} bap_bc_src_remove_group_cmd_t;

/// Structure for #BAP_BC_SRC_INFO indication message
typedef struct bap_bc_src_info_ind
{
    /// Indication code (shall be set to #BAP_BC_SRC_INFO)
    uint16_t ind_code;
    /// Group local index
    uint8_t grp_lid;
    /// Number of BISes
    uint8_t nb_bis;
    /// Broadcast Group configuration
    gapi_bg_config_t bg_cfg;
    /// List of Connection Handle values provided by the Controller (nb_bis elements)
    uint16_t conhdl[__ARRAY_EMPTY];
} bap_bc_src_info_ind_t;

/// @} BAP_BC_SRC_MSG

#endif // BAP_BC_SRC_MSG_H_
