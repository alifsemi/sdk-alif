/**
 ****************************************************************************************
 *
 * @file gaf_scan.h
 *
 * @brief Generic Audio Framework - Scanner - Definitions
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAF_SCAN_H_
#define GAF_SCAN_H_

#include "rom_build_cfg.h"

/**
 ****************************************************************************************
 * @defgroup GAF_SCAN Scanner module
 * @ingroup GAF_TB
 * @brief Description of Generic Audio Framework Scanner module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_SCAN_ENUM Enumerations
 * @ingroup GAF_SCAN
 * @brief Enumerations for Generic Audio Framework Scanner module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_SCAN_NATIVE_API Native API
 * @ingroup GAF_SCAN
 * @brief Description of Native API for Generic Audio Framework Scanner module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_SCAN_CALLBACK Callback Functions
 * @ingroup GAF_SCAN_NATIVE_API
 * @brief Description of callback functions for Generic Audio Framework Scanner module
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup GAF_SCAN_FUNCTION Functions
 * @ingroup GAF_SCAN_NATIVE_API
 * @brief Description of functions for Generic Audio Framework Scanner module
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gaf_cfg.h"                // Generic Audio Framework Configuration
#include "gaf.h"                    // GAF Definitions
#include "atc_csi.h"                // CSIS Definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// @addtogroup GAF_SCAN_ENUM
/// @{

/// List of command type value for Generic Audio Framework Scanner module
enum gaf_scan_cmd_type
{
    /// Start to scan for devices advertising support of GAF features
    GAF_SCAN_CMD_TYPE_START = 0,
    /// Stop scan
    GAF_SCAN_CMD_TYPE_STOP,

    GAF_SCAN_CMD_TYPE_MAX,
};

/// Scan configuration bit field meaning (see #gaf_scan_start function)
enum gaf_scan_cfg_bf
{
    /// Use Limited Discovery or General Discovery mode - Position
    GAF_SCAN_CFG_LIMITED_DISCOVERY_POS = 0,
    /// Use Limited Discovery or General Discovery mode - Bit
    GAF_SCAN_CFG_LIMITED_DISCOVERY_BIT = CO_BIT(GAF_SCAN_CFG_LIMITED_DISCOVERY_POS),

    /// Require presence of ASCS UUID in Service UUID AD data type - Position
    GAF_SCAN_CFG_ASCS_REQ_POS = 1,
    /// Require presence of ASCS UUID in Service UUID AD data type - Bit
    GAF_SCAN_CFG_ASCS_REQ_BIT = CO_BIT(GAF_SCAN_CFG_ASCS_REQ_POS),

    /// Require presence of HAS UUID in Service UUID AD data type - Position
    GAF_SCAN_CFG_HAS_REQ_POS = 2,
    /// Require presence of HAS UUID in Service UUID AD data type - Bit
    GAF_SCAN_CFG_HAS_REQ_BIT = CO_BIT(GAF_SCAN_CFG_HAS_REQ_POS),

    /// Require presence of TMAS UUID in Service UUID AD data type - Position
    GAF_SCAN_CFG_TMAS_REQ_POS = 3,
    /// Require presence of TMAS UUID in Service UUID AD data type - Bit
    GAF_SCAN_CFG_TMAS_REQ_BIT = CO_BIT(GAF_SCAN_CFG_TMAS_REQ_POS),
};

/// Scanning stop reason values
enum gaf_scan_stop_reason
{
    /// Requested by Upper Layer
    GAF_SCAN_STOP_REASON_UL = 0,
    /// Internal error
    GAF_SCAN_STOP_REASON_INTERNAL_ERROR,
    /// Timeout
    GAF_SCAN_STOP_REASON_TIMEOUT,
};

/// Report information bit field meaning
enum gaf_scan_report_info_bf
{
    /// Indicate if support of ASCS is reported in the advertising report - Position
    GAF_SCAN_REPORT_INFO_ASCS_POS = 0,
    /// Indicate if support of ASCS is reported in the advertising report - Position
    GAF_SCAN_REPORT_INFO_ASCS_BIT = CO_BIT(GAF_SCAN_REPORT_INFO_ASCS_POS),

    /// Indicate if support of TMAS is reported in the advertising report - Position
    GAF_SCAN_REPORT_INFO_TMAS_POS = 1,
    /// Indicate if support of TMAS is reported in the advertising report - Position
    GAF_SCAN_REPORT_INFO_TMAS_BIT = CO_BIT(GAF_SCAN_REPORT_INFO_TMAS_POS),

    /// Indicate if support of HAS is reported in the advertising report - Position
    GAF_SCAN_REPORT_INFO_HAS_POS = 2,
    /// Indicate if support of HAS is reported in the advertising report - Position
    GAF_SCAN_REPORT_INFO_HAS_BIT = CO_BIT(GAF_SCAN_REPORT_INFO_HAS_POS),

    /// Indicate if appearance is indicated in the advertising report - Position
    GAF_SCAN_REPORT_INFO_APPEARANCE_POS = 3,
    /// Indicate if appearance is indicated in the advertising report - Position
    GAF_SCAN_REPORT_INFO_APPEARANCE_BIT = CO_BIT(GAF_SCAN_REPORT_INFO_APPEARANCE_POS),

    /// Indicate if General or Targeted Announcement is part of the advertising report - Position
    GAF_SCAN_REPORT_INFO_ANNOUNCEMENT_POS = 4,
    /// Indicate if General or Targeted Announcement is part of the advertising report - Position
    GAF_SCAN_REPORT_INFO_ANNOUNCEMENT_BIT = CO_BIT(GAF_SCAN_REPORT_INFO_ANNOUNCEMENT_POS),

    /// Indicate if an RSI value is part of the advertising report - Position
    GAF_SCAN_REPORT_INFO_RSI_POS = 5,
    /// Indicate if an RSI value is part of the advertising report - Position
    GAF_SCAN_REPORT_INFO_RSI_BIT = CO_BIT(GAF_SCAN_REPORT_INFO_RSI_POS),
};

/// @} GAF_SCAN_ENUM

/*
 * CALLBACK FUNCTIONS DEFINITION
 ****************************************************************************************
 */

