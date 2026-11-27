/**
 ****************************************************************************************
 *
 * @file gapm_msg.h
 *
 * @brief Generic Access Profile Manager Message API.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_MSG_H_
#define GAPM_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_MSG_API Message API
 * @ingroup GAPM_API
 * @brief Message API for GAP Manager module
 * @details It handles messages from lower and higher layers not related to an ongoing connection.
 * @{
 * @}
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "gap.h"
#include "gapm.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPM_MSG_ID_API Message Identifiers
/// @ingroup GAPM_MSG_API
/// @{

/// GAP Manager Message Interface
/*@TRACE*/
enum gapm_msg_ids
{
    /* Default event */
    /// Command Complete event
    GAPM_CMP_EVT = MSG_ID(GAPM, 0x00u),
    /// Indicate that a message has been received on an unknown task
    GAPM_UNKNOWN_TASK_IND = MSG_ID(GAPM, 0x01u),
    /// Indication to the task that sends the unknown message
    /// see #gapm_unknown_msg_ind_t
    GAPM_UNKNOWN_MSG_IND = MSG_ID(GAPM, 0x02u),

    /* Device Configuration */
    /// Reset link layer and the host command
    GAPM_RESET_CMD = MSG_ID(GAPM, 0x03u),
    /// Set device configuration command
    GAPM_CONFIGURE_CMD = MSG_ID(GAPM, 0x04u),
    /// Set device le channel map
    GAPM_LE_SET_CHANNEL_MAP_CMD = MSG_ID(GAPM, 0x05u),
    /// Modify current IRK
    GAPM_LE_SET_IRK_CMD = MSG_ID(GAPM, 0x06u),
    /// Set Device Name
    GAPM_SET_NAME_CMD = MSG_ID(GAPM, 0x07u),
    /// Insert into SDP a device identification record command.
    GAPM_BT_SET_SDP_IDENTIFICATION_RECORD_CMD = MSG_ID(GAPM, 0x08u),
    /// Set acceptable parameters for subrating requests command
    GAPM_LE_SET_DEFAULT_SUBRATE_CMD = MSG_ID(GAPM, 0x09u),
    /// Set Rx Max Slot command
    GAPM_BT_SET_RX_MAX_SLOT_CMD = MSG_ID(GAPM, 0x0Au),
    #if (BLE_GAPC)
    /// Configure security level required for LE connections command
    /// (see #gapm_le_configure_security_level)
    GAPM_LE_CONFIGURE_SECURITY_LEVEL_CMD = MSG_ID(GAPM, 0x0Bu),
    #endif // (BLE_GAPC)
    /// Disable a supported feature\n
    /// (see #gapm_disable_feature)
    GAPM_DISABLE_FEATURE_CMD = MSG_ID(GAPM, 0x0Cu),
    /// Set Appearance
    GAPM_LE_SET_APPEARANCE_CMD = MSG_ID(GAPM, 0x0Du),

    /* Local device information */
    /// Get local device info command
    GAPM_GET_INFO_CMD = MSG_ID(GAPM, 0x10u),
    /// Local device version indication event
    GAPM_VERSION_IND = MSG_ID(GAPM, 0x11u),
    /// BD Address indication event
    /// Could be either controller address (if retrieved using #GAPM_GET_INFO_CMD: #GAPM_GET_CONTROLLER_BDADDR operation).
    /// Device identity (if retrieved using #GAPM_GET_INFO_CMD: #GAPM_GET_IDENTITY operation).
    /// Message can also be triggered by an activity when the resolvable or non-resolvable private address is renew.
    GAPM_BDADDR_IND = MSG_ID(GAPM, 0x12u),
    /// Advertising channel Tx power level
    GAPM_LE_ADV_TX_POWER_IND = MSG_ID(GAPM, 0x13u),
    /// Debug Indication containing information about memory usage.
    GAPM_DBG_MEMORY_INFO_IND = MSG_ID(GAPM, 0x14u),
    /// Indication containing controller antenna information
    GAPM_LE_ANTENNA_INFO_IND = MSG_ID(GAPM, 0x15u),
    /// Suggested Default Data Length indication
    GAPM_LE_SUGGESTED_DEFAULT_DATA_LEN_IND = MSG_ID(GAPM, 0x16u),
    /// Maximum Data Length indication
    GAPM_LE_MAX_DATA_LEN_IND = MSG_ID(GAPM, 0x17u),
    /// Indicate maximum advertising data length supported by controller
    /// see #gapm_le_max_adv_data_len_ind_t
    GAPM_LE_MAX_ADV_DATA_LEN_IND = MSG_ID(GAPM, 0x18u),
    /// Indicate number of available advertising sets
    /// see #gapm_le_nb_adv_sets_ind_t
    GAPM_LE_NB_ADV_SETS_IND = MSG_ID(GAPM, 0x19u),
    /// Indicate the transmit powers supported by the controller
    /// see #gapm_le_tx_power_ind_t
    GAPM_LE_TX_POWER_IND = MSG_ID(GAPM, 0x1Au),
    /// Indicate the RF path compensation values
    /// see #gapm_le_rf_path_compensation_ind_t
    GAPM_LE_RF_PATH_COMPENSATION_IND = MSG_ID(GAPM, 0x1Bu),
    /// Debug Indication containing statistics of the system.
    GAPM_DBG_STATISTICS_IND = MSG_ID(GAPM, 0x1Cu),
    /// Indicate list of features supported by controller (for split-host configuration only)
    GAPM_FEATURES_IND = MSG_ID(GAPM, 0x1Du),
    /// Indicate controller hardware error.
    GAPM_HW_ERROR_IND = MSG_ID(GAPM, 0x1Eu),

