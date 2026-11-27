/**
 ****************************************************************************************
 *
 * @file cap_cac_msg.h
 *
 * @brief Common Audio Profile - Common Audio Service Client - Message API Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CAP_CAC_MSG_H_
#define CAP_CAC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CAP_CAC_MSG Message API
 * @ingroup CAP_CAC
 * @brief Description of Message API for Common Audio Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "cap_msg.h"        // Message API Definitions
#include "cap_cac.h"        // Common Audio Profile - Common Audio Service Client Definitions

/// @addtogroup CAP_CAC_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// List of GAF_CMD command code values for Common Audio Profile Client module
enum cap_cac_msg_cmd_code
{
    /// Common Audio Service Client - Discover (see #cap_cac_discover_cmd_t)
    CAP_CAC_DISCOVER = GAF_CODE(CAP, CAC, CAP_CAC_CMD_TYPE_DISCOVER),
};

/// List of GAF_REQ request code values for Common Audio Profile Client module
enum cap_cac_msg_req_code
{
    /// Common Audio Service Client - Restore Bond Data (see #cap_cac_restore_bond_data_req_t)
    CAP_CAC_RESTORE_BOND_DATA = GAF_CODE(CAP, CAC, 0),
};

/// List of GAF_IND indication code values for Common Audio Profile Client module
enum cap_cac_msg_ind_code
{
    /// Common Audio Service Client - Bond Data (see #cap_cac_bond_data_ind_t)
    CAP_CAC_BOND_DATA = GAF_CODE(CAP, CAC, 0),
    /// Common Audio Service Client - Service Changed (see #cap_cac_svc_changed_ind_t)
    CAP_CAC_SVC_CHANGED = GAF_CODE(CAP, CAC, 0),
};

/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #CAP_CAC_RESTORE_BOND_DATA request message
typedef struct cap_cac_restore_bond_data_req
{
    /// Request code (shall be set to #CAP_CAC_RESTORE_BOND_DATA)
    uint16_t req_code;
    /// Connection local index
    uint8_t con_lid;
    /// Common Audio Service content description
    cap_cac_cas_t cas_info;
} cap_cac_restore_bond_data_req_t;

/// Structure for response message for Common Audio Service Client module
typedef struct cap_cac_rsp
{
    /// Request code (see #cap_cac_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} cap_cac_rsp_t;

/// Structure for #CAP_CAC_DISCOVER command message
typedef struct cap_cac_discover_cmd
{
    /// Command code (shall be set to #CAP_CAC_DISCOVER)
    uint16_t cmd_code;
    /// Connection local index
    uint8_t con_lid;
    /// Start handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t shdl;
    /// End handle for the discovery. Set GATT_INVALID_HDL if not provided
    uint16_t ehdl;
} cap_cac_discover_cmd_t;

/// Structure for command complete event for Common Audio Service Client module
typedef struct cap_cac_cmp_evt
{
    /// Command code (see #cap_cac_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
} cap_cac_cmp_evt_t;

/// Structure for #CAP_CAC_BOND_DATA indication message
typedef struct cap_cac_bond_data_ind
{
    /// Indication code (set to #CAP_CAC_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Common Audio Service content description
    cap_cac_cas_t cas_info;
    /// Start handle of included Coordinated Set Identification Service
    uint16_t csis_shdl;
    /// End handle of included Coordinated Set Identification Service
    uint16_t csis_ehdl;
} cap_cac_bond_data_ind_t;

/// Structure for #CAP_CAC_SVC_CHANGED indication message
typedef struct cap_cac_svc_changed_ind
{
    /// Indication code (set to #CAP_CAC_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} cap_cac_svc_changed_ind_t;

/// @} CAP_CAC_MSG

#endif // CAP_CAC_MSG_H_