/// @addtogroup GAF_SCAN_CALLBACK
/// @{

/**
 ****************************************************************************************
 * @brief Callback function called when handling of a command has been completed
 *
 * @param[in] cmd_type      Command type (see #gaf_scan_cmd_type enumeration)
 * @param[in] status        Status
 ****************************************************************************************
 */
typedef void (*gaf_scan_cb_cmp_evt)(uint8_t cmd_type, uint16_t status);

/**
 ****************************************************************************************
 * @brief Callback function called when scan has been stopped outside scope of #GAF_SCAN_CMD_TYPE_STOP command
 * handling
 *
 * @param[in] reason        Reason why scan has been stopped (see #gaf_scan_stop_reason enumeration)
 ****************************************************************************************
 */
typedef void (*gaf_scan_cb_stopped)(uint8_t reason);

/**
 ****************************************************************************************
 * @brief Callback function called each time a valid GAF advertising report is received
 *
 * @param[in] p_addr        Pointer to Advertiser address
 * @param[in] info_bf       Information bit field (see #gaf_scan_report_info_bf enumeration)
 * @param[in] p_air_info    Pointer to air information structure
 * @param[in] flags         Flags value (see #gap_ad_type_flags enumeration)
 * @param[in] appearance    Appearance
 * @param[in] tmap_roles    TMAP roles (see #tmap_role_bf enumeration)\n
 *                          Meaningful only if #GAF_SCAN_REPORT_INFO_TMAS_BIT bit set in info_bf
 * @param[in] p_rsi         Pointer to received Resolvable Set Identifier value
 * @param[in] length        Length of advertising data
 * @param[in] p_data        Pointer to advertising data
 ****************************************************************************************
 */
typedef void (*gaf_scan_cb_report)(const gap_bdaddr_t* p_addr, uint8_t info_bf,
                                   const gaf_adv_report_air_info_t* p_air_info, uint8_t flags, uint16_t appearance,
                                   uint16_t tmap_roles, const atc_csis_rsi_t* p_rsi, uint16_t length,
                                   const uint8_t* p_data);

/**
 ****************************************************************************************
 * @brief Callback function called each time a General or a Targeted Announcement is received
 *
 * @param[in] p_addr        Pointer to Advertiser address
 * @param[in] type_bf       Type bit field (see #gaf_announcement_type_bf enumeration)
 * @param[in] context_bf    Context bit field (see #bap_context_type_bf enumeration)
 * @param[in] p_metadata    Pointer to Metadata structure
 ****************************************************************************************
 */