    /* Security / Encryption Toolbox */
    /// (AES command) Resolve address command
    /// see #gapm_le_resolve_address
    GAPM_LE_RESOLVE_ADDR_CMD = MSG_ID(GAPM, 0x30u),
    /// Indicate that resolvable random address has been solved
    GAPM_LE_ADDR_RESOLVED_IND = MSG_ID(GAPM, 0x31u),
    /// (AES command) Generate a random address.
    /// see #gapm_le_generate_random_addr
    /// @note Running in parallel different AES command from different task is dangerous,
    ///       result could be provided to last task which request AES function.
    GAPM_LE_GENERATE_RANDOM_ADDR_CMD = MSG_ID(GAPM, 0x32u),
    /// (AES command) Use the AES-128 block in the controller
    /// see #gapm_le_aes_cipher
    GAPM_LE_USE_ENCRYPTION_BLOCK_CMD = MSG_ID(GAPM, 0x33u),
    ///  AES-128 block result indication
    GAPM_LE_ENCRYPTION_RESULT_IND = MSG_ID(GAPM, 0x34u),
    /// (AES command) Generate a 8-byte random number
    /// see gapm_le_generate_random_nb
    GAPM_LE_GENERATE_RANDOM_NB_CMD = MSG_ID(GAPM, 0x35u),
    /// Random Number Indication
    GAPM_LE_RANDOM_NB_IND = MSG_ID(GAPM, 0x36u),
    /// Request to provide DH Key
    GAPM_LE_COMPUTE_DH_KEY_CMD = MSG_ID(GAPM, 0x39u),
    /// Indicates the DH Key computation is complete and available
    GAPM_LE_DH_KEY_IND = MSG_ID(GAPM, 0x3Au),
    /// Retrieve Public Key
    GAPM_LE_GET_PUBLIC_KEY_CMD = MSG_ID(GAPM, 0x3Bu),
    /// Indicates the Public Key Pair value
    GAPM_LE_PUBLIC_KEY_IND = MSG_ID(GAPM, 0x3Cu),
    /// Generate some OOB Data before a secure connection paring
    GAPM_GENERATE_OOB_DATA_CMD = MSG_ID(GAPM, 0x3Du),
    /// Generated LE OOB Data for a following secure connection paring
    GAPM_LE_OOB_DATA_IND = MSG_ID(GAPM, 0x3Eu),
    /// Generated BT Classic OOB Data for a following secure connection paring
    GAPM_BT_OOB_DATA_IND = MSG_ID(GAPM, 0x3Fu),

    /* List Management Operations */
    /// Get local or peer address
    /// see #gapm_le_get_ral_addr_cmd_t
    GAPM_LE_GET_RAL_ADDR_CMD = MSG_ID(GAPM, 0x50u),
    /// Resolving address list address indication
    GAPM_LE_RAL_ADDR_IND = MSG_ID(GAPM, 0x51u),
    #if (HL_DEPRECATED_LIST)
    /// Set content of either filter accept list or resolving list or periodic advertiser list (DEPRECATED).\n
    /// See #gapm_le_fill_fal_cmd_t\n
    /// See #gapm_le_fill_ral_cmd_t\n
    /// See #gapm_le_fill_pal_cmd_t
    GAPM_LE_SET_LIST_CMD = MSG_ID(GAPM, 0x52u),
    #endif // (HL_DEPRECATED_LIST)
    /// Indicate size of list indicated in GAPM_GET_DEV_CONFIG_CMD message
    /// see #gapm_le_list_size_ind_t
    GAPM_LE_LIST_SIZE_IND = MSG_ID(GAPM, 0x53u),
    /// Update content of either Filter Accept list or Resolving list or Periodic Advertiser list\n
    /// or Monitored Advertisers list.\n
    /// See #gapm_le_list_update_cmd_t
    GAPM_LE_LIST_UPDATE_CMD = MSG_ID(GAPM, 0x54u),
    /// Set privacy mode for a device in Resolving list
    /// See #gapm_le_list_set_privacy_mode_cmd_t
    GAPM_LE_LIST_SET_PRIVACY_MODE_CMD = MSG_ID(GAPM, 0x55u),

    /* Air Operations */
    /// Create an advertising, a scanning, an initiating or a periodic synchronization activity
    /// see #gapm_activity_create_cmd_t
    /// see #gapm_le_activity_create_adv_cmd_t
    ///
    /// If #gapm_activity_create_cmd_t.own_addr_type equals either #GAPM_GEN_RSLV_ADDR or #GAPM_GEN_NON_RSLV_ADDR,
    /// #GAPM_BDADDR_IND is triggered to indicate resolvable or non-resolvable address generated for the activity.
    /// This message is also triggered when address is renewed.
    GAPM_ACTIVITY_CREATE_CMD = MSG_ID(GAPM, 0x60u),
    /// Start a previously created activity
    /// see #gapm_activity_start_cmd_t
    GAPM_ACTIVITY_START_CMD = MSG_ID(GAPM, 0x61u),
    /// Stop either a given activity or all existing activities
    /// see #gapm_activity_stop_cmd_t
    GAPM_ACTIVITY_STOP_CMD = MSG_ID(GAPM, 0x62u),
    /// Delete either a given activity or all existing activities
    /// see #gapm_activity_delete_cmd_t
    GAPM_ACTIVITY_DELETE_CMD = MSG_ID(GAPM, 0x63u),
    /// Indicate that an activity has well been created
    /// see #gapm_activity_created_ind_t
    GAPM_ACTIVITY_CREATED_IND = MSG_ID(GAPM, 0x64u),
    /// Indicate that an activity has been stopped and can be restarted
    /// see #gapm_activity_stopped_ind_t
    GAPM_ACTIVITY_STOPPED_IND = MSG_ID(GAPM, 0x65u),
    /// Set either advertising data or scan response data or periodic advertising data
    /// see #gapm_le_set_adv_data_cmd_t
    GAPM_LE_SET_ADV_DATA_CMD = MSG_ID(GAPM, 0x66u),
    /// Indicate reception of an advertising report (periodic or not), a scan response report
    /// see #gapm_le_adv_report_ind_t
    GAPM_LE_ADV_REPORT_IND = MSG_ID(GAPM, 0x67u),
    /// Indicate reception of a scan request
    /// see #gapm_le_scan_request_ind_t
    GAPM_LE_SCAN_REQUEST_IND = MSG_ID(GAPM, 0x68u),
    #if (BLE_PER_ADV)
    /// Indicate that synchronization has been successfully established with a periodic advertiser
    /// see #gapm_le_per_sync_established_ind_t
    GAPM_LE_PER_SYNC_ESTABLISHED_IND = MSG_ID(GAPM, 0x69u),
    /// Control reception or not of Periodic Advertising Report in a Periodic Advertising Sync activity
    /// see #gapm_le_control_per_sync_report_cmd_t
    GAPM_LE_CONTROL_PER_ADV_REPORT_CMD = MSG_ID(GAPM, 0x6Au),
    /// Control capturing IQ samples from the Constant Tone Extension of periodic advertising packets
    GAPM_LE_CONTROL_PER_SYNC_IQ_SAMPLING_CMD = MSG_ID(GAPM, 0x6Bu),
    /// Indicate reception of a IQ Report event over a periodic advertising sync activity
    GAPM_LE_PER_ADV_IQ_REPORT_IND = MSG_ID(GAPM, 0x6Cu),
    /// Control CTE transmission in a periodic advertising activity
    GAPM_LE_CONTROL_PER_ADV_CTE_TX_CMD = MSG_ID(GAPM, 0x6Du),
    #endif // (BLE_PER_ADV)
    /// Name of peer device retrieve from device name query procedure
    GAPM_PEER_NAME_IND = MSG_ID(GAPM, 0x6Eu),
    #if (BLE_BIS)
    /// Indicate reception of periodic advertising report that contains BIGInfo data
    /// see #gapm_le_big_info_ind_t
    GAPM_LE_BIG_INFO_IND = MSG_ID(GAPM, 0x6Fu),
    #endif // (BLE_BIS)
    /// Reception of an Inquiry report
    GAPM_BT_INQUIRY_REPORT_IND = MSG_ID(GAPM, 0x70u),
    /// Set policy for resolvable private address update when advertising data is updated
    GAPM_LE_SET_ADV_ADDR_CHANGE_REASONS_CMD = MSG_ID(GAPM, 0x71u),
    /// Peer features discovered during remote name discovery
    GAPM_BT_PEER_FEATURES_IND = MSG_ID(GAPM, 0x72u),
    #if (BLE_PER_ADV)
    /// Stop extended advertising for Periodic Advertising (with Response) activity
    /// See #gapm_le_per_adv_stop_synchronizability_cmd_t
    GAPM_LE_PER_ADV_STOP_SYNCHRONIZABILITY_CMD = MSG_ID(GAPM, 0x87u),
    #endif // (BLE_PER_ADV)
    /* LE Test Mode */
    /// Control of the test mode command
    GAPM_LE_CONTROL_TEST_MODE_CMD = MSG_ID(GAPM, 0x90u),
    /// Indicate end of test mode
    GAPM_LE_TEST_END_IND = MSG_ID(GAPM, 0x91u),
    /// Indicate reception of a IQ report in LE test mode
    GAPM_LE_TEST_IQ_REPORT_IND = MSG_ID(GAPM, 0x92u),

