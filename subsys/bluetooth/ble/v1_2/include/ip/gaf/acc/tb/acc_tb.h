/**
 ****************************************************************************************
 *
 * @file acc_tb.h
 *
 * @brief Audio Content Control - Bearer Control - Header file
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef ACC_TB_H_
#define ACC_TB_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup ACC_TB Telephone Bearer Service
 * @ingroup ACC
 * @brief Description of Telephone Bearer Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_TB_DEF Definitions
 * @ingroup ACC_TB
 * @brief Definitions for Telephone Bearer Service module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup ACC_TB_ENUM Enumerations
 * @ingroup ACC_TB
 * @brief Enumerations for Telephone Bearer Service module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "acc.h"            // Audio Content Control Defintiions

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup ACC_TB_DEF
/// @{

/// Length of Bearer Technology characteristic value
#define ACC_TB_TECHNO_LEN               (1)
/// Length of Bearer Signal Strength characteristic value
#define ACC_TB_SIGN_STRENGTH_LEN        (1)
/// Length of Bearer Signal Strength Reporting Interval characteristic value
#define ACC_TB_SIGN_STRENGTH_INTV_LEN   (1)
/// Length of Call Control Point Optional Opcodes characteristic value
#define ACC_TB_OPT_OPCODES_LEN          (2)
/// Length of Status Flags characteristic value
#define ACC_TB_STATUS_FLAGS_LEN         (1)
/// Minimum number of Calls for Join procedure
#define ACC_TB_JOIN_NB_CALL_MIN         (2)
/// Maximum Incoming or Outgoing URI length
#define ACC_TB_URI_MAX_LEN              (252)
/// Maximal Signal Strength value
#define ACC_TB_SIGN_STRENGTH_MAX        (100)
/// Signal Strength unavailable or meaningless
#define ACC_TB_SIGN_STRENGTH_UNAVA      (255)

/// @} ACC_TB_DEF

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup ACC_TB_ENUM
/// @{

/// Characteristic type values
enum acc_tb_char_type
{
    /// Bearer Provider Name characteristic
    ACC_TB_CHAR_TYPE_PROV_NAME = 0,
    /// Bearer Technology characteristic
    ACC_TB_CHAR_TYPE_TECHNO,
    /// Bearer Signal Strength characteristic
    ACC_TB_CHAR_TYPE_SIGN_STRENGTH,
    /// Call State characteristic
    ACC_TB_CHAR_TYPE_CALL_STATE,
    /// Bearer List Current Calls characteristic
    ACC_TB_CHAR_TYPE_CURR_CALLS_LIST,
    /// Status Flags characteristic
    ACC_TB_CHAR_TYPE_STATUS_FLAGS,
    /// Incoming Call Target Bearer URI characteristic
    ACC_TB_CHAR_TYPE_IN_TGT_CALLER_ID,
    /// Call Control Point characteristic
    ACC_TB_CHAR_TYPE_CALL_CTL_PT,
    /// Termination Reason characteristic
    ACC_TB_CHAR_TYPE_TERM_REASON,
    /// Incoming Call characteristic
    ACC_TB_CHAR_TYPE_INCOMING_CALL,
    /// Call Friendly Name characteristic
    ACC_TB_CHAR_TYPE_CALL_FRIENDLY_NAME,
    /// Bearer URI Schemes Supported List characteristic
    ACC_TB_CHAR_TYPE_URI_SCHEMES_LIST,

    /// All characteristics above are notification-capable
    ACC_TB_NTF_CHAR_TYPE_MAX,

    /// Bearer UCI characteristic
    ACC_TB_CHAR_TYPE_UCI = ACC_TB_NTF_CHAR_TYPE_MAX,
    /// Bearer Signal Strength Reporting Interval characteristic
    ACC_TB_CHAR_TYPE_SIGN_STRENGTH_INTV,
    /// Content Control ID characteristic
    ACC_TB_CHAR_TYPE_CCID,
    /// Call Control Point Optional Opcodes characteristic
    ACC_TB_CHAR_TYPE_CALL_CTL_PT_OPT_OPCODES,

    ACC_TB_CHAR_TYPE_MAX,
};

/// Descriptor type values
enum acc_tb_desc_type
{
    /// Client Characteristic Configuration descriptor for Bearer Provider Name characteristic
    ACC_TB_DESC_TYPE_CCC_PROV_NAME = 0,
    /// Client Characteristic Configuration descriptor for Bearer Technology characteristic
    ACC_TB_DESC_TYPE_CCC_TECHNO,
    /// Client Characteristic Configuration descriptor for Bearer Signal Strength characteristic
    ACC_TB_DESC_TYPE_CCC_SIGN_STRENGTH,
    /// Client Characteristic Configuration descriptor for Call State characteristic
    ACC_TB_DESC_TYPE_CCC_CALL_STATE,
    /// Client Characteristic Configuration descriptor for Bearer List Current Calls characteristic
    ACC_TB_DESC_TYPE_CCC_CURR_CALLS_LIST,
    /// Client Characteristic Configuration descriptor for Status Flags characteristic
    ACC_TB_DESC_TYPE_CCC_STATUS_FLAGS,
    /// Client Characteristic Configuration descriptor for Incoming Call Target Bearer
    /// URI characteristic
    ACC_TB_DESC_TYPE_CCC_IN_TGT_CALLER_ID,
    /// Client Characteristic Configuration descriptor for Call Control Point characteristic
    ACC_TB_DESC_TYPE_CCC_CALL_CTL_PT,
    /// Client Characteristic Configuration descriptor for Termination Reason characteristic
    ACC_TB_DESC_TYPE_CCC_TERM_REASON,
    /// Client Characteristic Configuration descriptor for Incoming Call characteristic
    ACC_TB_DESC_TYPE_CCC_INCOMING_CALL,
    /// Client Characteristic Configuration descriptor for Call Friendly Name characteristic
    ACC_TB_DESC_TYPE_CCC_CALL_FRIENDLY_NAME,
    /// Client Characteristic Configuration descriptor for Bearer URI Schemes Supported
    /// List characteristic
    ACC_TB_DESC_TYPE_CCC_URI_SCHEMES_LIST,

    ACC_TB_DESC_TYPE_MAX,
};

/// Bearer technology values
enum acc_tb_techno
{
    /// Reserved (No bearer | No SIM card)
    ACC_TB_TECHNO_RESERVED = 0,
    /// 3G technology
    ACC_TB_TECHNO_3G,
    /// 4G technology
    ACC_TB_TECHNO_4G,
    /// LTE technology
    ACC_TB_TECHNO_LTE,
    /// Wi-Fi technology
    ACC_TB_TECHNO_WIFI,
    /// 5G technology
    ACC_TB_TECHNO_5G,
    /// GSM technology
    ACC_TB_TECHNO_GSM,
    /// CDMA technology
    ACC_TB_TECHNO_CDMA,
    /// 2G technology
    ACC_TB_TECHNO_2G,
    /// WCDMA technology
    ACC_TB_TECHNO_WCDMA,

    ACC_TB_TECHNO_MAX,
};

/// Call state values
enum acc_tb_call_state
{
    /// Incoming
    ACC_TB_CALL_STATE_INCOMING = 0,
    /// Dialing
    ACC_TB_CALL_STATE_DIALING,
    /// Alerting
    ACC_TB_CALL_STATE_ALERTING,
    /// Active
    ACC_TB_CALL_STATE_ACTIVE,
    /// Locally Held
    ACC_TB_CALL_STATE_LOC_HELD,
    /// Remotely Held
    ACC_TB_CALL_STATE_REMOTE_HELD,
    /// Locally and Remotely Held
    ACC_TB_CALL_STATE_LOC_REMOTE_HELD,

    ACC_TB_CALL_STATE_MAX,
};

/// Termination reason values
enum acc_tb_term_reason
{
    /// URI value improperly formed
    ACC_TB_TERM_REASON_URI = 0,
    /// Call fail
    ACC_TB_TERM_REASON_CALL_FAIL,
    /// Remote party ended Call
    ACC_TB_TERM_REASON_REMOTE_END,
    /// Call ended from the Server
    ACC_TB_TERM_REASON_SRV_END,
    /// Line Busy
    ACC_TB_TERM_REASON_BUSY,
    /// Network Congestion
    ACC_TB_TERM_REASON_CONGESTION,
    /// Call ended from the Client
    ACC_TB_TERM_REASON_CLI_END,
    /// No service
    ACC_TB_TERM_REASON_NO_SVC,
    /// No answer
    ACC_TB_TERM_REASON_NO_ANSWER,
    /// Unspecified
    ACC_TB_TERM_REASON_UNSPECIFIED,

    ACC_TB_TERM_REASON_MAX,
};

/// Operation code values
enum acc_tb_opcode
{
    /// Accept
    ACC_TB_OPCODE_ACCEPT = 0,
    /// Terminate
    ACC_TB_OPCODE_TERMINATE,
    /// Local Hold
    ACC_TB_OPCODE_HOLD,
    /// Local Retrieve
    ACC_TB_OPCODE_RETRIEVE,
    /// Originate
    ACC_TB_OPCODE_ORIGINATE,
    /// Join
    ACC_TB_OPCODE_JOIN,

    ACC_TB_OPCODE_MAX
};

/// Direction values
enum acc_tb_direction
{
    /// Incoming
    ACC_TB_DIRECTION_INCOMING = 0,
    /// Outgoing
    ACC_TB_DIRECTION_OUTGOING,
};

/// Call control point notification result code values
enum acc_tb_cp_ntf_result
{
    /// Opcode write was successful
    ACC_TB_CP_NTF_RESULT_SUCCESS = 0,
    /// An invalid opcode was used for the Call Control Point write
    ACC_TB_CP_NTF_RESULT_OPCODE_NOT_SUPPORTED,
    /// Requested operation cannot be completed
    ACC_TB_CP_NTF_RESULT_OP_NOT_POSSIBLE,
    /// The Call Index used for the Call Control Point write is invalid
    ACC_TB_CP_NTF_RESULT_INVALID_CALL_INDEX,
    /// The opcode written to the Call Control Point was received when the current Call State
    /// for the Call Index is not in the expected state
    ACC_TB_CP_NTF_RESULT_STATE_MISMATCH,
    /// Lack of internal resources to complete the requested action
    ACC_TB_CP_NTF_RESULT_LACK_OF_RESSOURCES,
    /// The Outgoing URI is incorrect or invalid when an Originate opcode is sent
    ACC_TB_CP_NTF_RESULT_INVALID_URI,

    ACC_TB_CP_NTF_RESULT_MAX,
};

/// Call Control Point Optional Opcodes bit field meaning
enum acc_tb_opt_opcodes_bf
{
    /// Indicate if Local Hold and Local Retrieve operation codes are supported (= 1) or not
    ACC_TB_OPT_OPCODES_LOCAL_HOLD_POS = 0,
    ACC_TB_OPT_OPCODES_LOCAL_HOLD_BIT = CO_BIT(ACC_TB_OPT_OPCODES_LOCAL_HOLD_POS),

    /// Indicate if Join operation code is supported (= 1) or not
    ACC_TB_OPT_OPCODES_JOIN_POS = 1,
    ACC_TB_OPT_OPCODES_JOIN_BIT = CO_BIT(ACC_TB_OPT_OPCODES_JOIN_POS),

    ACC_TB_OPT_OPCODES_MASK = (ACC_TB_OPT_OPCODES_LOCAL_HOLD_BIT | ACC_TB_OPT_OPCODES_JOIN_BIT),
};

/// Status Flags bit field meaning
enum acc_tb_status_flags_bf
{
    /// Inband Ringtone
    ACC_TB_STATUS_FLAGS_INBAND_RINGTONE_POS = 0,
    ACC_TB_STATUS_FLAGS_INBAND_RINGTONE_BIT = CO_BIT(ACC_TB_STATUS_FLAGS_INBAND_RINGTONE_POS),

    /// Silent Mode
    ACC_TB_STATUS_FLAGS_SILENT_POS = 1,
    ACC_TB_STATUS_FLAGS_SILENT_BIT = CO_BIT(ACC_TB_STATUS_FLAGS_SILENT_POS),

    ACC_TB_STATUS_FLAGS_MASK = (ACC_TB_STATUS_FLAGS_INBAND_RINGTONE_BIT | ACC_TB_STATUS_FLAGS_SILENT_BIT),
};

/// Bearer List Current Calls characteristic value description
enum acc_tb_curr_call_pos
{
    /// Position of List Item length field
    ACC_TB_CURR_CALL_LIST_ITEM_LEN_POS = 0,
    /// Position of Call Index field
    ACC_TB_CURR_CALL_INDEX_POS,
    /// Position of Call State field
    ACC_TB_CURR_CALL_STATE_POS,
    /// Position of Call Flags field
    ACC_TB_CURR_CALL_FLAGS_POS,

    /// Minimum length
    ACC_TB_CURR_CALL_MIN_LEN,
    /// Minimum List Item length value
    ACC_TB_CURR_CALL_MIN_LIST_ITEM_LEN = ACC_TB_CURR_CALL_MIN_LEN - 1,

    /// Position of URI field
    ACC_TB_CURR_CALL_URI_POS = ACC_TB_CURR_CALL_MIN_LEN
};

/// Call State characteristic value description
enum acc_tb_call_state_pos
{
    /// Position of Call Index field
    ACC_TB_CALL_STATE_INDEX_POS = 0,
    /// Position of Call State field
    ACC_TB_CALL_STATE_STATE_POS,
    /// Position of Call Flags field
    ACC_TB_CALL_STATE_FLAGS_POS,

    /// Minimum length
    ACC_TB_CALL_STATE_MIN_LEN,
};

/// Incoming Call characteristic value description
enum acc_tb_incoming_call_pos
{
    /// Position of Call Index field
    ACC_TB_INCOMING_CALL_INDEX_POS = 0,

    /// Minimum length
    ACC_TB_INCOMING_CALL_MIN_LEN,

    /// Position of URI field
    ACC_TB_INCOMING_CALL_URI_POS = ACC_TB_INCOMING_CALL_MIN_LEN,
};

/// Call Control Point characteristic value description
enum acc_tb_cp_pos
{
    /// Position of Opcode field
    ACC_TB_CP_OPCODE_POS = 0,

    /// Minimum length
    ACC_TB_CP_MIN_LEN,

    /// Position of Parameter field
    ACC_TB_CP_PARAM_POS = ACC_TB_CP_MIN_LEN,
    /// Position of Call Index field for Accept, Terminate, Local Hold, Local Retrieve
    /// and Join operations
    ACC_TB_CP_CALL_ID_POS = ACC_TB_CP_PARAM_POS,
    /// Position of URI field for Originate field
    ACC_TB_CP_URI_POS = ACC_TB_CP_PARAM_POS,

    /// Length when only Call Index is provided
    ACC_TB_CP_CALL_ID_LEN = ACC_TB_CP_CALL_ID_POS + 1,

    /// Maximum length
    ACC_TB_CP_LEN_MAX = 64,
};

/// Termination Reason characteristic value description
enum acc_tb_term_reason_pos
{
    /// Position of Call Index field
    ACC_TB_TERM_REASON_INDEX_POS = 0,
    /// Position of Reason Code field
    ACC_TB_TERM_REASON_CODE_POS,

    /// Length
    ACC_TB_TERM_REASON_LEN,
};

/// Call Control Point characteristic notification value description
enum acc_tb_cp_ntf_pos
{
    /// Requested opcode
    ACC_TB_CP_NTF_OPCODE_POS = 0,
    /// Call index
    ACC_TB_CP_NTF_CALL_INDEX_POS,
    /// Result
    ACC_TB_CP_NTF_RESULT_POS,

    /// Length
    ACC_TB_CP_NTF_LEN,
};

/// Call Flags bit field meaning
enum acc_tb_call_flags_bf
{
    /// Indicate if call is an outgoing call - Position
    ACC_TB_CALL_FLAGS_OUTGOING_POS = 0,
    /// Indicate if call is an outgoing call - Bit
    ACC_TB_CALL_FLAGS_OUTGOING_BIT = CO_BIT(ACC_TB_CALL_FLAGS_OUTGOING_POS),

    /// Information withheld by server - Position
    ACC_TB_CALL_FLAGS_INFO_SERVER_POS = 1,
    /// Information withheld by server - Bit
    ACC_TB_CALL_FLAGS_INFO_SERVER_BIT = CO_BIT(ACC_TB_CALL_FLAGS_INFO_SERVER_POS),

    /// Information withheld by network - Position
    ACC_TB_CALL_FLAGS_INFO_NETWORK_POS = 2,
    /// Information withheld by server - Bit
    ACC_TB_CALL_FLAGS_INFO_NETWORK_BIT = CO_BIT(ACC_TB_CALL_FLAGS_INFO_NETWORK_POS),

    /// Call Flags value mask
    ACC_TB_CALL_FLAGS_MASK = ACC_TB_CALL_FLAGS_OUTGOING_BIT | ACC_TB_CALL_FLAGS_INFO_SERVER_BIT
                             | ACC_TB_CALL_FLAGS_INFO_SERVER_BIT,
};

/// Application error codes for Telephone Bearer Service
enum acc_tb_err
{
    /// Value changed during Read Long
    ACC_TB_ERR_VALUE_CHANGED = 0x80,
    /// Unexpected error
    ACC_TB_ERR_UNEXPECTED,
};

/// @} ACC_TB_ENUM

#endif // ACC_TB_H_
