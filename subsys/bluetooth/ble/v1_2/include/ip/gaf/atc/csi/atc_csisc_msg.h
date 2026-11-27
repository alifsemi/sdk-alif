/**
 ****************************************************************************************
 *
 * @file atc_csisc_msg.h
 *
 * @brief Audio Topology Control - Definition of Kernel Messages (Coordinated Set
 * Identification Set Coordinator)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ATC_CSISC_MSG_H_
#define ATC_CSISC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ATC_CSISC_MSG Message API
 * @ingroup ATC_CSISC
 * @brief Description of Message API for Coordinated Set Identification Service Set Coordinator module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "atc_msg.h"         // Audio Topology Control Kernel Messages Definitions
#include "csisc_msg.h"       // Coordinated Set Identification Set Coordinator - Message API Definitions
#include "atc_csisc.h"       // Coordinated Set Identification Set Coordinator ATC Definitions

/// @addtogroup ATC_CSISC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command codes for Coordinated Set Identification Set Coordinator module
enum atc_csisc_cmd_code
{
    /// Resolve an RSI value
    ATC_CSISC_RESOLVE = GAF_CODE(ATC, CSISC, ATC_CSISC_CMD_TYPE_RESOLVE),
    /// Discover instance of Coordinated Set Identification Service
    ATC_CSISC_DISCOVER = GAF_CODE(ATC, CSISC, ATC_CSISC_CMD_TYPE_DISCOVER),
    /// Lock peer device
    ATC_CSISC_LOCK = GAF_CODE(ATC, CSISC, ATC_CSISC_CMD_TYPE_LOCK),
    /// Get characteristic value
    ATC_CSISC_GET = GAF_CODE(ATC, CSISC, ATC_CSISC_CMD_TYPE_GET),
    /// Set configuration
    ATC_CSISC_SET_CFG = GAF_CODE(ATC, CSISC, ATC_CSISC_CMD_TYPE_SET_CFG),
};

/// List of GAF_REQ request codes for Coordinated Set Identification Set Coordinator module
enum atc_csisc_msg_req_code
{
    /// Configure use of Coordinated Set Identification Set Coordinator module
    ATC_CSISC_CONFIGURE = GAF_CODE(ATC, CSISC, 0),
    /// Restore bond data after reconnection with a trusted device
    ATC_CSISC_RESTORE_BOND_DATA = GAF_CODE(ATC, CSISC, 1),
    /// Add an SIRK from list of SIRKs used for RSI resolution
    ATC_CSISC_ADD_SIRK = GAF_CODE(ATC, CSISC, 2),
    /// Remove an SIRK from list of SIRKs used for RSI resolution
    ATC_CSISC_REMOVE_SIRK = GAF_CODE(ATC, CSISC, 3),
};

/// List of GAF_IND indication codes for Coordinated Set Identification Set Coordinator module
enum atc_csisc_msg_ind_code
{
    /// Inform upper layer about updated bond data
    ATC_CSISC_BOND_DATA = GAF_CODE(ATC, CSISC, 0),
    /// Inform upper layer about SIRK
    ATC_CSISC_SIRK = GAF_CODE(ATC, CSISC, 1),
    /// Inform upper layer about either Lock value, Rank value or Set Size value
    ATC_CSISC_INFO = GAF_CODE(ATC, CSISC, 2),
    /// Inform upper layer that position or content of an instance of CSIS has been changed
    ATC_CSISC_SVC_CHANGED = GAF_CODE(ATC, CSISC, 4),
};

/// List of GAF_REQ_IND request indication codes for Coordinated Set Identification Set Coordinator module
enum atc_csisc_msg_req_ind_codes
{
    /// Request LTK from upper layer in order to decrypt an SIRK
    ATC_CSISC_LTK = GAF_CODE(ATC, CSISC, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ATC_CSISC_RESOLVE command message
typedef csisc_resolve_cmd_t atc_csisc_resolve_cmd_t;

/// Structure for #ATC_CSISC_DISCOVER command message
typedef csisc_discover_cmd_t atc_csisc_discover_cmd_t;

/// Structure for #ATC_CSISC_LOCK command message
typedef csisc_lock_cmd_t atc_csisc_lock_cmd_t;

/// Structure for #ATC_CSISC_GET command message
typedef csisc_get_cmd_t atc_csisc_get_cmd_t;

/// Structure for #ATC_CSISC_SET_CFG command message
typedef csisc_set_cfg_cmd_t atc_csisc_set_cfg_cmd_t;

/// Structure command complete event
typedef csisc_cmp_evt_t atc_csisc_cmp_evt_t;

/// Structure for #ATC_CSISC_CONFIGURE request message
typedef struct atc_csisc_configure_req
{
    /// Request code (shall be set to #ATC_CSISC_CONFIGURE)
    uint16_t req_code;
    /// Number of SIRK values that can be stored
    uint8_t nb_sirk;
} atc_csisc_configure_req_t;

/// Structure for #ATC_CSISC_RESTORE_BOND_DATA request message
typedef csisc_restore_bond_data_req_t atc_csisc_restore_bond_data_req_t;

/// Structure for #ATC_CSISC_ADD_SIRK request message
typedef csisc_add_sirk_req_t atc_csisc_add_sirk_req_t;

/// Structure for #ATC_CSISC_REMOVE_SIRK request message
typedef csisc_remove_sirk_req_t atc_csisc_remove_sirk_req_t;

/// Structure for response message
typedef csisc_rsp_t atc_csisc_rsp_t;

/// Structure for #ATC_CSISC_BOND_DATA indication message
typedef csisc_bond_data_ind_t atc_csisc_bond_data_ind_t;

/// Structure for #ATC_CSISC_SIRK indication message
typedef csisc_sirk_ind_t atc_csisc_sirk_ind_t;

/// Structure for #ATC_CSISC_INFO indication message
typedef csisc_info_ind_t atc_csisc_info_ind_t;

/// Structure for #ATC_CSISC_SVC_CHANGED indication message
typedef csisc_svc_changed_ind_t atc_csisc_svc_changed_ind_t;

/// Structure for #ATC_CSISC_LTK request indication message
typedef csisc_ltk_req_ind_t atc_csisc_ltk_req_ind_t;

/// Structure for #ATC_CSISC_LTK confirmation message
typedef csisc_ltk_cfm_t atc_csisc_ltk_cfm_t;

/// @} ATC_CSISC_MSG

#endif // ATC_CSISC_MSG_H_
