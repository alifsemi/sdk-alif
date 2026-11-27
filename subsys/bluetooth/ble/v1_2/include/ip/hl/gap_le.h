/**
 ****************************************************************************************
 *
 * @file gap_le.h
 *
 * @brief Header file - Generic Access Profile (GAP)
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAP_LE_H_
#define GAP_LE_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gap.h"

/**
 * @defgroup GAP_DEFAULT_API Default Type and Enumeration Definition
 * @ingroup GAP_API
 * @{
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/**
 * @addtogroup GAP_ENUM_API
 * @{
 */

/// LE Channel map length
#define GAP_LE_CHANNEL_MAP_LEN (0x05u)
/// LE Feature Flags Length
#define GAP_LE_FEATURES_LEN (0x08u)
/// Legacy ADV Data length
#define GAP_ADV_DATA_LEN (0x1Fu)
/// Legacy Scan Response length
#define GAP_SCAN_RSP_DATA_LEN (0x1Fu)

// These values represent what the device supports
/// Minimum data packet transmission size (in octets)
#define GAP_LE_MIN_OCTETS (27u)
/// Minimum data packet transmission duration (in us)
#define GAP_LE_MIN_TIME (328u)
/// Maximum data packet transmission size (in octets)
#define GAP_LE_MAX_OCTETS (251u)
/// Maximum data packet transmission duration (in us)
#define GAP_LE_MAX_TIME (17040u)

/// Connection interval min (N*1.250ms)
#define GAP_LE_CON_INTERVAL_MIN (6u)
/// Connection interval Max (N*1.250ms)
#define GAP_LE_CON_INTERVAL_MAX (3200u)
/// Connection latency min (N*cnx evt)
#define GAP_LE_CON_LATENCY_MIN (0u)
/// Connection latency Max (N*cnx evt)
#define GAP_LE_CON_LATENCY_MAX (499u)
/// Supervision TO min (N*10ms)
#define GAP_LE_CON_SUP_TO_MIN (10u)
/// Supervision TO Max (N*10ms)
#define GAP_LE_CON_SUP_TO_MAX (3200u)

/// Length of resolvable random address prand part
#define GAP_ADDR_PRAND_LEN (3u)
/// Length of resolvable random address hash part
#define GAP_ADDR_HASH_LEN (3u)

/// Number of bytes needed for a bit field indicated presence of a given Advertising Flag value
/// in the Advertising or the Scan Response data
/// Advertising Flags is a 8-bit value, hence 256 value are possible
/// -> 256 / 8 bytes = 32 bytes are needed
#define GAP_AD_TYPE_BITFIELD_BYTES (32u)

/*
 * DEFINES - Optional for BLE application usage
 ****************************************************************************************
 */

/// Central idle timer
/// TGAP(conn_pause_central)
/// recommended value: 1 s: (100 for kernel timer)
#define GAP_TMR_CONN_PAUSE_CT (0x0064u)

/// Minimum time upon connection establishment before the peripheral
/// starts a connection update procedure: TGAP(conn_pause_peripheral)
/// recommended value: 5 s: (500 for kernel timer)
#define GAP_TMR_CONN_PAUSE_PH (0x01F4u)

/// Minimum time to perform scanning when user initiated
/// TGAP(scan_fast_period)
/// recommended value: 30.72 s: (3072 for kernel timer)
#define GAP_TMR_SCAN_FAST_PERIOD (0x0C00u)

/// Minimum time to perform advertising when user initiated
/// TGAP(adv_fast_period)
/// recommended value: 30 s: (3000 for kernel timer)
#define GAP_TMR_ADV_FAST_PERIOD (0x0BB8u)

/// Scan interval used during Link Layer Scanning State when
/// performing the Limited Discovery procedure
/// TGAP(lim_disc_scan_int)
/// recommended value: 11.25ms; (18 decimal)
#define GAP_LIM_DISC_SCAN_INT (0x0012u)

/// Scan interval in any discovery or connection establishment
/// procedure when user initiated: TGAP(scan_fast_interval)
/// recommended value: 30 to 60 ms; N * 0.625
#define GAP_SCAN_FAST_INTV (0x0030u)

/// Scan window in any discovery or connection establishment
/// procedure when user initiated: TGAP(scan_fast_window)
/// recommended value: 30 ms; N * 0.625
#define GAP_SCAN_FAST_WIND (0x0030u)

/// Scan interval in any discovery or connection establishment
/// procedure when background scanning: TGAP(scan_slow_interval1)
/// recommended value: 1.28 s : 0x00CD (205); N * 0.625
#define GAP_SCAN_SLOW_INTV1 (0x00CDu)

