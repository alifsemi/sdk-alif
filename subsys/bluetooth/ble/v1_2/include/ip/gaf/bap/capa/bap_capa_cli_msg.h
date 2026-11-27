/**
 ****************************************************************************************
 *
 * @file bap_capa_cli_msg.h
 *
 * @brief Basic Audio Profile - Capabilities Client - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_CAPA_CLI_MSG_H_
#define BAP_CAPA_CLI_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_CLI_MSG Message API
 * @ingroup BAP_CAPA_CLI
 * @brief Description of Message API for Capabilities Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap_msg.h"         // Basic Audio Profile Kernel Messages Definitions
#include "bap_capa_cli.h"    // Basic Audio Profile - Capabilities Client Definitions

/// @addtogroup BAP_CAPA_CLI_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Capabilities Client module
enum bap_capa_cli_msg_cmd_code
{
    /// Discover (see #bap_capa_cli_discover_cmd_t)
    BAP_CAPA_CLI_DISCOVER = GAF_CODE(BAP, CAPA_CLI, BAP_CAPA_CLI_CMD_TYPE_DISCOVER),
    /// Get (see #bap_capa_cli_get_cmd_t)
    BAP_CAPA_CLI_GET = GAF_CODE(BAP, CAPA_CLI, BAP_CAPA_CLI_CMD_TYPE_GET),
    /// Set Configuration (see #bap_capa_cli_set_cfg_cmd_t)
    BAP_CAPA_CLI_SET_CFG = GAF_CODE(BAP, CAPA_CLI, BAP_CAPA_CLI_CMD_TYPE_SET_CFG),
    /// Set Audio Locations (see #bap_capa_cli_set_location_cmd_t)
    BAP_CAPA_CLI_SET_LOCATION = GAF_CODE(BAP, CAPA_CLI, BAP_CAPA_CLI_CMD_TYPE_SET_LOCATION),
};

/// List of GAF_REQ request code values for Capabilities Client module
enum bap_capa_cli_msg_req_code
{
    /// Restore Bond Data (see #bap_capa_cli_restore_bond_data_req_t)
    BAP_CAPA_CLI_RESTORE_BOND_DATA = GAF_CODE(BAP, CAPA_CLI, 0),
};

/// List of GAF_IND indication codes for Capabilities Client
enum bap_capa_cli_msg_ind_code
{
    /// Bond Data (see #bap_capa_cli_bond_data_ind_t)
    BAP_CAPA_CLI_BOND_DATA = GAF_CODE(BAP, CAPA_CLI, 0),
    /// PAC Record (see #bap_capa_cli_record_ind_t)
    BAP_CAPA_CLI_RECORD = GAF_CODE(BAP, CAPA_CLI, 1),
    /// Audio Locations (see #bap_capa_cli_location_ind_t)
    BAP_CAPA_CLI_LOCATION = GAF_CODE(BAP, CAPA_CLI, 2),
    /// Available or Supported Audio Contexts (see #bap_capa_cli_context_ind_t)
    BAP_CAPA_CLI_CONTEXT = GAF_CODE(BAP, CAPA_CLI, 3),
    /// Service Changed (see #bap_capa_cli_svc_changed_ind_t)
    BAP_CAPA_CLI_SVC_CHANGED = GAF_CODE(BAP, CAPA_CLI, 5),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #BAP_CAPA_CLI_DISCOVER command message
typedef struct bap_capa_cli_discover_cmd
{
    /// Command code (shall be set to #BAP_CAPA_CLI_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Start handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t shdl;
    /// End handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t ehdl;
} bap_capa_cli_discover_cmd_t;

/// Structure for #BAP_CAPA_CLI_GET command message
typedef struct bap_capa_cli_get_cmd
{
    /// Command code (shall be set to #BAP_CAPA_CLI_GET)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type (see #bap_capa_char_type enumeration)
    uint8_t char_type;
    /// PAC local index
    uint8_t pac_lid;
} bap_capa_cli_get_cmd_t;

/// Structure for #BAP_CAPA_CLI_SET_CFG command message
typedef struct bap_capa_cli_set_cfg_cmd
{
    /// Command code (shall be set to #BAP_CAPA_CLI_SET_CFG)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type (see #bap_capa_char_type enumeration)
    uint8_t char_type;
    /// PAC local index
    uint8_t pac_lid;
    /// Enable
    uint8_t enable;
} bap_capa_cli_set_cfg_cmd_t;

/// Structure for #BAP_CAPA_CLI_SET_LOCATION command message
typedef struct bap_capa_cli_set_location_cmd
{
    /// Command code (shall be set to #BAP_CAPA_CLI_SET_LOCATION)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Direction (see #gaf_direction enumeration)
    uint8_t direction;
    /// Location bit field (see #gaf_loc_bf enumeration)
    uint32_t location_bf;
} bap_capa_cli_set_location_cmd_t;

/// Structure for command complete event message
typedef struct bap_capa_cli_cmp_evt
{
    /// Command code (see #bap_capa_cli_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Union
    union
    {
        uint8_t param;
        /// Direction (see #gaf_direction enumeration)
        /// Meaningful only for BAP_CAPA_CLI_SET_LOCATION command
        uint8_t direction;
        /// Characteristic type (see #bap_capa_char_type enumeration)
        /// Meaningless for BAP_CAPA_CLI_SET_LOCATION and BAP_CAPA_CLI_DISCOVER commands
        uint8_t char_type;
    } u;
    /// PAC local index
    /// Meaningless for BAP_CAPA_CLI_SET_LOCATION and BAP_CAPA_CLI_DISCOVER commands
    uint8_t pac_lid;
} bap_capa_cli_cmp_evt_t;

/// Structure for #BAP_CAPA_CLI_RESTORE_BOND_DATA request message
typedef struct bap_capa_cli_restore_bond_data_req
{
    /// Request code (shall be set to #BAP_CAPA_CLI_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Published Audio Capabilities Service content description
    bap_capa_cli_pacs_t pacs_info;
} bap_capa_cli_restore_bond_data_req_t;

/// Structure for response message
typedef struct bap_capa_cli_rsp
{
    /// Request code (see #bap_capa_cli_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} bap_capa_cli_rsp_t;

/// Structure for #BAP_CAPA_CLI_BOND_DATA indication message
typedef struct bap_capa_cli_bond_data_ind
{
    /// Indication code (shall be set to #BAP_CAPA_CLI_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Published Audio Capabilities Service content description
    bap_capa_cli_pacs_t pacs_info;
} bap_capa_cli_bond_data_ind_t;

/// Structure for #BAP_CAPA_CLI_RECORD indication message
typedef struct bap_capa_cli_record_ind
{
    /// Indication code (shall be set to #BAP_CAPA_CLI_RECORD)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// PAC local index
    uint8_t pac_lid;
    /// Record local index
    uint8_t record_lid;
    /// Number of records
    uint8_t nb_records;
    /// Codec ID
    gaf_codec_id_t codec_id;
    /// Codec Capabilities parameters structure
    bap_capa_param_t param;
    /// Codec Capabilities Metadata parameters structure
    bap_capa_metadata_param_t param_metadata;
    /// Length of additional Codec Capabilities
    uint8_t add_capa_len;
    /// Length of additional Metadata
    uint8_t add_metadata_len;
    /// Additional Codec Capabilities (in LTV format) followed by additional Metadata (in LTV format)\n
    /// Length of array is add_capa_len + add_metadata_len
    uint8_t val[__ARRAY_EMPTY];
} bap_capa_cli_record_ind_t;

/// Structure for #BAP_CAPA_CLI_CONTEXT indication message
typedef struct bap_capa_cli_context_ind
{
    /// Indication code (shall be set to #BAP_CAPA_CLI_CONTEXT)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Context type (see #bap_capa_context_type enumeration)
    uint8_t context_type;
    /// Context type bit field for Sink direction
    uint16_t context_bf_sink;
    /// Context type bit field for Source direction
    uint16_t context_bf_src;
} bap_capa_cli_context_ind_t;

/// Structure for #BAP_CAPA_CLI_LOCATION indication message
typedef struct bap_capa_cli_location_ind
{
    /// Indication code (shall be set to #BAP_CAPA_CLI_LOCATION)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Direction (see #gaf_direction enumeration)
    uint8_t direction;
    /// Location bit field (see #gaf_loc_bf enumeration)
    uint32_t location_bf;
} bap_capa_cli_location_ind_t;

/// Structure for #BAP_CAPA_CLI_SVC_CHANGED indication message
typedef struct bap_capa_cli_svc_changed_ind
{
    /// Indication code (shall be set to #BAP_CAPA_CLI_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} bap_capa_cli_svc_changed_ind_t;

/// @} BAP_CAPA_CLI_MSG

#endif // BAP_CAPA_CLI_MSG_H_
