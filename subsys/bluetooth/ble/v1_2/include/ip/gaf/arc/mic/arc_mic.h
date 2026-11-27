/**
 ****************************************************************************************
 *
 * @file arc_mic.h
 *
 * @brief Audio Rendering Control - Microphone Control - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_MIC_H_
#define ARC_MIC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_MIC Microphone Control Service
 * @ingroup ARC
 * @brief Description of Microphone Control Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_MIC_ENUM Enumerations
 * @ingroup ARC_MIC
 * @brief Enumerations for Microphone Control Service module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc.h"            // Audio Rendering Control Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_MIC_ENUM
/// @{

/// Position of fields in Mute characteristic value
enum arc_mic_mute_pos
{
    /// Mute field
    ARC_MIC_MUTE_POS_MUTE = 0,

    /// Length of Mute characteristic value
    ARC_MIC_MUTE_LEN
};

/// List of Microphone Control Service characteristics
enum arc_mic_char_type
{
    /// Mute characteristic
    ARC_MIC_CHAR_TYPE_MUTE = 0,

    /// Number of characteristics
    ARC_MIC_CHAR_TYPE_MAX,
};

/// List of Microphone Control Service descriptors
enum arc_mic_desc_type
{
    /// Mute characteristic
    ARC_MIC_DESC_TYPE_CCC_MUTE = 0,

    /// Number of descriptors
    ARC_MIC_DESC_TYPE_MAX,
};

/// List of Mute value for Microphone Control Service
enum arc_mic_mute
{
    /// Not muted
    ARC_MIC_MUTE_NOT_MUTED = 0,
    /// Muted
    ARC_MIC_MUTE_MUTED,
    /// Disabled
    ARC_MIC_MUTE_DISABLED,

    ARC_MIC_MUTE_MAX,
};

/// Error codes for Microphone Control Service
enum arc_mic_err
{
    /// Mute disabled
    ARC_MIC_ERR_MUTE_DISABLED = 0x80,
};

/// @} ARC_MIC_ENUM

#endif // ARC_MIC_H_
