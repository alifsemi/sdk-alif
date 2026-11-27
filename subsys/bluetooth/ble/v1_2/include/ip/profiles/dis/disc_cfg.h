/**
 ****************************************************************************************
 *
 * @file disc_cfg.h
 *
 * @brief Header file - Device Information Service Client - Configuration
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef DISC_CFG_H_
#define DISC_CFG_H_

#include "rom_build_cfg.h"

/*
 * DEFINES
 ****************************************************************************************
 */

// Support of Manufacturer Name String characteristic
#if (defined(CFG_DISC_ALL) || defined(DISC_MANUFACTURER))
#define DISC_MANUFACTURER (1)
#else // (defined(CFG_DISC_ALL) || defined(DISC_MANUFACTURER))
#define DISC_MANUFACTURER (0)
#endif // (defined(CFG_DISC_ALL) || defined(DISC_MANUFACTURER))

// Support of Model Number String characteristic
#if (defined(CFG_DISC_ALL) || defined(DISC_MODEL_NB))
#define DISC_MODEL_NB (1)
#else // (defined(CFG_DISC_ALL) || defined(DISC_MODEL_NB))
#define DISC_MODEL_NB (0)
#endif // (defined(CFG_DISC_ALL) || defined(DISC_MODEL_NB))

// Support of Serial Number String characteristic
#if (defined(CFG_DISC_ALL) || defined(DISC_SERIAL_NB))
#define DISC_SERIAL_NB (1)
#else // (defined(CFG_DISC_ALL) || defined(DISC_SERIAL_NB))
#define DISC_SERIAL_NB (0)
#endif // (defined(CFG_DISC_ALL) || defined(DISC_SERIAL_NB))

// Support of Hardware Revision String characteristic
#if (defined(CFG_DISC_ALL) || defined(DISC_HW_REV))
#define DISC_HW_REV (1)
#else // (defined(CFG_DISC_ALL) || defined(DISC_HW_REV))
#define DISC_HW_REV (0)
#endif // (defined(CFG_DISC_ALL) || defined(DISC_HW_REV))

// Support of Firmware Revision String characteristic
#if (defined(CFG_DISC_ALL) || defined(DISC_FW_REV))
#define DISC_FW_REV (1)
#else // (defined(CFG_DISC_ALL) || defined(DISC_FW_REV))
#define DISC_FW_REV (0)
#endif // (defined(CFG_DISC_ALL) || defined(DISC_FW_REV))

// Support of Software Revision String characteristic
#if (defined(CFG_DISC_ALL) || defined(DISC_SW_REV))
#define DISC_SW_REV (1)
#else // (defined(CFG_DISC_ALL) || defined(DISC_SW_REV))
#define DISC_SW_REV (0)
#endif // (defined(CFG_DISC_ALL) || defined(DISC_SW_REV))

// Support of System ID characteristic
#if (defined(CFG_DISC_ALL) || defined(DISC_SYSTEM_ID))
#define DISC_SYSTEM_ID (1)
#else // (defined(CFG_DISC_ALL) || defined(DISC_SYSTEM_ID))
#define DISC_SYSTEM_ID (0)
#endif // (defined(CFG_DISC_ALL) || defined(DISC_SYSTEM_ID))

// Support of IEEE 11073-20601 Regulatory Certification Data List characteristic
#if (defined(CFG_DISC_ALL) || defined(DISC_IEEE))
#define DISC_IEEE (1)
#else // (defined(CFG_DISC_ALL) || defined(DISC_IEEE))
#define DISC_IEEE (0)
#endif // (defined(CFG_DISC_ALL) || defined(DISC_IEEE))

// Support of PnP ID characteristic
#if (defined(CFG_DISC_ALL) || defined(DISC_PNP_ID))
#define DISC_PNP_ID (1)
#else // (defined(DISC_PNP_ID) || defined(DISC_PNP_ID))
#define DISC_PNP_ID (0)
#endif // (defined(CFG_DISC_ALL) || defined(DISC_PNP_ID))

// Support of UDI for Medical Devices characteristic
#if (defined(CFG_DISC_ALL) || defined(DISC_UDI))
#define DISC_UDI (1)
#else // (defined(DISC_PNP_ID) || defined(DISC_UDI))
#define DISC_UDI (0)
#endif // (defined(CFG_DISC_ALL) || defined(DISC_UDI))

#endif // DISC_CFG_H_
