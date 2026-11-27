/**
 ****************************************************************************************
 *
 * @file iass.h
 *
 * @brief Header file - Immediate Alert Service Server - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef IASS_H_
#define IASS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup IASS_API Immediate Alert Service Server
 * @ingroup IAS_API
 * @brief Description of API for Immediate Alert Service Server\n See \ref ias_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup IASS_API_NATIVE Native API
 * @ingroup IASS_API
 * @brief Description of Native API for Immediate Alert Service Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "ias.h"
#include "co_buf.h"

/// @addtogroup IASS_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for Immediate Alert Service (Server)
typedef struct
{
    /**
     ****************************************************************************************
     * @brief Inform about value written by peer device for Alert Level characteristic
     *
     * @param[in] conidx            Connection index
     * @param[in] p_buf             Pointer to buffer
     ****************************************************************************************
     */
    void (*cb_level)(uint8_t conidx, co_buf_t* p_buf);
} iass_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Immediate Alert Service as Server
 *
 * @param[in] p_cbs             Pointer to set of callback functions for backward communication\n
 *                              Shall not be NULL. All callback functions shall be set.
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t iass_add(const iass_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/// @} IASS_API_NATIVE

#endif // IASS_H_
