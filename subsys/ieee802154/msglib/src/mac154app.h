/**
 ****************************************************************************************
 *
 * @file mac154app.h
 *
 * @brief MAC154 Test App
 *
 * Copyright (C) RivieraWaves 2022-2022
 *
 ****************************************************************************************
 */

#ifndef MAC154APP_H_
#define MAC154APP_H_

/**
 ****************************************************************************************
 * @defgroup MAC154APP Test App
 * @ingroup MAC154API MAC154
 * @brief MAC154APP permit validation thru 3VT module.
 * The Test App contains various usages of IEEE MAC802.15.4 thanks to Riviera Waves API.
 * @{
 ****************************************************************************************
 */
#include "stdbool.h"
#define __ARRAY_EMPTY
#define TASK_ID_AHI          16
#define TASK_ID_MAC154APP    222
#define TASK_FIRST_MSG(task) ((uint16_t)((task) << 8))
#define MSG_ID(task, idx)    (TASK_FIRST_MSG((TASK_ID_##task)) + idx)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
/// TX Brust test activation (needed only for internal tests)
#define TESTING_PURPOSE_TX_BURST
#define MAC154APP_IND_DUMMY_HWORD 0xDEAD

#define MAC154_VENDOR_SPECIFIC_IE_OUI_LEN 3
#define MAC154_VENDOR_SPECIFIC_IE_MAX_LEN 127
/*
 * ENUMERATION DEFINITIONS
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// Short ID
typedef uint16_t short_id_t;

/// PAN ID
typedef uint16_t pan_id_t;

/*
 * MESSAGES
 ****************************************************************************************
 */
/**
 * @brief Information Element Types.
 *
 * @details See sections 7.4.2.1 and 7.4.3.1.
 */
enum mac154_ie_type {
	MAC154_IE_TYPE_HEADER = 0x0,
	MAC154_IE_TYPE_PAYLOAD,
};

// only MAC154_HEADER_IE_ELEMENT_ID_CSL_IE is used
enum mac154_header_ie_element_id {
	MAC154_HEADER_IE_ELEMENT_ID_VENDOR_SPECIFIC_IE = 0x00,
	MAC154_HEADER_IE_ELEMENT_ID_CSL_IE = 0x1a,
	MAC154_HEADER_IE_ELEMENT_ID_RIT_IE = 0x1b,
	MAC154_HEADER_IE_ELEMENT_ID_RENDEZVOUS_TIME_IE = 0x1d,
	MAC154_HEADER_IE_ELEMENT_ID_TIME_CORRECTION_IE = 0x1e,
	MAC154_HEADER_IE_ELEMENT_ID_HEADER_TERMINATION_1 = 0x7e,
	MAC154_HEADER_IE_ELEMENT_ID_HEADER_TERMINATION_2 = 0x7f,
};

enum mac154_header_ie_element_id_size {
	MAC154_HEADER_IE_ELEMENT_ID_CSL_IE_REDUCED_SIZE = 0x04,
	MAC154_HEADER_IE_ELEMENT_ID_CSL_IE_FULL_SIZE = 0x06,
};

struct mac154_header_ie_rendezvous_time_full {
	// in units of 10 symbol periods
	uint16_t rendezvous_time;
	uint16_t wakeup_interval;
};

/**
 * @brief Vendor specific information, see section 7.4.2.2
 */
struct mac154_header_ie_vendor_specific {
	uint8_t vendor_oui[MAC154_VENDOR_SPECIFIC_IE_OUI_LEN];
	uint8_t vendor_specific_info[MAC154_VENDOR_SPECIFIC_IE_MAX_LEN];
};

/**
 * @brief Reduced Rendezvous Time IE, see section 7.4.2.6
 * (macCslInterval is zero).
 */
struct mac154_header_ie_rendezvous_time_reduced {
	// in units of 10 symbol periods
	uint16_t rendezvous_time;
};

/** @brief Rendezvous Time IE, see section 7.4.2.6. */
struct mac154_header_ie_rendezvous_time {
	union {
		struct mac154_header_ie_rendezvous_time_full full;
		struct mac154_header_ie_rendezvous_time_reduced reduced;
	};
};

/// @brief Full CSL IE, see section 7.4.2.3.
struct mac154_header_ie_csl_full {
	// in units of 10 symbol periods
	uint16_t csl_phase;
	uint16_t csl_period;
	uint16_t csl_rendezvous_time;
};

