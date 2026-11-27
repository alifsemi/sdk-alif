/**
 ****************************************************************************************
 *
 * @file cgmsc_msg.h
 *
 * @brief Header file - Continuous Glucose Monitoring Service Client - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef CGMSC_MSG_H_
#define CGMSC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup CGMSC_API_MSG Message API
 * @ingroup CGMSC_API
 * @brief Description of Message API for Continuous Glucose Monitoring Service Client\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "cgmsc.h"
#include "ke_msg.h"

/// @addtogroup CGMSC_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Continuous Glucose Monitoring Service Client
enum cgmsc_msg_id
{
    /// Discover CGMS in peer device's database - See #cgmsc_discover_cmd_t
    CGMSC_DISCOVER_CMD = MSG_ID(CGMSC, 0x00u),
    /// Get characteristic value - See #cgmsc_get_cmd_t
    CGMSC_GET_CMD = MSG_ID(CGMSC, 0x01u),
    /// Write Client Characteristic Configuration descriptor - See #cgmsc_set_cccd_cmd_t
    CGMSC_SET_CCCD_CMD = MSG_ID(CGMSC, 0x02u),
    /// Set Session Start Time - See #cgmsc_set_session_start_time_cmd_t
    CGMSC_SET_SESSION_START_TIME_CMD = MSG_ID(CGMSC, 0x03u),
    /// Request an operation through either Record Access Control Point characteristic or CGM Specific Ops Control
    /// Point characteristic
    CGMSC_CONTROL_CMD = MSG_ID(CGMSC, 0x04u),
    /// Command completed event - See #cgmsc_cmp_evt_t
    CGMSC_CMP_EVT = MSG_ID(CGMSC, 0x05u),
    /// Restore bond data - See #cgmsc_restore_bond_data_req_t
    CGMSC_RESTORE_BOND_DATA_REQ = MSG_ID(CGMSC, 0x06u),
    /// Response for #CGMSC_RESTORE_BOND_DATA_REQ message - See #cgmsc_restore_bond_data_rsp_t
    CGMSC_RESTORE_BOND_DATA_RSP = MSG_ID(CGMSC, 0x07u),
    /// Bond data updated indication - See #cgmsc_bond_data_ind_t
    CGMSC_BOND_DATA_IND = MSG_ID(CGMSC, 0x08u),
    /// Received value indication - See #cgmsc_value_ind_t
    CGMSC_VALUE_IND = MSG_ID(CGMSC, 0x09u),
    /// Received measurement indication - See #cgmsc_measurement_ind_t
    CGMSC_MEASUREMENT_IND = MSG_ID(CGMSC, 0x0Au),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #CGMSC_DISCOVER_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
} cgmsc_discover_cmd_t;

/// Parameters of the #CGMSC_GET_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #cgms_char_type enumeration)
    uint8_t char_type;
} cgmsc_get_cmd_t;

/// Parameters of the #CGMSC_SET_CCCD_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Client Characteristic Configuration descriptor type (see #cgmsc_cccd_type enumeration)
    uint8_t cccd_type;
    /// Value (see #prf_cli_conf enumeration)
    uint16_t value;
} cgmsc_set_cccd_cmd_t;

/// Parameters of the #CGMSC_SET_SESSION_START_TIME_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Data
    uint8_t data[CGMS_SESSION_START_TIME_LEN];
} cgmsc_set_session_start_time_cmd_t;

/// Parameters of the #CGMSC_CONTROL_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #cgms_char_type enumeration)
    uint8_t char_type;
    /// Length
    uint16_t length;
    /// Data
    uint8_t data[__ARRAY_EMPTY];
} cgmsc_control_cmd_t;

/// Parameters of the #CGMSC_CMP_EVT message
typedef struct
{
    /// Command code (see #cgmsc_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status (see #hl_err enumeration)
    uint16_t status;
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #cgms_char_type enumeration) or Client Characteristic Configuration descriptor type
    /// (see #cgmsc_cccd_type enumeration)
    uint8_t type;
} cgmsc_cmp_evt_t;

/// Parameters of the #CGMSC_RESTORE_BOND_DATA_REQ message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Bond data
    cgmsc_bond_data_t bond_data;
} cgmsc_restore_bond_data_req_t;

/// Parameters of the #CGMSC_RESTORE_BOND_DATA_RSP message
typedef struct
{
    /// Status (see #hl_err enumeration)
    uint16_t status;
} cgmsc_restore_bond_data_rsp_t;

/// Parameters of the #CGMSC_BOND_DATA_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Bond data
    cgmsc_bond_data_t bond_data;
} cgmsc_bond_data_ind_t;

/// Parameters of the #CGMSC_VALUE_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Characteristic type (see #cgms_char_type enumeration)
    uint8_t char_type;
    /// Length
    uint16_t length;
    /// Value
    uint8_t data[__ARRAY_EMPTY];
} cgmsc_value_ind_t;

/// Parameters of the #CGMSC_MEASUREMENT_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Length
    uint16_t length;
    /// Value
    uint8_t data[__ARRAY_EMPTY];
} cgmsc_measurement_ind_t;

/// @} CGMSC_API_MSG

#endif // CGMSC_MSG_H_
