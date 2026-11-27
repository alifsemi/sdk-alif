/**
 ****************************************************************************************
 *
 * @file bap_capa_srv_msg.h
 *
 * @brief Basic Audio Profile - Capabilities Server - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_CAPA_SRV_MSG_H_
#define BAP_CAPA_SRV_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_SRV_MSG Message API
 * @ingroup BAP_CAPA_SRV
 * @brief Description of Message API for Capabilities Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap_msg.h"         // Basic Audio Profile - Message API Definitions
#include "bap_capa_srv.h"    // Basic Audio Profile - Capabilities Server Definitions

/// @addtogroup BAP_CAPA_SRV_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Capabilities Server
enum bap_capa_srv_msg_req_code
{
    /// Set Record (see #bap_capa_srv_set_record_req_t message structure)
    BAP_CAPA_SRV_SET_RECORD = GAF_CODE(BAP, CAPA_SRV, 0),
    /// Remove Record (see #bap_capa_srv_remove_record_req_t message structure)
    BAP_CAPA_SRV_REMOVE_RECORD = GAF_CODE(BAP, CAPA_SRV, 1),
    /// Restore Bond Data (see #bap_capa_srv_restore_bond_data_req_t message structure)
    BAP_CAPA_SRV_RESTORE_BOND_DATA = GAF_CODE(BAP, CAPA_SRV, 2),
    /// Set Location (see #bap_capa_srv_set_location_req_t message structure)
    BAP_CAPA_SRV_SET_LOCATION = GAF_CODE(BAP, CAPA_SRV, 3),
    /// Set Context (see #bap_capa_srv_set_context_req_t message structure)
    BAP_CAPA_SRV_SET_CONTEXT = GAF_CODE(BAP, CAPA_SRV, 4),
};

/// List of GAF_IND indication code values for Capabilities Server
enum bap_capa_srv_msg_ind_code
{
    /// Bond Data (see #bap_capa_srv_bond_data_ind_t message structure)
    BAP_CAPA_SRV_BOND_DATA = GAF_CODE(BAP, CAPA_SRV, 0),
};

/// List of GAF_REQ_IND request indication code values for Capabilities Server
enum bap_capa_srv_msg_req_ind_code
{
    /// Location update has been required (see #bap_capa_srv_location_req_ind_t and #bap_capa_srv_location_cfm_t
    /// message structures)
    BAP_CAPA_SRV_LOCATION = GAF_CODE(BAP, CAPA_SRV, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #BAP_CAPA_SRV_RESTORE_BOND_DATA request message
typedef struct bap_capa_srv_restore_bond_data_req
{
    /// Request code (shall be set to #BAP_CAPA_SRV_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field\n
    /// Each bit corresponds to a characteristic in the range [0, BAP_CAPA_CHAR_TYPE_PAC[
    uint8_t cli_cfg_bf;
    /// Client configuration bit field for Sink/Source PAC characteristic\n
    /// Each bit corresponds to an instance of the Sink/Source PAC characteristic (Sink placed first)
    uint16_t pac_cli_cfg_bf;
    /// Event bit field\n
    /// Each bit corresponds to a characteristic in the range [0, BAP_CAPA_CHAR_TYPE_PAC[
    uint8_t evt_bf;
    /// Event configuration bit field for Sink/Source PAC characteristics\n
    /// Each bit corresponds to an instance of the Sink/Source PAC characteristic (Sink placed first)
    uint16_t pac_evt_bf;
    /// Available Audio Contexts for Sink direction
    uint16_t context_bf_sink;
    /// Available Audio Contexts for Source direction
    uint16_t context_bf_src;
} bap_capa_srv_restore_bond_data_req_t;

/// Structure for #BAP_CAPA_SRV_SET_RECORD request message
typedef struct bap_capa_srv_set_record_req
{
    /// Request code (shall be set to #BAP_CAPA_SRV_SET_RECORD)
    uint16_t req_code;
    /// PAC local index
    uint8_t pac_lid;
    /// Record identifier
    uint8_t record_id;
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
} bap_capa_srv_set_record_req_t;

/// Structure for #BAP_CAPA_SRV_REMOVE_RECORD request message
typedef struct bap_capa_srv_remove_record_req
{
    /// Request code (shall be set to #BAP_CAPA_SRV_REMOVE_RECORD)
    uint16_t req_code;
    /// Record identifier
    uint8_t record_id;
} bap_capa_srv_remove_record_req_t;

/// Structure for #BAP_CAPA_SRV_SET_LOCATION request message
typedef struct bap_capa_srv_set_location_req
{
    /// Request code (shall be set to #BAP_CAPA_SRV_SET_LOCATION)
    uint16_t req_code;
    /// Direction (see #gaf_direction enumeration)
    uint8_t direction;
    /// Location bit field (see #gaf_loc_bf enumeration)
    uint32_t location_bf;
} bap_capa_srv_set_location_req_t;

/// Structure for #BAP_CAPA_SRV_SET_CONTEXT request message
typedef struct bap_capa_srv_set_context_req
{
    /// Request code (shall be set to #BAP_CAPA_SRV_SET_CONTEXT)
    uint16_t req_code;
    /// Context type (see #bap_capa_context_type enumeration)
    uint8_t context_type;
    /// Connection local index\n
    /// Meaningful only for Available Audio Contexts value\n
    /// Must be set to GAP_INVALID_CONIDX if indicated audio contexts are available for all connections
    uint8_t con_lid;
    /// Context bit field value for Sink direction
    uint16_t context_bf_sink;
    /// Context bit field value for Source direction
    uint16_t context_bf_src;
} bap_capa_srv_set_context_req_t;

/// Structure for Capabilities Server response message
typedef struct bap_capa_srv_rsp
{
    /// Request code (see #bap_capa_srv_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// First parameter
    union
    {
        uint8_t param;
        /// Connection local index\n
        /// Meaningful only for #BAP_CAPA_SRV_RESTORE_BOND_DATA and BAP_CAPA_SRV_SET_CONTEXT requests
        uint8_t con_lid;
        /// Record ID\n
        /// Meaningful only for #BAP_CAPA_SRV_SET_RECORD and BAP_CAPA_SRV_REMOVE_RECORD requests
        uint8_t record_id;
        /// Direction (see #gaf_direction enumeration)\n
        /// Meaningful only for #BAP_CAPA_SRV_SET_LOCATION request
        uint8_t direction;
    } u1;
    /// Second parameter
    union
    {
        uint8_t param;
        /// Context type (see #bap_capa_context_type enumeration)\n
        /// Meaningful only for #BAP_CAPA_SRV_SET_CONTEXT request
        uint8_t context_type;
        /// PAC local index\n
        /// Meaningful only for #BAP_CAPA_SRV_SET_RECORD request
        uint8_t pac_lid;
    } u2;
} bap_capa_srv_rsp_t;

/// Structure for #BAP_CAPA_SRV_BOND_DATA indication message
typedef struct bap_capa_srv_bond_data_ind
{
    /// Indication code (shall be set to #BAP_CAPA_SRV_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field\n
    /// Each bit corresponds to a characteristic in the range [0, BAP_CAPA_CHAR_TYPE_PAC[
    uint8_t cli_cfg_bf;
    /// Client configuration bit field for Sink/Source PAC characteristic\n
    /// Each bit corresponds to an instance of the Sink/Source PAC characteristic (Sink placed first)
    uint16_t pac_cli_cfg_bf;
} bap_capa_srv_bond_data_ind_t;

/// Structure for #BAP_CAPA_SRV_LOCATION request indication message
typedef struct bap_capa_srv_location_req_ind
{
    /// Request indication code (shall be set to #BAP_CAPA_SRV_LOCATION)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Direction (see #gaf_direction enumeration)
    uint8_t direction;
    /// Token that shall be provided back in the confirmation
    uint16_t token;
    /// Location bit field (see #gaf_loc_bf enumeration)
    uint32_t location_bf;
} bap_capa_srv_location_req_ind_t;

/// Structure for #BAP_CAPA_SRV_LOCATION confirmation message
typedef struct bap_capa_srv_location_cfm
{
    /// Request indication code (shall be set to #BAP_CAPA_SRV_LOCATION)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Direction (see #gaf_direction enumeration)
    uint8_t direction;
    /// Token that shall be provided back in the confirmation
    uint16_t token;
    /// Location bit field (see #gaf_loc_bf enumeration)
    uint32_t location_bf;
} bap_capa_srv_location_cfm_t;

/// @} BAP_CAPA_SRV_MSG

#endif // BAP_CAPA_SRV_MSG_H_
