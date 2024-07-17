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
#define MAC154APP_IND_DUMMY_HWORD                     0xDEAD

/*
 * ENUMERATION DEFINITIONS
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * MESSAGES
 ****************************************************************************************
 */
/// Message API of the MAC154APP task
enum mac154app_activities
{
	/// NOINIT
	MAC154APP_NOINIT = 0,
	/// IDLE
	MAC154APP_IDLE,
	/// RX activity
	MAC154APP_RX,
	/// TX activity
	MAC154APP_TX,
	/// ED activity
	MAC154APP_ED,
};

/// Default MAC154APP command structure
typedef struct mac154app_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

} mac154app_cmd_t;


/// Default MAC154APP command Complete structure
typedef struct mac154app_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_cmp_evt_t;

/// Message API of the MAC154APP task
enum mac154app_msg_id
{
	/// Command
	MAC154APP_CMD                = MSG_ID(MAC154APP, 0x00),
	/// Command complete event
	MAC154APP_CMP_EVT            = MSG_ID(MAC154APP, 0x01),
	/// Indication
	MAC154APP_IND                = MSG_ID(MAC154APP, 0x02),
	/// Request indication
	MAC154APP_REQ_IND            = MSG_ID(MAC154APP, 0x03),
	/// Confirmation
	MAC154APP_CFM                = MSG_ID(MAC154APP, 0x04),
	// nota cf ahi.c : reserved value: cste UNKNOWN_TASK_MSG = MSG_ID(MAC154APP, 0xF0);
};


/// MAC154APP_GET_VERSION command structure
typedef struct mac154app_get_version_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

} mac154app_get_version_cmd_t;

/// MAC154APP_GET_VERSION complete event structure
typedef struct mac154app_get_version_cmp_evt
{
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
typedef struct mac154app_get_str_version_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

} mac154app_get_str_version_cmd_t;

/// MAC154APP_GET_STR_VERSION complete event structure
typedef struct mac154app_get_str_version_cmp_evt
{
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
typedef struct mac154app_run_tx_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
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
typedef struct mac154app_run_tx_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_run_tx_cmp_evt_t;

/// MAC154APP_TX_SNGLE command structure
typedef struct mac154app_tx_single_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
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
typedef struct mac154app_tx_single_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// Detailed status of the operation (see enum #tx_status_enum)
	uint8_t tx_status;

	/// eventual acknowledgement Received signal strength indicator in dBm
	int8_t   ack_rssi;
	/// eventual acknowledgement timestamp when the frame was received in microseconds.
	uint32_t ack_timestamp_h;
	/// ... (2 fields unified for uint64 transparent use)
	uint32_t ack_timestamp_l;
	/// eventual acknowledgement
	uint8_t length;
	/// eventual acknowledgement
	uint8_t ack_msg_begin[__ARRAY_EMPTY];
} mac154app_tx_single_cmp_evt_t;


/// MAC154APP_START_RX command structure
typedef struct mac154app_start_rx_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
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

/// MAC154APP_START_RX complete event structure
typedef struct mac154app_start_rx_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_start_rx_cmp_evt_t;


/// MAC154APP_STOP_RX command structure
typedef struct mac154app_stop_rx_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

} mac154app_stop_rx_cmd_t;

/// MAC154APP_STOP_RX complete event structure
typedef struct mac154app_stop_rx_cmp_evt
{
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
typedef struct mac154app_promiscuous_get_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;
} mac154app_promiscuous_get_cmd_t;

/// MAC154APP_PROMISCUOUS_GET complete event structure
typedef struct mac154app_promiscuous_get_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #mac154_err)
	uint16_t status;
	/// promiscuous mode(boolean)
	uint8_t  answer;
} mac154app_promiscuous_get_cmp_evt_t;


/// MAC154APP_PROMISCUOUS_SET command structure
typedef struct mac154app_promiscuous_set_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

	/// promiscuous mode(boolean)
	uint8_t  input;
} mac154app_promiscuous_set_cmd_t;

/// MAC154APP_PROMISCUOUS_SET complete event structure
typedef struct mac154app_promiscuous_set_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_promiscuous_set_cmp_evt_t;



/// common dBm command structure
///  used for MAC154APP_TXPOWER_GET, MAC154APP_MINTXPOWER_GET, MAC154APP_MAXTXPOWER_GET & MAC154APP_LAST_RSSI_GET
typedef struct mac154app_dbm_get_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

	// uint8_t witchone_0current_1min_2max;// selection => see cmd_code
} mac154app_dbm_get_cmd_t;

