/**
 ****************************************************************************************
 *
 * @file arc_vocs_msg.h
 *
 * @brief Audio Rendering Control - Definition of Kernel Messages (Volume Offset Control Server)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_VOCS_MSG_H_
#define ARC_VOCS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_VOCS_MSG Message API
 * @ingroup ARC_VOCS
 * @brief Description of Message API for Volume Offset Control Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"            // GAF Defines
#include "arc_msg.h"        // Audio Rendering Control Kernel Messages Definitions
#include "arc_vocs.h"       // Audio Rendering Control - Volume Offset Control Server Definitions

/// @addtogroup ARC_VOCS_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_REQ request code values for Volume Offset Control Service Server module
enum arc_vocs_msg_req_code
{
    /// Configure (see #arc_vocs_configure_req_t)
    ARC_VOCS_CONFIGURE = GAF_CODE(ARC, VOCS, 0),
    /// Add (see #arc_vocs_add_req_t)
    ARC_VOCS_ADD = GAF_CODE(ARC, VOCS, 1),
    /// Restore Bond Data (see #arc_vocs_restore_bond_data_req_t)
    ARC_VOCS_RESTORE_BOND_DATA = GAF_CODE(ARC, VOCS, 2),
    /// Set (see #arc_vocs_set_req_t)
    ARC_VOCS_SET = GAF_CODE(ARC, VOCS, 3),
    /// Set Description (see #arc_vocs_set_description_req_t)
    ARC_VOCS_SET_DESCRIPTION = GAF_CODE(ARC, VOCS, 4),
};

/// List of GAF_IND indication code values for Volume Offset Control Service Server module
enum arc_vocs_msg_ind_code
{
    /// Offset (see #arc_vocs_offset_ind_t)
    ARC_VOCS_OFFSET = GAF_CODE(ARC, VOCS, 0),
    /// Bond Data (see #arc_vocs_bond_data_ind_t)
    ARC_VOCS_BOND_DATA = GAF_CODE(ARC, VOCS, 1),
};

/// List of GAF_REQ_IND indication code values for Volume Offset Control Service Server module
enum arc_vocs_msg_req_ind_code
{
    /// Set Audio Description (see #arc_vocs_set_description_req_ind_t)
    ARC_VOCS_SET_DESCRIPTION_RI = GAF_CODE(ARC, VOCS, 0),
    /// Set Location (see #arc_vocs_set_location_req_ind_t)
    ARC_VOCS_SET_LOCATION = GAF_CODE(ARC, VOCS, 1),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ARC_VOCS_ADD request message
typedef struct arc_vocs_add_req
{
    /// Request code (shall be set to #ARC_VOCS_ADD)
    uint16_t req_code;
    /// Maximum length of Audio Output Description
    uint8_t desc_max_len;
    /// Service configuration bit field (see #arc_vocs_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Required start handle\n
    /// If set to GATT_INVALID_HDL, the start handle will be automatically chosen
    uint16_t shdl;
} arc_vocs_add_req_t;

/// Structure for #ARC_VOCS_ADD response message
typedef struct arc_vocs_add_rsp
{
    /// Request code (shall be set to #ARC_VOCS_ADD)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Allocated output local index
    uint8_t output_lid;
} arc_vocs_add_rsp_t;

/// Structure for #ARC_VOCS_RESTORE_BOND_DATA request message
typedef struct arc_vocs_restore_bond_data_req
{
    /// Request code (shall be set to #ARC_VOCS_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Output local index
    uint8_t output_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
    /// Event bit field
    uint8_t evt_cfg_bf;
} arc_vocs_restore_bond_data_req_t;

/// Structure for #ARC_VOCS_CONFIGURE request message
typedef struct arc_vocs_configure_req
{
    /// Request code (shall be set to #ARC_VOCS_CONFIGURE)
    uint16_t req_code;
    /// Number of outputs
    uint8_t nb_outputs;
    /// Preferred MTU\n
    /// Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
} arc_vocs_configure_req_t;

/// Structure for #ARC_VOCS_SET request message
typedef struct arc_vocs_set_req
{
    /// Request code (shall be set to #ARC_VOCS_SET)
    uint16_t req_code;
    /// Output local index
    uint8_t output_lid;
    /// Set type
    uint8_t set_type;
    /// Union
    union
    {
        /// Value
        uint32_t value;
        /// Volume offset
        int16_t offset;
        /// Audio location bit field (see #gaf_loc_bf enumeration)
        uint32_t location_bf;
    } u;
} arc_vocs_set_req_t;

/// Structure for #ARC_VOCS_SET_DESCRIPTION request message
typedef struct arc_vocs_set_description_req
{
    /// Request code (shall be set to #ARC_VOCS_SET_DESCRIPTION)
    uint16_t req_code;
    /// Output local index
    uint8_t output_lid;
    /// Audio output description length
    uint8_t desc_len;
    /// Audio output description
    uint8_t desc[__ARRAY_EMPTY];
} arc_vocs_set_description_req_t;

/// Structure for request message
typedef struct arc_vocs_rsp
{
    /// Request code (see #arc_vocs_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Output local index
    uint8_t output_lid;
    /// Union
    union
    {
        /// Value
        uint8_t value;
        /// Connection local index
        uint8_t con_lid;
        /// Set type
        uint8_t set_type;
    } u;
} arc_vocs_rsp_t;

/// Structure for #ARC_VOCS_OFFSET indication message
typedef struct arc_vocs_offset_ind
{
    /// Indication code (shall be set to #ARC_VOCS_OFFSET)
    uint16_t ind_code;
    /// Output local index
    uint8_t output_lid;
    /// Offset
    int16_t offset;
} arc_vocs_offset_ind_t;

/// Structure for #ARC_VOCS_BOND_DATA indication message
typedef struct arc_vocs_bond_data_ind
{
    /// Indication code (shall be set to #ARC_VOCS_BOND_DATA)
    uint16_t ind_code;
    /// Output local index
    uint8_t output_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
} arc_vocs_bond_data_ind_t;

/// Structure for #ARC_VOCS_SET_DESCRIPTION request indication message
typedef struct arc_vocs_set_description_req_ind
{
    /// Request Indication code (shall be set to #ARC_VOCS_SET_DESCRIPTION_RI)
    uint16_t req_ind_code;
    /// Output local index
    uint8_t output_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Audio output description length
    uint8_t desc_len;
    /// Audio output description
    uint8_t desc[__ARRAY_EMPTY];
} arc_vocs_set_description_req_ind_t;

/// Structure for #ARC_VOCS_SET_DESCRIPTION confirmation message
typedef struct arc_vocs_set_description_cfm
{
    /// Request Indication code (shall be set to #ARC_VOCS_SET_DESCRIPTION_RI)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Output local index
    uint8_t output_lid;
    /// Audio output description length
    uint8_t desc_len;
    /// Audio output description
    uint8_t desc[__ARRAY_EMPTY];
} arc_vocs_set_description_cfm_t;

/// Structure for #ARC_VOCS_SET_LOCATION request indication message
typedef struct arc_vocs_set_location_req_ind
{
    /// Request Indication code (shall be set to #ARC_VOCS_SET_LOCATION)
    uint16_t req_ind_code;
    /// Output local index
    uint8_t output_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Audio location bit field (see #gaf_loc_bf enumeration)
    uint32_t location_bf;
} arc_vocs_set_location_req_ind_t;

/// Structure for #ARC_VOCS_SET_LOCATION confirmation message
typedef struct arc_vocs_set_location_cfm
{
    /// Request Indication code (shall be set to #ARC_VOCS_SET_LOCATION)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Output local index
    uint8_t output_lid;
    /// Audio location bit field (see #gaf_loc_bf enumeration)
    uint32_t location_bf;
} arc_vocs_set_location_cfm_t;

/// @} ARC_VOCS_MSG

#endif // ARC_VOCS_MSG_H_
