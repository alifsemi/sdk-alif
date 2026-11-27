/**
 ****************************************************************************************
 *
 * @file atc_csi.h
 *
 * @brief Audio Topology Control - Coordinated Set Identification - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ATC_CSI_H_
#define ATC_CSI_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ATC_CSIS Coordinated Set Identification Service
 * @ingroup ATC
 * @brief Description of Coordinated Set Identification Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ATC_CSIS_STRUCT Structures
 * @ingroup ATC_CSIS
 * @brief Structures for Coordinated Set Identification Service module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "atc.h"            // Audio Topology Control Definitions

#if (GAF_ATC_CSISM || GAF_ATC_CSISC)
#include "csis.h"           // Coordinated Set Identification Service Definitions
#endif //(GAF_ATC_CSISM || GAF_ATC_CSISC)

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// @addtogroup ATC_CSIS_STRUCT
/// @{

/// RSI
typedef struct atc_csis_rsi
{
    /// RSI
    uint8_t rsi[6];
} atc_csis_rsi_t;

/// @} ATC_CSIS_STRUCT

#endif // ATC_CSI_H_
