/**
 ****************************************************************************************
 *
 * @file bap.h
 *
 * @brief Basic Audio Profile - Header file
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_H_
#define BAP_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP Basic Audio Profile (BAP)
 * @ingroup GAF
 * @brief Description of Basic Audio Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_COMMON Common
 * @ingroup BAP
 * @brief Description of common API for Basic Audio Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_ENUM Enumerations
 * @ingroup BAP_COMMON
 * @brief Enumerations for Basic Audio Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_STRUCT Structures
 * @ingroup BAP_COMMON
 * @brief Structures for Basic Audio Profile block
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_NATIVE_API Native API
 * @ingroup BAP_COMMON
 * @brief Description of Native API for Basic Audio Profile block
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf_cfg.h"         // GAF Configuration
#include "gaf.h"             // Code definitions
#include "gapm_le.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Vendor Specific type
#define BAP_LTV_TYPE_VENDOR         (0xFF)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_ENUM
/// @{

/// Module type values for Basic Audio Profile block
enum bap_module_type
{
    /// Common module
    BAP_MODULE_COMMON = 0,
    /// Basic Audio Profile Capabilities Server module
    BAP_MODULE_CAPA_SRV = 1,
    /// Basic Audio Profile Capabilities Client module
    BAP_MODULE_CAPA_CLI = 2,
    /// Basic Audio Profile Unicast Server module
    BAP_MODULE_UC_SRV = 3,
    /// Basic Audio Profile Unicast Client module
    BAP_MODULE_UC_CLI = 4,
    /// Basic Audio Profile Broadcast Source module
    BAP_MODULE_BC_SRC = 5,
    /// Basic Audio Profile Broadcast Sink module
    BAP_MODULE_BC_SINK = 6,
    /// Basic Audio Profile Broadcast Scan module
    BAP_MODULE_BC_SCAN = 7,
    /// Basic Audio Profile Broadcast Assistant module
    BAP_MODULE_BC_ASSIST = 8,
    /// Basic Audio Profile Broadcast Delegator module
    BAP_MODULE_BC_DELEG = 9,

    /// Maximum value
    BAP_MODULE_MAX,
};

/// LTV structure format
enum bap_ltv_fmt
{
    /// Length
    BAP_LTV_LENGTH_POS = 0,
    /// Type
    BAP_LTV_TYPE_POS,
    /// Value
    BAP_LTV_VALUE_POS,

    /// Minimal length of LTV structure
    BAP_LTV_LENGTH_MIN = 1,
};

/// Codec Specific Capabilities Types values
enum bap_capa_type
{
    BAP_CAPA_TYPE_MIN = 1,

    /// Supported Sampling Frequencies
    BAP_CAPA_TYPE_SAMP_FREQ = BAP_CAPA_TYPE_MIN,
    /// Supported Frame Durations
    BAP_CAPA_TYPE_FRAME_DUR,
    /// Audio Channel Counts
    BAP_CAPA_TYPE_CHNL_CNT,
    /// Supported Octets per Codec Frame
    BAP_CAPA_TYPE_OCTETS_FRAME,
    /// Maximum Supported Codec Frames per SDU
    BAP_CAPA_TYPE_FRAMES_SDU,

    BAP_CAPA_TYPE_MAX,
};

/// Metadata types values
enum bap_metadata_type
{
    /// Preferred Audio Contexts bit field\n
    /// - 0b0 = Context Type is not a preferred use case for this codec configuration
    /// - 0b1 = Context Type is a preferred use case for this codec configuration.
    BAP_METADATA_TYPE_PREF_CONTEXTS = 1,
    /// Streaming Audio Contexts bit field\n
    /// - 0b0 = Context Type is not an intended use case for the Audio Stream
    /// - 0b1 = Context Type is an intended use case for the Audio Stream
    BAP_METADATA_TYPE_STREAM_CONTEXTS = 2,
    /// Program Info\n
    /// Title and/or summary of Audio Stream content: UTF-8 format
    BAP_METADATA_TYPE_PROGRAM_INFO = 3,
    /// Language\n
    /// 3-byte, lower case language code as defined in ISO 639-3
    BAP_METADATA_TYPE_LANGUAGE = 4,
    /// CCID List\n
    /// Array of CCID values
    BAP_METADATA_TYPE_CCID_LIST = 5,
    /// Parental Rating\n
    /// - Bits 0 - 3: Value representing the parental rating :
    ///     - 0x00 - no rating
    ///     - 0x01 - recommended for listeners of any age
    ///     - Other values - recommended for listeners of age Y years, where Y = value + 3 years.
    ///       e.g. 0x05 = recommended for listeners of 8 years or older
    /// - Bits 4 - 7: RFU
    /// The numbering scheme aligns with Annex F of EN 300 707 v1.2.1 which defines parental rating for viewing.
    BAP_METADATA_TYPE_PARENTAL_RATING = 6,
    /// Program Info URI\n
    /// A UTF-8 formatted URL link used to present more information about Program_Info
    BAP_METADATA_TYPE_PROGRAM_INFO_URI = 7,
    /// Audio Active State\n
    /// - 0x00: No audio data is being transmitted
    /// - 0x01: Audio data is being transmitted
    /// - 0x02-0xFF: RFU
    BAP_METADATA_TYPE_AUDIO_ACTIVE_STATE = 8,
    /// Broadcast Audio Immediate Rendering Flag
    BAP_METADATA_TYPE_BROADCAST_IMMEDIATE_RENDERING_FLAG = 9,
    /// Extended Metadata
    /// - Octet 0 - 1: Extended Metadata Type
    /// - Octet 2 - 254: Extended Metadata
    BAP_METADATA_TYPE_EXTENDED = 0xFE,
    /// Vendor Specific Metadata
    BAP_METADATA_TYPE_VENDOR_SPECIFIC = 0xFF,
};

/// Minimal value of length field for Codec Specific Capabilities LTV structure
enum bap_capa_length
{
    /// Supported Sampling Frequencies
    BAP_CAPA_LENGTH_SAMP_FREQ = 3,
    /// Supported Frame Durations
    BAP_CAPA_LENGTH_FRAME_DUR = 2,
    /// Audio Channel Counts
    BAP_CAPA_LENGTH_CHNL_CNT = 2,
    /// Supported Octets per Codec Frame
    BAP_CAPA_LENGTH_OCTETS_FRAME = 5,
    /// Maximum Supported Codec Frames per SDU
    BAP_CAPA_LENGTH_FRAMES_SDU = 2,
};

/// Minimal value of length field for Metadata LTV structure
enum bap_metadata_length
{
    /// Preferred Audio Contexts
    BAP_METADATA_LENGTH_PREF_CONTEXTS = 3,
    /// Streaming Audio Contexts
    BAP_METADATA_LENGTH_STREAM_CONTEXTS = 3,
    /// Language
    BAP_METADATA_LENGTH_LANGUAGE = 4,
    /// Parental Rating
    BAP_METADATA_LENGTH_PARENTAL_RATING = 2,
    /// Audio Active State
    BAP_METADATA_LENGTH_AUDIO_ACTIVE_STATE = 2,
    /// Broadcast Audio Immediate Rendering Flag
    BAP_METADATA_LENGTH_BROADCAST_IMMEDIATE_RENDERING_FLAG = 1,
};

/// Codec Specific Configuration Types values
enum bap_cfg_type
{
    BAP_CFG_TYPE_MIN = 1,

    /// Sampling Frequencies
    BAP_CFG_TYPE_SAMP_FREQ = BAP_CFG_TYPE_MIN,
    /// Frame Duration
    BAP_CFG_TYPE_FRAME_DUR,
    /// Audio Channel Allocation
    BAP_CFG_TYPE_CHNL_LOCATION,
    /// Octets per Codec Frame
    BAP_CFG_TYPE_OCTETS_FRAME,
    /// Codec Frame Blocks Per SDU
    BAP_CFG_TYPE_FRAMES_SDU,

    BAP_CFG_TYPE_MAX,
};

/// Minimal value of length field for Codec Specific Configuration LTV structure
enum bap_cfg_length
{
    /// Sampling Frequencies
    BAP_CFG_LENGTH_SAMP_FREQ = 2,
    /// Frame Duration
    BAP_CFG_LENGTH_FRAME_DUR = 2,
    /// Audio Channel Allocation
    BAP_CFG_LENGTH_CHNL_LOCATION = 5,
    /// Octets per Codec Frame
    BAP_CFG_LENGTH_OCTETS_FRAME = 3,
    /// Codec Frame Blocks Per SDU
    BAP_CFG_LENGTH_FRAMES_SDU = 2,
};

/// Data Path configuration bit field meaning
enum bap_dp_cfg_bf
{
    /// Data Path ID - Position
    BAP_DP_CFG_DP_ID_LSB = 0,
    /// Data Path ID - Mask
    BAP_DP_CFG_DP_ID_MASK = 0x00FF,

    /// Transparent mode (Codec resides in Application) - Position
    BAP_DP_CFG_TRANSPARENT_POS = 15,
    /// Transparent mode (Codec resides in Application) - Bit
    BAP_DP_CFG_TRANSPARENT_BIT = CO_BIT(BAP_DP_CFG_TRANSPARENT_POS),
};

/// Supported Roles bit field meaning
enum bap_role_bf
{
    /// Capabilities Server supported
    BAP_ROLE_SUPP_CAPA_SRV_POS = 0,
    BAP_ROLE_SUPP_CAPA_SRV_BIT = CO_BIT(BAP_ROLE_SUPP_CAPA_SRV_POS),
    /// Capabilities Client supported
    BAP_ROLE_SUPP_CAPA_CLI_POS = 1,
    BAP_ROLE_SUPP_CAPA_CLI_BIT = CO_BIT(BAP_ROLE_SUPP_CAPA_CLI_POS),
    /// Unicast Server supported
    BAP_ROLE_SUPP_UC_SRV_POS = 2,
    BAP_ROLE_SUPP_UC_SRV_BIT = CO_BIT(BAP_ROLE_SUPP_UC_SRV_POS),
    /// Unicast Client supported
    BAP_ROLE_SUPP_UC_CLI_POS = 3,
    BAP_ROLE_SUPP_UC_CLI_BIT = CO_BIT(BAP_ROLE_SUPP_UC_CLI_POS),
    /// Broadcast Source supported
    BAP_ROLE_SUPP_BC_SRC_POS = 4,
    BAP_ROLE_SUPP_BC_SRC_BIT = CO_BIT(BAP_ROLE_SUPP_BC_SRC_POS),
    /// Broadcast Sink supported
    BAP_ROLE_SUPP_BC_SINK_POS = 5,
    BAP_ROLE_SUPP_BC_SINK_BIT = CO_BIT(BAP_ROLE_SUPP_BC_SINK_POS),
    /// Broadcast Scan supported
    BAP_ROLE_SUPP_BC_SCAN_POS = 6,
    BAP_ROLE_SUPP_BC_SCAN_BIT = CO_BIT(BAP_ROLE_SUPP_BC_SCAN_POS),
    /// Broadcast Scan Assistant supported
    BAP_ROLE_SUPP_BC_ASSIST_POS = 7,
    BAP_ROLE_SUPP_BC_ASSIST_BIT = CO_BIT(BAP_ROLE_SUPP_BC_ASSIST_POS),
    /// Broadcast Scan Delegator supported
    BAP_ROLE_SUPP_BC_DELEG_POS  = 8,
    BAP_ROLE_SUPP_BC_DELEG_BIT  = CO_BIT(BAP_ROLE_SUPP_BC_DELEG_POS),
};

/// Sampling Frequency values
enum bap_sampling_freq
{
    /// Unknown - Proprietary value
    BAP_SAMPLING_FREQ_UNKNOWN = 0,
    /// Minimum value
    BAP_SAMPLING_FREQ_MIN = 1,
    /// 8000 Hz
    BAP_SAMPLING_FREQ_8000HZ = BAP_SAMPLING_FREQ_MIN,
    /// 11025 Hz
    BAP_SAMPLING_FREQ_11025HZ,
    /// 16000 Hz
    BAP_SAMPLING_FREQ_16000HZ,
    /// 22050 Hz
    BAP_SAMPLING_FREQ_22050HZ,
    /// 24000 Hz
    BAP_SAMPLING_FREQ_24000HZ,
    /// 32000 Hz
    BAP_SAMPLING_FREQ_32000HZ,
    /// 44100 Hz
    BAP_SAMPLING_FREQ_44100HZ,
    /// 48000 Hz
    BAP_SAMPLING_FREQ_48000HZ,
    /// 88200 Hz
    BAP_SAMPLING_FREQ_88200HZ,
    /// 96000 Hz
    BAP_SAMPLING_FREQ_96000HZ,
    /// 176400 Hz
    BAP_SAMPLING_FREQ_176400HZ,
    /// 192000 Hz
    BAP_SAMPLING_FREQ_192000HZ,
    /// 384000 Hz
    BAP_SAMPLING_FREQ_384000HZ,
    /// Maximum value
    BAP_SAMPLING_FREQ_MAX = BAP_SAMPLING_FREQ_384000HZ,
};

/// Supported Sampling Frequencies bit field meaning
enum bap_sampling_freq_bf
{
    /// 8000 Hz - Position
    BAP_SAMPLING_FREQ_8000HZ_POS = 0,
    /// 8000 Hz - Bit
    BAP_SAMPLING_FREQ_8000HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_8000HZ_POS),

    /// 11025 Hz - Position
    BAP_SAMPLING_FREQ_11025HZ_POS = 1,
    /// 11025 Hz - Bit
    BAP_SAMPLING_FREQ_11025HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_11025HZ_POS),

    /// 16000 Hz - Position
    BAP_SAMPLING_FREQ_16000HZ_POS = 2,
    /// 16000 Hz - Bit
    BAP_SAMPLING_FREQ_16000HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_16000HZ_POS),

    /// 22050 Hz - Position
    BAP_SAMPLING_FREQ_22050HZ_POS = 3,
    /// 22050 Hz - Bit
    BAP_SAMPLING_FREQ_22050HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_22050HZ_POS),

    /// 24000 Hz - Position
    BAP_SAMPLING_FREQ_24000HZ_POS = 4,
    /// 24000 Hz - Bit
    BAP_SAMPLING_FREQ_24000HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_24000HZ_POS),

    /// 32000 Hz - Position
    BAP_SAMPLING_FREQ_32000HZ_POS = 5,
    /// 32000 Hz - Bit
    BAP_SAMPLING_FREQ_32000HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_32000HZ_POS),

    /// 44100 Hz - Position
    BAP_SAMPLING_FREQ_44100HZ_POS = 6,
    /// 44100 Hz - Bit
    BAP_SAMPLING_FREQ_44100HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_44100HZ_POS),

    /// 48000 Hz - Position
    BAP_SAMPLING_FREQ_48000HZ_POS = 7,
    /// 48000 Hz - Bit
    BAP_SAMPLING_FREQ_48000HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_48000HZ_POS),

    /// 88200 Hz - Position
    BAP_SAMPLING_FREQ_88200HZ_POS = 8,
    /// 88200 Hz - Bit
    BAP_SAMPLING_FREQ_88200HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_88200HZ_POS),

    /// 96000 Hz - Position
    BAP_SAMPLING_FREQ_96000HZ_POS = 9,
    /// 96000 Hz - Bit
    BAP_SAMPLING_FREQ_96000HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_96000HZ_POS),

    /// 176400 Hz - Position
    BAP_SAMPLING_FREQ_176400HZ_POS = 10,
    /// 176400 Hz - Bit
    BAP_SAMPLING_FREQ_176400HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_176400HZ_POS),

    /// 192000 Hz - Position
    BAP_SAMPLING_FREQ_192000HZ_POS = 11,
    /// 192000 Hz - Bit
    BAP_SAMPLING_FREQ_192000HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_192000HZ_POS),

    /// 384000 Hz - Position
    BAP_SAMPLING_FREQ_384000HZ_POS = 12,
    /// 384000 Hz - Bit
    BAP_SAMPLING_FREQ_384000HZ_BIT = CO_BIT(BAP_SAMPLING_FREQ_384000HZ_POS),
};

/// Frame Duration values
enum bap_frame_dur
{
    /// Use 7.5ms Codec frames
    BAP_FRAME_DUR_7_5MS = 0,
    /// Use 10ms Codec frames
    BAP_FRAME_DUR_10MS,

    /// Maximum value
    BAP_FRAME_DUR_MAX = BAP_FRAME_DUR_10MS,

    /// Frame duration is unknown
    BAP_FRAME_DUR_UNKNOWN = 0xFF,
};

/// Supported Frame Durations bit field meaning
enum bap_frame_dur_bf
{
    /// 7.5ms frame duration is supported - Position
    BAP_FRAME_DUR_7_5MS_POS = 0,
    /// 7.5ms frame duration is supported - Bit
    BAP_FRAME_DUR_7_5MS_BIT = CO_BIT(BAP_FRAME_DUR_7_5MS_POS),

    /// 10ms frame duration is supported - Position
    BAP_FRAME_DUR_10MS_POS = 1,
    /// 10ms frame duration is supported - Bit
    BAP_FRAME_DUR_10MS_BIT = CO_BIT(BAP_FRAME_DUR_10MS_POS),

    /// 7.5ms frame duration is preferred - Position
    BAP_FRAME_DUR_7_5MS_PREF_POS = 4,
    /// 7.5ms frame duration is preferred - Bit
    BAP_FRAME_DUR_7_5MS_PREF_BIT = CO_BIT(BAP_FRAME_DUR_7_5MS_PREF_POS),

    /// 10ms frame duration is preferred - Position
    BAP_FRAME_DUR_10MS_PREF_POS = 5,
    /// 10ms frame duration is preferred - Bit
    BAP_FRAME_DUR_10MS_PREF_BIT = CO_BIT(BAP_FRAME_DUR_10MS_PREF_POS),

    /// RFU bits mask
    BAP_FRAME_DUR_RFU_MASK = 0xCC,
};

/// Context type bit field meaning
enum bap_context_type_bf
{
    // Prohibited value
    BAP_CONTEXT_TYPE_PROHIBITED = 0,

    /// Unspecified - Position
    BAP_CONTEXT_TYPE_UNSPECIFIED_POS = 0,
    /// Unspecified - Bit
    BAP_CONTEXT_TYPE_UNSPECIFIED_BIT = CO_BIT(BAP_CONTEXT_TYPE_UNSPECIFIED_POS),

    /// Conversational - Position
    BAP_CONTEXT_TYPE_CONVERSATIONAL_POS = 1,
    /// Conversational - Bit\n
    BAP_CONTEXT_TYPE_CONVERSATIONAL_BIT = CO_BIT(BAP_CONTEXT_TYPE_CONVERSATIONAL_POS),

    /// Media - Position
    BAP_CONTEXT_TYPE_MEDIA_POS = 2,
    /// Media - Bit\n
    BAP_CONTEXT_TYPE_MEDIA_BIT = CO_BIT(BAP_CONTEXT_TYPE_MEDIA_POS),

    /// Game - Position
    BAP_CONTEXT_TYPE_GAME_POS = 3,
    /// Game - Bit\n
    BAP_CONTEXT_TYPE_GAME_BIT = CO_BIT(BAP_CONTEXT_TYPE_GAME_POS),

    /// Instructional - Position
    BAP_CONTEXT_TYPE_INSTRUCTIONAL_POS = 4,
    /// Instructional - Bit\n
    BAP_CONTEXT_TYPE_INSTRUCTIONAL_BIT = CO_BIT(BAP_CONTEXT_TYPE_INSTRUCTIONAL_POS),

    /// Voice assistants - Position
    BAP_CONTEXT_TYPE_VOICE_ASSISTANTS_POS = 5,
    /// Voice assistants - Bit\n
    BAP_CONTEXT_TYPE_VOICE_ASSISTANTS_BIT = CO_BIT(BAP_CONTEXT_TYPE_VOICE_ASSISTANTS_POS),

    /// Live - Position
    BAP_CONTEXT_TYPE_LIVE_POS = 6,
    /// Live - Bit\n
    BAP_CONTEXT_TYPE_LIVE_BIT = CO_BIT(BAP_CONTEXT_TYPE_LIVE_POS),

    /// Sound effects - Position
    BAP_CONTEXT_TYPE_SOUND_EFFECTS_POS = 7,
    /// Sound effets - Bit\n
    BAP_CONTEXT_TYPE_SOUND_EFFECTS_BIT = CO_BIT(BAP_CONTEXT_TYPE_SOUND_EFFECTS_POS),

    /// Notifications - Position
    BAP_CONTEXT_TYPE_NOTIFICATIONS_POS = 8,
    /// Notifications - Bit\n
    BAP_CONTEXT_TYPE_NOTIFICATIONS_BIT = CO_BIT(BAP_CONTEXT_TYPE_NOTIFICATIONS_POS),

    /// Ringtone - Position
    BAP_CONTEXT_TYPE_RINGTONE_POS = 9,
    /// Ringtone - Bit\n
    BAP_CONTEXT_TYPE_RINGTONE_BIT = CO_BIT(BAP_CONTEXT_TYPE_RINGTONE_POS),

    /// Alerts - Position
    BAP_CONTEXT_TYPE_ALERTS_POS = 10,
    /// Alerts - Bit\n
    BAP_CONTEXT_TYPE_ALERTS_BIT = CO_BIT(BAP_CONTEXT_TYPE_ALERTS_POS),

    /// Emergency alarm - Position
    BAP_CONTEXT_TYPE_EMERGENCY_ALARM_POS = 11,
    /// Emergency alarm - Bit\n
    BAP_CONTEXT_TYPE_EMERGENCY_ALARM_BIT = CO_BIT(BAP_CONTEXT_TYPE_EMERGENCY_ALARM_POS),

    BAP_CONTEXT_TYPE_ALL = (BAP_CONTEXT_TYPE_UNSPECIFIED_BIT
                                | BAP_CONTEXT_TYPE_CONVERSATIONAL_BIT
                                | BAP_CONTEXT_TYPE_MEDIA_BIT
                                | BAP_CONTEXT_TYPE_GAME_BIT
                                | BAP_CONTEXT_TYPE_INSTRUCTIONAL_BIT
                                | BAP_CONTEXT_TYPE_VOICE_ASSISTANTS_BIT
                                | BAP_CONTEXT_TYPE_LIVE_BIT
                                | BAP_CONTEXT_TYPE_SOUND_EFFECTS_BIT
                                | BAP_CONTEXT_TYPE_NOTIFICATIONS_BIT
                                | BAP_CONTEXT_TYPE_RINGTONE_BIT
                                | BAP_CONTEXT_TYPE_ALERTS_BIT
                                | BAP_CONTEXT_TYPE_EMERGENCY_ALARM_BIT),
    BAP_CONTEXT_TYPE_RFU_LSB = BAP_CONTEXT_TYPE_EMERGENCY_ALARM_POS + 1,
    BAP_CONTEXT_TYPE_RFU_MASK = ~BAP_CONTEXT_TYPE_ALL,
};

/// @} BAP_ENUM

/*
 * TYPE DEFINITION
 ****************************************************************************************
 */

