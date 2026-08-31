/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Copy an OSPI flash image into the PSRAM XiP window and check the same
 * layout as samples/drivers/spi_flash_to_psram_copy (ALIFPSRM magic,
 * offset-encoded words, CRC32 tail).
 *
 * PSRAM comes from boards/alif_psram.overlay (OSPI0, APS512XXN). Flash comes
 * from the ospi-flash snippet (OSPI1 on E8). The two devices are on separate
 * controllers, so the copy is flash_read() into the PSRAM XiP window. No
 * AIPM / power-management overlay is used.
 */

#include <string.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/crc.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#if !DT_HAS_ALIAS(spi_psram)
#error "Devicetree alias spi-psram is required (boards/alif_psram.overlay)"
#endif

#if !DT_HAS_ALIAS(spi_flash0)
#error "Devicetree alias spi-flash0 is required (snippet ospi-flash)"
#endif

LOG_MODULE_REGISTER(hyperram_flash_copy, LOG_LEVEL_INF);

#define FLASH_NODE        DT_ALIAS(spi_flash0)
#define PSRAM_NODE        DT_ALIAS(spi_psram)
#define PSRAM_CTRL_NODE   DT_PARENT(PSRAM_NODE)

#define FLASH_SECTOR_SIZE DT_PROP(FLASH_NODE, sector_size)
/* 1 KiB image. Erase is still one full sector; the driver rejects a short erase. */
#define COPY_TEST_BYTES   1024U
#define COPY_CHUNK        1024U
#define COPY_FLASH_OFF    0x10000
#define COPY_PSRAM_OFF    0x0
#define COPY_PSRAM_OFF2   0x20000

#define PSRAM_XIP_BASE    ((uintptr_t)DT_PROP_BY_IDX(PSRAM_CTRL_NODE, xip_base_address, 0))
#define PSRAM_XIP_SIZE    ((size_t)DT_PROP_BY_IDX(PSRAM_CTRL_NODE, xip_base_address, 1))
#define PSRAM_DEV_SIZE    ((size_t)DT_PROP(PSRAM_NODE, size))
#define PSRAM_USABLE      MIN(PSRAM_XIP_SIZE, PSRAM_DEV_SIZE)

#define TEST_MAGIC        "ALIFPSRM"
#define TEST_MAGIC_LEN    8U

static const struct device *const flash_dev = DEVICE_DT_GET(FLASH_NODE);
static const struct device *const psram_dev = DEVICE_DT_GET(PSRAM_NODE);
static uint8_t test_image[COPY_TEST_BYTES];

static void build_test_image(uint8_t *buf, size_t size)
{
	uint32_t crc;

	memset(buf, 0, size);
	memcpy(buf, TEST_MAGIC, TEST_MAGIC_LEN);

	for (size_t off = TEST_MAGIC_LEN; off < (size - 4U); off += 4U) {
		uint32_t word = (uint32_t)off;

		memcpy(buf + off, &word, sizeof(word));
	}

	crc = crc32_ieee(buf, size - 4U);
	memcpy(buf + (size - 4U), &crc, sizeof(crc));
}

static bool range_ok(off_t psram_off, size_t len)
{
	if (psram_off < 0 || (size_t)psram_off > PSRAM_USABLE) {
		return false;
	}

	return len <= (PSRAM_USABLE - (size_t)psram_off);
}

/* flash_read() into the PSRAM XiP window. Length and offset are word aligned. */
static int flash_to_psram_copy(off_t flash_off, off_t psram_off, size_t len)
{
	uint8_t chunk[COPY_CHUNK];

	if (len == 0U) {
		return 0;
	}

	if ((len % 4U) != 0U || (psram_off % 4) != 0 || !range_ok(psram_off, len)) {
		return -EINVAL;
	}

	for (size_t done = 0; done < len; done += COPY_CHUNK) {
		size_t n = MIN((size_t)COPY_CHUNK, len - done);
		volatile uint32_t *dst;
		const uint32_t *src;
		int ret;

		ret = flash_read(flash_dev, flash_off + (off_t)done, chunk, n);
		if (ret != 0) {
			return ret;
		}

		dst = (volatile uint32_t *)(PSRAM_XIP_BASE + (uintptr_t)psram_off + done);
		if (done == 0U) {
			LOG_INF("flash read ok, first store at 0x%08lx",
				(unsigned long)(uintptr_t)dst);
		}
		src = (const uint32_t *)chunk;
		for (size_t i = 0; i < (n / sizeof(uint32_t)); i++) {
			dst[i] = src[i];
		}
	}

	return 0;
}

