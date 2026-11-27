/**
 ****************************************************************************************
 *
 * @file disc.h
 *
 * @brief Header file - Device Information Service Client - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef DISC_H_
#define DISC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup DISC_API Device Information Service Client
 * @ingroup DIS_API
 * @brief Description of API for Device Information Service Client\n See \ref dis_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DISC_API_COMMON Common
 * @ingroup DISC_API
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DISC_API_NATIVE Native API
 * @ingroup DISC_API
 * @brief Description of Native API for Device Information Service Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "dis.h"
#include "disc_cfg.h"

/// @addtogroup DISC_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Characteristic type
enum disc_char_type
{
    #if (HOST_MSG_API || DISC_MANUFACTURER)
    /// Manufacturer Name String characteristic
    DISC_CHAR_TYPE_MANUFACTURER_NAME = 0u,
    #endif // (HOST_MSG_API || DISC_MANUFACTURER)
    #if (HOST_MSG_API || DISC_MODEL_NB)
    /// Model Number String characteristic
    DISC_CHAR_TYPE_MODEL_NUMBER,
    #endif // (HOST_MSG_API || DISC_MODEL_NB)
    #if (HOST_MSG_API || DISC_SERIAL_NB)
    /// Serial Number String characteristic
    DISC_CHAR_TYPE_SERIAL_NUMBER,
    #endif // (HOST_MSG_API || DISC_SERIAL_NB)
    #if (HOST_MSG_API || DISC_HW_REV)
    /// Hardware Revision String characteristic
    DISC_CHAR_TYPE_HW_REVISION,
    #endif // (HOST_MSG_API || DISC_HW_REV)
    #if (HOST_MSG_API || DISC_FW_REV)
    /// Firmware Revision String characteristic
    DISC_CHAR_TYPE_FW_REVISION,
    #endif // (HOST_MSG_API || DISC_FW_REV)
    #if (HOST_MSG_API || DISC_SW_REV)
    /// Software Revision String characteristic
    DISC_CHAR_TYPE_SW_REVISION,
    #endif // (HOST_MSG_API || DISC_SW_REV)
    #if (HOST_MSG_API || DISC_SYSTEM_ID)
    /// System ID characteristic
    DISC_CHAR_TYPE_SYSTEM_ID,
    #endif // (HOST_MSG_API || DISC_SYSTEM_ID)
    #if (HOST_MSG_API || DISC_IEEE)
    /// IEEE 11073-20601 Regulatory Certification Data List characteristic
    DISC_CHAR_TYPE_IEEE,
    #endif // (HOST_MSG_API || DISC_IEEE)
    #if (HOST_MSG_API || DISC_PNP_ID)
    /// PnP ID characteristic
    DISC_CHAR_TYPE_PNP_ID,
    #endif // (HOST_MSG_API || DISC_PNP_ID)
    #if (HOST_MSG_API || DISC_UDI)
    /// UDI for Medical Devices characteristic
    DISC_CHAR_TYPE_UDI,
    #endif // (HOST_MSG_API || DISC_UDI)

    DISC_CHAR_TYPE_MAX,
};

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// Structure containing description of BAS discovered in peer device's database
typedef struct
{
    /// Characteristic information
    uint16_t val_hdl[DISC_CHAR_TYPE_MAX];
} disc_content_t;

/// @} DISC_API_COMMON

/// @addtogroup DISC_API_NATIVE
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
     * @brief Command completed event for #disc_get
     *
     * @param[in] conidx            Connection index
     * @param[in] status            Status (see #hl_err enumeration)
     * @param[in] char_type         Characteristic type (see #disc_char_type enumeration)
     ****************************************************************************************
     */
    void (*cb_get_cmp_evt)(uint8_t conidx, uint16_t status, uint8_t char_type);

    /**
     ****************************************************************************************
     * @brief Command completed event for #disc_discover
     *
     * @param[in] conidx            Connection index
     * @param[in] status            Status (see #hl_err enumeration)
     * @param[in] p_content         Pointer to DIS content structure
     ****************************************************************************************
     */
    void (*cb_discover_cmp_evt)(uint8_t conidx, uint16_t status, const disc_content_t* p_content);

    /**
     ****************************************************************************************
     * @brief Inform about received characteristic value
     *
     * @param[in] conidx            Connection index
     * @param[in] char_type         Characteristic type (see #disc_char_type enumeration)
     * @param[in] p_buf             Pointer to buffer containing received value
     ****************************************************************************************
     */
    void (*cb_value)(uint8_t conidx, uint8_t char_type, co_buf_t* p_buf);
} disc_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Device Information Service as Client
 *
 * @param[in] p_cbs         Pointer to set of callback functions for communication with upper layer
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t disc_add(const disc_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/**
 ****************************************************************************************
 * @brief Discover Device Information Service instances in a peer device's database
 *
 * @param[in] conidx            Connection index
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t disc_discover(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Get value of a Device Information Service characteristic
 *
 * @param[in] conidx            Connection index
 * @param[in] char_type         Characteristic type (see #disc_char_type enumeration)
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t disc_get(uint8_t conidx, uint8_t char_type);

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @return Pointer to content structure
 *
 * @param[in] conidx            Connection index
 ****************************************************************************************
 */
const disc_content_t* disc_get_content(uint8_t conidx);
#endif // (!HOST_MSG_API)

/// @} DISC_API_NATIVE

#endif // DISC_H_
