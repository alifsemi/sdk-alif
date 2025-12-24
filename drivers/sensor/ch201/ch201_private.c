/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <zephyr/logging/log.h>

/* Project Includes */
#include "ch201_private.h"

LOG_MODULE_DECLARE(ch201_tdk, CONFIG_SENSOR_LOG_LEVEL);

/**
 * @brief	CH201 INT gpio callback event.
 * @param[in]   event GPIO Event
 * @return	None
 */
void chbsp_gpio_handler(const struct device *dev, struct gpio_callback *cb,
			uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(pins);

	struct ch201_data *data = CONTAINER_OF(cb, struct ch201_data, gpio_cb);

	if (data->int1_wait_mode) {
		/* set status if in waiting mode */
		data->gpio_int_sts = true;
	} else {
		/* callback with devicde number */
		ch_interrupt(data->grp_ptr, data->dev_num);
	}
}

/**
 * @brief	CH201 INT line IO control.
 * @param[in]   enable	True or false
 * @return	None
 */
static void chbsp_intr_enable(bool enable)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;

	ch201_get_cur_dev(&cur_dev);
	cfg = cur_dev->config;

	if (enable) {
		/* Enable interrupt */
		gpio_pin_interrupt_configure_dt(&cfg->int_gpio,
						GPIO_INT_EDGE_TO_ACTIVE);
	} else {
		/* Disable interrupt */
		gpio_pin_interrupt_configure_dt(&cfg->int_gpio, GPIO_INT_DISABLE);
	}
}

/**
 * @brief	GPIO setup function.
 * @param[in]   None
 * @return	0 on success, Negative value on failure
 */
int ch201_gpios_configure(const struct device *dev)
{
	int ret;

	const struct ch201_config *cfg = dev->config;
	struct ch201_data *data        = dev->data;

	if ((!gpio_is_ready_dt(&cfg->prog_gpio))   ||
	    (!gpio_is_ready_dt(&cfg->int_gpio))    ||
	    (!gpio_is_ready_dt(&cfg->reset_gpio))) {
		return -ENODEV;
	}

	/* Set Program pin's direction to output and keep it low */
	ret = gpio_pin_configure_dt(&cfg->prog_gpio, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		return ret;
	}

	/* Set Resetn pin's direction to output and keep it high */
	ret = gpio_pin_configure_dt(&cfg->reset_gpio, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&cfg->int_gpio, GPIO_INPUT);
	if (ret != 0) {
		return ret;
	}
	gpio_init_callback(&data->gpio_cb, chbsp_gpio_handler,
			BIT(cfg->int_gpio.pin));
	ret = gpio_add_callback(cfg->int_gpio.port, &data->gpio_cb);
	if (ret != 0) {
		return ret;
	}

	data->state |= CH201_GPIO_DRIVER_READY;

	return 0;
}

/**
 * @brief	Assert reset for all sensors (drive RESET_N low).
 * @param[in]   None
 * @return	None
 */
void chbsp_reset_assert(void)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;

	ch201_get_cur_dev(&cur_dev);
	cfg = cur_dev->config;

	/* Drive the RESET pin to low */
	gpio_pin_set_dt(&cfg->reset_gpio, 1);
}

/**
 * @brief	Deassert reset for all sensors (drive RESET_N high).
 * @param[in]   None
 * @return	None
 */
void chbsp_reset_release(void)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;

	ch201_get_cur_dev(&cur_dev);
	cfg = cur_dev->config;

	/* Drive the RESET pin to High */
	gpio_pin_set_dt(&cfg->reset_gpio, 0);
}

/**
 * @brief	Assert PROG pin for specified device.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @return	None
 */
void chbsp_program_enable(ch_dev_t *dev_ptr)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data	  *data;

	ch201_get_cur_dev(&cur_dev);
	cfg = cur_dev->config;
	data = cur_dev->data;

	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	if (dev_num == data->dev_num) {
		/* Drive the PROG pin to high */
		gpio_pin_set_dt(&cfg->prog_gpio, 1);
	}
}

/**
 * @brief	Deassert PROG pin for specified device.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @return	None
 */
