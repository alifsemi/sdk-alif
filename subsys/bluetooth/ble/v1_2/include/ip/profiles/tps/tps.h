/**
 ****************************************************************************************
 *
 * @file tps.h
 *
 * @brief Header file - Tx Power Service - API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TPS_H_
#define TPS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TPS_API Tx Power Service (TPS)
 * @ingroup PROFILE_API
 * @brief Description of API for Tx Power Service\n See \ref tps_msc
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "prf_utils.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Length of Tx Power level value
#define TPS_LEVEL_SIZE (1u)

#endif // TPS_H_
