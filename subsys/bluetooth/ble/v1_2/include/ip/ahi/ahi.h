/**
 ****************************************************************************************
 *
 * @file ahi.h
 *
 * @brief This file contains definitions related to the Application Host Interface
 *
 * Copyright (C) RivieraWaves 2009-2022
 *
 *
 ****************************************************************************************
 */

#ifndef AHI_H_
#define AHI_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup AHI Application Host Interface
 * @ingroup AHI
 * @brief Application Host Interface, based on AHI functionality.
 *
 *@{
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"     // SW configuration

#if (AHI_TL_SUPPORT)
#include <stdint.h>

/*
 * DEFINES
 ****************************************************************************************
 */
/// Kernel message header length for transport through interface between App and SW stack.
#define AHI_MSG_HDR_LEN           (8u)
/// Maximum number of advertising reports that can be queued
#define AHI_MAX_ADV_REPORT       (10u)

#if (MAC154_EMB_PRESENT && !BLE_HOST_PRESENT)
/// MAC154App Reset size (Logical Channel + KE_HEADER + Reset_operation code + dummy)
#define AHI_RESET_MSG_LEN    (1u+AHI_MSG_HDR_LEN+2u+2u)
#else //(MAC154_EMB_PRESENT && !BLE_HOST_PRESENT)
/// GAPM Reset size (LC + KE_HEADER + Reset_operation code)
#define AHI_RESET_MSG_LEN    (1u+AHI_MSG_HDR_LEN+1u)
#endif //(MAC154_EMB_PRESENT && !BLE_HOST_PRESENT)

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern const uint8_t ahi_reset_msg[AHI_RESET_MSG_LEN];


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief AHI initialization function: initializes states and transport.
 *****************************************************************************************
 */
void ahi_init(void);

/**
 ****************************************************************************************
 * @brief Retrieve Task Identifier from Task number
 * (automatically update index of task in returned task id)
 *
 * @param task Task number
 * @return Task Identifier
 ****************************************************************************************
 */
uint16_t ahi_get_id_from_task(uint16_t task);

#endif //AHI_TL_SUPPORT

/// @} AHI
#endif // AHI_H_