void chbsp_program_disable(ch_dev_t *dev_ptr)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data	  *data;

	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	ch201_get_cur_dev(&cur_dev);
	cfg = cur_dev->config;
	data = cur_dev->data;

	if (dev_num == data->dev_num) {
		/* Drive the PROG pin to low */
		gpio_pin_set_dt(&cfg->prog_gpio, 0);
	}
}

/**
 * @brief	Configure INT1 as output for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_set_int1_dir_out(ch_group_t *grp_ptr)
{
	const struct device *cur_dev;
	struct ch201_data   *data;
	ch_dev_t *dev_ptr;

	ch201_get_cur_dev(&cur_dev);
	data	      = cur_dev->data;
	data->grp_ptr = grp_ptr;

	if (ch_get_num_ports(grp_ptr) == data->dev_num + 1) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, data->dev_num);
		if (ch_sensor_is_connected(dev_ptr)) {
			chbsp_set_int1_dir_out(dev_ptr);
		}
	}
}

/**
 * @brief	Configure INT1 as output for one sensor.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @return	None
 */
void chbsp_set_int1_dir_out(ch_dev_t *dev_ptr)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data	  *data;

	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	ch201_get_cur_dev(&cur_dev);
	cfg  = cur_dev->config;
	data = cur_dev->data;

	if (dev_num == data->dev_num) {
		/* Set the INT1 pin direction to Output */
		gpio_pin_configure_dt(&cfg->int_gpio, GPIO_OUTPUT_INACTIVE);
	}
}

/**
 * @brief	Configure INT1 as input for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_set_int1_dir_in(ch_group_t *grp_ptr)
{
	const struct device *cur_dev;
	struct ch201_data   *data;
	ch_dev_t *dev_ptr;

	ch201_get_cur_dev(&cur_dev);
	data = cur_dev->data;
	data->grp_ptr = grp_ptr;

	if (ch_get_num_ports(grp_ptr) == data->dev_num + 1) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, data->dev_num);
		if (ch_sensor_is_connected(dev_ptr)) {
			chbsp_set_int1_dir_in(dev_ptr);
		}
	}
}

/**
 * @brief	Configure INT1 as input for one sensor.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @return	None
 */
void chbsp_set_int1_dir_in(ch_dev_t *dev_ptr)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data         *data;

	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	ch201_get_cur_dev(&cur_dev);
	cfg = cur_dev->config;
	data = cur_dev->data;

	if (dev_num == data->dev_num) {
		/* Set the INT1 pin direction to Input */
		gpio_pin_configure_dt(&cfg->int_gpio, GPIO_INPUT);
	}
}

/**
 * @brief	Drive INT1 low for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_int1_clear(ch_group_t *grp_ptr)
{
	const struct device	  *cur_dev;
	struct ch201_data         *data;
	ch_dev_t *dev_ptr;

	ch201_get_cur_dev(&cur_dev);
	data = cur_dev->data;
	data->grp_ptr = grp_ptr;

	if (ch_get_num_ports(grp_ptr) == data->dev_num + 1) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, data->dev_num);
		if (ch_sensor_is_connected(dev_ptr)) {
			chbsp_int1_clear(dev_ptr);
		}
	}
}

/**
 * @brief	Drive INT1 low for one sensor.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @return	None
 */
void chbsp_int1_clear(ch_dev_t *dev_ptr)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data         *data;

	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	ch201_get_cur_dev(&cur_dev);
	cfg = cur_dev->config;
	data = cur_dev->data;

	if (dev_num == data->dev_num) {
		/* Drive the INT1 pin to low */
		gpio_pin_set_dt(&cfg->int_gpio, 0);
	}
}

/**
 * @brief	Drive INT1 high for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_int1_set(ch_group_t *grp_ptr)
{
	const struct device	  *cur_dev;
	struct ch201_data         *data;
	ch_dev_t *dev_ptr;

	ch201_get_cur_dev(&cur_dev);
	data = cur_dev->data;
	data->grp_ptr = grp_ptr;

	if (ch_get_num_ports(grp_ptr) == data->dev_num + 1) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, data->dev_num);
		if (ch_sensor_is_connected(dev_ptr)) {
			chbsp_int1_set(dev_ptr);
		}
	}
}

/**
 * @brief	Drive INT1 high for one sensor.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @return	None
 */
