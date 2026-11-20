/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "bluetooth.h"
#include "KwsMain.h"

LOG_MODULE_REGISTER(main, CONFIG_MAIN_LOG_LEVEL);

int main(void)
{
	configure_bluetooth();
	kws_initialize();

	return 0;
}
