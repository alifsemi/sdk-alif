/**
 ****************************************************************************************
 *
 * @file gapm_bt_inquiry_scan.h
 *
 * @brief Generic Access Profile Manager - BT-Classic Inquiry Scan Activities
 *
 * Copyright (C) RivieraWaves 2009-2022
 *
 ****************************************************************************************
 */

#ifndef GAPM_BT_INQUIRY_SCAN_H_
#define GAPM_BT_INQUIRY_SCAN_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm_bt.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_BT_INQUIRY_SCAN_ACTV_API BT-Classic Inquiry Scan
 *
 * @ingroup GAPM_ACTV_API
 *
 * @brief  Create and control Inquiry scan activity.
 *
 * Application can control only on inquiry scan activity even if several can be created.
 *
 * Inquiry Scan is used by a BT-Classic device to be discovered by other BT Classic devices.
 *
 * The application must follow the #gapm_bt_inquiry_scan_cb_actv_t callback interface to handle activities events:
 * \snippet{lineno} app_inquiry_scan.c APP_INQ_SCAN_ACTV_CB
 *
 *  Application can then create an inquiry scan activity using #gapm_bt_create_inquiry_scan.
 *  Once activity is created it can be immediately started using #gapm_bt_start_inquiry_scan.
 *
 * example:
 * \snippet{lineno} app_inquiry_scan.c APP_START_INQ_SCAN_ACTV
 *
 *
 * @note At least #GAP_ROLE_BT_CLASSIC role is required
 *
 * An application example is available in \ref app_test_bt_periph.c
 *
 * @{
 ****************************************************************************************
 */



/// Length of buffer header length required by Inquiry Scan activity
#define GAPM_BT_INQUIRY_SCAN_BUFFER_HEADER_LEN (0)
/// Length of buffer tail length required by Inquiry Scan activity
#define GAPM_BT_INQUIRY_SCAN_BUFFER_TAIL_LEN   (0)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Inquiry scan parameters
/*@TRACE*/
typedef struct gapm_bt_inquiry_scan_param
{
    /// Type of inquiry scan (see enum #gapm_bt_inquiry_type)
    uint8_t type;
    /// True to use interlaced scan, False to use standard scan
    bool interlaced;
    /// Inquiry scan interval (in slots, range [0x12 : 0x1000], 0 use default)
    uint16_t interval;
    /// Inquiry scan window (in slots, range [0x11 : 0x1000], 0 use default)
    uint16_t window;
    /// Inquiry scan duration in seconds (0 means no timeout)
    uint16_t duration_s;
} gapm_bt_inquiry_scan_param_t;

/*
 * INTERFACES
 ****************************************************************************************
 */

/// Callback structure required to create an Inquiry Scan activity
typedef gapm_actv_cb_t gapm_bt_inquiry_scan_cb_actv_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Create Inquiry Scan activity
 *
 * @param[in]  metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  p_cbs            Activity Callback interface
 * @param[out] p_actv_idx       Pointer used to return allocated activity index
 *
 * @return Execution status (see enum #hl_err).
 ****************************************************************************************
 */
uint16_t gapm_bt_create_inquiry_scan(uint32_t metainfo, const gapm_bt_inquiry_scan_cb_actv_t* p_cbs,
                                     uint8_t* p_actv_idx);

/**
 ****************************************************************************************
 * @brief Start Inquiry Scan activity
 *
 * @note Minimum buffer head: #GAPM_BT_INQUIRY_SCAN_BUFFER_HEADER_LEN, 
 *       and minimum buffer tail: #GAPM_BT_INQUIRY_SCAN_BUFFER_TAIL_LEN
 * 
 * @param[in] actv_idx       Activity local index
 * @param[in] p_param        Pointer to scan parameters
 * @param[in] p_eir_data     Pointer to Extended Inquiry Response data. NULL if no EIR data.
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_bt_start_inquiry_scan(uint8_t actv_idx, const gapm_bt_inquiry_scan_param_t* p_param,
                                    co_buf_t* p_eir_data);

/// @} GAPM_BT_INQUIRY_SCAN_ACTV_API

#endif /* GAPM_BT_INQUIRY_SCAN_H_ */
