/**
 ****************************************************************************************
 *
 * @file bcs_common.h
 *
 * @brief Header File - Body Composition Service common types.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */


#ifndef _BCS_COMMON_H_
#define _BCS_COMMON_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup BCS Body Composition Service
 * @ingroup Profile
 * @brief Body Composition Service
 *
 * The Body Composition Service enables the user to expose data related to body composition
 * from a body composition analyzer (Server) intended for consumer healthcare
 * as well as sports/fitness applications.
 *
 * This file contains all definitions that are common for the server and the client parts
 * of the profile.
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "prf_types.h"
#include <stdint.h>


/*
 * DEFINES
 ****************************************************************************************
 */

/// Indicate Measurement Unsuccessful to the Client
#define BCS_MEAS_UNSUCCESSFUL                   (0xFFFF)
/// Indicate Unknown User
#define BCS_MEAS_USER_ID_UNKNOWN_USER           (0xFF)
/// All features Supported
#define BCS_FEAT_ALL_SUPP                       (0x000007FF)
/// All valid bits of Flags field (bit [0:12])
#define BCS_MEAS_FLAGS_VALID                    (0x1FFF)
/// Feature value length
#define BCS_FEAT_VAL_LEN                        (4)
/// Measurement CCC length
#define BCS_MEAS_CCC_LEN                        (2)
/// Measurement Indication size (plus 3 bytes for meas_u, mass_resol and hght_resol)
#define BCS_MEAS_IND_SIZE                       (sizeof(bcs_meas_t) + 3)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// BCS Feature Support bit field
enum bcs_feat_bf
{
    /// Time Stamp Supported
    BCS_FEAT_TIME_STAMP_SUPP_POS                = 0,
    BCS_FEAT_TIME_STAMP_SUPP_BIT                = 0x00000001,

    /// Multiple Users Supported
    BCS_FEAT_MULTIPLE_USERS_SUPP_POS            = 1,
    BCS_FEAT_MULTIPLE_USERS_SUPP_BIT            = 0x00000002,

    /// Basal Metabolism Supported
    BCS_FEAT_BASAL_METAB_SUPP_POS               = 2,
    BCS_FEAT_BASAL_METAB_SUPP_BIT               = 0x00000004,

    /// Muscle percentate supported
    BCS_FEAT_MUSCLE_PERCENTAGE_SUPP_POS         = 3,
    BCS_FEAT_MUSCLE_PERCENTAGE_SUPP_BIT         = 0x00000008,

    /// Muscle mass supported
    BCS_FEAT_MUSCLE_MASS_SUPP_POS               = 4,
    BCS_FEAT_MUSCLE_MASS_SUPP_BIT               = 0x00000010,

    /// Fat Free mass Supported
    BCS_FEAT_FAT_FREE_MASS_SUPP_POS             = 5,
    BCS_FEAT_FAT_FREE_MASS_SUPP_BIT             = 0x00000020,

    /// Soft lean mass Supported
    BCS_FEAT_SOFT_LEAN_MASS_SUPP_POS            = 6,
    BCS_FEAT_SOFT_LEAN_MASS_SUPP_BIT            = 0x00000040,

    /// Body Water Mass Supported
    BCS_FEAT_BODY_WATER_MASS_SUPP_POS           = 7,
    BCS_FEAT_BODY_WATER_MASS_SUPP_BIT           = 0x00000080,

    /// Impedance Supported
    BCS_FEAT_IMPEDANCE_SUPP_POS                 = 8,
    BCS_FEAT_IMPEDANCE_SUPP_BIT                 = 0x00000100,

    /// Weight Supported
    BCS_FEAT_WEIGHT_SUPP_POS                    = 9,
    BCS_FEAT_WEIGHT_SUPP_BIT                    = 0x00000200,

    /// Height Supported
    BCS_FEAT_HEIGHT_SUPP_POS                    = 10,
    BCS_FEAT_HEIGHT_SUPP_BIT                    = 0x00000400,

    /// Mass Measurement Resolution bit field (bits[11:14])
    BCS_FEAT_MASS_RESOL_LSB                     = 11,
    BCS_FEAT_MASS_RESOL_MASK                    = 0x00007800,

    /// Height Measurement Resolution bit field (bits[15:17])
    BCS_FEAT_HGHT_RESOL_LSB                     = 15,
    BCS_FEAT_HGHT_RESOL_MASK                    = 0x00038000,
};

/// BCS Feature Measurement Mass Resolution values
enum bcs_feat_mass_resol_value
{
    /// Mass Measurement Resolution not specified
    BCS_MASS_RESOL_NOT_SPECIFIED               = 0,
    /// Resolution of 0.5 kg or 1 lb
    BCS_MASS_RESOL_05kg_1lb                    = 1,
    /// Resolution of 0.2 kg or 0.5 lb
    BCS_MASS_RESOL_02kg_05lb                   = 2,
    /// Resolution of 0.1 kg or 0.2 lb
    BCS_MASS_RESOL_01kg_02lb                   = 3,
    /// Resolution of 0.05 kg or 0.1 lb
    BCS_MASS_RESOL_005kg_01lb                  = 4,
    /// Resolution of 0.02 kg or 0.05 lb
    BCS_MASS_RESOL_002kg_005lb                 = 5,
    /// Resolution of 0.01 kg or 0.02 lb
    BCS_MASS_RESOL_001kg_002lb                 = 6,
    /// Resolution of 0.005 kg or 0.01 lb
    BCS_MASS_RESOL_0005kg_001lb                = 7,
    /// Reserved for future use bit 8 - 15
};

