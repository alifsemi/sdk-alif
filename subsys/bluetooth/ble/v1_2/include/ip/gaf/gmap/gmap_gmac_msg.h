/**
 ****************************************************************************************
 *
 * @file gmap_gmac_msg.h
 *
 * @brief Gaming Audio Profile - Gaming Audio Service Client - Message API Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GMAP_GMAC_MSG_H_
#define GMAP_GMAC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GMAP_GMAC_MSG Message API
 * @ingroup GMAP_GMAC
 * @brief Description of Message API for Gaming Audio Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gmap_msg.h"       // Message API Definitions
#include "gmap_gmac.h"      // Gaming Audio Profile - Gaming Audio Service Client Definitions

/// @addtogroup GMAP_GMAC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Gaming Audio Profile Client module
enum gmap_gmac_msg_cmd_code
{
    /// Gaming Audio Service Client - Discover (see #gmap_gmac_discover_cmd_t)
    GMAP_GMAC_DISCOVER = GAF_CODE(GMAP, GMAC, GMAP_GMAC_CMD_TYPE_DISCOVER),
};

/// List of GAF_REQ request code values for Gaming Audio Profile Client module
enum gmap_gmac_msg_req_code
{
    /// Gaming Audio Service Client - Restore Bond Data (see #gmap_gmac_restore_bond_data_req_t)
    GMAP_GMAC_RESTORE_BOND_DATA = GAF_CODE(GMAP, GMAC, 0),
};

/// List of GAF_IND indication code values for Gaming Audio Profile Client module
enum gmap_gmac_msg_ind_code
{
    /// Gaming Audio Service Client - Restore Bond Data (see #gmap_gmac_bond_data_ind_t)
    GMAP_GMAC_BOND_DATA = GAF_CODE(GMAP, GMAC, 0),
    /// Gaming Audio Service Client - Service Changed (see #gmap_gmac_svc_changed_ind_t)
    GMAP_GMAC_SVC_CHANGED = GAF_CODE(GMAP, GMAC, 1),
    /// Gaming Audio Service Client - Value (see #gmap_gmac_value_ind_t)
    GMAP_GMAC_VALUE = GAF_CODE(GMAP, GMAC, 2),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #GMAP_GMAC_RESTORE_BOND_DATA request message
typedef struct gmap_gmac_restore_bond_data_req
{
    /// Request code (shall be set to #GMAP_GMAC_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Description of Gaming Audio Service
    prf_svc_t gmas_info;
} gmap_gmac_restore_bond_data_req_t;

/// Structure of response message for Gaming Audio Service Client module
typedef struct gmap_gmac_rsp
{
    /// Request code (see #gmap_gmac_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} gmap_gmac_rsp_t;

/// Structure for #GMAP_GMAC_DISCOVER command message
typedef struct gmap_gmac_discover_cmd
{
    /// Command code (shall be set to #GMAP_GMAC_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
} gmap_gmac_discover_cmd_t;

/// Structure for command complete event message for Gaming Audio Service Client module
typedef struct gmap_gmac_cmp_evt
{
    /// Command code (see #gmap_gmac_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} gmap_gmac_cmp_evt_t;

/// Structure for #GMAP_GMAC_BOND_DATA indication message
typedef struct gmap_gmac_bond_data_ind
{
    /// Indication code (set to #GMAP_GMAC_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Description of Gaming Audio Service
    prf_svc_t gmas_info;
} gmap_gmac_bond_data_ind_t;

/// Structure for #GMAP_GMAC_SVC_CHANGED indication message
typedef struct gmap_gmac_svc_changed_ind
{
    /// Indication code (set to #GMAP_GMAC_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} gmap_gmac_svc_changed_ind_t;

/// Structure for #GMAP_GMAC_VALUE indication message
typedef struct gmap_gmac_value_ind
{
    /// Indication code (set to #GMAP_GMAC_VALUE)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type (see #gmap_char_type enumeration)
    uint8_t char_type;
    /// Value
    union_gmap_gmac_value_u value;
} gmap_gmac_value_ind_t;

/// @} HAP_HAC_MSG

#endif // GMAP_GMAC_MSG_H_
