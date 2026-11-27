/**
 ****************************************************************************************
 *
 * @file tmap_tmas.h
 *
 * @brief Telephony and Media Audio Profile - Telephony and Media Audio Service Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TMAP_TMAS_H_
#define TMAP_TMAS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TMAP_TMAS Telephony and Media Audio Service Server
 * @ingroup TMAP
 * @brief Description of Telephony and Media Service Server module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TMAP_TMAS_NATIVE_API Native API
 * @ingroup TMAP_TMAS
 * @brief Description of Native API for Telephony and Media Service Server module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tmap.h"               // Telephony and Media Audio Profile Definitions

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_TMAP_TMAS)

/// @addtogroup TMAP_TMAS_NATIVE_API
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of TMAP Server module
 *
 * @param[in] p_cfg_param          Pointer to configuration structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t tmap_tmas_configure(const tmap_tmas_cfg_param_t* p_cfg_param);

/**
 ****************************************************************************************
 * @return If use of Server Role for Telephony and Media Audio Service has been configured
 ****************************************************************************************
 */
bool tmap_tmas_is_configured();

/**
 ****************************************************************************************
 * @return Supported TMAP roles values
 ****************************************************************************************
 */
uint16_t tmap_tmas_get_roles(void);

/// @} TMAP_TMAS_NATIVE_API
#endif //(GAF_TMAP_TMAS)

#endif // TMAP_TMAS_H_