/// @addtogroup BAP_STRUCT
/// @{

/// Configuration structure for BAP Capabilities Server module
typedef struct bap_capa_srv_cfg
{
    /// Number of PAC Groups for Sink direction
    uint8_t nb_pacs_sink;
    /// Number of PAC Groups for Source direction
    uint8_t nb_pacs_src;
    /// Configuration bit field (see #bap_capa_srv_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Preferred MTU
    /// Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
    /// Required start handle
    /// If set to GATT_INVALID_HDL, the start handle will be automatically chosen
    uint16_t shdl;
    /// Supported Audio Locations bit field for Sink direction (see #gaf_loc_bf enumeration)
    /// Meaningful only if nb_pac_sink is different than 0
    uint32_t location_bf_sink;
    /// Supported Audio Locations bit field for Source direction (see #gaf_loc_bf enumeration)
    /// Meaningful only if nb_pac_src is different than 0
    uint32_t location_bf_src;
    /// Supported Audio Contexts bit field for Sink direction
    /// Meaningful only if nb_pac_sink is different than 0
    uint16_t supp_context_bf_sink;
    /// Supported Audio Contexts bit field for Source direction
    /// Meaningful only if nb_pac_src is different than 0
    uint16_t supp_context_bf_src;
} bap_capa_srv_cfg_t;