    /* BT Test Mode */
    /// Allows the local BR/EDR controller to enter test mode via LMP test commands.
    /// \if btdm see #gapm_bt_write_loopback_mode \endif
    GAPM_BT_WRITE_LOOPBACK_MODE_CMD = MSG_ID(GAPM, 0x93u),
    /// Allows the local BR/EDR controller to enter test mode via LMP test commands.
    /// \if btdm see #gapm_bt_write_simple_pairing_debug_mode \endif
    GAPM_BT_ENABLE_DEVICE_UNDER_TEST_MODE_CMD = MSG_ID(GAPM, 0x94u),
    /// Configures the BR/EDR controller to use a predefined Diffie Hellman private key for simple pairing
    /// \if btdm see #gapm_bt_write_simple_pairing_debug_mode \endif
    GAPM_BT_WRITE_SIMPLE_PAIRING_DEBUG_MODE_CMD = MSG_ID(GAPM, 0x95u),
    /// Configures the BR/EDR controller to enable and disable the two test modes
    /// \if btdm see #gapm_bt_write_secure_connections_test_mode \endif
    GAPM_BT_WRITE_SECURE_CONNECTIONS_TEST_MODE_CMD = MSG_ID(GAPM, 0x96u),
    /// Event triggered when executed when reads controller's loopback mode procedure is completed
    /// \if btdm see #GAPM_BT_READ_LOOPBACK_MODE \endif
    GAPM_BT_LOOPBACK_IND = MSG_ID(GAPM, 0x97),

    /* Channel Scan Activity */
    /// Set Channel Scan Activity
    GAPM_LE_CONTROL_CH_SCAN_CMD = MSG_ID(GAPM, 0x98u),

    /* Profile Management */
    /// Create new task for specific profile
    GAPM_ADD_PROFILE_CMD = MSG_ID(GAPM, 0xA0u),
    /// Inform that profile task has been added.
    GAPM_PROFILE_ADDED_IND = MSG_ID(GAPM, 0xA1u),

    /* Codecs */
    /// Get list of codecs supported by the controller\n
    /// (#GAPM_CODEC_GET_CMP_EVT is sent back once operation has been completed)
    GAPM_CODEC_GET_CMD = MSG_ID(GAPM, 0xA2u),
    /// Get list of codec capabilities supported by the controller for a given codec\n
    /// (#GAPM_CODEC_GET_CAPABILITIES_CMP_EVT is sent back once operation has been completed)
    GAPM_CODEC_GET_CAPABILITIES_CMD = MSG_ID(GAPM, 0xA3u),
    /// Get range of supported controller delays for a specified configuration of a given codec\n
    /// (#GAPM_CODEC_GET_DELAY_CMP_EVT is sent back once operation has been completed)
    GAPM_CODEC_GET_DELAY_CMD = MSG_ID(GAPM, 0xA4u),
    /// Complete event message for #GAPM_CODEC_GET_CMD message
    GAPM_CODEC_GET_CMP_EVT = MSG_ID(GAPM, 0xA5u),
    /// Complete event message for #GAPM_CODEC_GET_CAPABILITIES_CMD message
    GAPM_CODEC_GET_CAPABILITIES_CMP_EVT = MSG_ID(GAPM, 0xA6u),
    /// Complete event message for #GAPM_CODEC_GET_DELAY_CMD message
    GAPM_CODEC_GET_DELAY_CMP_EVT = MSG_ID(GAPM, 0xA7u),

    /* Unified Test Protocol */
    /// Enable/disable use of Unified Test Protocol over the air
    GAPM_LE_UTP_ENABLE_OTA_CMD = MSG_ID(GAPM, 0xB0u),

    /* ************************************************ */
    /* ------------ Vendor specific commands ---------- */
    /* ************************************************ */
    /// Provide all needed priority/increment pairs as a buffer that is copied directly to the priority table.\n
    /// This message should be sent before any activity is started in order to avoid unexpected scheduling issues.\n
    /// Activities whose priority increment is 0 no longer benefit from the random increment to ensure that their
    /// priority stays the same.\n
    /// See #gapm_vs_set_sched_prio_cmd_t
    GAPM_VS_SET_SCHED_PRIO_CMD = MSG_ID(GAPM, 0xD0u),

    /* ************************************************ */
    /* ---------------- Debug commands ---------------- */
    /* ************************************************ */
    /// Configure the Debug Platform I&Q Sampling generator
    GAPM_LE_DBG_CONFIGURE_IQGEN_CMD = MSG_ID(GAPM, 0xE0u),
    /// Undocumented security test command - Debug purpose only
    GAPM_LE_DBG_SECURITY_TEST_CMD = MSG_ID(GAPM, 0xE1u),
    /// Undocumented security test result - Debug purpose only
    GAPM_LE_DBG_SECURITY_TEST_IND = MSG_ID(GAPM, 0xE2u),

