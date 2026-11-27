/**
 ****************************************************************************************
 *
 * @file gapm_bt_inquiry.h
 *
 * @brief Generic Access Profile Manager - BT-Classic Inquiry Activities
 *
 * Copyright (C) RivieraWaves 2009-2022
 *
 ****************************************************************************************
 */

#ifndef GAPM_BT_INQUIRY_H_
#define GAPM_BT_INQUIRY_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm_bt.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_BT_INQUIRY_ACTV_API BT-Classic Inquiry
 *
 * @ingroup GAPM_ACTV_API
 *
 * @brief Create and control Inquiry activity.
 *
 * Application can control only on inquiry activity even if several can be created.
 *
 * Inquiry is used by a BT-Classic device to inform about its availability.
 *
 * The application must follow the #gapm_bt_inquiry_cb_actv_t callback interface to handle activities events:
 * \snippet{lineno} app_inquiry.c APP_INQ_ACTV_CB
 *
 *  Application can then create an inquiry activity using #gapm_bt_create_inquiry.
 *  Once activity is created it can be immediately started using #gapm_bt_start_inquiry.
 *
 * example:
 * \snippet{lineno} app_inquiry.c APP_START_INQ_ACTV
 *
 *
 * @note At least #GAP_ROLE_BT_CLASSIC role is required
 *
 * An application example is available in \ref app_test_bt_central.c
 *
 *
 * @{
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Inquiry parameters
/*@TRACE*/
typedef struct gapm_bt_inquiry_param
{
    /// Type of inquiry scan (see enum #gapm_bt_inquiry_type)
    uint8_t type;
    /// Inquiry scan duration (unit of 1.28 seconds, range [1, 48])
    uint8_t duration;
    /// Duration after inquiry to consider that EIR data reception fails (in baseband slot: 0.625ms unit)
    uint16_t eir_timeout;
    /// Number of response before stopping inquiry (0: unlimited).
    uint8_t num_rsp;
    /// True to enable periodic inquiry, false not periodic
    bool periodic;
    /// Minimum period duration (unit of 1.28 seconds, range [0x0002, 0xFFFE])
    uint16_t min_period_duration;
    /// Maximum period duration (unit of 1.28 seconds, range [0x0003, 0xFFFF])
    uint16_t max_period_duration;
} gapm_bt_inquiry_param_t;

/// Bluetooth classic inquiry report information
/*@TRACE*/
typedef struct gapm_bt_inquiry_report
{
    /// Class of device
    uint32_t class_of_device;
    /// Peer address
    gap_addr_t addr;
    /// RSSI  - 0x80 if invalid
    int8_t rssi;
    /// Page Scan Repetition Mode
    uint8_t page_scan_rep_mode;
    /// Clock Offset
    uint16_t clk_off;
} gapm_bt_inquiry_report_t;

/*
 * INTERFACES
 ****************************************************************************************
 */

/// Callback structure required to create an inquiry activity
typedef struct gapm_bt_inquiry_cb_actv
{
    /// Inherits Activity callback interface
    gapm_actv_cb_t actv;

    /**
     ****************************************************************************************
     * Callback executed when an inquiry report has been received
     *
     * @note Optional callback. Shall be set to a valid callback only for Name discovery procedure
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] p_report      Pointer to report information
     * @param[in] p_eir_data    Pointer to extend inquiry response data. NULL if nothing received
     ****************************************************************************************
     */
    void (*report_received)(uint32_t metainfo, uint8_t actv_idx, const gapm_bt_inquiry_report_t* p_report,
                            co_buf_t* p_eir_data);
} gapm_bt_inquiry_cb_actv_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Create Inquiry activity
 *
 * @param[in]  metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  p_cbs            Activity Callback interface
 * @param[out] p_actv_idx       Pointer used to return allocated activity index
 *
 * @return Execution status (see enum #hl_err).
 ****************************************************************************************
 */
uint16_t gapm_bt_create_inquiry(uint32_t metainfo, const gapm_bt_inquiry_cb_actv_t* p_cbs, uint8_t* p_actv_idx);

/**
 ****************************************************************************************
 * @brief Start Inquiry activity
 *
 * @param[in] actv_idx       Activity local index
 * @param[in] p_param        Pointer to inquiry parameters
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_bt_start_inquiry(uint8_t actv_idx, const gapm_bt_inquiry_param_t* p_param);

/// @} GAPM_BT_INQUIRY_ACTV_API

#endif /* GAPM_BT_INQUIRY_H_ */