/// Configuration structure for BAP Capabilities Client module
typedef struct bap_capa_cli_cfg
{
    /// Preferred MTU
    /// Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
} bap_capa_cli_cfg_t;

/// Configuration structure for BAP Unicast Server module
typedef struct bap_uc_srv_cfg
{
    /// Number of Sink ASE characteristic instances
    /// Shall be in the range [0, 15]
    /// Total number of Sink/Source ASE characteristics cannot be higher than 15
    /// At least one Sink/Source ASE characteristic shall be supported
    uint8_t nb_ase_chars_sink;
    /// Number of Source ASE characteristic instances
    /// Shall be in the range [0, 15]
    /// Total number of Sink/Source ASE characteristics cannot be higher than 15
    /// At least one Sink/Source ASE characteristic shall be supported
    uint8_t nb_ase_chars_src;
    /// Number of ASE configurations that can be maintained
    /// Shall be at least equal to nb_ase_chars_sink + nb_ase_chars_src
    /// Should be a multiple of nb_ase_chars_sink + nb_ase_chars_src
    /// Shall not be larger than (nb_ase_chars_sink + nb_ase_chars_src) * HOST_CONNECTION_MAX
    /// Cannot be higher than 32
    uint8_t nb_ases_cfg;
    /// Configuration bit field (see #bap_uc_srv_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Preferred MTU
    /// Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
    /// Required start handle
    /// If set to GATT_INVALID_HDL, the start handle will automatically chosen
    uint16_t shdl;
} bap_uc_srv_cfg_t;

