/**
 ****************************************************************************************
 *
 * @file plxp_common.h
 *
 * @brief Header File - Pulse Oximeter Service - Common Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _PLXP_COMMON_H_
#define _PLXP_COMMON_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup PLX_API Pulse Oximeter Service (PLXS)
 * @ingroup PROFILE_API
 * @brief Description of API for Pulse Oximeter Service
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup PLX_API_COMMON Common
 * @ingroup PLX_API
 ****************************************************************************************
 */

/*
 * INCLUDED FILES
 ****************************************************************************************
 */

#include "co_math.h"
#include "prf_types.h"

/// @addtogroup PLX_API_COMMON
/// @{


/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum Features size (supported features + measurement status + device sensor status)
#define PLXP_FEAT_SIZE_MAX                   (7)
/// Maximum Control Point size OpCode + Operator
#define PLXP_RACP_SIZE_MAX                   (2)
/// Measurement Status characteristic all valid field mask
#define PLXP_MEAS_STATUS_VALID_MASK          (0xFFE0)
/// Device and Sensor Status all valid field mask
#define PLXP_DEVICE_STATUS_VALID_MASK        (0xFFFF)
/// All Valid bits of spot check flags field
#define PLXP_SPOT_MEAS_FLAGS_VALID_MASK      (0x1F)
/// All Valid bits of continuous measurement flags field
#define PLXP_CONT_MEAS_FLAGS_VALID_MASK      (0x1F)

 /*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Control Point operation codes
enum plxp_cp_opcodes_id
{
    /// Reserved
    PLXP_OPCODE_RESERVED = 0,
    /// Get stored records
    PLXP_OPCODE_REPORT_STORED_RECORDS,
    /// Delete stored records
    PLXP_OPCODE_DELETE_STORED_RECORDS,
    /// Abort operation
    PLXP_OPCODE_ABORT_OPERATION,
    /// Report number of stored reports
    PLXP_OPCODE_REPORT_NUMBER_OF_STORED_RECORDS,
    /// Number of stored reports response
    PLXP_OPCODE_NUMBER_OF_STORED_RECORDS_RESP,
    /// Response
    PLXP_OPCODE_RESPONSE_CODE,

    /// Number of operation codes
    PLXP_OPCODE_MAX,
};

/// Control Point operators
enum plxp_cp_operator_id
{
    /// Null operator
    PLXP_OPERATOR_NULL = 0,
    /// All records
    PLXP_OPERATOR_ALL_RECORDS,
};

/// Control Point Response Code Values
enum plxp_cp_resp_code_id
{
    PLXP_RESP_SUCCESS = 1,
    PLXP_RESP_OP_CODE_NOT_SUPPORTED,
    PLXP_RESP_INVALID_OPERATOR,
    PLXP_RESP_OPERATOR_NOT_SUPPORTED,
    PLXP_RESP_INVALID_OPERAND,
    PLXP_RESP_NO_RECORDS_FOUND,
    PLXP_RESP_ABORT_UNSUCCESSFUL,
    PLXP_RESP_PROCEDURE_NOT_COMPLETED,
    PLXP_RESP_OPERAND_NOT_SUPPORTED,
};

/// Supported Features bit field
enum plxp_sup_feat_bf
{
    /// Measurement Status support is present
    PLXP_FEAT_MEAS_STATUS_SUP_POS = 0,
    PLXP_FEAT_MEAS_STATUS_SUP_BIT = CO_BIT(PLXP_FEAT_MEAS_STATUS_SUP_POS),

    /// Device and Sensor Status support is present
    PLXP_FEAT_DEV_SENSOR_STATUS_SUP_POS = 1,
    PLXP_FEAT_DEV_SENSOR_STATUS_SUP_BIT = CO_BIT(PLXP_FEAT_DEV_SENSOR_STATUS_SUP_POS),

    /// Measurement Storage for Spot-check measurements is supported
    PLXP_FEAT_MEAS_STORAGE_SUP_POS = 2,
    PLXP_FEAT_MEAS_STORAGE_SUP_BIT = CO_BIT(PLXP_FEAT_MEAS_STORAGE_SUP_POS),

    /// Timestamp for Spot-check measurements is supported
    PLXP_FEAT_TIMESTAMP_SUP_POS = 3,
    PLXP_FEAT_TIMESTAMP_SUP_BIT = CO_BIT(PLXP_FEAT_TIMESTAMP_SUP_POS),

    /// SpO2PR-Fast metric is supported
    PLXP_FEAT_SPO2PR_FAST_SUP_POS = 4,
    PLXP_FEAT_SPO2PR_FAST_SUP_BIT = CO_BIT(PLXP_FEAT_SPO2PR_FAST_SUP_POS),

    /// SpO2PR-Slow metric is supported
    PLXP_FEAT_SPO2PR_SLOW_SUP_POS = 5,
    PLXP_FEAT_SPO2PR_SLOW_SUP_BIT = CO_BIT(PLXP_FEAT_SPO2PR_SLOW_SUP_POS),

    /// Pulse Amplitude Index field is supported
    PLXP_FEAT_PULSE_AMPL_SUP_POS = 6,
    PLXP_FEAT_PULSE_AMPL_SUP_BIT = CO_BIT(PLXP_FEAT_PULSE_AMPL_SUP_POS),

    /// Multiple Bonds Supported
    PLXP_FEAT_MULT_BONDS_SUP_POS = 7,
    PLXP_FEAT_MULT_BONDS_SUP_BIT = CO_BIT(PLXP_FEAT_MULT_BONDS_SUP_POS),

    // Bit 8-15 RFU
};

/// Spot check measurement characteristic flags bit field
enum plxp_spot_check_flags_bf
{
    /// Timestamp field (bit 0)
    /// 0 for not present
    /// 1 for present
    PLXP_SPOT_MEAS_FLAGS_TIMESTAMP_POS = 0,
    PLXP_SPOT_MEAS_FLAGS_TIMESTAMP_BIT = CO_BIT(PLXP_SPOT_MEAS_FLAGS_TIMESTAMP_POS),

    /// Measurement Status field (bit 1)
    /// 0 for not present
    /// 1 for present
    PLXP_SPOT_MEAS_FLAGS_MEAS_STATUS_POS = 1,
    PLXP_SPOT_MEAS_FLAGS_MEAS_STATUS_BIT = CO_BIT(PLXP_SPOT_MEAS_FLAGS_MEAS_STATUS_POS),

    /// Device and Sensor Status field (bit 2)
    /// 0 for not present
    /// 1 for present
    PLXP_SPOT_MEAS_FLAGS_DEV_SENSOR_STATUS_POS = 2,
    PLXP_SPOT_MEAS_FLAGS_DEV_SENSOR_STATUS_BIT = CO_BIT(PLXP_SPOT_MEAS_FLAGS_DEV_SENSOR_STATUS_POS),

    /// Pulse Amplitude Index field (bit 3)
    /// 0 for not present
    /// 1 for present
    PLXP_SPOT_MEAS_FLAGS_PULSE_AMPLITUDE_POS = 3,
    PLXP_SPOT_MEAS_FLAGS_PULSE_AMPLITUDE_BIT = CO_BIT(PLXP_SPOT_MEAS_FLAGS_PULSE_AMPLITUDE_POS),

    /// Device Clock is Not Set (bit 4)
    /// 0 for not present
    /// 1 for present
    PLXP_SPOT_MEAS_FLAGS_CLOCK_NOT_SET_POS = 4,
    PLXP_SPOT_MEAS_FLAGS_CLOCK_NOT_SET_BIG = CO_BIT(PLXP_SPOT_MEAS_FLAGS_CLOCK_NOT_SET_POS),

    // Bit 5-7 RFU
};

/// Continuous measurement characteristic flags bit field
enum plxp_cont_meas_flags_bf
{
    /// SpO2PR-Fast field is present
    PLXP_CONT_MEAS_FLAGS_SPO2PR_FAST_POS = 0,
    PLXP_CONT_MEAS_FLAGS_SPO2PR_FAST_BIT = CO_BIT(PLXP_CONT_MEAS_FLAGS_SPO2PR_FAST_POS),

    //// SpO2PR-Slow field is present
    PLXP_CONT_MEAS_FLAGS_SPO2PR_SLOW_POS = 1,
    PLXP_CONT_MEAS_FLAGS_SPO2PR_SLOW_BIT = CO_BIT(PLXP_CONT_MEAS_FLAGS_SPO2PR_SLOW_POS),

    /// Measurement Status field is present
    PLXP_CONT_MEAS_FLAGS_MEAS_STATUS_POS = 2,
    PLXP_CONT_MEAS_FLAGS_MEAS_STATUS_BIT = CO_BIT(PLXP_CONT_MEAS_FLAGS_MEAS_STATUS_POS),

    /// Device and Sensor Status field is present
    PLXP_CONT_MEAS_FLAGS_DEV_SENSOR_STATUS_POS = 3,
    PLXP_CONT_MEAS_FLAGS_DEV_SENSOR_STATUS_BIT = CO_BIT(PLXP_CONT_MEAS_FLAGS_DEV_SENSOR_STATUS_POS),

    /// Pulse Amplitude Index field is present
    PLXP_CONT_MEAS_FLAGS_PULSE_AMPL_POS = 4,
    PLXP_CONT_MEAS_FLAGS_PULSE_AMPL_BIT = CO_BIT(PLXP_CONT_MEAS_FLAGS_PULSE_AMPL_POS),

    // Bit 5-7 RFU
};

/// Measurement Status field (not used)
enum plxp_meas_status_sup_bf
{
    // Bit 0-4 RFU
    /// Measurement Ongoing bit supported
    PLXP_MEAS_STATUS_MEAS_ONGOING_SUP_POS = 5,
    PLXP_MEAS_STATUS_MEAS_ONGOING_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_MEAS_ONGOING_SUP_POS),

    /// Early Estimated Data bit supported
    PLXP_MEAS_STATUS_EARLY_ESTIM_DATA_SUP_POS = 6,
    PLXP_MEAS_STATUS_EARLY_ESTIM_DATA_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_EARLY_ESTIM_DATA_SUP_POS),

    /// Validated Data bit supported
    PLXP_MEAS_STATUS_VALIDATED_DATA_SUP_POS = 7,
    PLXP_MEAS_STATUS_VALIDATED_DATA_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_VALIDATED_DATA_SUP_POS),

    /// Fully Qualified Data bit supported
    PLXP_MEAS_STATUS_FULLY_QUALIF_DATA_SUP_POS = 8,
    PLXP_MEAS_STATUS_FULLY_QUALIF_DATA_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_FULLY_QUALIF_DATA_SUP_POS),

    /// Data from Measurement Storage bit supported
    PLXP_MEAS_STATUS_DATA_FROM_MEAS_STRG_SUP_POS = 9,
    PLXP_MEAS_STATUS_DATA_FROM_MEAS_STRG_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_DATA_FROM_MEAS_STRG_SUP_POS),

    /// Data for Demonstration bit supported
    PLXP_MEAS_STATUS_DATA_FOR_DEMO_SUP_POS = 10,
    PLXP_MEAS_STATUS_DATA_FOR_DEMO_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_DATA_FOR_DEMO_SUP_POS),

    /// Data for Testing bit supported
    PLXP_MEAS_STATUS_DATA_FOR_TEST_SUP_POS = 11,
    PLXP_MEAS_STATUS_DATA_FOR_TEST_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_DATA_FOR_TEST_SUP_POS),

    /// Calibration Ongoing bit supported
    PLXP_MEAS_STATUS_CALIB_ONGOING_SUP_POS = 12,
    PLXP_MEAS_STATUS_CALIB_ONGOING_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_CALIB_ONGOING_SUP_POS),

    /// Measurement Unavailable bit supported
    PLXP_MEAS_STATUS_MEAS_UNAVAILB_SUP_POS = 13,
    PLXP_MEAS_STATUS_MEAS_UNAVAILB_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_MEAS_UNAVAILB_SUP_POS),

    /// Questionable Measurement Detected bit supported
    PLXP_MEAS_STATUS_QUEST_MEAS_DET_SUP_POS = 14,
    PLXP_MEAS_STATUS_QUEST_MEAS_DET_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_QUEST_MEAS_DET_SUP_POS),

    /// Invalid Measurement Detected bit supported
    PLXP_MEAS_STATUS_INV_MEAS_DET_SUP_POS = 15,
    PLXP_MEAS_STATUS_INV_MEAS_DET_SUP_BIT = CO_BIT(PLXP_MEAS_STATUS_INV_MEAS_DET_SUP_POS),
};

/// Device and Sensor Status field (not used)
enum plxp_dev_sensor_status_bf
{
    /// Extended Display Update Ongoing
    PLXP_DEV_STATUS_EXT_DISP_UPDATE_ONGOING_POS = 0,
    PLXP_DEV_STATUS_EXT_DISP_UPDATE_ONGOING_BIT = CO_BIT(PLXP_DEV_STATUS_EXT_DISP_UPDATE_ONGOING_POS),

    /// Signal Processing Irregularity Detected bit supported
    PLXP_DEV_STATUS_EQUIP_MALFUNC_DET_SUP_POS = 1,
    PLXP_DEV_STATUS_EQUIP_MALFUNC_DET_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_EQUIP_MALFUNC_DET_SUP_POS),

    /// Signal Processing Irregularity Detected bit supported
    PLXP_DEV_STATUS_SIGNAL_PROC_IRREGUL_DET_SUP_POS = 2,
    PLXP_DEV_STATUS_SIGNAL_PROC_IRREGUL_DET_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_SIGNAL_PROC_IRREGUL_DET_SUP_POS),

    /// Inadequate Signal Detected bit supported
    PLXP_DEV_STATUS_INADEQ_SIGNAL_DET_SUP_POS = 3,
    PLXP_DEV_STATUS_INADEQ_SIGNAL_DET_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_INADEQ_SIGNAL_DET_SUP_POS),

    /// Poor Signal Detected bit supported
    PLXP_DEV_STATUS_POOR_SIGNAL_DET_SUP_POS = 4,
    PLXP_DEV_STATUS_POOR_SIGNAL_DET_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_POOR_SIGNAL_DET_SUP_POS),

    /// Low Perfusion Detected bit supported
    PLXP_DEV_STATUS_LOW_PERF_DET_SUP_POS = 5,
    PLXP_DEV_STATUS_LOW_PERF_DET_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_LOW_PERF_DET_SUP_POS),

    /// Erratic Signal Detected bit supported
    PLXP_DEV_STATUS_ERRATIC_SIGNAL_DET_SUP_POS = 6,
    PLXP_DEV_STATUS_ERRATIC_SIGNAL_DET_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_ERRATIC_SIGNAL_DET_SUP_POS),

    /// Nonpulsatile Signal Detected bit supported
    PLXP_DEV_STATUS_NONPULS_SIGNAL_DET_SUP_POS = 7,
    PLXP_DEV_STATUS_NONPULS_SIGNAL_DET_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_NONPULS_SIGNAL_DET_SUP_POS),

    /// Questionable Pulse Detected bit supported
    PLXP_DEV_STATUS_QUEST_PULSE_DET_SUP_POS = 8,
    PLXP_DEV_STATUS_QUEST_PULSE_DET_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_QUEST_PULSE_DET_SUP_POS),

    /// Signal Analysis Ongoing bit supported
    PLXP_DEV_STATUS_SIGNAL_ANAL_ONGOING_SUP = 9,
    PLXP_DEV_STATUS_SIGNAL_ANAL_ONGOING_BIT = CO_BIT(PLXP_DEV_STATUS_SIGNAL_ANAL_ONGOING_SUP),

    /// Sensor Interference Detected bit supported
    PLXP_DEV_STATUS_SENSOR_INTERF_DET_SUP_POS = 10,
    PLXP_DEV_STATUS_SENSOR_INTERF_DET_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_SENSOR_INTERF_DET_SUP_POS),

    /// Sensor Unconnected to User bit supported
    PLXP_DEV_STATUS_SENSOR_UNCONNCTD_TO_USER_SUP_POS = 11,
    PLXP_DEV_STATUS_SENSOR_UNCONNCTD_TO_USER_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_SENSOR_UNCONNCTD_TO_USER_SUP_POS),

    /// Unknown Sensor Connected bit supported
    PLXP_DEV_STATUS_UNKNOWN_SENSOR_CONNCTD_SUP_POS = 12,
    PLXP_DEV_STATUS_UNKNOWN_SENSOR_CONNCTD_SUP_BIT   = CO_BIT(PLXP_DEV_STATUS_UNKNOWN_SENSOR_CONNCTD_SUP_POS),

    /// Sensor Displaced bit supported
    PLXP_DEV_STATUS_SENSOR_DISPL_SUP_POS = 13,
    PLXP_DEV_STATUS_SENSOR_DISPL_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_SENSOR_DISPL_SUP_POS),

    /// Sensor Malfunctioning bit supported
    PLXP_DEV_STATUS_SENSOR_MALFUNC_SUP_POS = 14,
    PLXP_DEV_STATUS_SENSOR_MALFUNC_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_SENSOR_MALFUNC_SUP_POS),

    /// Sensor Disconnected bit supported
    PLXP_DEV_STATUS_SENSOR_DISCONNCTD_SUP_POS = 15,
    PLXP_DEV_STATUS_SENSOR_DISCONNCTD_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_SENSOR_DISCONNCTD_SUP_POS),

    // Bit 16-23 RFU
};

/*
 * TYPES DEFINITION
 ****************************************************************************************
 */

