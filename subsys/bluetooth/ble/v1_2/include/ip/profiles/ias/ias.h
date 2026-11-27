/**
 ****************************************************************************************
 *
 * @file ias.h
 *
 * @brief Header file - Immediate Alert Service - API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef IAS_H_
#define IAS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup IAS_API Immediate Alert Service (IAS)
 * @ingroup PROFILE_API
 * @brief Description of API for Immediate Alert Service\n See \ref ias_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup IAS_API_COMMON Common
 * @ingroup IAS_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "prf_utils.h"

/// @addtogroup IAS_API_COMMON
/// @{

/*
 * DEFINES
 ****************************************************************************************
 */

/// Length of Alert Level characteristic value
#define IAS_ALERT_LEVEL_SIZE (1u)

/*
 * ENUMERATION
 ****************************************************************************************
 */

/// Alert level values
enum ias_alert_level
{
    /// No alert
    IAS_ALERT_LEVEL_NONE = 0u,
    /// Mild alert
    IAS_ALERT_LEVEL_MILD,
    /// High alert
    IAS_ALERT_LEVEL_HIGH,

    IAS_ALERT_LEVEL_MAX,
};

/// @} IAS_API_COMMON

#endif // IAS_H_