    /* ************************************************ */
    /* -------------- Internal usage only ------------- */
    /* ************************************************ */
    /// Message received to unknown task received
    GAPM_UNKNOWN_TASK_MSG = MSG_ID(GAPM, 0xF0u),
};

/// Meaning of bit field indicating features supported by Host
enum gapm_features_bf
{
    /// Bluetooth Low Energy Observer role
    GAPM_FEAT_BYTE0_LE_ROLE_OBSERVER_BIT = CO_BIT(0),
    /// Bluetooth Low Energy Broadcaster role
    GAPM_FEAT_BYTE0_LE_ROLE_BROADCASTER_BIT = CO_BIT(1),
    /// Bluetooth Low Energy Central role
    GAPM_FEAT_BYTE0_LE_ROLE_CENTRAL_BIT = CO_BIT(2),
    /// Bluetooth Low Energy Peripheral role
    GAPM_FEAT_BYTE0_LE_ROLE_PERIPHERAL_BIT = CO_BIT(3),
    /// Bluetooth Classic
    GAPM_FEAT_BYTE0_BT_BIT = CO_BIT(4),
    /// Broadcast Isochronous Stream
    GAPM_FEAT_BYTE0_BIS_BIT = CO_BIT(5),
    /// Connected Isochronous Stream
    GAPM_FEAT_BYTE0_CIS_BIT = CO_BIT(6),
    /// GATT Client
    GAPM_FEAT_BYTE0_GATT_CLI_BIT = CO_BIT(7),

    /// GATT Caching
    GAPM_FEAT_BYTE1_GATT_CACHING_BIT = CO_BIT(0),
    /// Enhanced ATT
    GAPM_FEAT_BYTE1_EATT_BIT = CO_BIT(1),
    /// Periodic Advertising
    GAPM_FEAT_BYTE1_PER_ADV_BIT = CO_BIT(2),
    /// Periodic Advertising Sync Transfer
    GAPM_FEAT_BYTE1_PAST_BIT = CO_BIT(3),
    /// Low Energy Power Control
    GAPM_FEAT_BYTE1_LE_PWR_CTRL_BIT = CO_BIT(4),
    /// Angle of Arrival
    GAPM_FEAT_BYTE1_AOA_BIT = CO_BIT(5),
    /// Angle of Departure
    GAPM_FEAT_BYTE1_AOD_BIT = CO_BIT(6),
    /// Subrating
    GAPM_FEAT_BYTE1_SUBRATING_BIT = CO_BIT(7),

    /// TWS LE Forwarding
    GAPM_FEAT_BYTE2_TWS_LE_FWD_BIT = CO_BIT(0),
    /// Encrypted Advertising Data
    GAPM_FEAT_BYTE2_EAD_BIT = CO_BIT(1),
    /// Security Levels characteristic
    GAPM_FEAT_BYTE2_SEC_LEVELS_CHAR_BIT = CO_BIT(2),
    /// Periodic Advertising With Response
    GAPM_FEAT_BYTE2_PAWR_BIT = CO_BIT(3),
    /// Coding Scheme Selection on Advertising
    GAPM_FEAT_BYTE2_CSSA_BIT = CO_BIT(4),
    /// GAF
    GAPM_FEAT_BYTE2_GAF_BIT = CO_BIT(5),
    /// Post 5.3 API - For internal use and testing only
    GAPM_FEAT_BYTE2_POST_53_API_BIT = CO_BIT(6),
    /// Automatic sending of Service Changed indication (DEPRECATED)
    GAPM_FEAT_BYTE2_OLD_SERVICE_CHANGED_BIT = CO_BIT(7),

    /// Old list content (FAL / RAL / PAL) management API (DEPRECATED)
    GAPM_FEAT_BYTE3_OLD_LIST_BIT = CO_BIT(0),
    /// Automatically add AD Type Flags in advertising data
    GAPM_FEAT_BYTE3_ADV_ADD_FLAGS_BIT = CO_BIT(1),
    /// Channel Sounding
    GAPM_FEAT_BYTE3_CHSD_BIT = CO_BIT(2),
    /// Channel Sounding - Test Mode
    GAPM_FEAT_BYTE3_CHSD_TEST_BIT = CO_BIT(3),
    /// Unified Test Protocol
    GAPM_FEAT_BYTE3_UTP_BIT = CO_BIT(4),
    /// Frame Space Update
    GAPM_FEAT_BYTE3_FSU_BIT = CO_BIT(5),
    /// Monitored Advertisers List
    GAPM_FEAT_BYTE3_MAL_BIT = CO_BIT(6),
    /// Constant Tone Extension connected responder
    GAPM_FEAT_BYTE3_CON_CTE_RSP_BIT = CO_BIT(7),

    /// Constant Tone Extension connected requester
    GAPM_FEAT_BYTE4_CON_CTE_REQ_BIT = CO_BIT(0),
    /// Constant Tone Extension connection less transmiter
    GAPM_FEAT_BYTE4_CONLESS_CTE_TX_BIT = CO_BIT(1),
    /// Constant Tone Extension connection less receiver
    GAPM_FEAT_BYTE4_CONLESS_CTE_RX_BIT = CO_BIT(2),

};

/// List of LTV type values for #GAPM_VERSION_IND message
enum gapm_version_type
{
    /// Supported Profiles\n
    /// Length shall be equal to number of 1-byte Profile Identifiers\n
    /// If present, unique in the LTV string
    GAPM_VERSION_TYPE_SUPPORTED_PROFILES = 0u,
    /// Supported Features bit field\n
    /// Length set to #GAPM_VERSION_LENGTH_SUPPORTED_FEATURES\n
    /// See #gapm_features_bf enumeration for bit field meaning\n
    GAPM_VERSION_TYPE_SUPPORTED_FEATURES,
    /// Profile SIG Version\n
    /// Length set to #GAPM_VERSION_LENGTH_PROFILE_SIG_VERSION\n
    ///     - Byte 0: Profile Task Identifier
    ///     - Byte 1: SIG Version X value (X.Y.Z)
    ///     - Byte 2: SIG Version Y value (X.Y.Z)
    ///     - Byte 3: SIG Version Z value (X.Y.Z)
    /// Can be present several times in the LTV string
    GAPM_VERSION_TYPE_PROFILE_SIG_VERSION,
};

/// Length of LTV values for #GAPM_VERSION_IND message
enum gapm_version_length
{
    /// Profile SIG Version (#GAPM_VERSION_TYPE_PROFILE_SIG_VERSION)
    GAPM_VERSION_LENGTH_PROFILE_SIG_VERSION = 4u,
    /// Length of Supported Features bit field value (#GAPM_VERSION_TYPE_SUPPORTED_FEATURES)
    GAPM_VERSION_LENGTH_SUPPORTED_FEATURES = 5u,
};
/// @} GAPM_MSG_ID_API

