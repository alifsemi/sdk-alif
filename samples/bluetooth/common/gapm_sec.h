/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef GAPM_SEC_H
#define GAPM_SEC_H
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include <zephyr/settings/settings.h>

#define BLE_BOND_KEYS_KEY_0  "ble/bond_keys_0"
#define BLE_BOND_KEYS_NAME_0 "bond_keys_0"
#define BLE_BOND_DATA_KEY_0  "ble/bond_data_0"
#define BLE_BOND_DATA_NAME_0 "bond_data_0"

typedef void (*pairing_status_cb)(uint16_t status, uint8_t con_idx, bool known_peer);

/* Init GAPM security module */
const gapc_security_cb_t *gapm_sec_init(bool security, pairing_status_cb pairing_cb,
					const gap_sec_key_t *irk);
/* Handle Connection confirmation and pairing */
void gapm_connection_confirm(uint8_t conidx, uint32_t metainfo, const gap_bdaddr_t *p_peer_addr);

#endif /* GAPM_SEC_H */
