/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _APP_CSISC_H
#define _APP_CSISC_H
#include "atc_csisc.h"
/**
 * @brief Callback function type for retrieving the LTK of a connected peer by its connection index
 *
 * @param conidx Connection index of the peer
 * @return Pointer to the LTK of the peer, or NULL if not available
 */
typedef void *(*peer_ltk_getter_t)(uint32_t const conidx);

/**
 * @brief Configure the CSISC module
 *
 * @param[in] peer_ltk_getter Function pointer to retrieve the LTK of a connected peer by its
 * connection index
 *
 * @return 0 on success
 */
uint16_t csisc_configure(peer_ltk_getter_t peer_ltk_getter);

/**
 * @brief Toggle CSISC lock state between locked and unlocked
 *
 * @param[in] conidx Connection index of the Set Member device
 * @param[in] set_lid Set LID of the Set Member device
 *
 * @return 0 on success
 */
uint16_t csisc_lock_toggle(uint8_t conidx, uint8_t set_lid);

/**
 * @brief Get a CSIS characteristic value from a connected Set Member device
 *
 * @param[in] conidx Connection index of the Set Member device
 * @param[in] set_lid Set LID of the Set Member device
 * @param[in] char_type Characteristic type to get (see enum csis_char_type)
 *
 * @return 0 on success
 */
uint16_t csisc_get_char(uint8_t conidx, uint8_t set_lid, uint8_t char_type);

/**
 * @brief Add a new CSIS device
 *
 * @param[in] p_addr Pointer to the Bluetooth address of the device
 * @param[in] p_rsi Pointer to the RSI of the device
 *
 * @return 0 on success
 */
uint16_t csisc_dev_add(gap_bdaddr_t const *const p_addr, atc_csis_rsi_t const *const p_rsi);

/**
 * @brief Handle a CSIS device disconnection
 *
 * @param[in] conidx Connection index of the disconnected device
 *
 * @return 0 on success
 */
uint16_t csisc_dev_disconnected(uint8_t conidx);

/**
 * @brief Process CSIS tasks
 */
void csisc_process(void);

#endif /* _APP_CSISC_H */
