/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef _APP_CONNECTION_H
#define _APP_CONNECTION_H

#include "csisc.h"
#include "atc_csi.h"
/**
 * @brief Connect to a peer device by its Bluetooth address
 *
 * @param[in] p_addr Pointer to the Bluetooth address of the peer device
 * @param[out] connidx Pointer to store the connection index of the established connection
 * @return 0 on success, error code otherwise
 */
int app_connect_device(gap_bdaddr_t const *const p_addr, uint8_t *connidx);

/**
 * @brief Disconnect from a connected peer device by its connection index
 *
 * @param[in] conidx Connection index of the peer device to disconnect from
 * @param[in] remove_bond Flag indicating whether to remove the bond information
 * @return 0 on success, error code otherwise
 */
int app_disconnect(uint8_t conidx, bool remove_bond);

#endif /* _APP_CONNECTION_H */