/// @addtogroup GAPM_MSG_OPID_API Operation Identifiers
/// @ingroup GAPM_MSG_API
/// @{

/// GAP Manager operation type - application interface
/*@TRACE*/
enum gapm_operation
{
    /* No Operation (if nothing has been requested)     */
    /* ************************************************ */
    /// No operation.
    GAPM_NO_OP = 0x00u,

    /* Configuration operations                         */
    /* ************************************************ */
    /// Reset BLE subsystem: LL and HL.
    GAPM_RESET = 0x01u,
    /// Perform a platform reset - Debug only
    GAPM_PLATFORM_RESET = 0x02u,
    /// Set device configuration
    GAPM_CONFIGURE = 0x03u,
    /// Set LE device channel map
    GAPM_LE_SET_CHANNEL_MAP = 0x04u,
    /// Set IRK
    GAPM_LE_SET_IRK = 0x05u,
    /// Set Device Name
    GAPM_SET_NAME = 0x06u,
    /// Insert into SDP a device identification record
    /// \if btdm see #gapm_bt_set_sdp_identification_record \endif
    GAPM_BT_SET_SDP_IDENTIFICATION_RECORD = 0x07u,
    /// Set acceptable parameters for subrating requests
    GAPM_LE_SET_DEFAULT_SUBRATE = 0x08u,
    /// Set Rx Max Slot
    GAPM_BT_SET_RX_MAX_SLOT = 0x09u,
    #if (BLE_GAPC)
    /// Configure security level required for LE connections
    GAPM_LE_CONFIGURE_SECURITY_LEVEL = 0x0Au,
    #endif // (BLE_GAPC)
    /// Disable a supported feature
    GAPM_DISABLE_FEATURE = 0x0Bu,
    /// Set Appearance
    GAPM_LE_SET_APPEARANCE = 0x0Cu,

    /* Retrieve device information                      */
    /* ************************************************ */
    /// Get Local device version
    /// (see #gapm_get_version)
    GAPM_GET_VERSION = 0x10u,
    /// Get Local device BD Address
    /// (see #gapm_get_controller_bdaddr)
    GAPM_GET_CONTROLLER_BDADDR = 0x11u,
    /// Get device advertising power level
    GAPM_LE_GET_ADV_TX_POWER = 0x12u,
    /// Get Filter Accept List Size.
    GAPM_LE_GET_FAL_SIZE = 0x13u,
    /// Retrieve Antenna information
    GAPM_LE_GET_ANTENNA_INFO = 0x14u,
    /// Get memory usage - Debug only
    GAPM_DBG_GET_MEMORY_INFO = 0x15u,
    /// Get Suggested Default LE Data Length
    GAPM_LE_GET_SUGGESTED_DEFAULT_DATA_LEN = 0x16u,
    /// Get Maximum LE Data Length
    GAPM_LE_GET_MAX_DATA_LEN = 0x17u,
    /// Get number of available advertising sets
    GAPM_LE_GET_NB_ADV_SETS = 0x18u,
    /// Get maximum advertising data length supported by the controller
    GAPM_LE_GET_MAX_ADV_DATA_LEN = 0x19u,
    /// Get minimum and maximum transmit powers supported by the controller
    GAPM_LE_GET_TX_POWER = 0x1Au,
    /// Get the RF Path Compensation values used in the TX Power Level and RSSI calculation
    GAPM_LE_GET_RF_PATH_COMPENSATION = 0x1Bu,
    /// Get statistics - Debug only
    GAPM_DBG_GET_STATISTICS = 0x1Cu,
    /// Get host identity BD Address
    /// (see #gapm_get_identity)
    GAPM_GET_IDENTITY = 0x1Du,
    /// Get list of features supported by controller (for split-host configuration only)\n
    /// Shall be sent after #GAPM_CONFIGURE procedure has been completed
    GAPM_GET_FEATURES = 0x1Eu,

    /* Security / Encryption Toolbox                    */
    /* ************************************************ */
    /// Resolve device address
    GAPM_LE_RESOLVE_ADDR = 0x30u,
    /// Generate a random address
    GAPM_LE_GENERATE_RANDOM_ADDR = 0x31u,
    /// Use the controller's AES-128 block
    GAPM_LE_USE_ENCRYPTION_BLOCK = 0x32u,
    /// Generate a 8-byte random number
    GAPM_LE_GENERATE_RANDOM_NB = 0x33u,
    /// Generate DH_Key
    GAPM_LE_COMPUTE_DH_KEY = 0x34u,
    /// Retrieve Public Key
    GAPM_LE_GET_PUBLIC_KEY = 0x35u,
    /// Generate LE OOB Data
    GAPM_LE_GENERATE_OOB_DATA = 0x36u,
    /// Generate BT Classic OOB Data
    GAPM_BT_GENERATE_OOB_DATA = 0x37u,

    /* List Management                                  */
    /* ************************************************ */
    /// Get resolving address list size
    GAPM_LE_GET_RAL_SIZE = 0x50u,
    /// Get resolving local address
    GAPM_LE_GET_RAL_LOCAL_RPA = 0x51u,
    /// Get resolving peer address
    GAPM_LE_GET_RAL_PEER_RPA = 0x52u,
    #if (HL_DEPRECATED_LIST)
    /// Set content of filter accept list
    GAPM_LE_FILL_FAL = 0x53u,
    /// Set content of resolving list
    GAPM_LE_FILL_RAL = 0x54u,
    #if (BLE_PER_ADV)
    /// Set content of periodic advertiser list
    GAPM_LE_FILL_PAL = 0x55u,
    #endif // (BLE_PER_ADV)
    #endif // (HL_DEPRECATED_LIST)
    #if (BLE_PER_ADV)
    /// Get periodic advertiser list size
    GAPM_LE_GET_PAL_SIZE = 0x56u,
    #endif // (BLE_PER_ADV)
    /// Add entry to either Filter Accept list or Resolving list or Periodic Advertiser list
    GAPM_LE_LIST_ADD = 0x57u,
    /// Remove entry from either Filter Accept list or Resolving list or Periodic Advertiser list
    GAPM_LE_LIST_REMOVE = 0x58u,
    /// Clear content of either Filter Accept list or Resolving list or Periodic Advertiser list
    GAPM_LE_LIST_CLEAR = 0x59u,
    /// Set privacy mode for a device in Resolving list
    GAPM_LE_LIST_SET_PRIVACY_MODE = 0x5Au,