/// common dBm complete event structure
///  used for MAC154APP_TXPOWER_GET, MAC154APP_MINTXPOWER_GET, MAC154APP_MAXTXPOWER_GET & MAC154APP_LAST_RSSI_GET
typedef struct mac154app_dbm_get_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// txpower (dBm value !)
	int8_t   answer_dbm;
} mac154app_dbm_get_cmp_evt_t;


/// MAC154APP_START_ED command structure
typedef struct mac154app_start_ed_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
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
typedef struct mac154app_start_ed_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// Number of measurements correctly done
	uint8_t  nmeasure;
	/// average measurement
	uint8_t  average;
	/// max measurement
	uint8_t  max;
} mac154app_start_ed_cmp_evt_t;



/// MAC154APP_TXPOWER_SET command structure
typedef struct mac154app_txpower_set_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

	/// txpower (dBm)
	uint8_t  input_dbm;
} mac154app_txpower_set_cmd_t;

/// MAC154APP_TXPOWER_SET complete event structure
typedef struct mac154app_txpower_set_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	///+    in {MAC154_ERR_NO_ERROR | PRF_OUT_OF_RANGE}
	uint16_t status;

} mac154app_txpower_set_cmp_evt_t;

/// MAC154APP_DBG_RW_MEM command structure
typedef struct mac154app_dbg_rw_mem_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
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
typedef struct mac154app_dbg_rw_mem_cmp_evt
{
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
typedef struct mac154app_dbg_rw_reg_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

	/// Write
	uint8_t write;
	/// Read/Write Address
	uint32_t addr;
	/// Write Data
	uint32_t data;
} mac154app_dbg_rw_reg_cmd_t;

/// MAC154APP_DBG_RW_REG complete event structure
typedef struct mac154app_dbg_rw_reg_cmp_evt
{
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
typedef struct mac154app_dbg_rw_rf_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

	/// Write
	uint8_t write;
	/// Read/Write Address
	uint32_t addr;
	/// Write Data
	uint32_t data;
} mac154app_dbg_rw_rf_cmd_t;

/// MAC154APP_DBG_RW_RF complete event structure
typedef struct mac154app_dbg_rw_rf_cmp_evt
{
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
typedef struct mac154app_unknown_msg_ind
{
	/// Indication code (see enum #mac154app_ind_code)
	uint16_t ind_code;
	/// Dummy parameter provided by upper layer for command execution
	uint16_t dummy;
	/// Message identifier
	uint16_t msg_id;
} mac154app_unknown_msg_ind_t;

/// MAC154APP_ERROR_MSG Indication structure definition
typedef struct mac154app_error_msg_ind
{
	/// Indication code (see enum #mac154app_ind_code)
	uint16_t ind_code;
	/// Dummy parameter - Currently has no significance
	uint16_t dummy;
	/// Type of error that has occured (see enum #mac154app_err_code)
	uint16_t err_code;
} mac154app_error_msg_ind_t;

/// MAC154APP_MM_RESET_MSG Indication structure definition
typedef struct mac154app_mm_reset_msg_ind
{
	/// Indication code (see enum #mac154app_ind_code)
	uint16_t ind_code;
	/// Dummy parameter - Currently has no significance
	uint16_t dummy;
	/// Current activity state  (see enum #mac154app_activities)
	uint8_t activity;
} mac154app_mm_reset_msg_ind_t;

/// MAC154APP_RX_FRAME Indication structure definition
typedef struct mac154app_rx_frame_ind
{
	/// Indication code (see enum #mac154app_ind_code)
	uint16_t ind_code;
	/// Dummy parameter provided by upper layer for command execution
	uint16_t dummy;
	/// Message identifier
	uint16_t msg_id;
	/// Received signal strength indicator in dBm
	int8_t   rssi;
	/// Frame pending
	bool frame_pending;
	/// The timestamp when the frame was received in microseconds. (2 fields unified for uint64 transparent use)
	uint32_t timestamp_h;
	/// The timestamp when the frame was received in microseconds. (2 fields unified for uint64 transparent use)
	uint32_t timestamp_l;
	/// frame length
	uint8_t  len;
	/// Received Data
	// uint8_t data[__ARRAY_EMPTY];
	/// Received Data
	uint8_t  data[127];

} mac154app_rx_frame_ind_t;


/// MAC154APP_RESET command structure
typedef struct mac154app_reset_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

} mac154app_reset_cmd_t;

