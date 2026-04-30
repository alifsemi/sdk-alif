/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _APP_GAF_SCAN_H
#define _APP_GAF_SCAN_H

#include "csisc.h"
#include "atc_csi.h"
/**
 * @brief Callback function type for scanning ready event.
 */
typedef void (*scanning_ready_callback_t)(void);

/**
 * @brief Callback function type for when a peer is found during scanning.
 *
 * @param[in] p_addr Pointer to the Bluetooth address of the found peer
 * @param[in] p_rsi Pointer to the RSI of the found peer
 * @return 0 on success
 */
typedef int (*peer_found_cb_t)(gap_bdaddr_t const *const p_addr, atc_csis_rsi_t const *const p_rsi);

/**
 * @brief Configure scanning for LE audio devices
 *
 * @return 0 on success
 */
int le_gaf_scan_configure(peer_found_cb_t peer_found_cb);

/**
 * @brief Start scanning for LE audio devices
 *
 * @return 0 on success
 */
int le_gaf_scan_start(scanning_ready_callback_t ready_cb, uint8_t req_bits);

/**
 * @brief Stop scanning for LE audio devices
 *
 * @return 0 on success
 */
int le_gaf_scan_stop(void);

#endif /* _APP_GAF_SCAN_H */
