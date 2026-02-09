/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef ZEPHYR_SAMPLES_SENSOR_CH201_LIB_CH201_BSP_H_
#define ZEPHYR_SAMPLES_SENSOR_CH201_LIB_CH201_BSP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#include "chirp_board_config.h"
#include "invn/soniclib/chirp_bsp.h"
#include "invn/soniclib/ch_rangefinder.h"

/* CH201 device status */
#define CH201_DEVICE_READY    (1 << 0U)

/* Reg address size in bytes */
#define CH201_REG_ADDR_SIZE   1

#define CONVERT_MS_TO_US(n)   (n * 1000U)

/* CH201 sensor configuration */
struct ch201_config {
	const struct device *i2c_dev;
	uint8_t i2c_addr;
	struct gpio_dt_spec prog_gpio;
	struct gpio_dt_spec int_gpio;
	struct gpio_dt_spec reset_gpio;
};

/* CH201 bus configuration */
struct ch201_bus_config {
	const struct device *i2c_dev;
	uint8_t dev_cnt;
	bool	connected;
	uint8_t i2c_addr[CONFIG_CH201_MAX_NUM_SENSORS];
};

/* CH201 device map */
struct ch201_dev_map {
	const struct device  *i2c_dev;
	uint8_t              i2c_addr;
	uint8_t              dev_num;
	uint8_t              bus_num;
	struct gpio_dt_spec  int_gpio;
	/* INT gpio interrupt status */
	volatile bool        gpio_int_sts;
	/* Wait mode on GPIO INT1 */
	volatile bool        int1_wait_mode;
	struct gpio_callback gpio_cb;
};

/* CH201 driver data */
struct ch201_dev_data {
	/* CH201 sensor group pointer            */
	ch_group_t               *grp_ptr;
	/* CH201 bus configuration               */
	struct ch201_bus_config  bus_cfg[CONFIG_CH201_NUM_BUSES];

	/* CH201 device map                      */
	struct ch201_dev_map     dev_map[CONFIG_CH201_MAX_NUM_SENSORS];

	/* Driver state                          */
	uint8_t                  state;
};

/**
 * @brief       Initialise the board.
 * @param[in]   grp_ptr	Pointer to the chirp group structure
 * @return      Execution status
 */
int chbsp_board_init(ch_group_t *grp_ptr);

#ifdef __cplusplus
}
#endif
#endif /* ZEPHYR_SAMPLES_SENSOR_CH201_LIB_CH201_BSP_H_ */
