/**
 ****************************************************************************************
 *
 * @file cgms.h
 *
 * @brief Header file - Continuous Glucose Monitoring Service - API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CGMS_H_
#define CGMS_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CGMS_API Continuous Glucose Monitoring Service (CGMS)
 * @ingroup PROFILE_API
 * @brief Description of API for Continuous Glucose Service\n See \ref cgms_msc
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup CGMS_API_COMMON Common
 * @ingroup CGMS_API
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "prf_utils.h"
#include "co_endian.h"
#include "cgmss_cfg.h"

/// @addtogroup CGMS_API_COMMON
/// @{

/*
 * DEFINES
 ****************************************************************************************
 */

/// Length of E2E-CRC value
#define CGMS_E2E_CRC_LEN (PRF_E2E_CRC_LEN)
/// Length of CGM Feature characteristic value (without E2E-CRC)
#define CGMS_FEATURE_LEN (4u)
/// Length of CGM Status characteristic value (without E2E-CRC)
#define CGMS_STATUS_LEN (5u)
/// Length of CGM Session Start Time characteristic value (without E2E-CRC)
#define CGMS_SESSION_START_TIME_LEN (9u)
/// Length of CGM Session Run Time characteristic value (without E2E-CRC)
#define CGMS_SESSION_RUN_TIME_LEN (2u)
/// Minimal length of CGM Measurement characteristic value (without E2E-CRC)
#define CGMS_MEASUREMENT_MIN_LEN (6u)
/// Minimal length of value written in Record Access Control Point characteristic value
#define CGMS_RACP_WRITTEN_MIN_LEN (2u)
/// Maximal length of value written in Record Access Control Point characteristic value
#define CGMS_RACP_WRITTEN_MAX_LEN (5u)
/// Minimal length of value written in CGM Specific Ops Control Point characteristic value (without E2E-CRC)
#define CGMS_SOCP_WRITTEN_MIN_LEN (1u)
/// Default E2E-CRC value
#define CGMS_DEFAULT_E2E_CRC (0xFFFFu)
/// Set fastest communication interval value
#define CGMS_SET_FASTEST_INTERVAL_VALUE (0xFFu)
/// Maximal length for RACP or SOCP response (without E2E-CRC)
#define CGMS_RACP_SOCP_RSP_MAX_LEN (7u)
/// Filter type - Time offset
#define CGMS_FILTER_TYPE_TIME_OFFSET (1u)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Error codes
enum cgms_err
{
    /// CRC is missing
    CGMS_ERR_MISSING_CRC= 0x80u,
    /// CRC is invalid
    CGMS_ERR_INVALID_CRC,
};

/// Characteristic type values
enum cgms_char_type
{
    /// CGM Measurement characteristic
    CGMS_CHAR_TYPE_MEASUREMENT = 0u,
    /// CGM Feature characteristic
    CGMS_CHAR_TYPE_FEATURE,
    /// Record Access Control Point characteristic
    CGMS_CHAR_TYPE_RACP,
    /// CGM Specific Ops Control Point characteristic
    CGMS_CHAR_TYPE_SOCP,
    /// CGM Status characteristic
    CGMS_CHAR_TYPE_STATUS,
    /// CGM Session Start Time characteristic
    CGMS_CHAR_TYPE_SESSION_START_TIME,
    /// CGM Session Run Time characteristic
    CGMS_CHAR_TYPE_SESSION_RUN_TIME,

    CGMS_CHAR_TYPE_MAX,
};

