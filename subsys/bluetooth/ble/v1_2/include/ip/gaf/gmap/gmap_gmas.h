/**
 ****************************************************************************************
 *
 * @file gmap_gmas.h
 *
 * @brief Gaming Audio Profile - Gaming Audio Service Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GMAP_GMAS_H_
#define GMAP_GMAS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GMAP_GMAS Gaming Audio Service Server
 * @ingroup GMAP
 * @brief Description of Gaming Audio Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GMAP_GMAS_NATIVE_API Native API
 * @ingroup GMAP_GMAS
 * @brief Description of Native API for Gaming Audio Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gmap.h"               // Gaming Audio Profile Definitions

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_GMAP_GMAS)

/// @addtogroup GMAP_GMAS_NATIVE_API
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of GMAP Server module
 *
 * @param[in] p_cfg_param          Pointer to configuration structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gmap_gmas_configure(const gmap_gmas_cfg_param_t* p_cfg_param);

/**
 ****************************************************************************************
 * @return If use of Server Role for Gaming Audio Service has been configured
 ****************************************************************************************
 */
bool gmap_gmas_is_configured(void);

/**
 ****************************************************************************************
 * @return Supported GMAP roles values
 ****************************************************************************************
 */
uint8_t gmap_gmas_get_roles(void);

/// @} GMAP_GMAS_NATIVE_API

#endif //(GAF_GMAP_GMAS)

#endif // GMAP_GMAS_H_