/// MAC154APP_RESET complete event structure
typedef struct mac154app_reset_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_reset_cmp_evt_t;


/// common Id command structure
///  used for MAC154APP_SHORT_ID_GET, MAC154APP_LONG_ID_GET, MAC154APP_PAN_ID_GET
///           MAC154APP_CCA_MODE_GET, MAC154APP_ED_THRESHOLD_GET
	/*@TRACE*/
typedef struct mac154app_id_get_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

} mac154app_id_get_cmd_t;

/// common Id complete event structure
///  used for MAC154APP_SHORT_ID_GET, MAC154APP_LONG_ID_GET, MAC154APP_PAN_ID_GET
///           MAC154APP_CCA_MODE_GET, MAC154APP_ED_THRESHOLD_GET
typedef struct mac154app_id_get_cmp_evt
{
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
typedef struct mac154app_timestamp_get_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

} mac154app_timestamp_get_cmd_t;

/// common Id complete event structure
///  used for MAC154APP_TIMESTAMP_GET
typedef struct mac154app_timestamp_get_cmp_evt
{
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
///      MAC154APP_PENDINGS_SHORT_ID_FIND, MAC154APP_PENDINGS_SHORT_ID_INSERT, MAC154APP_PENDINGS_SHORT_ID_REMOVE
///      MAC154APP_PENDINGS_LONG_ID_FIND, MAC154APP_PENDINGS_LONG_ID_INSERT, MAC154APP_PENDINGS_LONG_ID_REMOVE
///      MAC154APP_CCA_MODE_SET, MAC154APP_ED_THRESHOLD_SET
	/*@TRACE*/
typedef struct mac154app_id_set_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	uint16_t dummy;
	/// the ID value asked ..
	uint32_t value_h;
	/// .. (2 fields unified for uint64 transparent use)
	uint32_t value_l;
} mac154app_id_set_cmd_t;

/// common Id complete event structure
///  used for MAC154APP_SHORT_ID_SET, MAC154APP_LONG_ID_SET, MAC154APP_PAN_ID_SET,
typedef struct mac154app_id_set_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_id_set_cmp_evt_t;


/// common Id command structure
///  used for MAC154APP_TX_PRIO_GET, MAC154APP_RX_PRIO_GET, MAC154APP_ED_PRIO_GET
	/*@TRACE*/
typedef struct mac154app_prio_get_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

} mac154app_prio_get_cmd_t;

/// common Id complete event structure
///  used for MAC154APP_TX_PRIO_GET, MAC154APP_RX_PRIO_GET, MAC154APP_ED_PRIO_GET
typedef struct mac154app_prio_get_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
	/// the ID value asked
	uint8_t  prio;
} mac154app_prio_get_cmp_evt_t;


/// common Id command structure
///  used for MAC154APP_TX_PRIO_SET, MAC154APP_RX_PRIO_SET, MAC154APP_ED_PRIO_SET
	/*@TRACE*/
typedef struct mac154app_prio_set_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	uint16_t dummy;
	/// the prio value asked
	uint8_t  prio;
} mac154app_prio_set_cmd_t;

/// common Id complete event structure
///  used for MAC154APP_TX_PRIO_SET, MAC154APP_RX_PRIO_SET, MAC154APP_ED_PRIO_SET
typedef struct mac154app_prio_set_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_prio_set_cmp_evt_t;



/// MAC154APP_STATUS_GET command structure
typedef struct mac154app_status_get_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;

} mac154app_status_get_cmd_t;


/// MAC154APP_STATUS_GET complete event structure
typedef struct mac154app_status_get_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #mac154_err)
	uint16_t status;
	/// state machine status (see enum #mac154c_state_machine : noninit=0, idle=1,RX=2,TX=3,ED=4)
	uint8_t  i_mac154c_statemachine;
	/// promiscuous mode is activated
	uint8_t  is_promiscuous;
} mac154app_status_get_cmp_evt_t;

typedef struct mac154app_config_header_ie_csl_reduced_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;
	/// csl_period
	uint16_t csl_period;
	/// csl_phase
	uint16_t csl_phase;
} mac154app_config_header_ie_csl_reduced_cmd_t;

typedef struct mac154app_config_header_ie_csl_reduced_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_config_header_ie_csl_reduced_cmp_evt_t;