/// Feature bit field meaning (Byte 0)
enum cgms_feature_bf_byte_0
{
    /// Calibration supported
    CGMS_FEATURE_CALIB_SUP_POS = 0u,
    CGMS_FEATURE_CALIB_SUP_BIT = CO_BIT(CGMS_FEATURE_CALIB_SUP_POS),
    /// Patient High/Low Alerts supported
    CGMS_FEATURE_PAT_HIGH_LOW_ALERT_SUP_POS = 1u,
    CGMS_FEATURE_PAT_HIGH_LOW_ALERT_SUP_BIT = CO_BIT(CGMS_FEATURE_PAT_HIGH_LOW_ALERT_SUP_POS),
    /// Hypo Alert supported
    CGMS_FEATURE_HYPO_ALERT_SUP_POS = 2u,
    CGMS_FEATURE_HYPO_ALERT_SUP_BIT = CO_BIT(CGMS_FEATURE_HYPO_ALERT_SUP_POS),
    /// Hyper Alert supported
    CGMS_FEATURE_HYPER_ALERT_SUP_POS = 3u,
    CGMS_FEATURE_HYPER_ALERT_SUP_BIT = CO_BIT(CGMS_FEATURE_HYPER_ALERT_SUP_POS),
    /// Rate of Increase/Decrease Alert supported
    CGMS_FEATURE_RATE_OF_INCR_DECR_ALERT_SUP_POS = 4u,
    CGMS_FEATURE_RATE_OF_INCR_DECR_ALERT_SUP_BIT = CO_BIT(CGMS_FEATURE_RATE_OF_INCR_DECR_ALERT_SUP_POS),
    /// Device Specific Alert supported
    CGMS_FEATURE_DEV_SPEC_ALERT_SUP_POS = 5u,
    CGMS_FEATURE_DEV_SPEC_ALERT_SUP_BIT = CO_BIT(CGMS_FEATURE_DEV_SPEC_ALERT_SUP_POS),
    /// Sensor Malfunction Detection supported
    CGMS_FEATURE_SENSOR_MALFUNC_DETEC_SUP_POS = 6u,
    CGMS_FEATURE_SENSOR_MALFUNC_DETEC_SUP_BIT = CO_BIT(CGMS_FEATURE_SENSOR_MALFUNC_DETEC_SUP_POS),
    /// Sensor Temperature High-Low Detection supported
    CGMS_FEATURE_SENSOR_TEMP_HIGH_LOW_DETECT_SUP_POS = 7u,
    CGMS_FEATURE_SENSOR_TEMP_HIGH_LOW_DETECT_SUP_BIT = CO_BIT(CGMS_FEATURE_SENSOR_TEMP_HIGH_LOW_DETECT_SUP_POS),
};

/// Feature bit field meaning (Byte 1)
enum cgms_feature_bf_byte_1
{
    /// Sensor Result High-Low Detection supported
    CGMS_FEATURE_SENSOR_RESULT_HIGH_LOW_DETECT_SUP_POS = 0u,
    CGMS_FEATURE_SENSOR_RESULT_HIGH_LOW_DETECT_SUP_BIT = CO_BIT(CGMS_FEATURE_SENSOR_RESULT_HIGH_LOW_DETECT_SUP_POS),
    /// Low Battery Detection supported
    CGMS_FEATURE_LOW_BATT_DETECT_SUP_POS = 1u,
    CGMS_FEATURE_LOW_BATT_DETECT_SUP_BIT = CO_BIT(CGMS_FEATURE_LOW_BATT_DETECT_SUP_POS),
    /// Sensor Type Error Detection supported
    CGMS_FEATURE_SENSOR_TYPE_ERR_DETECT_SUP_POS = 2u,
    CGMS_FEATURE_SENSOR_TYPE_ERR_DETECT_SUP_BIT = CO_BIT(CGMS_FEATURE_SENSOR_TYPE_ERR_DETECT_SUP_POS),
    /// General Device Fault supported
    CGMS_FEATURE_GENERAL_DEV_FAULT_SUP_POS = 3u,
    CGMS_FEATURE_GENERAL_DEV_FAULT_SUP_BIT = CO_BIT(CGMS_FEATURE_GENERAL_DEV_FAULT_SUP_POS),
    /// E2E-CRC supported
    CGMS_FEATURE_E2E_CRC_SUP_POS = 4u,
    CGMS_FEATURE_E2E_CRC_SUP_BIT = CO_BIT(CGMS_FEATURE_E2E_CRC_SUP_POS),
    /// Multiple Bond supported
    CGMS_FEATURE_MULTI_BOND_SUP_POS = 5u,
    CGMS_FEATURE_MULTI_BOND_SUP_BIT = CO_BIT(CGMS_FEATURE_MULTI_BOND_SUP_POS),
    /// Multiple Session supported
    CGMS_FEATURE_MULTI_SESS_SUP_POS = 6u,
    CGMS_FEATURE_MULTI_SESS_SUP_BIT = CO_BIT(CGMS_FEATURE_MULTI_SESS_SUP_POS),
    /// CGM Trend Information supported
    CGMS_FEATURE_CGM_TREND_INFO_SUP_POS = 7u,
    CGMS_FEATURE_CGM_TREND_INFO_SUP_BIT = CO_BIT(CGMS_FEATURE_CGM_TREND_INFO_SUP_POS),
};

