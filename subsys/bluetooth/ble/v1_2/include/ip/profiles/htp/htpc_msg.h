/**
 ****************************************************************************************
 *
 * @file htpc_msg.h
 *
 * @brief Header file -  Health Thermometer Profile - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef HTPC_MSG_H_
#define HTPC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup HTPC_API_MSG Message API
 * @ingroup HTPC_API
 * @brief Description of Message API for Health Thermometer Service Client
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "htpc.h"

/// @addtogroup HTPC_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message id
/*@TRACE*/
enum htpc_msg_id
{
    /// Start the Health Thermometer Collector profile - at connection\n
    /// See #htpc_enable_req_t
    HTPC_ENABLE_REQ              = MSG_ID(HTPC, 0x00),
    /// Confirm that cfg connection has finished with discovery results, or that normal cnx started\n
    /// See #htpc_enable_rsp_t
    HTPC_ENABLE_RSP              = MSG_ID(HTPC, 0x01),
    /// Write Health Thermometer Notification Configuration Value request\n
    /// See #htpc_health_temp_ntf_cfg_req_t
    HTPC_HEALTH_TEMP_NTF_CFG_REQ = MSG_ID(HTPC, 0x02),
    /// Write Health Thermometer Notification Configuration Value response\n
    /// See #htpc_health_temp_ntf_cfg_rsp_t
    HTPC_HEALTH_TEMP_NTF_CFG_RSP = MSG_ID(HTPC, 0x03),
    /// APP request for measurement interval write\n
    /// See #htpc_wr_meas_intv_req_t
    HTPC_WR_MEAS_INTV_REQ        = MSG_ID(HTPC, 0x04),
    /// APP request for measurement interval write\n
    /// See #htpc_wr_meas_intv_rsp_t
    HTPC_WR_MEAS_INTV_RSP        = MSG_ID(HTPC, 0x05),
    /// Temperature value received from peer sensor\n
    /// See #htpc_temp_ind_t
    HTPC_TEMP_IND                = MSG_ID(HTPC, 0x06),
    /// Measurement interval update indication received from peer sensor\n
    /// See #htpc_meas_intv_ind_t
    HTPC_MEAS_INTV_IND           = MSG_ID(HTPC, 0x07),
    /// Generic message to read a HTP characteristic value\n
    /// See #htpc_rd_char_req_t
    HTPC_RD_CHAR_REQ             = MSG_ID(HTPC, 0x08),
    /// Read HTP characteristic value response\n
    /// See #htpc_rd_char_rsp_t
    HTPC_RD_CHAR_RSP             = MSG_ID(HTPC, 0x09),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref HTPC_ENABLE_REQ message
typedef struct htpc_enable_req
{
    /// Connection Index
    uint8_t conidx;
    /// Connection type
    uint8_t con_type;
    /// HTS existing handle values
    htpc_hts_content_t hts;
} htpc_enable_req_t;

/// Parameters of the @ref HTPC_ENABLE_RSP message
typedef struct htpc_enable_rsp
{
    /// Connection Index
    uint8_t conidx;
    /// Status
    uint16_t status;
    /// HTS handle values and characteristic properties
    htpc_hts_content_t hts;
} htpc_enable_rsp_t;

/// Parameters of the @ref HTPC_HEALTH_TEMP_NTF_CFG_REQ message
typedef struct htpc_health_temp_ntf_cfg_req
{
    /// Connection Index
    uint8_t conidx;
    /// Stop/notify/indicate value to configure into the peer characteristic
    uint16_t cfg_val;
    /// Own code for differentiating between Temperature Measurement,
    /// Intermediate Temperature and Measurement Interval chars (see #htpc_chars)
    uint8_t char_code;
} htpc_health_temp_ntf_cfg_req_t;

/// Parameters of the @ref HTPC_HEALTH_TEMP_NTF_CFG_RSP message
typedef struct htpc_health_temp_ntf_cfg_rsp
{
    /// Connection Index
    uint8_t conidx;
    /// Status
    uint16_t status;
} htpc_health_temp_ntf_cfg_rsp_t;

/// Parameters of the @ref HTPC_WR_MEAS_INTV_REQ message
typedef struct htpc_wr_meas_intv_req
{
    /// Connection Index
    uint8_t conidx;
    /// Interval value in seconds
    uint16_t intv;
} htpc_wr_meas_intv_req_t;

/// Parameters of the @ref HTPC_WR_MEAS_INTV_RSP message
typedef struct htpc_wr_meas_intv_rsp
{
    /// Connection Index
    uint8_t conidx;
    /// Status
    uint16_t status;
} htpc_wr_meas_intv_rsp_t;

/// Parameters of the @ref HTPC_TEMP_IND message
typedef struct htpc_temp_ind
{
    /// Connection Index
    uint8_t conidx;
    /// Temperature Measurement Structure
    htp_temp_meas_t temp_meas;
    /// Stable or intermediary type of temperature
    bool stable_meas;
} htpc_temp_ind_t;

/// Parameters of @ref HTPC_MEAS_INTV_IND message
typedef struct htpc_meas_intv_ind
{
    /// Connection Index
    uint8_t conidx;
    /// Interval in seconds
    uint16_t intv;
} htpc_meas_intv_ind_t;

/// Parameters of the @ref HTPC_RD_CHAR_REQ message
typedef struct htpc_rd_char_req
{
    /// Connection Index
    uint8_t conidx;
    /// Value identifier (see #htpc_val_id)
    uint8_t  val_id;
} htpc_rd_char_req_t;

/// Parameters of the @ref HTPC_RD_CHAR_RSP message
typedef struct htpc_rd_char_rsp
{
    /// Connection Index
    uint8_t conidx;
    /// Value identifier (see #htpc_val_id)
    uint8_t  val_id;
    /// Status of the request
    uint16_t status;
    /// Attribute length
    uint16_t length;
    /// Attribute value
    uint8_t  value[__ARRAY_EMPTY];
} htpc_rd_char_rsp_t;

/// @} HTPC_API_MSG

#endif // HTPC_MSG_H_
