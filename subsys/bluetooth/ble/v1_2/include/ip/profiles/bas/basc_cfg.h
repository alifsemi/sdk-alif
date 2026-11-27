/**
 ****************************************************************************************
 *
 * @file basc_cfg.h
 *
 * @brief Header file - Battery Service Client - Configuration
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BASC_CFG_H_
#define BASC_CFG_H_

#include "rom_build_cfg.h"

/*
 * DEFINES
 ****************************************************************************************
 */

// Support of Battery Level Status characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_LEVEL_STATUS))
#define BASC_LEVEL_STATUS (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_LEVEL_STATUS))
#define BASC_LEVEL_STATUS (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_LEVEL_STATUS))

// Support of Estimated Service Date characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_ESTIMATED_SERVICE_DATE))
#define BASC_ESTIMATED_SERVICE_DATE (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_ESTIMATED_SERVICE_DATE))
#define BASC_ESTIMATED_SERVICE_DATE (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_ESTIMATED_SERVICE_DATE))

// Support of Battery Critical Status characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_CRITICAL_STATUS))
#define BASC_CRITICAL_STATUS (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_CRITICAL_STATUS))
#define BASC_CRITICAL_STATUS (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_CRITICAL_STATUS))

// Support of Battery Energy Status characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_ENERGY_STATUS))
#define BASC_ENERGY_STATUS (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_ENERGY_STATUS))
#define BASC_ENERGY_STATUS (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_ENERGY_STATUS))

// Support of Battery Time Status characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_TIME_STATUS))
#define BASC_TIME_STATUS (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_TIME_STATUS))
#define BASC_TIME_STATUS (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_TIME_STATUS))

// Support of Battery Health Status characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_HEALTH_STATUS))
#define BASC_HEALTH_STATUS (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_HEALTH_STATUS))
#define BASC_HEALTH_STATUS (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_HEALTH_STATUS))

// Support of Battery Health Information characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_HEALTH_INFORMATION))
#define BASC_HEALTH_INFORMATION (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_HEALTH_INFORMATION))
#define BASC_HEALTH_INFORMATION (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_HEALTH_INFORMATION))

// Support of Battery Information characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_INFORMATION))
#define BASC_INFORMATION (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_INFORMATION))
#define BASC_INFORMATION (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_INFORMATION))

// Support of Manufacturer Name String characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_MANUFACTURER_NAME))
#define BASC_MANUFACTURER_NAME (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_MANUFACTURER_NAME))
#define BASC_MANUFACTURER_NAME (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_MANUFACTURER_NAME))

// Support of Model Number String characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_MODEL_NUMBER))
#define BASC_MODEL_NUMBER (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_MODEL_NUMBER))
#define BASC_MODEL_NUMBER (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_MODEL_NUMBER))

// Support of Serial Number String characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_SERIAL_NUMBER))
#define BASC_SERIAL_NUMBER (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_SERIAL_NUMBER))
#define BASC_SERIAL_NUMBER (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_SERIAL_NUMBER))

// Number of supported Battery Service instances
#if (!defined(CFG_BASC_MULTI) || (CFG_BASC_MULTI < 1))
#define BASC_MULTI (1)
#else // (!defined(CFG_BASC_MULTI) || (CFG_BASC_MULTI < 1))
#define BASC_MULTI (CFG_BASC_MULTI)
#endif // (!defined(CFG_BASC_MULTI) || (CFG_BASC_MULTI < 1))

// Support of Characteristic Presentation Format descriptor for Battery Level characteristic
#if (defined(CFG_BASC_ALL) || defined(CFG_BASC_PRESENTATION_FORMAT))
#define BASC_PRESENTATION_FORMAT (1)
#else // (defined(CFG_BASC_ALL) || defined(CFG_BASC_PRESENTATION_FORMAT))
#define BASC_PRESENTATION_FORMAT (0)
#endif // (defined(CFG_BASC_ALL) || defined(CFG_BASC_PRESENTATION_FORMAT))

#endif // BASC_CFG_H_
