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
#include "ch201_bsp.h"
#include <zephyr/device.h>

/* Invalid bus number */
#define CH201_INVALID_BUS 0xFFU

/* Config initialization macro */
#define CH201_CONFIG_INIT(inst)                                       \
{                                                                     \
	.i2c_dev = DEVICE_DT_GET(DT_BUS(DT_DRV_INST(inst))),          \
	.i2c_addr = DT_INST_REG_ADDR(inst),                           \
	.prog_gpio = GPIO_DT_SPEC_INST_GET(inst, prog_gpios),         \
	.int_gpio = GPIO_DT_SPEC_INST_GET(inst, int_gpios),           \
	.reset_gpio = GPIO_DT_SPEC_INST_GET(inst, reset_gpios),       \
}

#define DT_DRV_COMPAT invensense_ch201
static const struct ch201_config ch201_configs[] = {
#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
/* Create array of all sensor configs */
DT_INST_FOREACH_STATUS_OKAY(CH201_CONFIG_INIT)
#endif
};

/* Number of connect CH201 sensors */
#define NUM_CH201_SENSORS ARRAY_SIZE(ch201_configs)

/* CH201 runtime data */
struct ch201_dev_data ch201_data;

LOG_MODULE_REGISTER(ch201_tdk, CONFIG_SENSOR_LOG_LEVEL);

/**
 * @brief	CH201 INT gpio callback event.
 * @param[in]   port Interrupted port
 * @param[in]   cb   gpio callback pointer
 * @param[in]   pins Interrupted Pin indeces
 * @return	None
 */
void chbsp_gpio_handler(const struct device *port, struct gpio_callback *cb,
			uint32_t pins)
{
	ARG_UNUSED(port);
	ARG_UNUSED(pins);

	struct ch201_dev_data *data = &ch201_data;
	struct ch201_dev_map *dev = CONTAINER_OF(cb, struct ch201_dev_map, gpio_cb);

	if (dev->int1_wait_mode) {
		/* set status if in waiting mode */
		dev->gpio_int_sts = true;
	} else {
		/* callback with devicde number */
		ch_interrupt(data->grp_ptr, dev->dev_num);
	}
}

/**
 * @brief	CH201 INT line IO control.
 * @param[in]   cfg     ch201 configuration
 * @param[in]   enable  True or false
 * @return	None
 */
static void chbsp_intr_enable(const struct ch201_config *cfg, bool enable)
{
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
 * @brief	CH201 device gpio configure function.
 * @param[in]   cfg     ch201 configuration
 * @param[in]   dev     ch201 device data
 * @return	0 on success, Negative value on failure
 */
static int chbsp_gpios_configure(const struct ch201_config *cfg,
				struct ch201_dev_map *dev)
{
	int ret;

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
	gpio_init_callback(&dev->gpio_cb, chbsp_gpio_handler,
			BIT(cfg->int_gpio.pin));
	ret = gpio_add_callback(cfg->int_gpio.port, &dev->gpio_cb);
	if (ret != 0) {
		return ret;
	}
	return 0;
}

/**
 * @brief	Assert reset for all sensors (drive RESET_N low).
 * @param[in]   None
 * @return	None
 */
void chbsp_reset_assert(void)
{
	const struct ch201_config *cfg = ch201_configs;

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
	const struct ch201_config *cfg = ch201_configs;

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
	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	/* Drive the PROG pin to high */
	gpio_pin_set_dt(&ch201_configs[dev_num].prog_gpio, 1);
}

/**
 * @brief	Deassert PROG pin for specified device.
 * @param[in]   dev_ptr	Pointer to device descriptor
 * @return	None
 */
void chbsp_program_disable(ch_dev_t *dev_ptr)
{
	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	/* Drive the PROG pin to low */
	gpio_pin_set_dt(&ch201_configs[dev_num].prog_gpio, 0);
}

/**
 * @brief	Configure INT1 as output for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_set_int1_dir_out(ch_group_t *grp_ptr)
{
	ch_dev_t *dev_ptr;
	uint8_t dev_num;

	for (dev_num = 0; dev_num < ch_get_num_ports(grp_ptr); dev_num++) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, dev_num);
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
	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	/* Set the INT1 pin direction to Output */
	gpio_pin_configure_dt(&ch201_configs[dev_num].int_gpio, GPIO_OUTPUT_INACTIVE);
}

