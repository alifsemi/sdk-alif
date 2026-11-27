/**
 ****************************************************************************************
 *
 * @file bap_bc_deleg_msg.h
 *
 * @brief Basic Audio Profile - Broadcast Delegator - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_DELEG_MSG_H_
#define BAP_BC_DELEG_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC_DELEG_MSG Message API
 * @ingroup BAP_BC_DELEG
 * @brief Description of Message API for Broadcast Delegator module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap_msg.h"         // Basic Audio Profile API Messages Definitions
#include "bap_bc_deleg.h"    // Basic Audio Profile - Broadcast Delegator Definitions

#if (HOST_MSG_API && GAF_BAP_BC_DELEG)

/// @addtogroup BAP_BC_DELEG_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

#if (!(0))
/// List of GAF_CMD command code values for BAP Broadcast Delegator module
enum bap_bc_deleg_msg_cmd_code
{
    /// Start sending of Solicitation Requests (see #bap_bc_deleg_start_solicite_cmd_t)
    BAP_BC_DELEG_START_SOLICITE = GAF_CODE(BAP, BC_DELEG, BAP_BC_DELEG_CMD_TYPE_START_SOLICITE),
    /// Stop sending of Solicitation Requests (see #bap_bc_deleg_stop_solicite_cmd_t)
    BAP_BC_DELEG_STOP_SOLICITE = GAF_CODE(BAP, BC_DELEG, BAP_BC_DELEG_CMD_TYPE_STOP_SOLICITE),
};
#endif //(!(0))

/// List of GAF_REQ request code values for BAP Broadcast Delegator module
enum bap_bc_deleg_msg_req_code
{
    /// Set bonding information related to Broadcast Audio Scan Service after connection with a peer device
    /// (see #bap_bc_deleg_restore_bond_data_req_t)
    BAP_BC_DELEG_RESTORE_BOND_DATA = GAF_CODE(BAP, BC_DELEG, 0),
    /// Add a Broadcast Source from upper layer (see #bap_bc_deleg_add_source_req_t)
    BAP_BC_DELEG_ADD_SOURCE = GAF_CODE(BAP, BC_DELEG, 1),
    /// Update metadata of Broadcast Source from upper layer (see #bap_bc_deleg_modify_source_req_t)
    BAP_BC_DELEG_MODIFY_SOURCE = GAF_CODE(BAP, BC_DELEG, 2),
    /// Remove Broadcast Source from upper layer (see #bap_bc_deleg_remove_source_req_t)
    BAP_BC_DELEG_REMOVE_SOURCE = GAF_CODE(BAP, BC_DELEG, 3),
    #if (!(0))
    /// Get Activity index allocated by GAP for advertising activity created in order to send Solicitation Requests
    /// (see #bap_bc_deleg_get_actv_idx_req_t)
    BAP_BC_DELEG_GET_ACTV_IDX = GAF_CODE(BAP, BC_DELEG, 4),
    #endif //(!(0))
};

/// List of GAF_IND indication code values for BAP Broadcast Delegator module
enum bap_bc_deleg_msg_ind_code
{
    #if (!(0))
    /// Inform upper layer that sending of Solicitation Requests has been stopped
    /// (see #bap_bc_deleg_solicite_stopped_ind_t)
    BAP_BC_DELEG_SOLICITE_STOPPED = GAF_CODE(BAP, BC_DELEG, 0),
    #endif //(!(0))
    /// Inform upper layer about an updated client configuration for Broadcast Audio Scan Service
    /// (see #bap_bc_deleg_bond_data_ind_t)
    BAP_BC_DELEG_BOND_DATA = GAF_CODE(BAP, BC_DELEG, 1),
    /// Inform upper layer about Broadcast Assistant scanning state update (see #bap_bc_deleg_remote_scan_ind_t)
    BAP_BC_DELEG_REMOTE_SCAN = GAF_CODE(BAP, BC_DELEG, 2),
    /// Inform upper layer about Broadcast Code set by Broadcast Assistant (see #bap_bc_deleg_bcast_code_ind_t)
    BAP_BC_DELEG_BCAST_CODE = GAF_CODE(BAP, BC_DELEG, 3),
};

/// List of GAF_REQ_IND request indication code values for BAP Broadcast Delegator module
enum bap_bc_deleg_msg_req_ind_codes
{
    /// Inform upper layer that a Broadcast Scan Assistant device asking to add new Broadcast Source
    /// (see #bap_bc_deleg_add_source_req_ind_t)
    BAP_BC_DELEG_ADD_SOURCE_RI = GAF_CODE(BAP, BC_DELEG, 0),
    /// Inform upper layer that a Broadcast Scan Assistant device asking to update metadata of a Broadcast Source
    /// (see #bap_bc_deleg_modify_source_req_ind_t)
    BAP_BC_DELEG_MODIFY_SOURCE_RI = GAF_CODE(BAP, BC_DELEG, 1),
    /// Inform upper layer that a Broadcast Scan Assistant device asking to remove a Broadcast Source
    /// (see #bap_bc_deleg_remove_source_req_ind_t)
    BAP_BC_DELEG_REMOVE_SOURCE_RI = GAF_CODE(BAP, BC_DELEG, 2),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

#if (!(0))
/// Structure for #BAP_BC_DELEG_START_SOLICITE command message
typedef struct bap_bc_deleg_start_solicite_cmd
{
    /// Command code (shall be set to #BAP_BC_DELEG_START_SOLICITE)
    uint16_t cmd_code;
    /// Timeout duration in seconds\n
    /// 0 means that sending of Solicitation Requests will last until stopped by the upper layer
    uint16_t timeout_s;
    /// Advertising parameters
    bap_bc_adv_param_t adv_param;
    /// Additional advertising data (in LTV format)
    gaf_ltv_t adv_data;
} bap_bc_deleg_start_solicite_cmd_t;

/// Structure for #BAP_BC_DELEG_STOP_SOLICITE command message
typedef struct bap_bc_deleg_stop_solicite_cmd
{
    /// Command code (shall be set to #BAP_BC_DELEG_STOP_SOLICITE)
    uint16_t cmd_code;
} bap_bc_deleg_stop_solicite_cmd_t;

/// Structure for command complete message
typedef struct bap_bc_deleg_cmp_evt
{
    /// Command code (see #bap_bc_deleg_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status (see #gaf_err enumeration)
    uint16_t status;
    /// Source local index
    uint8_t src_lid;
} bap_bc_deleg_cmp_evt_t;
#endif //(!(0))

/// Structure for #BAP_BC_DELEG_RESTORE_BOND_DATA request message
typedef struct bap_bc_deleg_restore_bond_data_req
{
    /// Request code (shall be set to #BAP_BC_DELEG_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Configuration bit field (1 bit per Broadcast Receive State characteristic instance)
    uint16_t cli_cfg_bf;
} bap_bc_deleg_restore_bond_data_req_t;

/// Structure for #BAP_BC_DELEG_ADD_SOURCE request message
typedef struct bap_bc_deleg_add_source_req
{
    /// Request code (shall be set to #BAP_BC_DELEG_ADD_SOURCE)
    uint16_t req_code;
    /// Periodic Advertising identification
    bap_adv_id_t adv_id;
    /// Broadcast ID
    bap_bcast_id_t bcast_id;
    /// Information bit field (see #bap_bc_deleg_add_info_bf enumeration)
    uint8_t info_bf;
    /// Number of Subgroups
    uint8_t nb_subgroups;
    /// Subgroup information\n
    /// Is an array of nb_subgroups #bap_bc_sgrp_info_t structures, each structure has a variable length
    uint32_t sgrp_info[__ARRAY_EMPTY];
} bap_bc_deleg_add_source_req_t;

/// Structure for #BAP_BC_DELEG_MODIFY_SOURCE request message
typedef struct bap_bc_deleg_modify_source_req
{
    /// Request code (shall be set to #BAP_BC_DELEG_MODIFY_SOURCE)
    uint16_t req_code;
    /// Source local index
    uint8_t src_lid;
    /// Number of Subgroups
    uint8_t nb_subgroups;
    /// Subgroup information\n
    /// Is an array of nb_subgroups #bap_bc_sgrp_info_t structures, each structure has a variable length
    uint32_t sgrp_info[__ARRAY_EMPTY];
} bap_bc_deleg_modify_source_req_t;

/// Structure for #BAP_BC_DELEG_REMOVE_SOURCE request message
typedef struct bap_bc_deleg_remove_source_req
{
    /// Request code (shall be set to #BAP_BC_DELEG_REMOVE_SOURCE)
    uint16_t req_code;
    /// Source local index
    uint8_t src_lid;
} bap_bc_deleg_remove_source_req_t;

#if (!(0))
/// Structure for #BAP_BC_DELEG_GET_ACTV_IDX request message
typedef struct
{
    /// Request code (shall be set to #BAP_BC_DELEG_GET_ACTV_IDX)
    uint16_t req_code;
} bap_bc_deleg_get_actv_idx_req_t;
#endif // (!(0))

/// Structure for response message
typedef struct bap_bc_deleg_rsp
{
    /// Request code (see #bap_bc_deleg_msg_req_code enumeration)
    uint16_t req_code;
    /// Status (see #gaf_err enumeration)
    uint16_t status;
    /// Connection local index for #BAP_BC_DELEG_RESTORE_BOND_DATA request\n
    #if (!(0))
    /// Activity index for #BAP_BC_DELEG_GET_ACTV_IDX request\n
    #endif // (!(0))
    /// Source local index otherwise
    uint8_t lid;
    /// Bit field indicating Subgroups for which an error has been detected in Metadata\n
    /// Meaningful only for #BAP_BC_DELEG_ADD_SOURCE and #BAP_BC_DELEG_MODIFY_SOURCE requests
    uint32_t sgrp_err_bf;
} bap_bc_deleg_rsp_t;

#if (!(0))
/// Structure for #BAP_BC_DELEG_SOLICITE_STOPPED indication message
typedef struct bap_bc_deleg_solicite_stopped_ind
{
    /// Indication code (set to #BAP_BC_DELEG_SOLICITE_STOPPED)
    uint16_t ind_code;
    /// Reason why sending of Solicitation Requests has been stopped (see #bap_bc_deleg_stop_reason enumeration)
    uint8_t reason;
} bap_bc_deleg_solicite_stopped_ind_t;
#endif //(!(0))

/// Structure for #BAP_BC_DELEG_BOND_DATA indication message
typedef struct bap_bc_deleg_bond_data_ind
{
    /// Indication code (set to #BAP_BC_DELEG_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field (1 bit per Broadcast Receive State characteristic instance)
    uint16_t cli_cfg_bf;
} bap_bc_deleg_bond_data_ind_t;

/// Structure for #BAP_BC_DELEG_REMOTE_SCAN indication message
typedef struct bap_bc_deleg_remote_scan_ind
{
    /// Indication code (set to #BAP_BC_DELEG_REMOTE_SCAN)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Broadcast Assistant device scan state (see #bap_bc_deleg_scan_state enumeration)
    uint8_t state;
} bap_bc_deleg_remote_scan_ind_t;

/// Structure for #BAP_BC_DELEG_BCAST_CODE indication message
typedef struct bap_bc_deleg_bcast_code_ind
{
    /// Indication code (set to #BAP_BC_DELEG_REMOTE_SCAN)
    uint16_t ind_code;
    /// Source local index
    uint8_t src_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Broadcast code
    gaf_bcast_code_t bcast_code;
} bap_bc_deleg_bcast_code_ind_t;

/// Structure for #BAP_BC_DELEG_ADD_SOURCE_RI request indication message
typedef struct bap_bc_deleg_add_source_req_ind
{
    /// Request Indication code (set to #BAP_BC_DELEG_ADD_SOURCE_RI)
    uint16_t req_ind_code;
    /// Allocated Source local index
    uint8_t src_lid;
    /// Connection local index for device that has added the source
    uint8_t con_lid;
    /// Periodic Advertising identification
    bap_adv_id_t adv_id;
    /// Broadcast ID
    bap_bcast_id_t bcast_id;
    /// Periodic Advertising interval in frames\n
    /// From 0x0006 to 0xFFFE\n
    /// BAP_BC_UNKNOWN_PA_INTV indicates that interval is unknown
    uint16_t pa_intv_frames;
    /// Requested synchronization state for Periodic Advertising
    uint8_t pa_sync_req;
    /// Number of Subgroups
    uint8_t nb_subgroups;
    /// Subgroup information\n
    /// Must be casted as an array of #bap_bc_sgrp_info_t structures. Each structure has a variable size
    uint32_t subgroup_info[__ARRAY_EMPTY];
} bap_bc_deleg_add_source_req_ind_t;

/// Structure for #BAP_BC_DELEG_MODIFY_SOURCE_RI request indication message
typedef struct bap_bc_deleg_modify_source_req_ind
{
    /// Request Indication code (set to #BAP_BC_DELEG_MODIFY_SOURCE_RI)
    uint16_t req_ind_code;
    /// Source local index
    uint8_t src_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Periodic Advertising interval in frames\n
    /// From 0x0006 to 0xFFFE\n
    /// BAP_BC_UNKNOWN_PA_INTV indicates that interval is unknown
    uint16_t pa_intv_frames;
    /// Requested synchronization state for Periodic Advertising
    uint8_t pa_sync_req;
    /// Number of Subgroups
    uint8_t nb_subgroups;
    /// Subgroup information\n
    /// Must be casted as an array of #bap_bc_sgrp_info_t structures. Each structure has a variable size
    uint32_t subgroup_info[__ARRAY_EMPTY];
} bap_bc_deleg_modify_source_req_ind_t;

/// Structure for #BAP_BC_DELEG_REMOVE_SOURCE_RI request indication message
typedef struct bap_bc_deleg_remove_source_req_ind
{
    /// Request Indication code (set to #BAP_BC_DELEG_REMOVE_SOURCE_RI)
    uint16_t req_ind_code;
    /// Source local index
    uint8_t src_lid;
    /// Connection local index
    uint8_t con_lid;
} bap_bc_deleg_remove_source_req_ind_t;

/// Structure for #BAP_BC_DELEG_ADD_SOURCE_RI confirmation message
typedef struct bap_bc_deleg_add_source_cfm
{
    /// Request Indication code (shall be set to #BAP_BC_DELEG_ADD_SOURCE_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Source local index
    uint8_t src_lid;
    /// Bit field indicating Subgroups for which Metadata is provided
    uint32_t sgrp_metadata_bf;
    /// Subgroup Metadata\n
    /// Is an array of #bap_bc_sgrp_metadata_t structures (number of structures is equal to number of bits set to
    /// 1 in #sgrp_metadata_bf bit field), each structure has a variable length
    uint32_t sgrp_metadata[__ARRAY_EMPTY];
} bap_bc_deleg_add_source_cfm_t;

/// Structure for #BAP_BC_DELEG_MODIFY_SOURCE_RI confirmation message
typedef struct bap_bc_deleg_modify_source_cfm
{
    /// Request Indication code (shall be set to #BAP_BC_DELEG_MODIFY_SOURCE_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Source local index
    uint8_t src_lid;
    /// Bit field indicating Subgroups for which Metadata has been updated and is provided
    uint32_t sgrp_metadata_bf;
    /// Subgroup Metadata\n
    /// Is an array of #bap_bc_sgrp_metadata_t structures (number of structures is equal to number of bits set to
    /// 1 in #sgrp_metadata_bf bit field), each structure has a variable length
    uint32_t sgrp_metadata[__ARRAY_EMPTY];
} bap_bc_deleg_modify_source_cfm_t;

/// Structure for #BAP_BC_DELEG_REMOVE_SOURCE_RI confirmation message
typedef struct bap_bc_deleg_remove_source_cfm
{
    /// Request Indication code (shall be set to #BAP_BC_DELEG_REMOVE_SOURCE_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Source local index
    uint8_t src_lid;
} bap_bc_deleg_remove_source_cfm_t;

/// @} BAP_BC_DELEG_MSG
#endif //(HOST_MSG_API && GAF_BAP_BC_DELEG)

#endif // BAP_BC_DELEG_MSG_H_
