/**
 ****************************************************************************************
 *
 * @file gapm_bt_page.h
 *
 * @brief Generic Access Profile Manager - BT-Classic Page Activities
 *
 * Copyright (C) RivieraWaves 2009-2022
 *
 ****************************************************************************************
 */

#ifndef GAPM_BT_PAGE_H_
#define GAPM_BT_PAGE_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm_bt.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_BT_PAGE_ACTV_API  BT-Classic Page
 *
 * @ingroup GAPM_ACTV_API
 *
 * @brief Create and control Page activity.
 *
 * Application can control only on page activity even if several can be created.
 *
 * Page is used by a BT-Classic device to initiate a connection establishment.
 *
 * The application must follow the #gapm_bt_page_cb_actv_t callback interface to handle activities events:
 * \snippet{lineno} app_page.c APP_PAGE_ACTV_CB
 *
 * The application must also follow the #gapc_connection_req_cb_t callback interface to handle connection creation:
 * \snippet{lineno} app_connection.c APP_BT_CON_CB
 *
 * Application can then create an page activity using #gapm_bt_create_page.
 * Once activity is created it can be immediately started using either:
 *  - #gapm_bt_start_direct_connection
 *  - #gapm_bt_start_name_discovery
 *
 * When connection is established, the activity is automatically stopped.
 *
 * example:
 * \snippet{lineno} app_page.c APP_START_PAGE_ACTV
 *
 *
 * @note At least #GAP_ROLE_BT_CLASSIC role is required
 *
 * An application example is available in \ref app_test_bt_central.c
 *
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// Automatic packet type selection
#define GAPM_BT_PAGE_AUTO_PACKET_TYPE_SELECTION             (0xFFFF)

/// Page scan repetition mode
enum gapm_bt_page_scan_repetition_mode
{
    GAPM_SCAN_R0 = 0,
    GAPM_SCAN_R1,
    GAPM_SCAN_R2,
};


/// Type of paging activity to start
enum gapm_bt_page_type
{
    /// Connection establishement
    GAPM_BT_PAGE_DIRECT_CONNECTION = 0,
    /// Remote name discovery
    GAPM_BT_PAGE_NAME_DISCOVERY,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Page parameters
/*@TRACE*/
typedef struct gapm_bt_page_param
{
    /// Address of the device to connect
    gap_addr_t peer_addr;
    /// Page timeout to consider connection establishment failed (in baseband slot: 0.625ms unit)
    /// if zero, keep default page timeout
    uint16_t page_timeout;
    /// Bit field of supported and unsupported packet types (see enum #gapm_bt_packet_type_bf)
    /// For an automatic selection use #GAPM_BT_PAGE_AUTO_PACKET_TYPE_SELECTION (0xFFFF)
    uint16_t packet_type_bf;
    /// Peer device clock offset - shall be set to zero if unknown
    uint16_t clock_offset;
    /// Page Scan repetition mode (see enum #gapm_bt_page_scan_repetition_mode)
    uint8_t page_scan_repetition;
    /// True to allow peer device to becomes master of the connection, False to stay master.
    bool allow_role_switch;
} gapm_bt_page_param_t;

/*
 * INTERFACES
 ****************************************************************************************
 */

/// Callback structure required to create a connectable activity
typedef struct gapm_bt_page_cb_actv
{
    /// Inherits Activity callback interface
    gapm_actv_cb_t hdr;

    /**
     ****************************************************************************************
     * Callback executed when remote device name is retrieved.
     *
     * @note Optional callback, if NULL, BT-Classic name discovery is not supported.
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] p_addr        Pointer to remote device address
     * @param[in] length        Length of remote device name
     * @param[in] p_name        Remote device name (UTF-8 string)
     ****************************************************************************************
     */
    void (*remote_name)(uint32_t metainfo, uint8_t actv_idx, const gap_addr_t* p_addr, uint8_t length, const char* p_name);

    /**
     ****************************************************************************************
     * Callback executed when remote features are retrieved.
     *
     * @note Optional callback
     *
     * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
     * @param[in] actv_idx      Activity Local identifier
     * @param[in] p_addr        Pointer to remote device address
     * @param[in] p_features    Pointer to array that contains remote features (8 bytes)
     ****************************************************************************************
     */
    void (*remote_features)(uint32_t metainfo, uint8_t actv_idx, const gap_addr_t* p_addr, const uint8_t* p_features);
} gapm_bt_page_cb_actv_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Create Page activity - Create a Master BT Classic connection
 *
 * @param[in]  metainfo         Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in]  p_cbs            Activity Callback interface
 * @param[out] p_actv_idx       Pointer used to return allocated activity index
 *
 * @return Execution status (see enum #hl_err).
 ****************************************************************************************
 */
uint16_t gapm_bt_create_page(uint32_t metainfo, const gapm_bt_page_cb_actv_t* p_cbs, uint8_t* p_actv_idx);

/**
 ****************************************************************************************
 * @brief Start Page activity
 *
 * @param[in] actv_idx       Activity local index
 * @param[in] type           Type of paging activity to start (see #gapm_bt_page_type)
 * @param[in] p_param        Pointer to scan parameters
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
uint16_t gapm_bt_start_page(uint8_t actv_idx, uint8_t type, const gapm_bt_page_param_t* p_param);


/**
 ****************************************************************************************
 * @brief Start direct connection extablishment part of page activity
 *
 * @param[in] actv_idx       Activity local index
 * @param[in] p_param        Pointer to page parameters
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t gapm_bt_start_direct_connection(uint8_t actv_idx, const gapm_bt_page_param_t* p_param)
{
    return gapm_bt_start_page(actv_idx, GAPM_BT_PAGE_DIRECT_CONNECTION, p_param);
}

/**
 ****************************************************************************************
 * @brief Start remote name discovery part of page activity
 *
 * @param[in] actv_idx       Activity local index
 * @param[in] p_param        Pointer to page parameters
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_actv_cb_t.proc_cmp callback execution
 ****************************************************************************************
 */
__STATIC_FORCEINLINE uint16_t gapm_bt_start_name_discovery(uint8_t actv_idx, const gapm_bt_page_param_t* p_param)
{
    return gapm_bt_start_page(actv_idx, GAPM_BT_PAGE_NAME_DISCOVERY, p_param);
}

/// @} GAPM_BT_PAGE_ACTV_API
///
#endif /* GAPM_BT_PAGE_H_ */
