/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
 * OSPI-flash -> OSPI-HyperRAM (PSRAM) copy loader for the Balletto B1
 * (alif_b1_dk .../rtss_he).
 *
 * The OSPI flash and the HyperRAM PSRAM share the single OSPI0 controller
 * (SCLK, D0..D7 and the RXDS strobe). They cannot both be connected to the
 * bus at once, so this loader copies an arbitrarily large payload by
 * *alternating* the bus between the two devices, one SRAM-sized chunk at a
 * time -- there is no requirement that the payload fit in SRAM.
 *
 * Per chunk:
 *
 *   FLASH phase:
 *     - leave PSRAM XiP mode                        (aes_disable_xip)
 *     - apply the flash pin state                   (flash CS muxed,
 *                                                    PSRAM CS parked on GPIO)
 *     - restore flash DDR-capture timing            (ddr-drive-edge / rx-ds)
 *     - flash_read() one chunk into an SRAM buffer  (Zephyr flash driver,
 *                                                    IRQ-backed, owns OSPI0
 *                                                    command mode)
 *   PSRAM phase:
 *     - apply the PSRAM pin state                   (PSRAM CS muxed,
 *                                                    flash CS parked on GPIO)
 *     - set PSRAM DDR-capture timing                (ddr-drive-edge / rx-ds)
 *     - reconfigure OSPI0 for octal-DDR HyperRAM    (prepare_transfer +
 *                                                    ospi_hyperbus_xip_init +
 *                                                    aes_enable_xip)
 *     - copy the chunk into the PSRAM XiP window    (memory-mapped stores)
 *
 * Because the *deselected* device's chip-select is parked on GPIO, only one
 * device is ever electrically connected to the shared RXDS strobe, which is
 * what makes the octal-DDR flash read reliable (a muxed idle HyperRAM CS
 * otherwise loads the strobe and the read hangs).
 *
 * The PSRAM XiP window (0xA0000000) is in the Cortex-M55 default Device
 * memory region (non-cacheable, ordered), so the memory-mapped writes reach
 * the controller directly; a single completion barrier per chunk is enough.
 *
 * On return from a successful copy the bus is left in PSRAM XiP mode with the
 * PSRAM pins applied, so the copied data is immediately readable (execute-in-
 * place) at PSRAM_XIP_BASE and flash_to_psram_verify() can read it back.
 *
 * NOTE: board bring-up code. The per-device DDR-capture timing values below
 * are the vendor snippet defaults and can be tuned on real hardware.
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/pinctrl.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/barrier.h>
#include <zephyr/sys/crc.h>
#include <string.h>

#include "flash_to_psram.h"

#define FLASH_NODE	DT_ALIAS(spi_flash0)
#define PSRAM_NODE	DT_ALIAS(spi_psram)

#if DT_NODE_EXISTS(FLASH_NODE) && DT_NODE_EXISTS(PSRAM_NODE)

#include "ospi_hal.h"
#include "ospi.h"

/* The OSPI0 controller node (parent of both the flash and the PSRAM child). */
#define OSPI0_NODE	DT_PARENT(PSRAM_NODE)

/* PSRAM XiP window base address (declared on the OSPI0 controller node). */
#define PSRAM_XIP_BASE	DT_PROP_BY_IDX(OSPI0_NODE, xip_base_address, 0)
#define PSRAM_XIP_SIZE	DT_PROP_BY_IDX(OSPI0_NODE, xip_base_address, 1)

/* Usable destination = min(physical HyperRAM size, controller XiP window). */
#define PSRAM_USABLE_SIZE \
	MIN((size_t)PSRAM_XIP_SIZE, (size_t)DT_PROP(PSRAM_NODE, size))

/*
 * Per-device DDR-capture timing. Flash values are the OSPI0 node defaults;
 * PSRAM values are the vendor `ospi-psram` snippet defaults. They are applied
 * to the shared controller registers around each phase. Override from the
 * build if a board needs different values.
 */