/// Scan interval in any discovery or connection establishment
/// procedure when background scanning: TGAP(scan_slow_interval2)
/// recommended value: 2.56 s : 0x019A (410); N * 0.625
#define GAP_SCAN_SLOW_INTV2 (0x019Au)

/// Scan window in any discovery or connection establishment
/// procedure when background scanning: TGAP(scan_slow_window1)
/// recommended value: 11.25 ms : 0x0012 (18); N * 0.625
#define GAP_SCAN_SLOW_WIND1 (0x0012u)

/// Scan window in any discovery or connection establishment
/// procedure when background scanning: TGAP(scan_slow_window2)
/// recommended value: 22.5 ms : 0x0024 (36); N * 0.625
#define GAP_SCAN_SLOW_WIND2 (0x0024u)

/// Minimum to maximum advertisement interval in any discoverable
/// or connectable mode when user initiated: TGAP(adv_fast_interval1)
/// recommended value: 30 to 60 ms; N * 0.625
#define GAP_ADV_FAST_INTV1 (0x0030u)

/// Minimum to maximum advertisement interval in any discoverable
/// or connectable mode when user initiated: TGAP(adv_fast_interval2)
/// recommended value: 100 to 150 ms; N * 0.625
#define GAP_ADV_FAST_INTV2 (0x0064u)

/// Minimum to maximum advertisement interval in any discoverable or
/// connectable mode when background advertising: TGAP(adv_slow_interval)
/// recommended value: 1 to 1.2 s : 0x00B0 (176); N * 0.625
#define GAP_ADV_SLOW_INTV (0x00B0u)

/// Minimum connection interval upon any connection
/// establishment: TGAP(initial_conn_interval)
/// recommended value: 30 to 50 ms ; N * 1.25 ms
#define GAP_INIT_CONN_MIN_INTV (0x0018u)
/// Maximum connection interval upon any connection
#define GAP_INIT_CONN_MAX_INTV (0x0028u)

/// Connection supervision timeout
/// recommended value: 20s
#define GAP_CONN_SUPERV_TIMEOUT (0x07D0u)

/// Default Peripheral Preferred Connection Parameter maximum connection interval
#define GAP_PPCP_CONN_INTV_MAX (0x0064u)
/// Default Peripheral Preferred Connection Parameter minimum connection interval
#define GAP_PPCP_CONN_INTV_MIN (0x00C8u)
/// Default Peripheral Preferred Connection Parameter - slave latency
#define GAP_PPCP_SLAVE_LATENCY (0x0000u)
/// Default Peripheral Preferred Connection Parameter - supervision timeout
#define GAP_PPCP_STO_MULT (0x07D0u)

/// Acceptable encryption key size - strict access
#define GAP_SEC_ENC_KEY_SIZE (0x10u)

/*
 * Macros
 ****************************************************************************************
 */
/// Set bit in AD Type bit field
#define GAP_AD_TYPE_SET_BIT(bitfield, adv_flag)                             \
                bitfield[adv_flag / 8u] |= CO_BIT(adv_flag % 8u)

/// Check presence of bit in AD Type bit field
#define GAP_AD_TYPE_CHECK_BIT(bitfield, adv_flag)                           \
                (bitfield[adv_flag / 8u] & CO_BIT(adv_flag % 8u))