/// @brief Reduced CSL IE, see section 7.4.2.3.
struct mac154_header_ie_csl_reduced {
	// in units of 10 symbol periods
	uint16_t csl_phase;
	uint16_t csl_period;
};
/// @brief  generic CSL IE
struct mac154_header_ie_csl {
	union {
		struct mac154_header_ie_csl_full full;
		struct mac154_header_ie_csl_reduced reduced;
	};
};

/// @brief generic IE header
typedef struct mac154_header_ie {
	uint16_t length;
	uint16_t element_id_low; /* see enum mac154_header_ie_element_id */
	uint16_t element_id_high;
	uint16_t type; /* always 0 */
	union {
		struct mac154_header_ie_vendor_specific vendor_specific;
		struct mac154_header_ie_csl csl;
		struct mac154_header_ie_rendezvous_time rendezvous_time;
		/* add additional supported header IEs here */
	} content;
} mac154_header_ie_t;

/// Message API of the MAC154APP task
enum mac154app_activities {
	/// IDLE
	MAC154APP_IDLE = 1,
	/// RX activity
	MAC154APP_RX,
	/// TX activity
	MAC154APP_TX,
	/// ED activity
	MAC154APP_ED,
};

/// Default MAC154APP command structure
typedef struct mac154app_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

} mac154app_cmd_t;

/// Default MAC154APP command Complete structure
typedef struct mac154app_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_cmp_evt_t;

/// Message API of the MAC154APP task
enum mac154app_msg_id {
	/// Command
	MAC154APP_CMD = MSG_ID(MAC154APP, 0x00),
	/// Command complete event
	MAC154APP_CMP_EVT = MSG_ID(MAC154APP, 0x01),
	/// Indication
	MAC154APP_IND = MSG_ID(MAC154APP, 0x02),
	/// Request indication
	MAC154APP_REQ_IND = MSG_ID(MAC154APP, 0x03),
	/// Confirmation
	MAC154APP_CFM = MSG_ID(MAC154APP, 0x04),
	// nota cf ahi.c : reserved value: cste UNKNOWN_TASK_MSG = MSG_ID(MAC154APP, 0xF0);
};

/// MAC154APP_GET_VERSION command structure
typedef struct mac154app_get_version_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

} mac154app_get_version_cmd_t;

/// MAC154APP_GET_VERSION complete event structure
typedef struct mac154app_get_version_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// HW Version
	uint32_t hw_version;
	/// SW Version
	uint32_t sw_version;
} mac154app_get_version_cmp_evt_t;

/// MAC154APP_GET_STR_VERSION command structure
typedef struct mac154app_get_str_version_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

} mac154app_get_str_version_cmd_t;

/// MAC154APP_GET_STR_VERSION complete event structure
typedef struct mac154app_get_str_version_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// length of Version
	uint8_t length;
	// uint8_t s_version[__ARRAY_EMPTY];  /// Version string - FIXME
	/// Version string
	uint8_t s_version[100];
} mac154app_get_str_version_cmp_evt_t;

/// MAC154APP_RUN_TX command structure
typedef struct mac154app_run_tx_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;
	/// number of frames
	uint16_t nframes;
	/// frame length
	uint8_t len;
	/// Channel
	uint8_t channel;
	/// acknowledgement_asked
	uint8_t acknowledgement_asked;
} mac154app_run_tx_cmd_t;

/// MAC154APP_RUN_TX complete event structure
typedef struct mac154app_run_tx_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_run_tx_cmp_evt_t;

/// MAC154APP_TX_SNGLE command structure
typedef struct mac154app_tx_single_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

	/// Channel
	uint8_t channel;
	/// Clear Channel Assessment needed ?
	uint8_t cca_requested;
	/// Wait for acknowledgement
	uint8_t acknowledgement_asked;
	/// Timestamp
	uint32_t timestamp;
	/// frame length
	uint8_t len;
	/// Data
	uint8_t data[__ARRAY_EMPTY];
} mac154app_tx_single_cmd_t;

