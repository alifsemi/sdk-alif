/**
 ****************************************************************************************
 *
 * @file tmap_tmac_msg.h
 *
 * @brief Telephony and Media Audio Profile - Telephony and Media Audio Service Client - Message API Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TMAP_TMAC_MSG_H_
#define TMAP_TMAC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TMAP_TMAC_MSG Message API
 * @ingroup TMAP_TMAC
 * @brief Description of Message API for Telephony and Media Audio Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tmap_msg.h"       // Message API Definitions
#include "tmap_tmac.h"      // Telephony and Media Audio Profile - Telephony and Media Audio Service Client Definitions

/// @addtogroup TMAP_TMAC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Telephony and Media Audio Profile Client module
enum tmap_tmac_msg_cmd_code
{
    /// Telephony and Media Audio Service Client - Discover (see #tmap_tmac_discover_cmd_t)
    TMAP_TMAC_DISCOVER = GAF_CODE(TMAP, TMAC, TMAP_TMAC_CMD_TYPE_DISCOVER),
};

/// List of GAF_REQ request code values for Telephony and Media Audio Profile Client module
enum tmap_tmac_msg_req_code
{
    /// Telephony and Media Audio Service Client - Restore Bond Data (see #tmap_tmac_restore_bond_data_req_t)
    TMAP_TMAC_RESTORE_BOND_DATA = GAF_CODE(TMAP, TMAC, 0),
};

/// List of GAF_IND indication code values for Telephony and Media Audio Profile Client module
enum tmap_tmac_msg_ind_code
{
    /// Telephony and Media Audio Service Client - Restore Bond Data (see #tmap_tmac_bond_data_ind_t)
    TMAP_TMAC_BOND_DATA = GAF_CODE(TMAP, TMAC, 0),
    /// Telephony and Media Audio Service Client - Service Changed (see #tmap_tmac_svc_changed_ind_t)
    TMAP_TMAC_SVC_CHANGED = GAF_CODE(TMAP, TMAC, 1),
    /// Telephony and Media Audio Service Client - Role (see #tmap_tmac_role_ind_t)
    TMAP_TMAC_ROLE = GAF_CODE(TMAP, TMAC, 2),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #TMAP_TMAC_RESTORE_BOND_DATA request message
typedef struct tmap_tmac_restore_bond_data_req
{
    /// Request code (shall be set to #TMAP_TMAC_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Content description of Telephony and Media Audio Service
    tmap_tmac_tmas_t tmas_info;
} tmap_tmac_restore_bond_data_req_t;

/// Structure of response message for Telephony and Media Audio Service Client module
typedef struct tmap_tmac_rsp
{
    /// Request code (see #tmap_tmac_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} tmap_tmac_rsp_t;

/// Structure for #TMAP_TMAC_DISCOVER command message
typedef struct tmap_tmac_discover_cmd
{
    /// Command code (shall be set to #TMAP_TMAC_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Start handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t shdl;
    /// End handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t ehdl;
} tmap_tmac_discover_cmd_t;

/// Structure for command complete event message for Telephony and Media Audio Service Client module
typedef struct tmap_tmac_cmp_evt
{
    /// Command code (see #tmap_tmac_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} tmap_tmac_cmp_evt_t;

/// Structure for #TMAP_TMAC_BOND_DATA indication message
typedef struct tmap_tmac_bond_data_ind
{
    /// Indication code (set to #TMAP_TMAC_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Content description of Telephony and Media Audio Service
    tmap_tmac_tmas_t tmas_info;
} tmap_tmac_bond_data_ind_t;

/// Structure for #TMAP_TMAC_SVC_CHANGED indication message
typedef struct tmap_tmac_svc_changed_ind
{
    /// Indication code (set to #TMAP_TMAC_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} tmap_tmac_svc_changed_ind_t;

/// Structure for #TMAP_TMAC_ROLE indication message
typedef struct tmap_tmac_role_ind
{
    /// Indication code (set to #TMAP_TMAC_ROLE)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Role bit field (see #tmap_role_bf enumeration)
    uint16_t role_bf;
} tmap_tmac_role_ind_t;

/// @} HAP_HAC_MSG

#endif // TMAP_TMAC_MSG_H_