    /* Air Operations                                   */
    /* ************************************************ */
    /// Create advertising activity
    GAPM_LE_CREATE_ADV = 0x60u,
    /// Create scanning activity
    GAPM_LE_CREATE_SCAN = 0x61u,
    /// Create initiating activity
    GAPM_LE_CREATE_INIT = 0x62u,
    #if (BLE_PER_ADV)
    /// Create periodic synchronization activity
    GAPM_LE_CREATE_PER_SYNC = 0x63u,
    #endif // (BLE_PER_ADV)
    /// Start an activity
    GAPM_START_ACTIVITY = 0x64u,
    /// Stop an activity
    GAPM_STOP_ACTIVITY = 0x65u,
    /// Delete an activity
    GAPM_DELETE_ACTIVITY = 0x67u,
    /// Set advertising data
    GAPM_LE_SET_ADV_DATA = 0x69u,
    /// Set scan response data
    GAPM_LE_SET_SCAN_RESPONSE_DATA = 0x6Au,
    #if (BLE_PER_ADV)
    /// Set periodic advertising data
    GAPM_LE_SET_PER_ADV_DATA = 0x6Bu,
    /// Enable/Disable reception of periodic advertising report
    GAPM_LE_CONTROL_PER_SYNC_REPORT = 0x6Cu,
    /// Enable / Disable IQ sampling
    GAPM_LE_CONTROL_PER_SYNC_IQ_REPORT = 0x6Du,
    /// Enable / Disable CTE transmission
    GAPM_LE_CONTROL_PER_ADV_CTE_TX = 0x6Eu,
    /// Fake update of periodic advertising data in order to update the Data ID (DID)
    GAPM_LE_SET_PER_ADV_DATA_FAKE = 0x6Fu,
    #endif // (BLE_PER_ADV)
    /// Create inquiry activity
    GAPM_BT_CREATE_INQUIRY = 0x70u,
    /// Create inquiry scan activity
    GAPM_BT_CREATE_INQUIRY_SCAN = 0x71u,
    /// Create page activity
    GAPM_BT_CREATE_PAGE = 0x72u,
    /// Create page scan activity
    GAPM_BT_CREATE_PAGE_SCAN = 0x73u,
    /// Set policy for resolvable private address update when advertising data is updated
    GAPM_LE_SET_ADV_ADDR_CHANGE_REASONS = 0x74u,
    /// Stop extended advertising for Periodic Advertising (with Response) activity
    GAPM_LE_PER_ADV_STOP_SYNCHRONIZABILITY = 0x83u,

    /* LE Direct Test Mode                              */
    /* ************************************************ */
    /// Stop the test mode
    GAPM_LE_STOP_TEST = 0x90u,
    /// Start RX Test Mode
    GAPM_LE_START_TEST_RX = 0x91u,
    /// Start TX Test Mode
    GAPM_LE_START_TEST_TX = 0x92u,

    /* BT Test Mode                                     */
    /* ************************************************ */
    /// Allows the local BR/EDR controller to enter test mode via LMP test commands.
    GAPM_BT_WRITE_LOOPBACK_MODE = 0x93u,
    /// Configures the BR/EDR controller to use a predefined Diffie Hellman private key
    GAPM_BT_ENABLE_DEVICE_UNDER_TEST_MODE = 0x94u,
    /// Configures the BR/EDR controller to use a predefined Diffie Hellman private key for simple pairing
    GAPM_BT_WRITE_SIMPLE_PAIRING_DEBUG_MODE = 0x95u,
    /// Configures the BR/EDR controller to enable and disable the two test modes
    GAPM_BT_WRITE_SECURE_CONNECTIONS_TEST_MODE = 0x96u,
    /// Read BT loopback mode using #GAPM_GET_INFO_CMD
    /// \if btdm see #gapm_bt_read_loopback_mode \endif
    GAPM_BT_READ_LOOPBACK_MODE = 0x97u,

    /* Channel Scan Activity                            */
    /* ************************************************ */
    /// Start Channel Scan Activity
    GAPM_LE_START_CH_SCAN = 0x98u,
    /// Stop Channel Scan Activity
    GAPM_LE_STOP_CH_SCAN = 0x99u,

    /* Profile Management                               */
    /* ************************************************ */
    /// Create new task for specific profile
    GAPM_ADD_PROFILE = 0xA0u,

    /* Codecs                                           */
    /* ************************************************ */
    /// Get list of codecs supported by the controller
    GAPM_CODEC_GET = 0xA1u,
    /// Get list of codec capabilities supported by the controller for a given codec
    GAPM_CODEC_GET_CAPABILITIES = 0xA2u,
    /// Get range of supported controller delays for a specified configuration of a given codec
    GAPM_CODEC_GET_DELAY = 0xA3u,

    /* LE Unified Test Protocol                         */
    /* ************************************************ */
    /// Control Unified Test Protocol
    GAPM_LE_UTP_ENABLE_OTA = 0xB0u,

    /* Vendor Specific Commands                         */
    /* ************************************************ */
    /// Provide all needed priority/increment pairs as a buffer that is copied directly to the priority table
    GAPM_VS_SET_SCHED_PRIO = 0xD0u,

    /* Debug Commands                                   */
    /* ************************************************ */
    /// Configure the Debug Platform I&Q Sampling generator
    GAPM_LE_DBG_CONFIGURE_IQGEN = 0xE0u,
    /// Undocumented security test command - Debug purpose only
    GAPM_LE_DBG_SECURITY_TEST = 0xE1u,
};
/// @} GAPM_MSG_OPID_API

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPM_MSG_STRUCT_API Message Structures
/// @ingroup GAPM_MSG_API
/// @{

/// Operation command structure in order to keep requested operation.
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    uint8_t operation;
} gapm_operation_cmd_t;

/// Command complete event data structure
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    uint8_t operation;
    /// Status of the request
    uint16_t status;
    /// Activity index (valid only for air operation, else discard)
    uint8_t actv_idx;
} gapm_cmp_evt_t;

///  Reset link layer and the host command
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    /// - #GAPM_RESET: Reset BLE subsystem: LL and HL.
    uint8_t operation;
} gapm_reset_cmd_t;

/// Set device configuration command
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_CONFIGURE: Set device configuration
    uint8_t operation;
    /// Device configuration
    gapm_config_t cfg;
} gapm_configure_cmd_t;

/// Parameters of #GAPM_SET_NAME_CMD
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_SET_NAME: Set device name
    uint8_t operation;
    /// Size of the device name
    uint8_t name_len;
    /// Device Name
    uint8_t name[__ARRAY_EMPTY];
} gapm_set_name_cmd_t;

