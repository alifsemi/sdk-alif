/**
 ****************************************************************************************
 *
 * @file diss_cfg.h
 *
 * @brief Header file - Device Information Service Server - Configuration
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef DISS_CFG_H_
#define DISS_CFG_H_

#include "rom_build_cfg.h"

/*
 * DEFINES
 ****************************************************************************************
 */

// Support for flexible configuration of the service
#if (defined(CFG_DISS_FLEXIBLE))
#define DISS_FLEXIBLE (1)
#else // (defined(CFG_DISS_FLEXIBLE))
#define DISS_FLEXIBLE (0)
#endif // (defined(CFG_DISS_FLEXIBLE))

// Support of Manufacturer Name String characteristic
#if (defined(CFG_DISS_ALL) || defined(DISS_MANUFACTURER))
#define DISS_MANUFACTURER (1)
#else // (defined(CFG_DISS_ALL) || defined(DISS_MANUFACTURER))
#define DISS_MANUFACTURER (0)
#endif // (defined(CFG_DISS_ALL) || defined(DISS_MANUFACTURER))

// Support of Model Number String characteristic
#if (defined(CFG_DISS_ALL) || defined(DISS_MODEL_NB))
#define DISS_MODEL_NB (1)
#else // (defined(CFG_DISS_ALL) || defined(DISS_MODEL_NB))
#define DISS_MODEL_NB (0)
#endif // (defined(CFG_DISS_ALL) || defined(DISS_MODEL_NB))

// Support of Serial Number String characteristic
#if (defined(CFG_DISS_ALL) || defined(DISS_SERIAL_NB))
#define DISS_SERIAL_NB (1)
#else // (defined(CFG_DISS_ALL) || defined(DISS_SERIAL_NB))
#define DISS_SERIAL_NB (0)
#endif // (defined(CFG_DISS_ALL) || defined(DISS_SERIAL_NB))

// Support of Hardware Revision String characteristic
#if (defined(CFG_DISS_ALL) || defined(DISS_HW_REV))
#define DISS_HW_REV (1)
#else // (defined(CFG_DISS_ALL) || defined(DISS_HW_REV))
#define DISS_HW_REV (0)
#endif // (defined(CFG_DISS_ALL) || defined(DISS_HW_REV))

// Support of Firmware Revision String characteristic
#if (defined(CFG_DISS_ALL) || defined(DISS_FW_REV))
#define DISS_FW_REV (1)
#else // (defined(CFG_DISS_ALL) || defined(DISS_FW_REV))
#define DISS_FW_REV (0)
#endif // (defined(CFG_DISS_ALL) || defined(DISS_FW_REV))

// Support of Software Revision String characteristic
#if (defined(CFG_DISS_ALL) || defined(DISS_SW_REV))
#define DISS_SW_REV (1)
#else // (defined(CFG_DISS_ALL) || defined(DISS_SW_REV))
#define DISS_SW_REV (0)
#endif // (defined(CFG_DISS_ALL) || defined(DISS_SW_REV))

// Support of System ID characteristic
#if (defined(CFG_DISS_ALL) || defined(DISS_SYSTEM_ID))
#define DISS_SYSTEM_ID (1)
#else // (defined(CFG_DISS_ALL) || defined(DISS_SYSTEM_ID))
#define DISS_SYSTEM_ID (0)
#endif // (defined(CFG_DISS_ALL) || defined(DISS_SYSTEM_ID))

// Support of IEEE 11073-20601 Regulatory Certification Data List characteristic
#if (defined(CFG_DISS_ALL) || defined(DISS_IEEE))
#define DISS_IEEE (1)
#else // (defined(CFG_DISS_ALL) || defined(DISS_IEEE))
#define DISS_IEEE (0)
#endif // (defined(CFG_DISS_ALL) || defined(DISS_IEEE))

// Support of PnP ID characteristic
#if (defined(CFG_DISS_ALL) || defined(DISS_PNP_ID))
#define DISS_PNP_ID (1)
#else // (defined(DISS_PNP_ID) || defined(DISS_PNP_ID))
#define DISS_PNP_ID (0)
#endif // (defined(CFG_DISS_ALL) || defined(DISS_PNP_ID))

// Support of UDI for Medical Devices characteristic
#if (defined(CFG_DISS_ALL) || defined(DISS_UDI))
#define DISS_UDI (1)
#else // (defined(DISS_PNP_ID) || defined(DISS_UDI))
#define DISS_UDI (0)
#endif // (defined(CFG_DISS_ALL) || defined(DISS_UDI))

#endif // DISS_CFG_H_