/// Size of advertising data counting advertising type header
#define GAP_AD_TOTAL_LEN(data_size)      ((data_size) + 1u)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of supported BLE Features
enum gap_le_feature
{
    // Byte 0
    GAP_LE_FEAT_ENC = 0u,
    GAP_LE_FEAT_CON_PARAM_REQ_PROC = 1u,
    GAP_LE_FEAT_EXT_REJ_IND = 2u,
    GAP_LE_FEAT_SLAVE_INIT_FEAT_EXCHG = 3u,
    GAP_LE_FEAT_PING = 4u,
    GAP_LE_FEAT_DATA_PKT_LEN_EXT = 5u,
    GAP_LE_FEAT_LL_PRIVACY = 6u,
    GAP_LE_FEAT_EXT_SCAN_FILT_POLICY = 7u,
    // Byte 1
    GAP_LE_FEAT_2M_PHY = 8u,
    GAP_LE_FEAT_STABLE_MOD_IDX_TX = 9u,
    GAP_LE_FEAT_STABLE_MOD_IDX_RX = 10u,
    GAP_LE_FEAT_CODED_PHY = 11u,
    GAP_LE_FEAT_EXT_ADV = 12u,
    GAP_LE_FEAT_PER_ADV = 13u,
    GAP_LE_FEAT_CHAN_SEL_ALGO_2 = 14u,
    GAP_LE_FEAT_PWR_CLASS_1 = 15u,
    // Byte 2
    GAP_LE_FEAT_MIN_NUM_USED_CHAN_PROC = 16u,
    GAP_LE_FEAT_CON_CTE_REQ = 17u,
    GAP_LE_FEAT_CON_CTE_RSP = 18u,
    GAP_LE_FEAT_CONLESS_CTE_TX = 19u,
    GAP_LE_FEAT_CONLESS_CTE_RX = 20u,
    GAP_LE_FEAT_AOD = 21u,
    GAP_LE_FEAT_AOA = 22u,
    GAP_LE_FEAT_CTE_RX = 23u,
    // Byte 3
    GAP_LE_FEAT_PER_ADV_SYNC_TRANSF_TX = 24u,
    GAP_LE_FEAT_PER_ADV_SYNC_TRANSF_RX = 25u,
    GAP_LE_FEAT_SLEEP_CLK_ACC_UPD = 26u,
    GAP_LE_FEAT_PUB_KEY_VALID = 27u,
    GAP_LE_FEAT_CON_ISO_STREAM_MASTER = 28u,
    GAP_LE_FEAT_CON_ISO_STREAM_SLAVE = 29u,
    GAP_LE_FEAT_ISO_BROADCASTER = 30u,
    GAP_LE_FEAT_SYNCED_RECEIVER = 31u,
    // Byte 4
    GAP_LE_FEAT_ISO_CHANNELS_HOST_SUPPORT = 32u,
    GAP_LE_FEAT_POWER_CONTROL_REQ = 33u,
    GAP_LE_FEAT_POWER_CHANGE_IND = 34u,
    GAP_LE_FEAT_PATH_LOSS_MONITORING = 35u,

    GAP_LE_FEAT_MAX,
};

/// TK Type
enum gap_le_tk_type
{
    ///  TK get from out of band method
    GAP_TK_OOB = 0x00u,
    /// TK generated and shall be displayed by local device
    GAP_TK_DISPLAY,
    /// TK shall be entered by user using device keyboard
    GAP_TK_KEY_ENTRY
};

/// Low Energy OOB Data Present Flag Values
enum gap_le_oob_flag
{
    /// OOB Data not present
    GAP_OOB_AUTH_DATA_NOT_PRESENT = 0x00u,
    /// OOB data present
    GAP_OOB_AUTH_DATA_PRESENT,
    GAP_OOB_AUTH_DATA_LAST
};

/// Random Address type
enum gap_le_random_addr_type
{
    /// Static random address - 11 (MSB->LSB)
    GAP_BD_ADDR_STATIC = 0xC0u,
    /// Private non resolvable address - 00 (MSB->LSB)
    GAP_BD_ADDR_NON_RSLV = 0x00u,
    /// Private resolvable address - 01 (MSB->LSB)
    GAP_BD_ADDR_RSLV = 0x40u,
};

/// Bit field use to select the preferred TX or RX LE PHY. 0 means no preferences
enum gap_le_phy_bf
{
    /// No preferred PHY
    GAP_PHY_ANY = 0x00,
    /// LE 1M PHY preferred for an active link
    GAP_PHY_LE_1MBPS = (1u << 0),
    /// LE 2M PHY preferred for an active link
    GAP_PHY_LE_2MBPS = (1u << 1),
    /// LE Coded PHY preferred for an active link
    GAP_PHY_LE_CODED = (1u << 2),
};

/// Enumeration of TX/RX PHY values
enum gap_le_phy_val
{
    /// LE 1M PHY (TX or RX)
    GAP_PHY_1MBPS = 1u,
    /// LE 2M PHY (TX or RX)
    GAP_PHY_2MBPS = 2u,
    /// LE Coded PHY (RX Only)
    GAP_PHY_CODED = 3u,
    /// LE Coded PHY with S=8 data coding (TX Only)
    GAP_PHY_125KBPS = 3u,
    /// LE Coded PHY with S=2 data coding (TX Only)
    GAP_PHY_500KBPS = 4u,
};

/// Modulation index
enum gap_le_modulation_idx
{
    /// Assume transmitter will have a standard modulation index
    GAP_MODULATION_STANDARD = 0u,
    /// Assume transmitter will have a stable modulation index
    GAP_MODULATION_STABLE,
};

/// Packet Payload type for test mode
enum gap_le_packet_payload_type
{
    /// PRBS9 sequence "11111111100000111101..." (in transmission order)
    GAP_PKT_PLD_PRBS9 = 0u,
    /// Repeated "11110000" (in transmission order)
    GAP_PKT_PLD_REPEATED_11110000,
    /// Repeated "10101010" (in transmission order)
    GAP_PKT_PLD_REPEATED_10101010,
    /// PRBS15 sequence
    GAP_PKT_PLD_PRBS15,
    /// Repeated "11111111" (in transmission order) sequence
    GAP_PKT_PLD_REPEATED_11111111,
    /// Repeated "00000000" (in transmission order) sequence
    GAP_PKT_PLD_REPEATED_00000000,
    /// Repeated "00001111" (in transmission order) sequence
    GAP_PKT_PLD_REPEATED_00001111,
    /// Repeated "01010101" (in transmission order) sequence
    GAP_PKT_PLD_REPEATED_01010101,
};

