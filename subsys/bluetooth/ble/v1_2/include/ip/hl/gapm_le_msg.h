/**
 ****************************************************************************************
 *
 * @file gapm_le_msg.h
 *
 * @brief Generic Access Profile Manager Message API. Low Energy
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_LE_MSG_H_
#define GAPM_LE_MSG_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm_msg.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "gapm_le_scan.h"
#include "gapm_le_init.h"
#include "gapm_le_per_sync.h"
#include "gapm_le_test.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPM_MSG_STRUCT_API Message Structures
/// @ingroup GAPM_MSG_API
/// @{

/// Set new IRK
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_SET_IRK: Set device Identity Resolving key
    uint8_t operation;
    /// Device IRK used for resolvable random BD address generation (LSB first)
    gap_sec_key_t irk;
} gapm_le_set_irk_cmd_t;

/// Set device channel map
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_SET_CHANNEL_MAP: Set device channel map.
    uint8_t operation;
    /// Channel map
    struct le_ch_map ch_map;
} gapm_le_set_channel_map_cmd_t;

#if (BLE_GAPC)
/// Parameters of #GAPM_LE_CONFIGURE_SECURITY_LEVEL_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_CONFIGURE_SECURITY_LEVEL: Configure security level required for LE connections.
    uint8_t operation;
    /// LE connection security requirements (minimum security level - see #gap_sec_req enumeration)
    uint8_t sec_req_level;
} gapm_le_configure_security_level_cmd_t;
#endif // (BLE_GAPC)

/// Advertising channel Tx power level indication event
/*@TRACE*/
typedef struct
{
    /// Advertising channel Tx power level
    int8_t power_lvl;
} gapm_le_adv_tx_power_ind_t;

/// Indication containing controller antenna information
/*@TRACE*/
typedef struct
{
    /// Supported switching sampling rates bit field (see #gapm_le_per_sync_switch_sampling_rate enumeration)
    uint8_t supp_switching_sampl_rates;
    /// Number of antennae
    uint8_t antennae_num;
    /// Max length of switching pattern (number of antenna IDs in the pattern)
    uint8_t max_switching_pattern_len;
    /// Max CTE length
    uint8_t max_cte_len;
} gapm_le_antenna_info_ind_t;

/// Resolving Address indication event
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    uint8_t operation;
    /// Resolving List address
    gap_bdaddr_t addr;
} gapm_le_ral_addr_ind_t;

/// Resolve Address command
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_RESOLVE_ADDR: Resolve device address
    uint8_t operation;
    /// Number of provided IRK (sahlle be > 0)
    uint8_t nb_key;
    /// Resolvable random address to solve
    gap_addr_t addr;
    /// Array of IRK used for address resolution (MSB -> LSB)
    gap_sec_key_t irk[__ARRAY_EMPTY];
} gapm_le_resolve_addr_cmd_t;

/// Indicate that resolvable random address has been solved
/*@TRACE*/
typedef struct
{
    /// Resolvable random address solved
    gap_addr_t addr;
    /// IRK that correctly solved the random address
    gap_sec_key_t irk;
} gapm_le_addr_resolved_ind_t;

/// Generate a random address.
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_GENERATE_RANDOM_ADDR: Generate a random address
    uint8_t operation;
    /// Random address type #gap_le_random_addr_type
    ///  - #GAP_BD_ADDR_STATIC: Static random address
    ///  - #GAP_BD_ADDR_NON_RSLV: Private non resolvable address
    ///  - #GAP_BD_ADDR_RSLV: Private resolvable address
    uint8_t rnd_type;
} gapm_le_generate_random_addr_cmd_t;

/// Parameters of the @ref GAPM_LE_USE_ENCRYPTION_BLOCK_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    /// (shall be #GAPM_LE_USE_ENCRYPTION_BLOCK)
    uint8_t operation;
    /// True to cipher; False to de-cipher
    bool cipher;
    /// Operand 1
    uint8_t operand_1[GAP_KEY_LEN];
    /// Operand 2
    uint8_t operand_2[GAP_KEY_LEN];
} gapm_le_use_encryption_block_cmd_t;

/// Parameters of the @ref GAPM_LE_ENCRYPTION_RESULT_IND message
/*@TRACE*/
typedef struct
{
    /// Result (16 bytes)
    uint8_t result[GAP_KEY_LEN];
} gapm_le_encryption_result_ind_t;

