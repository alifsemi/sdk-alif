/**
 ****************************************************************************************
 *
 * @file tip_common.h
 *
 * @brief Header File - Time Profile common types.
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _TIP_COMMON_H_
#define _TIP_COMMON_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup TIP_API Time Profile (TIP)
 * @ingroup PROFILE_API
 * @brief Description of API for Time Profile.\n
 * It includes support for:
 *     - Current Time Service (CTS) - Mandatory
 *     - Next DST Change Service (NDCS) - Optional
 *     - Reference Time Update Service (RTUS) - Optional
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup TIP_API_COMMON Common
 * @ingroup TIP_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "prf_types.h"
#include "co_math.h"

/// @addtogroup TIP_API_COMMON
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Adjust Reason flags bit field meaning
enum tips_adj_reason_flags_bf
{
    /// Time update - Position
    TIPS_FLAG_MAN_TIME_UPDATE_POS = 0,
    /// Time update - Bit
    TIPS_FLAG_MAN_TIME_UPDATE_BIT = CO_BIT(TIPS_FLAG_MAN_TIME_UPDATE_POS),

    /// External time update - Position
    TIPS_FLAG_EXT_TIME_UPDATE_POS = 1,
    /// External time update - Bit
    TIPS_FLAG_EXT_TIME_UPDATE_BIT = CO_BIT(TIPS_FLAG_EXT_TIME_UPDATE_POS),

    /// Change of time zone - Position
    TIPS_FLAG_CHG_TIME_ZONE_POS = 2,
    /// Change of time zone - Bit
    TIPS_FLAG_CHG_TIME_ZONE_BIT = CO_BIT(TIPS_FLAG_CHG_TIME_ZONE_POS),

    /// DST change - Position
    TIPS_FLAG_DST_CHANGE_POS = 3,
    /// DST change - Bit
    TIPS_FLAG_DST_CHANGE_BIT = CO_BIT(TIPS_FLAG_DST_CHANGE_POS),
};

/// Time Update Control Point keys
enum tips_time_upd_cntl_pt
{
    /// Get
    TIPS_TIME_UPD_CTNL_PT_GET = 0x01,
    /// Cancel
    TIPS_TIME_UPD_CTNL_PT_CANCEL,
};

/// Time Update State Current State Key Values
enum tips_time_update_state
{
    /// Idle
    TIPS_TIME_UPD_STATE_IDLE = 0x00,
    /// Time update pending
    TIPS_TIME_UPD_STATE_PENDING,
};

/// Time Update State Result Key Values
enum tips_time_upd_result
{
    /// Success
    TIPS_TIME_UPD_RESULT_SUCCESS = 0x00,
    /// Canceled
    TIPS_TIME_UPD_RESULT_CANCELED,
    /// No connection
    TIPS_TIME_UPD_RESULT_NO_CONN,
    /// Error
    TIPS_TIME_UPD_RESULT_ERROR_RSP,
    /// Timeout
    TIPS_TIME_UPD_RESULT_TIMEOUT,
    /// Not attempted
    TIPS_TIME_UPD_NOT_ATTEMPTED,
};

/// Time Profile Supported Features bit flags
enum tips_features_bf
{
    /// NDCS supported
    TIPS_NDCS_SUPPORTED = 0x01,
    /// RTUS supported
    TIPS_RTUS_SUPPORTED = 0x02,
};

/// Value type
enum tipc_val_type
{
    /// Read response
    TIP_RD_RSP = 0x00,
    /// Notification
    TIP_NTF = 0x01,
};


/// Value identifier for read operations
enum tip_value_id
{
    /// Current Time
    TIP_VAL_CTS_CURRENT_TIME = 0x00,
    /// Local Time Info
    TIP_VAL_CTS_LOCAL_TIME_INFO = 0x01,
    /// Reference Time Info
    TIP_VAL_CTS_REF_TIME_INFO = 0x02,
    /// Next DST Change Time
    TIP_VAL_NDCS_TIME_DST = 0x03,
    /// Time Update State
    TIP_VAL_RTUS_TIME_UPD_STATE_VAL = 0x04,
    /// Client Characteristic Configuration for Current Time characteristic
    TIP_VAL_CTS_NTF_CCC_CFG  = 0x05,
    /// Invalid
    TIP_VAL_INVALID  = 0xFF,
};

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/*
 * Current Time Service
 */

/// Current Time Characteristic Structure
typedef struct tip_curr_time
{
    /// Date time
    prf_date_time_t date_time;
    /// Day of the week
    uint8_t day_of_week;
    /// 1/256th of a second
    uint8_t fraction_256;
    /// Adjust reason
    uint8_t adjust_reason;
} tip_curr_time_t;

