/**
 ****************************************************************************************
 *
 * @file hap.h
 *
 * @brief Hearing Access Profile - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef HAP_H_
#define HAP_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup HAP Hearing Access Profile (HAP)
 * @ingroup GAF
 * @brief Description of Hearing Access Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_COMMON Common
 * @ingroup HAP
 * @brief Description of Hearing Access Profile Common module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_COMMON_DEFINE Definitions
 * @ingroup HAP_COMMON
 * @brief Definitions for Hearing Access Profile Common module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_COMMON_ENUM Enumerations
 * @ingroup HAP_COMMON
 * @brief Enumerations for Hearing Access Profile Common module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_COMMON_STRUCT Structures
 * @ingroup HAP_COMMON
 * @brief Structures for Hearing Access Profile Common module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup HAP_COMMON_NATIVE_API Native API
 * @ingroup HAP_COMMON
 * @brief Description of Native API for Hearing Access Profile Common module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf.h"             // GAF Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup HAP_COMMON_DEFINE
/// @{

/// Minimal length of Preset name value
#define HAP_HAS_PRESET_NAME_LEN_MIN         (1)
/// Maximal length of Preset name value
#define HAP_HAS_PRESET_NAME_LEN_MAX         (40)
/// Minimal Preset Index value
#define HAP_HAS_PRESET_IDX_MIN              (1)

/// @} HAP_COMMON_DEFINE

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup HAP_COMMON_ENUM
/// @{

/// Characteristic type values for Hearing Access Service
enum hap_has_char_type
{
    /// Preset Control Point characteristic
    HAP_HAS_CHAR_TYPE_PRESET_CP = 0,

    /// Maximum value for characteristics supporting sending of indications
    HAP_HAS_CHAR_TYPE_IND_MAX,

    /// Active Preset Index characteristic
    HAP_HAS_CHAR_TYPE_ACTIVE_PRESET_INDEX = HAP_HAS_CHAR_TYPE_IND_MAX,
    /// Hearing Aid Features characteristic
    HAP_HAS_CHAR_TYPE_FEATURES,

    /// Maximum value for characteristics supporting sending of notifications
    HAP_HAS_CHAR_TYPE_NTF_MAX,
    /// Maximum value
    HAP_HAS_CHAR_TYPE_MAX = HAP_HAS_CHAR_TYPE_NTF_MAX,
};

/// Error code values for Preset Control Point characteristic
enum hap_has_cp_error
{
    /// Invalid Opcode
    HAP_HAS_CP_ERROR_INVALID_OPCODE = 0x80,
    /// Write Name Not Allowed
    HAP_HAS_CP_ERROR_WRITE_NAME_NOT_ALLOWED,
    /// Preset Synchronization Not Supported
    HAP_HAS_CP_ERROR_PRESET_SYNC_NOT_SUPPORTED,
    /// Preset Operation Not Possible
    HAP_HAS_CP_ERROR_PRESET_OP_NOT_POSSIBLE,
    /// Invalid Parameters Length
    HAP_HAS_CP_ERROR_INVALID_LENGTH,
};

/// Hearing Aid Type values
enum hap_has_features_type
{
    /// Binaural Hearing Aid
    HAP_HAS_FEATURES_TYPE_BINAURAL = 0,
    /// Monaural Hearing Aid
    HAP_HAS_FEATURES_TYPE_MONAURAL,
    /// Banded Hearing Aid
    HAP_HAS_FEATURES_TYPE_BANDED,
};

/// Hearing Aid Features characteristic value bit field meaning
enum hap_has_features_bf
{
    /// Indicate the Hearing Aid type of the server (see #hap_has_features_type enumeration) - Position
    HAP_HAS_FEATURES_TYPE_LSB = 0,
    /// Indicate the Hearing Aid type of the server (see #hap_has_features_type enumeration) - Mask
    HAP_HAS_FEATURES_TYPE_MASK = 0x03,
    /// Indicate if Preset Synchronization is supported (= 1) or not - Position
    HAP_HAS_FEATURES_PRESET_SYNC_SUPPORT_POS = 2,
    /// Indicate if Preset Synchronization is supported (= 1) or not - Bit
    HAP_HAS_FEATURES_PRESET_SYNC_SUPPORT_BIT = CO_BIT(HAP_HAS_FEATURES_PRESET_SYNC_SUPPORT_POS),
    /// Indicate if list of preset records on this server may be different (= 1)
    /// from the list of preset records in the other server of the coordinated set or not - Position
    HAP_HAS_FEATURES_INDEPENDENT_PRESETS_POS = 3,
    /// Indicate if list of preset records on this server may be different (= 1)
    /// from the list of preset records in the other server of the coordinated set or not - Bit
    HAP_HAS_FEATURES_INDEPENDENT_PRESETS_BIT = CO_BIT(HAP_HAS_FEATURES_INDEPENDENT_PRESETS_POS),
    /// Indicate if the list of preset records may change (= 1) or not - Position
    HAP_HAS_FEATURES_DYNAMIC_PRESETS_POS = 4,
    /// Indicate if the list of preset records may change (= 1) or not - Bit
    HAP_HAS_FEATURES_DYNAMIC_PRESETS_BIT = CO_BIT(HAP_HAS_FEATURES_DYNAMIC_PRESETS_POS),
    /// Indicate if the writable preset records are supported (= 1) or not - Position
    HAP_HAS_FEATURES_WRITABLE_PRESETS_POS = 5,
    /// Indicate if the writable preset records are supported (= 1) or not - Bit
    HAP_HAS_FEATURES_WRITABLE_PRESETS_BIT = CO_BIT(HAP_HAS_FEATURES_WRITABLE_PRESETS_POS),

    /// Mask providing RFU bits
    HAP_HAS_FEATURES_RFU_MASK = 0xC0,
};

/// Module type values for Hearing Access Profile block
enum hap_module_type
{
    /// Common Module
    HAP_MODULE_COMMON = 0,
    /// Hearing Access Service Server Module
    HAP_MODULE_HAS,
    /// Hearing Access Service Client Module
    HAP_MODULE_HAC,

    /// Maximum value
    HAP_MODULE_MAX,
};

/// Configuration bit field meaning (see #hap_configure function)
enum hap_cfg_bf
{
    /// Indicate if Server role is supported for Hearing Access Service - Position
    HAP_CFG_HAS_SUPP_POS = 0,
    /// Indicate if Server role is supported for Hearing Access Service - Bit
    HAP_CFG_HAS_SUPP_BIT = CO_BIT(HAP_CFG_HAS_SUPP_POS),

    /// Indicate if Client role is supported for Hearing Access Service - Position
    HAP_CFG_HAC_SUPP_POS = 1,
    /// Indicate if Client role is supported for Hearing Access Service - Bit
    HAP_CFG_HAC_SUPP_BIT = CO_BIT(HAP_CFG_HAC_SUPP_POS),
};

/// Change ID values for Preset Changed operation
enum hap_has_change_id
{
    /// Generic Update
    HAP_HAS_CHANGE_ID_GENERIC_UPDATE = 0,
    /// Preset Record Deleted
    HAP_HAS_CHANGE_ID_PRESET_RECORD_DELETED,
    /// Preset Record Available
    HAP_HAS_CHANGE_ID_PRESET_RECORD_AVAILABLE,
    /// Preset Record Unavailable
    HAP_HAS_CHANGE_ID_PRESET_RECORD_UNAVAILABLE,

    /// Maximum value
    HAP_HAS_CHANGE_ID_MAX,
};

/// @} HAP_COMMON_ENUM

/*
 * TYPE DEFINTIONS
 ****************************************************************************************
 */

