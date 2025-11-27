/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "alif_ble.h"
#include "batt_cli.h"
#include "central_itf.h"

static const char le_periph_device_name[]  = "ALIF_BATT_BLE";
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void)
{
	uint16_t err;

	/* init BLE */
	alif_ble_enable(NULL);

	/* Register peer device name to scan for */
	central_itf_reg_peer_name(le_periph_device_name);

	/* Configure device */
	err = central_itf_gapm_cfg();
	if (err) {
		return -1;
	}

	/* Add profile to BLE stack */
	add_profile_client();

	return 0;
}
