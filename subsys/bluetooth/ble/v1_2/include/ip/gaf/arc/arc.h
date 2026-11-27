/**
 ****************************************************************************************
 *
 * @file arc.h
 *
 * @brief Audio Rendering Control - Header file
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ARC_H_
#define ARC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ARC Audio Rendering Control (ARC)
 * @ingroup GAF
 * @brief Description of Audio Rendering Control block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_COMMON Common
 * @ingroup ARC
 * @brief Description of common API for Audio Rendering Control block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ARC_ENUM Enumerations
 * @ingroup ARC_COMMON
 * @brief Enumerations for Audio Rendering Control block
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf_cfg.h"         // GAF Configuration
#include "gaf.h"             // GAF Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ARC_ENUM
/// @{


/// Module type values for Audio Rendering Control block
enum arc_module_type
{
    /// Common module
    ARC_MODULE_COMMON = 0,
    /// Volume Control Service Server module
    ARC_MODULE_VCS = 1,
    /// Volume Control Service Client module
    ARC_MODULE_VCC = 2,
    /// Audio Input Control Service Server module
    ARC_MODULE_AICS = 3,
    /// Audio Input Control Service Client module
    ARC_MODULE_AICC = 4,
    /// Volume Offset Control Service Server module
    ARC_MODULE_VOCS = 5,
    /// Volume Offset Control Service Client module
    ARC_MODULE_VOCC = 6,
    /// Microphone Control Service Server module
    ARC_MODULE_MICS = 7,
    /// Microphone Control Service Client module
    ARC_MODULE_MICC = 8,

    /// Maximum value
    ARC_MODULE_MAX,
};

/// @} ARC_ENUM

#endif // ARC_H_