void chbsp_int1_set(ch_dev_t *dev_ptr)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data         *data;

	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	ch201_get_cur_dev(&cur_dev);
	cfg = cur_dev->config;
	data = cur_dev->data;

	if (dev_num == data->dev_num) {
		/* Drive the INT1 pin to high */
		gpio_pin_set_dt(&cfg->int_gpio, 1);
	}
}

/**
 * @brief	Enable INT1 interrupt for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_int1_interrupt_enable(ch_group_t *grp_ptr)
{
	const struct device	  *cur_dev;
	struct ch201_data         *data;
	ch_dev_t *dev_ptr;

	ch201_get_cur_dev(&cur_dev);
	data = cur_dev->data;

	if (ch_get_num_ports(grp_ptr) == data->dev_num + 1) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, data->dev_num);
		chbsp_int1_interrupt_enable(dev_ptr);
	}
}

/**
 * @brief	Enable INT1 interrupt for one sensor.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @return	None
 */
void chbsp_int1_interrupt_enable(ch_dev_t *dev_ptr)
{
	if (ch_sensor_is_connected(dev_ptr)) {
		chbsp_intr_enable(true);
	}
}

/**
 * @brief	Disable INT1 interrupt for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_int1_interrupt_disable(ch_group_t *grp_ptr)
{
	const struct device	  *cur_dev;
	struct ch201_data         *data;
	ch_dev_t *dev_ptr;

	ch201_get_cur_dev(&cur_dev);
	data = cur_dev->data;

	if (ch_get_num_ports(grp_ptr) == data->dev_num + 1) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, data->dev_num);
		chbsp_int1_interrupt_disable(dev_ptr);
	}
}

/**
 * @brief	Disable INT1 interrupt for one sensor.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @return	None
 */
void chbsp_int1_interrupt_disable(ch_dev_t *dev_ptr)
{
	if (ch_sensor_is_connected(dev_ptr)) {
		chbsp_intr_enable(false);
	}
}

/**
 * @brief	Delay for specified microseconds.
 * @param[in]   us Microseconds to delay
 * @return	None
 */

void chbsp_delay_us(uint32_t us)
{
	/* Check if we are in an Interrupt OR
	 * if the Scheduler is locked/not started
	 */
	if (k_is_in_isr() || k_is_pre_kernel()) {
		/* Busy wait is the ONLY option here */
		k_busy_wait(us);
	} else {
		/* We are in a normal thread, so sleeping is safe and efficient */
		k_usleep(us);
	}
}

/**
 * @brief	Delay for specified milliseconds.
 * @param[in]   ms Milliseconds to delay
 * @return	None
 */
void chbsp_delay_ms(uint32_t ms)
{
	/* Check if we are in an Interrupt OR
	 * if the Scheduler is locked/not started
	 */
	if (k_is_in_isr() || k_is_pre_kernel()) {
		/* Busy wait is the ONLY option here */
		k_busy_wait(CONVERT_MS_TO_US(ms));
	} else {
		/* We are in a normal thread, so sleeping is safe and efficient */
		k_msleep(ms);
	}
}

/**
 * @brief	Return a free-running counter value in milliseconds.
 * @param[in]   None
 * @return	Milliseconds tick count
 */
uint32_t chbsp_timestamp_ms(void)
{
	return k_uptime_get_32();
}

/**
 * @brief	Wait for any interrupt event or timeout.
 * @param[in]   time_out_ms Timeout in milliseconds
 * @param[in]   event_mask  Event bitmask to wait for
 * @return	0 if event received, 1 if timeout
 */
uint8_t chbsp_event_wait(uint16_t time_out_ms, uint32_t event_mask)
{
	const struct device *cur_dev;
	struct ch201_data   *data;
	uint32_t time_out = CONVERT_MS_TO_US(time_out_ms);

	ch201_get_cur_dev(&cur_dev);
	data = cur_dev->data;

	if (event_mask == (1 << CH201_I2C_DEV_NUM)) {
		do {
			k_usleep(1);
		} while ((--time_out) && (!data->gpio_int_sts));

		/* Disable waiting mode */
		data->int1_wait_mode = false;

		if (!data->gpio_int_sts) {
			return 1;
		}
		data->gpio_int_sts = false;

		return 0;
	} else {
		return 1;
	}
}

