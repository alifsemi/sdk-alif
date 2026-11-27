/**
 ****************************************************************************************
 *
 * @file gapc_le_msg.h
 *
 * @brief Generic Access Profile Controller  Message API. - Low Energy
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPC_LE_MSG_H_
#define GAPC_LE_MSG_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapc_msg.h"
#include "gapc_le.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPC_MSG_STRUCT_API Message Structures
/// @ingroup GAPC_MSG_API
/// @{

/// Indicate that a LE connection has been established
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Connection handle
    uint16_t conhdl;
    /// Connection interval
    uint16_t con_interval;
    /// Connection latency
    uint16_t con_latency;
    /// Link supervision timeout
    uint16_t sup_to;
    /// Clock accuracy
    uint8_t clk_accuracy;
    /// Peer address type
    uint8_t peer_addr_type;
    /// Peer BT address
    gap_addr_t peer_addr;
    /// Role of device in connection (0 = Central / 1 = Peripheral)
    uint8_t role;
} gapc_le_connection_req_ind_t;

/// Indication of peer features info
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// 8-byte array for LE features
    uint8_t features[GAP_LE_FEATURES_LEN];
} gapc_le_peer_features_ind_t;

/// Indication of ongoing connection Channel Map
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// channel map value
    struct le_ch_map ch_map;
} gapc_le_channel_map_ind_t;

/// Perform update of connection parameters command
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_UPDATE_PARAMS: Perform update of connection parameters.
    uint8_t operation;
    /// Connection interval minimum
    uint16_t intv_min;
    /// Connection interval maximum
    uint16_t intv_max;
    /// Latency
    uint16_t latency;
    /// Supervision timeout
    uint16_t time_out;
    /// Minimum Connection Event Duration
    uint16_t ce_len_min;
    /// Maximum Connection Event Duration
    uint16_t ce_len_max;
} gapc_le_update_params_cmd_t;

/// Request of updating connection parameters indication
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Connection interval minimum
    uint16_t intv_min;
    /// Connection interval maximum
    uint16_t intv_max;
    /// Latency
    uint16_t latency;
    /// Supervision timeout
    uint16_t time_out;
} gapc_le_update_params_req_ind_t;

/// Connection parameters updated indication
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    ///Connection interval value
    uint16_t con_interval;
    ///Connection latency value
    uint16_t con_latency;
    ///Supervision timeout
    uint16_t sup_to;
} gapc_le_params_updated_ind_t;

/// Master confirm or not that parameters proposed by peripheral are accepted or not
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// True to accept peer connection parameters, False otherwise
    bool accept;
    /// Minimum Connection Event Duration
    uint16_t ce_len_min;
    /// Maximum Connection Event Duration
    uint16_t ce_len_max;
} gapc_le_update_params_cfm_t;

/// Parameters of the #GAPC_LE_SET_PREFERRED_PERIPH_LATENCY_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_SET_PREFERRED_PERIPH_LATENCY_CMD : Set preferred slave latency
    uint8_t operation;
    /// Preferred latency that the controller should use on a connection (in number of connection events)
    uint16_t latency;
} gapc_le_set_preferred_periph_latency_cmd_t;

/// Parameters of the #GAPC_LE_SET_PREFERRED_PERIPH_EVENT_DURATION_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_SET_PREFERRED_PERIPH_EVENT_DURATION_CMD : Set preferred slave event duration
    uint8_t operation;
    /// Preferred event duration that the controller should use on a connection (N * 0.625 ms)
    uint16_t duration;
    /// Slave transmits a single packet per connection event (False/True)
    bool single_tx;
} gapc_le_set_preferred_periph_event_duration_cmd_t;

/// Parameters of the #GAPC_LE_SET_MAX_RX_SIZE_AND_TIME_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_SET_MAX_RX_SIZE_AND_TIME: Set maximum RX size and time using DLE negotiation
    uint8_t operation;
    /// Maximum RX size (in Bytes)
    uint16_t rx_octets;
    /// Maximum RX time (in us)
    uint16_t rx_time;
} gapc_le_set_max_rx_size_and_time_cmd_t;

#if (BLE_PAST)
/// Parameters of the #GAPC_LE_START_PAST_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_START_PAST : Periodic Advertising Sync Transfer
    uint8_t operation;
    /// Periodic Advertising or Periodic Sync activity index
    uint8_t actv_idx;
    /// A value provided by application
    uint16_t service_data;
} gapc_le_start_past_cmd_t;
#endif // (BLE_PAST)

/// Parameters of the #GAPC_LE_CONFIGURE_CTE_TX_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_CONFIGURE_CTE_TX: Constant Tone Extension Transmission configuration
    uint8_t operation;
    /// CTE types (bit0: AOA | bit1: AOD-1us | bit2: AOD-2us) (see #gap_le_cte_type_bf enumeration)
    uint8_t cte_types;
    /// Length of switching pattern (number of antenna IDs in the pattern)
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[__ARRAY_EMPTY];
} gapc_le_configure_cte_tx_cmd_t;

/// Parameters of the #GAPC_LE_CONFIGURE_CTE_RX_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_CONFIGURE_CTE_RX: Constant Tone Extension Reception configuration
    uint8_t operation;
    /// Sampling enable
    bool sample_en;
    /// Slot durations (1: 1us | 2: 2us)
    uint8_t slot_dur;
    /// Length of switching pattern (number of antenna IDs in the pattern)
    uint8_t switching_pattern_len;
    /// Antenna IDs
    uint8_t antenna_id[__ARRAY_EMPTY];
} gapc_le_configure_cte_rx_cmd_t;

/// Parameters of the #GAPC_LE_CONTROL_CTE_REQUEST_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_CONTROL_CTE_REQUEST: Constant Tone Extension request control (enable / disable)
    uint8_t operation;
    /// True to enable TX or RX Constant Tone Extension, False to disable
    bool enable;
    /// CTE request interval (in number of connection events)
    uint16_t interval;
    /// Requested CTE length (in 8us unit)
    uint8_t cte_len;
    /// Requested CTE type (0: AOA | 1: AOD-1us | 2: AOD-2us)  (see #gap_le_cte_type enumeration)
    uint8_t cte_type;
} gapc_le_control_cte_request_cmd_t;

/// Parameters of the #GAPC_LE_CONTROL_CTE_RESPONSE_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_CONTROL_CTE_RESPONSE: Constant Tone Extension response control (enable / disable)
    uint8_t operation;
    /// True to enable TX or RX Constant Tone Extension, False to disable
    bool enable;
} gapc_le_control_cte_response_cmd_t;

/// Indicate reception of a IQ Report event over a BLE connection
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Rx PHY  (see #gap_le_phy_val enumeration)
    uint8_t rx_phy;
    /// Data channel index
    uint8_t data_channel_idx;
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
    /// Connection event counter
    uint16_t con_evt_cnt;
    /// Number of samples
    uint8_t nb_samples;
    /// I/Q sample
    gap_le_iq_sample_t sample[__ARRAY_EMPTY];
} gapc_le_cte_iq_report_ind_t;

/// Parameters of #GAPC_LE_CTE_REQUEST_FAILED_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Reason status code of the CTE request failed (see #hl_err enumeration)
    uint16_t status;
} gapc_le_cte_request_failed_ind_t;

/// Parameters of the #GAPC_LE_SET_PACKET_SIZE_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_SET_PACKET_SIZE : Set the LE Data length value
    uint8_t operation;
    ///Preferred maximum number of payload octets that the local Controller should include
    ///in a single Link Layer Data Channel PDU.
    uint16_t tx_octets;
    ///Preferred maximum number of microseconds that the local Controller should use to transmit
    ///a single Link Layer Data Channel PDU
    uint16_t tx_time;
} gapc_le_set_packet_size_cmd_t;

/// Parameters of the #GAPC_LE_PACKET_SIZE_IND message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    ///The maximum number of payload octets in TX
    uint16_t max_tx_octets;
    ///The maximum time that the local Controller will take to TX
    uint16_t max_tx_time;
    ///The maximum number of payload octets in RX
    uint16_t max_rx_octets;
    ///The maximum time that the local Controller will take to RX
    uint16_t max_rx_time;
} gapc_le_packet_size_ind_t;

/// Set the PHY configuration for current active link
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_SET_PHY : Set the PHY configuration for current active link
    uint8_t operation;
    /// Supported LE PHY for data transmission (see #gap_le_phy_bf enumeration)
    uint8_t tx_phy;
    /// Supported LE PHY for data reception (see #gap_le_phy_bf enumeration)
    uint8_t rx_phy;
    /// PHY options (see #gapc_phy_option enumeration)
    uint8_t phy_opt;
} gapc_le_set_phy_cmd_t;

/// Active link PHY configuration. Triggered when configuration is read or during an update.
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// LE PHY for data transmission (see #gap_le_phy_bf enumeration)
    uint8_t tx_phy;
    /// LE PHY for data reception (see #gap_le_phy_bf enumeration)
    uint8_t rx_phy;
} gapc_le_phy_ind_t;

/// Parameters of the #GAPC_LE_CHANNEL_SELECTION_ALGO_IND
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Used channel selection algorithm
    uint8_t chan_sel_algo;
} gapc_le_channel_selection_algo_ind_t;

/// Local TX power indication
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// PHY (see #gapc_phy_pwr_value enumeration)
    uint8_t phy;
    /// Current transmit power level (dBm)
    int8_t tx_pwr;
    /// Max transmit power level (dBm)
    int8_t max_tx_pwr;
} gapc_le_local_tx_power_ind_t;

/// Remote TX power indication
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// PHY (see #gapc_phy_pwr_value enumeration)
    uint8_t phy;
    /// Transmit Power level (dBm)
    int8_t tx_pwr;
    /// Transmit Power level flags (see #gapc_pwr_ctrl_flags enumeration)
    uint8_t flags;
} gapc_le_peer_tx_power_ind_t;

/// Control TX Power Reports command
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_CONTROL_TX_POWER_REPORT: Enable or disable the reporting to the local Host of transmit power level
    ///                             changes in the local and remote Controllers for the ACL connection
    uint8_t operation;
    /// 1 To Enable local power changes reporting, 0 to disable.
    uint8_t local_en;
    /// 1 To Enable remote power changes reporting, 0 to disable.
    uint8_t remote_en;

} gapc_le_control_tx_power_report_cmd_t;

/// Local TX power change report indication
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// PHY (see #gapc_phy_pwr_value enumeration)
    uint8_t phy;
    /// Transmit Power level (dBm)
    int8_t tx_pwr;
    /// Transmit Power level flags (see #gapc_pwr_ctrl_flags enumeration)
    uint8_t flags;
    /// Delta (dB)
    int8_t delta;
} gapc_le_local_tx_power_report_ind_t;

/// Remote TX power change report indication
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// PHY (see #gapc_phy_pwr_value enumeration)
    uint8_t phy;
    /// Transmit Power level (dBm)
    int8_t tx_pwr;
    /// Transmit Power level flags (see #gapc_pwr_ctrl_flags enumeration)
    uint8_t flags;
    /// Delta (dB)
    int8_t delta;
} gapc_le_peer_tx_power_report_ind_t;

/// Control Path loss configuration
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_CONTROL_PATH_LOSS: Command is used to enable/disable path loss reporting for the connection
    uint8_t operation;
    /// 1 To Enable reporting, 0 to disable.
    uint8_t enable;
    /// High threshold (dB)
    uint8_t high_threshold;
    /// High hysteresis (dB)
    uint8_t high_hysteresis;
    /// Low threshold (dB)
    uint8_t low_threshold;
    /// Low hysteresis (dB)
    uint8_t low_hysteresis;
    /// Min time spent (conn events)
    uint16_t min_time;
} gapc_le_control_path_loss_cmd_t;

/// Path Loss Threshold Indication
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Current path loss (dB)
    uint8_t curr_path_loss;
    /// Zone entered (see #gapc_le_path_loss_zone enumeration)
    uint8_t zone_entered;
} gapc_le_path_loss_threshold_ind_t;

/// Start Encryption command procedure
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_ENCRYPT:  Start encryption procedure.
    uint8_t operation;
    /// Long Term Key information
    gapc_ltk_t ltk;
} gapc_le_encrypt_cmd_t;

/// Encryption requested by peer device indication message.
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Encryption Diversifier
    uint16_t ediv;
    /// Random Number
    gap_le_random_nb_t rand_nb;
} gapc_le_encrypt_req_ind_t;

/// Confirm requested Encryption information.
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Indicate if a LTK has been found for the peer device
    uint8_t found;
    /// Long Term Key
    gap_sec_key_t ltk;
    /// LTK Key Size
    uint8_t key_size;
} gapc_le_encrypt_cfm_t;

/// Start Security Request command procedure
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_REQUEST_SECURITY: Start security request procedure
    uint8_t operation;
    /// Authentication level (see #gap_auth)
    uint8_t auth;
} gapc_le_request_security_cmd_t;
/// Security requested by peer device indication message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Authentication level (#gap_auth)
    uint8_t auth;
} gapc_le_security_ind_t;

/// Parameters of the #GAPC_LE_NOTIFY_KEY_PRESSED_CMD message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_NOTIFY_KEY_PRESSED_CMD : Inform the remote device when keys have been entered or erased
    uint8_t operation;
    /// notification type
    uint8_t notification_type;
} gapc_le_notify_key_pressed_cmd_t;

/// Parameters of the #GAPC_LE_KEY_PRESSED_IND message
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// notification type
    uint8_t notification_type;
} gapc_le_key_pressed_ind_t;

/// Start Bonding command procedure
/*@TRACE*/
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_BOND:  Start bonding procedure.
    uint8_t operation;
    /// Pairing information
    gapc_pairing_t pairing;
    /// Deprecated parameter reserved for future use
    /// To configure required security level, use #GAPM_LE_CONFIGURE_SECURITY_LEVEL_CMD.
    uint8_t sec_req_level;
} gapc_le_bond_cmd_t;