static int flash_to_psram_verify(off_t psram_off, size_t len)
{
	const uint8_t *base = (const uint8_t *)(PSRAM_XIP_BASE + (uintptr_t)psram_off);
	uint32_t calc_crc;
	uint32_t trailer;

	if (len < (TEST_MAGIC_LEN + 4U) || (len % 4U) != 0U || !range_ok(psram_off, len)) {
		return -EINVAL;
	}

	if (memcmp(base, TEST_MAGIC, TEST_MAGIC_LEN) != 0) {
		LOG_INF("verify: magic FAIL (got %02x %02x %02x %02x)",
			base[0], base[1], base[2], base[3]);
		return -EIO;
	}

	for (size_t off = TEST_MAGIC_LEN; off < (len - 4U); off += 4U) {
		uint32_t word;

		memcpy(&word, base + off, sizeof(word));
		if (word != (uint32_t)off) {
			LOG_INF("verify: pattern FAIL @0x%zx exp 0x%08x got 0x%08x",
				off, (uint32_t)off, word);
			return -EIO;
		}
	}

	calc_crc = crc32_ieee(base, len - 4U);
	memcpy(&trailer, base + (len - 4U), sizeof(trailer));
	if (calc_crc != trailer) {
		LOG_INF("verify: crc32 FAIL (calc 0x%08x, stored 0x%08x)", calc_crc, trailer);
		return -EIO;
	}

	LOG_INF("verify: PASS -- %zu bytes at PSRAM XiP 0x%08lx",
		len, (unsigned long)(uintptr_t)base);
	return 0;
}

static void program_test_image(void)
{
	uint8_t readback[256];
	int ret;

	ret = flash_erase(flash_dev, COPY_FLASH_OFF, FLASH_SECTOR_SIZE);
	zassert_equal(ret, 0, "flash_erase @0x%x failed: %d", COPY_FLASH_OFF, ret);

	ret = flash_write(flash_dev, COPY_FLASH_OFF, test_image, sizeof(test_image));
	zassert_equal(ret, 0, "flash_write @0x%x failed: %d", COPY_FLASH_OFF, ret);

	for (size_t off = 0; off < sizeof(test_image); off += sizeof(readback)) {
		size_t n = MIN(sizeof(readback), sizeof(test_image) - off);

		ret = flash_read(flash_dev, COPY_FLASH_OFF + (off_t)off, readback, n);
		zassert_equal(ret, 0, "flash_read @0x%zx failed: %d",
				  (size_t)COPY_FLASH_OFF + off, ret);
		zassert_mem_equal(readback, &test_image[off], n,
				  "flash readback mismatch at image offset 0x%zx", off);
	}
}

static void assert_psram_matches(off_t psram_off, const char *tag)
{
	const uint8_t *xip = (const uint8_t *)(PSRAM_XIP_BASE + (uintptr_t)psram_off);
	int ret;

	ret = flash_to_psram_verify(psram_off, sizeof(test_image));
	zassert_equal(ret, 0, "%s: verify failed: %d", tag, ret);

	for (size_t off = 0; off < sizeof(test_image); off += sizeof(uint32_t)) {
		uint32_t got;
		uint32_t exp;

		memcpy(&got, xip + off, sizeof(got));
		memcpy(&exp, &test_image[off], sizeof(exp));
		zassert_equal(got, exp, "%s: mismatch at 0x%zx exp 0x%08x got 0x%08x",
				  tag, off, exp, got);
	}
}