/// PLX Features value
typedef struct plxp_features
{
    /// Supported Features (see #plxp_sup_feat_bf enumeration)
    uint16_t sup_feat;
    /// Measurement Status Support (see #plxp_meas_status_sup_bf enumeration)
    /// Meaningful only if Measurement Status feature indicated as supported in Supported Features
    uint16_t meas_stat_sup;
    /// Device and Sensor Status Support (see #plxp_dev_sensor_status_bf enumeration)
    /// Meaningful only if Device and Sensor Status feature indicated as supported in Supported Features
    uint32_t dev_stat_sup;
} plxp_features_t;

/// Record Access Control Point Request structure
typedef struct plxp_racp_req
{
    /// Operation code (see #plxp_cp_opcodes_id enumeration)
    uint8_t cp_opcode;
    /// Operator (see #plxp_cp_operator_id enumeration)
    uint8_t cp_operator;
} plxp_racp_req_t;

/// Record Access Control Point Response structure
typedef struct plxp_racp_rsp
{
    /// Request operation code (see #plxp_cp_opcodes_id enumeration)
    uint8_t req_cp_opcode;
    /// Response code (see #plxp_cp_resp_code_id enumeration)
    uint8_t rsp_code;
    /// Number of records
    uint16_t rec_num;
} plxp_racp_rsp_t;

