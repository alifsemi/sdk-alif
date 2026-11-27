/**
 ****************************************************************************************
 *
 * @file arc_voc.h
 *
 * @brief Audio Rendering Control - Volume Offset Control - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_VOC_H_
#define ARC_VOC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC_VOC Volume Offset Control Service
 * @ingroup ARC
 * @brief Description of Volume Offset Control Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOC_DEF Definitions
 * @ingroup ARC_VOC
 * @brief Definitions for Volume Offset Control Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_VOC_ENUM Enumerations
 * @ingroup ARC_VOC
 * @brief Enumerations for Volume Offset Control Service module
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

/// @addtogroup ARC_VOC_DEF
/// @{

/// Minimum offset value
#define ARC_VOC_OFFSET_MIN           (-255)
/// Maximum offset value
#define ARC_VOC_OFFSET_MAX           (255)

/// @} ARC_VOC_DEF

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_VOC_ENUM
/// @{

/// Position of fields in Offset State characteristic value
enum arc_voc_offset_pos
{
    /// Volume Offset field
    ARC_VOC_OFFSET_POS_OFFSET = 0,
    /// Change Counter field
    ARC_VOC_OFFSET_POS_CHANGE_CNT = 2,

    /// Length of Offset State characteristic value
    ARC_VOC_OFFSET_LEN
};

/// Position of fields in Audio Location characteristic value
enum arc_voc_location_pos
{
    /// Audio Location field
    ARC_VOC_LOCATION_POS_LOC = 0,

    /// Length of Audio Location characteristic value
    ARC_VOC_LOCATION_LEN = 4
};

/// Position of fields in value written in Volume Offset Control Point characteristic
enum arc_voc_cp_pos
{
    /// Operation Code field
    ARC_VOC_CP_POS_OPCODE = 0,
    /// Change Counter field
    ARC_VOC_CP_POS_CHANGE_CNT,
    /// Volume Offset field
    ARC_VOC_CP_POS_OFFSET,

    /// Length of Volume Offset Control Point characteristic value
    ARC_VOC_CP_LEN = ARC_VOC_CP_POS_OFFSET + 2,
};

/// Volume Offset Operation Code values
enum arc_voc_opcode
{
    /// Set Volume Offset
    ARC_VOC_OPCODE_SET_OFFSET = 1,

    ARC_VOC_OPCODE_MAX,
    ARC_VOC_OPCODE_MIN = ARC_VOC_OPCODE_SET_OFFSET,
};

/// Application error codes
enum arc_voc_err
{
    /// Invalid change counter
    ARC_VOC_ERR_INVALID_CHANGE_CNT = 0x80,
    /// Opcode not supported
    ARC_VOC_ERR_OPCODE_NOT_SUPP,
    /// Value out of range
    ARC_VOC_ERR_VALUE_OUT_OF_RANGE,
};

/// Set type values
enum arc_voc_set_type
{
    /// Volume offset
    ARC_VOC_SET_TYPE_OFFSET = 0,
    /// Audio location
    ARC_VOC_SET_TYPE_LOCATION,

    ARC_VOC_SET_TYPE_MAX,
};

/// List of Volume Offset Control Service characteristics
enum arc_voc_char_type
{
    /// Volume Offset State characteristic
    ARC_VOC_CHAR_TYPE_OFFSET = 0,
    /// Audio Location characteristic
    ARC_VOC_CHAR_TYPE_LOC,
    /// Audio Output Description characteristic
    ARC_VOC_CHAR_TYPE_DESC,

    ARC_VOC_CHAR_TYPE_NTF_MAX,

    /// Volume Offset Control Point characteristic
    ARC_VOC_CHAR_TYPE_CP = ARC_VOC_CHAR_TYPE_NTF_MAX,

    ARC_VOC_CHAR_TYPE_MAX,
};

/// List of Volume Control Service descriptors
enum arc_voc_desc_type
{
    /// Offset State characteristic
    ARC_VOC_DESC_TYPE_CCC_OFFSET = 0,
    /// Audio Location characteristic
    ARC_VOC_DESC_TYPE_CCC_LOC,
    /// Audio Output Description characteristic
    ARC_VOC_DESC_TYPE_CCC_DESC,

    ARC_VOC_DESC_TYPE_MAX,
};

/// @} ARC_VOC_ENUM

#endif // ARC_VOC_H_
