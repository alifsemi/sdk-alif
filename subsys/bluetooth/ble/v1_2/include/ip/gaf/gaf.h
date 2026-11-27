/**
 ****************************************************************************************
 *
 * @file gaf.h
 *
 * @brief Generic Audio Framework - Header file
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAF_H_
#define GAF_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GAF Generic Audio Framework (GAF)
 * @brief Description of Generic Audio Framework layer
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_TB Toolbox (TB)
 * @ingroup GAF
 * @brief Description of GAF Toolbox
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_COMMON Common
 * @ingroup GAF
 * @brief Description of enumerations, structures and definitions common for all GAF module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_DEF Definitions
 * @ingroup GAF_COMMON
 * @brief Definitions for GAF
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_ENUM Enumerations
 * @ingroup GAF_COMMON
 * @brief Enumerations for GAF
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_STRUCT Structures
 * @ingroup GAF_COMMON
 * @brief Structures for GAF
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf_cfg.h"                // Generic Audio Framework Configuration
#include "gap.h"                    // GAP Definitions
#include "gapi.h"                   // GAP Isochronous Definitions
#include "prf_types.h"              // Profile Definitions

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup GAF_DEF
/// @{

/// Maximum number of ASE (must not be higher than 7 due to uint8_t size, see ASCS block)
#define GAF_ASE_NB_MAX                       (7)
/// Invalid local index
#define GAF_INVALID_LID                      (0xFF)

/// Length of Broadcast Audio Scan Service advertising data
#define GAF_ADV_AUDIO_SCAN_SVC_DATA_LENGTH   (3)
/// Minimum length for Basic Audio Announcement
#define GAF_ADV_GRP_MIN_LEN                  (4)
/// Minimum length of Subgroup description in advertising data
#define GAF_ADV_SUBGRP_MIN_LEN               (8)
/// Minimum length of Stream description in advertising data
#define GAF_ADV_STREAM_MIN_LEN               (2)
/// Minimum length for CAP General/Targeted Announcement
#define GAF_CAP_ANNOUNCEMENT_AD_LEN          (4)
/// Minimum length for BAP General/Targeted Announcement
#define GAF_BAP_ANNOUNCEMENT_AD_LEN          (9)

/// Length of Codec ID value
#define GAF_CODEC_ID_LEN                     (CODEC_ID_LEN)
/// Position of Codec Format field in Codec ID
#define GAF_CODEC_ID_FORMAT_POS              (0)
/// Length of Vendor Specific Part of Codec ID
#define GAF_CODEC_ID_VENDOR_SPEC_LEN         (GAF_CODEC_ID_LEN - 1)

/// LC3 Codec ID
#define GAF_CODEC_ID_LC3                     { .codec_id = { GAPI_CODEC_FORMAT_LC3, 0, 0, 0, 0 } }
/// Transparent Codec ID (controller does not do any transcoding or resampling)
#define GAF_CODEC_ID_TRANSPARENT             { .codec_id = { GAPI_CODEC_FORMAT_TRANSPARENT, 0, 0, 0, 0 } }

/// @} GAF_DEF

/*
 * MACROS
 ****************************************************************************************
 */

/// Mask for GAF error code
#define GAF_ERR_CODE(idx)                    (0x0200 | idx)
/// Generate a command/request/request indication/indication code
#define GAF_CODE(layer, module, idx)         ((GAF_LAYER_ ## layer << 12)  | (layer ##_MODULE_ ## module << 8) | idx)
/// Retrieve layer from message code
#define GAF_LAYER(code)                      ((code & 0xF000) >> 12)
/// Retrieve module from message code
#define GAF_MODULE(code)                     ((code & 0x0F00) >> 8)
/// Retrieve code index from message code
#define GAF_CODE_IDX(code)                   (code & 0x00FF)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup GAF_ENUM
/// @{

/// GAF Layers
enum gaf_layers
{
    /// Generic Audio Framework
    GAF_LAYER_GAF = 0,
    /// Basic Audio Profile
    GAF_LAYER_BAP = 2,
    /// Common Audio Profile
    GAF_LAYER_CAP = 3,
    /// Audio Content Control
    GAF_LAYER_ACC = 4,
    /// Audio Rendering Control
    GAF_LAYER_ARC = 5,
    /// Audio Topology Control
    GAF_LAYER_ATC = 6,
    /// Telephony and Media Audio Profile
    GAF_LAYER_TMAP = 7,
    /// Hearing Access Profile
    GAF_LAYER_HAP = 8,
    /// Gaming Audio Profile
    GAF_LAYER_GMAP = 9,

