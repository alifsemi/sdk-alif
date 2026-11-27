/**
 ****************************************************************************************
 *
 * @file gaf_scan_msg.h
 *
 * @brief Generic Audio Framework - Scanner - Definition of Message API
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAF_SCAN_MSG_H_
#define GAF_SCAN_MSG_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GAF_SCAN_MSG Message API
 * @ingroup GAF_SCAN
 * @brief Description of Message API for Generic Audio Framework Scanner module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf_msg.h"               // GAF Message API Definitions
#include "gaf_scan.h"              // GAF Scanner Definitions

/// @addtogroup GAF_SCAN_MSG
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List of GAF_CMD command code values for Generic Audio Framework Scanner module
enum gaf_scan_msg_cmd_code
{
    /// Start scanning (see #gaf_scan_start_cmd_t)
    GAF_SCAN_START = GAF_CODE(GAF, SCAN, GAF_SCAN_CMD_TYPE_START),
    /// Stop scanning (see #gaf_scan_stop_cmd_t)
    GAF_SCAN_STOP = GAF_CODE(GAF, SCAN, GAF_SCAN_CMD_TYPE_STOP),
};

/// List of GAF_REQ request code values for Generic Audio Framework Scanner module
enum gaf_scan_msg_req_code
{
    /// Discover (see #gaf_scan_set_params_req_t)
    GAF_SCAN_SET_PARAMS = GAF_CODE(GAF, SCAN, 0),
};

/// List of GAF_IND request code values for Generic Audio Framework Scanner module
enum gaf_scan_msg_ind_code
{
    /// Scan stopped (see #gaf_scan_stopped_ind_t)
    GAF_SCAN_STOPPED = GAF_CODE(GAF, SCAN, 0),
    /// Scan report (see #gaf_scan_report_ind_t)
    GAF_SCAN_REPORT = GAF_CODE(GAF, SCAN, 1),
    /// Scan announcement (see #gaf_scan_announcement_ind_t)
    GAF_SCAN_ANNOUNCEMENT = GAF_CODE(GAF, SCAN, 2),
};


/*
 * API MESSAGES
 ****************************************************************************************
 */

/// Structure for #GAF_SCAN_SET_PARAMS request message
typedef struct gaf_scan_set_params_req
{
    /// Request code (shall be set to #GAF_SCAN_SET_PARAMS)
    uint16_t req_code;
    /// Scan interval in multiple of 0.625ms for quicker connection setup\n
    /// Must be higher than 2.5ms\n
    /// Set to 60ms by default
    uint16_t quick_intv_slot;
    /// Scan interval in multiple of 0.625ms for reduced power scan\n
    /// Must be higher than 2.5ms\n
    /// Set to 1.28s by default
    uint16_t intv_slot;
    /// Scan window in multiple of 0.625ms for quicker connection setup\n
    /// Must be higher than 2.5ms and not larger than scan interval\n
    /// Set to 30ms by default
    uint16_t quick_wd_slot;
    /// Scan window in multiple of 0.625ms for reduced power scan\n
    /// Must be higher than 2.5ms and not larger than scan interval\n
    /// Set to 11.25ms by default
    uint16_t wd_slot;
} gaf_scan_set_params_req_t;

/// Structure for response message
typedef struct gaf_scan_rsp
{
    /// Request code (see #gaf_scan_msg_req_code enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
} gaf_scan_rsp_t;

/// Structure for #GAF_SCAN_START command message
typedef struct gaf_scan_start_cmd
{
    /// Command code (shall be set to #GAF_SCAN_START)
    uint16_t cmd_code;
    /// Configuration bit field (see #gaf_scan_cfg_bf enumeration)
    uint8_t cfg_bf;
    /// Timeout duration in seconds\n
    /// 0 means that the scan will last until #GAF_SCAN_STOP command is used
    uint8_t timeout_s;
    /// Indicate on which PHY scan has to be performed\n
    /// LE 2M PHY is not allowed
    uint8_t phy;
} gaf_scan_start_cmd_t;

/// Structure for #GAF_SCAN_STOP command message
typedef struct gaf_scan_stop_cmd
{
    /// Command code (shall be set to #GAF_SCAN_STOP)
    uint16_t cmd_code;
} gaf_scan_stop_cmd_t;

/// Structure for command complete event message
typedef struct gaf_scan_cmp_evt
{
    /// Command code (see #gaf_scan_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
} gaf_scan_cmp_evt_t;

/// Structure for #GAF_SCAN_STOPPED indication message
typedef struct gaf_scan_stopped_ind
{
    /// Indication code (shall be set to #GAF_SCAN_STOPPED)
    uint16_t ind_code;
    /// Reason (see #gaf_scan_stop_reason enumeration)
    uint8_t reason;
} gaf_scan_stopped_ind_t;

/// Structure for #GAF_SCAN_REPORT indication message
typedef struct gaf_scan_report_ind
{
    /// Indication code (shall be set to #GAF_SCAN_REPORT)
    uint16_t ind_code;
    /// Advertiser address
    gap_bdaddr_t addr;
    /// Information bit field (see #gaf_scan_report_info_bf enumeration)
    uint8_t info_bf;
    /// Air information
    gaf_adv_report_air_info_t air_info;
    /// Flags (see #gap_ad_type_flags enumeration)
    uint8_t flags;
    /// Appearance
    uint16_t appearance;
    /// TMAP roles (see #tmap_role_bf enumeration)\n
    /// Meaningful only if #GAF_SCAN_REPORT_INFO_TMAS_BIT bit set in info_bf
    uint16_t tmap_roles;
    /// Resolvable Set Identifier
    atc_csis_rsi_t rsi;
    /// Length of complete advertising data
    uint16_t length;
    /// Complete advertising data report
    uint8_t data[__ARRAY_EMPTY];
} gaf_scan_report_ind_t;

/// Structure for #GAF_SCAN_ANNOUNCEMENT indication message
typedef struct gaf_scan_announcement_ind
{
    /// Indication code (shall be set to #GAF_SCAN_ANNOUNCEMENT)
    uint16_t ind_code;
    /// Advertiser address
    gap_bdaddr_t addr;
    /// Announcement type bit field (see #gaf_announcement_type_bf enumeration)
    uint8_t type_bf;
    /// Available Audio Context bit field
    uint32_t context_bf;
    /// Metadata in LTV format
    gaf_ltv_t metadata;
} gaf_scan_announcement_ind_t;

/// @} GAF_SCAN_MSG

#endif // GAF_SCAN_MSG_H_