/// Parameters of the @ref GAPM_LE_COMPUTE_DH_KEY_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    /// (shall be #GAPM_LE_COMPUTE_DH_KEY)
    uint8_t operation;
    /// peer public key
    gap_le_public_key_t pub_key;
} gapm_le_compute_dh_key_cmd_t;

/// Parameters of the @ref GAPM_LE_DH_KEY_IND message
/*@TRACE*/
typedef struct
{
    /// Result (32 bytes)
    uint8_t result[GAP_P256_KEY_LEN];
} gapm_le_dh_key_ind_t;

/// Parameters of the @ref GAPM_LE_GET_PUBLIC_KEY_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    /// (shall be #GAPM_LE_GET_PUBLIC_KEY)
    uint8_t operation;
} gapm_le_get_public_key_cmd_t;

/// Parameters of the @ref GAPM_LE_PUBLIC_KEY_IND message
/*@TRACE*/
typedef struct
{
    /// X coordinate
    uint8_t pub_key_x[GAP_P256_KEY_LEN];
    /// Y coordinate
    uint8_t pub_key_y[GAP_P256_KEY_LEN];
} gapm_le_public_key_ind_t;

/// Parameters of the @ref GAPM_LE_OOB_DATA_IND message
/*@TRACE*/
typedef struct
{
    /// Generated OOB data
    gap_oob_t oob;
} gapm_le_oob_data_ind_t;

/// Parameters of the @ref GAPM_LE_GENERATE_RANDOM_NB_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    /// (shall be #GAPM_LE_GENERATE_RANDOM_NB)
    uint8_t operation;
} gapm_le_generate_random_nb_cmd_t;

/// Parameters of the @ref GAPM_LE_RANDOM_NB_IND message
/*@TRACE*/
typedef struct
{
    /// Generation Random Number (16 bytes)
    uint8_t randnb[GAP_AES_LEN];
} gapm_le_random_nb_ind_t;

/// Indicates suggested default data length
/*@TRACE*/
typedef struct
{
    ///Host's suggested value for the Controller's maximum transmitted number of payload octets
    uint16_t suggted_max_tx_octets;
    ///Host's suggested value for the Controller's maximum packet transmission time
    uint16_t suggted_max_tx_time;
} gapm_le_suggested_default_data_len_ind_t;

/// Indicates maximum data length
/*@TRACE*/
typedef struct
{
    ///Maximum number of payload octets that the local Controller supports for transmission
    uint16_t suppted_max_tx_octets;
    ///Maximum time, in microseconds, that the local Controller supports for transmission
    uint16_t suppted_max_tx_time;
    ///Maximum number of payload octets that the local Controller supports for reception
    uint16_t suppted_max_rx_octets;
    ///Maximum time, in microseconds, that the local Controller supports for reception
    uint16_t suppted_max_rx_time;
} gapm_le_max_data_len_ind_t;

/// Control LE Test Mode command
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_STOP_TEST: Stop Test mode
    ///  - #GAPM_LE_START_TEST_RX: Start RX Test Mode
    ///  - #GAPM_LE_START_TEST_TX: Start TX Test Mode
    uint8_t operation;
    /// Tx or Rx Channel (Range 0x00 to 0x27)
    uint8_t channel;
    /// Length in bytes of payload data in each packet (only valid for TX mode, range 0x00-0xFF)
    uint8_t tx_data_length;
    /// Packet Payload type (only valid for TX mode see #gap_le_packet_payload_type enumeration)
    uint8_t tx_pkt_payload;
    /// Test PHY rate (see #gap_le_phy_val enumeration)
    uint8_t phy;
    /// Modulation Index (only valid for RX mode see #gap_le_modulation_idx enumeration)
    uint8_t modulation_idx;
    /// CTE length (in 8us unit) (Expected for RX Mode)
    uint8_t cte_len;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us) (Expected for TX Mode)
    uint8_t cte_type;
    /// Slot durations (only valid for RX mode)
    uint8_t slot_dur;
    /// Transmit power level in dBm (0x7E: minimum | 0x7F: maximum | range: -127 to +20)
    int8_t tx_pwr_lvl;
    /// Length of switching pattern (number of antenna IDs in the pattern)
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[__ARRAY_EMPTY];
} gapm_le_control_test_mode_t;

/// Parameters of #GAPM_LE_TEST_END_IND
typedef struct
{
    /// Number of received packets
    uint16_t nb_packet_received;
} gapm_le_test_end_ind_t;