/// MAC154APP_TX_SNGLE complete event structure
typedef struct mac154app_tx_single_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// Detailed status of the operation (see enum #tx_status_enum)
	uint8_t tx_status;

	/// eventual acknowledgement Received signal strength indicator in dBm
	int8_t ack_rssi;
	/// ... (2 fields unified for uint64 transparent use)
	uint32_t ack_timestamp_l;
	/// eventual acknowledgement timestamp when the frame was received in microseconds.
	uint32_t ack_timestamp_h;
	/// eventual acknowledgement
	uint8_t length;
	/// eventual acknowledgement
	uint8_t ack_msg_begin[__ARRAY_EMPTY];
} mac154app_tx_single_cmp_evt_t;

/// MAC154APP_START_RX command structure
typedef struct mac154app_start_rx_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;
	/// Channel
	uint8_t channel;
	/// mute_indications
	uint8_t b_mute_indications;
	/// nb of frames to wait, 0 for infinite
	uint8_t nb_frames;
	/// Timestamp
	uint32_t timestamp;
} mac154app_start_rx_cmd_t;

/// MAC154APP_STOP_RX command structure
typedef struct mac154app_stop_rx_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

} mac154app_stop_rx_cmd_t;

/// MAC154APP_STOP_RX complete event structure
typedef struct mac154app_stop_rx_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// Number of frames correctly received
	uint16_t nreceived;
} mac154app_stop_rx_cmp_evt_t;

/// MAC154APP_PROMISCUOUS_GET command structure
typedef struct mac154app_promiscuous_get_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;
} mac154app_promiscuous_get_cmd_t;

/// MAC154APP_PROMISCUOUS_GET complete event structure
typedef struct mac154app_promiscuous_get_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #mac154_err)
	uint16_t status;
	/// promiscuous mode(boolean)
	uint8_t answer;
} mac154app_promiscuous_get_cmp_evt_t;

/// MAC154APP_CONF_PROMISCUOUS command structure
typedef struct mac154app_config_promiscuous_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

	/// promiscuous mode(boolean)
	uint8_t input;
} mac154app_config_promiscuous_cmd_t;

/// common dBm command structure
///  used for MAC154APP_TXPOWER_GET, MAC154APP_MINTXPOWER_GET, MAC154APP_MAXTXPOWER_GET &
///  MAC154APP_LAST_RSSI_GET
typedef struct mac154app_dbm_get_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

	// uint8_t witchone_0current_1min_2max;// selection => see cmd_code
} mac154app_dbm_get_cmd_t;

/// common dBm complete event structure
///  used for MAC154APP_TXPOWER_GET, MAC154APP_MINTXPOWER_GET, MAC154APP_MAXTXPOWER_GET &
///  MAC154APP_LAST_RSSI_GET
typedef struct mac154app_dbm_get_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// txpower (dBm value !)
	int8_t answer_dbm;
} mac154app_dbm_get_cmp_evt_t;

/// MAC154APP_START_ED command structure
typedef struct mac154app_start_ed_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

	/// Channel
	uint8_t channel;
	///  Used threshold (dBm) - cf Energy above threshold
	int8_t threshold;
	/// nb of measurements to wait into a salvo for [7 or 8 bits] (unit ? tic each 128 µs)
	///+ ==> ie max 32 milli sec for the process
	uint8_t nb_tics;
	/// Timestamp
	uint32_t timestamp;
} mac154app_start_ed_cmd_t;

/// MAC154APP_START_ED complete event structure
typedef struct mac154app_start_ed_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// Number of measurements correctly done
	uint8_t nmeasure;
	/// average measurement
	uint8_t average;
	/// max measurement
	uint8_t max;
} mac154app_start_ed_cmp_evt_t;

/// MAC154APP_TXPOWER_SET command structure
typedef struct mac154app_txpower_set_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

	/// txpower (dBm)
	uint8_t input_dbm;
} mac154app_txpower_set_cmd_t;

/// MAC154APP_DBG_RW_MEM command structure
typedef struct mac154app_dbg_rw_mem_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

	/// Write
	uint8_t write;
	/// Read/Write Address
	uint32_t addr;
	/// Size
	uint8_t size;
	/// Write Data
	uint32_t data;
} mac154app_dbg_rw_mem_cmd_t;

/// MAC154APP_DBG_RW_MEM complete event structure
typedef struct mac154app_dbg_rw_mem_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (@see enum #hl_err & @see enum #mac154_err)
	uint16_t status;
	/// Read Data
	uint32_t data;
} mac154app_dbg_rw_mem_cmp_evt_t;

