/**
 ****************************************************************************************
 *
 * @file iasc.h
 *
 * @brief Header file - Immediate Alert Service Client - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef IASC_H_
#define IASC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup IASC_API Immediate Alert Service Client
 * @ingroup IAS_API
 * @brief Description of API for Immediate Alert Service Client\n See \ref ias_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup IASC_API_COMMON Common
 * @ingroup IASC_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup IASC_API_NATIVE Native API
 * @ingroup IASC_API
 * @brief Description of Native API for Immediate Alert Service Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "ias.h"
#include "co_buf.h"

/// @addtogroup IASC_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Command code
enum iasc_cmd_code
{
    /// Discover
    IASC_CMD_DISCOVER = 0u,
    /// Set level
    IASC_CMD_SET_LEVEL,
};

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// Structure containing description of IAS discovered in peer device's database
typedef struct
{
    /// Alert Level characteristic value handle
    uint16_t val_hdl;
    /// Properties
    uint8_t prop;
} iasc_content_t;

/// @} IASC_API_COMMON

/// @addtogroup IASC_API_NATIVE
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
     * @param[in] cmd_code          Command code (see #iasc_cmd_code enumeration)
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
    void (*cb_bond_data)(uint8_t conidx, const iasc_content_t* p_bond_data);
} iasc_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Immediate Alert Service as Client
 *
 * @param[in] p_cbs             Pointer to set of callback functions for backward communication\n
 *                              Shall not be NULL. All callback functions shall be set.
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t iasc_add(const iasc_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/**
 ****************************************************************************************
 * @brief Discover Immediate Alert Service instances in a peer device's database
 *
 * @param[in] conidx            Connection index
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t iasc_discover(uint8_t conidx);

#if (HL_BONDABLE)
/**
 ****************************************************************************************
 * @brief Restore bond data
 *
 * @param[in] conidx            Connection index
 * @param[in] p_bond_data       Pointer to bond data\n
 *                              Shall not be NULL
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t iasc_restore_bond_data(uint8_t conidx, const iasc_content_t* p_bond_data);
#endif // (HL_BONDABLE)

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
uint16_t iasc_set_level(uint8_t conidx, co_buf_t* p_buf);

/// @} IASC_API_NATIVE

#endif // IASC_H_
