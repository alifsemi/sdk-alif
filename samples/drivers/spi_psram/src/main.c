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
#include <errno.h>

#if defined(CONFIG_SOC_SERIES_E7) || defined(CONFIG_SOC_SERIES_E5) || \
	defined(CONFIG_SOC_SERIES_E3) || defined(CONFIG_SOC_SERIES_E1)
#include <zephyr/arch/arm/mpu/arm_mpu.h>
#include <zephyr/cache.h>
#include <zephyr/sys/barrier.h>
#endif

#define PSRAM_NODE		DT_ALIAS(spi_psram)
#define OSPI_XIP_BASE		DT_PROP_BY_IDX(DT_PARENT(PSRAM_NODE), xip_base_address, 0)

#if defined(CONFIG_SOC_SERIES_E7) || defined(CONFIG_SOC_SERIES_E5) || \
	defined(CONFIG_SOC_SERIES_E3) || defined(CONFIG_SOC_SERIES_E1)
#define PSRAM_TEST_RUNTIME_MPU_ATTR	1
typedef uint16_t psram_data_t;
#else
#define PSRAM_TEST_RUNTIME_MPU_ATTR	0
#if defined(CONFIG_SOC_SERIES_B1)
typedef uint16_t psram_data_t;
#else
/**32b used in HexRAM */
typedef uint32_t psram_data_t;
#endif
#endif

static void print_test_config(uint32_t ram_size)
{
	printk("Test configuration:\n");
	printk("  XIP base        : 0x%x\n", (uint32_t)OSPI_XIP_BASE);
	printk("  RAM size        : %u bytes\n", ram_size);
	printk("  Access width    : %u-bit\n", sizeof(psram_data_t) * 8);
	printk("  Pattern         : incrementing %u-bit value, wraps at data width\n",
	       sizeof(psram_data_t) * 8);
#if PSRAM_TEST_RUNTIME_MPU_ATTR
	printk("  MPU test cases  : Device, Normal non-cacheable, SRAM cacheable\n");
#else
	printk("  MPU test cases  : Direct read/write, no runtime MPU changes\n");
#endif
}

static void print_test_case(const char *mode, const char *write_attr, const char *read_attr)
{
	printk("\nTest case: %s\n", mode);
	printk("  Write attribute : %s\n", write_attr);
	printk("  Read attribute  : %s\n", read_attr);
}

#if PSRAM_TEST_RUNTIME_MPU_ATTR
static int ospi_xip_set_mair_idx(uint8_t mair_idx, size_t cache_size)
{
	for (uint32_t region = 0; region < mpu_config.num_regions; region++) {
		const struct arm_mpu_region *mpu_region = &mpu_config.mpu_regions[region];

		if (mpu_region->base != OSPI_XIP_BASE) {
			continue;
		}

		unsigned int key = irq_lock();

		(void)sys_cache_data_flush_and_invd_range((void *)OSPI_XIP_BASE, cache_size);
		barrier_dsync_fence_full();
		ARM_MPU_SetRegion(region, OSPI_XIP_BASE | mpu_region->attr.rbar,
				  mpu_region->attr.r_limit |
					  (mair_idx << MPU_RLAR_AttrIndx_Pos) |
					  MPU_RLAR_EN_Msk);
		barrier_dsync_fence_full();
		barrier_isync_fence_full();
		irq_unlock(key);

		return 0;
	}

	return -ENOENT;
}
#endif

static uint32_t verify_pattern(const char *mode, volatile psram_data_t *ptr, uint32_t ram_size)
{
	uint32_t total_errors = 0;

	printk("Reading back in %s mode:\n", mode);

	for (uint32_t index = 0; index < (ram_size / sizeof(psram_data_t)); index++) {
		psram_data_t got = ptr[index];
		psram_data_t expected = (psram_data_t)index;

		if (got != expected) {
			printk("%s read error at addr %x, line_item %u, got %x, expected %x\n",
			       mode, (index * sizeof(psram_data_t)),
			       index & ((32 / sizeof(psram_data_t)) - 1),
			       got, expected);
			total_errors++;
			break;
		}
	}

	printk("%s total errors = %u\n", mode, total_errors);

	return total_errors;
}

int main(void)
{
	const struct device *psram_dev = DEVICE_DT_GET(PSRAM_NODE);

	volatile psram_data_t *const ptr = (volatile psram_data_t *)OSPI_XIP_BASE;
	uint32_t ram_size;

	ram_size = DT_PROP(PSRAM_NODE, size);

	if (!device_is_ready(psram_dev)) {
		printk("%s: device not ready.\n", psram_dev->name);
		return -1;
	}

	printk("\nPSRAM XIP mode demo app started\n");
	print_test_config(ram_size);

#if PSRAM_TEST_RUNTIME_MPU_ATTR
	int ret;

	print_test_case("Device", "Device", "Device");

	ret = ospi_xip_set_mair_idx(MPU_MAIR_INDEX_DEVICE, ram_size);
	if (ret != 0) {
		printk("Failed to set OSPI XIP Device MPU attribute: %d\n", ret);
		return ret;
	}

	printk("Writing data to the XIP region:\n");

	for (uint32_t index = 0; index < (ram_size/sizeof(psram_data_t)); index++) {
		ptr[index] = (psram_data_t)index;
	}
	barrier_dsync_fence_full();

	(void)verify_pattern("Device", ptr, ram_size);

	print_test_case("Normal non-cacheable", "Device", "Normal non-cacheable");

	ret = ospi_xip_set_mair_idx(MPU_MAIR_INDEX_SRAM_NOCACHE, ram_size);
	if (ret != 0) {
		printk("Failed to set OSPI XIP Normal non-cacheable MPU attribute: %d\n", ret);
		return ret;
	}

	(void)verify_pattern("Normal non-cacheable", ptr, ram_size);

	print_test_case("SRAM cacheable", "Device", "SRAM cacheable");

	ret = ospi_xip_set_mair_idx(MPU_MAIR_INDEX_SRAM, ram_size);
	if (ret != 0) {
		printk("Failed to set OSPI XIP SRAM cacheable MPU attribute: %d\n", ret);
		return ret;
	}

	(void)verify_pattern("SRAM cacheable", ptr, ram_size);
#else
	print_test_case("Direct", "Default MPU/static attribute", "Default MPU/static attribute");

	printk("Writing data to the XIP region:\n");

	for (uint32_t index = 0; index < (ram_size/sizeof(psram_data_t)); index++) {
		ptr[index] = (psram_data_t)index;
	}

	(void)verify_pattern("Default", ptr, ram_size);
#endif

	printk("Done\n");

	return 0;
}
