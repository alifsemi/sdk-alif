/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef I3C_COMMON_H
#define I3C_COMMON_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/i3c.h>
#include <zephyr/drivers/i3c/ccc.h>
#include <zephyr/devicetree.h>

/* Max targets supported */
#define I3C_TEST_MAX_TARGETS 8

/* Target configuration from DT or Kconfig */
#define I3C_CONTROLLER_NODE DT_NODELABEL(i3c0)

/* Get target count from DT - count i3c devices on bus */
#if DT_HAS_COMPAT_STATUS_OKAY(bosch_bmi323)
#define I3C_TARGET_0_DA DT_PROP(DT_NODELABEL(bmi323), assigned_address)
#else
#define I3C_TARGET_0_DA 0x09
#endif
#define I3C_TEST_BMI323_ADDR I3C_TARGET_0_DA

#if DT_HAS_COMPAT_STATUS_OKAY(invensense_icm42670p)
#define I3C_TARGET_1_DA DT_PROP(DT_NODELABEL(icm42670), assigned_address)
#define I3C_TEST_ICM42670_ADDR I3C_TARGET_1_DA
#else
#define I3C_TARGET_1_DA 0x0A
#define I3C_TEST_ICM42670_ADDR 0x0A
#endif

/* Default targets array */
#ifndef I3C_TEST_NUM_TARGETS
#ifdef CONFIG_I3C_TEST_NUM_TARGETS
#define I3C_TEST_NUM_TARGETS CONFIG_I3C_TEST_NUM_TARGETS
#else
#define I3C_TEST_NUM_TARGETS 2
#endif
#endif

#ifndef I3C_TEST_TARGET_DA
#ifdef CONFIG_I3C_TEST_TARGET_DA
#define I3C_TEST_TARGET_DA CONFIG_I3C_TEST_TARGET_DA
#else
#define I3C_TEST_TARGET_DA I3C_TARGET_0_DA
#endif
#endif

#ifndef I3C_TEST_CHIP_ID_REG
#ifdef CONFIG_I3C_TEST_CHIP_ID_REG
#define I3C_TEST_CHIP_ID_REG CONFIG_I3C_TEST_CHIP_ID_REG
#else
#define I3C_TEST_CHIP_ID_REG 0x00
#endif
#endif

#ifndef I3C_TEST_CHIP_ID_VAL
#ifdef CONFIG_I3C_TEST_CHIP_ID_VAL
#define I3C_TEST_CHIP_ID_VAL CONFIG_I3C_TEST_CHIP_ID_VAL
#else
#define I3C_TEST_CHIP_ID_VAL 0x00
#endif
#endif

/* I3C Common Command Codes - test wrappers */
int i3c_do_ccc_rstdaa(const struct device *ctrl);

int i3c_do_ccc_entdaa(const struct device *ctrl);

int i3c_do_ccc_getpid(const struct device *ctrl, uint8_t da,
		     uint8_t *pid, size_t len);

int i3c_do_ccc_getbcr(const struct device *ctrl, uint8_t da, uint8_t *bcr);

int i3c_do_ccc_getdcr(const struct device *ctrl, uint8_t da, uint8_t *dcr);

int i3c_do_ccc_setmwl(const struct device *ctrl, uint8_t da, uint16_t mwl);

int i3c_do_ccc_getmwl(const struct device *ctrl, uint8_t da, uint16_t *mwl);

int i3c_do_ccc_getmrl(const struct device *ctrl, uint8_t da, uint16_t *mrl);

int i3c_do_ccc_getstatus(const struct device *ctrl, uint8_t da,
			 uint16_t *status);

int i3c_do_ccc_enec(const struct device *ctrl, uint8_t da, uint8_t events);

int i3c_do_ccc_disec(const struct device *ctrl, uint8_t da, uint8_t events);

int i3c_do_ccc_enec_broadcast(const struct device *ctrl, uint8_t events);

int i3c_do_ccc_disec_broadcast(const struct device *ctrl, uint8_t events);

int i3c_do_ccc_getcaps(const struct device *ctrl, uint8_t da,
		       uint8_t *caps, size_t len);

int i3c_do_ccc_rstact(const struct device *ctrl, uint8_t da, uint8_t action);

int i3c_do_ccc_enthdr(const struct device *ctrl, uint8_t hdr_mode);

/* I3C Transfer operations */
int i3c_transfer_write(const struct device *ctrl, uint8_t da,
		       const uint8_t *buf, size_t len);

int i3c_transfer_read(const struct device *ctrl, uint8_t da,
		      uint8_t *buf, size_t len);

int i3c_transfer_write_read(const struct device *ctrl, uint8_t da,
			    const uint8_t *write_buf, size_t write_len,
			    uint8_t *read_buf, size_t read_len);

/* Register operations (write reg addr then read/write data) */
int i3c_reg_read(const struct device *ctrl, uint8_t da, uint8_t reg,
		 uint8_t *buf, size_t len);

int i3c_reg_write(const struct device *ctrl, uint8_t da, uint8_t reg,
		  const uint8_t *buf, size_t len);

int i3c_reg8_read(const struct device *ctrl, uint8_t da,
		  uint8_t reg, uint8_t *val);

int i3c_reg8_write(const struct device *ctrl, uint8_t da,
		   uint8_t reg, uint8_t val);

/* Target helpers */
struct i3c_device_desc *i3c_find_target(const struct device *ctrl, uint8_t da);
int i3c_get_target_count(const struct device *ctrl);