    GAF_LAYER_MAX,
};

/// Module type values
enum gaf_module_type
{
    /// Common
    GAF_MODULE_COMMON = 0,
    /// Advertiser
    GAF_MODULE_ADV,
    /// Scanner
    GAF_MODULE_SCAN,
    /// Client
    GAF_MODULE_CLI,

    GAF_MODULE_MAX,
};

/// GAF-specific error codes
/// Also see #hl_err enumeration
enum gaf_err
{
    /// No error
    GAF_ERR_NO_ERROR = 0, //!< Value: 0x0000
    /// Invalid parameters
    GAF_ERR_INVALID_PARAM = GAF_ERR_CODE(0x01), //!< Value: 0x0201
    /// Command disallowed
    GAF_ERR_COMMAND_DISALLOWED = GAF_ERR_CODE(0x02), //!< Value: 0x0202
    /// Unknown command
    GAF_ERR_UNKNOWN_COMMAND = GAF_ERR_CODE(0x03), //!< Value: 0x0203
    /// Unknown request
    GAF_ERR_UNKNOWN_REQUEST = GAF_ERR_CODE(0x04), //!< Value: 0x0204
    /// Insufficient resources
    GAF_ERR_INSUFFICIENT_RESOURCES = GAF_ERR_CODE(0x05), //!< Value: 0x0205
    /// Invalid connection
    GAF_ERR_INVALID_CONNECTION = GAF_ERR_CODE(0x06), //!< Value: 0x0206
    /// Busy
    GAF_ERR_BUSY = GAF_ERR_CODE(0x07), //!< Value: 0x0207
    /// Failed
    GAF_ERR_FAILED = GAF_ERR_CODE(0x08), //!< Value: 0x0208
    /// Read procedure successful but read value was not correct
    GAF_ERR_READ_ERROR = GAF_ERR_CODE(0x09), //!< Value: 0x0209
    /// Indicated direction is valid but not supported
    GAF_ERR_DIRECTION_NOT_SUPPORTED = GAF_ERR_CODE(0x0A), //!< Value: 0x020A
    /// Invalid Output
    GAF_ERR_INVALID_OUTPUT = GAF_ERR_CODE(0x0B), //!< Value: 0x020B
    /// Invalid Input
    GAF_ERR_INVALID_INPUT = GAF_ERR_CODE(0x0C), //!< Value: 0x020C
    /// Invalid PAC
    GAF_ERR_INVALID_PAC = GAF_ERR_CODE(0x0D), //!< Value: 0x020D
    /// Optional feature not supported by peer device
    GAF_ERR_PEER_NOT_SUPPORT = GAF_ERR_CODE(0x0E), //!< Value: 0x020E
    /// Invalid Source
    GAF_ERR_INVALID_SRC = GAF_ERR_CODE(0x0F), //!< Value: 0x020F
    /// Invalid ASE
    GAF_ERR_INVALID_ASE = GAF_ERR_CODE(0x10), //!< Value: 0x0210

    /// Link Group already exists
    GAF_ERR_LINK_GROUP_EXISTS = GAF_ERR_CODE(0x11), //!< Value: 0x0211
    /// Link already exists
    GAF_ERR_LINK_EXISTS = GAF_ERR_CODE(0x12), //!< Value: 0x0212
    /// Invalid Link
    GAF_ERR_INVALID_LINK = GAF_ERR_CODE(0x13), //!< Value: 0x0213

    /// Not configured
    GAF_ERR_NOT_CONFIGURED = GAF_ERR_CODE(0x14), //!< Value: 0x0214

    /// Internal error
    GAF_ERR_INTERNAL_ERROR = GAF_ERR_CODE(0x15), //!< Value: 0x0215

