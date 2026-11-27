/**
 ****************************************************************************************
 *
 * @file tpsc.h
 *
 * @brief Header file - Tx Power Service Client - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TPSC_H_
#define TPSC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TPSC_API Tx Power Service Client
 * @ingroup TPS_API
 * @brief Description of API for Tx Power Service Client\n See \ref tps_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TPSC_API_COMMON Common
 * @ingroup TPSC_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TPSC_API_NATIVE Native API
 * @ingroup TPSC_API
 * @brief Description of Native API for Tx Power Service Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "tps.h"
#include "co_buf.h"

/// @addtogroup TPSC_API_COMMON
/// @{

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// Structure containing description of TPS discovered in peer device's database
typedef struct
{
    /// Tx Power Level characteristic value handle
    uint16_t val_hdl;
    /// Properties
    uint8_t prop;
} tpsc_content_t;

/// @} TPSC_API_COMMON

/// @addtogroup TPSC_API_NATIVE
/// @{

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback function for backward communication with upper layer
typedef struct
{
    /**
     ****************************************************************************************
     * @brief Command completed event for #tpsc_get_level
     *
     * @param[in] conidx            Connection index
     * @param[in] status            Status (see #hl_err enumeration)
     ****************************************************************************************
     */
    void (*cb_get_cmp_evt)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Command completed event for #tpsc_discover
     *
     * @param[in] conidx            Connection index
     * @param[in] status            Status (see #hl_err enumeration)
     * @param[in] p_content         Pointer to TPS content structure
     ****************************************************************************************
     */
    void (*cb_discover_cmp_evt)(uint8_t conidx, uint16_t status, const tpsc_content_t* p_content);

    /**
     ****************************************************************************************
     * @brief Inform about received Tx Power Level value
     *
     * @param[in] conidx            Connection index
     * @param[in] p_buf             Pointer to buffer
     ****************************************************************************************
     */
    void (*cb_level)(uint8_t conidx, co_buf_t* p_buf);
} tpsc_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Tx Power Service as Client
 *
 * @param[in] p_cbs             Pointer to set of callback functions for backward communication\n
 *                              Shall not be NULL. All callback functions shall be set.
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tpsc_add(const tpsc_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/**
 ****************************************************************************************
 * @brief Discover Tx Power Service in a peer device's database
 *
 * @param[in] conidx            Connection index
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tpsc_discover(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Get Tx Power Level characteristic value
 *
 * @param[in] conidx            Connection index
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t tpsc_get_level(uint8_t conidx);

/// @} TPSC_API_NATIVE

#endif // TPSC_H_
