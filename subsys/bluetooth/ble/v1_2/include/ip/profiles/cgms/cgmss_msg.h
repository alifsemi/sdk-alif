/**
 ****************************************************************************************
 *
 * @file cgmss_msg.h
 *
 * @brief Header file - Continuous Glucose Monitoring Service Server - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CGMSS_MSG_H_
#define CGMSS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CGMSS_API_MSG Message API
 * @ingroup CGMSS_API
 * @brief Description of Message API for Continuous Glucose Monitoring Service Server\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 *     - Configuration parameter is a uint8_t bit field whose content is defined in #cgmss_config_bf enumeration
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "cgmss.h"

/// @addtogroup CGMSS_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Battery Service Server
enum cgmss_msg_id
{
    /// Send a measurement - See #cgmss_send_measurement_req_t
    CGMSS_SEND_MEASUREMENT_REQ = MSG_ID(CGMSS, 0x00u),
    /// Send update for CGM Feature characteristic value - See #cgmss_send_features_req_t
    CGMSS_SEND_FEATURES_REQ = MSG_ID(CGMSS, 0x01u),
    /// Send response for one of Control Point characteristic - See #cgmss_send_control_response_req_t
    CGMSS_SEND_CONTROL_RESPONSE_REQ = MSG_ID(CGMSS, 0x02u),
    /// Response - See #cgmss_rsp_t
    CGMSS_RSP = MSG_ID(CGMSS, 0x03u),
    /// Indication or notification sent indication - See #cgmss_sent_ind_t
    CGMSS_SENT_IND = MSG_ID(CGMSS, 0x04u),
    /// Request to set Session Start Time indication - See #cgmss_set_session_start_time_req_ind_t\n
    /// Shall answer with #CGMSS_SET_VALUE_CFM message
    CGMSS_SET_SESSION_START_TIME_REQ_IND = MSG_ID(CGMSS, 0x05u),
    /// Value request indication - See #cgmss_value_req_ind_t\n
    /// Shall answer with #CGMSS_VALUE_CFM message
    CGMSS_VALUE_REQ_IND = MSG_ID(CGMSS, 0x06u),
    /// Control request indication - See #cgmss_control_req_ind_t\n
    /// Shall answer with #CGMSS_VALUE_CFM message
    CGMSS_CONTROL_REQ_IND = MSG_ID(CGMSS, 0x07u),
    /// Get CCCD value request indication - See #cgmss_get_cccd_req_ind_t\n
    /// Shall answer with #CGMSS_GET_CCCD_CFM message
    CGMSS_GET_CCCD_REQ_IND = MSG_ID(CGMSS, 0x08u),
    /// Set CCCD value request indication - See #cgmss_set_cccd_req_ind_t\n
    /// Shall answer with #CGMSS_SET_VALUE_CFM message
    CGMSS_SET_CCCD_REQ_IND = MSG_ID(CGMSS, 0x09u),
    /// Value confirmation - See #cgmss_value_cfm_t
    CGMSS_VALUE_CFM = MSG_ID(CGMSS, 0x0Au),
    /// Set CCCD value confirmation - See #cgmss_set_value_cfm_t
    CGMSS_SET_VALUE_CFM = MSG_ID(CGMSS, 0x0Bu),
    /// CCCD value confirmation - See #cgmss_get_cccd_cfm_t
    CGMSS_GET_CCCD_CFM = MSG_ID(CGMSS, 0x0Cu),
};

/// List of request codes
enum cgmss_msg_req_code
{
    /// Send a measurement
    CGMSS_REQ_SEND_MEASUREMENT = 0u,
    /// Send update for CGM Feature characteristic value
    CGMSS_REQ_SEND_FEATURES,
    /// Send response for one of Control Point characteristic
    CGMSS_REQ_SEND_CONTROL_RESPONSE,
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #CGMSS_SEND_MEASUREMENT_REQ message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Length
    uint8_t length;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} cgmss_send_measurement_req_t;

/// Parameters of the #CGMSS_SEND_FEATURES_REQ message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Length
    uint8_t length;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} cgmss_send_features_req_t;

/// Parameters of the #CGMSS_SEND_CONTROL_RESPONSE_REQ message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #cgms_char_type enumeration)
    uint8_t char_type;
    /// Length
    uint8_t length;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} cgmss_send_control_response_req_t;

/// Parameters of the #CGMSS_RSP message
typedef struct
{
    /// Request code (see #cgmss_msg_req_code enumeration)
    uint8_t req_code;
    /// Status (see #hl_err enumeration)
    uint16_t status;
} cgmss_rsp_t;

/// Parameters of the #CGMSS_SENT_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #cgms_char_type enumeration)
    uint8_t char_type;
    /// Status (see #hl_err enumeration)
    uint16_t status;
} cgmss_sent_ind_t;

/// Parameters of the #CGMSS_SET_SESSION_START_TIME_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Token
    uint16_t token;
    /// Data
    uint8_t data[CGMS_SESSION_START_TIME_LEN];
} cgmss_set_session_start_time_req_ind_t;

/// Parameters of the #CGMSS_VALUE_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #cgms_char_type enumeration)
    uint8_t char_type;
    /// Token
    uint16_t token;
} cgmss_value_req_ind_t;

/// Parameters of the #CGMSS_CONTROL_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #cgms_char_type enumeration)
    uint8_t char_type;
    /// Token
    uint16_t token;
    /// Length
    uint8_t length;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} cgmss_control_req_ind_t;

/// Parameters of the #CGMSS_GET_CCCD_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #cgms_char_type enumeration)
    uint8_t char_type;
    /// Token
    uint16_t token;
} cgmss_get_cccd_req_ind_t;

/// Parameters of the #CGMSS_SET_CCCD_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #cgms_char_type enumeration)
    uint8_t char_type;
    /// Token
    uint16_t token;
    /// Value
    uint16_t value;
} cgmss_set_cccd_req_ind_t;

/// Parameters of the #CGMSS_VALUE_CFM message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Token
    uint16_t token;
    /// Characteristic type (see #cgms_char_type enumeration)
    uint8_t char_type;
    /// Length
    uint8_t length;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} cgmss_value_cfm_t;

/// Parameters of the #CGMSS_SET_VALUE_CFM message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Status (see #hl_err message)
    uint16_t status;
    /// Token
    uint16_t token;
} cgmss_set_value_cfm_t;

/// Parameters of the #CGMSS_GET_CCCD_CFM message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Token
    uint16_t token;
    /// Data
    uint8_t data[PRF_CCC_DESC_LEN];
} cgmss_get_cccd_cfm_t;

/// @} CGMSS_API_MSG

#endif // CGMSS_MSG_H_