/// MAC154APP_DBG_RW_REG command structure
typedef struct mac154app_dbg_rw_reg_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

	/// Write
	uint8_t write;
	/// Read/Write Address
	uint32_t addr;
	/// Write Data
	uint32_t data;
} mac154app_dbg_rw_reg_cmd_t;

/// MAC154APP_DBG_RW_REG complete event structure
typedef struct mac154app_dbg_rw_reg_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// Read Data
	uint32_t data;
} mac154app_dbg_rw_reg_cmp_evt_t;

/// MAC154APP_DBG_RW_RF command structure
typedef struct mac154app_dbg_rw_rf_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

	/// Write
	uint8_t write;
	/// Read/Write Address
	uint32_t addr;
	/// Write Data
	uint32_t data;
} mac154app_dbg_rw_rf_cmd_t;

/// MAC154APP_DBG_RW_RF complete event structure
typedef struct mac154app_dbg_rw_rf_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// Read Data
	uint32_t data;
} mac154app_dbg_rw_rf_cmp_evt_t;

/// MAC154APP_UNKNOWN_MSG Indication structure definition
typedef struct mac154app_unknown_msg_ind {
	/// Indication code (see enum #mac154app_ind_code)
	uint16_t ind_code;
	/// Dummy parameter provided by upper layer for command execution
	uint16_t dummy;
	/// Message identifier
	uint16_t msg_id;
} mac154app_unknown_msg_ind_t;

/// MAC154APP_ERROR_MSG Indication structure definition
typedef struct mac154app_error_msg_ind {
	/// Indication code (see enum #mac154app_ind_code)
	uint16_t ind_code;
	/// Dummy parameter - Currently has no significance
	uint16_t dummy;
	/// Type of error that has occured (see enum #mac154app_err_code)
	uint16_t err_code;
} mac154app_error_msg_ind_t;

/// MAC154APP_MM_RESET_MSG Indication structure definition
typedef struct mac154app_mm_reset_msg_ind {
	/// Indication code (see enum #mac154app_ind_code)
	uint16_t ind_code;
	/// Dummy parameter - Currently has no significance
	uint16_t dummy;
	/// Current activity state  (see enum #mac154app_activities)
	uint8_t activity;
} mac154app_mm_reset_msg_ind_t;

/// MAC154APP_RX_FRAME Indication structure definition
typedef struct mac154app_rx_frame_ind {
	/// Indication code (see enum #mac154app_ind_code)
	uint16_t ind_code;
	/// Dummy parameter provided by upper layer for command execution
	uint16_t dummy;
	/// Message identifier
	uint16_t msg_id;
	/// Received signal strength indicator in dBm
	int8_t rssi;
	/// Frame pending
	bool frame_pending;
	/// Security Enabled Bit was set in the ACK
	bool ack_seb;
	/// Key index set in the ACK
	uint8_t ack_keyid;
	////  Frame counter set in the ACK
	uint32_t ack_fc;
	/// The timestamp when the frame was received in microseconds. (2 fields unified for uint64
	/// transparent use)
	uint32_t timestamp_l;
	/// The timestamp when the frame was received in microseconds. (2 fields unified for uint64
	/// transparent use)
	uint32_t timestamp_h;
	/// frame length
	uint8_t len;
	/// Received Data
	// uint8_t data[__ARRAY_EMPTY];
	/// Received Data
	uint8_t data[127];

} mac154app_rx_frame_ind_t;

/// MAC154APP_RESET command structure
typedef struct mac154app_reset_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

} mac154app_reset_cmd_t;

/// common Id command structure
///  used for MAC154APP_SHORT_ID_GET, MAC154APP_LONG_ID_GET, MAC154APP_PAN_ID_GET
///           MAC154APP_CCA_MODE_GET, MAC154APP_ED_THRESHOLD_GET
/*@TRACE*/
typedef struct mac154app_id_get_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

} mac154app_id_get_cmd_t;

/// common Id complete event structure
///  used for MAC154APP_SHORT_ID_GET, MAC154APP_LONG_ID_GET, MAC154APP_PAN_ID_GET
///           MAC154APP_CCA_MODE_GET, MAC154APP_ED_THRESHOLD_GET
typedef struct mac154app_id_get_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// .. (2 fields unified for uint64 transparent use)
	uint32_t value_l;
	/// the ID value asked ..
	uint32_t value_h;
} mac154app_id_get_cmp_evt_t;

/// common Id command structure
///  used for MAC154APP_TIMESTAMP_GET
/*@TRACE*/
typedef struct mac154app_timestamp_get_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

} mac154app_timestamp_get_cmd_t;