/// Parameters of #GAPM_LE_SET_APPEARANCE_CMD
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_SET_APPEARANCE: Set Appearance
    uint8_t operation;
    /// Appearance
    uint16_t appearance;
} gapm_le_set_appearance_cmd_t;

/// Parameters of #GAPM_GET_INFO_CMD
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_GET_VERSION: Get Local device version
    ///  - #GAPM_GET_CONTROLLER_BDADDR: Get controller BD Address
    ///  - #GAPM_GET_IDENTITY: Get host identity address
    ///  - #GAPM_LE_GET_ADV_TX_POWER: Get device advertising power level
    ///  - #GAPM_DBG_GET_MEMORY_INFO: Get memory usage (debug only)
    ///  - #GAPM_DBG_GET_STATISTICS: Get device statistics
    ///  - #GAPM_LE_GET_SUGGESTED_DEFAULT_DATA_LEN: Get Suggested Default LE Data Length
    ///  - #GAPM_LE_GET_MAX_DATA_LEN: Get Maximum LE Data Length
    ///  - #GAPM_LE_GET_FAL_SIZE: Get size of Filter Accept List
    ///  - #GAPM_LE_GET_PAL_SIZE: Get size of Periodic Address List
    ///  - #GAPM_LE_GET_RAL_SIZE: Get size of Resolving Address List
    ///  - #GAPM_LE_GET_NB_ADV_SETS: Read number of advertising sets currently supported by the controller
    ///  - #GAPM_LE_GET_MAX_ADV_DATA_LEN: Get maximum data length for advertising data
    ///  - #GAPM_LE_GET_ANTENNA_INFO: Retrieve Antenna information
    ///  - #GAPM_LE_GET_TX_POWER: Get minimum and maximum transmit power values supported by the controller
    ///  - #GAPM_LE_GET_RF_PATH_COMPENSATION: Get the RF Path Compensation values used in the TX Power Level and RSSI
    /// calculation
    ///  - #GAPM_BT_READ_LOOPBACK_MODE: Read BT loopback mode
    ///  - #GAPM_GET_FEATURES: Get list of features supported by controller
    uint8_t operation;
} gapm_get_info_cmd_t;

/// Parameters of #GAPM_DISABLE_FEATURE_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_DISABLE_FEATURE: Disable GAPM feature.
    uint8_t operation;
    /// Feature to be disabled descriptor (see #gapm_optional_features)
    uint16_t feature_idx;
} gapm_disable_feature_cmd_t;

/// Local device version indication event
/*@TRACE*/
typedef struct
{
    /// HCI version
    uint8_t hci_ver;
    /// LMP version
    uint8_t lmp_ver;
    /// Host version
    uint8_t host_ver;
    /// HCI revision
    uint16_t hci_subver;
    /// LMP subversion
    uint16_t lmp_subver;
    /// Host revision
    uint16_t host_subver;
    /// Manufacturer name
    uint16_t manuf_name;
    /// Length of information field
    uint16_t length;
    /// Information (in LTV format)\n
    /// See #gapm_version_type enumeration for list of available type and their meaning
    uint8_t info[__ARRAY_EMPTY];
} gapm_version_ind_t;

/// Parameters for #GAPM_FEATURES_IND message
/*@TRACE*/
typedef struct
{
    /// List of features
    le_features_t features;
} gapm_features_ind_t;

/// Parameters for #GAPM_HW_ERROR_IND message
/*@TRACE*/
typedef struct
{
    /// HW error code
    uint8_t code;

} gapm_hw_error_ind_t;

/// Parameters of @ref GAPM_BDADDR_IND message
/*@TRACE*/
typedef struct
{
    /// Address information
    gap_bdaddr_t addr;
    /// Activity index
    uint8_t actv_idx;
} gapm_bdaddr_ind_t;

/// Parameters of the @ref GAPM_GENERATE_OOB_DATA_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    /// - #GAPM_LE_GENERATE_OOB_DATA: LE OOB data
    /// - #GAPM_BT_GENERATE_OOB_DATA: BT-Classic OOB data
    uint8_t operation;
} gapm_generate_oob_data_cmd_t;

/// Create new task for specific profile
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_ADD_PROFILE: Add new profile task
    uint8_t operation;
    /// Service Security level - Only for a GATT service (see #gatt_svc_info_bf enumeration)
    uint8_t sec_lvl;
    /// GATT user priority
    uint8_t user_prio;
    /// Profile Application identifier (use profile identifier)
    uint16_t prf_api_id;
    /// Application task number
    uint16_t app_task;
    /// Service start handle
    /// Only applies for services - Ignored by collectors
    /// 0: dynamically allocated in Attribute database
    uint16_t start_hdl;
    /// 32 bits value that contains value to initialize profile (database parameters, etc...)
    uint32_t param[__ARRAY_EMPTY];
} gapm_add_profile_cmd_t;

/// Inform that profile task has been added.
/*@TRACE*/
typedef struct
{
    /// Profile task identifier
    uint16_t prf_task_id;
    /// Profile task number allocated
    uint16_t prf_task_nb;
    /// Service start handle
    /// Only applies for services - Ignored by collectors
    uint16_t start_hdl;
} gapm_profile_added_ind_t;

/// Indicate that a message has been received on an unknown task
/*@TRACE*/
typedef struct
{
    /// Message identifier
    uint16_t msg_id;
    /// Task identifier
    uint16_t task_id;
} gapm_unknown_task_ind_t;

/// Create an advertising, a scanning, an initiating, a periodic synchronization activity command (common)
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_LE_CREATE_ADV: Create advertising activity
    ///  - #GAPM_LE_CREATE_SCAN: Create scanning activity
    ///  - #GAPM_LE_CREATE_INIT: Create initiating activity
    ///  - #GAPM_LE_CREATE_PER_SYNC: Create periodic synchronization activity
    ///  - #GAPM_BT_CREATE_INQUIRY: Create inquiry activity
    ///  - #GAPM_BT_CREATE_INQUIRY_SCAN: Create inquiry scan activity
    ///  - #GAPM_BT_CREATE_PAGE: Create page activity
    ///  - #GAPM_BT_CREATE_PAGE_SCAN: Create page scan activity
    uint8_t operation;
    /// Own address type (see #gapm_le_own_addr enumeration)
    uint8_t own_addr_type;
} gapm_activity_create_cmd_t;

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
    uint16_t u_param[__ARRAY_EMPTY]; // 16-bit aligned parameters
} gapm_activity_start_cmd_t;

