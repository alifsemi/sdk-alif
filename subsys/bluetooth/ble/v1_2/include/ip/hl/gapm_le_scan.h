/**
 ****************************************************************************************
 *
 * @file gapm_le_scan.h
 *
 * @brief Generic Access Profile Manager - Low Energy Scan Activities
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_LE_SCAN_H_
#define GAPM_LE_SCAN_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm_le.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_LE_SCAN_ACTV_API LE Scanning
 *
 * @ingroup GAPM_ACTV_API
 *
 * @brief Create and control LE Scanning activity.
 *
 * Even if application can create several scanning, only one can be active (started) at a time.
 *
 * The application must have a callback structure to handle activities events
 * \snippet{lineno} app_test_le_central.c APP_SCAN_ACTV_CB
 *
 * Application can create an scanning activity using #gapm_le_create_scan.
 *
 * example:
 * \snippet{lineno} app_test_le_central.c APP_CREATE_SCAN
 *
 * Once activity is created, application can immediately start scan.
 *
 * An application example is available in \ref app_test_le_central.c
 *
 * @note At least #GAP_ROLE_LE_OBSERVER role is required
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// Scanning Types
enum gapm_le_scan_type
{
    /// General discovery
    GAPM_SCAN_TYPE_GEN_DISC = 0u,
    /// Limited discovery
    GAPM_SCAN_TYPE_LIM_DISC,
    /// Observer
    GAPM_SCAN_TYPE_OBSERVER,
    /// Selective observer
    GAPM_SCAN_TYPE_SEL_OBSERVER,
    /// Connectable discovery
    GAPM_SCAN_TYPE_CONN_DISC,
    /// Selective connectable discovery
    GAPM_SCAN_TYPE_SEL_CONN_DISC,
};

/// Scanning properties bit field bit value
enum gapm_le_scan_prop
{
    /// See # GAPM_SCAN_PROP_PHY_1M_BIT
    GAPM_SCAN_PROP_PHY_1M_POS = 0u,
    /// Scan advertisement on the LE 1M PHY
    GAPM_SCAN_PROP_PHY_1M_BIT = CO_BIT(GAPM_SCAN_PROP_PHY_1M_POS),

    /// See #GAPM_SCAN_PROP_PHY_CODED_BIT
    GAPM_SCAN_PROP_PHY_CODED_POS = 1u,
    /// Scan advertisement on the LE Coded PHY
    GAPM_SCAN_PROP_PHY_CODED_BIT = CO_BIT(GAPM_SCAN_PROP_PHY_CODED_POS),

    /// See #GAPM_SCAN_PROP_ACTIVE_1M_BIT
    GAPM_SCAN_PROP_ACTIVE_1M_POS = 2u,
    /// Active scan on LE 1M PHY (Scan Request PDUs may be sent)
    GAPM_SCAN_PROP_ACTIVE_1M_BIT = CO_BIT(GAPM_SCAN_PROP_ACTIVE_1M_POS),

    /// See #GAPM_SCAN_PROP_ACTIVE_CODED_BIT
    GAPM_SCAN_PROP_ACTIVE_CODED_POS = 3u,
    /// Active scan on LE Coded PHY (Scan Request PDUs may be sent)
    GAPM_SCAN_PROP_ACTIVE_CODED_BIT = CO_BIT(GAPM_SCAN_PROP_ACTIVE_CODED_POS),

    /// See #GAPM_SCAN_PROP_ACCEPT_RPA_BIT
    GAPM_SCAN_PROP_ACCEPT_RPA_POS = 4u,
    /// Accept directed advertising packets if we use a RPA and target address cannot be solved by the
    /// controller
    GAPM_SCAN_PROP_ACCEPT_RPA_BIT = CO_BIT(GAPM_SCAN_PROP_ACCEPT_RPA_POS),

    /// See #GAPM_SCAN_PROP_FILT_TRUNC_BIT
    GAPM_SCAN_PROP_FILT_TRUNC_POS = 5u,
    /// Filter truncated advertising or scan response reports
    GAPM_SCAN_PROP_FILT_TRUNC_BIT = CO_BIT(GAPM_SCAN_PROP_FILT_TRUNC_POS),

};

/// Filtering policy for duplicated packets
enum gapm_le_scan_dup_filter_pol
{
    /// Disable filtering of duplicated packets
    GAPM_DUP_FILT_DIS = 0u,
    /// Enable filtering of duplicated packets
    GAPM_DUP_FILT_EN,
    /// Enable filtering of duplicated packets, reset for each scan period
    GAPM_DUP_FILT_EN_PERIOD,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Scanning parameters
/*@TRACE*/
typedef struct
{
    /// Type of scanning to be started (see #gapm_le_scan_type enumeration)
    uint8_t type;
    /// Properties for the scan procedure (see #gapm_le_scan_prop enumeration for bit signification)
    uint8_t prop;
    /// Duplicate packet filtering policy
    uint8_t dup_filt_pol;
    /// Reserved for future use
    uint8_t rsvd;
    /// Scan window opening parameters for LE 1M PHY
    gapm_le_scan_wd_op_param_t scan_param_1m;
    /// Scan window opening parameters for LE Coded PHY
    gapm_le_scan_wd_op_param_t scan_param_coded;
    /// Scan duration (in unit of 10ms). 0 means that the controller will scan continuously until
    /// reception of a stop command from the application
    uint16_t duration;
    /// Scan period (in unit of 1.28s). Time interval between two consequent starts of a scan duration
    /// by the controller. 0 means that the scan procedure is not periodic
    uint16_t period;
} gapm_le_scan_param_t;

