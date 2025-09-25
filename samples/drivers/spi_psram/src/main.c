/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <stdio.h>
#include <stdint.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

int main(void)
{
	const struct device *psram_dev = DEVICE_DT_GET(DT_ALIAS(spi_psram));
	uint32_t *const ptr = (uint32_t *) DT_PROP_BY_IDX(DT_PARENT(DT_ALIAS(spi_psram)),
						xip_base_address, 0);
	uint32_t total_errors = 0, ram_size;

	ram_size = DT_PROP(DT_ALIAS(spi_psram), size);

	if (!device_is_ready(psram_dev)) {
		printk("%s: device not ready.\n", psram_dev->name);
		return -1;
	}

	printk("\nPSRAM XIP mode demo app started\n");

	printk("Writing data to the XIP region:\n");

	for (uint32_t index = 0; index < (ram_size/sizeof(uint32_t)); index++) {
		ptr[index] = index;
	}

	printk("Reading back:\n");

	for (uint32_t index = 0; index < (ram_size/sizeof(uint32_t)); index++) {
		if (ptr[index] != index) {
			printk("Data error at addr %x, got %x, expected %x\n",
			(index * sizeof(uint32_t)), ptr[index], index);
			total_errors++;
		}
	}

	printk("Done, total errors = %d\n", total_errors);

	return 0;
}
