/**
 ****************************************************************************************
 *
 * @file cpps_msg.h
 *
 * @brief Header file - Cycling Power Profile Sensor - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef _CPPS_MSG_H_
#define _CPPS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @addtogroup CPPS
 * @ingroup Profile
 * @brief Cycling Power Profile  - Message API.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "cpp_common.h"

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Messages for Cycling Power Profile Sensor
/*@TRACE*/
enum cpps_msg_id
{
    /// Start the Cycling Power Profile Server profile
    CPPS_ENABLE_REQ              = MSG_ID(CPPS, 0x00),
    /// Confirm profile
    CPPS_ENABLE_RSP              = MSG_ID(CPPS, 0x01),

    /// Gets packed data for advertising
    CPPS_GET_ADV_DATA_REQ        = MSG_ID(CPPS, 0x02),
    /// Response with packed data for advertising
    CPPS_GET_ADV_DATA_RSP        = MSG_ID(CPPS, 0x03),

    /// Send a CP Measurement to the peer device (Notification)
    CPPS_NTF_CP_MEAS_CMD         = MSG_ID(CPPS, 0x04),

    /// Send a CP Vector to the peer device (Notification)
    CPPS_NTF_CP_VECTOR_CMD       = MSG_ID(CPPS, 0x05),

    /// Send a complete event status to the application
    CPPS_CMP_EVT                 = MSG_ID(CPPS, 0x06),

    /// Indicate that an attribute value has been written
    CPPS_CFG_NTFIND_IND          = MSG_ID(CPPS, 0x07),

    /// Indicate that Control Point characteristic value has been written
    CPPS_CTNL_PT_REQ_RECV_IND    = MSG_ID(CPPS, 0x08),
    /// Application response after receiving a CPPS_CTNL_PT_REQ_IND message
    CPPS_CTNL_PT_RSP_SEND_CMD    = MSG_ID(CPPS, 0x09),
};

/// Operation Code used in the profile state machine
enum cpps_op_code
{
    /// Reserved Operation Code
    CPPS_RESERVED_OP_CODE                          = 0x00,
    /// Send CP Measurement Operation Code
    CPPS_NTF_MEAS_OP_CODE                          = 0x01,
    /// Send Vector Operation Code
    CPPS_NTF_VECTOR_OP_CODE                        = 0x02,
    /// Send Control Point response
    CPPS_CTNL_PT_RESP_OP_CODE                      = 0x03,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref CPPS_CREATE_DB_REQ message
struct cpps_db_cfg
{
    /**
     * CP Feature Value - Not supposed to be modified during the lifetime of the device
     * This bit field is set in order to decide which features are supported:
     *   Supported features (specification) ---------------- Bits 0 to 18
     */
    uint32_t cp_feature;
    /// Initial count for wheel revolutions
    uint32_t wheel_rev;
    /**
     * Profile characteristic configuration:
     *   Enable broadcaster mode in Measurement Characteristic --- Bit 0
     *   Enable Control Point Characteristic (*) ----------------- Bit 1
     *
     * (*) Note this characteristic is mandatory if server supports:
     *     - Wheel Revolution Data
     *     - Multiple Sensor Locations
     *     - Configurable Settings
     *     - Offset Compensation
     *     - Server allows to be requested for parameters (CPP_CTNL_PT_REQ... codes)
     */
    uint8_t prfl_config;
    /**
     * Indicate the sensor location.
     */
    uint8_t sensor_loc;
};

/// Parameters of the @ref CPPS_ENABLE_REQ message
struct cpps_enable_req
{
    /// Connection index
    uint8_t conidx;
    /**
     * Profile characteristic configuration:
     *   Measurement Characteristic notification config --- Bit 0
     *   Measurement Characteristic broadcast config ------ Bit 1
     *   Vector Characteristic notification config -------- Bit 2
     *   Control Point Characteristic indication config --- Bit 3
     */
    uint8_t prfl_ntf_ind_cfg;
};

/// Parameters of the @ref CPPS_ENABLE_RSP message
struct cpps_enable_rsp
{
    /// Connection index
    uint8_t conidx;
    /// status
    uint16_t status;
};

/// Parameters of the @ref CPPS_GET_ADV_DATA_REQ message
struct cpps_get_adv_data_req
{
    /// Measurement parameters
    cpp_cp_meas_t parameters;
};

/// Parameters of the @ref CPPS_GET_ADV_DATA_RSP message
struct cpps_get_adv_data_rsp
{
    /// response status code
    uint16_t status;
    /// Data length
    uint8_t  data_len;
    /// Data to advertise
    uint8_t  adv_data[__ARRAY_EMPTY];
};

/// Parameters of the @ref CPPS_NTF_CP_MEAS_CMD message
struct cpps_ntf_cp_meas_cmd
{
    /// Bit field of connection index on which measurement will be send
    /// will be mask according to client that supports measurement reception
    uint32_t      conidx_bf;
    /// Cumulative Wheel Revolutions
    int16_t       cumul_wheel_rev;
    /// Measurement parameters
    cpp_cp_meas_t parameters;
};

/// Parameters of the @ref CPPS_NTF_CP_VECTOR_CMD message
struct cpps_ntf_cp_vector_cmd
{
    /// Bit field of connection index on which vector will be send
    /// will be mask according to client that supports vector reception
    uint32_t        conidx_bf;
    /// Vector parameters
    cpp_cp_vector_t parameters;
};

/// Parameters of the @ref CPPS_CTNL_PT_REQ_RECV_IND message
struct cpps_ctnl_pt_req_recv_ind
{
    /// Connection index
    uint8_t                   conidx;
    /// Operation Code (see enum #cpp_ctnl_pt_code)
    uint8_t                   op_code;
    /// Value
    union cpp_ctnl_pt_req_val value;
};

/// Parameters of the @ref CPPS_CTNL_PT_RSP_SEND_CMD message
struct cpps_ctnl_pt_rsp_send_cmd
{
    /// Connection index
    uint8_t                   conidx;
    /// Reserved for future use
    uint8_t                   rfu;
    /// Operation Code (see enum #cpp_ctnl_pt_code)
    uint8_t                   op_code;
    /// Control point response value (see enum #cpp_ctnl_pt_resp_val)
    uint8_t                   resp_val;
    /// Value
    union cpp_ctnl_pt_rsp_val value;
};

/// Parameters of the @ref CPPS_CFG_NTFIND_IND message
struct cpps_cfg_ntfind_ind
{
    /// Connection index
    uint8_t  conidx;
    /// Characteristic Code (CP Measurement or Control Point)
    uint8_t  char_code;
    /// Char. Client Characteristic Configuration
    uint16_t ntf_cfg;
};

/// Parameters of the @ref CPPS_CMP_EVT message
struct cpps_cmp_evt
{
    /// Connection index - 0xFF if meaningless (CPPS_NTF_CP_MEAS_CMD / CPPS_NTF_CP_VECTOR_CMD)
    uint8_t  conidx;
    /// Operation Code
    uint8_t  operation;
    /// Operation Status
    uint16_t status;
};

/// @} CPPS

#endif //(_CPPS_MSG_H_)
