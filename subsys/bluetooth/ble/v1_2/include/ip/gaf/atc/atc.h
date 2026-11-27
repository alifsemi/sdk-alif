/**
 ****************************************************************************************
 *
 * @file atc.h
 *
 * @brief Audio Topology Control - Header file
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ATC_H_
#define ATC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ATC Audio Topology Control (ATC)
 * @ingroup GAF
 * @brief Description of Audio Topology Control block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_COMMON Common
 * @ingroup ATC
 * @brief Description of common API for Audio Topology Control block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_ENUM Enumerations
 * @ingroup ATC_COMMON
 * @brief Enumerations for Audio Topology Control block
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf_cfg.h"         // GAF Configuration

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ATC_ENUM
/// @{

/// Module type values for Audio Topology Control block
enum atc_module_type
{
    /// Common module
    ATC_MODULE_COMMON = 0,
    /// Coordinated Set Identification Service Set Member module
    ATC_MODULE_CSISM = 1,
    /// Coordinated Set Identification Service Set Coordinator module
    ATC_MODULE_CSISC = 2,

    /// Maximum value
    ATC_MODULE_MAX,
};

/// @} ATC_ENUM

#endif // ATC_H_