    /// No ASE available
    GAF_ERR_NO_AVA_ASE = GAF_ERR_CODE(0x16), //!< Value: 0x0216
    /// Invalid Bearer
    GAF_ERR_INVALID_BEARER = GAF_ERR_CODE(0x17), //!< Value: 0x0217
    /// Invalid Call
    GAF_ERR_INVALID_CALL = GAF_ERR_CODE(0x18), //!< Value: 0x0218
    /// Invalid Media
    GAF_ERR_INVALID_MEDIA = GAF_ERR_CODE(0x19), //!< Value: 0x0219
    /// Optional feature not supported locally
    GAF_ERR_LOCAL_NOT_SUPPORT = GAF_ERR_CODE(0x1A), //!< Value: 0x021A
    /// Discovery error
    GAF_ERR_DISC_ERROR = GAF_ERR_CODE(0x1B), //!< Value: 0x021B
    /// Invalid Record
    GAF_ERR_INVALID_RECORD = GAF_ERR_CODE(0x1C), //!< Value: 0x021C
    /// Invalid ASE ID
    GAF_ERR_INVALID_ASE_ID = GAF_ERR_CODE(0x1D), //!< Value: 0x021D
    /// Disconnection has occurred on the ACL link during the procedure
    GAF_ERR_DISCONNECTED = GAF_ERR_CODE(0x1E), //!< Value: 0x021E
    /// Not ready
    GAF_ERR_NOT_READY = GAF_ERR_CODE(0x1F), //!< Value: 0x021F
    /// Timeout
    GAF_ERR_TIMEOUT = GAF_ERR_CODE(0x20), //!< Value: 0x0220

    /// ASE procedure stopped due to reception of Disabling state
    GAF_ERR_ASE_DISABLING = GAF_ERR_CODE(0x21), //!< Value: 0x0221
    /// ASE procedure stopped due to reception of Releasing state
    GAF_ERR_ASE_RELEASING = GAF_ERR_CODE(0x22), //!< Value: 0x0222

    /// Procedure has been aborted due to another procedure\n
    GAF_ERR_ABORTED = GAF_ERR_CODE(0x23), //!< Value: 0x0223
};

/// GAF Configuration bit field meaning
enum gaf_cfg_bf
{
    /// Client module supported
    GAF_CFG_CLI_SUPP_POS = 0,
    GAF_CFG_CLI_SUPP_BIT = CO_BIT(GAF_CFG_CLI_SUPP_POS),
    /// Advertiser module supported
    GAF_CFG_ADV_SUPP_POS = 1,
    GAF_CFG_ADV_SUPP_BIT = CO_BIT(GAF_CFG_ADV_SUPP_POS),
    /// Scanner module supported
    GAF_CFG_SCAN_SUPP_POS = 2,
    GAF_CFG_SCAN_SUPP_BIT = CO_BIT(GAF_CFG_SCAN_SUPP_POS),
};

/// ASE Direction
enum gaf_direction
{
    /// Sink direction
    GAF_DIRECTION_SINK = 0,
    /// Source direction
    GAF_DIRECTION_SRC,

    GAF_DIRECTION_MAX,
};

/// Direction requirements bit field
enum gaf_direction_bf
{
    /// Required for sink direction
    GAF_DIRECTION_BF_SINK_POS = 0,
    GAF_DIRECTION_BF_SINK_BIT = 0x01,

    /// Required for source direction
    GAF_DIRECTION_BF_SRC_POS = 1,
    GAF_DIRECTION_BF_SRC_BIT = 0x02,

    /// Required for both directions
    GAF_DIRECTION_BF_BOTH = GAF_DIRECTION_BF_SRC_BIT + GAF_DIRECTION_BF_SINK_BIT,
};

/// Announcement type values
enum gaf_announcement_type
{
    /// General Announcement
    GAF_ANNOUNCEMENT_TYPE_GENERAL = 0,
    /// Targeted Announcement
    GAF_ANNOUNCEMENT_TYPE_TARGETED = 1,
};

/// Announcement type bit field meaning
enum gaf_announcement_type_bf
{
    /// General/Targeted Announcement
    GAF_ANNOUNCEMENT_POS = 0,
    GAF_ANNOUNCEMENT_BIT = CO_BIT(GAF_ANNOUNCEMENT_POS),
    /// CAP Announcement received
    GAF_ANNOUNCEMENT_CAP_POS = 1,
    GAF_ANNOUNCEMENT_CAP_BIT = CO_BIT(GAF_ANNOUNCEMENT_CAP_POS),
    /// BAP Announcement received
    GAF_ANNOUNCEMENT_BAP_POS = 2,
    GAF_ANNOUNCEMENT_BAP_BIT = CO_BIT(GAF_ANNOUNCEMENT_BAP_POS),
};

/// Format of Targeted/General Announcement
enum gaf_announcement_fmt
{
    /// Length - Position
    GAF_ANNOUNCEMENT_LENGTH_POS = 0,
    /// AD type - Position\n
    /// Set to Service Data - 16-bit UUID
    GAF_ANNOUNCEMENT_AD_TYPE_POS,
    /// Service UUID - Position\n
    /// Set to Audio Stream Control Service UUID
    GAF_ANNOUNCEMENT_UUID_POS,
    /// Announcement Type - Position\n
    /// (see #gaf_announcement_type enumeration)
    GAF_ANNOUNCEMENT_TYPE_POS = GAF_ANNOUNCEMENT_UUID_POS + 2,
    /// Available Audio Contexts - Position
    GAF_ANNOUNCEMENT_CONTEXTS_POS,
    /// Metadata length - Position
    GAF_ANNOUNCEMENT_METADATA_LENGTH_POS = GAF_ANNOUNCEMENT_CONTEXTS_POS + 4,
    /// Metadata - Position
    GAF_ANNOUNCEMENT_METADATA_POS,