/*
 * INTERFACES
 ****************************************************************************************
 */

/// Callback structure required to create a LE scan activity
typedef struct
{
    /// Inherits Activity callback interface
    gapm_le_cb_actv_t le_actv;

    /**
     ****************************************************************************************
     * Callback executed when a valid advertising report is received
     *
     * @note Mandatory callback. Shall be set to a valid callback
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] p_info        Pointer to advertising report information (also present in report metadata)
     * @param[in] p_report      Pointer to buffer that contains advertising data
     ****************************************************************************************
     */
    void (*report_received)(uint32_t metainfo, uint8_t actv_idx, const gapm_le_adv_report_info_t* p_info,
                            co_buf_t* p_report);
} gapm_le_scan_cb_actv_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#if (HL_LE_OBSERVER)
/**
 ****************************************************************************************
 * @brief Create an scan activity.
 *
 * @param[in]  metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  own_addr_type    Own address type (see #gapm_le_own_addr enumeration)
 * @param[in]  p_cbs            Activity Callback interface
 * @param[out] p_actv_idx       Pointer used to return allocated activity index
 *
 * @return Execution status (see #hl_err enumeration).
 ****************************************************************************************
 */
uint16_t gapm_le_create_scan(uint32_t metainfo, uint8_t own_addr_type, const gapm_le_scan_cb_actv_t* p_cbs,
                             uint8_t* p_actv_idx);

/**
 ****************************************************************************************
 * @brief Start scan activity.
 *
 * @param[in] actv_idx          Activity local index
 * @param[in] p_param           Scan parameters start parameters
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_le_start_scan(uint8_t actv_idx, const gapm_le_scan_param_t* p_param);

/**
 ***************************************************************************************
 * @brief Control flow of advertising reports. If disabled, reports are dropped without
 *        informing application.
 *
 * @param[in] enable   True to enable report, false to drop them
 ***************************************************************************************
 */
void gapm_le_control_adv_report_flow(bool enable);
#endif // (HL_LE_OBSERVER)

/// @} GAPM_LE_SCAN_ACTV_API

#endif /* GAPM_LE_SCAN_H_ */
