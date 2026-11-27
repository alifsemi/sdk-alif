/**
 ****************************************************************************************
 *
 * @file lls.h
 *
 * @brief Header file - Link Loss Service - API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef LLS_H_
#define LLS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup LLS_API Link Loss Service (LLS)
 * @ingroup PROFILE_API
 * @brief Description of API for Link Loss Service\n See \ref lls_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup LLS_API_COMMON Common
 * @ingroup LLS_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "prf_utils.h"

/// @addtogroup LLS_API_COMMON
/// @{

/*
 * DEFINES
 ****************************************************************************************
 */

/// Length of Alert Level characteristic value
#define LLS_ALERT_LEVEL_SIZE (1u)

/*
 * ENUMERATION
 ****************************************************************************************
 */

/// Alert level values
enum lls_alert_level
{
    /// No alert
    LLS_ALERT_LEVEL_NONE = 0u,
    /// Mild alert
    LLS_ALERT_LEVEL_MILD,
    /// High alert
    LLS_ALERT_LEVEL_HIGH,

    LLS_ALERT_LEVEL_MAX
};

/// @} LLS_API_COMMON

#endif // LLS_H_
