/**
 ****************************************************************************************
 *
 * @file bass_msg.h
 *
 * @brief Header file - Battery Service Server - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BASS_MSG_H_
#define BASS_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BASS_API_MSG Message API
 * @ingroup BASS_API
 * @brief Description of Message API for Battery Service Server\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 *     - Configuration parameter is a uint16_t bit field whose content is defined in #bass_config_bf enumeration
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bass.h"

/// @addtogroup BASS_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Battery Service Server
enum bass_msg_id
{
    /// Add service instance request - No parameters
    BASS_ADD_INSTANCE_REQ = MSG_ID(BASS, 0x00u),
    /// Update value request - See #bass_update_value_req_t
    BASS_UPDATE_VALUE_REQ = MSG_ID(BASS, 0x01u),
    /// Response - See #bass_rsp_t
    BASS_RSP = MSG_ID(BASS, 0x02u),
    /// Indication or notification sent indication - See #bass_sent_ind_t
    BASS_SENT_IND = MSG_ID(BASS, 0x03u),
    /// Value request indication - See #bass_value_req_ind_t
    BASS_VALUE_REQ_IND = MSG_ID(BASS, 0x04u),
    /// Get CCCD value request indication - See #bass_get_cccd_req_ind_t
    BASS_GET_CCCD_REQ_IND = MSG_ID(BASS, 0x05u),
    /// Presentation format request indication - See #bass_presentation_format_req_ind_t
    BASS_PRESENTATION_FORMAT_REQ_IND = MSG_ID(BASS, 0x06u),
    /// Value confirmation - See #bass_value_cfm_t
    BASS_VALUE_CFM = MSG_ID(BASS, 0x07u),
    /// Set CCCD value request indication - See #bass_set_cccd_req_ind_t
    BASS_SET_CCCD_REQ_IND = MSG_ID(BASS, 0x08u),
    /// Set CCCD value confirmation - See #bass_set_cccd_cfm_t
    BASS_SET_CCCD_CFM = MSG_ID(BASS, 0x09u),
};

/// List of request codes
enum bass_msg_req_code
{
    /// Add a service instance
    BASS_REQ_ADD_INSTANCE = 0u,
    /// Update value
    BASS_REQ_UPDATE_VALUE,
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #BASS_UPDATE_VALUE_REQ message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Characteristic type (see #bass_char_type enumeration)\n
    uint8_t char_type;
    /// Event type (see #gatt_evt_type enumeration)
    uint8_t evt_type;
    /// Length
    uint8_t length;
    /// Value\n
    /// For more details about data composition:
    ///     - Battery Level, see #bas_level_size enumeration
    ///     - Battery Level Status, see #bas_level_status_size enumeration
    ///     - Battery Critical Status, see #bas_critical_status_size enumeration
    ///     - Battery Energy Status, see #bas_energy_status_size enumeration
    ///     - Battery Time Status, see #bas_time_status_size enumeration
    ///     - Battery Health Status, see #bas_health_status_size enumeration
    ///     - Battery Health Information, see #bas_health_info_size enumeration
    ///     - Battery Information, see #bas_info_size enumeration
    ///     - Estimated Service Date, see #bas_service_date_size
    uint8_t value[__ARRAY_EMPTY];
} bass_update_value_req_t;

/// Parameters of the #BASS_RSP message
typedef struct
{
    /// Request code (see #bass_msg_req_code enumeration)
    uint8_t req_code;
    /// Status (see #hl_err enumeration)
    uint16_t status;
} bass_rsp_t;

/// Parameters of the #BASS_SENT_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Characteristic type (see #bass_char_type enumeration)
    uint8_t char_type;
    /// Status (see #hl_err enumeration)
    uint16_t status;
} bass_sent_ind_t;

/// Parameters of the #BASS_VALUE_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Characteristic type (see #bass_char_type enumeration)
    uint8_t char_type;
    /// Token
    uint16_t token;
} bass_value_req_ind_t;

/// Parameters of the #BASS_GET_CCCD_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Characteristic type (see #bass_char_type enumeration)
    uint8_t char_type;
    /// Token
    uint16_t token;
} bass_get_cccd_req_ind_t;

/// Parameters of the #BASS_VALUE_CFM message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Token
    uint16_t token;
    /// Length
    uint8_t length;
    /// Value
    uint8_t value[__ARRAY_EMPTY];
} bass_value_cfm_t;

/// Parameters of the #BASS_PRESENTATION_FORMAT_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Token
    uint16_t token;
} bass_presentation_format_req_ind_t;

/// Parameters of the #BASS_SET_CCCD_REQ_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Characteristic type (see #bass_char_type enumeration)
    uint8_t char_type;
    /// Token
    uint16_t token;
    /// Value (see #basc_char_type enumeration)
    uint16_t value;
} bass_set_cccd_req_ind_t;

/// Parameters of the #BASS_SET_CCCD_CFM message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Status (see #hl_err message)
    uint16_t status;
    /// Token
    uint16_t token;
} bass_set_cccd_cfm_t;

/// @} BASS_API_MSG

#endif // BASS_MSG_H_
