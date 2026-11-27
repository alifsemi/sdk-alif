/**
 ****************************************************************************************
 *
 * @file cap.h
 *
 * @brief Common Audio Profile - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CAP_H_
#define CAP_H_


#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CAP Common Audio Profile (CAP)
 * @ingroup GAF
 * @brief Description of Common Audio Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_COMMON Common
 * @ingroup CAP
 * @brief Description of common API for Common Audio Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_ENUM Enumerations
 * @ingroup CAP_COMMON
 * @brief Enumerations for Common Audio Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_STRUCT Structures
 * @ingroup CAP_COMMON
 * @brief Structures for Common Audio Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CAP_NATIVE_API Native API
 * @ingroup CAP_COMMON
 * @brief Description of Native API for Common Audio Profile block
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"             // GAF Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup CAP_ENUM
/// @{

/// Module type values for Common Audio Profile block
enum cap_module_type
{
    /// Common Module
    CAP_MODULE_COMMON = 0,
    /// Common Audio Service Server Module
    CAP_MODULE_CAS = 1,
    /// Common Audio Service Client Module
    CAP_MODULE_CAC = 2,

    /// Maximum value
    CAP_MODULE_MAX,
};

/// Configuration bit field meaning (see #cap_configure function)
enum cap_cfg_bf
{
    /// Indicate if Server role is supported for Common Audio Service - Position
    CAP_CFG_CAS_SUPP_POS = 0,
    /// Indicate if Server role is supported for Common Audio Service - Bit
    CAP_CFG_CAS_SUPP_BIT = CO_BIT(CAP_CFG_CAS_SUPP_POS),

    /// Indicate if Client role is supported for Common Audio Service - Position
    CAP_CFG_CAC_SUPP_POS = 1,
    /// Indicate if Client role is supported for Common Audio Service - Bit
    CAP_CFG_CAC_SUPP_BIT = CO_BIT(CAP_CFG_CAC_SUPP_POS),
};

/// @} CAP_ENUM

/*
 * TYPE DEFINTIONS
 ****************************************************************************************
 */

/// @addtogroup CAP_STRUCT
/// @{

typedef struct cap_cac_cb cap_cac_cb_t;

/// Configuration Parameter structure for Common Audio Service Server
typedef struct cap_cas_cfg_param
{
    /// Local index of Coordinated Set Identification Service instance included in the Common Audio Service
    /// GAF_INVALID_LID if device in not part of a coordinated set
    uint8_t set_lid;
    /// Required start handle\n
    /// If set to GATT_INVALID_HANDLE, the start handle will be automatically chosen
    uint16_t shdl;
} cap_cas_cfg_param_t;

/// @} CAP_STRUCT

/*
 * API FUNCTION DEFINITION
 ****************************************************************************************
 */

#if (GAF_CAP)
/// @addtogroup CAP_NATIVE_API
/// @{

/**
 ****************************************************************************************
 * @brief Configure Common Audio Profile block
 *
 * @param[in] cfg_bf            Configuration bit field (see #cap_cfg_bf enumeration)
 * @param[in] p_cfg_param_cas   Pointer to Configuration Parameters for Common Audio Service Server\n
 *                              Cannot be NULL if support of Server Role for Common Audio Service is indicated as
 *                              supported in cfg_bf
 * @param[in] p_cb_cac          Pointer to set of callback functions for Common Audio Service Client\n
 *                              Cannot be NULL if support of Client Role for Common Audio Service is indicated as
 *                              supported in cfg_bf
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t cap_configure(uint8_t cfg_bf, const cap_cas_cfg_param_t* p_cfg_param_cas, const cap_cac_cb_t* p_cb_cac);

/// @} CAP_COMMON_NATIVE_API
#endif //(GAF_CAP)

#endif // CAP_H_