/**
 * @brief	Notify an interrupt event (ISR context).
 * @param[in]   event_mask Event bitmask corresponding to device(s)
 * @return	None
 */
void chbsp_event_notify(uint32_t event_mask)
{
	const struct device *cur_dev;
	struct ch201_data   *data;

	ch201_get_cur_dev(&cur_dev);
	data = cur_dev->data;

	if (event_mask == (1 << CH201_I2C_DEV_NUM)) {
		data->grp_ptr->io_int_callback(data->grp_ptr, data->dev_num,
						CH_INTERRUPT_TYPE_DATA_RDY);
	}
}

/**
 * @brief	Prepare mechanism to wait for an event.
 * @param[in]   event_mask Event bitmask to configure
 * @return	None
 */
void chbsp_event_wait_setup(uint32_t event_mask)
{
	const struct device *cur_dev;
	struct ch201_data   *data;

	ch201_get_cur_dev(&cur_dev);
	data = cur_dev->data;

	if (event_mask == (1 << CH201_I2C_DEV_NUM)) {
		/* Enable waiting mode */
		data->int1_wait_mode = true;
	}
}

/**
 * @brief	Initialize host I2C hardware and software structures.
 * @param[in]   None
 * @return	0 on success, 1 on failure
 */
int chbsp_i2c_init(void)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data	  *data;

	ch201_get_cur_dev(&cur_dev);
	cfg  = cur_dev->config;
	data = cur_dev->data;

	if (!i2c_is_ready_dt(&cfg->i2c)) {
		LOG_ERR("I2C init failed\n");
		return 1;
	}

	data->state |= CH201_I2C_DRIVER_READY;

	/* Set device number to zero as only one CH201 sensor on board */
	data->dev_num = CH201_I2C_DEV_NUM;

	return 0;
}

/**
 * @brief	Get I2C address/bus/flags for a device.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @param[in]   dev_num	Device number within group
 * @param[out]  info_ptr   Pointer to I2C info structure to fill
 * @return	0 on success, 1 on error
 */
uint8_t chbsp_i2c_get_info(ch_group_t *grp_ptr, uint8_t dev_num,
			ch_i2c_info_t *info_ptr)
{
	ARG_UNUSED(grp_ptr);

	if (dev_num != CH201_I2C_BUS_NUM) {
		return 1;
	}

	info_ptr->address = CH201_I2C_ADDRS;
	info_ptr->bus_num = CH201_I2C_BUS_NUM;
	info_ptr->drv_flags = 0;

	return 0;
}

/**
 * @brief	Write bytes to an I2C slave.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @param[in]   data	   Pointer to data buffer
 * @param[in]   num_bytes  Number of bytes to write
 * @return	0 on success, 1 on error or NACK
 */
int chbsp_i2c_write(ch_dev_t *dev_ptr, const uint8_t *data, uint16_t num_bytes)
{
	int32_t  ret;
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data	  *dev_data;
	uint8_t  i2c_addr = ch_get_i2c_address(dev_ptr);

	if ((i2c_addr != CH201_I2C_ADDRS) &&
		(i2c_addr != CH_I2C_ADDR_PROG)) {
		return 1;
	}

	ch201_get_cur_dev(&cur_dev);
	cfg  = cur_dev->config;
	dev_data = cur_dev->data;

	if (!(dev_data->state & CH201_I2C_DRIVER_READY)) {
		return 1;
	}

	ret = i2c_write(cfg->i2c.bus, data, num_bytes, i2c_addr);
	if (ret != 0) {
		return 1;
	}

	return 0;
}

/**
 * @brief	Write bytes to an I2C slave using memory/register addressing.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @param[in]   mem_addr   Internal memory/register address
 * @param[in]   data	   Pointer to data buffer
 * @param[in]   num_bytes  Number of bytes to write
 * @return	0 on success, 1 on error or NACK
 */
