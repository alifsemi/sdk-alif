/**
 ****************************************************************************************
 *
 * @file cap_cac.h
 *
 * @brief Common Audio Profile - Common Audio Service Server - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CAP_CAS_H_
#define CAP_CAS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CAP_CAS Common Audio Service Client
 * @ingroup CAP
 * @brief Description of Common Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_CAS_NATIVE_API Native API
 * @ingroup CAP_CAS
 * @brief Description of Native API for Common Audio Service Client module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_CAS_FUNCTION Functions
 * @ingroup CAP_CAS_NATIVE_API
 * @brief Description of functions for Common Audio Service Client module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "cap.h"                // Common Audio Profile Definitions
#include "prf_types.h"          // Profile common types

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_CAP_CAS)
/// @addtogroup CAP_CAS_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of CAP Server module
 *
 * @param[in] p_cfg_param          Pointer to configuration structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t cap_cas_configure(const cap_cas_cfg_param_t* p_cfg_param);

/// @} CAP_CAS_FUNCTION
#endif //(GAF_CAP_CAS)

#endif // CAP_CAS_H_
