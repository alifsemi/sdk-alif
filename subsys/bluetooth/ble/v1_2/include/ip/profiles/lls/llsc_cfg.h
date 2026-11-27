/**
 ****************************************************************************************
 *
 * @file llsc_cfg.h
 *
 * @brief Header file - Link Loss Service Client - Configuration
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef LLSC_CFG_H_
#define LLSC_CFG_H_

#include "rom_build_cfg.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#if (defined(CFG_LLSC_ALL) || defined(CFG_LLSC_READ_LEVEL))
// Support of Read operation for Alert Level characteristic
#define LLSC_READ_LEVEL (1)
#else // (defined(CFG_LLSC_ALL) || defined(CFG_LLSC_READ_LEVEL))
#define LLSC_READ_LEVEL (0)
#endif // (defined(CFG_LLSC_ALL) || defined(CFG_LLSC_READ_LEVEL))

#endif // LLSC_CFG_H_
