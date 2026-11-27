/**
 ****************************************************************************************
 *
 * @file basc_msg.h
 *
 * @brief Header file - Battery Service Client - Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef BASC_MSG_H_
#define BASC_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup BASC_API_MSG Message API
 * @ingroup BASC_API
 * @brief Description of Message API for Battery Service Client\n
 * Support for service shall be first added using #GAPM_ADD_PROFILE_CMD message
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "basc.h"
#include "ke_msg.h"

/// @addtogroup BASC_API_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Message IDs for Battery Service Client
enum basc_msg_id
{
    /// Discover BAS in peer device's database - See #basc_discover_cmd_t
    BASC_DISCOVER_CMD = MSG_ID(BASC, 0x00u),
    /// Get characteristic value - See #basc_get_cmd_t
    BASC_GET_CMD = MSG_ID(BASC, 0x01u),
    /// Write Client Characteristic Configuration descriptor - See #basc_set_cccd_cmd_t
    BASC_SET_CCCD_CMD = MSG_ID(BASC, 0x02u),
    /// Get Characteristic Presentation Format descriptor value - See #basc_get_presentation_format_cmd_t
    BASC_GET_PRESENTATION_FORMAT_CMD = MSG_ID(BASC, 0x03u),
    /// Command completed event - See #basc_cmp_evt_t
    BASC_CMP_EVT = MSG_ID(BASC, 0x04u),
    /// Restore bond data - See #basc_restore_bond_data_req_t
    BASC_RESTORE_BOND_DATA_REQ = MSG_ID(BASC, 0x05u),
    /// Response - See #basc_rsp_t
    BASC_RSP = MSG_ID(BASC, 0x06u),
    /// Bond data updated indication - See #basc_bond_data_ind_t
    BASC_BOND_DATA_IND = MSG_ID(BASC, 0x07u),
    /// Received value indication - See #basc_value_ind_t
    BASC_VALUE_IND = MSG_ID(BASC, 0x08u),
    /// Received Characteristic Presentation Format descriptor value indication - See #basc_presentation_format_ind_t
    BASC_PRESENTATION_FORMAT_IND = MSG_ID(BASC, 0x09u),
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the #BASC_DISCOVER_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
} basc_discover_cmd_t;

/// Parameters of the #BASC_GET_CMD message
typedef struct
{
    /// Connection ind
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Characteristic type (see #basc_char_type enumeration)
    uint8_t char_type;
} basc_get_cmd_t;

/// Parameters of the #BASC_SET_CCCD_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Characteristic type (see #basc_char_type enumeration)
    uint8_t char_type;
    /// Value (see #prf_cli_conf enumeration)
    uint16_t value;
} basc_set_cccd_cmd_t;

/// Parameters of the #BASC_GET_PRESENTATION_FORMAT_CMD message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
} basc_get_presentation_format_cmd_t;

/// Parameters of the #BASC_CMP_EVT message
typedef struct
{
    /// Command code (see #basc_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status (see #hl_err enumeration)
    uint16_t status;
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Characteristic type (see #basc_char_type enumeration)
    uint8_t char_type;
} basc_cmp_evt_t;

/// Parameters of the #BASC_RESTORE_BOND_DATA_REQ message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Number of instances
    uint8_t nb_instances;
    /// Bond data
    basc_content_t bond_data[__ARRAY_EMPTY];
} basc_restore_bond_data_req_t;

/// Parameters of the #BASC_RSP message
typedef struct
{
    /// Status (see #hl_err enumeration)
    uint16_t status;
} basc_rsp_t;

/// Parameters of the #BASC_BOND_DATA_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Number of instances
    uint8_t nb_instances;
    /// Bond data
    basc_content_t bond_data[__ARRAY_EMPTY];
} basc_bond_data_ind_t;

/// Parameters of the #BASC_VALUE_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Characteristic type (see #basc_char_type enumeration)
    uint8_t char_type;
    /// Length
    uint16_t length;
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
} basc_value_ind_t;

/// Parameters of the #BASC_PRESENTATION_FORMAT_IND message
typedef struct
{
    /// Connection index
    uint8_t conidx;
    /// Instance index
    uint8_t instance_idx;
    /// Length
    uint16_t length;
    /// Value
    uint8_t value[__ARRAY_EMPTY];
} basc_presentation_format_ind_t;

/// @} BASC_API_MSG

#endif // BASC_MSG_H_
