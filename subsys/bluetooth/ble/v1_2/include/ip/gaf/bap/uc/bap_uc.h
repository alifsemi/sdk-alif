/**
 ****************************************************************************************
 *
 * @file bap_uc.h
 *
 * @brief Basic Audio Profile - Unicast - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_UC_H_
#define BAP_UC_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_UC Unicast module
 * @ingroup BAP
 * @brief Description of Basic Audio Profile Unicast module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_COMMON Common
 * @ingroup BAP_UC
 * @brief Description of common API for Basic Audio Profile Unicast module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_ENUM Enumerations
 * @ingroup BAP_UC_COMMON
 * @brief Enumerations for Basic Audio Profile Unicast module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_STRUCT Structures
 * @ingroup BAP_UC_COMMON
 * @brief Structures for Basic Audio Profile Unicast module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_DEFINE Definitions
 * @ingroup BAP_UC_COMMON
 * @brief Definitions for Basic Audio Profile Unicast module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap.h"            // Basic Audio Profile Definitions

/*
 * DEFINES
 ****************************************************************************************
 */


/// @addtogroup BAP_UC_DEFINE
/// @{

/// Maximum Presentation Delay for Unicast (in microseconds)
#define BAP_UC_MAX_PRES_DELAY_US          (0x00FFFFFF)

/// @} BAP_UC_DEFINE

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_UC_ENUM
/// @{

/// List of Audio Stream Control Service characteristics
enum bap_uc_char_type
{
    /// ASE Control Point characteristic
    BAP_UC_CHAR_TYPE_CP = 0,
    /// Sink/Source ASE characteristic
    BAP_UC_CHAR_TYPE_ASE,

    BAP_UC_CHAR_TYPE_MAX,
};

/// Operation code values for ASE Control Point characteristic
enum bap_uc_opcode
{
    /// Configure Codec
    BAP_UC_OPCODE_CFG_CODEC = 1,
    BAP_UC_OPCODE_MIN = BAP_UC_OPCODE_CFG_CODEC,
    /// Configure QoS
    BAP_UC_OPCODE_CFG_QOS,
    /// Enable
    BAP_UC_OPCODE_ENABLE,
    /// Receiver Start Ready
    BAP_UC_OPCODE_RX_START_READY,
    /// Disable
    BAP_UC_OPCODE_DISABLE,
    /// Receiver Stop Ready
    BAP_UC_OPCODE_RX_STOP_READY,
    /// Update Metadata
    BAP_UC_OPCODE_UPDATE_METADATA,
    /// Release
    BAP_UC_OPCODE_RELEASE,

    BAP_UC_OPCODE_MAX,
};

/// ASE State values
enum bap_uc_ase_state
{
    /// Idle
    BAP_UC_ASE_STATE_IDLE = 0,
    /// Codec configured
    BAP_UC_ASE_STATE_CODEC_CONFIGURED,
    /// QoS configured
    BAP_UC_ASE_STATE_QOS_CONFIGURED,
    /// Enabling
    BAP_UC_ASE_STATE_ENABLING,
    /// Streaming
    BAP_UC_ASE_STATE_STREAMING,
    /// Disabling
    BAP_UC_ASE_STATE_DISABLING,
    /// Releasing
    BAP_UC_ASE_STATE_RELEASING,

    BAP_UC_ASE_STATE_MAX,
};

/// Response Code values for ASE Control Point characteristic
enum bap_uc_cp_rsp_code
{
    /// Success
    BAP_UC_CP_RSP_CODE_SUCCESS = 0,
    /// Unsupported Opcode
    BAP_UC_CP_RSP_CODE_UNSUPPORTED_OPCODE,
    /// Invalid Length
    BAP_UC_CP_RSP_CODE_INVALID_LENGTH,
    /// Invalid ASE ID
    BAP_UC_CP_RSP_CODE_INVALID_ASE_ID,
    /// Invalid ASE State Machine Transition
    BAP_UC_CP_RSP_CODE_INVALID_TRANSITION,
    /// Invalid ASE direction
    BAP_UC_CP_RSP_CODE_INVALID_ASE_DIRECTION,
    /// Unsupported Audio Capabilities
    BAP_UC_CP_RSP_CODE_UNSUPPORTED_AUDIO_CAPA,
    /// Unsupported Configuration Parameter value
    BAP_UC_CP_RSP_CODE_UNSUPPORTED_CFG_PARAM,
    /// Rejected Configuration Parameter value
    BAP_UC_CP_RSP_CODE_REJECTED_CFG_PARAM,
    /// Invalid Configuration Parameter value
    BAP_UC_CP_RSP_CODE_INVALID_CFG_PARAM,
    /// Unsupported Metadata
    BAP_UC_CP_RSP_CODE_UNSUPPORTED_METADATA,
    /// Rejected Metadata
    BAP_UC_CP_RSP_CODE_REJECTED_METADATA,
    /// Invalid Metadata
    BAP_UC_CP_RSP_CODE_INVALID_METADATA,
    /// Insufficient Resources
    BAP_UC_CP_RSP_CODE_INSUFFICIENT_RESOURCES,
    /// Unspecified Error
    BAP_UC_CP_RSP_CODE_UNSPECIFIED_ERROR,

