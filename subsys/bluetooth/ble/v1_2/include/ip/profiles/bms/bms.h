/**
 ****************************************************************************************
 *
 * @file bms.h
 *
 * @brief Header file - Bond Management Service - API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BMS_H_
#define BMS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BMS_API Bond Management Service (BMS)
 * @ingroup PROFILE_API
 * @brief Description of API for Bond Management Service
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BMS_API_COMMON Common
 * @ingroup BMS_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "prf_types.h"
#include "hl_error.h"
#include "co_math.h"

/// @addtogroup BMS_API_COMMON
/// @{

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximal length of Authorization Code
#define BMS_AUTH_CODE_LENGTH_MAX               (511)
/// Maximal length of Bond Management Feature value
/// Note: Client shall be robust to reception of more bytes
#define BMS_FEATURES_LENGTH_MAX                (3)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Error code values
enum bms_error
{
    /// No error
    BMS_ERR_NO_ERROR = 0,
    /// Insufficient authorization
    BMS_ERR_OPCODE_INSUFFICIENT_AUTH = ATT_ERR_INSUFF_AUTHOR,
    /// Operation code not supported
    BMS_ERR_OPCODE_NOT_SUPPORTED = 0x80,
    /// Operation failed
    BMS_ERR_OPERATION_FAILED,
};

/// Operation code values
enum bms_opcode
{
    /// Minimum operation code value
    BMS_OPCODE_MIN = 1,

    /// Delete bond of requesting device (BR/EDR and LE)
    BMS_OPCODE_DELETE_BTDM = BMS_OPCODE_MIN,
    /// Delete bond of requesting device (BR/EDR transport only)
    BMS_OPCODE_DELETE_BT,
    /// Delete bond of requesting device (LE transport only)
    BMS_OPCODE_DELETE_LE,
    /// Delete all bonds on server (BR/EDR and LE)
    BMS_OPCODE_DELETE_ALL_BTDM,
    /// Delete all bonds on server (BR/EDR transport only)
    BMS_OPCODE_DELETE_ALL_BT,
    /// Delete all bonds on server (LE transport only)
    BMS_OPCODE_DELETE_ALL_LE,
    /// Delete all but the active bond on server (BR/EDR and LE)
    BMS_OPCODE_DELETE_ALL_BUT_BTDM,
    /// Delete all but the active bond on server (BR/EDR transport only)
    BMS_OPCODE_DELETE_ALL_BUT_BT,
    /// Delete all but the active bond on server (LE transport only)
    BMS_OPCODE_DELETE_ALL_BUT_LE,

    /// Maximum operation code value
    BMS_OPCODE_MAX,
};

/// Bond Management Control Point characteristic mapping value
enum bms_cp_value_mapping
{
    /// Operation code
    BMS_CP_VALUE_OPCODE_POS = 0,

    /// Minimum length to write for Bond Management Control Point characteristic
    BMS_CP_VALUE_LENGTH_MIN,

    /// Authorization code
    BMS_CP_VALUE_AUTH_CODE_POS = BMS_CP_VALUE_LENGTH_MIN,
};

/// Meaning for bytes in Bond Management Feature value
enum bms_features_bf
{
    /// Byte 0 - Bit 0: Delete bond of requesting device (BR/EDR and LE) - Position
    BMS_FEATURES_BYTE0_DELETE_BTDM_POS = 0,
    /// Byte 0 - Bit 0: Delete bond of requesting device (BR/EDR and LE) - Bit
    BMS_FEATURES_BYTE0_DELETE_BTDM_BIT = CO_BIT(BMS_FEATURES_BYTE0_DELETE_BTDM_POS),
    /// Byte 0 - Bit 1: Delete bond of requesting device (BR/EDR and LE) with authorization code - Position
    BMS_FEATURES_BYTE0_DELETE_BTDM_AUTH_CODE_POS = 1,
    /// Byte 0 - Bit 1: Delete bond of requesting device (BR/EDR and LE) with authorization code - Bit
    BMS_FEATURES_BYTE0_DELETE_BTDM_AUTH_CODE_BIT = CO_BIT(BMS_FEATURES_BYTE0_DELETE_BTDM_AUTH_CODE_POS),
    /// Byte 0 - Bit 2: Delete bond of requesting device (BR/EDR transport only) - Position
    BMS_FEATURES_BYTE0_DELETE_BT_POS = 2,
    /// Byte 0 - Bit 2: Delete bond of requesting device (BR/EDR transport only) - Bit
    BMS_FEATURES_BYTE0_DELETE_BT_BIT = CO_BIT(BMS_FEATURES_BYTE0_DELETE_BT_POS),
    /// Byte 0 - Bit 3: Delete bond of requesting device (BR/EDR transport only) with authorization code - Position
    BMS_FEATURES_BYTE0_DELETE_BT_AUTH_CODE_POS = 3,
    /// Byte 0 - Bit 3: Delete bond of requesting device (BR/EDR transport only) with authorization code - Bit
    BMS_FEATURES_BYTE0_DELETE_BT_AUTH_CODE_BIT = CO_BIT(BMS_FEATURES_BYTE0_DELETE_BT_AUTH_CODE_POS),
    /// Byte 0 - Bit 4: Delete bond of requesting device (LE transport only) - Position
    BMS_FEATURES_BYTE0_DELETE_LE_POS = 4,
    /// Byte 0 - Bit 4: Delete bond of requesting device (LE transport only) - Bit
    BMS_FEATURES_BYTE0_DELETE_LE_BIT = CO_BIT(BMS_FEATURES_BYTE0_DELETE_LE_POS),
    /// Byte 0 - Bit 5: Delete bond of requesting device (LE transport only) with authorization code- Position
    BMS_FEATURES_BYTE0_DELETE_LE_AUTH_CODE_POS = 5,
    /// Byte 0 - Bit 5: Delete bond of requesting device (LE transport only) with authorization code - Bit
    BMS_FEATURES_BYTE0_DELETE_LE_AUTH_CODE_BIT = CO_BIT(BMS_FEATURES_BYTE0_DELETE_LE_AUTH_CODE_POS),
    /// Byte 0 - Bit 6: Delete all bonds on server (BR/EDR and LE) - Position
    BMS_FEATURES_BYTE0_DELETE_ALL_BTDM_POS = 6,
    /// Byte 0 - Bit 6: Delete all bonds on server (BR/EDR and LE) - Bit
    BMS_FEATURES_BYTE0_DELETE_ALL_BTDM_BIT = CO_BIT(BMS_FEATURES_BYTE0_DELETE_ALL_BTDM_POS),
    /// Byte 0 - Bit 7: Delete all bonds on server (BR/EDR and LE) with authorization code - Position
    BMS_FEATURES_BYTE0_DELETE_ALL_BTDM_AUTH_CODE_POS = 7,
    /// Byte 0 - Bit 7: Delete all bonds on server (BR/EDR and LE) - Bit
    BMS_FEATURES_BYTE0_DELETE_ALL_BTDM_AUTH_CODE_BIT = CO_BIT(BMS_FEATURES_BYTE0_DELETE_ALL_BTDM_AUTH_CODE_POS),

    /// Mask for features excluded if BT not supported (Byte 0)
    BMS_FEATURES_BYTE0_EXCLUDED_WITHOUT_BT_MASK = (BMS_FEATURES_BYTE0_DELETE_BTDM_BIT
                                                   | BMS_FEATURES_BYTE0_DELETE_BTDM_AUTH_CODE_BIT
                                                   | BMS_FEATURES_BYTE0_DELETE_BT_BIT
                                                   | BMS_FEATURES_BYTE0_DELETE_BT_AUTH_CODE_BIT
                                                   | BMS_FEATURES_BYTE0_DELETE_ALL_BTDM_BIT
                                                   | BMS_FEATURES_BYTE0_DELETE_ALL_BTDM_AUTH_CODE_BIT),

    /// Byte 1 - Bit 0: Delete all bonds on server (BR/EDR transport only) - Position
    BMS_FEATURES_BYTE1_DELETE_ALL_BT_POS = 0,
    /// Byte 1 - Bit 0: Delete all bonds on server (BR/EDR transport only) - Bit
    BMS_FEATURES_BYTE1_DELETE_ALL_BT_BIT = CO_BIT(BMS_FEATURES_BYTE1_DELETE_ALL_BT_POS),
    /// Byte 1 - Bit 1: Delete all bonds on server (BR/EDR transport only) with authorization code - Position
    BMS_FEATURES_BYTE1_DELETE_ALL_BT_AUTH_CODE_POS = 1,
    /// Byte 1 - Bit 1: Delete all bonds on server (BR/EDR transport only) with authorization code - Bit
    BMS_FEATURES_BYTE1_DELETE_ALL_BT_AUTH_CODE_BIT = CO_BIT(BMS_FEATURES_BYTE1_DELETE_ALL_BT_AUTH_CODE_POS),
    /// Byte 1 - Bit 2: Delete all bonds on server (LE transport only) - Position
    BMS_FEATURES_BYTE1_DELETE_ALL_LE_POS = 2,
    /// Byte 1 - Bit 2: Delete all bonds on server (LE transport only) - Bit
    BMS_FEATURES_BYTE1_DELETE_ALL_LE_BIT = CO_BIT(BMS_FEATURES_BYTE1_DELETE_ALL_LE_POS),
    /// Byte 1 - Bit 3: Delete all bonds on server (LE transport only) with authorization code - Position
    BMS_FEATURES_BYTE1_DELETE_ALL_LE_AUTH_CODE_POS = 3,
    /// Byte 1 - Bit 3: Delete all bonds on server (LE transport only) with authorization code - Bit
    BMS_FEATURES_BYTE1_DELETE_ALL_LE_AUTH_CODE_BIT = CO_BIT(BMS_FEATURES_BYTE1_DELETE_ALL_LE_AUTH_CODE_POS),
    /// Byte 1 - Bit 4: Delete bond of all except the requesting device on the server (BR/EDR and LE) - Position
    BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BTDM_POS = 4,
    /// Byte 1 - Bit 4: Delete bond of all except the requesting device on the server (BR/EDR and LE) - Bit
    BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BTDM_BIT = CO_BIT(BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BTDM_POS),
    /// Byte 1 - Bit 5: Delete bond of all except the requesting device on the server (BR/EDR and LE) with
    /// authorization code - Position
    BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BTDM_AUTH_CODE_POS = 5,
    /// Byte 1 - Bit 5: Delete bond of all except the requesting device on the server (BR/EDR and LE) with
    /// authorization code - Bit
    BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BTDM_AUTH_CODE_BIT = CO_BIT(BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BTDM_AUTH_CODE_POS),
    /// Byte 1 - Bit 6: Delete bond of all except the requesting device on the server (BR/EDR transport
    /// only) - Position
    BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BT_POS = 6,
    /// Byte 1 - Bit 6: Delete bond of all except the requesting device on the server (BR/EDR transport only) - Bit
    BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BT_BIT = CO_BIT(BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BT_POS),
    /// Byte 1 - Bit 7: Delete bond of all except the requesting device on the server (BR/EDR transport only) with
    /// authorization code - Position
    BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BT_AUTH_CODE_POS = 7,
    /// Byte 1 - Bit 7: Delete bond of all except the requesting device on the server (BR/EDR transport only) with
    /// authorization code - Bit
    BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BT_AUTH_CODE_BIT = CO_BIT(BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BT_AUTH_CODE_POS),

    /// Mask for features excluded if BT not supported (Byte 1)
    BMS_FEATURES_BYTE1_EXCLUDED_WITHOUT_BT_MASK = (BMS_FEATURES_BYTE1_DELETE_ALL_BT_BIT
                                                   | BMS_FEATURES_BYTE1_DELETE_ALL_BT_AUTH_CODE_BIT
                                                   | BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BTDM_BIT
                                                   | BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BTDM_AUTH_CODE_BIT
                                                   | BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BT_BIT
                                                   | BMS_FEATURES_BYTE1_DELETE_ALL_BUT_BT_AUTH_CODE_BIT),

    /// Byte 2 - Bit 0: Delete bond of all except the requesting device on the server (LE transport only) - Position
    BMS_FEATURES_BYTE2_DELETE_ALL_BUT_LE_POS = 0,
    /// Byte 2 - Bit 0: Delete bond of all except the requesting device on the server (LE transport only) - Bit
    BMS_FEATURES_BYTE2_DELETE_ALL_BUT_LE_BIT = CO_BIT(BMS_FEATURES_BYTE2_DELETE_ALL_BUT_LE_POS),
    /// Byte 2 - Bit 1: Delete bond of all except the requesting device on the server (LE transport only) with
    /// authorization code - Position
    BMS_FEATURES_BYTE2_DELETE_ALL_BUT_LE_AUTH_CODE_POS = 1,
    /// Byte 2 - Bit 1: Delete bond of all except the requesting device on the server (LE transport only) with
    /// authorization code - Bit
    BMS_FEATURES_BYTE2_DELETE_ALL_BUT_LE_AUTH_CODE_BIT = CO_BIT(BMS_FEATURES_BYTE2_DELETE_ALL_BUT_LE_AUTH_CODE_POS),
};

/// @} BMS_API_COMMON

#endif // BMS_H_