    /// Minimal length
    GAF_ANNOUNCEMENT_LEN_MIN = GAF_ANNOUNCEMENT_METADATA_POS,
};

/// Audio Locations bit field meaning
enum gaf_loc_bf
{
    GAF_LOC_MONO_AUDIO = 0,

    /// Front Left
    GAF_LOC_FRONT_LEFT_POS = 0,
    GAF_LOC_FRONT_LEFT_BIT = CO_BIT(GAF_LOC_FRONT_LEFT_POS),
    /// Front Right
    GAF_LOC_FRONT_RIGHT_POS = 1,
    GAF_LOC_FRONT_RIGHT_BIT = CO_BIT(GAF_LOC_FRONT_RIGHT_POS),
    /// Front Center
    GAF_LOC_FRONT_CENTER_POS = 2,
    GAF_LOC_FRONT_CENTER_BIT = CO_BIT(GAF_LOC_FRONT_CENTER_POS),
    /// Low Frequency Effect 1
    GAF_LOC_LFE1_POS = 3,
    GAF_LOC_LFE1_BIT = CO_BIT(GAF_LOC_LFE1_POS),
    /// Back Left
    GAF_LOC_BACK_LEFT_POS = 4,
    GAF_LOC_BACK_LEFT_BIT = CO_BIT(GAF_LOC_BACK_LEFT_POS),
    /// Back Right
    GAF_LOC_BACK_RIGHT_POS = 5,
    GAF_LOC_BACK_RIGHT_BIT = CO_BIT(GAF_LOC_BACK_RIGHT_POS),
    /// Front Left Center
    GAF_LOC_FRONT_LEFT_CENTER_POS = 6,
    GAF_LOC_FRONT_LEFT_CENTER_BIT = CO_BIT(GAF_LOC_FRONT_LEFT_CENTER_POS),
    /// Front Right Center
    GAF_LOC_FRONT_RIGHT_CENTER_POS = 7,
    GAF_LOC_FRONT_RIGHT_CENTER_BIT = CO_BIT(GAF_LOC_FRONT_RIGHT_CENTER_POS),
    /// Back Center
    GAF_LOC_BACK_CENTER_POS = 8,
    GAF_LOC_BACK_CENTER_BIT = CO_BIT(GAF_LOC_BACK_CENTER_POS),
    /// Low Frequency Effect 2
    GAF_LOC_LFE2_POS = 9,
    GAF_LOC_LFE2_BIT = CO_BIT(GAF_LOC_LFE2_POS),
    /// Side Left
    GAF_LOC_SIDE_LEFT_POS = 10,
    GAF_LOC_SIDE_LEFT_BIT = CO_BIT(GAF_LOC_SIDE_LEFT_POS),
    /// Side Right
    GAF_LOC_SIDE_RIGHT_POS = 11,
    GAF_LOC_SIDE_RIGHT_BIT = CO_BIT(GAF_LOC_SIDE_RIGHT_POS),
    /// Top Front Left
    GAF_LOC_TOP_FRONT_LEFT_POS = 12,
    GAF_LOC_TOP_FRONT_LEFT_BIT = CO_BIT(GAF_LOC_TOP_FRONT_LEFT_POS),
    /// Top Front Right
    GAF_LOC_TOP_FRONT_RIGHT_POS = 13,
    GAF_LOC_TOP_FRONT_RIGHT_BIT = CO_BIT(GAF_LOC_TOP_FRONT_RIGHT_POS),
    /// Top Front Center
    GAF_LOC_TOP_FRONT_CENTER_POS = 14,
    GAF_LOC_TOP_FRONT_CENTER_BIT = CO_BIT(GAF_LOC_TOP_FRONT_CENTER_POS),
    /// Top Center
    GAF_LOC_TOP_CENTER_POS = 15,
    GAF_LOC_TOP_CENTER_BIT = CO_BIT(GAF_LOC_TOP_CENTER_POS),
    /// Top Back Left
    GAF_LOC_TOP_BACK_LEFT_POS = 16,
    GAF_LOC_TOP_BACK_LEFT_BIT = CO_BIT(GAF_LOC_TOP_BACK_LEFT_POS),
    /// Top Back Right
    GAF_LOC_TOP_BACK_RIGHT_POS = 17,
    GAF_LOC_TOP_BACK_RIGHT_BIT = CO_BIT(GAF_LOC_TOP_BACK_RIGHT_POS),
    /// Top Side Left
    GAF_LOC_TOP_SIDE_LEFT_POS = 18,
    GAF_LOC_TOP_SIDE_LEFT_BIT = CO_BIT(GAF_LOC_TOP_SIDE_LEFT_POS),
    /// Top Side Right
    GAF_LOC_TOP_SIDE_RIGHT_POS = 19,
    GAF_LOC_TOP_SIDE_RIGHT_BIT = CO_BIT(GAF_LOC_TOP_SIDE_RIGHT_POS),
    /// Top Back Center
    GAF_LOC_TOP_BACK_CENTER_POS = 20,
    GAF_LOC_TOP_BACK_CENTER_BIT = CO_BIT(GAF_LOC_TOP_BACK_CENTER_POS),
    /// Bottom Front Center
    GAF_LOC_BOTTOM_FRONT_CENTER_POS = 21,
    GAF_LOC_BOTTOM_FRONT_CENTER_BIT = CO_BIT(GAF_LOC_BOTTOM_FRONT_CENTER_POS),
    /// Bottom Front Left
    GAF_LOC_BOTTOM_FRONT_LEFT_POS = 22,
    GAF_LOC_BOTTOM_FRONT_LEFT_BIT = CO_BIT(GAF_LOC_BOTTOM_FRONT_LEFT_POS),
    /// Bottom Front Right
    GAF_LOC_BOTTOM_FRONT_RIGHT_POS = 23,
    GAF_LOC_BOTTOM_FRONT_RIGHT_BIT = CO_BIT(GAF_LOC_BOTTOM_FRONT_RIGHT_POS),
    /// Front Left Wide
    GAF_LOC_FRONT_LEFT_WIDE_POS = 24,
    GAF_LOC_FRONT_LEFT_WIDE_BIT = CO_BIT(GAF_LOC_FRONT_LEFT_WIDE_POS),
    /// Front Right Wide
    GAF_LOC_FRONT_RIGHT_WIDE_POS = 25,
    GAF_LOC_FRONT_RIGHT_WIDE_BIT = CO_BIT(GAF_LOC_FRONT_RIGHT_WIDE_POS),
    /// Left Surround
    GAF_LOC_LEFT_SURROUND_POS = 26,
    GAF_LOC_LEFT_SURROUND_BIT = CO_BIT(GAF_LOC_LEFT_SURROUND_POS),
    /// Right Surround
    GAF_LOC_RIGHT_SURROUND_POS = 27,
    GAF_LOC_RIGHT_SURROUND_BIT = CO_BIT(GAF_LOC_RIGHT_SURROUND_POS),
};