/// Stop one or all activity(ies) command
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_STOP_ACTIVITY: Stop a given activity
    uint8_t operation;
    /// Activity identifier - used only if operation is GAPM_STOP_ACTIVITY
    uint8_t actv_idx;
} gapm_activity_stop_cmd_t;

/// Delete one or all activity(ies) command
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_DELETE_ACTIVITY: Delete a given activity
    uint8_t operation;
    /// Activity identifier - used only if operation is GAPM_DELETE_ACTIVITY
    uint8_t actv_idx;
} gapm_activity_delete_cmd_t;

/// Indicate creation of an activity
/*@TRACE
 @trc_exec activity_map[$actv_idx] = $actv_type
 activity_map = {}*/
typedef struct
{
    /// Activity identifier
    uint8_t actv_idx;
    /// Activity type (see #gapm_actv_type enumeration)
    uint8_t actv_type;
    /// Selected TX power for advertising activity
    int8_t tx_pwr;
} gapm_activity_created_ind_t;

/// Indicate that an activity has been stopped
/*@TRACE*/
typedef struct
{
    /// Activity identifier
    uint8_t actv_idx;
    /// Activity type (see #gapm_actv_type enumeration)
    uint8_t actv_type;
    /// Activity stop reason (see #hl_err enumeration)
    uint16_t reason;
    /// In case of periodic advertising, indicate if periodic advertising has been stopped
    uint8_t per_adv_stop;
} gapm_activity_stopped_ind_t;

/// Indicate that an unknown message has been received
/*@TRACE*/
typedef struct
{
    /// Unknown message id
    uint16_t unknown_msg_id;
} gapm_unknown_msg_ind_t;

/// Name of peer device indication
/*@TRACE*/
typedef struct
{
    /// peer device bd address
    gap_bdaddr_t addr;
    /// peer device name length
    uint8_t name_len;
    /// peer device name
    uint8_t name[__ARRAY_EMPTY];
} gapm_peer_name_ind_t;


/// Parameters of #GAPM_CODEC_GET_CMD message
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_CODEC_GET
    uint8_t operation;
} gapm_codec_get_cmd_t;

/// Parameters of #GAPM_CODEC_GET_CAPABILITIES_CMD message
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_CODEC_GET_CAPABILITIES
    uint8_t operation;
    /// Codec ID
    uint8_t codec_id[CODEC_ID_LEN];
    /// Transport type (see #transport_id enumeration)
    uint8_t transport_type;
    /// Direction (see #data_path_direction enumeration)
    uint8_t direction;
} gapm_codec_get_capabilities_cmd_t;

/// Parameters of #GAPM_CODEC_GET_DELAY_CMD message
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_CODEC_GET_DELAY
    uint8_t operation;
    /// Codec ID
    uint8_t codec_id[CODEC_ID_LEN];
    /// Transport type (see #transport_id enumeration)
    uint8_t transport_type;
    /// Direction (see #data_path_direction enumeration)
    uint8_t direction;
    /// Length of codec configuration
    uint8_t codec_cfg_len;
    /// Codec configuration
    uint8_t codec_cfg[__ARRAY_EMPTY];
} gapm_codec_get_delay_cmd_t;

/// Parameters of #GAPM_CODEC_GET_CMP_EVT message
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_CODEC_GET
    uint8_t operation;
    /// Status (see #hl_err enumeration)
    uint16_t status;
    /// Version (see #gapm_codec_get_version enumeration)
    uint8_t version;
    /// Length of codecs list
    uint16_t codec_list_len;
    /// Codecs list
    /// Has following structure for version = #GAPM_CODEC_GET_VERSION_V1:\n
    ///     uint8_t nb_supp_std_codecs\n
    ///     uint8_t std_codec_id[nb_supp_std_codecs]\n
    ///     uint8_t nb_supp_vs_codecs\n
    ///     uint8_t vs_codec_id[4][nb_supp_vs_codecs]\n
    /// Has following structure for version = #GAPM_CODEC_GET_VERSION_V2:\n
    ///     uint8_t nb_supp_std_codecs\n
    ///     uint8_t std_codec_id[nb_supp_std_codecs]\n
    ///     uint8_t std_codec_transport_id[nb_supp_std_codecs]\n
    ///     uint8_t nb_supp_vs_codecs\n
    ///     uint8_t vs_codec_id[4][nb_supp_vs_codecs]\n
    ///     uint8_t vs_codec_transport_id[nb_supp_vs_codecs]
    uint8_t codec_list[__ARRAY_EMPTY];
} gapm_codec_get_cmp_evt_t;

/// Parameters of #GAPM_CODEC_GET_CAPABILITIES_CMP_EVT message
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_CODEC_GET_CAPABILITIES
    uint8_t operation;
    /// Status (see #hl_err enumeration)
    uint16_t status;
    /// Number of codecs capabilities
    uint8_t nb_codec_capa;
    /// Total length of codec capabilities
    uint16_t codec_capa_len;
    /// Codec capabilities\n
    /// Has following structure:\n
    ///     uint8_t codec_capa_len[i]\n
    ///     uint8_t codec_capa[i][codec_capa_len[i]]\n
    /// with i in range [0, nb_codec_capa]
    uint8_t codec_capa[__ARRAY_EMPTY];
} gapm_codec_get_capabilities_cmp_evt_t;

/// Parameters of #GAPM_CODEC_GET_DELAY_CMP_EVT message
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_CODEC_GET_CAPABILITIES
    uint8_t operation;
    /// Status (see #hl_err enumeration)
    uint16_t status;
    /// Minimal delay in microseconds
    uint32_t delay_min_us;
    /// Maximal delay in microseconds
    uint32_t delay_max_us;
} gapm_codec_get_delay_cmp_evt_t;

/// Parameters of #GAPM_VS_SET_SCHED_PRIO_CMD message
/*@TRACE*/
typedef struct
{
    /// Requested operation type (see #gapm_operation enumeration)
    ///  - #GAPM_VS_SET_SCHED_PRIO: Set scheduling priorities
    uint8_t operation;
    /// Number of priority/increment pairs\n
    /// Must be set to RWIP_PRIO_IDX_MAX
    uint8_t pair_count;
    /// Priority/increment pairs'n
    /// Length must be 2 * pair_count
    uint8_t prio_data[__ARRAY_EMPTY];
} gapm_vs_set_sched_prio_cmd_t;

/// Controller Hardware error indication
/*@TRACE*/
typedef struct gapm_ctrl_hw_err_ind
{
    /// See #enum co_error "HW ERROR CODES"
    uint8_t hw_err_code;
} gapm_ctrl_hw_err_ind_t;

/// @} GAPM_MSG_STRUCT_API

#endif /* GAPM_MSG_H_ */
