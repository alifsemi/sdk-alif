/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * This file is meant to route the client process calling to the
 * corresponding implementation.
 */

#include "gatt.h"
#include "batt_cli.h"

void profile_client_process(uint16_t svc_id, uint8_t conidx, uint8_t event)
{
	switch (svc_id)	{
	case PRF_ID_BASC:
		battery_client_process(conidx, event);
		break;

	default:
		break;
	}
}