/// PLX Spot-Check Measurement value
typedef struct plxp_spot_meas
{
    /// Flags (see #plxp_spot_check_flags_bf enumeration)
    uint8_t spot_flags;
    /// SpO2 (Percentage with a resolution of 1)
    uint16_t sp_o2;
    /// Pulse Rate (beats per minute, with a resolution of 1)
    uint16_t pr;
    /// Timestamp (if present)
    prf_date_time_t timestamp;
    /// Measurement Status (see #plxp_meas_status_sup_bf enumeration)
    uint16_t meas_stat;
    /// Device and Sensor Status (see #plxp_dev_sensor_status_bf enumeration)
    uint32_t dev_sensor_stat;
    /// Pulse Amplitude Index - Unit is percentage with a resolution of 1
    uint16_t pulse_ampl;
} plxp_spot_meas_t;

/// SpO2 - PR measurements
typedef struct plxp_spo2pr
{
    /// SpO2 (percentage with a resolution of 1)
    uint16_t sp_o2;
    /// Pulse Rate (beats per minute, with a resolution of 1)
    uint16_t pr;
} plxp_spo2pr_t;

/// PLX Continuous Measurement Value
typedef struct plxp_cont_meas
{
    /// Flags (see #plxp_cont_meas_flags_bf enumeration)
    uint8_t cont_flags;
    /// SpO2 - PR measurements - Normal
    plxp_spo2pr_t normal;
    /// SpO2 - PR measurements - Fast
    plxp_spo2pr_t fast;
    /// SpO2 - PR measurements - Slow
    plxp_spo2pr_t slow;
    /// Measurement Status (see #plxp_meas_status_sup_bf enumeration)
    uint16_t meas_stat;
    /// Device and Sensor Status (see #plxp_dev_sensor_status_bf enumeration)
    uint32_t dev_sensor_stat;
    /// Pulse Amplitude Index
    uint16_t pulse_ampl;
} plxp_cont_meas_t;

/// @} PLX_API_COMMON

#endif // _PLXP_COMMON_H_