/**
 * @brief	Configure INT1 as input for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_set_int1_dir_in(ch_group_t *grp_ptr)
{
	ch_dev_t *dev_ptr;
	uint8_t dev_num;

	for (dev_num = 0; dev_num < ch_get_num_ports(grp_ptr); dev_num++) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, dev_num);
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
	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	/* Set the INT1 pin direction to Input */
	gpio_pin_configure_dt(&ch201_configs[dev_num].int_gpio, GPIO_INPUT);
}

/**
 * @brief	Drive INT1 low for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_int1_clear(ch_group_t *grp_ptr)
{
	ch_dev_t *dev_ptr;
	uint8_t dev_num;

	for (dev_num = 0; dev_num < ch_get_num_ports(grp_ptr); dev_num++) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, dev_num);
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
	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	/* Drive the INT1 pin to low */
	gpio_pin_set_dt(&ch201_configs[dev_num].int_gpio, 0);
}

/**
 * @brief	Drive INT1 high for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_int1_set(ch_group_t *grp_ptr)
{
	ch_dev_t *dev_ptr;
	uint8_t dev_num;

	for (dev_num = 0; dev_num < ch_get_num_ports(grp_ptr); dev_num++) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, dev_num);
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
	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	/* Drive the INT1 pin to high */
	gpio_pin_set_dt(&ch201_configs[dev_num].int_gpio, 1);
}

/**
 * @brief	Enable INT1 interrupt for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_int1_interrupt_enable(ch_group_t *grp_ptr)
{
	ch_dev_t *dev_ptr;
	uint8_t dev_num;

	for (dev_num = 0; dev_num < ch_get_num_ports(grp_ptr); dev_num++) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, dev_num);
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
	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	if (dev_num < CONFIG_CH201_MAX_NUM_SENSORS) {
		if (ch_sensor_is_connected(dev_ptr)) {
			chbsp_intr_enable(&ch201_configs[dev_num], true);
		}
	}
}

/**
 * @brief	Disable INT1 interrupt for a group of sensors.
 * @param[in]   grp_ptr	Pointer to group descriptor
 * @return	None
 */
