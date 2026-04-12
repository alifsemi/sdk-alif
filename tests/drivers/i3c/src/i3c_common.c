/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/i3c.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include "i3c_common.h"

LOG_MODULE_REGISTER(i3c_common, LOG_LEVEL_INF);

/* CCC wrapper functions */
int i3c_do_ccc_rstdaa(const struct device *ctrl)
{
	return i3c_ccc_do_rstdaa_all(ctrl);
}

int i3c_do_ccc_entdaa(const struct device *ctrl)
{
	struct i3c_ccc_payload ccc = { .ccc.id = I3C_CCC_ENTDAA };

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_do_ccc_getpid(const struct device *ctrl, uint8_t da,
		      uint8_t *pid, size_t len)
{
	if (!pid || len < 6) {
		return -EINVAL;
	}

	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 1,
		.data = pid,
		.data_len = 6,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_GETPID,
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	int ret = i3c_do_ccc(ctrl, &ccc);

	if (ret < 0) {
		return ret;
	}
	return target.num_xfer;
}

int i3c_do_ccc_getbcr(const struct device *ctrl, uint8_t da, uint8_t *bcr)
{
	if (!bcr) {
		return -EINVAL;
	}

	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 1,
		.data = bcr,
		.data_len = 1,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_GETBCR,
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_do_ccc_getdcr(const struct device *ctrl, uint8_t da, uint8_t *dcr)
{
	if (!dcr) {
		return -EINVAL;
	}

	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 1,
		.data = dcr,
		.data_len = 1,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_GETDCR,
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_do_ccc_setmwl(const struct device *ctrl, uint8_t da, uint16_t mwl)
{
	uint8_t data[2] = { (mwl >> 8) & 0xFF, mwl & 0xFF };
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 0,
		.data = data,
		.data_len = 2,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_SETMWL(false),
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_do_ccc_getmwl(const struct device *ctrl, uint8_t da, uint16_t *mwl)
{
	if (!mwl) {
		return -EINVAL;
	}

	uint8_t data[2];
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 1,
		.data = data,
		.data_len = 2,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_GETMWL,
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	int ret = i3c_do_ccc(ctrl, &ccc);

	if (ret >= 0) {
		*mwl = ((uint16_t)data[0] << 8) | data[1];
	}
	return ret;
}

int i3c_do_ccc_enec(const struct device *ctrl, uint8_t da, uint8_t events)
{
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 0,
		.data = &events,
		.data_len = 1,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_ENEC(false),
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_do_ccc_disec(const struct device *ctrl, uint8_t da, uint8_t events)
{
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 0,
		.data = &events,
		.data_len = 1,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_DISEC(false),
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_do_ccc_getmrl(const struct device *ctrl, uint8_t da, uint16_t *mrl)
{
	if (!mrl) {
		return -EINVAL;
	}

	uint8_t data[2];
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 1,
		.data = data,
		.data_len = 2,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_GETMRL,
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	int ret = i3c_do_ccc(ctrl, &ccc);

	if (ret >= 0) {
		*mrl = ((uint16_t)data[0] << 8) | data[1];
	}
	return ret;
}

int i3c_do_ccc_getstatus(const struct device *ctrl, uint8_t da,
			 uint16_t *status)
{
	if (!status) {
		return -EINVAL;
	}

	uint8_t data[2];
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 1,
		.data = data,
		.data_len = 2,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_GETSTATUS,
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	int ret = i3c_do_ccc(ctrl, &ccc);

	if (ret >= 0) {
		*status = ((uint16_t)data[0] << 8) | data[1];
	}
	return ret;
}

int i3c_do_ccc_enec_broadcast(const struct device *ctrl, uint8_t events)
{
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_ENEC(true),
		.ccc.data = &events,
		.ccc.data_len = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_do_ccc_disec_broadcast(const struct device *ctrl, uint8_t events)
{
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_DISEC(true),
		.ccc.data = &events,
		.ccc.data_len = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_do_ccc_getcaps(const struct device *ctrl, uint8_t da,
		       uint8_t *caps, size_t len)
{
	if (!caps || len == 0) {
		return -EINVAL;
	}

	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 1,
		.data = caps,
		.data_len = len,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_GETCAPS,
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

int i3c_do_ccc_rstact(const struct device *ctrl, uint8_t da, uint8_t action)
{
	struct i3c_ccc_target_payload target = {
		.addr = da,
		.rnw = 0,
		.data = &action,
		.data_len = 1,
	};
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_RSTACT(false),
		.targets.payloads = &target,
		.targets.num_targets = 1,
	};

	return i3c_do_ccc(ctrl, &ccc);
}

/* ENTHDR - Enter HDR Mode (Broadcast, no data needed) */
int i3c_do_ccc_enthdr(const struct device *ctrl, uint8_t hdr_mode)
{
	struct i3c_ccc_payload ccc = {
		.ccc.id = I3C_CCC_ENTHDR(hdr_mode),
	};

	return i3c_do_ccc(ctrl, &ccc);
}

/* Transfer wrappers */
int i3c_transfer_write(const struct device *ctrl, uint8_t da,
		       const uint8_t *buf, size_t len)
{
	if (!buf || len == 0) {
		return -EINVAL;
	}

	struct i3c_device_desc *desc = i3c_find_target(ctrl, da);

	if (!desc) {
		return -ENODEV;
	}

	struct i3c_msg msg = {
		.buf = (uint8_t *)buf,
		.len = len,
		.flags = I3C_MSG_WRITE | I3C_MSG_STOP,
	};

	return i3c_transfer(desc, &msg, 1);
}

int i3c_transfer_read(const struct device *ctrl, uint8_t da,
		      uint8_t *buf, size_t len)
{
	if (!buf || len == 0) {
		return -EINVAL;
	}

	struct i3c_device_desc *desc = i3c_find_target(ctrl, da);

	if (!desc) {
		return -ENODEV;
	}

	struct i3c_msg msg = {
		.buf = buf,
		.len = len,
		.flags = I3C_MSG_READ | I3C_MSG_STOP,
	};

	return i3c_transfer(desc, &msg, 1);
}

int i3c_transfer_write_read(const struct device *ctrl, uint8_t da,
			    const uint8_t *write_buf, size_t write_len,
			    uint8_t *read_buf, size_t read_len)
{
	if (!write_buf || !read_buf || write_len == 0 || read_len == 0) {
		return -EINVAL;
	}

	struct i3c_device_desc *desc = i3c_find_target(ctrl, da);

	if (!desc) {
		return -ENODEV;
	}

	struct i3c_msg msgs[2] = {
		{
			.buf = (uint8_t *)write_buf,
			.len = write_len,
			.flags = I3C_MSG_WRITE | I3C_MSG_RESTART,
		},
		{
			.buf = read_buf,
			.len = read_len,
			.flags = I3C_MSG_READ | I3C_MSG_STOP,
		},
	};

	return i3c_transfer(desc, msgs, 2);
}

/* Register operations */
int i3c_reg_read(const struct device *ctrl, uint8_t da, uint8_t reg,
		 uint8_t *buf, size_t len)
{
	return i3c_transfer_write_read(ctrl, da, &reg, 1, buf, len);
}

int i3c_reg_write(const struct device *ctrl, uint8_t da, uint8_t reg,
		  const uint8_t *buf, size_t len)
{
	if (!buf || len == 0) {
		return -EINVAL;
	}

	struct i3c_device_desc *desc = i3c_find_target(ctrl, da);

	if (!desc) {
		return -ENODEV;
	}

	/* Allocate temp buffer for reg + data */
	uint8_t *tmp = k_malloc(len + 1);

	if (!tmp) {
		return -ENOMEM;
	}

	tmp[0] = reg;
	memcpy(&tmp[1], buf, len);

	struct i3c_msg msg = {
		.buf = tmp,
		.len = len + 1,
		.flags = I3C_MSG_WRITE | I3C_MSG_STOP,
	};

	int ret = i3c_transfer(desc, &msg, 1);

	k_free(tmp);

	return ret;
}

int i3c_reg8_read(const struct device *ctrl, uint8_t da,
		  uint8_t reg, uint8_t *val)
{
	if (!val) {
		return -EINVAL;
	}
	return i3c_reg_read(ctrl, da, reg, val, 1);
}

int i3c_reg8_write(const struct device *ctrl, uint8_t da,
		   uint8_t reg, uint8_t val)
{
	return i3c_reg_write(ctrl, da, reg, &val, 1);
}

/* Target helpers */
struct i3c_device_desc *i3c_find_target(const struct device *ctrl, uint8_t da)
{
	struct i3c_device_desc *desc;

	I3C_BUS_FOR_EACH_I3CDEV(ctrl, desc) {
		if (desc->dynamic_addr == da) {
			return desc;
		}
	}

	return NULL;
}

int i3c_get_target_count(const struct device *ctrl)
{
	int count = 0;
	struct i3c_device_desc *desc;

	I3C_BUS_FOR_EACH_I3CDEV(ctrl, desc) {
		count++;
	}

	return count;
}

/* Chip ID validation */
int i3c_check_chip_id(const struct device *ctrl, uint8_t da,
		      uint8_t reg, uint8_t expected)
{
	uint8_t val;
	int ret = i3c_reg8_read(ctrl, da, reg, &val);

	if (ret < 0) {
		return ret;
	}

	if (val != expected) {
		return -EIO;
	}

	return 0;
}

/* Multi-target iteration helpers */
int i3c_test_get_target_count(const struct device *ctrl)
{
	return i3c_get_target_count(ctrl);
}

int i3c_test_run_on_all_targets(const struct device *ctrl,
				int (*test_fn)(const struct device *ctrl,
					       uint8_t da))
{
	struct i3c_device_desc *desc;
	int count = 0;
	int errors = 0;

	I3C_BUS_FOR_EACH_I3CDEV(ctrl, desc) {
		if (count >= I3C_TEST_MAX_TARGETS) {
			break;
		}
		int ret = test_fn(ctrl, desc->dynamic_addr);

		if (ret < 0) {
			errors++;
		}
		count++;
	}

	return errors > 0 ? -EIO : count;
}

/* ============================================================================
 * Frequency Mode Implementation
 * ============================================================================
 * This provides runtime frequency mode testing with actual bus configuration.
 *
 * Modes:
 *   SDR:    Standard I3C mode (12.5 MHz)
 *   HDR-DDR: Double Data Rate mode (25 MHz)
 */

static const char * const mode_names[] = {
	"SDR",
#ifdef I3C_TEST_ENABLE_HDR_MODES
	"HDR-DDR",
	"HDR-TSP",
	"HDR-TSL",
#endif
#ifdef I3C_TEST_ENABLE_I2C_FM
	"I2C-FM",
	"I2C-FM+"
#endif
};

const char *i3c_test_mode_name(enum i3c_test_speed_mode mode)
{
	if (mode >= 0 && mode < I3C_TEST_NUM_MODES) {
		return mode_names[mode];
	}
	return "UNKNOWN";
}

/* Current speed mode tracking
 * Start as UNKNOWN to force initial configuration on first test run.
 * If we start as SDR, we'd skip configuration thinking it's already done.
 */
static enum i3c_test_speed_mode current_mode = I3C_TEST_NUM_MODES;
/**
 * @brief Attach a target device to the I3C controller.
 *
 * Wrapper around i3c_attach_i3c_device for test use.
 *
 * @param ctrl I3C controller device
 * @param desc Target device descriptor
 * @return 0 on success, negative errno on failure
 */
int i3c_test_attach_target(const struct device *ctrl,
			   struct i3c_device_desc *desc)
{
	int ret;

	ret = i3c_attach_i3c_device(desc);
	if (ret != 0) {
		LOG_ERR("Failed to attach target: %d", ret);
	}
	return ret;
}

/**
 * @brief Detach a target device from the I3C controller.
 *
 * Wrapper around i3c_detach_i3c_device for test use.
 *
 * @param ctrl I3C controller device
 * @param desc Target device descriptor
 * @return 0 on success, negative errno on failure
 */
int i3c_test_detach_target(const struct device *ctrl,
			   struct i3c_device_desc *desc)
{
	int ret;

	ret = i3c_detach_i3c_device(desc);
	if (ret != 0) {
		LOG_ERR("Failed to detach target: %d", ret);
	}
	return ret;
}

/**
 * @brief Perform RSTDAA + DAA sequence for test setup.
 *
 * Designed for test_before() fixture. Runs a single RSTDAA+DAA
 * sequence. Per MIPI I3C spec section 5.1.3, RSTDAA resets all
 * dynamic addresses and ENTDAA rediscovers all targets.
 *
 * @param ctrl I3C controller device
 * @return Number of assigned devices (>= 0), negative errno on failure
 */
int i3c_test_do_rstdaa_daa(const struct device *ctrl)
{
	struct i3c_device_desc *desc;
	int assigned = 0;
	int ret;

	if (!ctrl) {
		return -EINVAL;
	}

	ret = i3c_do_ccc_rstdaa(ctrl);
	if (ret < 0) {
		LOG_ERR("RSTDAA failed: %d", ret);
		return ret;
	}

	k_msleep(100);

	ret = i3c_recover_bus(ctrl);
	if (ret < 0) {
		LOG_DBG("Pre-DAA bus recovery returned: %d", ret);
	}

	k_msleep(100);

	ret = i3c_do_daa(ctrl);
	if (ret < 0) {
		LOG_ERR("DAA failed: %d", ret);
		return ret;
	}

	k_msleep(100);

	I3C_BUS_FOR_EACH_I3CDEV(ctrl, desc) {
		if (desc->dynamic_addr != 0U) {
			assigned++;
		}
	}

	LOG_INF("DAA completed, %d target(s) have dynamic addresses",
		assigned);

	return assigned;
}

/* Reset I3C bus to SDR mode - configures frequency only, NO RSTDAA+DAA */
int i3c_test_reset_to_sdr(const struct device *ctrl)
{
	struct i3c_config_controller config;
	int ret;

	/* NOTE: We do NOT call i3c_recover_bus() here because it resets
	 * the controller hardware, which clears the driver's device table.
	 * This causes a mismatch: targets retain their dynamic addresses
	 * from DAA, but the driver forgets them, leading to transfer failures.
	 */

	/* Skip reconfiguration if already in SDR mode.
	 * i3c_configure() may reset the controller hardware, which destroys
	 * driver/device state synchronization. Only configure if needed.
	 */
	if (current_mode == I3C_TEST_SDR) {
		LOG_DBG("Already in SDR mode, skipping reconfiguration");
		return 0;
	}

	/* Configure SDR mode at 12.5 MHz (matches device tree i3c-scl-hz) */
	memset(&config, 0, sizeof(config));
	config.scl.i3c = I3C_TEST_SDR_FREQ_HZ;
	/* Set open drain timing to meet I3C spec requirements */
	config.scl_od_min.high_ns = 41;
	config.scl_od_min.low_ns = 200;
	ret = i3c_configure(ctrl, I3C_CONFIG_CONTROLLER, &config);
	if (ret < 0) {
		LOG_ERR("Failed to configure SDR mode: %d", ret);
		return ret;
	}
	LOG_INF("SDR mode configured at %u Hz", I3C_TEST_SDR_FREQ_HZ);

	current_mode = I3C_TEST_SDR;
	/* Delay for bus to stabilize */
	k_msleep(100);

	return 0;
}

uint32_t i3c_test_mode_frequency(enum i3c_test_speed_mode mode)
{
	switch (mode) {
	case I3C_TEST_SDR:
		return I3C_TEST_SDR_FREQ_HZ;
#ifdef I3C_TEST_ENABLE_I2C_FM
	case I3C_TEST_I2C_FM:
		return I3C_TEST_I2C_FM_HZ;
	case I3C_TEST_I2C_FMPLUS:
		return I3C_TEST_I2C_FMPLUS_HZ;
#endif

#ifdef I3C_TEST_ENABLE_HDR_MODES
	case I3C_TEST_HDR_DDR:
	case I3C_TEST_HDR_TSP:
	case I3C_TEST_HDR_TSL:
		return I3C_TEST_HDR_FREQ_HZ;
#endif
	default:
		return 0;
	}
}

int i3c_test_set_speed_mode(const struct device *ctrl,
			    enum i3c_test_speed_mode mode)
{
	struct i3c_config_controller config;
	int ret;

	if (!ctrl || mode < 0 || mode >= I3C_TEST_NUM_MODES) {
		return -EINVAL;
	}

	/* Get current configuration */
	ret = i3c_config_get(ctrl, I3C_CONFIG_CONTROLLER, &config);
	if (ret < 0) {
		LOG_WRN("Failed to get I3C config: %d", ret);
		/* Continue with default values */
		memset(&config, 0, sizeof(config));
	}

	switch (mode) {
	case I3C_TEST_SDR:
		/* Skip reconfiguration if already in SDR mode.
		 * Repeated i3c_configure() calls may reset the controller,
		 * causing driver/device state mismatch.
		 */
		if (current_mode == I3C_TEST_SDR) {
			LOG_DBG("Already in SDR mode, skipping reconfiguration");
			return 0;
		}

		/* Configure SDR mode at 12.5 MHz (matches DT i3c-scl-hz) */
		config.scl.i3c = I3C_TEST_SDR_FREQ_HZ;
		ret = i3c_configure(ctrl, I3C_CONFIG_CONTROLLER, &config);
		if (ret < 0) {
			LOG_ERR("Failed to set SDR frequency: %d", ret);
			return ret;
		}
		current_mode = mode;
		LOG_INF("I3C bus SDR mode (%u Hz)", config.scl.i3c);
		break;

	/* HDR modes - disabled by default,
	 * enable I3C_TEST_ENABLE_HDR_MODES if supported
	 */
#ifdef I3C_TEST_ENABLE_HDR_MODES
	uint8_t hdr_mode;

	case I3C_TEST_HDR_DDR:
		hdr_mode = 0; /* ENTHDR0 = HDR-DDR */
		goto configure_hdr;
	case I3C_TEST_HDR_TSP:
		hdr_mode = 1; /* ENTHDR1 = HDR-TSP */
		goto configure_hdr;
	case I3C_TEST_HDR_TSL:
		hdr_mode = 2; /* ENTHDR2 = HDR-TSL */
		goto configure_hdr;

configure_hdr:
		/* First ensure we're in SDR mode before attempting HDR entry */
		if (current_mode != I3C_TEST_SDR) {
			i3c_test_reset_to_sdr(ctrl);
		}

		/* HDR mode requires ENTHDR CCC to enter */
		ret = i3c_do_ccc_enthdr(ctrl, hdr_mode);
		if (ret < 0) {
			LOG_WRN("Target(s) do not support %s mode: %d",
				mode_names[mode], ret);
			/* Reset to SDR - this includes DAA */
			i3c_test_reset_to_sdr(ctrl);
			return -ENOTSUP;
		}

		/* Set HDR frequency (25 MHz) after successful HDR entry */
		config.scl.i3c = I3C_TEST_HDR_FREQ_HZ;
		ret = i3c_configure(ctrl, I3C_CONFIG_CONTROLLER, &config);
		if (ret < 0) {
			LOG_DBG("HDR config returned: %d", ret);
			/* Non-fatal - driver may auto-configure for HDR */
		}
		current_mode = mode;
		LOG_INF("I3C bus configured for %s mode (%u Hz)",
			mode_names[mode], I3C_TEST_HDR_FREQ_HZ);
		break;
#endif

#ifdef I3C_TEST_ENABLE_I2C_FM
	case I3C_TEST_I2C_FM:
		/* Configure I2C Fast Mode - 400 kHz */
		config.scl.i2c = I3C_TEST_I2C_FM_HZ;
		ret = i3c_configure(ctrl, I3C_CONFIG_CONTROLLER, &config);
		if (ret < 0) {
			LOG_ERR("Failed to set I2C FM frequency: %d", ret);
			return ret;
		}
		current_mode = mode;
		LOG_INF("I3C bus I2C FM mode (%u Hz)", config.scl.i2c);
		break;

	case I3C_TEST_I2C_FMPLUS:
		/* Configure I2C Fast Mode Plus - 1 MHz */
		config.scl.i2c = I3C_TEST_I2C_FMPLUS_HZ;
		ret = i3c_configure(ctrl, I3C_CONFIG_CONTROLLER, &config);
		if (ret < 0) {
			LOG_ERR("Failed to set I2C FM+ frequency: %d", ret);
			return ret;
		}
		current_mode = mode;
		LOG_INF("I3C bus I2C FM+ mode (%u Hz)", config.scl.i2c);
		break;
#endif

	default:
		return -ENOTSUP;
	}

	return 0;
}

enum i3c_test_speed_mode i3c_test_get_speed_mode(const struct device *ctrl)
{
	ARG_UNUSED(ctrl);
	return current_mode;
}
