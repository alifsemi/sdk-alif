/**
 ****************************************************************************************
 *
 * @file gap_bt.h
 *
 * @brief Header file - Generic Access Profile (GAP) - BT Classic
 *
 * Copyright (C) RivieraWaves 2009-2022
 *
 ****************************************************************************************
 */

#ifndef GAP_BT_H_
#define GAP_BT_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gap.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup GAP_ENUM_API
/// @{

/// Maximum Extended Inquiry Response data size
#define GAP_BT_EIR_MAX_DATA_SIZE            (240)
/// Classic Feature Flags Length
#define GAP_BT_FEATS_LEN                    (0x08)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// BT Classic OOB Data Present Flag Values
enum gap_bt_oob_flag
{
    /// No OOB data present
    GAP_BT_OOB_NO_DATA = 0x00,
    /// P-192 OOB data received from peer device
    GAP_BT_OOB_P192 = 0x01,
    /// P-256 OOB data received from peer device
    GAP_BT_OOB_P256 = 0x02,
    /// P-192 and P-256 OOB data received from peer device
    GAP_BT_OOB_P192_AND_P256 = 0x03,
};

/// BT Classic authentication requirement bit field
enum gap_bt_auth_req_bf
{
    /// Men in the middle protection required
    GAP_BT_AUTH_MITM_REQ_POS = 0,
    GAP_BT_AUTH_MITM_REQ_BIT = 0x01,
    /// Dedicated bonding -- if set, link automatically disconnected once pairing is finished
    GAP_BT_AUTH_DEDICATED_BOND_POS = 1,
    GAP_BT_AUTH_DEDICATED_BOND_BIT = 0x02,
    /// General Bonding
    GAP_BT_AUTH_BOND_POS = 2,
    GAP_BT_AUTH_BOND_BIT = 0x04,
};

/// Link Policy Bit Field
enum gap_bt_link_policy_bf
{
    /// Role Switch enabled
    GAP_BT_ROLE_SWITCH_ENABLE_BIT = (1 << 0),
    /// Hold Mode enabled
    GAP_BT_HOLD_MODE_ENABLE_BIT = (1 << 1),
    /// Sniff mode enabled
    GAP_BT_SNIFF_MODE_ENABLE_BIT = (1 << 2),
};

/// @} GAP_ENUM_API

#endif // GAP_BT_H_
