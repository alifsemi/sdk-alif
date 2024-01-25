/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include "wm8904.h"

LOG_MODULE_REGISTER(wm8904, CONFIG_WM8904_LOG_LEVEL);

#define DT_DRV_COMPAT cirrus_wm8904

struct wm8904_data {
};

struct wm8904_config {
	struct i2c_dt_spec i2c;
};

struct register_write {
	uint8_t addr;
	uint16_t val;
	uint32_t delay_ms;
};

const struct register_write wm8904_register_setup[] = {
	/* Set high performance bias and disable bias current generator */
	{.addr = WM8904_BIAS_CONTROL_0, .val = BIAS_CNTL_ISEL_HP_BIAS, .delay_ms = 0},

	/* Enable VMID buffer to unsued outputs, vmid reference voltage with fast startup */
	{.addr = WM8904_VMID_CONTROL_0,
	 .val = VMID_CNTL0_VMID_BUF_ENA | VMID_CNTL0_VMID_RES_FAST | VMID_CNTL0_VMID_ENA,
	 .delay_ms = 5},

	/* VMID reference voltage setup with normal operation */
	{.addr = WM8904_VMID_CONTROL_0,
	 .val = VMID_CNTL0_VMID_BUF_ENA | VMID_CNTL0_VMID_RES_NORMAL | VMID_CNTL0_VMID_ENA,
	 .delay_ms = 0},

	/* Enable bias current generator */
	{.addr = WM8904_BIAS_CONTROL_0,
	 .val = BIAS_CNTL_ISEL_HP_BIAS | BIAS_CNTL_BIAS_ENA,
	 .delay_ms = 0},

	/* Enable ADC left and right input programmable gain amplifiers */
	{.addr = WM8904_POWER_MANAGEMENT_0,
	 .val = PWR_MGMT0_INL_ENA | PWR_MGMT0_INR_ENA,
	 .delay_ms = 0},

	/* Enable left and right headphone output */
	{.addr = WM8904_POWER_MANAGEMENT_2,
	 .val = PWR_MGMT2_HPL_PGA_ENA | PWR_MGMT2_HPR_PGA_ENA,
	 .delay_ms = 0},

	/* DAC De-emphasis control set to 48Khz, Unmute, oversamping rate set to high performance to
	 * improve SNR. Slowly ramp up the volume on unmute, Slow volume ramp up rate on unmute,
	 * Stereo(default), DAC filter(Normal)
	 */
	{.addr = WM8904_DAC_DIGITAL_1,
	 .val = DAC_DG1_DEEMPH(3) | DAC_DG1_OSR128 | DAC_DG1_UNMUTE_RAMP | DAC_DG1_MUTERATE,
	 .delay_ms = 0},

	/* Input select for left/right headphone  and left/right line output mux. No bypass used */
	{.addr = WM8904_ANALOGUE_OUT12_ZC, .val = 0x0000, .delay_ms = 0},

	/* Enable charge pump digits. Adjusts output voltage to optimize power consumption */
	{.addr = WM8904_CHARGE_PUMP_0, .val = CHRG_PMP_CP_ENA, .delay_ms = 0},

	/* Enable dynamic chage pump power based on real time audio level */
	{.addr = WM8904_CHARGE_PUMP_0, .val = CLS_W0_CP_DYN_PWR, .delay_ms = 0},

	/* Configure FLL to provide system clock.
	 * System clock requirements, assuming stereo DAC/ADC operation:
	 * - Must be >= 3 MHz
	 * - Must be >=256 * Fs
	 *
	 * For a frequency of 48 kHz, the system clock must be >= 12.288 MHz.
	 *
	 * The system clock must also be an integer multiple of the sample rate.
	 *
	 * In this case we can choose exactly 12.288 MHz
	 *
	 * With 2-channel 16-bit audio at 48 kHz, MCLK is set to 1.536 MHz which is too slow to be
	 * used directly, so the FLL (frequency locked loop) must be used as the system clock
	 * source.
	 *
	 * To set up the FLL, we have the following equations:
	 *
	 * (1): Fout = Fvco / FLL_OUTDIV
	 * (2): Fvco = Fref * N.K * FLL_FRATIO
	 *
	 * Fvco must be in the range 90 - 100 MHz
	 * N.K must be a fractional (not integer) value for the best performance
	 *
	 * Since we have Fref = 1.536 MHz and Fout = 12.288 MHz = 8 * Fref, we can re-arrange the
	 * equations to find N.K in terms of the other settable parameters:
	 *
	 *          Fout = (Fref * N.K * FLL_FRATIO) / FLL_OUTDIV
	 *      8 * Fref = (Fref * N.K * FLL_FRATIO) / FLL_OUTDIV
	 * (3):      N.K = (8 * FLL_OUTDIV) / FLL_FRATIO
	 *
	 * This means that for N.K to be a fractional value, (8 * FLL_OUTDIV) must *not* be
	 * divisible by FLL_FRATIO, and therefore we cannot set FLL_FRATIO to 1, 2, or 4. If we set
	 * FLL_FRATIO=8, we must choose an odd value of FLL_OUTDIV.
	 *
	 * Then we can choose a value of FLL_OUTDIV using equation (1) and the constraint that Fvco
	 * must be within 90 - 100 MHz:
	 * FLL_OUTDIV = 8  --> Fvco = 12.288 * 8 = 98.304 MHz
	 *
	 * Choosing FLL_FRATIO = 8, we can then calculate the value of N.K from equation (3):
	 * N.K = (4 * 8) / 8 = 3.0
	 *
	 * So for the register values:
	 * FLL_FRATIO = 8
	 * FLL_OUTDIV = 8
	 * N = 4
	 * K = 0.0 --> register value is 0.0 * 65536 = 0
	 */
	{.addr = WM8904_FLL_CONTROL_1, .val = 0x0000, .delay_ms = 0},
	{.addr = WM8904_FLL_CONTROL_2, .val = FLL_C2_OUTDIV(8) | FLL_C2_FRATIO_DIV8, .delay_ms = 0},
	{.addr = WM8904_FLL_CONTROL_3, .val = FLL_C3_K(0), .delay_ms = 0},
	{.addr = WM8904_FLL_CONTROL_4, .val = FLL_C4_N(4), .delay_ms = 0},
	{.addr = WM8904_FLL_CONTROL_5, .val = FLL_C5_CLK_REF_SRC_BCLK, .delay_ms = 0},
	{.addr = WM8904_FLL_CONTROL_1, .val = FLL_C1_FRACN_ENA | FLL_C1_FLL_ENA, .delay_ms = 5},

	/* Set clock rates - 48Khz, SYSCLK/fs ratio = 256 (12.244Mhz/48Khz) */
	{.addr = WM8904_CLOCK_RATES_1,
	 .val = CLK_RATE1_SYS_RATE_256 | CLK_RATE1_SAMPLE_RATE_48K,
	 .delay_ms = 0},

	/* Set SYSCLK source to FLL output, Enable system clock, DSP clock enable */
	{.addr = WM8904_CLOCK_RATES_2,
	 CLK_RTE2_SYSCLK_SRC | CLK_RTE2_CLK_SYS_ENA | CLK_RTE2_CLK_DSP_ENA,
	 .delay_ms = 0},

	/* I2S digital audio interface and 16 bit word length */
	{.addr = WM8904_AUDIO_INTERFACE_1,
	 .val = AUD_INT1_AIF_WL_16BIT | AUD_INT1_AIF_FMT_I2S,
	 .delay_ms = 0},

	/* Set up IN2L and IN2R as the ADC inputs, Single ended mode(default) */
	{.addr = WM8904_ANALOGUE_LEFT_INPUT_1, .val = ANLG_LIN1_IP_SEL_N_IN2L, .delay_ms = 0},
	{.addr = WM8904_ANALOGUE_RIGHT_INPUT_1, .val = ANLG_RIN1_IP_SEL_N_IN2R, .delay_ms = 0},

#ifdef CONFIG_WM8904_MONO_OUTPUT_MODE
	/* If mono output mode is selected, send the left input channel to the right DAC */
	{.addr = WM8904_AUDIO_INTERFACE_0, .val = AUD_INT0_AIFADCR_SRC, .delay_ms = 0},
#endif

	/* Enable DAC/ADC */
	{.addr = WM8904_POWER_MANAGEMENT_6,
	 .val = PWR_MGMT6_DACL_ENA | PWR_MGMT6_DACR_ENA | PWR_MGMT6_ADCL_ENA | PWR_MGMT6_ADCR_ENA,
	 .delay_ms = 5},

	/* Unmute analog input PGA and use 0dB default volume */
	{.addr = WM8904_ANALOGUE_LEFT_INPUT_0, .val = ANLG_LIN0_VOL(0x05), .delay_ms = 0},
	{.addr = WM8904_ANALOGUE_RIGHT_INPUT_0, .val = ANLG_RIN0_VOL(0x05), .delay_ms = 0},

	/* Enable input stage of HPOUTL and HPOUTR */
	{.addr = WM8904_ANALOGUE_HP_0, .val = ANLG_HP0_HPL_ENA | ANLG_HP0_HPR_ENA, .delay_ms = 0},

	/* Enable intermediate stage of HPOUTL and HPOUTR */
	{.addr = WM8904_ANALOGUE_HP_0,
	 .val = ANLG_HP0_HPL_ENA | ANLG_HP0_HPR_ENA | ANLG_HP0_HPL_ENA_DLY | ANLG_HP0_HPL_ENA_DLY,
	 .delay_ms = 0},

	/* Enable dc servo channels */
	{.addr = WM8904_DC_SERVO_0,
	 .val = DC_SRV0_DCS_ENA_CHAN_0 | DC_SRV0_DCS_ENA_CHAN_1 | DC_SRV0_DCS_ENA_CHAN_2 |
		DC_SRV0_DCS_ENA_CHAN_3,
	 .delay_ms = 0},

	/* Enable dc servo startup mode */
	{.addr = WM8904_DC_SERVO_1,
	 .val = DC_SRV1_DCS_TRIG_STARTUP_0 | DC_SRV1_DCS_TRIG_STARTUP_1 |
		DC_SRV1_DCS_TRIG_STARTUP_2 | DC_SRV1_DCS_TRIG_STARTUP_3,
	 .delay_ms = 100},

	/* Enable output stage of HPOUTL and HPOUTR */
	{.addr = WM8904_ANALOGUE_HP_0,
	 .val = ANLG_HP0_HPL_ENA_OUTP | ANLG_HP0_HPR_ENA_OUTP | ANLG_HP0_HPL_ENA_DLY |
		ANLG_HP0_HPR_ENA_DLY | ANLG_HP0_HPL_ENA | ANLG_HP0_HPR_ENA,
	 .delay_ms = 0},

	/* Removing the short of HPOUTL and HPOUTR */
	{.addr = WM8904_ANALOGUE_HP_0,
	 .val = ANLG_HP0_HPL_ENA_OUTP | ANLG_HP0_HPR_ENA_OUTP | ANLG_HP0_HPL_ENA_DLY |
		ANLG_HP0_HPR_ENA_DLY | ANLG_HP0_HPL_ENA | ANLG_HP0_HPR_ENA |
		ANLG_HP0_HPL_RMV_SHORT | ANLG_HP0_HPR_RMV_SHORT,
	 .delay_ms = 0},

	/* Headphone output volume control, 0dB, Update L and R simultaneously */
	{.addr = WM8904_ANALOGUE_OUT1_LEFT,
	 .val = ANLG_OUT1_HPOUTL_VU | ANLG_OUT1_HPOUTL_VOL(0x39),
	 .delay_ms = 0},
	{.addr = WM8904_ANALOGUE_OUT1_RIGHT,
	 .val = ANLG_OUT1_HPOUTR_VU | ANLG_OUT1_HPOUTR_VOL(0x39),
	 .delay_ms = 100},
};