/**
 * Time Zone Characteristic - UUID:0x2A0E
 * Min value : -48 (UTC-12:00), Max value : 56 (UTC+14:00)
 * -128 : Time zone offset is not known
 */
typedef int8_t tip_time_zone_t;

/**
 * DST Offset Characteristic - UUID:0x2A2D
 * Min value : 0, Max value : 8
 * 255 = DST is not known
 */
typedef uint8_t tip_dst_offset_t;

/// Local Time Info Characteristic Structure - UUID:0x2A0F
typedef struct tip_loc_time_info
{
    /// Time Zone
    tip_time_zone_t  time_zone;
    /// DST offset
    tip_dst_offset_t dst_offset;
} tip_loc_time_info_t;

/**
 * Time Source Characteristic - UUID:0x2A13
 * Min value : 0, Max value : 6
 * 0 = Unknown
 * 1 = Network Time Protocol
 * 2 = GPS
 * 3 = Radio Time Signal
 * 4 = Manual
 * 5 = Atomic Clock
 * 6 = Cellular Network
 */
typedef uint8_t tip_time_source_t;

/**
 * Time Accuracy Characteristic - UUID:0x2A12
 * Accuracy (drift) of time information in steps of 1/8 of a second (125ms) compared
 * to a reference time source. Valid range from 0 to 253 (0s to 31.5s). A value of
 * 254 means Accuracy is out of range (> 31.5s). A value of 255 means Accuracy is
 * unknown.
 */
typedef uint8_t tip_time_accuracy_t;

///Reference Time Info Characteristic Structure - UUID:0x2A14
typedef struct tip_ref_time_info
{
    /// Time source
    tip_time_source_t   time_source;
    /// Time Accuracy
    tip_time_accuracy_t time_accuracy;
    /**
     * Days since last update about Reference Source
     * Min value : 0, Max value : 254
     * 255 = 255 or more days
     */
    uint8_t             days_update;
    /**
     * Hours since update about Reference Source
     * Min value : 0, Mac value : 23
     * 255 = 255 or more days (If Days Since Update = 255, then Hours Since Update shall
     * also be set to 255)
     */
    uint8_t             hours_update;
} tip_ref_time_info_t;

/*
 * Next DST Change Service
 */

/// Time With DST Characteristic Structure - UUID:0x2A11
typedef struct tip_time_with_dst
{
    /// Date and Time of the Next DST Change
    prf_date_time_t  date_time;
    /// DST Offset that will be in effect after this change
    tip_dst_offset_t dst_offset;
} tip_time_with_dst_t;

/*
 * Reference Time Update Service
 */

/**
 * Time Update Control Point Characteristic - UUID:0x2A16
 * The Time Update Control Point Characteristic enables a client to issue a command
 * request to update the time in the time server.
 * 0x01 = Get Reference Update = Forces the state machine to Update Pending and
 * starts the time update procedure
 * 0x02 = Cancel Reference Update = Forces the state machine to idle and stops the
 * attempt to receive a time update.
 */
typedef uint8_t tip_time_upd_contr_pt_t;

/// Time Update State Characteristic Structure - UUID:0x2A17
typedef struct tip_time_upd_state
{
    /**
     * The Time Update Status Characteristic exposes the status of the time update
     * process and the result of the last update in the server.
     */

    /**
     * Current State
     * Min value : 0, Max value = 1
     * 0 = Idle
     * 1 = Update Pending
     */
    uint8_t current_state;
    /**
     * Result
     * Min value : 0, Max Value : 5
     * 0 = Successful
     * 1 = Canceled
     * 2 = No Connection To Reference
     * 3 = Reference responded with an error
     * 4 = Timeout
     * 5 = Update not attempted after reset
     */
    uint8_t result;
} tip_time_upd_state_t;


/// Union of value that can be read
union tip_value
{
    /// Current Time (#TIP_VAL_CTS_CURRENT_TIME)
    tip_curr_time_t curr_time;
    /// Local Time Information (#TIP_VAL_CTS_LOCAL_TIME_INFO)
    tip_loc_time_info_t loc_time_info;
    /// Reference Time Information (#TIP_VAL_CTS_REF_TIME_INFO)
    tip_ref_time_info_t ref_time_info;
    /// Time With DST (#TIP_VAL_NDCS_TIME_DST)
    tip_time_with_dst_t time_with_dst;
    /// Time Update State (#TIP_VAL_RTUS_TIME_UPD_STATE_VAL)
    tip_time_upd_state_t time_upd_state;
    /// Notification Configuration (#TIP_VAL_CTS_NTF_CCC_CFG)
    uint16_t ntf_cfg;
};

/// @} TIP_API_COMMON

#endif // _TIP_COMMON_H_
