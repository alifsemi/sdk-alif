/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

 /*
  * This file handles the discovery and reading of a battery service.
  */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "batt_cli.h"
#include "basc.h"
#include "prf.h"

LOG_MODULE_REGISTER(batt_cli, LOG_LEVEL_DBG);

/**
 * @brief Handles battery service discovery state machine
 *
 * @param event The discovery event to process
 * @param conidx Connection index
 * @param status Status code from the operation
 *
 * This function implements a state machine for discovering and configuring
 * the Battery Service on a connected BLE peripheral device.
 */
static void service_discovery(uint8_t event, uint8_t conidx, uint16_t status);



static void on_cb_enable_cmp(uint8_t conidx, uint16_t status, uint16_t cmd_code,
				uint8_t instance_idx, uint8_t char_type)
{
	switch (cmd_code) {
	case BASC_CMD_DISCOVER:
		if (status != GAP_ERR_NO_ERROR) {
			LOG_ERR("Failed to discover Battery Service: 0x%02x", status);
			service_discovery(EVENT_SERVICE_DISCOVERY_START, conidx, GAP_ERR_NO_ERROR);
			break;
		}

		LOG_INF("Battery service discovered");

		/* Read Battery Level */
		LOG_INF("Read Battery Level");
		service_discovery(EVENT_SERVICE_CONTENT_DISCOVERED, conidx, GAP_ERR_NO_ERROR);

		/* Enable notifications*/
		service_discovery(EVENT_LEVEL_READ, conidx, GAP_ERR_NO_ERROR);
		break;

	case BASC_CMD_GET:
		if (status) {
			LOG_ERR("Individual battery level read status failed with error 0x%02x: ",
				status);
		}
		break;

	case BASC_CMD_SET_CCCD:
		LOG_DBG("Notifications enabled");
		service_discovery(EVENT_SENDING_EVENTS_ENABLED, conidx, status);
	default:
		break;
	}
}

static void on_cb_bond_data(uint8_t conidx, uint8_t nb_instances, const basc_content_t *p_bond_data)
{
}

static void on_cb_value(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, co_buf_t *p_buf)
{
	uint8_t level;

	level = *co_buf_data(p_buf);
	LOG_DBG("Battery level: %d", level);
}

static const basc_cbs_t cbs_basc = {
	.cb_cmp_evt = on_cb_enable_cmp,
	.cb_bond_data = on_cb_bond_data,
	.cb_value = on_cb_value,
};

uint16_t add_profile_client(void)
{
	uint8_t err;

	err = prf_add_profile(TASK_ID_BASC, 0, 0, NULL, &cbs_basc, NULL);

	if (err) {
		LOG_ERR("error adding profile 0x%02X", err);
		return err;
	}

	return GAP_ERR_NO_ERROR;
}

static void service_discovery(uint8_t event, uint8_t conidx, uint16_t err)
{
	if (err == GAP_ERR_NO_ERROR) {
		switch (event) {
		case EVENT_SERVICE_DISCOVERY_START:
			basc_discover(conidx);
			break;

		case EVENT_SERVICE_CONTENT_DISCOVERED:
			err = basc_get(conidx, 0, BASC_CHAR_TYPE_LEVEL);
			if (err) {
				LOG_ERR("Error reading level 0x%02x", err);
			}
			break;

		case EVENT_LEVEL_READ:
			co_buf_t *p_buf;

			if (prf_buf_alloc(&p_buf, PRF_CCC_DESC_LEN) == GAP_ERR_NO_ERROR) {
				co_write16(co_buf_data(p_buf), co_htole16(PRF_CLI_START_NTF));
				err = basc_set_cccd(conidx, 0u, BASC_CHAR_TYPE_LEVEL, p_buf);
				co_buf_release(p_buf);
				if (err) {
					LOG_ERR("Error starting notifications 0x%02x", err);
				}
			}
			break;

		case EVENT_SENDING_EVENTS_ENABLED:
			break;

		default:
			break;
		}
	} else {
		LOG_ERR("service discovery process error 0x%02x\n", err);
	}
}

void battery_client_process(uint8_t conidx, uint8_t event)
{
	service_discovery(event, conidx, GAP_ERR_NO_ERROR);
}