#ifndef FLASH_DDR_DRIVE_EDGE
#define FLASH_DDR_DRIVE_EDGE	1U
#endif
#ifndef FLASH_RX_DS_DELAY
#define FLASH_RX_DS_DELAY	11U
#endif
#ifndef PSRAM_DDR_DRIVE_EDGE
#define PSRAM_DDR_DRIVE_EDGE	0U
#endif
#ifndef PSRAM_RX_DS_DELAY
#define PSRAM_RX_DS_DELAY	8U
#endif

/* PSRAM chip-select (SS0_A). */
#define PSRAM_CS_PIN		0U

/* HyperRAM R/W transactions are octal-DDR commands with 16-bit frames.
 * Note! Must be a positive multiple of 2
 */
#define PSRAM_OSPI_DFS		16U

/* SRAM staging buffer size per alternating chunk (tunable). */
#ifndef FLASH_TO_PSRAM_CHUNK
#define FLASH_TO_PSRAM_CHUNK	(8U * 1024U)
#endif

/* Pin states (owned via helper DT nodes so the flash driver is unaffected). */
PINCTRL_DT_DEFINE(DT_NODELABEL(ospi0_flash_pins));
PINCTRL_DT_DEFINE(DT_NODELABEL(ospi0_psram_pins));

static const struct pinctrl_dev_config *const flash_pins =
	PINCTRL_DT_DEV_CONFIG_GET(DT_NODELABEL(ospi0_flash_pins));
static const struct pinctrl_dev_config *const psram_pins =
	PINCTRL_DT_DEV_CONFIG_GET(DT_NODELABEL(ospi0_psram_pins));

static struct ospi_regs *const ospi_regs =
	(struct ospi_regs *)DT_REG_ADDR(OSPI0_NODE);
static struct ospi_aes_regs *const ospi_aes =
	(struct ospi_aes_regs *)DT_PROP_BY_IDX(OSPI0_NODE, aes_reg, 0);

static uint8_t stage_buf[FLASH_TO_PSRAM_CHUNK] __aligned(4);

static HAL_OSPI_Handle_T psram_handle;
static bool psram_handle_ready;

static void loader_ospi_event_cb(uint32_t event, void *user_data)
{
	ARG_UNUSED(event);
	ARG_UNUSED(user_data);
}

/*
 * One-time: grab a second OSPI0 HAL instance for the PSRAM chip-select. This
 * writes the shared controller globals (thresholds, bus speed, baud2); the
 * per-phase timing (ddr-drive-edge / rx-ds-delay) is (re)programmed on every
 * switch, so the values passed here only seed the instance.
 */
static int psram_handle_init(void)
{
	struct ospi_init init_config;
	int32_t ret;

	if (psram_handle_ready) {
		return 0;
	}

	memset(&init_config, 0, sizeof(init_config));
	init_config.core_clk = DT_PROP(OSPI0_NODE, clock_frequency);
	init_config.bus_speed = DT_PROP(OSPI0_NODE, bus_speed);
	init_config.tx_fifo_threshold = DT_PROP(OSPI0_NODE, tx_fifo_threshold);
	init_config.rx_fifo_threshold = 0U;
	init_config.rx_sample_delay = 0U;
	init_config.ddr_drive_edge = PSRAM_DDR_DRIVE_EDGE;
	init_config.cs_pin = PSRAM_CS_PIN;
	init_config.rx_ds_delay = PSRAM_RX_DS_DELAY;
	init_config.baud2_delay = DT_PROP(OSPI0_NODE, baud2_delay);
	init_config.base_regs = (uint32_t *)ospi_regs;
	init_config.aes_regs = (uint32_t *)ospi_aes;
	init_config.event_cb = loader_ospi_event_cb;
	init_config.user_data = NULL;

	ret = alif_hal_ospi_initialize(&psram_handle, &init_config);
	if (ret != 0) {
		printk("copy: psram HAL init failed: %d\n", (int)ret);
		return -EIO;
	}

	psram_handle_ready = true;
	return 0;
}