/// Indicate reception of a IQ Report event over a direct test mode.
/*@TRACE*/
typedef struct
{
    /// Data channel index
    uint8_t channel_idx;
    /// RSSI (in 0.1 dBm)
    int16_t rssi;
    /// RSSI antenna ID
    uint8_t rssi_antenna_id;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us) (see #gap_le_cte_type enumeration)
    uint8_t cte_type;
    /// Slot durations (1: 1us | 2: 2us)
    uint8_t slot_dur;
    /// Packet status
    uint8_t pkt_status;
    /// Periodic Adv Event Counter
    uint16_t pa_evt_cnt;
    /// Number of samples
    uint8_t nb_samples;
    /// I/Q sample
    gap_le_iq_sample_t sample[__ARRAY_EMPTY];
} gapm_le_test_iq_report_ind_t;

/// Create an advertising activity command
/// Extends #gapm_activity_create_cmd_t
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_CREATE_ADV: Create advertising activity
    uint8_t operation;
    /// Own address type (see #gapm_le_own_addr enumeration)
    uint8_t own_addr_type;
    /// Advertising type (see #gapm_le_adv_type enumeration)
    uint8_t type;
    /// Advertising parameters (optional, shall be present only if operation is GAPM_LE_CREATE_ADV)
    gapm_le_adv_create_param_t adv_param;
    /// Configuration for secondary advertising (valid only if advertising type is
    /// GAPM_ADV_TYPE_EXTENDED or GAPM_ADV_TYPE_PERIODIC)
    gapm_le_adv_secondary_cfg_t second_cfg;
    #if (BLE_PER_ADV)
    /// Configuration for periodic advertising (valid only if advertising type is
    /// GAPM_ADV_TYPE_PERIODIC)
    gapm_le_adv_periodic_cfg_t period_cfg;
    /// Configuration for  constant tone extension (valid only if advertising type is
    /// GAPM_ADV_TYPE_PERIODIC)
    gapm_le_adv_cte_cfg_t cte_cfg;
    /// Length of switching pattern (number of antenna IDs in the pattern)
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[__ARRAY_EMPTY];
    #endif // (BLE_PER_ADV)
} gapm_le_activity_create_adv_cmd_t;

/// Set advertising, scan response or periodic advertising data command
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_SET_ADV_DATA: Set advertising data
    ///  - #GAPM_LE_SET_SCAN_RESPONSE_DATA: Set scan response data
    ///  - #GAPM_LE_SET_PER_ADV_DATA: Set periodic advertising data
    ///  - #GAPM_LE_SET_PER_ADV_DATA_FAKE: Set unchanged periodic advertising data
    uint8_t operation;
    /// Activity identifier
    uint8_t actv_idx;
    /// Data length
    /// Meaningless for set unchanged periodic advertising data case
    uint16_t length;
    /// Data
    /// Meaningless for set unchanged periodic advertising data case
    uint8_t data[__ARRAY_EMPTY];
} gapm_le_set_adv_data_cmd_t;



/// Indicate reception of scan request
/*@TRACE*/
typedef struct
{
    /// Activity identifier
    uint8_t actv_idx;
    /// Transmitter device address
    gap_bdaddr_t trans_addr;
} gapm_le_scan_request_ind_t;

/// Indicate reception of advertising, scan response or periodic advertising data
/*@TRACE*/
typedef struct
{
    /// Activity identifier
    uint8_t actv_idx;
    /// Bit field providing information about the received report (see #gapm_le_adv_report_info_bf enumeration)
    uint8_t info;
    /// Transmitter device address
    gap_bdaddr_t trans_addr;
    /// Target address (in case of a directed advertising report)
    gap_bdaddr_t target_addr;
    /// TX power (in dBm)
    int8_t tx_pwr;
    /// RSSI (between -127 and +20 dBm)
    int8_t rssi;
    /// Primary PHY on which advertising report has been received (see #gapm_le_phy_type enumeration)
    uint8_t phy_prim;
    /// Secondary PHY on which advertising report has been received (see #gapm_le_phy_type enumeration)
    uint8_t phy_second;
    /// Advertising SID
    /// Valid only for periodic advertising report
    uint8_t adv_sid;
    /// Periodic advertising interval (in unit of 1.25ms, min is 7.5ms)
    /// Valid only for periodic advertising report
    uint16_t period_adv_intv;
    /// Report length
    uint16_t length;
    /// Report
    uint8_t data[__ARRAY_EMPTY];
} gapm_le_adv_report_ind_t;