/// common Id complete event structure
///  used for MAC154APP_TIMESTAMP_GET
typedef struct mac154app_timestamp_get_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// .. (2 fields unified for uint64 transparent use)
	uint32_t value_l;
	/// the ID value asked ..
	uint32_t value_h;
} mac154app_timestamp_get_cmp_evt_t;

/// common Id command structure
///  used for MAC154APP_SHORT_ID_SET, MAC154APP_LONG_ID_SET, MAC154APP_PAN_ID_SET,
///      MAC154APP_PENDINGS_SHORT_ID_FIND, MAC154APP_PENDINGS_SHORT_ID_INSERT,
///      MAC154APP_PENDINGS_SHORT_ID_REMOVE MAC154APP_PENDINGS_LONG_ID_FIND,
///      MAC154APP_PENDINGS_LONG_ID_INSERT, MAC154APP_PENDINGS_LONG_ID_REMOVE
///      MAC154APP_CCA_MODE_SET, MAC154APP_ED_THRESHOLD_SET
/*@TRACE*/
typedef struct mac154app_id_set_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	uint16_t dummy;
	/// the ID value asked ..
	uint32_t value_h;
	/// .. (2 fields unified for uint64 transparent use)
	uint32_t value_l;
} mac154app_id_set_cmd_t;

/// common Id command structure
///  used for MAC154APP_TX_PRIO_GET, MAC154APP_RX_PRIO_GET, MAC154APP_ED_PRIO_GET
/*@TRACE*/
typedef struct mac154app_prio_get_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

} mac154app_prio_get_cmd_t;

/// common Id complete event structure
///  used for MAC154APP_TX_PRIO_GET, MAC154APP_RX_PRIO_GET, MAC154APP_ED_PRIO_GET
typedef struct mac154app_prio_get_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// the ID value asked
	uint8_t prio;
} mac154app_prio_get_cmp_evt_t;

/// common Id command structure
///  used for MAC154APP_TX_PRIO_SET, MAC154APP_RX_PRIO_SET, MAC154APP_ED_PRIO_SET
/*@TRACE*/
typedef struct mac154app_prio_set_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	uint16_t dummy;
	/// the prio value asked
	uint8_t prio;
} mac154app_prio_set_cmd_t;

/// MAC154APP_STATUS_GET command structure
typedef struct mac154app_status_get_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;

} mac154app_status_get_cmd_t;

/// MAC154APP_STATUS_GET complete event structure
typedef struct mac154app_status_get_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #mac154_err)
	uint16_t status;
	/// state machine status (see enum #mac154c_state_machine : noninit=0,
	/// idle=1,RX=2,TX=3,ED=4)
	uint8_t i_mac154c_statemachine;
	/// promiscuous mode is activated
	uint8_t is_promiscuous;
} mac154app_status_get_cmp_evt_t;

typedef struct mac154app_config_enh_ack_header_ie_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;
	/// extended address
	/// the ID value asked ..
	uint32_t value_h;
	/// .. (2 fields unified for uint64 transparent use)
	uint32_t value_l;
	/// device short identifier
	short_id_t short_id;
	/// Purge IE if enable
	uint16_t purge_ie;
	/// Pointer on the header_ie to be added
	struct mac154_header_ie header_ie;
} mac154app_config_enh_ack_header_ie_cmd_t;

typedef struct mac154app_config_ack_frame_pending_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;
	/// extended address
	/// the ID value asked ..
	uint32_t value_h;
	/// .. (2 fields unified for uint64 transparent use)
	uint32_t value_l;
	/// device short identifier
	short_id_t short_id;
	/// extended if false: short
	bool extended;
	/// enabled or not
	bool enabled;
} mac154app_config_ack_frame_pending_cmd_t;

typedef struct mac154app_config_csl_period_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;
	/// csl_period
	uint16_t csl_period;
} mac154app_config_csl_period_cmd_t;

