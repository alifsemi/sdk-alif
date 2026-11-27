/**
 ****************************************************************************************
 *
 * @file arc_vc.h
 *
 * @brief Audio Rendering Control - Volume Control - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_VC_H_
#define ARC_VC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_VC Volume Control Service
 * @ingroup ARC
 * @brief Description of Volume Control Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VC_DEF Definitions
 * @ingroup ARC_VC
 * @brief Definitions for Volume Control Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VC_ENUM Enumerations
 * @ingroup ARC_VC
 * @brief Enumerations for Volume Control Service module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc.h"            // Audio Rendering Control Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup ARC_VC_DEF
/// @{

/// Minimum volume value
#define ARC_VC_VOLUME_MIN           (0)
/// Maximum volume value
#define ARC_VC_VOLUME_MAX           (255)

/// @} ARC_VC_DEF

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_VC_ENUM
/// @{

/// Position of fields in Volume State characteristic value
enum arc_vc_state_pos
{
    /// Volume Setting field
    ARC_VC_STATE_POS_SETTING = 0,
    /// Mute field
    ARC_VC_STATE_POS_MUTE,
    /// Change Counter field
    ARC_VC_STATE_POS_CHANGE_CNT,

    /// Length of Volume State characteristic value
    ARC_VC_STATE_LEN
};

/// Position of fields in value written in Volume Control Point characteristic
enum arc_vc_cp_pos
{
    /// Operation Code field
    ARC_VC_CP_POS_OPCODE = 0,
    /// Change Counter field
    ARC_VC_CP_POS_CHANGE_CNT,
    /// Volume Setting field
    ARC_VC_CP_POS_SETTING,

    /// Minimal length of Volume Control Point characteristic value
    ARC_VC_CP_LEN_MIN = ARC_VC_CP_POS_CHANGE_CNT + 1,
    ARC_VC_CP_LEN_MAX = ARC_VC_CP_POS_SETTING + 1,
};

/// Position of fields in Volume Flags characteristic value
enum arc_vc_flags_pos
{
    /// Volume Flags field
    ARC_VC_FLAGS_POS_FLAGS = 0,

    /// Length of Volume Flags characteristic value
    ARC_VC_FLAGS_LEN
};

/// Volume Operation Code values
enum arc_vc_opcode
{
    /// Relative Volume Down
    ARC_VC_OPCODE_VOL_DOWN = 0,
    /// Relative Volume Up
    ARC_VC_OPCODE_VOL_UP,
    /// Unmute/Relative Volume Down
    ARC_VC_OPCODE_VOL_DOWN_UNMUTE,
    /// Unmute/Relative Volume Up
    ARC_VC_OPCODE_VOL_UP_UNMUTE,
    /// Set Absolute Volume
    ARC_VC_OPCODE_VOL_SET_ABS,
    /// Unmute
    ARC_VC_OPCODE_VOL_UNMUTE,
    /// Mute
    ARC_VC_OPCODE_VOL_MUTE,

    ARC_VC_OPCODE_MAX
};

/// Application error codes
enum arc_vc_err
{
    /// Invalid change counter
    ARC_VC_ERR_INVALID_CHANGE_CNT = 0x80,
    /// Opcode not supported
    ARC_VC_ERR_OPCODE_NOT_SUPP,
};

/// Volume Flags characteristic value bit field meaning
enum arc_vc_flags_bf
{
    /// Volume Setting Persisted bit
    /// 0x00 for Reset Volume Setting and 0x01 for User Set Volume Setting
    ARC_VC_FLAGS_SETTING_PERSISTED_POS = 0,
    ARC_VC_FLAGS_SETTING_PERSISTED_BIT = CO_BIT(ARC_VC_FLAGS_SETTING_PERSISTED_POS),
};

/// List of Volume Control Service characteristics
enum arc_vc_char_type
{
    /// Volume State characteristic
    ARC_VC_CHAR_TYPE_STATE = 0,
    /// Volume Flags characteristic
    ARC_VC_CHAR_TYPE_FLAGS,

    ARC_VC_CHAR_TYPE_NTF_MAX,

    /// Volume Control Point characteristic
    ARC_VC_CHAR_TYPE_CP = ARC_VC_CHAR_TYPE_NTF_MAX,

    ARC_VC_CHAR_TYPE_MAX,
};

/// List of Volume Control Service descriptors
enum arc_vc_desc_type
{
    /// Volume State characteristic
    ARC_VC_DESC_TYPE_CCC_STATE = 0,
    /// Volume Flags characteristic
    ARC_VC_DESC_TYPE_CCC_FLAGS,

    ARC_VC_DESC_TYPE_MAX,
};

/// List of Mute value for Volume Control Service
enum arc_vc_mute
{
    /// Not muted
    ARC_VC_NOT_MUTED = 0,
    /// Muted
    ARC_VC_MUTED
};

/// @} ARC_VC_ENUM

#endif // ARC_VC_H_
