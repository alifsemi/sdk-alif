/**
 ****************************************************************************************
 *
 * @file cgmss_cfg.h
 *
 * @brief Header file - Continuous Glucose Monitoring Service - Configuration
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CGMSS_CFG_H_
#define CGMSS_CFG_H_

#include "rom_build_cfg.h"

/*
 * DEFINES
 ****************************************************************************************
 */

// Support for flexible configuration of the service
#if (defined(CFG_CGMSS_FLEXIBLE))
#define CGMSS_FLEXIBLE (1)
#else // (defined(CFG_CGMSS_FLEXIBLE))
#define CGMSS_FLEXIBLE (0)
#endif // (defined(CFG_CGMSS_FLEXIBLE))

// Support of indication for CGM Feature characteristic
#if (defined(CFG_CGMSS_ALL) || defined(CFG_CGMSS_FEATURE_IND))
#define CGMSS_FEATURE_IND (1)
#else // (defined(CFG_CGMSS_ALL) || defined(CFG_CGMSS_FEATURE_IND))
#define CGMSS_FEATURE_IND (0)
#endif // (defined(CFG_CGMSS_ALL) || defined(CFG_CGMSS_FEATURE_IND))

// Support of indication for E2E-CRC
#if (defined(CFG_CGMSS_ALL) || defined(CFG_CGMSS_E2E_CRC))
#define CGMSS_E2E_CRC (1)
#else // (defined(CFG_CGMSS_ALL) || defined(CFG_CGMSS_E2E_CRC))
#define CGMSS_E2E_CRC (0)
#endif // (defined(CFG_CGMSS_ALL) || defined(CFG_CGMSS_E2E_CRC))

#endif // CGMSS_CFG_H_