/* Hand the shared bus to the flash: leave XiP, mux flash pins, flash timing. */
static int bus_to_flash(void)
{
	int ret;

	aes_disable_xip(ospi_aes);
	ret = pinctrl_apply_state(flash_pins, PINCTRL_STATE_DEFAULT);
	if (ret != 0) {
		printk("copy: apply flash pinctrl failed: %d\n", ret);
		return ret;
	}
	ospi_set_ddr_drive_edge(ospi_regs, FLASH_DDR_DRIVE_EDGE);
	ospi_aes->AES_RXDS_DLY = FLASH_RX_DS_DELAY;
	return 0;
}

/*
 * Hand the shared bus to the PSRAM: mux PSRAM pins, PSRAM timing, reconfigure
 * OSPI0 for octal-DDR HyperRAM and enter memory-mapped XiP.
 */
static int bus_to_psram(void)
{
	struct ospi_trans_config trans_conf;
	struct ospi_xip_config xip_cfg;
	int ret;

	ret = pinctrl_apply_state(psram_pins, PINCTRL_STATE_DEFAULT);
	if (ret != 0) {
		printk("copy: apply psram pinctrl failed: %d\n", ret);
		return ret;
	}
	ospi_set_ddr_drive_edge(ospi_regs, PSRAM_DDR_DRIVE_EDGE);
	ospi_aes->AES_RXDS_DLY = PSRAM_RX_DS_DELAY;

	memset(&trans_conf, 0, sizeof(trans_conf));
	trans_conf.frame_size = PSRAM_OSPI_DFS;
	trans_conf.frame_format = OSPI_FRF_OCTAL;
	trans_conf.ddr_enable = OSPI_DDR_ENABLE;
	ret = alif_hal_ospi_prepare_transfer(psram_handle, &trans_conf);
	if (ret != 0) {
		printk("copy: PSRAM transfer setup failed: %d\n", ret);
		return ret;
	}

	memset(&xip_cfg, 0, sizeof(xip_cfg));
	xip_cfg.xip_wait_cycles = DT_PROP(OSPI0_NODE, xip_wait_cycles);
	xip_cfg.xip_cs_pin = PSRAM_CS_PIN;
	ospi_hyperbus_xip_init(ospi_regs, &xip_cfg, false);

	aes_enable_xip(ospi_aes);
	return 0;
}

/* Copy one chunk from the SRAM buffer into the PSRAM XiP window (16-bit). */
static void psram_xip_write(off_t psram_off, const uint8_t *src, size_t len)
{
	volatile uint16_t *dst =
		(volatile uint16_t *)(uintptr_t)(PSRAM_XIP_BASE + psram_off);
	const uint16_t *words = (const uint16_t *)src;
	size_t nwords = len / sizeof(uint16_t);

	for (size_t i = 0; i < nwords; i++) {
		dst[i] = words[i];
	}

	barrier_dsync_fence_full();
}

int flash_to_psram_copy(off_t flash_off, off_t psram_off, size_t len)
{
	const struct device *flash_dev = DEVICE_DT_GET(FLASH_NODE);
	int ret;

	if (len == 0U) {
		return 0;
	}

	/* 16-bit HyperRAM frames and the XiP window must contain the destination. */
	if (psram_off < 0 || (len % 2U) != 0U || (psram_off % 2) != 0 ||
	    (size_t)psram_off > PSRAM_USABLE_SIZE ||
	    len > PSRAM_USABLE_SIZE - (size_t)psram_off) {
		return -EINVAL;
	}

	if (!device_is_ready(flash_dev)) {
		return -ENODEV;
	}

	ret = psram_handle_init();
	if (ret != 0) {
		return ret;
	}

	printk("copy: %zu bytes flash@0x%lx -> psram@0x%lx (chunk %uB)\n",
	       len, (unsigned long)flash_off, (unsigned long)psram_off,
	       (unsigned int)FLASH_TO_PSRAM_CHUNK);

	for (size_t done = 0; done < len; done += FLASH_TO_PSRAM_CHUNK) {
		size_t chunk = MIN((size_t)FLASH_TO_PSRAM_CHUNK, len - done);

		/* FLASH phase: stage one chunk from flash into SRAM. */
		ret = bus_to_flash();
		if (ret != 0) {
			return ret;
		}
		ret = flash_read(flash_dev, flash_off + done, stage_buf, chunk);
		if (ret != 0) {
			printk("copy: flash_read @0x%zx failed: %d\n",
			       (size_t)flash_off + done, ret);
			return ret;
		}

		/* PSRAM phase: push the chunk into the PSRAM XiP window. */
		ret = bus_to_psram();
		if (ret != 0) {
			return ret;
		}
		psram_xip_write(psram_off + done, stage_buf, chunk);
	}

	printk("copy: done (%zu bytes, bus left in PSRAM XiP mode)\n", len);
	return 0;
}