/// Configuration structure for BAP Unicast Client module
typedef struct bap_uc_cli_cfg
{
    /// Configuration bit field
    uint8_t cfg_bf;
    /// Number of ASE configurations that can be maintained\n
    /// Shall be at larger than 0
    uint8_t nb_ases_cfg;
    /// Preferred MTU\n
    /// Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
    /// Timeout duration in seconds for reception of notification for ASE Control Point characteristic and for
    /// some notifications of ASE characteristic\n
    /// From 1s to 5s, 0 means 1s
    uint8_t timeout_s;
} bap_uc_cli_cfg_t;

/// Broadcast Scanner configuration structure
typedef struct bap_bc_scan_cfg
{
    /// Number of Broadcast Source information that may be stored in the cache
    uint8_t cache_size;
    /// Number of Periodic Synchronizations that may be established in parallel
    uint8_t nb_sync;
} bap_bc_scan_cfg_t;

/// Broadcast Assistant configuration structure
typedef struct bap_bc_assist_cfg
{
    /// Configuration bit field (see #bap_bc_assist_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Preferred MTU - Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
} bap_bc_assist_cfg_t;

/// Broadcast Delegator configuration structure
typedef struct bap_bc_deleg_cfg
{
    /// Number of supported Broadcast Sources Up to 15
    uint8_t nb_srcs;
    /// Configuration bit field (see #bap_bc_deleg_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Required start handle - If set to GATT_INVALID_HDL, the start handle will be automatically chosen
    uint16_t shdl;
    /// Preferred MTU - Values from 0 to 63 are equivalent to 64
    uint16_t pref_mtu;
} bap_bc_deleg_cfg_t;