/**
 * @brief Helper function to write a 16-bit register in an I2C device.
 */
static int i2c_reg_write_16bit_dt(const struct i2c_dt_spec *spec, uint8_t reg_addr, uint16_t data)
{
	uint8_t buf[3] = {reg_addr, data >> 8, data & 0xFF};

	return i2c_write_dt(spec, buf, 3);
}

/**
 * @brief Helper function to read a 16-bit register in an I2C device.
 */
static int i2c_reg_read_16bit_dt(const struct i2c_dt_spec *spec, uint8_t reg_addr, uint16_t *data)
{
	uint8_t buf[2];

	int ret = i2c_write_read_dt(spec, &reg_addr, 1, buf, 2);

	if (ret) {
		return ret;
	}

	*data = ((uint16_t)buf[0] << 8) | buf[1];

	return 0;
}

int wm8904_init(const struct device *dev)
{
	const struct wm8904_config *dev_cfg = dev->config;

	/* Reset device and then read ID */
	int ret = i2c_reg_write_16bit_dt(&dev_cfg->i2c, WM8904_SW_RESET_AND_ID, 0xFFFF);

	if (ret) {
		LOG_ERR("Failed to reset WM8904 device");
		return ret;
	}

	uint16_t dev_id;

	ret = i2c_reg_read_16bit_dt(&dev_cfg->i2c, WM8904_SW_RESET_AND_ID, &dev_id);
	if (ret) {
		LOG_ERR("Failed to read WM8904 device ID");
		return ret;
	}

	if (dev_id != WM8904_DEV_ID) {
		LOG_ERR("Got incorrect dev ID from WM8904 device: %x", dev_id);
		return -ENODEV;
	}

	/* Update all registers with chosen configuration */
	for (uint32_t i = 0; i < ARRAY_SIZE(wm8904_register_setup); i++) {
		const struct register_write *reg = &wm8904_register_setup[i];

		ret = i2c_reg_write_16bit_dt(&dev_cfg->i2c, reg->addr, reg->val);
		if (ret) {
			LOG_ERR("Register write to address %x failed", reg->addr);
			return ret;
		}

		if (reg->delay_ms) {
			k_msleep(reg->delay_ms);
		}
	}

	LOG_DBG("WM8904 initialisation completed");

	return 0;
}

#define WM8904_DEFINE(inst)                                                                        \
	static struct wm8904_data wm8904_data_##inst;                                              \
	static const struct wm8904_config wm8904_config_##inst = {                                 \
		.i2c = I2C_DT_SPEC_INST_GET(inst),                                                 \
	};                                                                                         \
	DEVICE_DT_INST_DEFINE(inst, wm8904_init, NULL, &wm8904_data_##inst, &wm8904_config_##inst, \
			      POST_KERNEL, CONFIG_WM8904_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(WM8904_DEFINE)
