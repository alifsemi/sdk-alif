/**
 ****************************************************************************************
 *
 * @file wscp_common.h
 *
 * @brief Header File - Weight SCale Profile common types.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _WSCP_COMMON_H_
#define _WSCP_COMMON_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup WSCP Weight SCale Profile
 * @ingroup Profile
 * @brief Weight SCale Profile
 *
 * The WSCP enables a collector device to connect and interact with a BLE enabled
 * Weighing Scale sharing its current and stored weight data.
 *
 * This file contains all definitions that are common for the server and the client parts
 * of the profile.
 *****************************************************************************************
 */
#include "co_math.h"
#include "prf_types.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Indicate Measurement Unsuccessful to the Client
#define WSC_MEASUREMENT_UNSUCCESSFUL                    (0xFFFF)
/// All valid bits of Flags field (bit [0:3])
#define WSC_MEAS_FLAGS_VALID                            (0xF)
/// Indicate Unknown User, can be used for Guests
#define WSC_MEAS_USER_ID_UNKNOWN_USER                   (0xFF)
/// Feature value length
#define WSC_FEAT_VAL_SIZE                               (4)
/// Measurement CCC length
#define WCS_MEAS_CCC_LEN                                (2)
/// Measurement data size
#define WSC_MEAS_SIZE                                   (18)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// WSC Feature Support bit field
enum wsc_feat_bf
{
    /// Time Stamp Supported
    WSC_FEAT_TIME_STAMP_SUPP_POS                = 0,
    WSC_FEAT_TIME_STAMP_SUPP_BIT                = CO_BIT(WSC_FEAT_TIME_STAMP_SUPP_POS),

    /// Multiple Users Supported
    WSC_FEAT_MULTIPLE_USERS_SUPP_POS            = 1,
    WSC_FEAT_MULTIPLE_USERS_SUPP_BIT            = CO_BIT(WSC_FEAT_MULTIPLE_USERS_SUPP_POS),

    /// BMI Supported
    WSC_FEAT_BMI_SUPP_POS                       = 2,
    WSC_FEAT_BMI_SUPP_BIT                       = CO_BIT(WSC_FEAT_BMI_SUPP_POS),

    /// Weight Measurement Resolution bits[3:6]
    WSC_FEAT_WGHT_RESOL_LSB                     = 3,
    WSC_FEAT_WGHT_RESOL_MASK                    = 0x00000078,

    /// Height Measurement Resolution bits[7:9]
    WSC_FEAT_HGHT_RESOL_LSB                     = 7,
    WSC_FEAT_HGHT_RESOL_MASK                    = 0x00000380,
};

/// WSC Feature Measurement Weight Resolution values
enum wsc_feat_wght_resol_value
{
    /// Weight Measurement Resolution not specified
    WSC_WGHT_RESOL_NOT_SPECIFIED                    = 0,
    /// Resolution of 0.5 kg or 1 lb
    WSC_WGHT_RESOL_05kg_1lb                         = 1,
    /// Resolution of 0.2 kg or 0.5 lb
    WSC_WGHT_RESOL_02kg_05lb                        = 2,
    /// Resolution of 0.1 kg or 0.2 lb
    WSC_WGHT_RESOL_01kg_02lb                        = 3,
    /// Resolution of 0.05 kg or 0.1 lb
    WSC_WGHT_RESOL_005kg_01lb                       = 4,
    /// Resolution of 0.02 kg or 0.05 lb
    WSC_WGHT_RESOL_002kg_005lb                      = 5,
    /// Resolution of 0.01 kg or 0.02 lb
    WSC_WGHT_RESOL_001kg_002lb                      = 6,
    /// Resolution of 0.005 kg or 0.01 lb
    WSC_WGHT_RESOL_0005kg_001lb                     = 7,
    /// Reserved for future use 8 - 15
};

/// WSC Feature Measurement Height Resolution values
enum wsc_feat_hght_resol_value
{
    /// Height Measurement Resolution not specified
    WSC_HGHT_RESOL_NOT_SPECIFIED               = 0,
    /// Resolution of 0.01 meter or 1 inch
    WSC_HGHT_RESOL_001mtr_1inch                = 1,
    /// Resolution of 0.005 meter or 0.5 inch
    WSC_HGHT_RESOL_0005mtr_05inch              = 2,
    /// Resolution of 0.001 meter or 0.1 inch
    WSC_HGHT_RESOL_0001mtr_01inch              = 3,
    /// Reserved for future use 4 - 7

};

/// WSC Measurement Flags bit field
enum wsc_meas_flags_bf
{
    /// Measurement Units (bit 0)
    /// 0 for SI (Weight and Mass in units of kilogram (kg) and Height in units of meter)
    /// 1 for Imperial (Weight and Mass in units of pound (lb) and Height in units of inch (in))
    WSC_MEAS_FLAGS_UNITS_IMPERIAL_POS               = 0,
    WSC_MEAS_FLAGS_UNITS_IMPERIAL_BIT               = CO_BIT(WSC_MEAS_FLAGS_UNITS_IMPERIAL_POS),

    /// Time stamp present (bit 1)
    /// 0 for not present
    /// 1 for present
    WSC_MEAS_FLAGS_TIMESTAMP_PRESENT_POS            = 1,
    WSC_MEAS_FLAGS_TIMESTAMP_PRESENT_BIT            = CO_BIT(WSC_MEAS_FLAGS_TIMESTAMP_PRESENT_POS),

    /// User ID present (bit 2)
    /// 0 for not present
    /// 1 for present
    WSC_MEAS_FLAGS_USER_ID_PRESENT_POS              = 2,
    WSC_MEAS_FLAGS_USER_ID_PRESENT_BIT              = CO_BIT(WSC_MEAS_FLAGS_USER_ID_PRESENT_POS),

    /// BMI and Height present (bit 3)
    /// 0 for not present
    /// 1 for present
    WSC_MEAS_FLAGS_BMI_PRESENT_POS                  = 3,
    WSC_MEAS_FLAGS_BMI_PRESENT_BIT                  = CO_BIT(WSC_MEAS_FLAGS_BMI_PRESENT_POS),
};

/// Measurement Data
typedef struct wsc_meas
{
    /// Presence of optional fields and measurement units
    uint8_t         flags;
    /// Weight value
    uint16_t        weight;
    /// Time Stamp value
    prf_date_time_t time_stamp;
    /// User ID value
    uint8_t         user_id;
    /// Body Mass Index
    uint16_t        bmi;
    /// Height value
    uint16_t        height;
} wsc_meas_t;

/// Measurement properties
typedef struct wsc_meas_prop
{
    /// Weight Resolution
    uint8_t         wght_resol;
    /// Measurement Units
    uint8_t         meas_u;
    /// Height Resolution
    uint8_t         hght_resol;
} wsc_meas_prop_t;


/// @} WSCP


#endif /* _WSCP_COMMON_H_ */