int chbsp_i2c_mem_write(ch_dev_t *dev_ptr, uint16_t mem_addr,
			uint8_t *data, uint16_t num_bytes)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data	  *dev_data;

	__ALIGNED(4) uint8_t tx_buf[num_bytes + CH201_REG_ADDR_SIZE];
	int32_t  ret;
	uint8_t  iter;

	uint8_t i2c_addr = ch_get_i2c_address(dev_ptr);

	ch201_get_cur_dev(&cur_dev);
	cfg  = cur_dev->config;
	dev_data = cur_dev->data;

	if (!(dev_data->state & CH201_I2C_DRIVER_READY)) {
		return 1;
	}

	if ((i2c_addr != CH201_I2C_ADDRS) &&
		(i2c_addr != CH_I2C_ADDR_PROG)) {
		return 1;
	}

	/* Store register's address in 0th index */
	tx_buf[0] = mem_addr;

	if (num_bytes) {
		for (iter = 0U; iter < num_bytes; iter++) {
			tx_buf[iter + CH201_REG_ADDR_SIZE] = data[iter];
		}
	}

	/* Send msg to Ch201 sensor */
	ret = i2c_write(cfg->i2c.bus, tx_buf,
			(num_bytes + CH201_REG_ADDR_SIZE), i2c_addr);
	if (ret != 0) {
		return 1;
	}

	return 0;
}

/**
 * @brief	Read bytes from an I2C slave.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @param[out]  data	   Pointer to receive buffer
 * @param[in]   num_bytes  Number of bytes to read
 * @return	0 on success, 1 on error or NACK
 */
int chbsp_i2c_read(ch_dev_t *dev_ptr, uint8_t *data, uint16_t num_bytes)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data	  *dev_data;
	int32_t  ret;
	uint8_t i2c_addr = ch_get_i2c_address(dev_ptr);

	if ((i2c_addr != CH201_I2C_ADDRS) &&
		(i2c_addr != CH_I2C_ADDR_PROG)) {
		return 1;
	}

	ch201_get_cur_dev(&cur_dev);
	cfg  = cur_dev->config;
	dev_data = cur_dev->data;

	if (!(dev_data->state & CH201_I2C_DRIVER_READY)) {
		return 1;
	}

	ret = i2c_read(cfg->i2c.bus, data, num_bytes, i2c_addr);
	if (ret != 0) {
		return 1;
	}
	return 0;
}

/**
 * @brief	Read bytes from an I2C slave using memory/register addressing.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @param[in]   mem_addr   Internal memory/register address
 * @param[out]  data	   Pointer to receive buffer
 * @param[in]   num_bytes  Number of bytes to read
 * @return	0 on success, 1 on error or NACK
 */
int chbsp_i2c_mem_read(ch_dev_t *dev_ptr, uint16_t mem_addr,
			uint8_t *data, uint16_t num_bytes)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;
	struct ch201_data	  *dev_data;
	int32_t  ret;

	uint8_t i2c_addr = ch_get_i2c_address(dev_ptr);

	ch201_get_cur_dev(&cur_dev);
	cfg  = cur_dev->config;
	dev_data = cur_dev->data;

	if (!(dev_data->state & CH201_I2C_DRIVER_READY)) {
		return 1;
	}

	if ((i2c_addr != CH201_I2C_ADDRS) &&
		(i2c_addr != CH_I2C_ADDR_PROG)) {
		return 1;
	}

	/* Receive data from slave */
	ret = i2c_write_read(cfg->i2c.bus, i2c_addr, &mem_addr,
			CH201_REG_ADDR_SIZE, data, num_bytes);
	if (ret != 0) {
		return 1;
	}
	return 0;
}

/**
 * @brief	Reset I2C bus associated with device.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @return	None
 */
void chbsp_i2c_reset(ch_dev_t *dev_ptr)
{
	const struct device	  *cur_dev;
	const struct ch201_config *cfg;

	uint8_t i2c_addr = ch_get_i2c_address(dev_ptr);

	if ((i2c_addr == CH201_I2C_ADDRS) ||
		(i2c_addr == CH_I2C_ADDR_PROG)) {
		ch201_get_cur_dev(&cur_dev);
		cfg  = cur_dev->config;
		/* There is no direct bus reset feature available.
		 * Call bus clear feature that internally
		 * calls bus reset if cannot be recovered
		 */
		i2c_recover_bus(cfg->i2c.bus);
	}
}

/**
 * @brief	Output a text string via serial interface for debugging.
 * @param[in]	str	NUL-terminated string to print
 * @return	None
 */
void chbsp_print_str(const char *str)
{
	LOG_INF("%s", str);
}

