/**
 ****************************************************************************************
 *
 * @file gapm_bt.h
 *
 * @brief Generic Access Profile Manager - BT-Classic Activities
 *
 * Copyright (C) RivieraWaves 2009-2022
 *
 ****************************************************************************************
 */

#ifndef GAPM_BT_H_
#define GAPM_BT_H_

#include "rom_build_cfg.h"

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "gapm.h"

/**
 ****************************************************************************************
 * @addtogroup GAPM_BT_API BT-Classic
 * @ingroup GAPM_API
 * @brief Set of functions and interfaces required to create and manage a BT-Classic activity.
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup GAPM_BT_SEC_API Security toolbox
 * @ingroup GAPM_BT_API
 * @brief OOB data
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// @addtogroup GAPM_ENUM_API
/// @{

/// Inquiry or Inquiry Scan type
enum gapm_bt_inquiry_type
{
    /// General discoverable or discovery mode
    GAPM_INQUIRY_GENERAL = 0,
    /// Limited discoverable or discovery mode
    GAPM_INQUIRY_LIMITED,
};

/// Packet type configuration bit field
enum gapm_bt_packet_type_bf
{
    /// 2-DH1 shall not be used.
    GAPM_PKT_NO_USE_2_DH1_POS = 1,
    GAPM_PKT_NO_USE_2_DH1_BIT = (1 << 1),
    /// 3-DH1 shall not be used.
    GAPM_PKT_NO_USE_3_DH1_POS = 2,
    GAPM_PKT_NO_USE_3_DH1_BIT = (1 << 2),
    /// DH1 may be used.
    GAPM_PKT_MAY_USE_DH1_POS = 4,
    GAPM_PKT_MAY_USE_DH1_BIT = (1 << 4),
    /// 2-DH3 shall not be used.
    GAPM_PKT_NO_USE_2_DH3_POS = 8,
    GAPM_PKT_NO_USE_2_DH3_BIT = (1 << 8),
    /// 3-DH3 shall not be used.
    GAPM_PKT_NO_USE_3_DH3_POS = 9,
    GAPM_PKT_NO_USE_3_DH3_BIT = (1 << 9),
    /// DM3 may be used.
    GAPM_PKT_MAY_USE_DM3_POS = 10,
    GAPM_PKT_MAY_USE_DM3_BIT = (1 << 10),
    /// DH3 may be used.
    GAPM_PKT_MAY_USE_DH3_POS = 11,
    GAPM_PKT_MAY_USE_DH3_BIT = (1 << 11),
    /// 2-DH5 shall not be used.
    GAPM_PKT_NO_USE_2_DH5_POS = 12,
    GAPM_PKT_NO_USE_2_DH5_BIT = (1 << 12),
    /// 3-DH5 shall not be used.
    GAPM_PKT_NO_USE_3_DH5_POS = 13,
    GAPM_PKT_NO_USE_3_DH5_BIT = (1 << 13),
    /// DM5 may be used.
    GAPM_PKT_MAY_USE_DM5_POS = 14,
    GAPM_PKT_MAY_USE_DM5_BIT = (1 << 14),
    /// DH5 may be used.
    GAPM_PKT_MAY_USE_DH5_POS = 15,
    GAPM_PKT_MAY_USE_DH5_BIT = (1 << 15),
};

/// @} GAPM_ENUM_API

/*
 * INTERFACES
 ****************************************************************************************
 */

/// @addtogroup GAPM_BT_SEC_API
/// @{

/**
 ***************************************************************************************
 * @brief Function executed when procedure execution is over.
 *
 * @param[in] metainfo        Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] status          Procedure execution status  (see enum #hl_err)
 * @param[in] p_oob_192       Pointer to generated P-192 OOB data (NULL if status != GAP_ERR_NO_ERROR)
 * @param[in] p_oob_256       Pointer to generated P-256 OOB data (NULL if status != GAP_ERR_NO_ERROR)
 ***************************************************************************************
 */
typedef void (*gapm_bt_oob_cb)(uint32_t metainfo, uint16_t status, const gap_oob_t* p_oob_192,
                               const gap_oob_t* p_oob_256);

/// @} GAPM_BT_SEC_API

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/// @addtogroup GAPM_CONFIG_API
/// @{

/**
 ****************************************************************************************
 * @brief Insert into SDP a device identification record.
 *
 * @param[in] vendor_id_source  Designates which organization assigned the vendor_id attribute
 * (see enum #gap_vendor_id_source)
 * @param[in] vendor_id         Uniquely identify the vendor of the device
 * @param[in] product_id        Distinguish between different products made by the vendor
 * @param[in] version           Numeric expression identifying the device release number in Binary-Coded Decimal
 *
 * @return Status of SDP record insertion (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t gapm_bt_set_sdp_identification_record(uint16_t vendor_id_source, uint16_t vendor_id, uint16_t product_id,
                                               uint16_t version);

/**
 ****************************************************************************************
 * @brief Set Rx Max Slot limit for all new connections (1 slot / 3 slot / 5 slot)
 *
 * @param[in] metainfo      Metadata information that will be returned in procedure callback function (see \glos{METAINFO})
 * @param[in] rx_max_slot  Maximum Rx Slot (in slots)
 * @param[in] cmp_cb        Function called when procedure is over.
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_proc_cmp_cb callback execution
 ****************************************************************************************
 */
uint16_t gapm_bt_set_rx_max_slot(uint32_t metainfo, uint8_t rx_max_slot, gapm_proc_cmp_cb cmp_cb);

/// @}

/// @addtogroup GAPM_PROC_API
/// @{

/**
 ****************************************************************************************
 * @brief Get next available service record handle - shall be use only by BT Classic profiles
 *
 * @return Next available service record handle
 ****************************************************************************************
 */
uint32_t gapm_bt_get_sdp_next_service_record_handle(void);

/// @} GAPM_PROC_API

/// @addtogroup GAPM_BT_SEC_API
/// @{

/**
 ***************************************************************************************
 * @brief Generate BT Classic OOB data using
 *
 *        OOB data must be conveyed to peer device through an out of band method.
 *
 * @param[in] metainfo  Metadata information that will be returned in procedure callback functions (see \glos{METAINFO})
 * @param[in] res_cb    Function called when procedure is over
 *
 * @return Execution status (see enum #hl_err).
 *         If returns GAP_ERR_NO_ERROR, upper layer SW shall wait for #gapm_bt_oob_cb callback execution
 ***************************************************************************************
 */
uint16_t gapm_bt_generate_oob_data(uint32_t metainfo, gapm_bt_oob_cb res_cb);

/// @} GAPM_BT_SEC_API

#endif /* GAPM_BT_H_ */
