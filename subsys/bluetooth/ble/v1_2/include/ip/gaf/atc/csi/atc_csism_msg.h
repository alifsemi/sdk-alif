/**
 ****************************************************************************************
 *
 * @file atc_csism_msg.h
 *
 * @brief Audio Topology Control - Definition of Kernel Messages (Coordinated Set
 * Identification Set Member)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ATC_CSISM_MSG_H_
#define ATC_CSISM_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ATC_CSISM_MSG Message API
 * @ingroup ATC_CSISM
 * @brief Description of Message API for Coordinated Set Identification Service Set Member module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "atc_msg.h"         // Audio Topology Control Kernel Messages Definitions
#include "csism_msg.h"       // Coordinated Set Identification Set Member - Message API Definitions

/// @addtogroup ATC_CSISM_MSG
/// @{

/// List of GAF_CMD command codes for Coordinated Set Identification Set Member module
enum atc_csism_msg_cmd_code
{
    /// Configure and add an instance of the Coordinated Set Identification Service
    ATC_CSISM_ADD = GAF_CODE(ATC, CSISM, ATC_CSISM_CMD_TYPE_ADD),
    /// Set SIRK value
    ATC_CSISM_SET_SIRK = GAF_CODE(ATC, CSISM, ATC_CSISM_CMD_TYPE_SET_SIRK),
    /// Update RSI
    ATC_CSISM_UPDATE_RSI = GAF_CODE(ATC, CSISM, ATC_CSISM_CMD_TYPE_UPDATE_RSI),
};

/// List of GAF_REQ request codes for Coordinated Set Identification Set Member module
enum atc_csism_msg_req_code
{
    /// Configure use of Coordinated Set Identification Set Member module
    ATC_CSISM_CONFIGURE = GAF_CODE(ATC, CSISM, 0),
    /// Restore bond data after reconnection with a trusted device
    ATC_CSISM_RESTORE_BOND_DATA = GAF_CODE(ATC, CSISM, 1),
    /// Update size of Coordinated Set
    ATC_CSISM_SET_SIZE = GAF_CODE(ATC, CSISM, 2),
};

/// List of GAF_IND indication codes for Coordinated Set Identification Set Member module
enum atc_csism_msg_ind_code
{
    /// Inform upper layer that device has been locked
    ATC_CSISM_LOCK = GAF_CODE(ATC, CSISM, 0),
    /// Inform upper layer about an update of bond data
    ATC_CSISM_BOND_DATA = GAF_CODE(ATC, CSISM, 1),
    /// Inform upper layer about generated RSI
    ATC_CSISM_RSI = GAF_CODE(ATC, CSISM, 2),
};

/// List of GAF_REQ_IND request indication codes for Coordinated Set Identification Set Member module
enum atc_csism_msg_req_ind_code
{
    /// Request LTK from upper layer for encryption of SIRK
    ATC_CSISM_LTK = GAF_CODE(ATC, CSISM, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #ATC_CSISM_ADD command message
typedef csism_add_cmd_t atc_csism_add_cmd_t;

/// Structure for #ATC_CSISM_SET_SIRK command message
typedef  csism_set_sirk_cmd_t atc_csism_set_sirk_cmd_t;

/// Structure for #ATC_CSISM_UPDATE_RSI command message
typedef  csism_update_rsi_cmd_t atc_csism_update_rsi_cmd_t;

/// Structure command complete event
typedef csism_cmp_evt_t atc_csism_cmp_evt_t;

/// Structure for #ATC_CSISM_CONFIGURE request message
typedef struct atc_csism_configure_req
{
    /// Request code (shall be set to #ATC_CSISM_CONFIGURE)
    uint16_t req_code;
    /// Number of Coordinated Sets
    uint8_t nb_sets;
} atc_csism_configure_req_t;

/// Structure for #ATC_CSISM_RESTORE_BOND_DATA request message
typedef csism_restore_bond_data_req_t atc_csism_restore_bond_data_req_t;

/// Structure for #ATC_CSISM_SET_SIZE request message
typedef csism_set_size_req_t atc_csism_set_size_req_t;

/// Structure for response message
typedef csism_rsp_t atc_csism_rsp_t;

/// Structure for #ATC_CSISM_LOCK indication message
typedef csism_lock_ind_t atc_csism_lock_ind_t;

/// Structure for #ATC_CSISM_BOND_DATA indication message
typedef csism_bond_data_ind_t atc_csism_bond_data_ind_t;

/// Structure for #ATC_CSISM_RSI indication message
typedef csism_rsi_ind_t atc_csism_rsi_ind_t;

/// Structure for #ATC_CSISM_LTK request indication message
typedef csism_ltk_req_ind_t atc_csism_ltk_req_ind_t;

/// Structure for #ATC_CSISM_LTK confirmation message
typedef csism_ltk_cfm_t atc_csism_ltk_cfm_t;

/// @} ATC_CSISM_MSG

#endif // ATC_CSISM_MSG_H_
