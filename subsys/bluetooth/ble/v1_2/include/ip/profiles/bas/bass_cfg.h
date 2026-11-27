/**
 ****************************************************************************************
 *
 * @file bass_cfg.h
 *
 * @brief Header file - Battery Service - Configuration
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BASS_CFG_H_
#define BASS_CFG_H_

#include "rom_build_cfg.h"

/*
 * DEFINES
 ****************************************************************************************
 */

// Support for flexible configuration of the service
#if (defined(CFG_BASS_FLEXIBLE))
#define BASS_FLEXIBLE (1)
#else // (defined(CFG_BASS_FLEXIBLE))
#define BASS_FLEXIBLE (0)
#endif // (defined(CFG_BASS_FLEXIBLE))

// Support of Battery Level Status characteristic
#if (defined(CFG_BASS_ALL) || defined(CFG_BASS_LEVEL_STATUS))
#define BASS_LEVEL_STATUS (1)
#else // (defined(CFG_BASS_ALL) || defined(CFG_BASS_LEVEL_STATUS))
#define BASS_LEVEL_STATUS (0)
#endif // (defined(CFG_BASS_ALL) || defined(CFG_BASS_LEVEL_STATUS))

// Support of Estimated Service Date characteristic
#if (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_ESTIMATED_SERVICE_DATE)))
#define BASS_ESTIMATED_SERVICE_DATE (1)
#else // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_ESTIMATED_SERVICE_DATE)))
#define BASS_ESTIMATED_SERVICE_DATE (0)
#endif // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_ESTIMATED_SERVICE_DATE)))

// Support of Battery Critical Status characteristic
#if (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_CRITICAL_STATUS)))
#define BASS_CRITICAL_STATUS (1)
#else // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_CRITICAL_STATUS)))
#define BASS_CRITICAL_STATUS (0)
#endif // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_CRITICAL_STATUS)))

// Support of Battery Energy Status characteristic
#if (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_ENERGY_STATUS)))
#define BASS_ENERGY_STATUS (1)
#else // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_ENERGY_STATUS)))
#define BASS_ENERGY_STATUS (0)
#endif // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_ENERGY_STATUS)))

// Support of Battery Time Status characteristic
#if (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_TIME_STATUS)))
#define BASS_TIME_STATUS (1)
#else // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_TIME_STATUS)))
#define BASS_TIME_STATUS (0)
#endif // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_TIME_STATUS)))

// Support of Battery Health Status characteristic
#if (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_HEALTH_STATUS)))
#define BASS_HEALTH_STATUS (1)
#else // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_HEALTH_STATUS)))
#define BASS_HEALTH_STATUS (0)
#endif // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_HEALTH_STATUS)))

// Support of Battery Health Information characteristic
#if (defined(CFG_BASS_ALL) || (BASS_HEALTH_STATUS && defined(CFG_BASS_HEALTH_INFORMATION)))
#define BASS_HEALTH_INFORMATION (1)
#else // (defined(CFG_BASS_ALL) || (BASS_HEALTH_STATUS && defined(CFG_BASS_HEALTH_INFORMATION)))
#define BASS_HEALTH_INFORMATION (0)
#endif // (defined(CFG_BASS_ALL) || (BASS_HEALTH_STATUS && defined(CFG_BASS_HEALTH_INFORMATION)))

// Support of Battery Information characteristic
#if (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_INFORMATION)))
#define BASS_INFORMATION (1)
#else // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_INFORMATION)))
#define BASS_INFORMATION (0)
#endif // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_INFORMATION)))

// Support of Manufacturer Name String characteristic
#if (defined(CFG_BASS_ALL) || defined(CFG_BASS_MANUFACTURER_NAME))
#define BASS_MANUFACTURER_NAME (1)
#else // (defined(CFG_BASS_ALL) || defined(CFG_BASS_MANUFACTURER_NAME))
#define BASS_MANUFACTURER_NAME (0)
#endif // (defined(CFG_BASS_ALL) || defined(CFG_BASS_MANUFACTURER_NAME))