#if (BLE_BIS && HL_LE_OBSERVER)
/// Indicate reception of periodic advertising report that contains BIGInfo data
/*@TRACE*/
typedef struct
{
    /// Activity identifier
    uint8_t actv_idx;
    /// Big Info report
    gapm_le_big_info_t report;
} gapm_le_big_info_ind_t;
#endif // (BLE_BIS && HL_LE_OBSERVER)

/// Indicate that synchronization has been established with a periodic advertiser
/*@TRACE*/
typedef struct
{
    /// Activity identifier
    uint8_t actv_idx;
    /// PHY on which synchronization has been established (#gap_le_phy_val)
    uint8_t phy;
    /// Periodic advertising interval (in unit of 1.25ms, min is 7.5ms)
    uint16_t intv;
    /// Advertising SID
    uint8_t adv_sid;
    /// Advertiser clock accuracy (see #gapm_clock_accuracy enumeration)
    uint8_t clk_acc;
    /// Advertiser address
    gap_bdaddr_t addr;
    /// Only valid for a Periodic Advertising Sync Transfer, else ignore
    uint16_t serv_data;
} gapm_le_per_sync_established_ind_t;

/// Read local or peer address
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_GET_RAL_LOCAL_RPA: Get resolving local address
    ///  - #GAPM_LE_GET_RAL_PEER_RPA: Get resolving peer address
    uint8_t operation;
    /// Peer device identity
    gap_bdaddr_t peer_identity;
} gapm_le_get_ral_addr_cmd_t;

#if (HL_DEPRECATED_LIST)
/// Set content of either filter accept list or resolving list or periodic advertiser list command (common part)
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_FILL_FAL: Set filter accept list content
    ///  - #GAPM_LE_FILL_RAL: Set resolving list content
    ///  - #GAPM_LE_FILL_PAL: Set periodic advertiser list content
    uint8_t operation;
    /// Number of entries to be added in the list. 0 means that list content has to be cleared
    uint8_t size;
} gapm_le_fill_list_cmd_t;

/// Set content of filter accept list
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_FILL_FAL: Set filter accept list content
    uint8_t operation;
    /// Number of entries to be added in the list. 0 means that list content has to be cleared
    uint8_t size;
    /// List of entries to be added in the list
    gap_bdaddr_t wl_info[__ARRAY_EMPTY];
} gapm_le_fill_fal_cmd_t;

/// Set content of resolving list command
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_FILL_RAL: Set resolving list content
    uint8_t operation;
    /// Number of entries to be added in the list. 0 means that list content has to be cleared
    uint8_t size;
    /// List of entries to be added in the list
    gap_le_ral_info_t ral_info[__ARRAY_EMPTY];
} gapm_le_fill_ral_cmd_t;

#if (BLE_PER_ADV)
/// Set content of periodic advertiser list command
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_FILL_PAL: Set periodic advertiser list content
    uint8_t operation;
    /// Number of entries to be added in the list. 0 means that list content has to be cleared
    uint8_t size;
    /// List of entries to be added in the list
    gapm_le_per_adv_bdaddr_t pal_info[__ARRAY_EMPTY];
} gapm_le_fill_pal_cmd_t;
#endif // (BLE_PER_ADV)
#endif // (HL_DEPRECATED_LIST)

/// Parameters of #GAPM_LE_LIST_UPDATE_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///     - #GAPM_LE_LIST_ADD
    ///     - #GAPM_LE_LIST_REMOVE
    ///     - #GAPM_LE_LIST_CLEAR
    uint8_t operation;
    /// List type (see #gapm_le_list_type enumeration)
    uint8_t type;
    /// Information entry
    union
    {
        /// BD address\n
        /// For operation = #GAPM_LE_LIST_ADD and type = GAPM_LE_LIST_TYPE_FAL,\n
        /// Or operation = #GAPM_LE_LIST_REMOVE and type = GAPM_LE_LIST_TYPE_FAL,\n
        /// Or operation = #GAPM_LE_LIST_REMOVE and type = GAPM_LE_LIST_TYPE_RAL,\n
        /// Or operation = #GAPM_LE_LIST_REMOVE and type = GAPM_LE_LIST_TYPE_MAL
        gap_bdaddr_t bd_addr;
        /// Resolving list entry information\n
        /// For operation = #GAPM_LE_LIST_ADD and type = GAPM_LE_LIST_TYPE_RAL
        gapm_le_list_ral_entry_t ral_entry;
        #if (BLE_PER_ADV)
        /// Periodic advertiser list entry\n
        /// For operation = #GAPM_LE_LIST_ADD and type = GAPM_LE_LIST_TYPE_PAL,\n
        /// Or operation = #GAPM_LE_LIST_REMOVE and type = GAPM_LE_LIST_TYPE_PAL,\n
        gapm_le_per_adv_bdaddr_t pal_entry;
        #endif // (BLE_PER_ADV)
    } u;
} gapm_le_list_update_cmd_t;