static void *flash_copy_setup(void)
{
	LOG_INF("flash -> PSRAM copy (image %u bytes, flash 0x%x, XiP 0x%08lx)",
		(unsigned int)sizeof(test_image), COPY_FLASH_OFF,
		(unsigned long)PSRAM_XIP_BASE);

	zassert_true(device_is_ready(psram_dev), "PSRAM %s is not ready", psram_dev->name);
	zassert_true(device_is_ready(flash_dev), "OSPI flash %s is not ready", flash_dev->name);

	build_test_image(test_image, sizeof(test_image));
	program_test_image();
	return NULL;
}

ZTEST(hyperram_flash_copy, test_copy_rejects_bad_args)
{
	size_t too_big = PSRAM_USABLE + 4U;

	LOG_INF("=== reject bad copy/verify arguments ===");

	zassert_equal(flash_to_psram_copy(0, 0, 0), 0, "zero-length copy should succeed");
	zassert_equal(flash_to_psram_copy(COPY_FLASH_OFF, 1, 16), -EINVAL,
			  "unaligned PSRAM offset must be rejected");
	zassert_equal(flash_to_psram_copy(COPY_FLASH_OFF, 0, 2), -EINVAL,
			  "length that is not word aligned must be rejected");
	zassert_equal(flash_to_psram_copy(COPY_FLASH_OFF, 0, too_big), -EINVAL,
			  "copy past the PSRAM window must be rejected");

	zassert_equal(flash_to_psram_verify(0, 8), -EINVAL,
			  "verify shorter than magic + CRC must be rejected");
	zassert_equal(flash_to_psram_verify(0, 14), -EINVAL,
			  "verify length that is not word aligned must be rejected");
	zassert_equal(flash_to_psram_verify(0, too_big), -EINVAL,
			  "verify past the PSRAM window must be rejected");
}

ZTEST(hyperram_flash_copy, test_copy_and_verify_image)
{
	int ret;

	LOG_INF("=== copy %u bytes flash@0x%x -> psram@0x%x ===",
		(unsigned int)sizeof(test_image), COPY_FLASH_OFF, COPY_PSRAM_OFF);

	ret = flash_to_psram_copy(COPY_FLASH_OFF, COPY_PSRAM_OFF, sizeof(test_image));
	zassert_equal(ret, 0, "copy failed: %d", ret);

	assert_psram_matches(COPY_PSRAM_OFF, "psram@0");
	LOG_INF("copy and verify PASSED");
}

ZTEST(hyperram_flash_copy, test_copy_second_region_keeps_first)
{
	int ret;

	LOG_INF("=== second copy to psram@0x%x leaves the first region intact ===",
		COPY_PSRAM_OFF2);

	ret = flash_to_psram_copy(COPY_FLASH_OFF, COPY_PSRAM_OFF, sizeof(test_image));
	zassert_equal(ret, 0, "first copy failed: %d", ret);

	ret = flash_to_psram_copy(COPY_FLASH_OFF, COPY_PSRAM_OFF2, sizeof(test_image));
	zassert_equal(ret, 0, "second copy failed: %d", ret);

	assert_psram_matches(COPY_PSRAM_OFF2, "psram@0x20000");
	assert_psram_matches(COPY_PSRAM_OFF, "psram@0 after second copy");
	LOG_INF("second-region copy PASSED");
}

#if IS_ENABLED(CONFIG_TEST_PSRAM_FLASH_COPY_FULL)
/* Same check as the sample: first 4MB of OSPI flash. Requires test.bin from
 * gen_test_bin.py --size 4M programmed at flash offset 0.
 */
ZTEST(hyperram_flash_copy, test_copy_preprogrammed_4mb)
{
	const size_t len = 4U * 1024U * 1024U;
	int ret;

	LOG_INF("=== copy preprogrammed 4MB test.bin ===");

	ret = flash_to_psram_copy(0, 0, len);
	zassert_equal(ret, 0, "4MB copy failed: %d", ret);

	ret = flash_to_psram_verify(0, len);
	zassert_equal(ret, 0,
			  "4MB verify failed (%d). Program test.bin from "
			  "gen_test_bin.py --size 4M at OSPI flash offset 0",
			  ret);
}
#endif

ZTEST_SUITE(hyperram_flash_copy, NULL, flash_copy_setup, NULL, NULL, NULL);

