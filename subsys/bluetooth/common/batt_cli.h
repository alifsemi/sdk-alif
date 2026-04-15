/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef BATT_CLI_H
#define BATT_CLI_H

#include <stdint.h>

/* Event values for Service Discovery state machine */
enum batt_client_discovery_events {
	/* Request to start */
	EVENT_SERVICE_DISCOVERY_START = 0U,
	/* Discovery of Battery Service instances has been completed */
	EVENT_SERVICE_CONTENT_DISCOVERED,
	/* Battery Level value has been read */
	EVENT_LEVEL_READ,
	/* Sending of notifications for Battery Level characteristics has been enabled */
	EVENT_SENDING_EVENTS_ENABLED,
};

/**
 * @brief Add battery client profile to BLE stack
 *
 * @return GAP_ERR_NO_ERROR (0x00) on success, error code otherwise
 */
uint16_t add_profile_client(void);

/**
 * @brief Process battery client events and initiate service discovery
 *
 * @param conidx Connection index for the BLE connection
 * @param event Event type to process
 */
void battery_client_process(uint8_t conidx, uint8_t event);

#endif /* BATT_CLI_H */
