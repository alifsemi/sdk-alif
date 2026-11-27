/**
 ****************************************************************************************
 *
 * @file tmap.h
 *
 * @brief Telephony and Media Audio Profile - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef TMAP_H_
#define TMAP_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TMAP Telephony and Media Audio Profile (TMAP)
 * @ingroup GAF
 * @brief Description of Telephony and Media Audio Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TMAP_COMMON Common
 * @ingroup TMAP
 * @brief Description of Telephony and Media Audio Profile Common module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TMAP_COMMON_ENUM Enumerations
 * @ingroup TMAP_COMMON
 * @brief Enumerations for Telephony and Media Audio Profile Common module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TMAP_COMMON_STRUCT Structures
 * @ingroup TMAP_COMMON
 * @brief Structures for Telephony and Media Audio Profile Common module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TMAP_COMMON_NATIVE_API Native API
 * @ingroup TMAP_COMMON
 * @brief Description of Native API for Telephony and Media Audio Profile Common module
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

/// @addtogroup TMAP_COMMON_ENUM
/// @{

/// Module type values for Telephony and Media Audio Profile block
enum tmap_module_type
{
    /// Common Module
    TMAP_MODULE_COMMON = 0,
    /// Telephony and Media Audio Service Server Module
    TMAP_MODULE_TMAS = 1,
    /// Telephony and Media Audio Service Client Module
    TMAP_MODULE_TMAC = 2,

    /// Maximum value
    TMAP_MODULE_MAX,
};

/// Configuration bit field meaning (see #tmap_configure function)
enum tmap_cfg_bf
{
    /// Indicate if Server role is supported for Telephony and Media Audio Service
    TMAP_CFG_TMAS_SUPP_POS = 0,
    TMAP_CFG_TMAS_SUPP_BIT = CO_BIT(TMAP_CFG_TMAS_SUPP_POS),

    /// Indicate if Client role is supported for Telephony and Media Audio Service
    TMAP_CFG_TMAC_SUPP_POS = 1,
    TMAP_CFG_TMAC_SUPP_BIT = CO_BIT(TMAP_CFG_TMAC_SUPP_POS),
};

/// Position of fields in Role characteristic value
enum tmap_role_pos
{
    /// Role field
    TMAP_ROLE_POS_ROLE = 0,

    /// Length of Role characteristic value
    TMAP_ROLE_LEN = 2,
};

/// Characteristic type values for Telephony and Media Audio Service
enum tmap_char_type
{
    /// TMAP Role characteristic
    TMAP_CHAR_TYPE_ROLE = 0,

    TMAP_CHAR_TYPE_MAX,
};

/// TMAP Role characteristic bit field meaning
enum tmap_role_bf
{
    /// Indicate if Server supports Call Gateway role (= 1) or not - Position
    TMAP_ROLE_CG_POS = 0,
    /// Indicate if Server supports Call Gateway role (= 1) or not - Bit
    TMAP_ROLE_CG_BIT = CO_BIT(TMAP_ROLE_CG_POS),

    /// Indicate if Server supports Call Terminal role (= 1) or not - Position
    TMAP_ROLE_CT_POS = 1,
    /// Indicate if Server supports Call Terminal role (= 1) or not - Bit
    TMAP_ROLE_CT_BIT = CO_BIT(TMAP_ROLE_CT_POS),

    /// Indicate if Server supports Unicast Media Sender role (= 1) or not - Position
    TMAP_ROLE_UMS_POS = 2,
    /// Indicate if Server supports Unicast Media Sender role (= 1) or not - Bit
    TMAP_ROLE_UMS_BIT = CO_BIT(TMAP_ROLE_UMS_POS),

    /// Indicate if Server supports Unicast Media Receiver role (= 1) or not - Position
    TMAP_ROLE_UMR_POS = 3,
    /// Indicate if Server supports Unicast Media Receiver role (= 1) or not - Bit
    TMAP_ROLE_UMR_BIT = CO_BIT(TMAP_ROLE_UMR_POS),

    /// Indicate if Server supports Broadcast Media Sender role (= 1) or not - Position
    TMAP_ROLE_BMS_POS = 4,
    /// Indicate if Server supports Broadcast Media Sender role (= 1) or not - Bit
    TMAP_ROLE_BMS_BIT = CO_BIT(TMAP_ROLE_BMS_POS),

    /// Indicate if Server supports Broadcast Media Receiver role (= 1) or not - Position
    TMAP_ROLE_BMR_POS = 5,
    /// Indicate if Server supports Broadcast Media Receiver role (= 1) or not - Bit
    TMAP_ROLE_BMR_BIT = CO_BIT(TMAP_ROLE_BMR_POS),

    /// Mask indicating that all roles are supported - LSB
    TMAP_ROLE_ALLSUPP_LSB = 0,
    /// Mask indicating that all roles are supported
    TMAP_ROLE_ALLSUPP_MASK = 0x003F,

    /// Mask indicating valid bits - LSB
    TMAP_ROLE_VALID_LSB = 0,
    /// Mask indicating valid bits
    TMAP_ROLE_VALID_MASK = 0x003F,

    /// Mask indicating RFU bits - LSB
    TMAP_ROLE_RFU_LSB = 6,
    /// Mask indicating RFU bits
    TMAP_ROLE_RFU_MASK = 0xFFC0,
};

/// @} TMAP_COMMON_ENUM

/*
 * TYPE DEFINTIONS
 ****************************************************************************************
 */

/// @addtogroup TMAP_COMMON_STRUCT
/// @{

typedef struct tmap_tmac_cb tmap_tmac_cb_t;

/// Configuration Parameter structure for Telephony and Media Audio Service Server
typedef struct tmap_tmas_cfg_param
{
    /// Bit field indicating supported TMAP roles (see #tmap_role_bf enumeration)
    uint16_t role_bf;
    /// Required start handle\n
    /// If set to GATT_INVALID_HANDLE, the start handle will be automatically chosen
    uint16_t shdl;
} tmap_tmas_cfg_param_t;

/// @} TMAP_COMMON_STRUCT

/*
 * EXTERNAL FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_TMAP)
/// @addtogroup TMAP_COMMON_NATIVE_API
/// @{

/**
 ****************************************************************************************
 * @brief Configure Telephony and Media Profile block
 *
 * @param[in] cfg_bf            Configuration bit field (see #tmap_cfg_bf enumeration)
 * @param[in] p_cfg_param_tmas  Pointer to Configuration Parameters for Telephony and Media Audio Service Server\n
 *                              Cannot be NULL if support of Server Role for Telephony and Media Audio Service is
 *                              indicated as supported in cfg_bf
 * @param[in] p_cb_tmac         Pointer to set of callback functions for Telephony and Media Audio Service Client\n
 *                              Cannot be NULL if support of Client Role for Telephony and Media Audio Service is
 *                              indicated as supported in cfg_bf
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t tmap_configure(uint8_t cfg_bf, const tmap_tmas_cfg_param_t* p_cfg_param_tmas,
                        const tmap_tmac_cb_t* p_cb_tmac);

/**
 ****************************************************************************************
 * @brief Check validity of supported roles bit field
 *
 * @param[in] role_bf           Roles bit field (see #tmap_role_bf enumeration)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t tmap_check_role(uint16_t role_bf);

/// @} TMAP_COMMON_NATIVE_API
#endif //(GAF_TMAP)


#endif // TMAP_H_