/// Parameters of #GAPM_LE_LIST_SET_PRIVACY_MODE_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///     - #GAPM_LE_LIST_SET_PRIVACY_MODE
    uint8_t operation;
    /// Peer identity
    gap_bdaddr_t peer_identity;
    /// Privacy mode (see #gap_le_privacy_mode enumeration)
    uint8_t privacy_mode;
} gapm_le_list_set_privacy_mode_cmd_t;

/// List Size indication event
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///     - #GAPM_LE_GET_FAL_SIZE
    ///     - #GAPM_LE_GET_RAL_SIZE
    ///     - #GAPM_LE_GET_PAL_SIZE
    uint8_t operation;
    /// List size
    uint8_t size;
} gapm_le_list_size_ind_t;

/// Maximum advertising data length indication event
/*@TRACE*/
typedef struct
{
    /// Maximum advertising data length supported by controller
    uint16_t length;
} gapm_le_max_adv_data_len_ind_t;

/// Number of available advertising sets indication event
/*@TRACE*/
typedef struct
{
    /// Number of available advertising sets
    uint8_t nb_adv_sets;
} gapm_le_nb_adv_sets_ind_t;

/// Indicate the transmit powers supported by the controller
/*@TRACE*/
typedef struct
{
    /// Minimum TX power
    int8_t min_tx_pwr;
    /// Maximum TX power
    int8_t max_tx_pwr;
} gapm_le_tx_power_ind_t;

/// Indicate the RF path compensation values
/*@TRACE*/
typedef struct
{
    /// RF TX path compensation
    int16_t tx_path_comp;
    /// RF RX path compensation
    int16_t rx_path_comp;
} gapm_le_rf_path_compensation_ind_t;

/// Control reception or not of Periodic Advertising Report in a Periodic Advertising Sync activity
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_CONTROL_PER_SYNC_REPORT: Enable / Disable reception of periodic advertising report
    uint8_t operation;
    /// Activity identifier
    uint8_t actv_idx;
    /// bit field that contains list of reports that are enabled or not (see #gapm_le_per_sync_report_en_bf enumeration)
    uint8_t report_en_bf;
} gapm_le_control_per_sync_report_cmd_t;

/// Control capturing IQ samples from the Constant Tone Extension of periodic advertising packets
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_CONTROL_PER_SYNC_IQ_REPORT: Enable / Disable IQ sampling
    uint8_t operation;
    /// Activity identifier
    uint8_t actv_idx;
    /// True to enable IQ sampling, false to disable
    uint8_t enable;
    /// Slot durations (1: 1us | 2: 2us)
    uint8_t slot_dur;
    /// Max sampled CTEs
    uint8_t max_sampl_cte;
    /// Length of switching pattern
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[__ARRAY_EMPTY];
} gapm_le_control_per_sync_iq_sampling_cmd_t;

/// Indicate reception of a IQ Report event over a periodic advertising sync activity
/*@TRACE*/
typedef struct
{
    /// Activity identifier
    uint8_t actv_idx;
    /// Data channel index
    uint8_t channel_idx;
    /// RSSI (in 0.1 dBm)
    int16_t rssi;
    /// RSSI antenna ID
    uint8_t rssi_antenna_id;
    /// CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us) (see #gap_le_cte_type enumeration)
    uint8_t cte_type;
    /// Slot durations (1: 1us | 2: 2us)
    uint8_t slot_dur;
    /// Packet status
    uint8_t pkt_status;
    /// Periodic Adv Event Counter
    uint16_t pa_evt_cnt;
    /// Number of samples
    uint8_t nb_samples;
    /// I/Q sample
    gap_le_iq_sample_t sample[__ARRAY_EMPTY];
} gapm_le_per_adv_iq_report_ind_t;

/// Control CTE transmission in a periodic advertising activity
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_CONTROL_PER_ADV_CTE_TX: Control CTE transmission in a periodic advertising activity
    uint8_t operation;
    /// Activity identifier
    uint8_t actv_idx;
    /// True to enable CTE transmission, false to disable
    uint8_t enable;
} gapm_le_control_per_adv_cte_tx_cmd_t;