/// Feature bit field meaning (Byte 2)
enum cgms_feature_bf_byte_2
{
    /// CGM Quality supported
    CGMS_FEATURE_CGM_QUALITY_SUP_POS = 0u,
    CGMS_FEATURE_CGM_QUALITY_SUP_BIT = CO_BIT(CGMS_FEATURE_CGM_QUALITY_SUP_POS),
};

/// CGM type values
enum cgms_type
{
    /// Capillary whole blood
    CGMS_TYPE_CAPILLARY_WHOLE_BLOOD = 1u,
    /// Capillary Plasma
    CGMS_TYPE_CAPILLARY_PLASMA,
    /// Capillary whole blood 2
    CGMS_TYPE_CAPILLARY_WHOLE_BLOOD_2,
    /// Venous plasma
    CGMS_TYPE_VENOUS_PLASMA,
    /// Arterial whole blood
    CGMS_TYPE_ARTERIAL_WHOLE_BLOOD,
    /// Arterial plasma
    CGMS_TYPE_ARTERIAL_PLASMA,
    /// Undetermined whole blood
    CGMS_TYPE_UNDETERMINED_WHOLE_BLOOD,
    /// Undetermined plasma
    CGMS_TYPE_UNDETERMINED_PLASMA,
    /// Interstitial fluid
    CGMS_TYPE_INTERSTITIAL_FLUID,
    /// Control solution
    CGMS_TYPE_CONTROL_SOLUTION,
};

/// Sample location values
enum cgms_sample_location
{
    /// Finger
    CGMS_SAMPLE_LOCATION_FINGER = 1u,
    /// Alternate site test
    CGMS_SAMPLE_LOCATION_ALTERNATE_SITE_TEST,
    /// Earlobe
    CGMS_SAMPLE_LOCATION_EARLOBE,
    /// Control solution
    CGMS_SAMPLE_LOCATION_CONTROL_SOLUTION,
    /// Subcutaneous tissue
    CGMS_SAMPLE_LOCATION_SUBCUTANEOUS_TISSUE,
    /// Location not available
    CGMS_SAMPLE_LOCATION_NOT_AVAILABLE,
};

/// Operation code values for Record Access Control Point characteristic
enum cgms_ra_opcode
{
    /// Report stored records
    CGMS_RA_OPCODE_REPORT = 1u,
    /// Delete stored records
    CGMS_RA_OPCODE_DELETE,
    /// Abort operation
    CGMS_RA_OPCODE_ABORT,
    /// Report number of stored records
    CGMS_RA_OPCODE_REPORT_NUMBER,
    /// Number of stored records response
    CGMS_RA_OPCODE_REPORT_NUMBER_RSP,
    /// Response code
    CGMS_RA_OPCODE_RSP_CODE,
};

