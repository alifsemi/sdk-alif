/**
 ****************************************************************************************
 *
 * @file arc_vocc_msg.h
 *
 * @brief Audio Rendering Control - Definition of API Messages for Volume Offset Control Service Client module
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_VOCC_MSG_H_
#define ARC_VOCC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_VOCC_MSG Message API
 * @ingroup ARC_VOCC
 * @brief Description of Message API for Volume Offset Control Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc_msg.h"        // Audio Rendering Control - API Messages Definitions
#include "arc_vocc.h"       // Audio Rendering Control - Volume Offset Control Service Client Definitions

/// @addtogroup ARC_VOCC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Volume Offset Control Service Client module
enum arc_vocc_msg_cmd_code
{
    /// Discover (see #arc_vocc_discover_cmd_t)
    ARC_VOCC_DISCOVER = GAF_CODE(ARC, VOCC, ARC_VOCC_CMD_TYPE_DISCOVER),
    /// Get (see #arc_vocc_get_cmd_t)
    ARC_VOCC_GET = GAF_CODE(ARC, VOCC, ARC_VOCC_CMD_TYPE_GET),
    /// Set Audio Description (see #arc_vocc_set_description_cmd_t)
    ARC_VOCC_SET_DESCRIPTION = GAF_CODE(ARC, VOCC, ARC_VOCC_CMD_TYPE_SET_DESCRIPTION),
    /// Set (see #arc_vocc_set_cmd_t)
    ARC_VOCC_SET = GAF_CODE(ARC, VOCC, ARC_VOCC_CMD_TYPE_SET),
    /// Set Configuration (see #arc_vocc_set_cfg_cmd_t
    ARC_VOCC_SET_CFG = GAF_CODE(ARC, VOCC, ARC_VOCC_CMD_TYPE_SET_CFG),
};

/// List of GAF_REQ request code values for Volume Offset Control Service Client module
enum arc_vocc_msg_req_code
{
    /// Configure (see #arc_vocc_configure_req_t)
    ARC_VOCC_CONFIGURE = GAF_CODE(ARC, VOCC, 0),
    /// Restore Bond Data (see #arc_vocc_restore_bond_data_req_t)
    ARC_VOCC_RESTORE_BOND_DATA = GAF_CODE(ARC, VOCC, 1),
};

/// List of GAF_IND indication code values for Volume Offset Control Service Client module
enum arc_vocc_msg_ind_code
{
    /// Bond Data (see #arc_vocc_bond_data_ind_t)
    ARC_VOCC_BOND_DATA = GAF_CODE(ARC, VOCC, 0),
    /// Value (see #arc_vocc_value_ind_t)
    ARC_VOCC_VALUE = GAF_CODE(ARC, VOCC, 1),
    /// Audio Description (see #arc_vocc_description_ind_t)
    ARC_VOCC_DESCRIPTION = GAF_CODE(ARC, VOCC, 2),
    /// Service Changed (see #arc_vocc_svc_changed_ind_t)
    ARC_VOCC_SVC_CHANGED = GAF_CODE(ARC, VOCC, 4),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ARC_VOCC_DISCOVER command message
typedef struct arc_vocc_discover_cmd
{
    /// Command code (shall be set to #ARC_VOCC_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Number of outputs
    uint8_t nb_outputs;
    /// Service handles
    prf_svc_t svc_hdl[__ARRAY_EMPTY];
} arc_vocc_discover_cmd_t;

/// Structure for #ARC_VOCC_GET command message
typedef struct arc_vocc_get_cmd
{
    /// Command code (shall be set to #ARC_VOCC_GET)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
    /// Characteristic type
    uint8_t char_type;
} arc_vocc_get_cmd_t;

/// Structure for #ARC_VOCC_SET_DESCRIPTION command message
typedef struct arc_vocc_set_description_cmd
{
    /// Command code (shall be set to #ARC_VOCC_SET_DESCRIPTION)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
    /// Audio output description length
    uint16_t desc_len;
    /// Audio output description
    uint8_t desc[__ARRAY_EMPTY];
} arc_vocc_set_description_cmd_t;

/// Structure for #ARC_VOCC_SET command message
typedef struct arc_vocc_set_cmd
{
    /// Command code (shall be set to #ARC_VOCC_SET)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
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
} arc_vocc_set_cmd_t;

/// Structure for #ARC_VOCC_SET_CFG command message
typedef struct arc_vocc_set_cfg_cmd
{
    /// Command code (shall be set to #ARC_VOCC_SET_CFG)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Enable or disable
    uint8_t enable;
} arc_vocc_set_cfg_cmd_t;

/// Structure for command complete event message
typedef struct arc_vocc_cmp_evt
{
    /// Command code (see #arc_vocc_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
    /// Union
    union
    {
        /// Value
        uint8_t value;
        /// Set type
        uint8_t set_type;
        /// Characteristic type
        uint8_t char_type;
    } u;
} arc_vocc_cmp_evt_t;

/// Structure for #ARC_VOCC_CONFIGURE request message
typedef struct arc_vocc_configure_req
{
    /// Request code (shall be set to #ARC_VOCC_CONFIGURE)
    uint16_t req_code;
    /// Preferred MTU\n
    /// Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
} arc_vocc_configure_req_t;

/// Structure for #ARC_VOCC_RESTORE_BOND_DATA request message
typedef struct arc_vocc_restore_bond_data_req
{
    /// Request code (shall be set to #ARC_VOCC_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Number of outputs
    uint8_t nb_outputs;
    /// Description of found Volume Offset Control Service instances
    arc_vocc_vocs_t vocs_info[__ARRAY_EMPTY];
} arc_vocc_restore_bond_data_req_t;

/// Structure for response message
typedef struct arc_vocc_rsp
{
    /// Request code (see #arc_vocc_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} arc_vocc_rsp_t;

/// Structure for #ARC_VOCC_BOND_DATA indication message
typedef struct arc_vocc_bond_data_ind
{
    /// Indication code (shall be set to #ARC_VOCC_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
    /// Description of found Volume Offset Control Service instance
    arc_vocc_vocs_t vocs_info;
} arc_vocc_bond_data_ind_t;

/// Structure for #ARC_VOCC_VALUE indication message
typedef struct arc_vocc_value_ind
{
    /// Indication code (shall be set to #ARC_VOCC_VALUE)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
    /// Characteristic type
    uint8_t char_type;
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
} arc_vocc_value_ind_t;

/// Structure for #ARC_VOCC_DESCRIPTION indication message
typedef struct arc_vocc_description_ind
{
    /// Indication code (shall be set to #ARC_VOCC_DESCRIPTION)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
    /// Audio output description length
    uint16_t desc_len;
    /// Audio output description
    uint8_t desc[__ARRAY_EMPTY];
} arc_vocc_description_ind_t;

/// Structure for #ARC_VOCC_SVC_CHANGED indication message
typedef struct arc_vocc_svc_changed_ind
{
    /// Indication code (shall be set to #ARC_VOCC_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} arc_vocc_svc_changed_ind_t;

/// @} ARC_VOCC_MSG

#endif // ARC_VOCC_MSG_H_
