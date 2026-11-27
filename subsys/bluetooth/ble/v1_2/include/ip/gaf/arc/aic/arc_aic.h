/**
 ****************************************************************************************
 *
 * @file arc_aic.h
 *
 * @brief Audio Rendering Control - Audio Input Control - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_AIC_H_
#define ARC_AIC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_AIC Audio Input Control Service
 * @ingroup ARC
 * @brief Description of Audio Input Control Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AIC_ENUM Enumerations
 * @ingroup ARC_AIC
 * @brief Enumerations for Audio Input Control Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_AIC_STRUCT Structures
 * @ingroup ARC_AIC
 * @brief Structures for Audio Input Control Service module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "arc.h"            // Audio Rendering Control Defintiions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_AIC_ENUM
/// @{

/// Position of fields in Audio Input State characteristic value
enum arc_aic_state_pos
{
    /// Gain Setting field
    ARC_AIC_STATE_POS_GAIN = 0,
    /// Mute field
    ARC_AIC_STATE_POS_MUTE,
    /// Gain Mode field
    ARC_AIC_STATE_POS_GAIN_MODE,
    /// Change Counter field
    ARC_AIC_STATE_POS_CHANGE_CNT,

    /// Length of Audio Input State characteristic value
    ARC_AIC_STATE_LEN
};

/// Position of fields in Gain Setting Properties characteristic value
enum arc_aic_gain_prop_pos
{
    /// Gain Settings Units field
    ARC_AIC_GAIN_PROP_POS_UNITS = 0,
    /// Gain Settings Minimum field
    ARC_AIC_GAIN_PROP_POS_MIN,
    /// Gain Settings Maximum field
    ARC_AIC_GAIN_PROP_POS_MAX,

    /// Length of Gain Setting Properties characteristic value
    ARC_AIC_GAIN_PROP_LEN
};

/// Position of fields in Audio Input Type characteristic value
enum arc_aic_type_pos
{
    /// Input Type
    ARC_AIC_TYPE_POS_TYPE = 0,

    /// Length of Audio Input Type characteristic value
    ARC_AIC_TYPE_LEN,
};

/// Position of fields in Audio Input Status characteristic value
enum arc_aic_status_pos
{
    /// Input Status
    ARC_AIC_STATUS_POS_STATUS = 0,

    /// Length of Audio Input Status characteristic value
    ARC_AIC_STATUS_LEN,
};

/// Position of fields in value written in Audio Input Control Point characteristic
enum arc_aic_cp_pos
{
    /// Operation Code field
    ARC_AIC_CP_POS_OPCODE = 0,
    /// Change Counter field
    ARC_AIC_CP_POS_CHANGE_CNT,
    /// Gain Setting field
    ARC_AIC_CP_POS_GAIN,

    /// Minimal length of Audio Input Control Point characteristic value
    ARC_AIC_CP_LEN_MIN = ARC_AIC_CP_POS_CHANGE_CNT + 1,
    ARC_AIC_CP_LEN_MAX = ARC_AIC_CP_POS_GAIN + 1,
};

/// Operation Code values for Audio Input Control Point characteristic
enum arc_aic_opcode
{
    /// Set Gain Setting
    ARC_AIC_OPCODE_SET_GAIN = 1,
    /// Unmute
    ARC_AIC_OPCODE_UNMUTE,
    /// Mute
    ARC_AIC_OPCODE_MUTE,
    /// Set Manual Gain Mode
    ARC_AIC_OPCODE_SET_MANUAL_MODE,
    /// Set Automatic Gain Mode
    ARC_AIC_OPCODE_SET_AUTO_MODE,
};

/// Application error codes
enum arc_aic_err
{
    /// Invalid Change Counter
    ARC_AIC_ERR_INVALID_CHANGE_CNT = 0x80,
    /// Opcode Not Supported
    ARC_AIC_ERR_OPCODE_NOT_SUPP,
    /// Mute Disabled
    ARC_AIC_ERR_MUTE_DISABLED,
    /// Value Out of Range
    ARC_AIC_ERR_OUT_OF_RANGE,
    /// Gain Mode Change Not Allowed
    ARC_AIC_ERR_GAIN_MODE_CHANGE_NOT_ALLOWED,
};

/// List of Audio Input Control Service characteristics
enum arc_aic_char_type
{
    /// Audio Input State characteristic
    ARC_AIC_CHAR_TYPE_STATE = 0,
    /// Audio Input Status characteristic
    ARC_AIC_CHAR_TYPE_STATUS,
    /// Audio Input Description characteristic
    ARC_AIC_CHAR_TYPE_DESC,

    ARC_AIC_CHAR_TYPE_NTF_MAX,

    /// Gain Setting Properties characteristic
    ARC_AIC_CHAR_TYPE_GAIN_PROP = ARC_AIC_CHAR_TYPE_NTF_MAX,
    /// Audio Input Type characteristic
    ARC_AIC_CHAR_TYPE_TYPE,
    /// Audio Input Control Point characteristic
    ARC_AIC_CHAR_TYPE_CP,

    ARC_AIC_CHAR_TYPE_MAX,
};

/// List of Audio Input Control Service descriptors
enum arc_aic_desc_type
{
    /// Audio Input State characteristic
    ARC_AIC_DESC_TYPE_CCC_STATE = 0,
    /// Audio Input Status characteristic
    ARC_AIC_DESC_TYPE_CCC_STATUS,
    /// Audio Input Description characteristic
    ARC_AIC_DESC_TYPE_CCC_DESC,

    ARC_AIC_DESC_TYPE_MAX,
};

/// Mute values
enum arc_aic_mute
{
    /// Not muted
    ARC_AIC_MUTE_NOT_MUTED = 0,
    /// Muted
    ARC_AIC_MUTE_MUTED,
    /// Disabled
    ARC_AIC_MUTE_DISABLED,

    ARC_AIC_MUTE_MAX,
};

/// Gain Mode values
enum arc_aic_gain_mode
{
    /// Manual only
    ARC_AIC_GAIN_MODE_MANUAL_ONLY = 0,
    /// Automatic only
    ARC_AIC_GAIN_MODE_AUTO_ONLY,
    /// Manual
    ARC_AIC_GAIN_MODE_MANUAL,
    /// Automatic
    ARC_AIC_GAIN_MODE_AUTO,

    ARC_AIC_GAIN_MODE_MAX
};

/// Input Status values
enum arc_aic_input_status
{
    /// Inactive
    ARC_AIC_INPUT_STATUS_INACTIVE = 0,
    /// Active
    ARC_AIC_INPUT_STATUS_ACTIVE,

    ARC_AIC_INPUT_STATUS_MAX,
};

/// Input Type values
enum arc_aic_input_type
{
    /// Unspecified Input
    ARC_AIC_INPUT_TYPE_UNSPEC = 0,
    /// Bluetooth Audio Stream
    ARC_AIC_INPUT_TYPE_BT,
    /// Microphone
    ARC_AIC_INPUT_TYPE_MICROPHONE,
    /// Analog Interface
    ARC_AIC_INPUT_TYPE_ANALOG,
    /// Digital
    ARC_AIC_INPUT_TYPE_DIGITAL,
    /// Radio (AM/FM/XM/...)
    ARC_AIC_INPUT_TYPE_RADIO,
    /// Streaming Audio Source
    ARC_AIC_INPUT_TYPE_STREAMING,

    ARC_AIC_INPUT_TYPE_MAX,
};

/// @} ARC_AIC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_AIC_STRUCT
/// @{

/// Input state structure
typedef struct arc_aic_state
{
    /// Gain
    int8_t gain;
    /// Mute
    uint8_t mute;
    /// Gain mode
    uint8_t gain_mode;
} arc_aic_state_t;

/// Gain Setting Properties structure
typedef struct arc_aic_gain_prop
{
    /// Gain Units
    uint8_t gain_units;
    /// Gain Minimum
    int8_t gain_min;
    /// Gain Maximum
    int8_t gain_max;
} arc_aic_gain_prop_t;

/// @} ARC_AIC_STRUCT

#endif // ARC_AIC_H_