typedef struct mac154app_config_header_ie_csl_full_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;
	/// csl_period
	uint16_t csl_period;
	/// csl_phase
	uint16_t csl_phase;
	/// csl_rendezvous_time
	uint16_t csl_rendezvous_time;
} mac154app_config_header_ie_csl_full_cmd_t;

typedef struct mac154app_config_header_ie_csl_full_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_config_header_ie_csl_full_cmp_evt_t;

typedef struct mac154app_config_rx_slot_cmd
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter whose meaning is upper layer dependent and which is returned in command complete event and
	///+ indications sent during command handling. It can be used as a sequence number for instance.
	uint16_t dummy;
	/// start
	uint32_t start;
	/// duration
	uint16_t duration;
	/// channel
	uint8_t channel;
} mac154app_config_rx_slot_cmd_t;

typedef struct mac154app_config_rx_slot_cmp_evt
{
	/// Command code (see enum #mac154app_cmd_code)
	uint16_t cmd_code;
	/// Dummy parameter provided by upper layer for command execution.
	uint16_t dummy;
	/// Status of the operation (see enum #hl_err & #mac154_err)
	uint16_t status;
} mac154app_config_rx_slot_cmp_evt_t;

typedef struct mac154app_get_csl_phase_cmp_evt
{
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

/// MAC154APP_CMD command codes
enum mac154app_cmd_code
{
	/// Get version
	MAC154APP_GET_VERSION     = 0X0000,
	/// Get version string (with build date)
	MAC154APP_GET_STR_VERSION = 0X0001,
	/// Start RX
	MAC154APP_START_RX        = 0X0002,
	/// Stop  RX
	MAC154APP_STOP_RX         = 0X0003,
	/// Send TX Single
	MAC154APP_TX_SINGLE       = 0X0004,
	/// Timestamp getter (u64)
	MAC154APP_TIMESTAMP_GET   = 0X0005,
	/// Promiscuous mode getter (boolean)
	MAC154APP_PROMISCUOUS_GET = 0X0006,
	/// Promiscuous mode setter (boolean)
	MAC154APP_PROMISCUOUS_SET = 0X0007,
	/// TxPower getter (dBm)
	MAC154APP_TXPOWER_GET     = 0X0008,
	/// minTxPower getter (dBm)
	MAC154APP_MINTXPOWER_GET  = 0X0009,
	/// maxTxPower getter (dBm)
	MAC154APP_MAXTXPOWER_GET  = 0X000A,
	/// TxPower setter (dBm)
	MAC154APP_TXPOWER_SET     = 0X000B,
	/// Last Received Signal Strength Indication (dBm) getter
	MAC154APP_LAST_RSSI_GET   = 0X000C,
	/// Start Energy Detection (ED)
	MAC154APP_START_ED        = 0X000E,

	/// mac154c state machine + promiscuous
	MAC154APP_STATUS_GET      = 0x0010,
	/// Short Id getter   (U16)
	MAC154APP_SHORT_ID_GET    = 0x0011,
	/// Short Id setter   (U16)
	MAC154APP_SHORT_ID_SET    = 0x0012,
	/// Long Id getter    (U64)
	MAC154APP_LONG_ID_GET     = 0x0013,
	/// Long Id setter    (U64)
	MAC154APP_LONG_ID_SET     = 0x0014,
	/// Pan Id getter     (U16)
	MAC154APP_PAN_ID_GET      = 0x0015,
	/// Pan Id setter     (U16)
	MAC154APP_PAN_ID_SET      = 0x0016,
	/// Pending seak short Id   (U16)
	MAC154APP_PENDINGS_SHORT_ID_FIND   = 0x0017,
	/// Pending insert short Id (U16)
	MAC154APP_PENDINGS_SHORT_ID_INSERT = 0x0018,
	/// Pending remove short Id (U16)
	MAC154APP_PENDINGS_SHORT_ID_REMOVE = 0x0019,
	/// Pending seak long Id    (U64)
	MAC154APP_PENDINGS_LONG_ID_FIND    = 0x0020,
	/// Pending insert long Id  (U64)
	MAC154APP_PENDINGS_LONG_ID_INSERT  = 0x0021,
	/// Pending remove long Id  (U64)
	MAC154APP_PENDINGS_LONG_ID_REMOVE  = 0x0022,
	/// TX Priority getter   (U8)
	MAC154APP_TX_PRIO_GET     = 0x0023,
	/// TX Priority setter   (U8)
	MAC154APP_TX_PRIO_SET     = 0x0024,
	/// RX Priority getter   (U8)
	MAC154APP_RX_PRIO_GET     = 0x0025,
	/// RX Priority setter   (U8)
	MAC154APP_RX_PRIO_SET     = 0x0026,
	/// ED Priority getter   (U8)
	MAC154APP_ED_PRIO_GET     = 0x0027,
	/// ED Priority setter   (U8)
	MAC154APP_ED_PRIO_SET     = 0x0028,
	/// CCA Mode for CCA TX      (U8 cf enum)
	MAC154APP_CCA_MODE_SET     = 0x0030,
	/// ED_THRESHOLD For CCA TX  (I8)
	MAC154APP_ED_THRESHOLD_SET = 0x0031,
	/// CCA Mode for CCA TX      (U8 cf enum!)
	MAC154APP_CCA_MODE_GET     = 0x0032,
	/// ED_THRESHOLD For CCA TX  (I8)
	MAC154APP_ED_THRESHOLD_GET = 0x0033,
	 /// CSL PERIOD getter
	MAC154APP_CSL_PERIOD_GET = 0x0034,
	/// CSL PERIOD setter
	MAC154APP_CSL_PERIOD_SET = 0x0035,
	/// IE CSL HEADER REDUCED CONF
	MAC154APP_CONF_CSL_IE_HEADER_REDUCED = 0x0036,
	/// IE CSL HEADER FULL CONF
	MAC154APP_CONF_CSL_IE_HEADER_FULL = 0x0037,
	/// CSL PHASE getter
	MAC154APP_CSL_PHASE_GET = 0x0040,
	/// CSL FRAME COUNTER update
	MAC154APP_FRAME_COUNTER_UPDATE = 0x0041,
	/// CSL RX SLOT CONFIG
	MAC154APP_CONF_RX_SLOT = 0x0042,
	/// Csl seek short Id   (U16)
	MAC154APP_CSL_SHORT_ID_FIND = 0x0043,
	/// Csl insert short Id (U16)
	MAC154APP_CSL_SHORT_ID_INSERT = 0x0044,
	/// Csl remove short Id (U16)
	MAC154APP_CSL_SHORT_ID_REMOVE = 0x0045,
	/// Csl seek long Id    (U64)
	MAC154APP_CSL_LONG_ID_FIND = 0x0046,
	/// Csl insert long Id  (U64)
	MAC154APP_CSL_LONG_ID_INSERT = 0x0047,
	/// Csl remove long Id  (U64)
	MAC154APP_CSL_LONG_ID_REMOVE = 0x0048,

			// WARNING PAN identifier association process (mgd upper)
	// reserved                 0X010x   (mac154app_ind_code)
	// avoid same values of mac154app_ind_code
	//      => 0X0100, 0X0101
	/// Debug Read/Write Mem
	MAC154APP_DBG_RW_MEM      = 0X0A00,
	/// Debug Read/Write Register
	MAC154APP_DBG_RW_REG      = 0X0A01,
	/// Debug Read/Write RF
	MAC154APP_DBG_RW_RF       = 0X0A02,
	/// Launch a burst of TX (testing purpose only)
	MAC154APP_RUN_TX          = 0X0A03,
	/// Reset
	MAC154APP_RESET           = 0XFFFF,
};


/// MAC154APP_IND indication codes
enum mac154app_ind_code
{
	/// Event triggered when an unknown message has been received by MAC154APP layer  from an upper layer.
	MAC154APP_UNKNOWN_MSG     = 0x0000,
	/// Event triggered when an frame has been received by MAC154APP layer.
	MAC154APP_RX_FRAME        = 0x0100,
	/// Event triggered when an error occurs
	MAC154APP_ERR_INFO        = 0x0101,
	/// Event triggered when multimode reset is invoked
	MAC154APP_MM_RESET        = 0x0102,
	// TODO reserve eventual new value into mac154app_cmd_code
};

/// MAC154APP_ERR error codes
enum mac154app_err_code
{
	/// No error
	MAC154APP_ERR_NO_ERROR             = 0x0000,
	/// Error when UART communication is out of sync
	MAC154APP_ERR_HW_OUT_OF_SYNC       = 0x0001,
	/// Error when UART communication is out of sync
	MAC154APP_ERR_HW_MEM_ALLOC_FAIL    = 0x0002,
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