    BAP_UC_CP_RSP_CODE_MAX,
};

/// Reason values for ASE Control Point characteristic
enum bap_uc_cp_reason
{
    BAP_UC_CP_REASON_MIN = 1,
    /// Codec ID
    BAP_UC_CP_REASON_CODEC_ID = BAP_UC_CP_REASON_MIN,
    /// Codec Specific Configuration
    BAP_UC_CP_REASON_CODEC_SPEC_CFG,
    /// SDU Interval
    BAP_UC_CP_REASON_SDU_INTERVAL,
    /// Framing
    BAP_UC_CP_REASON_FRAMING,
    /// PHY
    BAP_UC_CP_REASON_PHY,
    /// Maximum SDU Size
    BAP_UC_CP_REASON_MAX_SDU_SIZE,
    /// Retransmission Number
    BAP_UC_CP_REASON_RETX_NB,
    /// Maximum Transport Latency
    BAP_UC_CP_REASON_MAX_TRANS_LATENCY,
    /// Presentation Delay
    BAP_UC_CP_REASON_PRES_DELAY,
    /// Invalid ASE CIS Mapping
    BAP_UC_CP_REASON_INVALID_ASE_CIS_MAPPING,

    BAP_UC_CP_REASON_MAX,
};

/// Target Latency values
enum bap_uc_tgt_latency
{
    BAP_UC_TGT_LATENCY_MIN = 1,

    /// Target lower latency
    BAP_UC_TGT_LATENCY_LOWER = BAP_UC_TGT_LATENCY_MIN,
    /// Target balanced latency and reliability
    BAP_UC_TGT_LATENCY_BALENCED,
    /// Target higher reliability
    BAP_UC_TGT_LATENCY_RELIABLE,

    BAP_UC_TGT_LATENCY_MAX,
};

/// Target PHY values
enum bap_uc_tgt_phy
{
    BAP_UC_TGT_PHY_MIN = 1,

    /// 1M PHY
    BAP_UC_TGT_PHY_1M = BAP_UC_TGT_PHY_MIN,
    /// 2M PHY
    BAP_UC_TGT_PHY_2M,
    /// Coded PHY
    BAP_UC_TGT_PHY_CODED,

    BAP_UC_TGT_PHY_MAX,
};

/// @} BAP_UC_ENUM

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup BAP_UC_STRUCT
/// @{

/// QoS Requirement structure
typedef struct bap_qos_req
{
    /// Presentation Delay minimum microseconds
    uint32_t pres_delay_min_us;
    /// Presentation Delay maximum in microseconds
    uint32_t pres_delay_max_us;
    /// Minimum preferred presentation delay in microseconds
    /// 0 means no preference
    /// If not equal to 0, shall be >= pres_delay_min_us
    uint32_t pref_pres_delay_min_us;
    /// Maximum preferred presentation delay in microseconds
    /// 0 means no preference
    /// If not equal to 0, shall be <= pres_delay_max_us
    uint32_t pref_pres_delay_max_us;
    /// Maximum Transport latency in milliseconds
    /// From 5ms (0x5) to 4000ms (0xFA0)
    uint16_t trans_latency_max_ms;
    /// PDU framing arrangement
    uint8_t framing;
    /// Preferred PHY bit field
    uint8_t phy_bf;
    /// Preferred maximum number of retransmissions for each CIS Data PDU
    /// From 0 to 0xFF
    uint8_t retx_nb;
} bap_qos_req_t;

/// QoS Configuration structure
typedef struct bap_qos_cfg
{
    /// PDU framing arrangement
    uint8_t framing;
    /// PHY
    uint8_t phy;
    /// Maximum number of retransmissions for each CIS Data PDU
    /// From 0 to 0xFF
    uint8_t retx_nb;
    /// Maximum SDU size
    /// From 0 to 4095 bytes (0xFFF)
    uint16_t max_sdu_size;
    /// Maximum Transport latency in milliseconds
    /// From 5ms (0x5) to 4000ms (0xFA0)
    uint16_t trans_latency_max_ms;
    /// Presentation Delay in microseconds
    uint32_t pres_delay_us;
    /// SDU interval in microseconds
    /// From 255us (0xFF) to 1048575us (0xFFFFF)
    uint32_t sdu_intv_us;
} bap_qos_cfg_t;

/// @} BAP_UC_STRUCT

#endif // BAP_UC_H_