#if (BLE_SUBRATING)
/// Parameters of #GAPC_LE_UPDATE_SUBRATE_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_UPDATE_SUBRATE
    uint8_t operation;
    /// Subrating requirements
    gap_le_subrate_req_t subrate_req;
} gapc_le_update_subrate_cmd_t;

/// Parameters of #GAPC_LE_SUBRATE_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Applied subrating parameters
    gapc_le_subrate_t subrate_params;
} gapc_le_subrate_ind_t;
#endif // (BLE_SUBRATING)

/// Parameters of #GAPC_LE_SET_TX_POWER_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// GAP request type:
    /// - #GAPC_LE_SET_TX_POWER
    uint8_t operation;
    /// TX power (in dBm)
    int8_t tx_pwr;
} gapc_le_set_tx_power_cmd_t;

/// Parameters of #GAPC_LE_SET_TX_POWER_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// TX power (in dBm)
    int8_t new_tx_pwr;
} gapc_le_set_tx_power_ind_t;

#if(GATT_INDICATE_LEGACY_MTU_CHANGED)
/// Parameters of #GAPC_LE_ATT_LEGACY_BEARER_MTU_CHANGED_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// New MTU value
    uint16_t mtu;
} gapc_le_att_legacy_bearer_mtu_changed_ind_t;
#endif // (GATT_INDICATE_LEGACY_MTU_CHANGED)




/// @} GAPC_MSG_STRUCT_API

#endif /* GAPC_LE_MSG_H_ */
