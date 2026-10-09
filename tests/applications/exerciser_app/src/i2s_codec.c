/* SPDX-License-Identifier: Apache-2.0 */

#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/audio/codec.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include "exerciser_app.h"

LOG_MODULE_REGISTER(exerciser_codec, LOG_LEVEL_INF);

#if CODEC_NODE_OKAY
#define CODEC_NODE     DT_NODELABEL(audio_codec)
#define CODEC_I2C_BUS  DT_BUS(CODEC_NODE)
#define CODEC_I2C_ADDR DT_REG_ADDR(CODEC_NODE)

/* WM8904 registers used for debug readback. */
#define WM8904_REG_POWER_MGMT_0      (0x0C)
#define WM8904_REG_POWER_MGMT_2      (0x0E)
#define WM8904_REG_POWER_MGMT_6      (0x12)
#define WM8904_REG_ANALOG_LEFT_IN_0  (0x2C)
#define WM8904_REG_ANALOG_RIGHT_IN_0 (0x2D)
#define WM8904_REG_ANALOG_LEFT_IN_1  (0x2E)
#define WM8904_REG_ANALOG_RIGHT_IN_1 (0x2F)
#define WM8904_REG_ANALOG_OUT12_ZC   (0x3D)
#define WM8904_REG_ANALOG_HP_0       (0x5A)

#define SAMPLE_FREQUENCY CONFIG_SAMPLE_FREQ
#define SAMPLE_BIT_WIDTH 16U
#define CHANNEL_COUNT    2U
#define BLOCK_SIZE       (480U * CHANNEL_COUNT * sizeof(int16_t))
#define TIMEOUT_MS       SYS_FOREVER_MS

static const uint8_t capture_input = 2U;

static int configure_codec(const struct device *codec_dev, audio_route_t route)
{
	struct audio_codec_cfg audio_cfg = {0};
	int ret;

	audio_cfg.dai_route = route;
	audio_cfg.dai_type = AUDIO_DAI_TYPE_I2S;
	audio_cfg.dai_cfg.i2s.word_size = SAMPLE_BIT_WIDTH;
	audio_cfg.dai_cfg.i2s.channels = CHANNEL_COUNT;
	audio_cfg.dai_cfg.i2s.format = I2S_FMT_DATA_FORMAT_I2S;
	audio_cfg.dai_cfg.i2s.options = I2S_OPT_FRAME_CLK_MASTER | I2S_OPT_BIT_CLK_MASTER;
	audio_cfg.dai_cfg.i2s.frame_clk_freq = SAMPLE_FREQUENCY;
	audio_cfg.dai_cfg.i2s.mem_slab = NULL;
	audio_cfg.dai_cfg.i2s.block_size = BLOCK_SIZE;
	audio_cfg.dai_cfg.i2s.timeout = TIMEOUT_MS;

	ret = audio_codec_configure(codec_dev, &audio_cfg);
	if (ret < 0) {
		LOG_ERR("Failed to configure codec route %d: %d", route, ret);
		return ret;
	}

	audio_codec_start_output(codec_dev);

	return 0;
}

static int select_capture_input(const struct device *codec_dev, uint8_t input)
{
	int ret;

	ret = audio_codec_route_input(codec_dev, AUDIO_CHANNEL_FRONT_LEFT, input);
	if (ret < 0) {
		LOG_ERR("Failed to route WM8904 left capture input IN%u: %d", input, ret);
		return ret;
	}

	ret = audio_codec_route_input(codec_dev, AUDIO_CHANNEL_FRONT_RIGHT, input);
	if (ret < 0) {
		LOG_ERR("Failed to route WM8904 right capture input IN%u: %d", input, ret);
		return ret;
	}

	LOG_INF("WM8904 capture input set to IN%u", input);
	return 0;
}

static int read_codec_reg(const struct device *i2c_dev, uint8_t reg, uint16_t *value)
{
	uint8_t raw[2];
	int ret;

	ret = i2c_write_read(i2c_dev, CODEC_I2C_ADDR, &reg, sizeof(reg), raw, sizeof(raw));
	if (ret < 0) {
		return ret;
	}

	*value = ((uint16_t)raw[0] << 8) | raw[1];
	return 0;
}

static void dump_codec_state(const struct device *i2c_dev)
{
	uint16_t reg = 0U;

	if (read_codec_reg(i2c_dev, WM8904_REG_POWER_MGMT_0, &reg) == 0) {
		LOG_INF("WM8904 R0C POWER_MGMT_0 = 0x%04x", reg);
	}
	if (read_codec_reg(i2c_dev, WM8904_REG_POWER_MGMT_2, &reg) == 0) {
		LOG_INF("WM8904 R0E POWER_MGMT_2 = 0x%04x", reg);
	}
	if (read_codec_reg(i2c_dev, WM8904_REG_POWER_MGMT_6, &reg) == 0) {
		LOG_INF("WM8904 R12 POWER_MGMT_6 = 0x%04x", reg);
	}
	if (read_codec_reg(i2c_dev, WM8904_REG_ANALOG_LEFT_IN_0, &reg) == 0) {
		LOG_INF("WM8904 R2C ANALOG_LEFT_IN_0 = 0x%04x", reg);
	}
	if (read_codec_reg(i2c_dev, WM8904_REG_ANALOG_RIGHT_IN_0, &reg) == 0) {
		LOG_INF("WM8904 R2D ANALOG_RIGHT_IN_0 = 0x%04x", reg);
	}
	if (read_codec_reg(i2c_dev, WM8904_REG_ANALOG_LEFT_IN_1, &reg) == 0) {
		LOG_INF("WM8904 R2E ANALOG_LEFT_IN_1 = 0x%04x", reg);
	}
	if (read_codec_reg(i2c_dev, WM8904_REG_ANALOG_RIGHT_IN_1, &reg) == 0) {
		LOG_INF("WM8904 R2F ANALOG_RIGHT_IN_1 = 0x%04x", reg);
	}
	if (read_codec_reg(i2c_dev, WM8904_REG_ANALOG_OUT12_ZC, &reg) == 0) {
		LOG_INF("WM8904 R3D ANALOG_OUT12_ZC = 0x%04x", reg);
	}
	if (read_codec_reg(i2c_dev, WM8904_REG_ANALOG_HP_0, &reg) == 0) {
		LOG_INF("WM8904 R5A ANALOG_HP_0 = 0x%04x", reg);
	}
}

void codec_thread(void)
{
	const struct device *const codec_i2c_dev = DEVICE_DT_GET(CODEC_I2C_BUS);
	const struct device *const codec_dev = DEVICE_DT_GET(CODEC_NODE);
	int ret;

	LOG_INF("WM8904 analog bypass demo at %u Hz", SAMPLE_FREQUENCY);

	if (!device_is_ready(codec_dev)) {
		LOG_ERR("%s is not ready", codec_dev->name);
		return;
	}

	if (!device_is_ready(codec_i2c_dev)) {
		LOG_ERR("%s is not ready", codec_i2c_dev->name);
		return;
	}

	audio_codec_stop_output(codec_dev);

	ret = configure_codec(codec_dev, AUDIO_ROUTE_BYPASS);
	if (ret < 0) {
		return;
	}

	ret = select_capture_input(codec_dev, capture_input);
	if (ret < 0) {
		audio_codec_stop_output(codec_dev);
		return;
	}

	dump_codec_state(codec_i2c_dev);
	LOG_INF("Bypass active on IN%u", capture_input);

	while (1) {
		k_msleep(1000);
	}
}
#endif /* CODEC_NODE_OKAY */