/// Operator values for Record Access Control Point characteristic
enum cgms_ra_operator
{
    /// NULL
    CGMS_RA_OPERATOR_NULL = 0u,
    /// All records
    CGMS_RA_OPERATOR_ALL,
    /// Less than or equal to
    CGMS_RA_OPERATOR_LESS_THAN_OR_EQUAL,
    /// Greater than or equal to
    CGMS_RA_OPERATOR_GREAT_THAN_OR_EQUAL,
    /// Within range of (inclusive)
    CGMS_RA_OPERATOR_WITHIN_RANGE_OF,
    /// First record (i.e. oldest record)
    CGMS_RA_OPERATOR_FIRST,
    /// Last record (i.e. most recent record)
    CGMS_RA_OPERATOR_LAST,
};

/// Response code values for Record Access Control Point characteristic
enum cgms_ra_rsp_code
{
    /// Success
    CGMS_RA_RSP_CODE_SUCCESS = 1u,
    /// Operation code not supported
    CGMS_RA_RSP_CODE_OPCODE_NOT_SUPPORTED,
    /// Invalid operator
    CGMS_RA_RSP_CODE_INVALID_OPERATOR,
    /// Operator not supported
    CGMS_RA_RSP_CODE_OPERATOR_NOT_SUPPORTED,
    /// Invalid operand
    CGMS_RA_RSP_CODE_INVALID_OPERAND,
    /// No records found
    CGMS_RA_RSP_CODE_NO_RECORDS_FOUND,
    /// Abort unsuccessful
    CGMS_RA_RSP_CODE_ABORT_UNSUCCESSFUL,
    /// Procedure not completed
    CGMS_RA_RSP_CODE_PROCEDURE_NOT_COMPLETED,
    /// Operand not supported
    CGMS_RA_RSP_CODE_OPERAND_NOT_SUPPORTED,
};

/// Operation code values for CGM Specific Ops Control Point characteristic
enum cgms_specific_opcode
{
    /// Set CGM Communication Interval\n
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// Operand: Interval in minutes (uint8_t)
    CGMS_SPECIFIC_OPCODE_SET_INTERVAL = 1u,
    /// Get CGM Communication Interval\n
    /// The normal response has #CGMS_SPECIFIC_OPCODE_INTERVAL_RSP operation code\n
    /// For error conditions, the response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// No operand
    CGMS_SPECIFIC_OPCODE_GET_INTERVAL,
    /// CGM Communication Interval response\n
    /// This is the normal response to #CGMS_SPECIFIC_OPCODE_GET_INTERVAL operation code\n
    /// Operand: Interval in minutes (uint8_t)
    CGMS_SPECIFIC_OPCODE_INTERVAL_RSP,

    /// Set Glucose Calibration Value\n
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// Operand: Calibration value (see #cgms_calibration enumeration)
    CGMS_SPECIFIC_OPCODE_SET_GLUCOSE_CALIBRATION_VALUE,
    /// Get Glucose Calibration Value\n
    /// The normal response has #CGMS_SPECIFIC_OPCODE_GLUCOSE_CALIBRATION_VALUE_RSP operation code\n
    /// For error conditions, the response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// Operand: Calibration Data Record Number (uint16_t)
    CGMS_SPECIFIC_OPCODE_GET_GLUCOSE_CALIBRATION_VALUE,
    /// Glucose Calibration Value response
    /// This is the normal response to #CGMS_SPECIFIC_OPCODE_GET_GLUCOSE_CALIBRATION_VALUE operation code\n
    /// Operand: Calibration value (see #cgms_calibration enumeration)
    CGMS_SPECIFIC_OPCODE_GLUCOSE_CALIBRATION_VALUE_RSP,

    /// Set Patient High Alert Level\n
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// Operand: Patient High bG value in mg/dL (medfloat16)
    CGMS_SPECIFIC_OPCODE_SET_HIGH_ALERT_LEVEL,
    /// Get Patient High Alert Level\n
    /// The normal response has #CGMS_SPECIFIC_OPCODE_HIGH_ALERT_LEVEL_RSP operation code\n
    /// For error conditions, the response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// No operand
    CGMS_SPECIFIC_OPCODE_GET_HIGH_ALERT_LEVEL,
    /// Patient High Alert Level response\n
    /// This is the normal response to #CGMS_SPECIFIC_OPCODE_GET_HIGH_ALERT_LEVEL operation code\n
    /// Operand: Patient High bG value in mg/dL (medfloat16)
    CGMS_SPECIFIC_OPCODE_HIGH_ALERT_LEVEL_RSP,

