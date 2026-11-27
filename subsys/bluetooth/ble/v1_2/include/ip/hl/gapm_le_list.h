/**
 ****************************************************************************************
 *
 * @file gapm_le_list.h
 *
 * @brief Generic Access Profile Manager - List management
 *
 * Copyright (C) RivieraWaves 2009-2025
 * Release Identifier: 0e0cd311
 *
 ****************************************************************************************
 */

#ifndef GAPM_LE_LIST_H_
#define GAPM_LE_LIST_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm.h"
#include "gap_le.h"
#include "co_buf.h"
#include "co_bt_defines.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_LE_LIST_API List management
 * @ingroup GAPM_LE_API
 * @brief Filter Accept List, Resolving Address list, Periodic Advertising List
 ****************************************************************************************
 */

/// @addtogroup GAPM_ENUM_API
/// @{

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// List type values
enum gapm_le_list_type
{
    /// Filter Accept list
    GAPM_LE_LIST_TYPE_FAL = 0u,
    /// Resolving list
    GAPM_LE_LIST_TYPE_RAL,
    /// Periodic Advertiser list
    GAPM_LE_LIST_TYPE_PAL,
    /// Monitored Advertisers list
    GAPM_LE_LIST_TYPE_MAL,
};

/// @} GAPM_ENUM_API

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// @addtogroup GAPM_STRUCT_API
/// @{

/// Resolving list device information
/*@TRACE*/
typedef struct
{
    /// Device identity
    gap_bdaddr_t addr;
    /// Peer IRK
    uint8_t peer_irk[GAP_KEY_LEN];
    /// Local IRK
    uint8_t local_irk[GAP_KEY_LEN];
} gapm_le_list_ral_entry_t;

#if (BLE_PER_ADV)
/// Periodic advertising address information
/*@TRACE*/
typedef struct
{
    /// BD Address of device
    uint8_t addr[GAP_BD_ADDR_LEN];
    /// Address type of the device 0=public/1=private random (see #gap_addr_type enumeration)
    uint8_t addr_type;
    /// Advertising SID
    uint8_t adv_sid;
} gapm_le_per_adv_bdaddr_t;
#endif // (BLE_PER_ADV)

/// @} GAPM_STRUCT_API

/*
 * INTERFACES
 ****************************************************************************************
 */