/// @} GAF_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAF_STRUCT
/// @{

/// Codec Identifier
typedef struct gaf_codec_id
{
    /// Codec ID value
    uint8_t codec_id[GAF_CODEC_ID_LEN];
} gaf_codec_id_t;

/// Data value in LTV format
typedef gapi_ltv_t gaf_ltv_t;

/// Broadcast code used for stream encryption
typedef struct gaf_bcast_code
{
    /// Broadcast Code value
    uint8_t bcast_code[GAP_KEY_LEN];
} gaf_bcast_code_t;

/// Advertising air information
typedef struct gaf_adv_report_air_info
{
    /// TX power (in dBm)
    int8_t tx_pwr;
    /// RSSI (between -127 and +20 dBm)
    int8_t rssi;
} gaf_adv_report_air_info_t;

typedef struct gaf_adv_cfg gaf_adv_cfg_t;

/// @} GAF_STRUCT

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

typedef struct gaf_adv_cb gaf_adv_cb_t;
typedef struct gaf_scan_cb gaf_scan_cb_t;
typedef struct gaf_cli_cb gaf_cli_cb_t;

/*
 * GLOBAL FUNCTIONS
 ****************************************************************************************
 */

#if (GAF_LIB)
uint16_t gaf_configure(uint8_t role_bf, const gaf_adv_cfg_t* p_cfg_adv, const gaf_adv_cb_t* p_cb_adv,
                       const gaf_scan_cb_t* p_cb_scan, const gaf_cli_cb_t* p_cb_cli);
#endif //(GAF_LIB)

#endif // GAF_H_