/* Target attach/detach for test setup/teardown */
int i3c_test_attach_target(const struct device *ctrl,
			   struct i3c_device_desc *desc);

int i3c_test_detach_target(const struct device *ctrl,
			   struct i3c_device_desc *desc);

/* Chip ID validation */
int i3c_check_chip_id(const struct device *ctrl, uint8_t da,
		      uint8_t reg, uint8_t expected);

/* ============================================================================
 * Frequency Mode Support
 * ============================================================================
 * I3C bus speed modes:
 *   - SDR: Single Data Rate (12.5 MHz typical)
 *   - HDR-DDR: Double Data Rate mode (up to 25 MHz)
 *   - HDR-TSP/TSL: Ternary symbol modes (up to 25 MHz)
 *
 * I2C Legacy speeds for mixed bus:
 *   - FM: Fast Mode (400 kHz) - disabled by default
 *   - FM+: Fast Mode Plus (1 MHz) - disabled by default
 *
 * HDR and I2C legacy modes are disabled by default as current targets
 * only support SDR.
 */

/* Uncomment to enable I2C-FM (400 kHz) mode testing:
 * #define I3C_TEST_ENABLE_I2C_FM
 */

/* Uncomment to enable HDR mode testing:
 * #define I3C_TEST_ENABLE_HDR_MODES
 */

enum i3c_test_speed_mode {
	/* I3C Native modes - tested first */
	I3C_TEST_SDR = 0,        /* SDR (12.5 MHz) - always enabled */

#ifdef I3C_TEST_ENABLE_HDR_MODES
	/* HDR modes - require I3C SDR context, tested after SDR */
	I3C_TEST_HDR_DDR = 1,    /* HDR-DDR (25 MHz) */
	I3C_TEST_HDR_TSP = 2,    /* HDR-TSP (25 MHz) */
	I3C_TEST_HDR_TSL = 3,    /* HDR-TSL (25 MHz) */
#endif

	/* I2C Legacy modes - tested last (require different bus context) */
#ifdef I3C_TEST_ENABLE_I2C_FM
	I3C_TEST_I2C_FM = 4,     /* I2C Fast Mode (400 kHz) */
	I3C_TEST_I2C_FMPLUS = 5, /* I2C Fast Mode Plus (1 MHz) */
#endif

	I3C_TEST_NUM_MODES       /* Total supported modes */
};

/* I3C frequency values in Hz */
#define I3C_TEST_SDR_FREQ_HZ      12500000U  /* 12.5 MHz */
#define I3C_TEST_HDR_FREQ_HZ      25000000U  /* 25 MHz for all HDR modes */

/* I2C legacy speed values in Hz */
#define I3C_TEST_I2C_FM_HZ        400000U    /* 400 kHz I2C FM */
#define I3C_TEST_I2C_FMPLUS_HZ    1000000U   /* 1 MHz I2C FM+ */

/* Set current test speed mode */
int i3c_test_set_speed_mode(const struct device *ctrl,
			    enum i3c_test_speed_mode mode);

/* Get current test speed mode */
enum i3c_test_speed_mode i3c_test_get_speed_mode(const struct device *ctrl);

/* Reset bus to SDR mode - configures frequency, does NOT do RSTDAA+DAA */
int i3c_test_reset_to_sdr(const struct device *ctrl);

/**
 * @brief Perform RSTDAA + DAA for test setup.
 *
 * Runs the full sequence on every call. The RSTDAA CCC helper keeps
 * cached target descriptors and address slots synchronized with the bus.
 *
 * @param ctrl I3C controller device
 * @return Number of assigned devices (>= 0), negative errno on failure
 */
int i3c_test_do_rstdaa_daa(const struct device *ctrl);

/* Run test on all speed modes */
#define I3C_TEST_FOR_EACH_MODE(ctrl, mode) \
	const struct device *_test_ctrl = (ctrl); \
	enum i3c_test_speed_mode mode; \
	int _mode_setup_ret; \
	for (mode = I3C_TEST_SDR; mode < I3C_TEST_NUM_MODES; mode++) { \
		_mode_setup_ret = i3c_test_set_speed_mode(_test_ctrl, mode); \
		if (_mode_setup_ret == 0) { \
			LOG_INF("Testing in %s mode (%u Hz)", \
				i3c_test_mode_name(mode), \
				i3c_test_mode_frequency(mode));

#define I3C_TEST_END_EACH_MODE \
		} \
	} \
	/* Return to SDR after HDR modes to ensure clean state */ \
	if (mode != I3C_TEST_SDR) { \
		i3c_test_reset_to_sdr(_test_ctrl); \
	} \
	(void)_test_ctrl;

/* Get mode name and frequency for logging */
const char *i3c_test_mode_name(enum i3c_test_speed_mode mode);
uint32_t i3c_test_mode_frequency(enum i3c_test_speed_mode mode);

/* Multi-target test execution macro */
/* Run test code block on each target with assigned dynamic address.
 * Skip if none found. Note: static_addr may be 0 for I3C-only devices.
 */
#define I3C_TEST_FOR_EACH_TARGET(ctrl, desc) \
	struct i3c_device_desc *desc = NULL; \
	int _target_found = 0; \
	I3C_BUS_FOR_EACH_I3CDEV((ctrl), desc) { \
		if (desc->dynamic_addr == 0) { \
			continue; \
		} \
		_target_found = 1; \
		{

#define I3C_TEST_END_EACH_TARGET \
		} \
	} \
	if (!_target_found) { \
		ztest_test_skip(); \
		(void)_target_found; \
	}
#endif