/// @addtogroup GAPM_LE_LIST_API
/// @{

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo      Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status        Procedure execution status  (see #hl_err enumeration)
 * @param[in] size          Size of the list
 ***************************************************************************************
 */
typedef void (*gapm_le_list_size_cb)(uint32_t metainfo, uint16_t status, uint8_t size);

/// @} GAPM_LE_LIST_API

/// @addtogroup GAPM_LE_SEC_API
/// @{

/**
 ***************************************************************************************
 * @brief Function executed when procedure get local or peer RPA execution is over.
 *
 * @param[in] metainfo        Metadata information provided by API user (see \glos{METAINFO})
 * @param[in] status          Procedure execution status  (see #hl_err enumeration)
 * @param[in] p_addr          Pointer to generated random address
 *                            (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_le_rpa_cb)(uint32_t metainfo, uint16_t status, const gap_addr_t* p_addr);

/// @} GAPM_LE_SEC_API

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPM_LE_LIST_API
/// @{

#if (HL_DEPRECATED_LIST)
/**
 ***************************************************************************************
 * @brief Fill filter accept list entries
 *
 * Filter Accept List is cleared before inserting new entries
 * Application should wait #gapm_proc_cmp_cb callback execution before starting a new procedure
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] size      Size of the array size
 * @param[in] p_array   Pointer filter accept list entries. Information must stay valid and available after function execution
 *                      and until procedure completes (global variable).
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_fill_fal(uint32_t metainfo, uint8_t size, const gap_bdaddr_t* p_array, gapm_proc_cmp_cb cmp_cb);

/**
 ***************************************************************************************
 * @brief Fill resolving address list entries
 *
 * Resolving list is cleared before inserting new entries
 * Application should wait #gapm_proc_cmp_cb callback execution before starting a new procedure
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] size      Size of the array size
 * @param[in] p_array   Pointer filter accept list entries. Information must stay valid and available after function execution
 *                      and until procedure completes (global variable).
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_fill_ral(uint32_t metainfo, uint8_t size, const gap_le_ral_info_t* p_array,
                          gapm_proc_cmp_cb cmp_cb);

#if (BLE_PER_ADV)
/**
 ***************************************************************************************
 * @brief Fill periodic advertising list entries
 *
 * Resolving list is cleared before inserting new entries
 * Application should wait #gapm_proc_cmp_cb callback execution before starting a new procedure
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] size      Size of the array size
 * @param[in] p_array   Pointer filter accept list entries. Information must stay valid and available after function execution
 *                      and until procedure completes (global variable).
 * @param[in] cmp_cb    Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_fill_pal(uint32_t metainfo, uint8_t size, const gapm_le_per_adv_bdaddr_t* p_array,
                          gapm_proc_cmp_cb cmp_cb);

#endif // (BLE_PER_ADV)
#endif // (HL_DEPRECATED_LIST)

/**
 ***************************************************************************************
 * @brief Add device in Filter Accept list
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_entry       Pointer to device BD address
 * @param[in] cb_cmp_evt    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration)
 ***************************************************************************************
 */
uint16_t gapm_le_list_add_fal(uint32_t metainfo, const gap_bdaddr_t* p_entry, gapm_proc_cmp_cb cb_cmp_evt);

/**
 ***************************************************************************************
 * @brief Remove device from Filter Accept list
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_entry       Pointer to device BD address
 * @param[in] cb_cmp_evt    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration)
 ***************************************************************************************
 */
uint16_t gapm_le_list_remove_fal(uint32_t metainfo, const gap_bdaddr_t* p_entry, gapm_proc_cmp_cb cb_cmp_evt);

/**
 ***************************************************************************************
 * @brief Add device in Resolving list.\n
 * Default privacy mode for added peer device is Network. Device Privacy Mode can be selected using
 * #gapm_le_list_set_privacy_mode function.
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_entry       Pointer to peer device information
 * @param[in] cb_cmp_evt    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration)
 ***************************************************************************************
 */
uint16_t gapm_le_list_add_ral(uint32_t metainfo, const gapm_le_list_ral_entry_t* p_entry, gapm_proc_cmp_cb cb_cmp_evt);

/**
 ***************************************************************************************
 * @brief Remove device from Resolving list
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_entry       Pointer to peer device BD address
 * @param[in] cb_cmp_evt    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration)
 ***************************************************************************************
 */
uint16_t gapm_le_list_remove_ral(uint32_t metainfo, const gap_bdaddr_t* p_entry, gapm_proc_cmp_cb cb_cmp_evt);

#if (BLE_PER_ADV)
/**
 ***************************************************************************************
 * @brief Add device in Periodic Advertiser list
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_entry       Pointer to periodic advertiser information
 * @param[in] cb_cmp_evt    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration)
 ***************************************************************************************
 */
uint16_t gapm_le_list_add_pal(uint32_t metainfo, const gapm_le_per_adv_bdaddr_t* p_entry, gapm_proc_cmp_cb cb_cmp_evt);

/**
 ***************************************************************************************
 * @brief Remove device from Periodic Advertiser list
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_entry       Pointer to periodic advertiser information
 * @param[in] cb_cmp_evt    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration)
 ***************************************************************************************
 */
uint16_t gapm_le_list_remove_pal(uint32_t metainfo, const gapm_le_per_adv_bdaddr_t* p_entry,
                                 gapm_proc_cmp_cb cb_cmp_evt);
#endif // (BLE_PER_ADV)


/**
 **************************************************************************************
 * @brief Clear content of either Filter Accept list or Resolving list or Periodic Advertiser list
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] type          List type (see #gapm_le_list_type enumeration)
 * @param[in] cb_cmp_evt    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration)
 ***************************************************************************************
 */
uint16_t gapm_le_list_clear(uint32_t metainfo, uint8_t type, gapm_proc_cmp_cb cb_cmp_evt);

/**
 ***************************************************************************************
 * @brief Specify privacy mode (Network or Device) to be used for a given entry in the resolving list.
 *
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] p_peer_identity   Pointer to peer identity
 * @param[in] privacy_mode      Privacy mode (see #gap_le_privacy_mode enumeration)
 * @param[in] cb_cmp_evt        Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration)
 ***************************************************************************************
 */
uint16_t gapm_le_list_set_privacy_mode(uint32_t metainfo, const gap_bdaddr_t* p_peer_identity, uint8_t privacy_mode,
                                       gapm_proc_cmp_cb cb_cmp_evt);

/**
 ***************************************************************************************
 * @brief Retrieve local resolvable private address generated by controller for a specific peer identity
 *
 * Application should wait #gapm_le_rpa_cb callback execution before starting a new procedure
 *
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO}).
 * @param[in] p_peer_identity   Pointer to peer identity address.
 * @param[in] res_cb            Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_rpa_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_ral_local_rpa(uint32_t metainfo, const gap_bdaddr_t* p_peer_identity, gapm_le_rpa_cb res_cb);

/**
 ***************************************************************************************
 * @brief Retrieve peer resolvable private address detected by controller for a specific peer identity
 *
 * Application should wait #gapm_le_rpa_cb callback execution before starting a new procedure
 *
 * @param[in] metainfo          Metadata information that will be returned in procedure callback functions (see \glos{METAINFO}).
 * @param[in] p_peer_identity   Pointer to peer identity address.
 * @param[in] res_cb            Function called when procedure is over.
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_rpa_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_ral_peer_rpa(uint32_t metainfo, const gap_bdaddr_t* p_peer_identity, gapm_le_rpa_cb res_cb);

/**
 ***************************************************************************************
 * @brief Get filter accept list size. Information returned in res_cb function
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_list_size_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_fal_size(uint32_t metainfo, gapm_le_list_size_cb res_cb);

#if (BLE_PER_ADV)
/**
 ***************************************************************************************
 * @brief Get periodic advertising list size. Information returned in res_cb function
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_list_size_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_pal_size(uint32_t metainfo, gapm_le_list_size_cb res_cb);
#endif // (BLE_PER_ADV)

/**
 ***************************************************************************************
 * @brief Get resolving address list size. Information returned in res_cb function
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see #hl_err enumeration).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_le_list_size_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_le_get_ral_size(uint32_t metainfo, gapm_le_list_size_cb res_cb);

/// @} GAPM_LE_LIST_API

#endif // GAPM_LE_LIST_H_