/// @addtogroup HAP_COMMON_STRUCT
/// @{

typedef struct hap_has_cb hap_has_cb_t;
typedef struct hap_hac_cb hap_hac_cb_t;
typedef struct hap_has_cfg_param hap_has_cfg_param_t;
typedef struct hap_hac_cfg_param hap_hac_cfg_param_t;

/// @} HAP_COMMON_STRUCT

/*
 * API FUNCTION DEFINITION
 ****************************************************************************************
 */

#if (GAF_HAP)
/// @addtogroup HAP_COMMON_NATIVE_API
/// @{

/**
 ****************************************************************************************
 * @brief Configure Hearing Access Profile block
 *
 * @param[in] cfg_bf            Configuration bit field (see #hap_cfg_bf enumeration)
 * @param[in] p_cfg_param_has   Pointer to Configuration structure for Hearing Access Service Server\n
 *                              Cannot be NULL if support of Server Role for Hearing Access Service is indicated in
 *                              cfg_bf
 * @param[in] p_cb_has          Pointer to set to callback functions for communication with upper layer (HAS)
 * @param[in] p_cfg_param_hac   Pointer to Configuration structure for Hearing Access Service Client\n
 *                              Cannot be NULL if support of Client Role for Hearing Access Service is indicated in
 *                              cfg_bf
 * @param[in] p_cb_hac          Pointer to set to callback functions for communication with upper layer (HAC)
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t hap_configure(uint8_t cfg_bf, const hap_has_cfg_param_t* p_cfg_param_has, const hap_has_cb_t* p_cb_has,
                       const hap_hac_cfg_param_t* p_cfg_param_hac, const hap_hac_cb_t* p_cb_hac);

/// @} HAP_COMMON_NATIVE_API
#endif //(GAF_HAP)

#endif // HAP_H_