enum mac154_filter_type /// MAC154 Filter Type Mode
{
	MAC154_FILTER_TYPE_IEEE_ADDR = 0,
	MAC154_FILTER_TYPE_SHORT_ADDR = 1,
	MAC154_FILTER_TYPE_PAN_ID = 2,
	MAC154_FILTER_TYPE_SRC_IEEE_ADDR = 3,
	MAC154_FILTER_TYPE_SRC_SHORT_ADDR = 4,
};
typedef struct mac154app_filter_set_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;
	/// Select which filter type
	uint16_t filter_type;
	/// extended address
	/// the ID value asked ..
	uint32_t value_h;
	/// .. (2 fields unified for uint64 transparent use)
	uint32_t value_l;
	/// short address
	short_id_t short_id;
	/// PAN ID
	short_id_t pan_id;
} mac154app_filter_set_cmd_t;

typedef struct mac154app_config_rx_slot_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command
	/// complete event and
	///+ indications sent during command handling. It can be used as a sequence number for
	///instance.
	uint16_t dummy;
	/// start
	uint32_t start;
	/// duration
	uint16_t duration;
	/// channel
	uint8_t channel;
} mac154app_config_rx_slot_cmd_t;

typedef struct mac154app_get_csl_phase_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// csl phase
	uint16_t csl_phase;
	/// .. (2 fields unified for uint64 transparent use)
	uint32_t value_l;
	/// the ID value asked ..
	uint32_t value_h;

} mac154app_get_csl_phase_cmp_evt_t;

typedef struct mac154app_config_mac_keys_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Key material phase
	uint8_t key_value[16];
	/// Initial value of frame counter associated with the key
	uint32_t key_frame_counter;
	/// Indicate if per-key frame counter should be used
	bool frame_counter_per_key;
	/// Key identifier mode
	uint8_t key_id_mode;
	/// Key identifier
	uint8_t key_id[9];
} mac154app_config_mac_keys_cmd_t;

typedef struct mac154app_config_frame_counter_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Frame counter value
	uint32_t frame_counter;
} mac154app_config_frame_counter_cmd_t;

typedef struct mac154app_config_mac_keys_cmp_evt {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// Entry index used
	uint16_t entry_idx;
} mac154app_config_mac_keys_cmp_evt_t;

typedef struct mac154app_expected_rx_time_cmd {
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Expected RX time
	uint32_t expected_time;
} mac154app_expected_rx_time_cmd_t;

/// MAC154APP_CMD command codes
enum mac154app_cmd_code {
	/// Get version
	MAC154APP_GET_VERSION = 0X0000,
	/// Get version string (with build date)
	MAC154APP_GET_STR_VERSION = 0X0001,
	/// Start RX
	MAC154APP_START_RX = 0X0002,
	/// Stop  RX
	MAC154APP_STOP_RX = 0X0003,
	/// Send TX Single
	MAC154APP_TX_SINGLE = 0X0004,
	/// Timestamp getter (u64)
	MAC154APP_TIMESTAMP_GET = 0X0005,
	/// Promiscuous mode getter (boolean)
	MAC154APP_PROMISCUOUS_GET = 0X0006,
	/// Promiscuous mode setter (boolean)
	/// Set promiscuous mode
	MAC154APP_CONF_PROMISCUOUS = 0x0007,
	/// TxPower getter (dBm)
	MAC154APP_TXPOWER_GET = 0X0008,
	/// minTxPower getter (dBm)
	MAC154APP_MINTXPOWER_GET = 0X0009,
	/// maxTxPower getter (dBm)
	MAC154APP_MAXTXPOWER_GET = 0X000A,
	/// TxPower setter (dBm)
	MAC154APP_TXPOWER_SET = 0X000B,
	/// Last Received Signal Strength Indication (dBm) getter
	MAC154APP_LAST_RSSI_GET = 0X000C,
	/// Start Energy Detection (ED)
	MAC154APP_START_ED = 0X000E,

