/**
 ****************************************************************************************
 *
 * @file bap_bc_cfg.h
 *
 * @brief Basic Audio Profile - Broadcast Audio Stream Configuration Settings
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BAP_BC_CFG_H_
#define BAP_BC_CFG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BAP_BC_CFG Audio Stream configuration
 * @ingroup BAP_BC
 * @brief Broadcast Audio Stream configuration settings
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_CFG_LATENCY Low Latency
 * @ingroup BAP_BC_CFG
 * @brief Settings for low latency audio data
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_CFG_RELIABILITY High-Reliability
 * @ingroup BAP_BC_CFG
 * @brief Settings for high-reliability audio data
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup BAP_BC_CFG_GAMING Gaming
 * @ingroup BAP_BC_CFG
 * @brief Settings for Gaming audio data
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bap.h"                // Basic Audio Profile Definitions

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Audio stream configuration settings structure
typedef struct bap_bc_cfg
{
    /// Presentation delay in microseconds
    uint16_t pres_delay_us;
    /// SDU interval in microseconds
    uint16_t sdu_intv_us;
    /// Sampling frequency
    uint8_t samp_freq;
    /// Framing
    uint8_t framing;
    /// Max SDU size
    uint16_t max_sdu;
    /// Number of retransmission (RTN)
    uint8_t rtn;
    /// Maximum transport latency in milliseconds
    uint8_t max_tlatency_ms;
} bap_bc_cfg_t;

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup BAP_BC_CFG_LATENCY
/// @{

/// 8_1_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_8_1_1            { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_8000HZ,    \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 26,                            \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 8 }

/// 8_2_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_8_2_1            { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_8000HZ,    \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 30,                            \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms =  10 }

/// 16_1_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_16_1_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_16000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 30,                            \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 8 }

/// 16_2_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_16_2_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_16000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 40,                            \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 10 }

/// 24_1_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_24_1_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_24000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 45,                            \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 8 }

/// 24_2_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_24_2_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_24000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 60,                            \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 10 }

/// 32_1_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_32_1_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 60,                            \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 8 }

/// 32_2_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_32_2_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 80,                            \
                                      .rtn = 2,                                 \
                                      .max_tlatency_ms = 10 }

/// 441_1_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_441_1_1          { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 8163,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_44100HZ,   \
                                      .framing = ISO_FRAMED_MODE,               \
                                      .max_sdu = 97,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 24 }

/// 441_2_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_441_2_1          { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10884,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_44100HZ,   \
                                      .framing = ISO_FRAMED_MODE,               \
                                      .max_sdu = 130,                           \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 31 }

/// 48_1_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_1_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 75,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 15 }

/// 48_2_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_2_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 100,                           \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 20 }

/// 48_3_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_3_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 90,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 15 }

/// 48_4_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_4_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 120,                           \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 20 }

/// 48_5_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_5_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 117,                           \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 15 }

/// 48_6_1 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_6_1           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 155,                           \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 20 }

/// @} BAP_BC_CFG_LATENCY

/// @addtogroup BAP_BC_CFG_RELIABILITY
/// @{

/// 8_1_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_8_1_2            { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_8000HZ,    \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 26,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 45 }

/// 8_2_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_8_2_2            { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_8000HZ,    \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 30,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms =  60 }

/// 16_1_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_16_1_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_16000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 30,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 45 }

/// 16_2_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_16_2_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_16000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 40,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 60 }

/// 24_1_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_24_1_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_24000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 45,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 45 }

/// 24_2_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_24_2_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_24000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 60,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 60 }

/// 32_1_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_32_1_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 60,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 45 }

/// 32_2_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_32_2_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_32000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 80,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 60 }

/// 441_1_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_441_1_2          { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 8163,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_44100HZ,   \
                                      .framing = ISO_FRAMED_MODE,               \
                                      .max_sdu = 97,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 54 }

/// 441_2_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_441_2_2          { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10884,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_44100HZ,   \
                                      .framing = ISO_FRAMED_MODE,               \
                                      .max_sdu = 130,                           \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 60 }

/// 48_1_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_1_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 75,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 50 }

/// 48_2_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_2_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 100,                           \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 65 }

/// 48_3_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_3_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 90,                            \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 50 }

/// 48_4_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_4_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 120,                           \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 65 }

/// 48_5_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_5_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 117,                           \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 50 }

/// 48_6_2 configuration (see #bap_bc_cfg_t)
#define BAP_BC_CFG_48_6_2           { .pres_delay_us = 40000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu = 155,                           \
                                      .rtn = 4,                                 \
                                      .max_tlatency_ms = 65 }

/// @} BAP_BC_CFG_RELIABILITY

/// @addtogroup BAP_BC_CFG_GAMING
/// @{

/// 48_1_g configuration (see #bap_uc_cfg_t)
#define BAP_BC_CFG_48_1_g           { .pres_delay_us = 10000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 75,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 8 }

/// 48_2_g configuration (see #bap_uc_cfg_t)
#define BAP_BC_CFG_48_2_g           { .pres_delay_us = 10000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 100,                      \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 10 }

/// 48_3_g configuration (see #bap_uc_cfg_t)
#define BAP_BC_CFG_48_3_g           { .pres_delay_us = 10000,                   \
                                      .sdu_intv_us = 7500,                      \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 90,                       \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 8 }

/// 48_4_g configuration (see #bap_uc_cfg_t)
#define BAP_BC_CFG_48_4_g           { .pres_delay_us = 10000,                   \
                                      .sdu_intv_us = 10000,                     \
                                      .samp_freq = BAP_SAMPLING_FREQ_48000HZ,   \
                                      .framing = ISO_UNFRAMED_MODE,             \
                                      .max_sdu_size = 120,                      \
                                      .rtn = 1,                                 \
                                      .max_tlatency_ms = 10 }

/// @} BAP_BC_CFG_GAMING

#endif // BAP_BC_CFG_H_