    /// Set Patient Low Alert Level\n
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// Operand: Patient Low bG value in mg/dL (medfloat16)
    CGMS_SPECIFIC_OPCODE_SET_LOW_ALERT_LEVEL,
    /// Get Patient Low Alert Level\n
    /// The normal response has #CGMS_SPECIFIC_OPCODE_LOW_ALERT_LEVEL_RSP operation code\n
    /// For error conditions, the response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// No operand
    CGMS_SPECIFIC_OPCODE_GET_LOW_ALERT_LEVEL,
    /// Patient Low Alert Level response\n
    /// This is the normal response to #CGMS_SPECIFIC_OPCODE_GET_LOW_ALERT_LEVEL operation code\n
    /// Operand: Patient Low bG value in mg/dL (medfloat16)
    CGMS_SPECIFIC_OPCODE_LOW_ALERT_LEVEL_RSP,

    /// Set Hypo Alert Level\n
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// Operand: Hypo Alert Level in mg/dL (medfloat16)
    CGMS_SPECIFIC_OPCODE_SET_HYPO_ALERT_LEVEL,
    /// Get Hypo Alert Level\n
    /// The normal response has #CGMS_SPECIFIC_OPCODE_HYPO_ALERT_LEVEL_RSP operation code\n
    /// For error conditions, the response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// No operand
    CGMS_SPECIFIC_OPCODE_GET_HYPO_ALERT_LEVEL,
    /// Hypo Alert Level response\n
    /// This is the normal response to #CGMS_SPECIFIC_OPCODE_GET_HYPO_ALERT_LEVEL operation code\n
    /// Operand: Hypo Alert Level in mg/dL (medfloat16)
    CGMS_SPECIFIC_OPCODE_HYPO_ALERT_LEVEL_RSP,

    /// Set Hyper Alert Level\n
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// Operand: Hyper Alert Level in mg/dL (medfloat16)
    CGMS_SPECIFIC_OPCODE_SET_HYPER_ALERT_LEVEL,
    /// Get Hyper Alert Level\n
    /// The normal response has #CGMS_SPECIFIC_OPCODE_HYPER_ALERT_LEVEL_RSP operation code\n
    /// For error conditions, the response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// No operand
    CGMS_SPECIFIC_OPCODE_GET_HYPER_ALERT_LEVEL,
    /// Hyper Alert Level response\n
    /// This is the normal response to #CGMS_SPECIFIC_OPCODE_GET_HYPER_ALERT_LEVEL operation code\n
    /// Operand: Hyper Alert Level in mg/dL (medfloat16)
    CGMS_SPECIFIC_OPCODE_HYPER_ALERT_LEVEL_RSP,

    /// Set Rate of Decrease Alert Level\n
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// Operand: Rate of Decrease Alert Level value in mg/dL/min (medfloat16)
    CGMS_SPECIFIC_OPCODE_SET_RATE_DECREASE,
    /// Get Rate of Decrease Alert Level\n
    /// The normal response has #CGMS_SPECIFIC_OPCODE_RATE_DECREASE_RSP operation code\n
    /// For error conditions, the response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// No operand
    CGMS_SPECIFIC_OPCODE_GET_RATE_DECREASE,
    /// Rate of Decrease Alert Level Response\n
    /// This is the normal response to #CGMS_SPECIFIC_OPCODE_GET_RATE_DECREASE operation code\n
    /// Operand: Rate of Decrease Alert Level value in mg/dL/min (medfloat16)
    CGMS_SPECIFIC_OPCODE_RATE_DECREASE_RSP,

