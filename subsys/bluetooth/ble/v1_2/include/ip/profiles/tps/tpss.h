/**
 ****************************************************************************************
 *
 * @file tpss.h
 *
 * @brief Header file - Tx Power Service Server - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TPSS_H_
#define TPSS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TPSS_API Tx Power Service Server
 * @ingroup TPS_API
 * @brief Description of API for Tx Power Service Server\n See \ref tps_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TPSS_API_NATIVE Native API
 * @ingroup TPSS_API
 * @brief Description of Native API for Tx Power Service Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tps.h"
#include "co_buf.h"

/// @addtogroup TPSS_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for Tx Power Service (Server)
typedef struct
{
    /**
     ****************************************************************************************
     * @brief Request Tx Power Level value\n
     *        #tpss_level_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] token             Token
     ****************************************************************************************
     */
    void (*cb_level_req)(uint8_t conidx, uint16_t token);
} tpss_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Tx Power Service as Server
 *
 * @param[in] p_cbs             Pointer to set of callback functions for backward communication\n
 *                              Shall not be NULL. All callback functions shall be set.
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tpss_add(const tpss_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/**
 ****************************************************************************************
 * @brief Provide Tx Power Level value requested by peer device
 *
 * @param[in] conidx            Connection index
 * @param[in] token             Token
 * @param[in] p_buf             Pointer to buffer
 ****************************************************************************************
 */
void tpss_level_cfm(uint8_t conidx, uint16_t token, co_buf_t* p_buf);

/// @} TPSS_API_NATIVE

#endif // TPSS_H_