typedef void (*gaf_scan_cb_announcement)(const gap_bdaddr_t* p_addr, uint8_t type_bf, uint32_t context_bf,
                                         const gaf_ltv_t* p_metadata);

/*
 * CALLBACK SET DEFINITION
 ****************************************************************************************
 */

/// Set of callback functions for Generic Audio Framework Client module
struct gaf_scan_cb
{
    /// Callback function called when handling of a command has been completed
    gaf_scan_cb_cmp_evt cb_cmp_evt;
    /// Callback function called when scan has been stopped outside scope of #GAF_SCAN_CMD_TYPE_STOP command
    /// handling
    gaf_scan_cb_stopped cb_stopped;
    /// Callback function called each time a valid GAF advertising report is received
    gaf_scan_cb_report cb_report;
    /// Callback function called each time a General or a Targeted Announcement is received
    gaf_scan_cb_announcement cb_announcement;
};

/// @} GAF_SCAN_CALLBACK

/*
 * API FUNCTIONS DECLARATION
 ****************************************************************************************
 */

#if (GAF_SCAN)
/// @addtogroup GAF_SCAN_FUNCTION
/// @{

/**
 ****************************************************************************************
 * @brief Configure use of GAF Scanner module
 *
 * @param[in] p_cb          Pointer to set of callback functions for communication with upper layers
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gaf_scan_configure(const gaf_scan_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Update scan configuration used for all started scan procedures\n
 * Scan is started with a scan interval set to quick_intv_slot and scan window set to quick_wd_slot\n
 * If scan is still running 30 seconds after the start of this procedure, the scan parameters are updated
 * (intv_slot and wd_slot values are used) so that power consumption is reduced.
 *
 * @param[in] quick_intv_slot       Scan interval in multiple of 0.625ms for quicker connection setup\n
 *                                  Must be higher than 2.5ms\n
 *                                  Set to 60ms by default
 * @param[in] intv_slot             Scan interval in multiple of 0.625ms for reduced power scan\n
 *                                  Must be higher than 2.5ms\n
 *                                  Set to 1.28s by default
 * @param[in] quick_wd_slot         Scan window in multiple of 0.625ms for quicker connection setup\n
 *                                  Must be higher than 2.5ms and not larger than scan interval\n
 *                                  Set to 30ms by default
 * @param[in] wd_slot               Scan window in multiple of 0.625ms for reduced power scan\n
 *                                  Must be higher than 2.5ms and not larger than scan interval\n
 *                                  Set to 11.25ms by default
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gaf_scan_set_params(uint16_t quick_intv_slot, uint16_t intv_slot, uint16_t quick_wd_slot, uint16_t wd_slot);

/**
 ****************************************************************************************
 * @brief Start scan\n
 * #gaf_scan_cb_cmp_evt callback function called once procedure has been completed with #GAF_SCAN_CMD_TYPE_START
 * command type value
 * #gaf_scan_cb_report callback function called each time a valid GAF advertising report is received
 * #gaf_scan_cb_announcement callback function called each time an Audio Announcement is received if ASCS is
 * required if cfg_bf value
 * #gaf_scan_cb_stopped callback function called at end of timeout duration if duration is set to a non-zero value
 *
 * @param[in] cfg_bf        Configuration bit field (see #gaf_scan_cfg_bf enumeration)
 * @param[in] timeout_s     Timeout duration in seconds\n
 *                          0 means that the scan will last until #GAF_SCAN_STOP command is used
 * @param[in] phy           Indicate on which PHY scan has to be performed\n
 *                          LE 2M PHY is not allowed
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gaf_scan_start(uint8_t cfg_bf, uint8_t timeout_s, uint8_t phy);

/**
 ****************************************************************************************
 * @brief Stop scan\n
 * #gaf_scan_cb_cmp_evt callback function called once procedure has been completed with #GAF_SCAN_CMD_TYPE_STOP
 * command type value
 *
 * @return An error status (see #gaf_err enumeration)
 ****************************************************************************************
 */
uint16_t gaf_scan_stop(void);

/// @} GAF_SCAN_FUNCTION
#endif //(GAF_SCAN)

#endif // GAF_SCAN_H_