	/// mac154c state machine + promiscuous
	MAC154APP_STATUS_GET = 0x0010,
	/// Short Id getter   (U16)
	MAC154APP_SHORT_ID_GET = 0x0011,
	/// Long Id getter    (U64)
	MAC154APP_LONG_ID_GET = 0x0013,
	/// Pan Id getter     (U16)
	MAC154APP_PAN_ID_GET = 0x0015,
	/// TX Priority getter   (U8)
	MAC154APP_TX_PRIO_GET = 0x0023,
	/// TX Priority setter   (U8)
	MAC154APP_TX_PRIO_SET = 0x0024,
	/// RX Priority getter   (U8)
	MAC154APP_RX_PRIO_GET = 0x0025,
	/// RX Priority setter   (U8)
	MAC154APP_RX_PRIO_SET = 0x0026,
	/// ED Priority getter   (U8)
	MAC154APP_ED_PRIO_GET = 0x0027,
	/// ED Priority setter   (U8)
	MAC154APP_ED_PRIO_SET = 0x0028,
	/// CCA Mode for CCA TX      (U8 cf enum)
	MAC154APP_CCA_MODE_SET = 0x0030,
	/// ED_THRESHOLD For CCA TX  (I8)
	MAC154APP_ED_THRESHOLD_SET = 0x0031,
	/// CCA Mode for CCA TX      (U8 cf enum!)
	MAC154APP_CCA_MODE_GET = 0x0032,
	/// ED_THRESHOLD For CCA TX  (I8)
	MAC154APP_ED_THRESHOLD_GET = 0x0033,
	/// CSL PERIOD getter
	MAC154APP_CSL_PERIOD_GET = 0x0034,
	/// CSL PHASE getter
	MAC154APP_CSL_PHASE_GET = 0x0040,
	/// CSL RX SLOT CONFIG
	MAC154APP_CONF_RX_SLOT = 0x0042,
	/// Configure MAC keys
	MAC154APP_CONF_MAC_KEYS = 0x0049,
	/// Configure Frame counter
	MAC154APP_CONF_FRAME_CNT = 0x004A,
	/// Configure Frame counter if larger
	MAC154APP_CONF_FRAME_CNT_IF_LARGER = 0x004B,
	/// Configure a timepoint at which an RX frame is expected to arrive.
	MAC154APP_CONF_EXPECTED_RX_TIME = 0x004C,
	/// Configure MAC keys
	MAC154APP_CLEAR_MAC_KEYS = 0x004E,
	/// Configure IE
	MAC154APP_CONF_IE_HEADER = 0x004F,
	MAC154APP_CONF_ACK_FP = 0x0050,
	MAC154APP_FILTER_SET = 0x0051,
	MAC154APP_CONF_CSL_PERIOD = 0x0052,

	// WARNING PAN identifier association process (mgd upper)
	// reserved                 0X010x   (mac154app_ind_code)
	// avoid same values of mac154app_ind_code
	//      => 0X0100, 0X0101
	/// Debug Read/Write Mem
	MAC154APP_DBG_RW_MEM = 0X0A00,
	/// Debug Read/Write Register
	MAC154APP_DBG_RW_REG = 0X0A01,
	/// Debug Read/Write RF
	MAC154APP_DBG_RW_RF = 0X0A02,
	/// Launch a burst of TX (testing purpose only)
	MAC154APP_RUN_TX = 0X0A03,
	/// Reset
	MAC154APP_RESET = 0XFFFF,
};

/// MAC154APP_IND indication codes
enum mac154app_ind_code {
	/// Event triggered when an unknown message has been received by MAC154APP layer  from an
	/// upper layer.
	MAC154APP_UNKNOWN_MSG = 0x0000,
	/// Event triggered when an frame has been received by MAC154APP layer.
	MAC154APP_RX_FRAME = 0x0100,
	/// Event triggered when an error occurs
	MAC154APP_ERR_INFO = 0x0101,
	/// Event triggered when multimode reset is invoked
	MAC154APP_MM_RESET = 0x0102,
	// TODO reserve eventual new value into mac154app_cmd_code
};

/// MAC154APP_ERR error codes
enum mac154app_err_code {
	/// No error
	MAC154APP_ERR_NO_ERROR = 0x0000,
	/// Error when UART communication is out of sync
	MAC154APP_ERR_HW_OUT_OF_SYNC = 0x0001,
	/// Error when UART communication is out of sync
	MAC154APP_ERR_HW_MEM_ALLOC_FAIL = 0x0002,
	/// Error HW exception occured on the controller
	MAC154APP_ERR_HW_EXCEPTION_OCCURED = 0x0003,
};

/*
 * MESSAGES IDENTIFIERS
 ****************************************************************************************
 */

/*
 * MESSAGES STRUCTURES
 ****************************************************************************************
 */

/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

// ========================== API ENTRY POINTS beg =============================
// ========================== API ENTRY POINTS end =============================

/**
 * @brief      Initialize the Mac 15.4 Test Application
 */

/**
 * @brief      Send to the Host the Event to inform which activity has stopped
 */

/**
 * @brief      Send to the host hardware error notification from controller
 */

/// @} MAC154C
#endif // MAC154C_H_