/// Broadcast Group Parameters structure
typedef struct bap_bc_grp_param
{
    /// SDU interval in microseconds
    /// From 256us (0x00000100) to 1.048575s (0x000FFFFF)
    uint32_t sdu_intv_us;
    /// Maximum size of an SDU
    /// From 1 to 4095 bytes
    uint16_t max_sdu;
    /// Maximum time (in milliseconds) between the first transmission of an SDU to the end of the last transmission
    /// of the same SDU
    /// From 0ms to 4.095s (0x0FFF)
    uint16_t max_tlatency_ms;
    /// Sequential or Interleaved scheduling (see #iso_packing enumeration)
    uint8_t packing;
    /// Unframed or framed mode (see #iso_frame enumeration)
    uint8_t framing;
    /// Bitfield indicating PHYs that can be used by the controller for transmission of SDUs
    /// (see #le_phy_mask enumeration)
    uint8_t phy_bf;
    /// Number of times every PDU should be transmitted
    /// From 0 to 15
    uint8_t rtn;
} bap_bc_grp_param_t;

/// Advertising Parameters structure
typedef struct bap_bc_adv_param
{
    /// Minimum advertising interval in multiple of 0.625ms
    /// From 20ms (0x00000020) to 10485.759375s (0x00FFFFFF)
    uint32_t adv_intv_min_slot;
    /// Maximum advertising interval in multiple of 0.625ms
    /// From 20ms (0x00000020) to 10485.759375s (0x00FFFFFF)
    uint32_t adv_intv_max_slot;
    /// Channel Map (see #adv_channel_map enumeration)
    uint8_t ch_map;
    /// PHY for primary advertising (see #gap_le_phy_val enumeration)
    /// Only LE 1M and LE Coded PHYs are allowed
    uint8_t phy_prim;
    /// PHY for secondary advertising (see #gap_le_phy_val enumeration)
    uint8_t phy_second;
    /// Advertising SID
    /// From 0x00 to 0x0F
    uint8_t adv_sid;
    /// Required TX Power level in dBm (from -127 to 20 dBm)\n
    /// ADV_TX_PWR_NO_PREF (0x7F) means no preference\n
    /// TX Power level value chosen by controller reported in created callback function
    int8_t tx_pwr;
    /// Own address type (see #gapm_le_own_addr enumeration)
    uint8_t own_addr_type;
    /// Maximum advertising events the Controller can skip before sending the AUX_ADV_IND packets on the
    /// secondary advertising physical channel
    uint8_t max_skip;
    /// Indicate if TX Power must be part of advertising data
    bool send_tx_pwr;
} bap_bc_adv_param_t;

