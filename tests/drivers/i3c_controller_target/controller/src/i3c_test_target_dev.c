/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/i3c.h>
#include <zephyr/kernel.h>

#define DT_DRV_COMPAT alif_i3c_test_target

#define I3C_TEST_TARGET_DEVICE(inst)                                                               \
	DEVICE_DT_INST_DEFINE(inst, NULL, NULL, NULL, NULL, POST_KERNEL,                           \
			      CONFIG_KERNEL_INIT_PRIORITY_DEVICE, NULL);

DT_INST_FOREACH_STATUS_OKAY(I3C_TEST_TARGET_DEVICE)
