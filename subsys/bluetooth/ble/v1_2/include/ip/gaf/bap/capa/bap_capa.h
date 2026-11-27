/**
 ****************************************************************************************
 *
 * @file bap_capa.h
 *
 * @brief Basic Audio Profile - Capabilities - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_CAPA_H_
#define BAP_CAPA_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA Capabilities module
 * @ingroup BAP
 * @brief Description of Basic Audio Profile Capabilities module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_COMMON Common
 * @ingroup BAP_CAPA
 * @brief Description of common API for Basic Audio Profile Capabilities module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_CAPA_ENUM Enumerations
 * @ingroup BAP_CAPA
 * @brief Enumerations for Basic Audio Profile Capabilities module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap.h"            // Basic Audio Profile Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_CAPA_ENUM
/// @{

/// Characteristic type values for Published Audio Capabilities Service
enum bap_capa_char_type
{
    /// Available Audio Contexts characteristic
    BAP_CAPA_CHAR_TYPE_CONTEXT_AVA = 0,
    /// Supported Audio Contexts characteristic
    BAP_CAPA_CHAR_TYPE_CONTEXT_SUPP,
    /// Sink Audio Locations characteristic
    BAP_CAPA_CHAR_TYPE_LOC_SINK,
    /// Source Audio Locations characteristic
    BAP_CAPA_CHAR_TYPE_LOC_SRC,
    /// PAC characteristic
    BAP_CAPA_CHAR_TYPE_PAC,

    BAP_CAPA_CHAR_TYPE_MAX,
};

/// Context type values
enum bap_capa_context_type
{
    /// Supported Audio Contexts
    BAP_CAPA_CONTEXT_TYPE_SUPP = 0,
    /// Available Audio Contexts
    BAP_CAPA_CONTEXT_TYPE_AVA,

    BAP_CAPA_CONTEXT_TYPE_MAX
};

/// @} BAP_CAPA_ENUM

#endif // BAP_CAPA_H_