/// Configure the Debug Platform I&Q Sampling generator
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_DBG_CONFIGURE_IQGEN: Configure the Debug Platform I&Q Sampling generator
    uint8_t operation;
    /// Antenna switch/sample control
    /// bit[0]: 0: up-sweep; 1: up-down sweep (internal switching mode)
    /// bit[1]: 0: 1us intervals; 1: 2us intervals (internal switching mode)
    /// bit[2]: 0: internal switching mode; 1: baseband switching mode
    uint8_t mode;
    /// Number of antenna patterns
    uint8_t nb_antenna;
    /// I/Q sample control
    gapm_le_dbg_iq_ctrl_t iq_ctrl[__ARRAY_EMPTY];
} gapm_le_dbg_configure_iqgen_cmd_t;

/// Start Channel Scan activity
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see enum #gapm_operation)
    ///  - #GAPM_LE_START_CH_SCAN: Start Channel Scan Activity
    ///  - #GAPM_LE_STOP_CH_SCAN:  Stop Channel Scan Activity
    uint8_t operation;
    /// Window duration of one channel in us
    uint32_t win_duration;
    /// Channel Scan event in us
    uint32_t scan_duration;
    /// Channel Scan interval in Time = N*1.25ms
    uint16_t intv;
    /// Channel map
    struct le_ch_map ch_map;
} gapm_le_control_ch_scan_cmd_t;

/// Initiating start parameters
typedef struct
{
    /// Initiating type (see #gapm_le_init_type enumeration)
    uint8_t type;
    /// Initiating parameters
    gapm_le_init_param_t param;
} gapm_le_init_start_param_t;

/// Activity parameters
typedef union gapm_le_activity_start_param
{
    /// Additional advertising parameters (for advertising activity)
    //@trc_union @activity_map[$parent.actv_idx] == GAPM_ACTV_TYPE_ADV
    gapm_le_adv_param_t         adv_add_param;
    /// Scan parameters (for scanning activity)
    //@trc_union @activity_map[$parent.actv_idx] == GAPM_ACTV_TYPE_SCAN
    gapm_le_scan_param_t        scan_param;
    /// Initiating parameters (for initiating activity)
    //@trc_union @activity_map[$parent.actv_idx] == GAPM_ACTV_TYPE_INIT
    gapm_le_init_start_param_t  init_param;
    #if (HL_LE_OBSERVER && BLE_PER_ADV)
    /// Periodic synchronization parameters (for periodic synchronization activity)
    //@trc_union @activity_map[$parent.actv_idx] == GAPM_ACTV_TYPE_PER_SYNC
    gapm_le_per_sync_param_t    per_sync_param;
    #endif // (HL_LE_OBSERVER && BLE_PER_ADV)
} gapm_le_activity_start_param_u;

/// Start a given activity command
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_START_ACTIVITY: Start a given activity
    uint8_t operation;
    /// Activity identifier
    uint8_t actv_idx;
    /// Activity parameters
    gapm_le_activity_start_param_u u_param;
} gapm_le_activity_start_cmd_t;

#if (BLE_PER_ADV)
/// Parameters of #GAPM_LE_PER_ADV_STOP_SYNCHRONIZABILITY_CMD message
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_PER_ADV_STOP_SYNCHRONIZABILITY
    uint8_t operation;
    /// Activity identifier
    uint8_t actv_idx;
} gapm_le_per_adv_stop_synchronizability_cmd_t;
#endif // (BLE_PER_ADV)

#if (BT_53)
/// Parameters of #GAPM_LE_SET_ADV_ADDR_CHANGE_REASONS_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_SET_ADV_ADDR_CHANGE_REASONS
    uint8_t operation;
    /// Activity identifier
    uint8_t actv_idx;
    /// Change reasons
    uint8_t change_reasons;
} gapm_le_set_adv_addr_change_reasons_cmd_t;
#endif // (BT_53)

#if (BLE_SUBRATING)
/// Parameters of #GAPM_LE_SET_DEFAULT_SUBRATE_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_SET_DEFAULT_SUBRATE
    uint8_t operation;
    /// Subrating requirements
    gap_le_subrate_req_t subrate_req;
} gapm_le_set_default_subrate_cmd_t;
#endif // (BLE_SUBRATING)





/// @} GAPM_MSG_STRUCT_API

#endif /* GAPM_LE_MSG_H_ */
