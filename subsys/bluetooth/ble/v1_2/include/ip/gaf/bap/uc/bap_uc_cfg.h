/**
 ****************************************************************************************
 *
 * @file bap_uc_cfg.h
 *
 * @brief Basic Audio Profile - Unicast Audio Stream Configuration Settings
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_UC_CFG_H_
#define BAP_UC_CFG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CFG Audio Stream configuration
 * @ingroup BAP_UC
 * @brief Unicast Audio Stream QoS configuration settings
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CFG_LATENCY Low Latency
 * @ingroup BAP_UC_CFG
 * @brief Settings for low latency audio data
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CFG_RELIABILITY High-Reliability
 * @ingroup BAP_UC_CFG
 * @brief Settings for high-reliability audio data
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_UC_CFG_GAMING Gaming
 * @ingroup BAP_UC_CFG
 * @brief Settings for Gaming audio data
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap_uc.h"             // Basic Audio Profile Definitions

/// @addtogroup BAP_UC_CFG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// BAP Unicast configuration indices
enum bap_uc_cfg_idx
{
    /// 8_1_1
    BAP_UC_CFG_8_1_1_IDX = 0,
    /// 8_2_1
    BAP_UC_CFG_8_2_1_IDX,
    /// 8_1_2
    BAP_UC_CFG_8_1_2_IDX,
    /// 8_2_2
    BAP_UC_CFG_8_2_2_IDX,
    /// 16_1_1
    BAP_UC_CFG_16_1_1_IDX,
    /// 16_2_1
    BAP_UC_CFG_16_2_1_IDX,
    /// 16_1_2
    BAP_UC_CFG_16_1_2_IDX,
    /// 16_2_2
    BAP_UC_CFG_16_2_2_IDX,
    /// 24_1_1
    BAP_UC_CFG_24_1_1_IDX,
    /// 24_2_1
    BAP_UC_CFG_24_2_1_IDX,
    /// 24_1_2
    BAP_UC_CFG_24_1_2_IDX,
    /// 24_2_2
    BAP_UC_CFG_24_2_2_IDX,
    /// 32_1_1
    BAP_UC_CFG_32_1_1_IDX,
    /// 32_2_1
    BAP_UC_CFG_32_2_1_IDX,
    /// 32_1_2
    BAP_UC_CFG_32_1_2_IDX,
    /// 32_2_2
    BAP_UC_CFG_32_2_2_IDX,
    /// 441_1_1
    BAP_UC_CFG_441_1_1_IDX,
    /// 441_2_1
    BAP_UC_CFG_441_2_1_IDX,
    /// 441_1_2
    BAP_UC_CFG_441_1_2_IDX,
    /// 441_2_2
    BAP_UC_CFG_441_2_2_IDX,
    /// 48_1_1
    BAP_UC_CFG_48_1_1_IDX,
    /// 48_2_1
    BAP_UC_CFG_48_2_1_IDX,
    /// 48_3_1
    BAP_UC_CFG_48_3_1_IDX,
    /// 48_4_1
    BAP_UC_CFG_48_4_1_IDX,
    /// 48_5_1
    BAP_UC_CFG_48_5_1_IDX,
    /// 48_6_1
    BAP_UC_CFG_48_6_1_IDX,
    /// 48_1_2
    BAP_UC_CFG_48_1_2_IDX,
    /// 48_2_2
    BAP_UC_CFG_48_2_2_IDX,
    /// 48_3_2
    BAP_UC_CFG_48_3_2_IDX,
    /// 48_4_2
    BAP_UC_CFG_48_4_2_IDX,
    /// 48_5_2
    BAP_UC_CFG_48_5_2_IDX,
    /// 48_6_2
    BAP_UC_CFG_48_6_2_IDX,
    /// 16_1_gs
    BAP_UC_CFG_16_1_GS_IDX,
    /// 16_2_gs
    BAP_UC_CFG_16_2_GS_IDX,
    /// 32_1_gs
    BAP_UC_CFG_32_1_GS_IDX,
    /// 32_2_gs
    BAP_UC_CFG_32_2_GS_IDX,
    /// 48_1_gs
    BAP_UC_CFG_48_1_GS_IDX,
    /// 48_2_gs
    BAP_UC_CFG_48_2_GS_IDX,
    /// 32_1_gr
    BAP_UC_CFG_32_1_GR_IDX,
    /// 32_2_gr
    BAP_UC_CFG_32_2_GR_IDX,
    /// 48_1_gr
    BAP_UC_CFG_48_1_GR_IDX,
    /// 48_2_gr
    BAP_UC_CFG_48_2_GR_IDX,
    /// 48_3_gr
    BAP_UC_CFG_48_3_GR_IDX,
    /// 48_4_gr
    BAP_UC_CFG_48_4_GR_IDX,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Configuration structure
typedef struct bap_uc_cfg
{
    /// PDU framing arrangement
    uint8_t framing;
    /// Maximum number of retransmissions for each CIS Data PDU
    /// From 0 to 0xFF
    uint8_t rtn;
    /// Sampling frequency (see #bap_sampling_freq enumeration)
    uint8_t samp_freq;
    /// Maximum SDU size
    /// From 0 to 4095 bytes (0xFFF)
    uint16_t max_sdu_size;
    /// Maximum Transport latency in milliseconds
    /// From 5ms (0x5) to 4000ms (0xFA0)
    uint16_t max_tlatency_ms;
    /// Presentation Delay in microseconds
    uint32_t pres_delay_us;
    /// SDU interval in microseconds
    /// From 255us (0xFF) to 1048575us (0xFFFFF)
    uint32_t sdu_intv_us;
} bap_uc_cfg_t;

/// @} BAP_UC_CFG

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup BAP_UC_CFG_LATENCY
/// @{

/// 8_1_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_8_1_1            { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_8000HZ,    \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 26,                       \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 8 }

/// 8_2_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_8_2_1            { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_8000HZ,    \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 30,                       \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms =  10 }

/// 16_1_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_16_1_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_16000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 30,                       \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 8 }

/// 16_2_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_16_2_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_16000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 40,                       \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 10 }

/// 24_1_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_24_1_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_24000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 45,                       \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 8 }

/// 24_2_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_24_2_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_24000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 60,                       \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 10 }

/// 32_1_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_32_1_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 60,                       \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 8 }

/// 32_2_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_32_2_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 80,                       \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 10 }

/// 441_1_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_441_1_1          { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 8163,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_44100HZ,   \
                                      .framing = ISO_FRAMED_MODE,               \
                                      .max_sdu_size = 97,                       \
                                      .rtn = 5,                                 \
                                      .max_tlatency_ms = 24 }

/// 441_2_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_441_2_1          { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10884,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_44100HZ,   \
                                      .framing = ISO_FRAMED_MODE,               \
                                      .max_sdu_size = 130,                      \
                                      .rtn = 5,                                 \
                                      .max_tlatency_ms = 31 }

/// 48_1_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_1_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 75,                       \
                                      .rtn = 5,                                 \
                                      .max_tlatency_ms = 15 }

/// 48_2_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_2_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 100,                      \
                                      .rtn = 5,                                 \
                                      .max_tlatency_ms = 20 }

/// 48_3_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_3_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 90,                       \
                                      .rtn = 5,                                 \
                                      .max_tlatency_ms = 15 }

/// 48_4_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_4_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 120,                      \
                                      .rtn = 5,                                 \
                                      .max_tlatency_ms = 20 }

/// 48_5_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_5_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 117,                      \
                                      .rtn = 5,                                 \
                                      .max_tlatency_ms = 15 }

/// 48_6_1 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_6_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 155,                      \
                                      .rtn = 5,                                 \
                                      .max_tlatency_ms = 20 }

/// @} BAP_UC_CFG_LATENCY

/// @addtogroup BAP_UC_CFG_RELIABILITY
/// @{

/// 8_1_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_8_1_2            { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_8000HZ,    \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 26,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 75 }

/// 8_2_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_8_2_2            { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_8000HZ,    \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 30,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms =  95 }

/// 16_1_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_16_1_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_16000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 30,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 75 }

/// 16_2_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_16_2_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_16000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 40,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 95 }

/// 24_1_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_24_1_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_24000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 45,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 75 }

/// 24_2_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_24_2_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_24000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 60,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 95 }

/// 32_1_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_32_1_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 60,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 75 }

/// 32_2_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_32_2_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 80,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 95 }

/// 441_1_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_441_1_2          { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 8163,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_44100HZ,   \
                                      .framing = ISO_FRAMED_MODE,               \
                                      .max_sdu_size = 97,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 80 }

/// 441_2_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_441_2_2          { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10884,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_44100HZ,   \
                                      .framing = ISO_FRAMED_MODE,               \
                                      .max_sdu_size = 130,                      \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 85 }

/// 48_1_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_1_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 75,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 75 }

/// 48_2_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_2_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 100,                      \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 95 }

/// 48_3_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_3_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 90,                       \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 75 }

/// 48_4_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_4_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 120,                      \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 100 }

/// 48_5_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_5_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 117,                      \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 75 }

/// 48_6_2 configuration (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_6_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 155,                      \
                                      .rtn = 13,                                \
                                      .max_tlatency_ms = 100 }

/// @} BAP_UC_CFG_RELIABILITY

/// @addtogroup BAP_UC_CFG_GAMING
/// @{

/// 16_1_gs configuration (UGT to UGG) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_16_1_gs          { .pres_delay_us = 60000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_16000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 30,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 15 }

/// 16_2_gs configuration (UGT to UGG) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_16_2_gs          { .pres_delay_us = 60000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_16000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 40,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 20 }

/// 32_1_gs configuration (UGT to UGG) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_32_1_gs          { .pres_delay_us = 60000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 60,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 15 }

/// 32_2_gs configuration (UGT to UGG) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_32_2_gs          { .pres_delay_us = 60000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 80,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 20 }

/// 48_1_gs configuration (UGT to UGG) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_1_gs          { .pres_delay_us = 60000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 75,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 15 }

/// 48_2_gs configuration (UGT to UGG) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_2_gs          { .pres_delay_us = 60000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 100,                      \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 20 }

/// 32_1_gr configuration (UGG to UGT) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_32_1_gr          { .pres_delay_us = 10000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 60,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 15 }

/// 32_2_gr configuration (UGG to UGT) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_32_2_gr          { .pres_delay_us = 10000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 80,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 20 }

/// 48_1_gr configuration (UGG to UGT) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_1_gr          { .pres_delay_us = 10000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 75,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 15 }

/// 48_2_gr configuration (UGG to UGT) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_2_gr          { .pres_delay_us = 10000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 100,                      \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 20 }

/// 48_3_gr configuration (UGG to UGT) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_3_gr          { .pres_delay_us = 10000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 90,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 15 }

/// 48_4_gr configuration (UGG to UGT) (see #bap_uc_cfg_t)
#define BAP_UC_CFG_48_4_gr          { .pres_delay_us = 10000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 120,                      \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 20 }

/// @} BAP_UC_CFG_GAMING

#endif // BAP_UC_CFG_H_
