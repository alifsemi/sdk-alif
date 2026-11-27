/**
 ****************************************************************************************
 *
 * @file llsc.h
 *
 * @brief Header file - Link Loss Service Client - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef LLSC_H_
#define LLSC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup LLSC_API Link Loss Service Client
 * @ingroup LLS_API
 * @brief Description of API for Link Loss Service Client\n See \ref lls_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup LLSC_API_COMMON Common
 * @ingroup LLSC_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup LLSC_API_NATIVE Native API
 * @ingroup LLSC_API
 * @brief Description of Native API for Link Loss Service Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "lls.h"
#include "llsc_cfg.h"
#include "co_buf.h"

/// @addtogroup LLSC_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Command code
enum llsc_cmd_code
{
    /// Discover
    LLSC_CMD_DISCOVER = 0u,
    /// Get level
    LLSC_CMD_GET_LEVEL,
    /// Set level
    LLSC_CMD_SET_LEVEL,
};

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// Structure containing description of LLS discovered in peer device's database
typedef struct
{
    /// Alert Level characteristic value handle
    uint16_t val_hdl;
    /// Properties
    uint8_t prop;
} llsc_content_t;

/// @} LLSC_API_COMMON

/// @addtogroup LLSC_API_NATIVE
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
     * @brief Command completed event
     *
     * @param[in] conidx            Connection index
     * @param[in] status            Status (see #hl_err enumeration)
     * @param[in] cmd_code          Command code (see #llsc_cmd_code enumeration)
     ****************************************************************************************
     */
    void (*cb_cmp_evt)(uint8_t conidx, uint16_t status, uint8_t cmd_code);

    /**
     ****************************************************************************************
     * @brief Inform about an update of bond data to be stored
     *
     * @param[in] conidx            Connection index
     * @param[in] p_bond_data       Pointer to bond data
     ****************************************************************************************
     */
    void (*cb_bond_data)(uint8_t conidx, const llsc_content_t* p_bond_data);

    #if (LLSC_READ_LEVEL)
    /**
     ****************************************************************************************
     * @brief Inform about received alert level value
     *
     * @param[in] conidx            Connection index
     * @param[in] p_buf             Pointer to buffer
     ****************************************************************************************
     */
    void (*cb_level)(uint8_t conidx, co_buf_t* p_buf);
    #endif // (LLSC_READ_LEVEL)
} llsc_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Link Loss Service as Client
 *
 * @param[in] p_cbs         Pointer to set of callback functions for communication with upper layer
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t llsc_add(const llsc_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/**
 ****************************************************************************************
 * @brief Discover Link Loss Service instances in a peer device's database
 *
 * @param[in] conidx            Connection index
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t llsc_discover(uint8_t conidx);

#if (HL_BONDABLE)
/**
 ****************************************************************************************
 * @brief Restore bond data
 *
 * @param[in] conidx            Connection index
 * @param[in] p_bond_data       Pointer to bond data
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t llsc_restore_bond_data(uint8_t conidx, const llsc_content_t* p_bond_data);
#endif // (HL_BONDABLE)

#if (LLSC_READ_LEVEL)
/**
 ****************************************************************************************
 * @brief Get Alert Level characteristic value
 *
 * @param[in] conidx            Connection index
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t llsc_get_level(uint8_t conidx);
#endif // (LLSC_READ_LEVEL)

/**
 ****************************************************************************************
 * @brief Set Alert Level characteristic value
 *
 * @param[in] conidx            Connection index
 * @param[in] p_buf             Pointer to buffer
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t llsc_set_level(uint8_t conidx, co_buf_t* p_buf);

/// @} LLSC_API_NATIVE

#endif // LLSC_H_
