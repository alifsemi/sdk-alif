/**
 ****************************************************************************************
 *
 * @file bap_bc.h
 *
 * @brief Basic Audio Profile - Broadcast - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_H_
#define BAP_BC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC Broadcast module
 * @ingroup BAP
 * @brief Description of Basic Audio Profile Broadcast module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_COMMON Common
 * @ingroup BAP_BC
 * @brief Description of common API for Basic Audio Profile Broadcast module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_ENUM Enumerations
 * @ingroup BAP_BC_COMMON
 * @brief Enumerations for Basic Audio Profile Broadcast module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_STRUCT Structures
 * @ingroup BAP_BC_COMMON
 * @brief Structures for Basic Audio Profile Broadcast module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_DEFINE Definitions
 * @ingroup BAP_BC_COMMON
 * @brief Definitions for Basic Audio Profile Broadcast module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap.h"                // Basic Audio Profile Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_DEFINE
/// @{

/// Failed to synchronized to BIG value for BIS Sync State
#define BAP_BC_BIG_SYNC_FAILED                      (0xFFFFFFFF)
/// No preference for synchronization with BIG
#define BAP_BC_BIG_SYNC_NO_PREF                     (0xFFFFFFFF)
/// Unknown Periodic Advertising interval
#define BAP_BC_UNKNOWN_PA_INTV                      (0xFFFF)
/// Maximum Presentation Delay for Broadcast (in microseconds)
#define BAP_BC_MAX_PRES_DELAY_US                    (0x0000FFFF)
/// Length of Broadcast Id
#define BAP_BC_BROADCAST_ID_LEN                     (3)
/// Maximum number of Subgroups in a Group
#define BAP_BC_NB_SUBGROUPS_MAX                     (31)
/// Length of header for Basic Audio Announcement (without length field)
#define BAP_BC_BASIC_AUDIO_ANNOUNCE_HEAD_LENGTH     (3)
/// Minimal length of Broadcast Audio Announcement
#define BAP_BC_BCAST_AUDIO_ANNOUNCEMENT_LEN         (6)
/// Minimal length of Public Broadcast Announcement Service UUID is present
#define BAP_BC_PUBLIC_BCAST_ANNOUNCEMENT_MIN_LEN    (5)
/// Minimal Public Broadcast Name length
#define BAP_BC_PUBLIC_BCAST_NAME_LEN_MIN            (4)
/// Maximal Public Broadcast Name length
#define BAP_BC_PUBLIC_BCAST_NAME_LEN_MAX            (32)

/// @} BAP_BC_DEFINE

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_ENUM
/// @{

/// Values for PA Sync State subfield in Scan State field in Broadcast Receive State characteristic value
enum bap_bc_rx_state_pa
{
    /// Not synchronized to PA
    BAP_BC_RX_STATE_PA_NOT_SYNCED = 0,
    /// SyncInfo Request
    BAP_BC_RX_STATE_PA_SYNCINFO_REQ,
    /// Synchronized to PA
    BAP_BC_RX_STATE_PA_SYNCED,
    /// Failed to synchronize with PA
    BAP_BC_RX_STATE_PA_FAILED,
    /// No PAST
    BAP_BC_RX_STATE_PA_NO_PAST,

    BAP_BC_RX_STATE_PA_MAX,
};

/// Values for PA Sync field in Broadcast Audio Scan Control Point characteristic value
enum bap_bc_pa_sync
{
    /// Do not synchronize to PA
    BAP_BC_PA_SYNC_NO_SYNC = 0,
    /// Synchronize to PA, PAST on client
    BAP_BC_PA_SYNC_SYNC_PAST,
    /// Synchronize to PA, no PAST on client
    BAP_BC_PA_SYNC_SYNC_NO_PAST,

    BAP_BC_PA_SYNC_MAX
};

/// Values for BIG encryption subfield in Scan State field in Broadcast Receive State characteristic value
enum bap_bc_big_encrypt_state
{
    /// Not encrypted
    BAP_BC_BIG_ENCRYPT_STATE_NOT_ENCRYPTED = 0,
    /// Broadcast code required
    BAP_BC_BIG_ENCRYPT_STATE_CODE_REQUIRED,
    /// Decrypting
    BAP_BC_BIG_ENCRYPT_STATE_DECRYPTING,
    /// Bad code
    BAP_BC_BIG_ENCRYPT_STATE_BAD_CODE,

    BAP_BC_BIG_ENCRYPT_STATE_MAX,
};

/// Error codes for Broadcast Audio Scan Service
enum bap_bc_bass_err
{
    /// Opcode not supported
    BAP_BC_BASS_ERR_OPCODE_NOT_SUPPORTED = 0x80,
    /// Invalid Source ID
    BAP_BC_BASS_ERR_INVALID_SRC_ID,
};

/// Service Data bit field meaning for Periodic Advertising Sync Transfer procedure
enum bap_bc_bass_service_data_bf
{
    /// AdvA parameter matches AdvA field of the ADV_EXT_IND PDUs transmitted by the Broadcast Source
    BAP_BC_BASS_SERVICE_DATA_ADVA_DIFF_EA_POS = 0,
    BAP_BC_BASS_SERVICE_DATA_ADVA_DIFF_EA_BIT = CO_BIT(BAP_BC_BASS_SERVICE_DATA_ADVA_DIFF_EA_POS),
    /// AdvA parameter matches Source Address field for the Broadcast Receive State characteristic
    BAP_BC_BASS_SERVICE_DATA_ADVA_DIFF_SRC_ADDR_POS = 1,
    BAP_BC_BASS_SERVICE_DATA_ADVA_DIFF_SRC_ADDR_BIT = CO_BIT(BAP_BC_BASS_SERVICE_DATA_ADVA_DIFF_SRC_ADDR_POS),
    /// RFU bits
    BAP_BC_BASS_SERVICE_DATA_RFU_LSB = 2,
    BAP_BC_BASS_SERVICE_DATA_RFU_MASK = 0x00FC,
    /// Source ID for the Broadcast Receive State characteristic
    BAP_BC_BASS_SERVICE_DATA_SRC_ID_LSB = 8,
    BAP_BC_BASS_SERVICE_DATA_SRC_ID_MASK = 0xFF00,
};

#if (GAF_PBP)
/// Public Broadcast Profile features bit field for Public Broadcast Announcements
enum bap_bc_pbp_features_bf
{
    /// Indicate if Broadcast Streams are encrypted and require a Broadcast Code - Position
    BAP_BC_PBP_FEATURES_ENCRYPTED_POS = 0,
    /// Indicate if Broadcast Streams are encrypted and require a Broadcast Code - Bit
    BAP_BC_PBP_FEATURES_ENCRYPTED_BIT = CO_BIT(BAP_BC_PBP_FEATURES_ENCRYPTED_POS),

    /// Indicate if a Standard Quality configuration is supported for one of the streams - Position
    BAP_BC_PBP_FEATURES_STANDARD_QUALITY_PRESENT_POS = 1,
    /// Indicate if a Standard Quality configuration is supported for one of the streams - Bit
    BAP_BC_PBP_FEATURES_STANDARD_QUALITY_PRESENT_BIT = CO_BIT(BAP_BC_PBP_FEATURES_STANDARD_QUALITY_PRESENT_POS),

    /// Indicate if a Standard Quality configuration is supported for one of the streams - Position
    BAP_BC_PBP_FEATURES_HIGH_QUALITY_PRESENT_POS = 2,
    /// Indicate if a Standard Quality configuration is supported for one of the streams - Bit
    BAP_BC_PBP_FEATURES_HIGH_QUALITY_PRESENT_BIT = CO_BIT(BAP_BC_PBP_FEATURES_HIGH_QUALITY_PRESENT_POS),
};
#endif //(GAF_PBP)

/// @} BAP_BC_ENUM

/*
 * TYPES DEFINTIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_STRUCT
/// @{

/// Broadcast ID
typedef struct bap_bcast_id
{
    /// ID
    uint8_t id[BAP_BC_BROADCAST_ID_LEN];
} bap_bcast_id_t;

/// Subgroup information structure
typedef struct bap_bc_sgrp_info
{
    /// Structure total length in bytes (len field included)\n
    /// Shall be a multiple of 4
    uint16_t len;
    /// Metadata parameters
    bap_cfg_metadata_param_t metadata_param;
    /// BIS synchronization bit field
    uint32_t bis_sync_bf;
    /// Additional Metadata (in LTV format)
    gaf_ltv_t add_metadata;
} bap_bc_sgrp_info_t;


/// Subgroup metadata structure
typedef struct bap_bc_sgrp_metadata
{
    /// Structure total length in bytes (len field included)\n
    /// Shall be a multiple of 4
    uint16_t len;
    /// Metadata parameters
    bap_cfg_metadata_param_t metadata_param;
    /// Additional Metadata (in LTV format)
    gaf_ltv_t add_metadata;
} bap_bc_sgrp_metadata_t;

/// @} BAP_BC_STRUCT

#endif // BAP_BC_H_
