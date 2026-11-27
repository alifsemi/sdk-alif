/**
 ****************************************************************************************
 *
 * @file htpt_msg.h
 *
 * @brief Header file - Health Thermometer Profile Thermometer - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef HTPT_MSG_H_
#define HTPT_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup HTPT_API_MSG Message API
 * @ingroup HTPT_API
 * @brief Description of Message API for Health Thermometer Service Server
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "htpt.h"

/// @addtogroup HTPT_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Health Thermometer Profile Thermometer
/*@TRACE*/
enum htpt_msg_id
{
    /// Start the Health Thermometer Profile Thermometer profile - at connection\n
    /// See #htpt_enable_req_t
    HTPT_ENABLE_REQ            = MSG_ID(HTPT, 0x00),
    /// Enable confirmation\n
    /// See #htpt_enable_rsp_t
    HTPT_ENABLE_RSP            = MSG_ID(HTPT, 0x01),
    /// Send temperature value from APP\n
    /// See #htpt_temp_send_req_t
    HTPT_TEMP_SEND_REQ         = MSG_ID(HTPT, 0x02),
    /// Send temperature response\n
    /// See #htpt_temp_send_rsp_t
    HTPT_TEMP_SEND_RSP         = MSG_ID(HTPT, 0x03),
    /// Indicate Measurement Interval\n
    /// See #htpt_meas_intv_upd_req_t
    HTPT_MEAS_INTV_UPD_REQ     = MSG_ID(HTPT, 0x04),
    /// Send Measurement Interval response\n
    /// See #htpt_meas_intv_upd_rsp_t
    HTPT_MEAS_INTV_UPD_RSP     = MSG_ID(HTPT, 0x05),
    /// Inform APP of new measurement interval value requested by a peer device\n
    /// See #htpt_meas_intv_chg_req_ind_t
    HTPT_MEAS_INTV_CHG_REQ_IND = MSG_ID(HTPT, 0x06),
    /// APP Confirm message of new measurement interval value requested by a peer device\n
    /// If accepted, it triggers indication on measurement interval attribute\n
    /// See #htpt_meas_intv_chg_cfm_t
    HTPT_MEAS_INTV_CHG_CFM     = MSG_ID(HTPT, 0x07),
    /// Inform APP that Indication Configuration has been changed - use to update bond data\n
    /// See #htpt_cfg_indntf_ind_t
    HTPT_CFG_INDNTF_IND        = MSG_ID(HTPT, 0x08),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #HTPT_ENABLE_REQ message
typedef struct htpt_enable_req
{
    /// Connection index
    uint8_t conidx;
    /// Notification configuration (Bond Data to restore: see enum #htpt_ntf_ind_cfg)
    uint8_t ntf_ind_cfg;
} htpt_enable_req_t;

/// Parameters of the #HTPT_ENABLE_RSP message
typedef struct htpt_enable_rsp
{
    /// Connection index
    uint8_t conidx;
    /// Status of enable request
    uint16_t status;
} htpt_enable_rsp_t;

/// Parameters of the #HTPT_TEMP_SEND_REQ message
typedef struct htpt_temp_send_req
{
    /// Temperature Measurement
    htp_temp_meas_t temp_meas;
    /// Stable or intermediary type of temperature (True stable meas, else false)
    bool stable_meas;
} htpt_temp_send_req_t;

/// Parameters of the #HTPT_TEMP_SEND_RSP message
typedef struct htpt_temp_send_rsp
{
    /// Status
    uint16_t status;
} htpt_temp_send_rsp_t;

/// Parameters of the #HTPT_MEAS_INTV_UPD_REQ message
typedef struct htpt_meas_intv_upd_req
{
    /// Measurement Interval value
    uint16_t meas_intv;
} htpt_meas_intv_upd_req_t;

/// Parameters of the #HTPT_MEAS_INTV_UPD_RSP message
typedef struct htpt_meas_intv_upd_rsp
{
    /// status
    uint16_t status;
} htpt_meas_intv_upd_rsp_t;

/// Parameters of the #HTPT_MEAS_INTV_CHG_REQ_IND message
typedef struct htpt_meas_intv_chg_req_ind
{
    /// Connection index
    uint8_t conidx;
    /// new measurement interval
    uint16_t intv;
} htpt_meas_intv_chg_req_ind_t;

/// Parameters of the #HTPT_MEAS_INTV_CHG_CFM message
typedef struct htpt_meas_intv_chg_cfm
{
    /// Connection index
    uint8_t conidx;
    /// status of the request
    uint16_t status;
} htpt_meas_intv_chg_cfm_t;

/// Parameters of the #HTPT_CFG_INDNTF_IND message
typedef struct htpt_cfg_indntf_ind
{
    /// connection index
    uint8_t conidx;
    /// Notification Configuration (see enum #htpt_ntf_ind_cfg)
    uint8_t ntf_ind_cfg;
} htpt_cfg_indntf_ind_t;

/// @} HTPT_API_MSG

#endif // HTPT_MSG_H_