// Support of Model Number String characteristic
#if (defined(CFG_BASS_ALL) || defined(CFG_BASS_MODEL_NUMBER))
#define BASS_MODEL_NUMBER (1)
#else // (defined(CFG_BASS_ALL) || defined(CFG_BASS_MODEL_NUMBER))
#define BASS_MODEL_NUMBER (0)
#endif // (defined(CFG_BASS_ALL) || defined(CFG_BASS_MODEL_NUMBER))

// Support of Serial Number String characteristic
#if (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_SERIAL_NUMBER)))
#define BASS_SERIAL_NUMBER (1)
#else // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_SERIAL_NUMBER)))
#define BASS_SERIAL_NUMBER (0)
#endif // (defined(CFG_BASS_ALL) || (BASS_LEVEL_STATUS && defined(CFG_BASS_SERIAL_NUMBER)))

// Support of multi-instantiation of Battery Service
#if (!defined(CFG_BASS_MULTI) || (CFG_BASS_MULTI < 1))
#define BASS_MULTI (1)
#else // (!defined(CFG_BASS_MULTI) || (CFG_BASS_MULTI < 1))
#define BASS_MULTI (CFG_BASS_MULTI)
#endif // (!defined(CFG_BASS_MULTI) || (CFG_BASS_MULTI < 1))

// Support of Characteristic Presentation Format descriptor for Battery Level characteristic
#if (defined(CFG_BASS_ALL) || defined(CFG_BASS_PRESENTATION_FORMAT) || (BASS_MULTI > 1))
#define BASS_PRESENTATION_FORMAT (1)
#else // (defined(CFG_BASS_ALL) || defined(CFG_BASS_PRESENTATION_FORMAT))
#define BASS_PRESENTATION_FORMAT (0)
#endif // (defined(CFG_BASS_ALL) || defined(CFG_BASS_PRESENTATION_FORMAT))

/*
 * Support of indications for Battery Level Status, Estimated Service Date, Battery Energy Status, Battery Time Status
 * and Battery Health Status characteristics
 */
#if (defined(CFG_BASS_ALL) || defined(CFG_BASS_CRITICAL))
#define BASS_CRITICAL (1)
#else // (defined(CFG_BASS_ALL) || defined(CFG_BASS_CRITICAL))
#define BASS_CRITICAL (0)
#endif // (defined(CFG_BASS_ALL) || defined(CFG_BASS_CRITICAL))

// Support of indications
#if (BASS_INFORMATION || BASS_HEALTH_INFORMATION || BASS_MANUFACTURER_NAME || BASS_MODEL_NUMBER || BASS_SERIAL_NUMBER \
     || BASS_CRITICAL_STATUS                                                                                          \
     || (BASS_CRITICAL                                                                                                \
         && (BASS_LEVEL_STATUS || BASS_ESTIMATED_SERVICE_DATE || BASS_ENERGY_STATUS || BASS_TIME_STATUS               \
             || BASS_HEALTH_STATUS)))
#define BASS_INDICATIONS (1)
#else // (BASS_INFORMATION || BASS_HEALTH_INFORMATION ...)
#define BASS_INDICATIONS (0)
#endif // (BASS_INFORMATION || BASS_HEALTH_INFORMATION ...)

// Support of at least one optional characteristic
#if (BASS_INFORMATION || BASS_HEALTH_INFORMATION || BASS_MANUFACTURER_NAME || BASS_MODEL_NUMBER || BASS_SERIAL_NUMBER \
     || BASS_LEVEL_STATUS || BASS_ESTIMATED_SERVICE_DATE || BASS_ENERGY_STATUS || BASS_TIME_STATUS                    \
     || BASS_HEALTH_STATUS || BASS_CRITICAL_STATUS)
#define BASS_OPTIONAL_CHAR (1)
#else // (BASS_INFORMATION || BASS_HEALTH_INFORMATION ...)
#define BASS_OPTIONAL_CHAR (0)
#endif // (BASS_INFORMATION || BASS_HEALTH_INFORMATION ...)

#endif // BASS_CFG_H_
