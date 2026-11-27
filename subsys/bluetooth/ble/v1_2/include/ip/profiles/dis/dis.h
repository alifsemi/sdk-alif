/**
 ****************************************************************************************
 *
 * @file dis.h
 *
 * @brief Header file - Device Information Service - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef DIS_H_
#define DIS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup DIS_API Device Information Service (DIS)
 * @ingroup PROFILE_API
 * @brief Description of Device Information Service API\n See \ref dis_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DIS_API_COMMON Common
 * @ingroup DIS_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "prf_utils.h"

/// @addtogroup DIS_API_COMMON
/// @{

/*
 * DEFINES
 ****************************************************************************************
 */

/// Length of PnP ID characteristic value
#define DIS_PNP_ID_VALUE_LENGTH (7u)

/// @} DIS_API_COMMON

#endif // DIS_H_
