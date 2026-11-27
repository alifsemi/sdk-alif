/**
 ****************************************************************************************
 *
 * @file htp.h
 *
 * @brief Header file - Health Thermometer Profile - API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef HTP_H_
#define HTP_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup HTP_API Health Thermometer Service (HTS)
 * @ingroup PROFILE_API
 * @brief Description of API for Health Thermometer Service
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HTP_API_COMMON Common
 * @ingroup HTP_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "prf_types.h"
#include "htp_common.h"

/// @addtogroup HTP_API_COMMON
/// @{

/*
 * DEFINES
 ****************************************************************************************
 */

/// Out of range error code
#define HTP_OUT_OF_RANGE_ERR_CODE        (0x80)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Temperature unit values
enum htp_unit
{
    /// Celsius
    HTP_UNIT_CELCIUS = 0,
    /// Fahrenheit
    HTP_UNIT_FAHRENHEIT,
};

/// @} HTP_API_COMMON

#endif // HTP_H_
