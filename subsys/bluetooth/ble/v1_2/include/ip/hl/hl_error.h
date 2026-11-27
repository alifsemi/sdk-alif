/**
 ****************************************************************************************
 *
 * @file hl_error.h
 *
 * @brief File that contains all error codes.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 *
 ****************************************************************************************
 */

#ifndef HL_ERROR_H_
#define HL_ERROR_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup HL_ERROR_API High Layer Error Codes
 * @brief High layer error codes
 *
 * This module contains the primitives that allow an application accessing and running the
 * RivieraWaves Host protocol stack
 *
 * @{
 ****************************************************************************************
 */

/*
 * MACROS
 ****************************************************************************************
 */

/// Error code from HCI TO HL Range - from 0x90 to 0xD0
#define RW_ERR_HCI_TO_HL(err)     (((err) != 0) ? (((uint16_t) err) + 0x90) : (0))
/// Error code from HL TO HCI Range - from 0x90 to 0xD0
#define RW_ERR_HL_TO_HCI(err)     (((err) > 0x90) ? ((err) - 0x90) : (0))
/// Mask for GAPI error code
#define HL_ERR_CODE_GAPI(idx)     (0x0100 | idx)
// !!!!! (0x0200 | idx) reserved for GAF

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List all HL error codes
enum hl_err
{
    /// No error
    GAP_ERR_NO_ERROR = 0x00, //!< Value: 0x00

    // ----------------------------------------------------------------------------------
    // -------------------------  ATT Specific Error ------------------------------------
    // ----------------------------------------------------------------------------------
    /// Handle is invalid
    ATT_ERR_INVALID_HANDLE = 0x01, //!< Value: 0x01
    /// Read permission disabled
    ATT_ERR_READ_NOT_PERMITTED = 0x02, //!< Value: 0x02
    /// Write permission disabled
    ATT_ERR_WRITE_NOT_PERMITTED = 0x03, //!< Value: 0x03
    /// Incorrect PDU
    ATT_ERR_INVALID_PDU = 0x04, //!< Value: 0x04
    /// Authentication privilege not enough
    ATT_ERR_INSUFF_AUTHEN = 0x05, //!< Value: 0x05
    /// Request not supported or not understood
    ATT_ERR_REQUEST_NOT_SUPPORTED = 0x06, //!< Value: 0x06
    /// Incorrect offset value
    ATT_ERR_INVALID_OFFSET = 0x07, //!< Value: 0x07
    /// Authorization privilege not enough
    ATT_ERR_INSUFF_AUTHOR = 0x08, //!< Value: 0x08
    /// Capacity queue for reliable write reached
    ATT_ERR_PREPARE_QUEUE_FULL = 0x09, //!< Value: 0x09
    /// Attribute requested not existing
    ATT_ERR_ATTRIBUTE_NOT_FOUND = 0x0A, //!< Value: 0x0A
    /// Attribute requested not long
    ATT_ERR_ATTRIBUTE_NOT_LONG = 0x0B, //!< Value: 0x0B
    /// Encryption size not sufficient
    ATT_ERR_INSUFF_ENC_KEY_SIZE = 0x0C, //!< Value: 0x0C
    /// Invalid length of the attribute value
    ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN = 0x0D, //!< Value: 0x0D
    /// Operation not fit to condition
    ATT_ERR_UNLIKELY_ERR = 0x0E, //!< Value: 0x0E
    /// Attribute requires encryption before operation
    ATT_ERR_INSUFF_ENC = 0x0F, //!< Value: 0x0F
    /// Attribute grouping not supported
    ATT_ERR_UNSUPP_GRP_TYPE = 0x10, //!< Value: 0x10
    /// Resources not sufficient to complete the request
    ATT_ERR_INSUFF_RESOURCE = 0x11, //!< Value: 0x11
    /// The server requests the client to rediscover the database.
    ATT_ERR_DB_OUT_OF_SYNC = 0x12, //!< Value: 0x12
    /// The attribute parameter value was not allowed.
    ATT_ERR_VALUE_NOT_ALLOWED = 0x13, //!< Value: 0x13
    /// Application error (also used in PRF Errors)
    ATT_ERR_APP_ERROR = 0x80, //!< Value: 0x80

    // ----------------------------------------------------------------------------------
    // ------------------------- L2CAP Specific Error -----------------------------------
    // ----------------------------------------------------------------------------------
    /// Message cannot be sent because connection lost. (disconnected)
    L2CAP_ERR_CONNECTION_LOST = 0x30, //!< Value: 0x30
    /// MTU size exceed or invalid MTU proposed
    L2CAP_ERR_INVALID_MTU = 0x31, //!< Value: 0x31
    /// MPS size exceed or invalid MPS proposed
    L2CAP_ERR_INVALID_MPS = 0x32, //!< Value: 0x32
    /// Invalid Channel ID
    L2CAP_ERR_INVALID_CID = 0x33, //!< Value: 0x33
    /// Invalid PDU
    L2CAP_ERR_INVALID_PDU = 0x34, //!< Value: 0x34
    /// Connection refused - unacceptable parameters
    L2CAP_ERR_UNACCEPTABLE_PARAM = 0x35, //!< Value: 0x35
    /// Connection refused - insufficient authentication
    L2CAP_ERR_INSUFF_AUTHEN = 0x36, //!< Value: 0x36
    /// Connection refused - insufficient authorization
    L2CAP_ERR_INSUFF_AUTHOR = 0x37, //!< Value: 0x37
    /// Connection refused - insufficient encryption key size
    L2CAP_ERR_INSUFF_ENC_KEY_SIZE = 0x38, //!< Value: 0x38
    /// Connection Refused - insufficient encryption
    L2CAP_ERR_INSUFF_ENC = 0x39, //!< Value: 0x39
    /// Connection refused - PSM / SPSM not supported
    L2CAP_ERR_PSM_SPSM_NOT_SUPP = 0x3A, //!< Value: 0x3A
    /// No more credit
    L2CAP_ERR_INSUFF_CREDIT = 0x3B, //!< Value: 0x3B
    /// Command not understood by peer device
    L2CAP_ERR_NOT_UNDERSTOOD = 0x3C, //!< Value: 0x3C
    /// Credit error, invalid number of credit received
    L2CAP_ERR_CREDIT_ERROR = 0x3D, //!< Value: 0x3D
    /// Channel identifier already allocated
    L2CAP_ERR_CID_ALREADY_ALLOC = 0x3E, //!< Value: 0x3E
    /// Unknown PDU
    L2CAP_ERR_UNKNOWN_PDU = 0x3F, //!< Value: 0x3F

    // ----------------------------------------------------------------------------------
    // -------------------------- GAP Specific Error ------------------------------------
    // ----------------------------------------------------------------------------------
    /// Invalid parameters set
    GAP_ERR_INVALID_PARAM = 0x40, //!< Value: 0x40
    /// Problem with protocol exchange, get unexpected response
    GAP_ERR_PROTOCOL_PROBLEM = 0x41, //!< Value: 0x41
    /// Request not supported by software configuration
    GAP_ERR_NOT_SUPPORTED = 0x42, //!< Value: 0x42
    /// Request not allowed in current state.
    GAP_ERR_COMMAND_DISALLOWED = 0x43, //!< Value: 0x43
    /// Requested operation canceled.
    GAP_ERR_CANCELED = 0x44, //!< Value: 0x44
    /// Requested operation timeout.
    GAP_ERR_TIMEOUT = 0x45, //!< Value: 0x45
    /// Link connection lost during operation.
    GAP_ERR_DISCONNECTED = 0x46, //!< Value: 0x46
    /// Search algorithm finished, but no result found
    GAP_ERR_NOT_FOUND = 0x47, //!< Value: 0x47
    /// Request rejected by peer device
    GAP_ERR_REJECTED = 0x48, //!< Value: 0x48
    /// Problem with privacy configuration
    GAP_ERR_PRIVACY_CFG_PB = 0x49, //!< Value: 0x49
    /// Duplicate or invalid advertising data
    GAP_ERR_ADV_DATA_INVALID = 0x4A, //!< Value: 0x4A
    /// Insufficient resources
    GAP_ERR_INSUFF_RESOURCES = 0x4B, //!< Value: 0x4B
    /// Unexpected Error
    GAP_ERR_UNEXPECTED = 0x4C, //!< Value: 0x4C
    // Value: 0x4D available
    /// Buffer cannot be used due to invalid header or tail length
    GAP_ERR_INVALID_BUFFER = 0x4E, //!< Value: 0x4E
    /// Request cannot be performed because an on-going procedure blocks it
    GAP_ERR_BUSY = 0x4F, //!< Value: 0x4F
    /// Resource is already registered - cannot be registered twice
    GAP_ERR_ALREADY_REGISTERED = 0x5A, //!< Value: 0x5A
    /// MIC failure detected
    GAP_ERR_MIC_FAILURE = 0x5B, //!< Value: 0x5B

    // ----------------------------------------------------------------------------------
    // ------------------------- GATT Specific Error ------------------------------------
    // ----------------------------------------------------------------------------------
    /// Problem with ATTC protocol response
    GATT_ERR_INVALID_ATT_LEN = 0x50, //!< Value: 0x50
    /// Error in service search
    GATT_ERR_INVALID_TYPE_IN_SVC_SEARCH = 0x51, //!< Value: 0x51
    /// Invalid write data
    GATT_ERR_WRITE = 0x52, //!< Value: 0x52
    /// Signed write error
    GATT_ERR_SIGNED_WRITE = 0x53, //!< Value: 0x53
    /// No attribute client defined
    GATT_ERR_ATTRIBUTE_CLIENT_MISSING = 0x54, //!< Value: 0x54
    /// No attribute server defined
    GATT_ERR_ATTRIBUTE_SERVER_MISSING = 0x55, //!< Value: 0x55
    /// Permission set in service/attribute are invalid
    GATT_ERR_INVALID_PERM = 0x56, //!< Value: 0x56
    /// The Attribute bearer is closed
    GATT_ERR_ATT_BEARER_CLOSE = 0x57, //!< Value: 0x57
    /// No more Attribute bearer available
    GATT_ERR_NO_MORE_BEARER = 0x58, //!< Value: 0x58
    // 0x5A to 0x5F mapped to GAP error

    // ----------------------------------------------------------------------------------
    // ------------------------- SMP Specific Error -------------------------------------
    // ----------------------------------------------------------------------------------
    // SMP Protocol Errors detected on local device
    /// The user input of pass key failed, for example, the user canceled the operation.
    SMP_ERR_LOC_PASSKEY_ENTRY_FAILED = 0x61, //!< Value: 0x61
    /// The OOB Data is not available.
    SMP_ERR_LOC_OOB_NOT_AVAILABLE = 0x62, //!< Value: 0x62
    /// The pairing procedure cannot be performed as authentication requirements cannot be met
    /// due to IO capabilities of one or both devices.
    SMP_ERR_LOC_AUTH_REQ = 0x63, //!< Value: 0x63
    /// The confirm value does not match the calculated confirm value.
    SMP_ERR_LOC_CONF_VAL_FAILED = 0x64, //!< Value: 0x64
    /// Pairing is not supported by the device.
    SMP_ERR_LOC_PAIRING_NOT_SUPP = 0x65, //!< Value: 0x65
    /// The resultant encryption key size is insufficient for the security requirements of
    /// this device.
    SMP_ERR_LOC_ENC_KEY_SIZE = 0x66, //!< Value: 0x66
    /// The SMP command received is not supported on this device.
    SMP_ERR_LOC_CMD_NOT_SUPPORTED = 0x67, //!< Value: 0x67
    /// Pairing failed due to an unspecified reason.
    SMP_ERR_LOC_UNSPECIFIED_REASON = 0x68, //!< Value: 0x68
    /// Pairing or Authentication procedure is disallowed because too little time has elapsed
    /// since last pairing request or security request.
    SMP_ERR_LOC_REPEATED_ATTEMPTS = 0x69, //!< Value: 0x69
    /// The command length is invalid or a parameter is outside of the specified range.
    SMP_ERR_LOC_INVALID_PARAM = 0x6A, //!< Value: 0x6A
    /// Indicates to the remote device that the DHKey Check value received doesn't
    /// match the one calculated by the local device.
    SMP_ERR_LOC_DHKEY_CHECK_FAILED = 0x6B, //!< Value: 0x6B
    /// Indicates that the confirm values in the numeric comparison protocol do not match.
    SMP_ERR_LOC_NUMERIC_COMPARISON_FAILED = 0x6C, //!< Value: 0x6C
    /// Indicates that the pairing over the LE transport failed due to a Pairing Request sent
    /// over the BR/EDR transport in process.
    SMP_ERR_LOC_BREDR_PAIRING_IN_PROGRESS = 0x6D, //!< Value: 0x6D
    /// Indicates that the BR/EDR Link Key generated on the BR/EDR transport cannot be
    /// used to derive and distribute keys for the LE transport.
    SMP_ERR_LOC_CROSS_TRANSPORT_KEY_GENERATION_NOT_ALLOWED = 0x6E, //!< Value: 0x6E
    // SMP Protocol Errors detected by remote device
    /// The user input of passkey failed, for example, the user canceled the operation.
    SMP_ERR_REM_PASSKEY_ENTRY_FAILED = 0x71, //!< Value: 0x71
    /// The OOB Data is not available.
    SMP_ERR_REM_OOB_NOT_AVAILABLE = 0x72, //!< Value: 0x72
    /// The pairing procedure cannot be performed as authentication requirements cannot be
    /// met due to IO capabilities of one or both devices.
    SMP_ERR_REM_AUTH_REQ = 0x73, //!< Value: 0x73
    /// The confirm value does not match the calculated confirm value.
    SMP_ERR_REM_CONF_VAL_FAILED = 0x74, //!< Value: 0x74
    /// Pairing is not supported by the device.
    SMP_ERR_REM_PAIRING_NOT_SUPP = 0x75, //!< Value: 0x75
    /// The resultant encryption key size is insufficient for the security requirements of
    /// this device.
    SMP_ERR_REM_ENC_KEY_SIZE = 0x76, //!< Value: 0x76
    /// The SMP command received is not supported on this device.
    SMP_ERR_REM_CMD_NOT_SUPPORTED = 0x77, //!< Value: 0x77
    /// Pairing failed due to an unspecified reason.
    SMP_ERR_REM_UNSPECIFIED_REASON = 0x78, //!< Value: 0x78
    /// Pairing or Authentication procedure is disallowed because too little time has elapsed
    /// since last pairing request or security request.
    SMP_ERR_REM_REPEATED_ATTEMPTS = 0x79, //!< Value: 0x79
    /// The command length is invalid or a parameter is outside of the specified range.
    SMP_ERR_REM_INVALID_PARAM = 0x7A, //!< Value: 0x7A
    /// Indicates to the remote device that the DHKey Check value received doesn't
    /// match the one calculated by the local device.
    SMP_ERR_REM_DHKEY_CHECK_FAILED = 0x7B, //!< Value: 0x7B
    /// Indicates that the confirm values in the numeric comparison protocol do not match.
    SMP_ERR_REM_NUMERIC_COMPARISON_FAILED = 0x7C, //!< Value: 0x7C
    /// Indicates that the pairing over the LE transport failed due to a Pairing Request sent
    /// over the BR/EDR transport in process.
    SMP_ERR_REM_BREDR_PAIRING_IN_PROGRESS = 0x7D, //!< Value: 0x7D
    /// Indicates that the BR/EDR Link Key generated on the BR/EDR transport cannot be
    /// used to derive and distribute keys for the LE transport.
    SMP_ERR_REM_CROSS_TRANSPORT_KEY_GENERATION_NOT_ALLOWED = 0x7E, //!< Value: 0x7E
    // SMP Errors triggered by local device
    /// The provided resolvable address has not been resolved.
    SMP_ERR_ADDR_RESOLV_FAIL = 0x20, //!< Value: 0x20
    /// The Signature Verification Failed
    SMP_ERR_SIGN_VERIF_FAIL = 0x21, //!< Value: 0x21
    /// The encryption procedure failed because the slave device didn't find the LTK
    /// needed to start an encryption session.
    SMP_ERR_ENC_KEY_MISSING = 0x22, //!< Value: 0x22
    /// The encryption procedure failed because the slave device doesn't support the
    /// encryption feature.
    SMP_ERR_ENC_NOT_SUPPORTED = 0x23, //!< Value: 0x23
    /// A timeout has occurred during the start encryption session.
    SMP_ERR_ENC_TIMEOUT = 0x24, //!< Value: 0x24
    /// Encryption cannot start because peer device not bonded
    SMP_ERR_NOT_BONDED = 0x25, //!< Value: 0x25

    // ----------------------------------------------------------------------------------
    //------------------------ Profiles specific error codes ----------------------------
    // ----------------------------------------------------------------------------------
    /// Application Error
    PRF_APP_ERROR = 0x80, //!< Value: 0x80
    /// Invalid parameter in request
    PRF_ERR_INVALID_PARAM = 0x81, //!< Value: 0x81
    /// Inexistent handle for sending a read/write characteristic request
    PRF_ERR_INEXISTENT_HDL = 0x82, //!< Value: 0x82
    /// Discovery stopped due to missing attribute according to specification
    PRF_ERR_STOP_DISC_CHAR_MISSING = 0x83, //!< Value: 0x83
    /// Too many SVC instances found -> protocol violation
    PRF_ERR_MULTIPLE_SVC = 0x84, //!< Value: 0x84
    /// Discovery stopped due to found attribute with incorrect properties
    PRF_ERR_STOP_DISC_WRONG_CHAR_PROP = 0x85, //!< Value: 0x85
    /// Too many Char. instances found-> protocol violation
    PRF_ERR_MULTIPLE_CHAR = 0x86, //!< Value: 0x86
    /// Feature mismatch
    PRF_ERR_MISMATCH = 0x87, //!< Value: 0x87
    /// Request not allowed
    PRF_ERR_REQ_DISALLOWED = 0x89, //!< Value: 0x89
    /// Notification Not Enabled
    PRF_ERR_NTF_DISABLED = 0x8A, //!< Value: 0x8A
    /// Indication Not Enabled
    PRF_ERR_IND_DISABLED = 0x8B, //!< Value: 0x8B
    /// Feature not supported by profile
    PRF_ERR_FEATURE_NOT_SUPPORTED = 0x8C, //!< Value: 0x8C
    /// Read value has an unexpected length
    PRF_ERR_UNEXPECTED_LEN = 0x8D, //!< Value: 0x8D
    /// Disconnection occurs
    PRF_ERR_DISCONNECTED = 0x8E, //!< Value: 0x8E
    /// Procedure Timeout
    PRF_ERR_PROC_TIMEOUT = 0x8F, //!< Value: 0x8F
    /// Requested write operation cannot be fulfilled for reasons other than permissions
    PRF_ERR_WRITE_REQ_REJECTED = 0xFC, //!< Value: 0xFC
    /// Client characteristic configuration improperly configured
    PRF_CCCD_IMPR_CONFIGURED = 0xFD, //!< Value: 0xFD
    /// Procedure already in progress
    PRF_PROC_IN_PROGRESS = 0xFE, //!< Value: 0xFE
    /// Out of Range
    PRF_OUT_OF_RANGE = 0xFF, //!< Value: 0xFF

    // ----------------------------------------------------------------------------------
    //-------------------- LL Error codes conveyed to upper layer -----------------------
    // ----------------------------------------------------------------------------------
    /// Unknown HCI Command
    LL_ERR_UNKNOWN_HCI_COMMAND = 0x91, //!< Value: 0x91
    /// Unknown Connection Identifier
    LL_ERR_UNKNOWN_CONNECTION_ID = 0x92, //!< Value: 0x92
    /// Hardware Failure
    LL_ERR_HARDWARE_FAILURE = 0x93, //!< Value: 0x93
    /// BT Page Timeout
    LL_ERR_PAGE_TIMEOUT = 0x94, //!< Value: 0x94
    /// Authentication failure
    LL_ERR_AUTH_FAILURE = 0x95, //!< Value: 0x95
    /// Pin code missing
    LL_ERR_PIN_MISSING = 0x96, //!< Value: 0x96
    /// Memory capacity exceed
    LL_ERR_MEMORY_CAPA_EXCEED = 0x97, //!< Value: 0x97
    /// Connection Timeout
    LL_ERR_CON_TIMEOUT = 0x98, //!< Value: 0x98
    /// Connection limit Exceed
    LL_ERR_CON_LIMIT_EXCEED = 0x99, //!< Value: 0x99
    /// Synchronous Connection limit exceed
    LL_ERR_SYNC_CON_LIMIT_DEV_EXCEED = 0x9A, //!< Value: 0x9A
    /// ACL Connection exits
    LL_ERR_ACL_CON_EXISTS = 0x9B, //!< Value: 0x9B
    /// Command Disallowed
    LL_ERR_COMMAND_DISALLOWED = 0x9C, //!< Value: 0x9C
    /// Connection rejected due to limited resources
    LL_ERR_CONN_REJ_LIMITED_RESOURCES = 0x9D, //!< Value: 0x9D
    /// Connection rejected due to security reason
    LL_ERR_CONN_REJ_SECURITY_REASONS = 0x9E, //!< Value: 0x9E
    /// Connection rejected due to unacceptable BD Addr
    LL_ERR_CONN_REJ_UNACCEPTABLE_BDADDR = 0x9F, //!< Value: 0x9F
    /// Connection rejected due to Accept connection timeout
    LL_ERR_CONN_ACCEPT_TIMEOUT_EXCEED = 0xA0, //!< Value: 0xA0
    /// Not Supported
    LL_ERR_UNSUPPORTED = 0xA1, //!< Value: 0xA1
    /// invalid parameters
    LL_ERR_INVALID_HCI_PARAM = 0xA2, //!< Value: 0xA2
    /// Remote user terminate connection
    LL_ERR_REMOTE_USER_TERM_CON = 0xA3, //!< Value: 0xA3
    /// Remote device terminate connection due to low resources
    LL_ERR_REMOTE_DEV_TERM_LOW_RESOURCES = 0xA4, //!< Value: 0xA4
    /// Remote device terminate connection due to power off
    LL_ERR_REMOTE_DEV_POWER_OFF = 0xA5, //!< Value: 0xA5
    /// Connection terminated by local host
    LL_ERR_CON_TERM_BY_LOCAL_HOST = 0xA6, //!< Value: 0xA6
    /// Repeated attempts
    LL_ERR_REPEATED_ATTEMPTS = 0xA7, //!< Value: 0xA7
    /// Pairing not Allowed
    LL_ERR_PAIRING_NOT_ALLOWED = 0xA8, //!< Value: 0xA8
    /// Unknown PDU Error
    LL_ERR_UNKNOWN_LMP_PDU = 0xA9, //!< Value: 0xA9
    /// Unsupported remote feature
    LL_ERR_UNSUPPORTED_REMOTE_FEATURE = 0xAA, //!< Value: 0xAA
    /// Sco Offset rejected
    LL_ERR_SCO_OFFSET_REJECTED = 0xAB, //!< Value: 0xAB
    /// SCO Interval Rejected
    LL_ERR_SCO_INTERVAL_REJECTED = 0xAC, //!< Value: 0xAC
    /// SCO air mode Rejected
    LL_ERR_SCO_AIR_MODE_REJECTED = 0xAD, //!< Value: 0xAD
    /// Invalid LMP parameters
    LL_ERR_INVALID_LMP_PARAM = 0xAE, //!< Value: 0xAE
    /// Unspecified error
    LL_ERR_UNSPECIFIED_ERROR = 0xAF, //!< Value: 0xAF
    /// Unsupported LMP Parameter value
    LL_ERR_UNSUPPORTED_LMP_PARAM_VALUE = 0xB0, //!< Value: 0xB0
    /// Role Change Not allowed
    LL_ERR_ROLE_CHANGE_NOT_ALLOWED = 0xB1, //!< Value: 0xB1
    /// LMP Response timeout
    LL_ERR_LMP_RSP_TIMEOUT = 0xB2, //!< Value: 0xB2
    /// LMP Collision
    LL_ERR_LMP_COLLISION = 0xB3, //!< Value: 0xB3
    /// LMP Pdu not allowed
    LL_ERR_LMP_PDU_NOT_ALLOWED = 0xB4, //!< Value: 0xB4
    /// Encryption mode not accepted
    LL_ERR_ENC_MODE_NOT_ACCEPT = 0xB5, //!< Value: 0xB5
    /// Link Key Cannot be changed
    LL_ERR_LINK_KEY_CANT_CHANGE = 0xB6, //!< Value: 0xB6
    /// Quality of Service not supported
    LL_ERR_QOS_NOT_SUPPORTED = 0xB7, //!< Value: 0xB7
    /// Error, instant passed
    LL_ERR_INSTANT_PASSED = 0xB8, //!< Value: 0xB8
    /// Pairing with unit key not supported
    LL_ERR_PAIRING_WITH_UNIT_KEY_NOT_SUP = 0xB9, //!< Value: 0xB9
    /// Transaction collision
    LL_ERR_DIFF_TRANSACTION_COLLISION = 0xBA, //!< Value: 0xBA
    /// Unacceptable parameters
    LL_ERR_QOS_UNACCEPTABLE_PARAM = 0xBC, //!< Value: 0xBC
    /// Quality of Service rejected
    LL_ERR_QOS_REJECTED = 0xBD, //!< Value: 0xBD
    /// Channel class not supported
    LL_ERR_CHANNEL_CLASS_NOT_SUP = 0xBE, //!< Value: 0xBE
    /// Insufficient security
    LL_ERR_INSUFFICIENT_SECURITY = 0xBF, //!< Value: 0xBF
    /// Parameters out of mandatory range
    LL_ERR_PARAM_OUT_OF_MAND_RANGE = 0xC0, //!< Value: 0xC0
    /// Role switch pending
    LL_ERR_ROLE_SWITCH_PEND = 0xC2, //!< Value: 0xC2
    /// Reserved slot violation
    LL_ERR_RESERVED_SLOT_VIOLATION = 0xC4, //!< Value: 0xC4
    /// Role Switch fail
    LL_ERR_ROLE_SWITCH_FAIL = 0xC5, //!< Value: 0xC5
    /// Error, EIR too large
    LL_ERR_EIR_TOO_LARGE = 0xC6, //!< Value: 0xC6
    /// Simple pairing not supported by host
    LL_ERR_SP_NOT_SUPPORTED_HOST = 0xC7, //!< Value: 0xC7
    /// Host pairing is busy
    LL_ERR_HOST_BUSY_PAIRING = 0xC8, //!< Value: 0xC8
    /// Controller is busy
    LL_ERR_CONTROLLER_BUSY = 0xCA, //!< Value: 0xCA
    /// Unacceptable connection parameters
    LL_ERR_UNACCEPTABLE_CONN_PARAM = 0xCB, //!< Value: 0xCB
    /// Direct Advertising Timeout
    LL_ERR_DIRECT_ADV_TO = 0xCC, //!< Value: 0xCC
    /// Connection Terminated due to a MIC failure
    LL_ERR_TERMINATED_MIC_FAILURE = 0xCD, //!< Value: 0xCD
    /// Connection failed to be established
    LL_ERR_CONN_FAILED_TO_BE_EST = 0xCE, //!< Value: 0xCE
    /// MAC Connection Failed
    LL_ERR_MAC_CONN_FAILED = 0xCF, //!< Value: 0xCF
    /// Coarse Clock Adjustment Rejected but Will Try to Adjust Using Clock Dragging
    LL_ERR_CCA_REJ_USE_CLOCK_DRAG = 0xD0, //!< Value: 0xD0
    /// Type0 Submap Not Defined
    LL_ERR_TYPE0_SUBMAP_NOT_DEFINED = 0xD1, //!< Value: 0xD1
    /// Unknown Advertising Identifier
    LL_ERR_UNKNOWN_ADVERTISING_ID = 0xD2, //!< Value: 0xD2
    /// Limit Reached
    LL_ERR_LIMIT_REACHED = 0xD3, //!< Value: 0xD3
    /// Operation Cancelled by Host
    LL_ERR_OPERATION_CANCELED_BY_HOST = 0xD4, //!< Value: 0xD4
    /// Packet Too Long
    LL_ERR_PKT_TOO_LONG = 0xD5, //!< Value: 0xD5
    /// Too late
    LL_ERR_TOO_LATE = 0xD6, //!< Value: 0xD6
    /// Too early
    LL_ERR_TOO_EARLY = 0xD7, //!< Value: 0xD7

    /// Group already exists and cannot be added
    GAPI_ERR_GROUP_ALREADY_EXISTS = HL_ERR_CODE_GAPI(0x00), //!< Value: 0x0100
    /// Synchronization lost
    GAPI_ERR_SYNC_LOST = HL_ERR_CODE_GAPI(0x01), //!< Value: 0x0101
    /// Interface already exists
    GAPI_ERR_INTF_EXISTS = HL_ERR_CODE_GAPI(0x02), //!< Value: 0x0102
    /// Unknown interface,
    GAPI_ERR_UNKNOWN_INTERFACE = HL_ERR_CODE_GAPI(0x03), //!< Value: 0x0103
    /// Data path can be setup because stream is not enabled
    GAPI_ERR_STREAM_NOT_ENABLED = HL_ERR_CODE_GAPI(0x04), //!< Value: 0x0104
    /// Data path is setup and cannot be reconfigured
    GAPI_ERR_DP_SETUP = HL_ERR_CODE_GAPI(0x05), //!< Value: 0x0105
    /// Data path is not setup and cannot be removed
    GAPI_ERR_DP_NOT_SETUP = HL_ERR_CODE_GAPI(0x06), //!< Value: 0x0106
    /// Data path is not configured and cannot be setup
    GAPI_ERR_DP_NOT_CONFIGURED = HL_ERR_CODE_GAPI(0x07), //!< Value: 0x0107
};

/// @} HL_ERROR_API

#endif // HL_ERROR_H_
