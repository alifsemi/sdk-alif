/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>

/* Prints cert IDs early in boot so they appear before application logs. */
static int cert_boot_print(void)
{
	printk("*************************************\n");
	printk("  Brand:   " CONFIG_CERT_BRAND_NAME "\n");
	printk("  Model:   " CONFIG_CERT_MODEL_NAME "\n");
	if (sizeof(CONFIG_CERT_FCC_ID) > 1) {
		printk("  FCC ID:  " CONFIG_CERT_FCC_ID "\n");
	}
	if (sizeof(CONFIG_CERT_ISED_ID) > 1) {
		printk("  IC ID:   " CONFIG_CERT_ISED_ID "\n");
	}
	if (sizeof(CONFIG_CERT_NCC_ID) > 1) {
		printk("  NCC:     " CONFIG_CERT_NCC_ID "\n");
	}
	if (sizeof(CONFIG_CERT_JP_ID) > 1) {
		printk("  JP:      " CONFIG_CERT_JP_ID "\n");
	}
	printk("*************************************\n");
	return 0;
}

SYS_INIT(cert_boot_print, PRE_KERNEL_2, 0);
