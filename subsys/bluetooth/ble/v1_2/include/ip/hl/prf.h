/**
 ****************************************************************************************
 *
 * @file prf.h
 *
 * @brief Native API for profiles.
 *
 * Used to manage life cycle of profiles
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef PRF_H_
#define PRF_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup PROFILE_API Profiles
 * @brief Bluetooth Host Profiles
 *
 * The PROFILE of the stack contains the profile layers.
 * #prf_add_profile is default function used to instantiate a profile.
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup PROFILE_FUNC_API Loading
 * @ingroup PROFILE_API
 * @brief Function used to load profile before using it.
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gap.h"

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @addtogroup PROFILE_FUNC_API
/// @{

/**
 ****************************************************************************************
 * @brief This function is used to instantiate a profile (service or client) into SW.
 *        This function allocates profile attribute database and required environment variables.
 *
 *        Profile must be instantiated after execution of #gapm_configure.
 *        Parameter field should be set according to profile settings which are described
 *        in corresponding profile interface specifications.
 *
 * @note  Allocated profiles are freed if a #gapm_reset or #gapm_configure functions are executed.
 *
 * @note This function will be deprecated soon, if available, use the XXX_profile_load function where XXX represents
 * the profile name.
 *
 * @param[in]     api_id       Profile API identifier (see TASK_API_ID enumeration)
 * @param[in]     sec_lvl      Security level (see #gatt_svc_info_bf enumeration)
 * @param[in]     user_prio    GATT User priority
 * @param[in]     p_params     Configuration parameters of profile collector or service (32 bits aligned)
 * @param[in]     p_cb         Callback structure that handles event from profile.
 * @param[in,out] p_start_hdl  Pointer to Service start handle (0 - dynamically allocated), only applies for services.
 *
 * @return status of adding profile task
 ****************************************************************************************
 */
uint16_t prf_add_profile(uint8_t api_id, uint8_t sec_lvl, uint8_t user_prio, const void* p_params, const void* p_cb,
                         uint16_t* p_start_hdl);

/// @} PROFILE_FUNC_API

#endif /* PRF_H_ */