/// Periodic Advertising Parameters structure
typedef struct bap_bc_per_adv_param
{
    /// Minimum Periodic Advertising interval in multiple of 1.25ms
    /// Must be higher than 7.5ms (0x0006)
    uint16_t adv_intv_min_frame;
    /// Maximum Periodic Advertising interval in multiple of 1.25ms
    /// Must be higher than 7.5ms (0x0006)
    uint16_t adv_intv_max_frame;
    /// Indicate if TX Power must be part of periodic advertising data
    bool send_tx_pwr;
} bap_bc_per_adv_param_t;

#if (BLE_PER_ADV)
/// Advertising identification structure
typedef gapm_le_per_adv_bdaddr_t bap_adv_id_t;
#endif //(BLE_PER_ADV)

/// Codec Capabilities parameters structure
typedef struct bap_capa_param
{
    /// Supported Sampling Frequencies bit field (see #bap_sampling_freq_bf enumeration)\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// Mandatory for LC3
    uint16_t sampling_freq_bf;
    /// Supported Frame Durations bit field (see #bap_frame_dur_bf enumeration)\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// Mandatory for LC3
    uint8_t frame_dur_bf;
    /// Supported Audio Channel Counts\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// For LC3, absence in the Codec Specific Capabilities is equivalent to 1 channel supported (forced to 0x01
    /// on reception side)
    uint8_t chan_cnt_bf;
    /// Supported Octets Per Codec Frame - Minimum\n
    /// Not part of the Codec Specific Capabilities is equal to 0 and frame_octet_max also equal to 0\n
    /// Mandatory for LC3
    uint16_t frame_octet_min;
    /// Supported Octets Per Codec Frame - Maximum\n
    /// Not part of the Codec Specific Capabilities is equal to 0 and frame_octet_min also equal to 0\n
    /// Mandatory for LC3
    uint16_t frame_octet_max;
    /// Supported Maximum Codec Frames Per SDU\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// For LC3, absence in the Codec Specific Capabilities is equivalent to 1 Frame Per SDU (forced to 1 on
    /// reception side)
    uint8_t max_frames_sdu;
} bap_capa_param_t;