void chbsp_group_int1_interrupt_disable(ch_group_t *grp_ptr)
{
	ch_dev_t *dev_ptr;
	uint8_t dev_num;

	for (dev_num = 0; dev_num < ch_get_num_ports(grp_ptr); dev_num++) {
		dev_ptr = ch_get_dev_ptr(grp_ptr, dev_num);
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
	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	if (dev_num < CONFIG_CH201_MAX_NUM_SENSORS) {
		if (ch_sensor_is_connected(dev_ptr)) {
			chbsp_intr_enable(&ch201_configs[dev_num], false);
		}
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
 * @param[in]   event_mask Each bit of the mask represents a device
 * @return	0 if event received, 1 if timeout
 */
uint8_t chbsp_event_wait(uint16_t time_out_ms, uint32_t event_mask)
{
	struct ch201_dev_map *dev;
	uint32_t time_out           = CONVERT_MS_TO_US(time_out_ms);
	int dev_num = find_lsb_set(event_mask);

	if (dev_num < 1) {
		return 1;
	}

	dev = &ch201_data.dev_map[dev_num];

	do {
		k_usleep(1);
	} while ((--time_out) && (!dev->gpio_int_sts));

	/* Disable waiting mode */
	dev->int1_wait_mode = false;

	if (!dev->gpio_int_sts) {
		return 1;
	}
	dev->gpio_int_sts = false;

	return 0;
}

/**
 * @brief	Notify an interrupt event (ISR context).
 * @param[in]   event_mask Each bit of the mask represents a device
 * @return	None
 */
void chbsp_event_notify(uint32_t event_mask)
{
	struct ch201_dev_data *data = &ch201_data;
	uint8_t dev_num;

	for (dev_num = 0; dev_num < find_msb_set(event_mask); dev_num++) {
		if (event_mask & (1 << dev_num)) {
			data->grp_ptr->io_int_callback(data->grp_ptr, dev_num,
						CH_INTERRUPT_TYPE_DATA_RDY);
		}
	}
}

/**
 * @brief	Prepare mechanism to wait for an event.
 * @param[in]   event_mask Each bit of the mask represents a device
 * @return	None
 */
void chbsp_event_wait_setup(uint32_t event_mask)
{
	struct ch201_dev_data *data = &ch201_data;
	uint8_t dev_num;

	for (dev_num = 0; dev_num < find_msb_set(event_mask); dev_num++) {
		if (event_mask & (1 << dev_num)) {
			/* Enable waiting mode */
			data->dev_map[dev_num].int1_wait_mode = true;
		}
	}
}

/**
 * @brief	Initialize host I2C hardware and software structures.
 * @param[in]   None
 * @return	0 on success, 1 on failure
 */
int chbsp_i2c_init(void)
{
	struct ch201_dev_data *data = &ch201_data;
	uint8_t iter;

	for (iter = 0; iter < CONFIG_CH201_NUM_BUSES; iter++) {
		if (!device_is_ready(data->bus_cfg[iter].i2c_dev)) {
			LOG_ERR("I2C Bus master %d init failed\n", iter);
			return 1;
		}
	}
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

	struct ch201_dev_data *data = &ch201_data;

	if (dev_num >= CONFIG_CH201_MAX_NUM_SENSORS) {
		return 1;
	}

	info_ptr->address = data->dev_map[dev_num].i2c_addr;
	info_ptr->bus_num = data->dev_map[dev_num].bus_num;
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
	struct ch201_dev_data *dev_data = &ch201_data;
	uint8_t dev_num                 = ch_get_dev_num(dev_ptr);
	uint8_t slv_addr                = ch_get_i2c_address(dev_ptr);
	struct ch201_dev_map *d_map;
	int32_t  ret;

	if (!(dev_data->state & CH201_DEVICE_READY)) {
		return 1;
	}

	if (dev_num < CONFIG_CH201_MAX_NUM_SENSORS) {
		d_map = &dev_data->dev_map[dev_num];
		ret = i2c_write(d_map->i2c_dev, data,
				num_bytes, slv_addr);
		if (ret != 0) {
			return 1;
		}
	} else {
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
	struct ch201_dev_data *dev_data = &ch201_data;
	uint8_t dev_num                 = ch_get_dev_num(dev_ptr);
	uint8_t slv_addr                = ch_get_i2c_address(dev_ptr);
	struct ch201_dev_map *d_map;
	int32_t  ret;
	uint8_t  iter;

	__ALIGNED(4) uint8_t tx_buf[num_bytes + CH201_REG_ADDR_SIZE];

	if (!(dev_data->state & CH201_DEVICE_READY)) {
		return 1;
	}

	if (dev_num < CONFIG_CH201_MAX_NUM_SENSORS) {
		d_map = &dev_data->dev_map[dev_num];
		/* Store register's address in 0th index */
		tx_buf[0] = mem_addr;

		if (num_bytes) {
			for (iter = 0U; iter < num_bytes; iter++) {
				tx_buf[iter + CH201_REG_ADDR_SIZE] = data[iter];
			}
		}

		/* Send msg to Ch201 sensor */
		ret = i2c_write(d_map->i2c_dev, tx_buf,
				(num_bytes + CH201_REG_ADDR_SIZE),
				slv_addr);
		if (ret != 0) {
			return 1;
		}
	} else {
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
	struct ch201_dev_data *dev_data = &ch201_data;
	uint8_t dev_num                 = ch_get_dev_num(dev_ptr);
	uint8_t slv_addr                = ch_get_i2c_address(dev_ptr);
	struct ch201_dev_map *d_map;
	int32_t  ret;

	if (!(dev_data->state & CH201_DEVICE_READY)) {
		return 1;
	}

	if (dev_num < CONFIG_CH201_MAX_NUM_SENSORS) {
		d_map = &dev_data->dev_map[dev_num];
		ret = i2c_read(d_map->i2c_dev, data, num_bytes, slv_addr);
		if (ret != 0) {
			return 1;
		}
	} else {
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
	struct ch201_dev_data *dev_data = &ch201_data;
	uint8_t dev_num                 = ch_get_dev_num(dev_ptr);
	uint8_t slv_addr                = ch_get_i2c_address(dev_ptr);
	struct ch201_dev_map *d_map;
	int32_t  ret;

	if (!(dev_data->state & CH201_DEVICE_READY)) {
		return 1;
	}

	if (dev_num < CONFIG_CH201_MAX_NUM_SENSORS) {
		d_map = &dev_data->dev_map[dev_num];
		/* Receive data from slave */
		ret = i2c_write_read(d_map->i2c_dev, slv_addr, &mem_addr,
				CH201_REG_ADDR_SIZE, data, num_bytes);
		if (ret != 0) {
			return 1;
		}
	} else {
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
	struct ch201_dev_data *data = &ch201_data;
	uint8_t dev_num = ch_get_dev_num(dev_ptr);

	if (dev_num >= CONFIG_CH201_MAX_NUM_SENSORS) {
		return;
	}

	/* There is no direct bus reset feature available.
	 * Call bus clear feature that internally
	 * calls bus reset if cannot be recovered
	 */
	i2c_recover_bus(data->dev_map[dev_num].i2c_dev);
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

/**
 * @brief	Configures the bus ch201 device hanging on
 * @param[in]	cfg  ch201 device configuration
 * @param[in]	data ch201 devices data
 * @return      Bus number of the device
 */
static uint8_t chbsp_bus_config(const struct ch201_config *cfg, struct ch201_dev_data *data)
{
	uint8_t iter;
	uint8_t bus_num;
	bool available_bus;
	struct ch201_bus_config *b_cfg;

	bus_num = CH201_INVALID_BUS;
	available_bus = false;

	for (iter = 0; iter < CONFIG_CH201_NUM_BUSES; iter++) {
		b_cfg = &data->bus_cfg[iter];
		if (cfg->i2c_dev == b_cfg->i2c_dev) {
			bus_num = iter;
			b_cfg->i2c_addr[b_cfg->dev_cnt++] = cfg->i2c_addr;
			available_bus = true;
		}
	}

	if (!available_bus) {
		for (iter = 0; iter < CONFIG_CH201_NUM_BUSES; iter++) {
			b_cfg = &data->bus_cfg[iter];
			if (!b_cfg->connected) {
				b_cfg->i2c_dev = cfg->i2c_dev;
				bus_num = iter;
				b_cfg->i2c_addr[b_cfg->dev_cnt++] = cfg->i2c_addr;
				b_cfg->connected = true;
			}
		}
	}
	return bus_num;
}

/**
 * @brief       Initialise the board.
 * @param[in]   grp_ptr	Pointer to the chirp group structure
 * @return      Execution status
 */
int chbsp_board_init(ch_group_t *grp_ptr)
{
	const struct ch201_config *cfg;
	struct ch201_dev_data *data = &ch201_data;
	uint8_t bus_num;
	int ret;
	uint8_t dev_num;

	if (!NUM_CH201_SENSORS) {
		LOG_ERR("No CH201 device connected on the bus\n");
		return -ENODEV;
	}

	for (dev_num = 0; dev_num < NUM_CH201_SENSORS; dev_num++) {
		cfg = &ch201_configs[dev_num];

		/* Configure the ch201 device bus */
		bus_num = chbsp_bus_config(cfg, data);
		if (bus_num != CH201_INVALID_BUS) {
			/* Store the device info to runtime data */
			data->dev_map[dev_num].i2c_dev = cfg->i2c_dev;
			data->dev_map[dev_num].i2c_addr = cfg->i2c_addr;
			data->dev_map[dev_num].bus_num = bus_num;
			data->dev_map[dev_num].dev_num = dev_num;
			data->dev_map[dev_num].int_gpio = cfg->int_gpio;

			/* Initialize gpios of all CH201 devices */
			ret = chbsp_gpios_configure(cfg, &data->dev_map[dev_num]);
			if (ret < 0) {
				LOG_ERR("CH201 device %d gpio init failed\n", cfg->i2c_addr);
				return -ENODEV;
			}
		} else {
			LOG_ERR("Invalid CH201 Bus number\n");
			return -ENODEV;
		}
	}

	/* Set CH201 devices and buses information
	 * required by the library
	 */
	grp_ptr->num_ports = NUM_CH201_SENSORS;
	grp_ptr->num_buses = CONFIG_CH201_NUM_BUSES;
	grp_ptr->rtc_cal_pulse_ms = CONFIG_CH201_RTC_CAL_PULSE_MS;

	data->grp_ptr = grp_ptr;
	/* Set device status as ready */
	data->state = CH201_DEVICE_READY;

	return 0;
}