/// BCS Feature Measurement Height Resolution values
enum bcs_feat_hght_resol_value
{
    /// Height Measurement Resolution not specified
    BCS_HGHT_RESOL_NOT_SPECIFIED               = 0,
    /// Resolution of 0.01 meter or 1 inch
    BCS_HGHT_RESOL_001mtr_1inch                = 1,
    /// Resolution of 0.005 meter or 0.5 inch
    BCS_HGHT_RESOL_0005mtr_05inch              = 2,
    /// Resolution of 0.001 meter or 0.1 inch
    BCS_HGHT_RESOL_0001mtr_01inch              = 3,
    /// Reserved for future use bit 8 - 15
};

/// BCS Measurement Flags bit field
enum bcs_meas_flags_bf
{
    /// Measurement Units (bit 0)
    /// 0 for SI (Weight and Mass in units of kilogram (kg) and Height in units of meter)
    /// 1 for Imperial (Weight and Mass in units of pound (lb) and Height in units of inch (in))
    BCS_MEAS_FLAGS_UNITS_IMPERIAL_POS               = 0,
    BCS_MEAS_FLAGS_UNITS_IMPERIAL_BIT               = 0x0001,

    /// Time stamp present (bit 1)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_TIMESTAMP_PRESENT_POS            = 1,
    BCS_MEAS_FLAGS_TIMESTAMP_PRESENT_BIT            = 0x0002,

    /// User ID present (bit 2)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_USER_ID_PRESENT_POS              = 2,
    BCS_MEAS_FLAGS_USER_ID_PRESENT_BIT              = 0x0004,

    /// Basal Metabolism present (bit 3)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_BASAL_METAB_PRESENT_POS          = 3,
    BCS_MEAS_FLAGS_BASAL_METAB_PRESENT_BIT          = 0x0008,

    /// Muscle Percentage present (bit 4)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_MUSCLE_PERCENT_PRESENT_POS       = 4,
    BCS_MEAS_FLAGS_MUSCLE_PERCENT_PRESENT_BIT       = 0x0010,

    /// Muscle Mass present (bit 5)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_MUSCLE_MASS_PRESENT_POS          = 5,
    BCS_MEAS_FLAGS_MUSCLE_MASS_PRESENT_BIT          = 0x0020,

    /// Fat Free Mass present (bit 6)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_FAT_FREE_MASS_PRESENT_POS        = 6,
    BCS_MEAS_FLAGS_FAT_FREE_MASS_PRESENT_BIT        = 0x0040,

    /// Soft Lean Mass present (bit 7)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_SOFT_LEAN_MASS_PRESENT_POS       = 7,
    BCS_MEAS_FLAGS_SOFT_LEAN_MASS_PRESENT_BIT       = 0x0080,

    /// Body Water Mass present (bit 8)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_BODY_WATER_MASS_PRESENT_POS      = 8,
    BCS_MEAS_FLAGS_BODY_WATER_MASS_PRESENT_BIT      = 0x0100,

    /// Impedance present (bit 9)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_IMPEDANCE_PRESENT_POS            = 9,
    BCS_MEAS_FLAGS_IMPEDANCE_PRESENT_BIT            = 0x0200,

    /// Weight present (bit 10)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_WEIGHT_PRESENT_POS               = 10,
    BCS_MEAS_FLAGS_WEIGHT_PRESENT_BIT               = 0x0400,

    /// Height present (bit 11)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_HEIGHT_PRESENT_POS               = 11,
    BCS_MEAS_FLAGS_HEIGHT_PRESENT_BIT               = 0x0800,

    /// Multiple packet Measurement (bit 12)
    /// 0 for not present
    /// 1 for present
    BCS_MEAS_FLAGS_MULTIPACKET_MEAS_POS             = 12,
    BCS_MEAS_FLAGS_MULTIPACKET_MEAS_BIT             = 0x1000,
};

/*
 * TYPE DEFINITION
 ****************************************************************************************
 */

/// Measurement value
typedef struct bcs_meas
{
    /// Presence of optional fields and measurement units
    uint16_t        flags;
    /// Body fat percentage value
    uint16_t        body_fat_percent;
    /// Time stamp value
    prf_date_time_t time_stamp;
    /// User ID value
    uint8_t         user_id;
    /// Basal Metabolism value
    uint16_t        basal_metab;
    /// Muscle Percentage value
    uint16_t        muscle_percent;
    /// Muscle Mass value
    uint16_t        muscle_mass;
    /// Fat Free Mass value
    uint16_t        fat_free_mass;
    /// Soft Lean Mass value
    uint16_t        soft_lean_mass;
    /// Body Water Mass value
    uint16_t        body_water_mass;
    /// Impedance Value
    uint16_t        impedance;
    /// Weight value
    uint16_t        weight;
    /// Height value
    uint16_t        height;
} bcs_meas_t;

/// @} bcs_common

#endif // (_BCS_COMMON_H_)
