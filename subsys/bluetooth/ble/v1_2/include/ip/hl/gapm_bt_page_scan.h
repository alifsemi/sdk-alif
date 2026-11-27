/**
 ****************************************************************************************
 *
 * @file gapm_bt_page_scan.h
 *
 * @brief Generic Access Profile Manager - BT-Classic Page Scan Activities
 *
 * Copyright (C) RivieraWaves 2009-2022
 *
 ****************************************************************************************
 */

#ifndef GAPM_BT_PAGE_SCAN_H_
#define GAPM_BT_PAGE_SCAN_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm_bt.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_BT_PAGE_SCAN_ACTV_API  BT-Classic Page Scan
 *
 * @ingroup GAPM_ACTV_API
 *
 * @brief  Create and control Page scan activity.
 *
 * Application can control only on page scan activity even if several can be created.
 *
 * Page Scan is used by a BT-Classic device to handle connection establishment initiated by a device in Page mode.
 *
 * The application must follow the #gapm_bt_page_scan_cb_actv_t callback interface to handle activities events:
 * \snippet{lineno} app_page_scan.c APP_PAGE_SCAN_ACTV_CB
 *
 * The application must also follow the #gapc_connection_req_cb_t callback interface to handle connection creation:
 * \snippet{lineno} app_connection.c APP_BT_CON_CB
 *
 *  Application can then create an page scan activity using #gapm_bt_create_page_scan.
 *  Once activity is created it can be immediately started using #gapm_bt_start_page_scan.
 *  When connection is established, page scan activity is automatically stopped and can be reused for a new
 *  connection establishment
 *
 * example:
 * \snippet{lineno} app_page_scan.c APP_START_PAGE_SCAN_ACTV
 *
 * @note At least #GAP_ROLE_BT_CLASSIC role is required
 *
 * An application example is available in \ref app_test_bt_periph.c
 *
 * @{
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Page scan parameters
/*@TRACE*/
typedef struct gapm_bt_page_scan_param
{
    /// Inquiry scan duration in seconds (0 means no timeout)
    uint16_t duration_s;
    /// True to use interlaced scan, False to use standard scan
    bool interlaced;
    /// Inquiry scan interval (in slots, range [0x12 : 0x1000], 0 use default)
    uint16_t interval;
    /// Inquiry scan window (in slots, range [0x11 : 0x1000], 0 use default)
    uint16_t window;
    /// Try to be master of link during connection establishment
    bool try_role_switch;
} gapm_bt_page_scan_param_t;

/*
 * INTERFACES
 ****************************************************************************************
 */

/// Callback structure required to create an page scan activity
typedef struct gapm_bt_page_scan_cb_actv
{
    /// Inherits Activity callback interface
    gapm_actv_cb_t hdr;

    /**
     ****************************************************************************************
     * Callback executed when a BT classic connection try to be established.
     * Check if upper SW authorize connection establishment.
     *
     * @note Optional callback, if NULL, connection is automatically authorized
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] p_addr        Pointer to peer initiator address
     *
     * @return True to accept the connection, false to reject it.
     ****************************************************************************************
     */
    bool (*connection_authorize)(uint32_t metainfo, uint8_t actv_idx, gap_addr_t* p_addr);
} gapm_bt_page_scan_cb_actv_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Create Page Scan activity
 *
 * @param[in]  metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  p_cbs            Activity Callback interface
 * @param[out] p_actv_idx       Pointer used to return allocated activity index
 *
 * @return Execution status (see enum #hl_err).
 ****************************************************************************************
 */
uint16_t gapm_bt_create_page_scan(uint32_t metainfo, const gapm_bt_page_scan_cb_actv_t* p_cbs, uint8_t* p_actv_idx);

/**
 ****************************************************************************************
 * @brief Start Page Scan activity
 *
 * @param[in] actv_idx       Activity local index
 * @param[in] p_param        Pointer to scan parameters
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_bt_start_page_scan(uint8_t actv_idx, const gapm_bt_page_scan_param_t* p_param);

/// @} GAPM_BT_PAGE_SCAN_ACTV_API

#endif /* GAPM_BT_PAGE_SCAN_H_ */
