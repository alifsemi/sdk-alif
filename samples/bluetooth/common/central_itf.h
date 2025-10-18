/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef CENTRAL_ITF_H_
#define CENTRAL_ITF_H_

/**
 * @file central_itf.h
 * @brief BLE Central Interface
 *
 * This module provides a high-level interface for BLE central
 * operations including scanning and connection management.
 *
 */

 /**
  * @brief Configure GAPM for central role
  * @return 0 on success, error code otherwise
  */
uint16_t central_itf_gapm_cfg(void);

/**
 * @brief Register peer device name for scan and directed connection.
 * @param p_name Device name to scan for
 * @return 0 on success, error code otherwise
 */
uint16_t central_itf_reg_peer_name(const char *p_name);

#endif /* CENTRAL_ITF_H_ */
