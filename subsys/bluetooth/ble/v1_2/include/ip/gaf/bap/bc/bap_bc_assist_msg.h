/**
 ****************************************************************************************
 *
 * @file bap_bc_assist_msg.h
 *
 * @brief Basic Audio Profile - Broadcast Assistant - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_ASSIST_MSG_H_
#define BAP_BC_ASSIST_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC_ASSIST_MSG Message API
 * @ingroup BAP_BC_ASSIST
 * @brief Description of Message API for Broadcast Assistant module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap_msg.h"         // Basic Audio Profile API Messages Definitions
#include "bap_bc_assist.h"   // Basic Audio Profile - Broadcast Assistant Definitions

/// @addtogroup BAP_BC_ASSIST_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for BAP Broadcast Assistant module
enum bap_bc_assist_msg_cmd_code
{
    /// Start discovery of Solicitation Requests sent by Delegator devices (see #bap_bc_assist_start_scan_cmd_t)
    BAP_BC_ASSIST_START_SCAN = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_START_SCAN),
    /// Stop discovery of Solicitation Requests (see #bap_bc_assist_stop_scan_cmd_t)
    BAP_BC_ASSIST_STOP_SCAN = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_STOP_SCAN),
    /// Discover Broadcast Audio Scan Service in peer device database (see #bap_bc_assist_discover_cmd_t)
    BAP_BC_ASSIST_DISCOVER = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_DISCOVER),
    /// Get value of an instance of Broadcast Receive State characteristic (see #bap_bc_assist_get_state_cmd_t)
    BAP_BC_ASSIST_GET_STATE = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_GET_STATE),
    /// Enable or disable sending of notifications for an instance of the Broadcast Receive State characteristic
    /// (see #bap_bc_assist_set_cfg_cmd_t)
    BAP_BC_ASSIST_SET_CFG = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_SET_CFG),
    /// Inform peer Delegator device that Assistant has either started or stopped scan on its behalf
    /// (see #bap_bc_assist_update_scan_cmd_t)
    BAP_BC_ASSIST_UPDATE_SCAN = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_UPDATE_SCAN),
    /// Add a Broadcast Source (see #bap_bc_assist_add_source_cmd_t)
    BAP_BC_ASSIST_ADD_SOURCE = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_ADD_SOURCE),
    /// Add a Broadcast Source locally created (Source and Assistant colocated)
    /// (see #bap_bc_assist_add_source_local_cmd_t)
    BAP_BC_ASSIST_ADD_SOURCE_LOCAL = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_ADD_SOURCE_LOCAL),
    /// Remove a Broadcast Source (see #bap_bc_assist_remove_source_cmd_t)
    BAP_BC_ASSIST_REMOVE_SOURCE = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_REMOVE_SOURCE),
    /// Update a Broadcast Source (see #bap_bc_assist_modify_source_cmd_t)
    BAP_BC_ASSIST_MODIFY_SOURCE = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_MODIFY_SOURCE),
    /// Update a Broadcast Source locally created (Source and Assistant colocated)
    /// (see #bap_bc_assist_modify_source_local_cmd_t)
    BAP_BC_ASSIST_MODIFY_SOURCE_LOCAL = GAF_CODE(BAP, BC_ASSIST, BAP_BC_ASSIST_CMD_TYPE_MODIFY_SOURCE_LOCAL),
};

/// List of GAF_REQ request code values for BAP Broadcast Assistant module
enum bap_bc_assist_msg_req_code
{
    /// Restore Bond Data after reconnection with a device with which a trusted relationship has been established
    /// (see #bap_bc_assist_restore_bond_data_req_t)
    BAP_BC_ASSIST_RESTORE_BOND_DATA = GAF_CODE(BAP, BC_ASSIST, 0),
    /// Provide synchronization information and metadata for a Subgroup (see #bap_bc_assist_set_sgrp_info_req_t)
    BAP_BC_ASSIST_SET_SGRP_INFO = GAF_CODE(BAP, BC_ASSIST, 1),
    /// Update write type used for Broadcast Source Control Point characteristic - Mainly for PTS testing purpose
    BAP_BC_ASSIST_SET_WRITE_TYPE = GAF_CODE(BAP, BC_ASSIST, 2),
};

/// List of GAF_IND indication code values for BAP Broadcast Assistant module
enum bap_bc_assist_msg_ind_code
{
    /// Indicate that scan for Solicitation Requests has been stopped due to a timeout
    /// (see #bap_bc_assist_scan_timeout_ind_t)
    BAP_BC_ASSIST_SCAN_TIMEOUT = GAF_CODE(BAP, BC_ASSIST, 0),
    /// Indicate that a Solicitation Request has been received (see #bap_bc_assist_solicitation_ind_t)
    BAP_BC_ASSIST_SOLICITATION = GAF_CODE(BAP, BC_ASSIST, 1),
    /// Indicate that Broadcast Audio Scan Service has been discovered and provides its content
    /// (see #bap_bc_assist_bond_data_ind_t)
    BAP_BC_ASSIST_BOND_DATA = GAF_CODE(BAP, BC_ASSIST, 2),
    /// Indicate state of a Broadcast Source (see #bap_bc_assist_source_state_ind_t)
    BAP_BC_ASSIST_SOURCE_STATE = GAF_CODE(BAP, BC_ASSIST, 4),
    /// Indicate state of a Subgroup for a Broadcast Source (see #bap_bc_assist_source_state_subgroup_ind_t)
    BAP_BC_ASSIST_SOURCE_STATE_SGRP = GAF_CODE(BAP, BC_ASSIST, 5),
    /// Indicate that peer device has indicated a Service Changed impacting the discovered Broadcast Audio Scan
    /// Service (see #bap_bc_assist_svc_changed_ind_t)
    BAP_BC_ASSIST_SVC_CHANGED = GAF_CODE(BAP, BC_ASSIST, 6),
};

/// List of GAF_REQ_IND request indication code values for BAP Broadcast Assistant module
enum bap_bc_assist_msg_req_ind_code
{
    /// Request Broadcast Code for a BIG (see #bap_bc_assist_bcast_code_req_ind_t)
    BAP_BC_ASSIST_BCAST_CODE = GAF_CODE(BAP, BC_ASSIST, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #BAP_BC_ASSIST_START_SCAN command message
typedef struct bap_bc_assist_start_scan_cmd
{
    /// Request code (shall be set to #BAP_BC_ASSIST_START_SCAN)
    uint16_t cmd_code;
    /// Timeout in seconds
    uint16_t timeout_s;
} bap_bc_assist_start_scan_cmd_t;

/// Structure for #BAP_BC_ASSIST_STOP_SCAN command message
typedef struct bap_bc_assist_stop_scan_cmd
{
    /// Request code (shall be set to #BAP_BC_ASSIST_STOP_SCAN)
    uint16_t cmd_code;
} bap_bc_assist_stop_scan_cmd_t;

/// Structure for #BAP_BC_ASSIST_DISCOVER command message
typedef struct bap_bc_assist_discover_cmd
{
    /// Command code (shall be set to #BAP_BC_ASSIST_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Start handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t shdl;
    /// End handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t ehdl;
} bap_bc_assist_discover_cmd_t;

/// Structure for #BAP_BC_ASSIST_GET_STATE command message
typedef struct bap_bc_assist_get_state_cmd
{
    /// Command code (shall be set to #BAP_BC_ASSIST_GET_STATE)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
} bap_bc_assist_get_state_cmd_t;

/// Structure for #BAP_BC_ASSIST_SET_CFG command message
typedef struct bap_bc_assist_set_cfg_cmd
{
    /// Command code (shall be set to #BAP_BC_ASSIST_SET_CFG)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
    /// Enable
    uint8_t enable;
} bap_bc_assist_set_cfg_cmd_t;

/// Structure for #BAP_BC_ASSIST_UPDATE_SCAN command message
typedef struct bap_bc_assist_update_scan_cmd
{
    /// Command code (shall be set to #BAP_BC_ASSIST_UPDATE_SCAN)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Started
    uint8_t started;
} bap_bc_assist_update_scan_cmd_t;

/// Structure for #BAP_BC_ASSIST_ADD_SOURCE command message
typedef struct bap_bc_assist_add_source_cmd
{
    /// Command code (shall be set to #BAP_BC_ASSIST_ADD_SOURCE)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Periodic Advertising identification
    bap_adv_id_t adv_id;
    /// Broadcast ID
    bap_bcast_id_t bcast_id;
    /// Required PA synchronization state
    uint8_t pa_sync;
    /// Periodic Advertising interval in frames\n
    /// From 0x0006 to 0xFFFE\n
    /// BAP_BC_UNKNOWN_PA_INTV indicates that interval is unknown
    uint16_t pa_intv_frames;
    /// Number of Subgroups\n
    /// From 0 to 32
    uint8_t nb_subgroups;
    /// Required BIS synchronization bit field for the first Subgroup\n
    /// Meaningful only if nb_subgroups != 0\n
    /// #BAP_BC_ASSIST_SET_SGRP_INFO request messages must follow for each additional Subgroup
    uint32_t bis_sync_bf;
    /// Metadata for the first Subgroup\n
    /// Meaningful only if nb_subgroups != 0
    bap_cfg_metadata_t metadata;
} bap_bc_assist_add_source_cmd_t;

/// Structure for #BAP_BC_ASSIST_ADD_SOURCE_LOCAL command message
typedef struct bap_bc_assist_add_source_local_cmd
{
    /// Command code (shall be set to #BAP_BC_ASSIST_ADD_SOURCE_LOCAL)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Group local index
    uint8_t grp_lid;
    /// Required PA synchronization state
    uint8_t pa_sync;
    /// Number of Subgroups\n
    /// From 0 to 32\n
    /// If not 0, shall be set to the number of Subgroups in the indicated Group
    uint8_t nb_subgroups;
    /// Required BIS synchronization bit field for each Subgroup\n
    /// Must be an array of nb_subgroups elements
    uint32_t bis_sync_bf[__ARRAY_EMPTY];
} bap_bc_assist_add_source_local_cmd_t;

/// Structure for #BAP_BC_ASSIST_MODIFY_SOURCE command message
typedef struct bap_bc_assist_modify_source_cmd
{
    /// Command code (shall be set to #BAP_BC_ASSIST_MODIFY_SOURCE)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
    /// Required PA synchronization state
    uint8_t pa_sync;
    /// Periodic Advertising interval in frames\n
    /// From 0x0006 to 0xFFFE\n
    /// BAP_BC_UNKNOWN_PA_INTV indicates that interval is unknown
    uint16_t pa_intv_frames;
    /// Number of Subgroups\n
    /// From 0 to 32
    uint8_t nb_subgroups;
    /// Required BIS synchronization bit field for the first Subgroup\n
    /// Meaningful only if nb_subgroups != 0\n
    /// #BAP_BC_ASSIST_SET_SGRP_INFO request messages must follow for each additional Subgroup
    uint32_t bis_sync_bf;
    /// Metadata for the first Subgroup\n
    /// Meaningful only if nb_subgroups != 0
    bap_cfg_metadata_t metadata;
} bap_bc_assist_modify_source_cmd_t;

/// Structure for #BAP_BC_ASSIST_MODIFY_SOURCE_LOCAL command message
typedef struct bap_bc_assist_modify_source_local_cmd
{
    /// Command code (shall be set to #BAP_BC_ASSIST_MODIFY_SOURCE_LOCAL)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
    /// Group local index
    uint8_t grp_lid;
    /// Required PA synchronization state
    uint8_t pa_sync;
    /// Number of Subgroups\n
    /// From 0 to 32\n
    /// If not 0, shall be set to the number of Subgroups in the indicated Group
    uint8_t nb_subgroups;
    /// Required BIS synchronization bit field for each Subgroup\n
    /// Must be an array of nb_subgroups elements
    uint32_t bis_sync_bf[__ARRAY_EMPTY];
} bap_bc_assist_modify_source_local_cmd_t;

/// Structure for #BAP_BC_ASSIST_REMOVE_SOURCE command message
typedef struct bap_bc_assist_remove_source_cmd
{
    /// Command code (shall be set to #BAP_BC_ASSIST_REMOVE_SOURCE)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
} bap_bc_assist_remove_source_cmd_t;

/// Structure for command complete event message
typedef struct bap_bc_assist_cmp_evt
{
    /// Command code (see #bap_bc_assist_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
} bap_bc_assist_cmp_evt_t;

/// Structure for #BAP_BC_ASSIST_RESTORE_BOND_DATA request message
typedef struct bap_bc_assist_restore_bond_data_req
{
    /// Request code (shall be set to #BAP_BC_ASSIST_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Service description
    bap_bc_assist_bass_t bass_info;
} bap_bc_assist_restore_bond_data_req_t;

/// Structure for #BAP_BC_ASSIST_SET_SGRP_INFO request message
typedef struct bap_bc_assist_set_sgrp_info_req
{
    /// Request code (shall be set to #BAP_BC_ASSIST_SET_SGRP_INFO)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Subgroup index\n
    /// From 1 to (nb_subgroups - 1)\n
    /// nb_subgroups value provided in either #BAP_BC_ASSIST_ADD_SOURCE or #BAP_BC_ASSIST_MODIFY_SOURCE command
    /// message
    uint8_t sgrp_idx;
    /// Required BIS synchronization bit field for the Subgroup
    uint32_t bis_sync_bf;
    /// Metadata for the Subgroup
    bap_cfg_metadata_t metadata;
} bap_bc_assist_set_sgrp_info_req_t;

/// Structure for #BAP_BC_ASSIST_SET_WRITE_TYPE request message
typedef struct bap_bc_assist_set_write_type_req
{
    /// Request code (shall be set to #BAP_BC_ASSIST_SET_WRITE_TYPE)
    uint16_t req_code;
    /// Indicate if used of reliable write is preferred
    bool reliable;
} bap_bc_assist_set_write_type_req_t;

/// Structure for response message
typedef struct bap_bc_assist_rsp
{
    /// Request code (see #bap_bc_assist_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index\n
    /// For:\n
    ///     - #BAP_BC_ASSIST_RESTORE_BOND_DATA request
    ///     - #BAP_BC_ASSIST_SET_SGRP_INFO request
    uint8_t con_lid;
    /// Subgroup index
    /// For:\n
    ///     - #BAP_BC_ASSIST_SET_SGRP_INFO request
    uint8_t sgrp_idx;
} bap_bc_assist_rsp_t;

/// Structure for #BAP_BC_ASSIST_BOND_DATA indication message
typedef struct bap_bc_assist_bond_data_ind
{
    /// Indication code (set to #BAP_BC_ASSIST_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Service description
    bap_bc_assist_bass_t bass_info;
} bap_bc_assist_bond_data_ind_t;

/// Structure for #BAP_BC_ASSIST_SOURCE_STATE indication message
typedef struct bap_bc_assist_source_state_ind
{
    /// Indication code (set to #BAP_BC_ASSIST_SOURCE_STATE)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
    /// Indicate if the instance of Broadcast Receive State characteristic is used\n
    /// Below parameters are meaningful only if is_used = true
    bool is_used;
    /// Source index
    uint8_t src_id;
    /// Periodic Advertising identification
    bap_adv_id_t adv_id;
    /// Broadcast ID
    bap_bcast_id_t bcast_id;
    /// Synchronization state for PA (see #bap_bc_rx_state_pa enumeration)
    uint8_t pa_sync;
    /// Encryption state for BIG (see #bap_bc_big_encrypt_state enumeration)
    uint8_t big_enc;
    /// Bad Broadcast Code\n
    /// Meaningful only if big_enc = #BAP_BC_BIG_ENCRYPT_STATE_BAD_CODE
    uint8_t bad_code[GAP_KEY_LEN];
    /// Number of subgroups\n
    /// From 0 to 32
    uint8_t nb_subgroups;
    /// BIS synchronization bit field for first Subgroup\n
    /// #BAP_BC_ASSIST_SOURCE_STATE_SGRP indication message sent for each following Subgroup\n
    /// Meaningful only if nb_subgroups != 0
    uint32_t bis_sync_bf;
    /// Metadata for the first Subgroup\n
    /// Meaningful only if nb_subgroups != 0
    bap_cfg_metadata_t metadata;
} bap_bc_assist_source_state_ind_t;

/// Structure for #BAP_BC_ASSIST_SOURCE_STATE_SGRP indication message
typedef struct bap_bc_assist_source_state_subgroup_ind
{
    /// Indication code (set to #BAP_BC_ASSIST_SOURCE_STATE_SGRP)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
    /// Subgroup index\n
    /// From 1 to (nb_subgroups - 1)\n
    /// nb_subgroups value provided in either #BAP_BC_ASSIST_SOURCE_STATE indication message
    uint8_t sgrp_idx;
    /// BIS synchronization bit field for the Subgroup
    uint32_t bis_sync_bf;
    /// Metadata for the Subgroup
    bap_cfg_metadata_t metadata;
} bap_bc_assist_source_state_subgroup_ind_t;

/// Structure for #BAP_BC_ASSIST_SCAN_TIMEOUT indication message
typedef struct bap_bc_assist_scan_timeout_ind
{
    /// Indication code (set to #BAP_BC_ASSIST_SCAN_TIMEOUT)
    uint16_t ind_code;
} bap_bc_assist_scan_timeout_ind_t;

/// Structure for #BAP_BC_ASSIST_SOLICITATION indication message
typedef struct bap_bc_assist_solicitation_ind
{
    /// Indication code (set to #BAP_BC_ASSIST_SOLICITATION)
    uint16_t ind_code;
    /// Address type
    uint8_t addr_type;
    /// Address
    uint8_t addr[GAP_BD_ADDR_LEN];
    /// Length of advertising data
    uint16_t length;
    /// Advertising data
    uint8_t adv_data[__ARRAY_EMPTY];
} bap_bc_assist_solicitation_ind_t;

/// Structure for #BAP_BC_ASSIST_SVC_CHANGED indication message
typedef struct bap_bc_assist_svc_changed_ind
{
    /// Indication code (set to #BAP_BC_ASSIST_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} bap_bc_assist_svc_changed_ind_t;

/// Structure for #BAP_BC_ASSIST_BCAST_CODE request indication message
typedef struct bap_bc_assist_bcast_code_req_ind
{
    /// Request indication code (set to #BAP_BC_ASSIST_BCAST_CODE)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
} bap_bc_assist_bcast_code_req_ind_t;

/// Structure for #BAP_BC_ASSIST_BCAST_CODE confirmation message
typedef struct bap_bc_assist_bcast_code_cfm
{
    /// Request indication code (shall be set to #BAP_BC_ASSIST_BCAST_CODE)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
    /// Broadcast code
    uint8_t bcast_code[GAP_KEY_LEN];
} bap_bc_assist_bcast_code_cfm_t;

/// @} BAP_BC_ASSIST_MSG

#endif // BAP_BC_ASSIST_MSG_H_