/// Codec Capabilities structure
typedef struct bap_capa
{
    /// Parameters structure
    bap_capa_param_t param;
    /// Additional Codec Capabilities (in LTV format)
    gaf_ltv_t add_capa;
} bap_capa_t;

/// Codec Capabilities structure (Additional Codec Capabilities provided as a pointer)
typedef struct bap_capa_ptr
{
    /// Parameters structure
    bap_capa_param_t param;
    /// Pointer to Additional Codec Capabilities (in LTV format)
    const gaf_ltv_t* p_add_capa;
} bap_capa_ptr_t;

/// Codec Capabilities Metadata parameters structure
typedef struct bap_capa_metadata_param
{
    /// Preferred Audio Contexts bit field (see #bap_context_type_bf enumeration)
    uint16_t context_bf;
} bap_capa_metadata_param_t;

/// Codec Capabilities Metadata structure
typedef struct bap_capa_metadata
{
    /// Parameters structure
    bap_capa_metadata_param_t param;
    /// Additional Metadata (in LTV format)
    gaf_ltv_t add_metadata;
} bap_capa_metadata_t;

/// Codec Capabilities Metadata structure (Additional Metadata provided as a pointer)
typedef struct bap_capa_metadata_ptr
{
    /// Parameters structure
    bap_capa_metadata_param_t param;
    /// Pointer to Additional Codec Capabilities (in LTV format)
    const gaf_ltv_t* p_add_metadata;
} bap_capa_metadata_ptr_t;

/// Codec Configuration parameters structure
typedef struct bap_cfg_param
{
    /// Audio Locations of the Audio Channels being configured for the codec (i.e the number of codec frames per
    /// block) and their ordering within a single block of codec frames (see #gaf_loc_bf enumeration)
    /// When transmitted, part of Codec Specific Configuration only if not equal to 0
    /// When received, 0 shall be interpreted as a single channel with no specified Audio Location
    uint32_t location_bf;
    /// Length of a codec frame in octets
    uint16_t frame_octet;
    /// Sampling Frequency (see #bap_sampling_freq enumeration)
    uint8_t sampling_freq;
    /// Frame Duration (see #bap_frame_dur enumeration)
    uint8_t frame_dur;
    /// Number of blocks of codec frames that shall be sent or received in a single SDU
    uint8_t frames_sdu;
} bap_cfg_param_t;