/* Layout produced by test_4mb.bin: 8-byte magic, offset-encoded words, CRC32 tail. */
#define TEST_MAGIC		"ALIFPSRM"
#define TEST_MAGIC_LEN		8U

int flash_to_psram_verify(off_t psram_off, size_t len)
{
	const uint8_t *base = (const uint8_t *)(uintptr_t)(PSRAM_XIP_BASE + psram_off);
	uint32_t calc_crc, trailer;
	bool magic_ok, pattern_ok, crc_ok;

	if (len < (TEST_MAGIC_LEN + 4U) || (len % 4U) != 0U) {
		return -EINVAL;
	}

	/* Destination range must fit inside the PSRAM XiP window. */
	if (psram_off < 0 || (size_t)psram_off > PSRAM_USABLE_SIZE ||
	    len > PSRAM_USABLE_SIZE - (size_t)psram_off) {
		return -EINVAL;
	}

	/* 1. Magic header copied intact. */
	magic_ok = (memcmp(base, TEST_MAGIC, TEST_MAGIC_LEN) == 0);
	if (magic_ok) {
		printk("verify: magic OK (%.8s)\n", base);
	} else {
		printk("verify: magic FAIL (got %02x %02x %02x %02x ...)\n",
		       base[0], base[1], base[2], base[3]);
	}

	/* 2. Every word (past the magic, before the CRC tail) equals its offset. */
	pattern_ok = true;
	for (size_t off = TEST_MAGIC_LEN; off < (len - 4U); off += 4U) {
		uint32_t word;

		memcpy(&word, base + off, sizeof(word));
		if (word != (uint32_t)off) {
			printk("verify: pattern FAIL @0x%05zx exp 0x%08x got 0x%08x\n",
			       off, (uint32_t)off, word);
			pattern_ok = false;
			break;
		}
	}
	if (pattern_ok) {
		printk("verify: offset-word pattern OK (0x%x..0x%x)\n",
		       (uint32_t)TEST_MAGIC_LEN, (uint32_t)(len - 8U));
	}

	/* 3. CRC32 over everything but the 4-byte trailer. */
	calc_crc = crc32_ieee(base, len - 4U);
	memcpy(&trailer, base + (len - 4U), sizeof(trailer));
	crc_ok = (calc_crc == trailer);
	printk("verify: crc32 %s (calc 0x%08x, stored 0x%08x)\n",
	       crc_ok ? "OK" : "FAIL", calc_crc, trailer);

	if (magic_ok && pattern_ok && crc_ok) {
		printk("verify: PASS -- %zu bytes copied to PSRAM XiP @0x%08lx\n",
		       len, (unsigned long)(uintptr_t)base);
		return 0;
	}

	printk("verify: FAIL\n");
	return -EIO;
}

#else /* !(flash0 && psram aliases) */

int flash_to_psram_copy(off_t flash_off, off_t psram_off, size_t len)
{
	ARG_UNUSED(flash_off);
	ARG_UNUSED(psram_off);
	ARG_UNUSED(len);

	return -ENOTSUP;
}

int flash_to_psram_verify(off_t psram_off, size_t len)
{
	ARG_UNUSED(psram_off);
	ARG_UNUSED(len);

	return -ENOTSUP;
}

#endif /* flash0 && psram aliases */