    /// Set Rate of Increase Alert Level\n
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// Operand: Rate of Increase Alert Level value in mg/dL/min (medfloat16)
    CGMS_SPECIFIC_OPCODE_SET_RATE_INCREASE,
    /// Get Rate of Increase Alert Level\n
    /// The normal response has #CGMS_SPECIFIC_OPCODE_GET_RATE_INCREASE operation code\n
    /// For error conditions, the response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// No operand
    CGMS_SPECIFIC_OPCODE_GET_RATE_INCREASE,
    /// Rate of Increase Alert Level Response\n
    /// This is the normal response to #CGMS_SPECIFIC_OPCODE_GET_RATE_INCREASE operation code\n
    /// Operand: Rate of Increase Alert Level value in mg/dL/min (medfloat16)
    CGMS_SPECIFIC_OPCODE_RATE_INCREASE_RSP,

    /// Reset Device Specific Alert
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// No operand
    CGMS_SPECIFIC_OPCODE_RESET_DEV_SPEC_ALERT,
    /// Start Session\n
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// No operand
    CGMS_SPECIFIC_OPCODE_START_SESSION,
    /// Stop Session\n
    /// The response has #CGMS_SPECIFIC_OPCODE_RSP operation code\n
    /// No operand
    CGMS_SPECIFIC_OPCODE_STOP_SESSION,
    /// Response\n
    /// Operand: Request operation code (uint8_t) followed by response code value (uint8_t)
    CGMS_SPECIFIC_OPCODE_RSP,
};

/// Response code values for CGM Specific Ops Control Point characteristic
enum cgms_specific_rsp_code
{
    /// Success
    CGMS_SPECIFIC_RSP_CODE_SUCCESS = 1u,
    /// Operation code not supported
    CGMS_SPECIFIC_RSP_CODE_OPCODE_NOT_SUPPORTED,
    /// Invalid operand
    CGMS_SPECIFIC_RSP_CODE_INVALID_OPERAND,
    /// Procedure not completed
    CGMS_SPECIFIC_RSP_CODE_PROCEDURE_NOT_COMPLETED,
    /// Parameters out of range
    CGMS_SPECIFIC_RSP_CODE_OUT_OF_RANGE,
};

/// Position of fields inside Glucose Calibration value
enum cgms_calibration
{
    /// Glucose Concentration of Calibration (sfloat, mg/dL)
    CGMS_CALIBRATION_CONCENTRATION_OFFSET = 0u,
    /// Calibration time (minutes)
    CGMS_CALIBRATION_TIME_OFFSET = 2u,
    /// Calibration Type / Sample Location
    CGMS_CALIBRATION_TYPE_LOCATION_OFFSET = 4u,
    /// Next Calibration Time (minutes)
    CGMS_CALIBRATION_NEXT_TIME_OFFSET = 5u,
    /// Calibration Data Record Number
    CGMS_CALIBRATION_DATA_RECORD_NUMBER_OFFSET = 7u,
    /// Calibration Status
    CGMS_CALIBRATION_STATUS_OFFSET = 9u,
};

/*
 * MACROS
 ****************************************************************************************
 */

#if (CGMSS_E2E_CRC || BLE_CGMS_CLIENT)
/**
 ****************************************************************************************
 * @brief Allocate buffer to be used for Continuous Glucose Monitoring Service
 *
 * @param[in] pp_buf            Pointer at which address of allocated buffer is returned
 * @param[in] data_len          Data length part
 *
 * @return An error status (see #hl_err enumeration)
 ****************************************************************************************
 */
#define cgms_buf_alloc(pp_buf, data_len) \
    co_buf_alloc(pp_buf, GATT_BUFFER_HEADER_LEN, data_len, GATT_BUFFER_TAIL_LEN + CGMS_E2E_CRC_LEN)
#else // (CGMSS_E2E_CRC || BLE_CGMS_CLIENT)
#define cgms_buf_alloc(pp_buf, data_len) \
    prf_buf_alloc(pp_buf, data_len)
#endif // (CGMSS_E2E_CRC || BLE_CGMS_CLIENT)

/// @} CGMS_API_COMMON

#endif // CGMS_H_