/// Codec Configuration structure
typedef struct bap_cfg
{
    /// Parameters structure
    bap_cfg_param_t param;
    /// Additional Codec Configuration (in LTV format)
    gaf_ltv_t add_cfg;
} bap_cfg_t;

/// Codec Configuration structure (Additional Codec Configuration provided as a pointer)
typedef struct bap_cfg_ptr
{
    /// Parameters structure
    bap_cfg_param_t param;
    /// Pointer to Additional Codec Configuration (in LTV format)
    const gaf_ltv_t* p_add_cfg;
} bap_cfg_ptr_t;

/// Codec Configuration Metadata parameters structure
typedef struct bap_cfg_metadata_param
{
    /// Streaming Audio Contexts bit field (see #bap_context_type_bf enumeration)
    uint16_t context_bf;
} bap_cfg_metadata_param_t;

/// Codec Configuration Metadata structure
typedef struct bap_cfg_metadata
{
    /// Parameters structure
    bap_cfg_metadata_param_t param;
    /// Additional Metadata value (in LTV format)
    gaf_ltv_t add_metadata;
} bap_cfg_metadata_t;

///  Codec Configuration Metadata structure (with additional Metadata provided as pointer)
typedef struct bap_cfg_metadata_ptr
{
    /// Parameters structure
    bap_cfg_metadata_param_t param;
    /// Pointer to additional Metadata value (in LTV format)
    const gaf_ltv_t* p_add_metadata;
} bap_cfg_metadata_ptr_t;

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

typedef struct bap_capa_srv_cb bap_capa_srv_cb_t;
typedef struct bap_capa_cli_cb bap_capa_cli_cb_t;
typedef struct bap_uc_srv_cb bap_uc_srv_cb_t;
typedef struct bap_uc_cli_cb  bap_uc_cli_cb_t;
typedef struct bap_bc_src_cb bap_bc_src_cb_t;
typedef struct bap_bc_sink_cb   bap_bc_sink_cb_t;
typedef struct bap_bc_scan_cb bap_bc_scan_cb_t;
typedef struct bap_bc_assist_cb bap_bc_assist_cb_t;
typedef struct bap_bc_deleg_cb  bap_bc_deleg_cb_t;

/// Basic Audio Profile callback set for each roles
typedef struct bap_cb
{
    /// Capabilities Server callback functions
    const bap_capa_srv_cb_t* p_capa_srv_cb;
    /// Capabilities Client callback functions
    const bap_capa_cli_cb_t* p_capa_cli_cb;
    /// Unicast Server callback functions
    const bap_uc_srv_cb_t* p_uc_srv_cb;
    /// Unicast Client callbacks
    const bap_uc_cli_cb_t* p_uc_cli_cb;
    /// Broadcast Source callback functions
    const bap_bc_src_cb_t* p_bc_src_cb;
    /// Broadcast Sink callback functions
    const bap_bc_sink_cb_t* p_bc_sink_cb;
    /// Broadcast Scan callback functions
    const bap_bc_scan_cb_t* p_bc_scan_cb;
    /// Broadcast Scan Assistant callbacks
    const bap_bc_assist_cb_t* p_bc_assist_cb;
    /// Broadcast Scan Delegator callbacks
    const bap_bc_deleg_cb_t* p_bc_deleg_cb;
} bap_cb_t;

/// @} BAP_STRUCT

/*
 * API FUNCTION DEFINITION
 ****************************************************************************************
 */

/// @addtogroup BAP_NATIVE_API
/// @{

/**
 ****************************************************************************************
 * @brief Configure Basic Audio profile
 *
 * @param[in] role_bf           Supported role bit field (see #bap_role_bf enumeration)
 * @param[in] p_capa_srv_cfg    Pointer to Capabilities Server configuration
 * @param[in] p_capa_cli_cfg    Pointer to Capabilities Client configuration
 * @param[in] p_uc_srv_cfg      Pointer to Unicast Server configuration
 * @param[in] p_uc_cli_cfg      Pointer to Unicast Client configuration
 * @param[in] p_bc_deleg_cfg    Pointer to Broadcast Delegator configuration
 * @param[in] p_bc_assist_cfg   Pointer to Broadcast Assistant configuration
 * @param[in] p_cb              Pointer to callback structure
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t bap_configure(uint32_t role_bf, bap_capa_srv_cfg_t* p_capa_srv_cfg, bap_capa_cli_cfg_t* p_capa_cli_cfg,
                       bap_uc_srv_cfg_t* p_uc_srv_cfg, bap_uc_cli_cfg_t* p_uc_cli_cfg,
                       bap_bc_deleg_cfg_t* p_bc_deleg_cfg, bap_bc_assist_cfg_t* p_bc_assist_cfg,
                       const bap_cb_t* p_cb);

/// @} BAP_NATIVE_API

#endif // BAP_H_
