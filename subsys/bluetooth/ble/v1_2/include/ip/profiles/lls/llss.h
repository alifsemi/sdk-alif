/**
 ****************************************************************************************
 *
 * @file llss.h
 *
 * @brief Header file - Link Loss Service Server - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef LLSS_H_
#define LLSS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup LLSS_API Link Loss Service Server
 * @ingroup LLS_API
 * @brief Description of API for Link Loss Service Server\n See \ref lls_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup LLSS_API_NATIVE Native API
 * @ingroup LLSS_API
 * @brief Description of Native API for Link Loss Service Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "lls.h"
#include "co_buf.h"

/// @addtogroup LLSS_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for Link Loss Service (Server)
typedef struct
{
    /**
     ****************************************************************************************
     * @brief Request Alert Level characteristic value\n
     *        #llss_get_level_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] token             Token
     ****************************************************************************************
     */
    void (*cb_get_level_req)(uint8_t conidx, uint16_t token);

    /**
     ****************************************************************************************
     * @brief Request feedback about set Alert Level characteristic value\n
     *        #llss_set_level_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] token             Token
     * @param[in] p_buf             Pointer to buffer
     ****************************************************************************************
     */
    void (*cb_set_level_req)(uint8_t conidx, uint16_t token, co_buf_t* p_buf);
} llss_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Link Loss Service as Server
 *
 * @param[in] p_cbs             Pointer to set of callback functions for backward communication\n
 *                              Shall not be NULL. All callback functions shall be set.
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t llss_add(const llss_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/**
 ****************************************************************************************
 * @brief Provide Alert Level value requested by peer device
 *
 * @param[in] conidx            Connection index
 * @param[in] token             Token
 * @param[in] p_buf             Pointer to buffer
 ****************************************************************************************
 */
void llss_get_level_cfm(uint8_t conidx, uint16_t token, co_buf_t* p_buf);

/**
 ****************************************************************************************
 * @brief Provide feedback about Alert Level value set by peer device
 *
 * @param[in] conidx            Connection index
 * @param[in] status            Status (see #hl_err enumeration)
 * @param[in] token             Token
 ****************************************************************************************
 */
void llss_set_level_cfm(uint8_t conidx, uint16_t status, uint16_t token);

/// @} LLSS_API_NATIVE

#endif // LLSS_H_
