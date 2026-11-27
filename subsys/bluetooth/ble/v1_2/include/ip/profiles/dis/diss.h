/**
 ****************************************************************************************
 *
 * @file diss.h
 *
 * @brief Header file - Device Information Service Server - Native API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef DISS_H_
#define DISS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup DISS_API Device Information Service Server
 * @ingroup DIS_API
 * @brief Description of API for Device Information Service Server\n See \ref dis_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DISS_API_NATIVE Native API
 * @ingroup DISS_API
 * @brief Description of Native API for Device Information Service Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "dis.h"
#include "diss_cfg.h"

/// @addtogroup DISS_API_NATIVE
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Characteristic type
enum diss_char_type
{
    #if (HOST_MSG_API || DISS_MANUFACTURER)
    /// Manufacturer Name String characteristic
    DISS_CHAR_TYPE_MANUFACTURER_NAME = 0u,
    #endif // (HOST_MSG_API || DISS_MANUFACTURER)
    #if (HOST_MSG_API || DISS_MODEL_NB)
    /// Model Number String characteristic
    DISS_CHAR_TYPE_MODEL_NUMBER,
    #endif // (HOST_MSG_API || DISS_MODEL_NB)
    #if (HOST_MSG_API || DISS_SERIAL_NB)
    /// Serial Number String characteristic
    DISS_CHAR_TYPE_SERIAL_NUMBER,
    #endif // (HOST_MSG_API || DISS_SERIAL_NB)
    #if (HOST_MSG_API || DISS_HW_REV)
    /// Hardware Revision String characteristic
    DISS_CHAR_TYPE_HW_REVISION,
    #endif // (HOST_MSG_API || DISS_HW_REV)
    #if (HOST_MSG_API || DISS_FW_REV)
    /// Firmware Revision String characteristic
    DISS_CHAR_TYPE_FW_REVISION,
    #endif // (HOST_MSG_API || DISS_FW_REV)
    #if (HOST_MSG_API || DISS_SW_REV)
    /// Software Revision String characteristic
    DISS_CHAR_TYPE_SW_REVISION,
    #endif // (HOST_MSG_API || DISS_SW_REV)
    #if (HOST_MSG_API || DISS_SYSTEM_ID)
    /// System ID characteristic
    DISS_CHAR_TYPE_SYSTEM_ID,
    #endif // (HOST_MSG_API || DISS_SYSTEM_ID)
    #if (HOST_MSG_API || DISS_IEEE)
    /// IEEE 11073-20601 Regulatory Certification Data List characteristic
    DISS_CHAR_TYPE_IEEE,
    #endif // (HOST_MSG_API || DISS_IEEE)
    #if (HOST_MSG_API || DISS_PNP_ID)
    /// PnP ID characteristic
    DISS_CHAR_TYPE_PNP_ID,
    #endif // (HOST_MSG_API || DISS_PNP_ID)
    #if (HOST_MSG_API || DISS_UDI)
    /// UDI for Medical Devices characteristic
    DISS_CHAR_TYPE_UDI,
    #endif // (HOST_MSG_API || DISS_UDI)

    DISS_CHAR_TYPE_MAX,
};

/*
 * NATIVE API CALLBACKS
 ****************************************************************************************
 */

/// Set of callback functions for Device Information Service (Server)
typedef struct
{
    /**
     ****************************************************************************************
     * @brief Request value after read of one of Device Information Service's characteristics/n
     *        #diss_value_cfm function shall be called
     *
     * @param[in] conidx            Connection index
     * @param[in] char_type         Characteristic type (see #diss_char_type enumeration)
     * @param[in] token             Token
     ****************************************************************************************
     */
    void (*cb_value_req)(uint8_t conidx, uint8_t char_type, uint16_t token);
} diss_cbs_t;

/*
 * NATIVE API FUNCTIONS
 ****************************************************************************************
 */

#if (!HOST_MSG_API)
/**
 ****************************************************************************************
 * @brief Add support of Device Information Service as Server
 *
 * @param[in] char_type_bf  Bit field (based on #diss_char_type enumeration) indicating characteristics that are
 *                          supported\n
 *                          Meaningful only if compiled with DISS_FLEXIBLE option
 * @param[in] p_cbs         Pointer to set of callback functions for communication with upper layer
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
uint16_t diss_add(uint16_t char_type_bf, const diss_cbs_t* p_cbs);
#endif // (!HOST_MSG_API)

/**
 ****************************************************************************************
 * @brief Provide value requested by peer device
 *
 * @param[in] conidx            Connection index
 * @param[in] token             Token
 * @param[in] p_buf             Pointer to buffer containing value
 ****************************************************************************************
 */
void diss_value_cfm(uint8_t conidx, uint16_t token, co_buf_t* p_buf);

/// @} DISS_API_NATIVE

#endif // DISS_H_