/// Constant Tone Extension type
enum gap_le_cte_type
{
    /// AoA Constant Tone Extension Response
    GAP_CTE_AOA = 0x00u,
    /// AoD Constant Tone Extension Response with 1 us slots
    GAP_CTE_AOD_1US_SLOT = 0x01u,
    /// AoD Constant Tone Extension Response with 2 us slots
    GAP_CTE_AOD_2US_SLOT = 0x02u,
};

/// Constant Tone Extension type bit field
enum gap_le_cte_type_bf
{
    /// Allow AoA Constant Tone Extension Response
    GAP_CTE_AOA_BIT = (1u << 0),
    GAP_CTE_AOA_POS = 0u,
    /// Allow AoD Constant Tone Extension Response with 1 us slots
    GAP_CTE_AOD_1US_SLOT_BIT = (1u << 1),
    GAP_CTE_AOD_1US_SLOT_POS = 1u,
    /// Allow AoD Constant Tone Extension Response with 2 us slots
    GAP_CTE_AOD_2US_SLOT_BIT = (1u << 2),
    GAP_CTE_AOD_2US_SLOT_POS = 2u,
};

/// Privacy mode used for entry into the resolving list
enum gap_le_privacy_mode
{
    ///Network privacy mode
    GAP_PRIV_TYPE_NETWORK = 0x00u,
    ///Device privacy mode
    GAP_PRIV_TYPE_DEVICE = 0x01u,
};


/// @} GAP_ENUM_API

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/**
 * @defgroup GAP_STRUCT_API Structures
 * @ingroup GAP_DEFAULT_API
 * @{
 */

///Random number structure
/*@TRACE*/
typedef struct
{
    ///8-byte array for random number
    uint8_t nb[GAP_RAND_NB_LEN];
} gap_le_random_nb_t;

/// P256 Public key data format
typedef struct
{
    /// X coordinate of the key
    uint8_t x[GAP_P256_KEY_LEN];
    /// X coordinate of the key
    uint8_t y[GAP_P256_KEY_LEN];
} gap_le_public_key_t;

/// P256 DH-key data format
typedef struct
{
    /// Value of the DH-key
    uint8_t key[GAP_P256_KEY_LEN];
} gap_le_dh_key_t;

#if (HL_DEPRECATED_LIST)
/// Resolving list device information
/*@TRACE*/
typedef struct
{
    /// Device identity
    gap_bdaddr_t addr;
    /// Privacy Mode (see #gap_le_privacy_mode enumeration)
    uint8_t priv_mode;
    /// Peer IRK
    uint8_t peer_irk[GAP_KEY_LEN];
    /// Local IRK
    uint8_t local_irk[GAP_KEY_LEN];
} gap_le_ral_info_t;
#endif // (HL_DEPRECATED_LIST)

/// AES Cipher result
typedef struct
{
    /// AES cipher result value
    uint8_t value[GAP_AES_LEN];
} gap_le_aes_result_t;

/// Random number generated with an AES block
typedef struct
{
    /// AES cipher result value
    uint8_t value[GAP_AES_LEN];
} gap_le_aes_random_nb_t;

/// I/Q sample
/*@TRACE*/
typedef struct
{
    /// I sample
    int8_t i;
    /// Q sample
    int8_t q;
} gap_le_iq_sample_t;

#if (BLE_SUBRATING)
/// Subrating requirements structure
typedef struct
{
    /// Minimum subrate factor\n
    /// Range: 0x0001 to 0x01F4
    uint16_t subrate_min;
    /// Maximum subrate factor\n
    /// Range: 0x0001 to 0x01F4
    uint16_t subrate_max;
    /// Maximal peripheral latency\n
    /// Range: 0x0000 to 0x01F3
    uint16_t max_latency;
    /// Continuation number\n
    /// Range: 0x0000 to 0x01F3
    uint16_t continuation_number;
    /// Supervision timeout (in multiple of 10ms)\n
    /// Range: 0x000A (100ms) to 0x0C80 (32s)
    uint16_t supervision_to_10ms;
} gap_le_subrate_req_t;
#endif // (BLE_SUBRATING)

/// @} GAP_STRUCT_API
/// @} GAP_DEFAULT_API

#endif // GAP_LE_H_
