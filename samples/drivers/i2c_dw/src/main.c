/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

#define SLV_I2C_ADDR	0x50
#define I2C_MASTER	DT_ALIAS(master_i2c)
#define I2C_SLAVE	DT_ALIAS(slave_i2c)

/* Function will send a set of predefined data
 * 0xAA 0XAB 0xAC 0xAD from i2c master to i2c Slave
 * also the master will read one byte data 0x60 from the slave.
 * Predefined data values are randomly chosen, just to verify
 * write and read capabilities of the i2c instances.
 * Function will continuously print the write and read datas.
 */
int start_i2c_transfer(void)
{
	uint8_t tx_data[4];
	uint8_t rx_data[1];
	int ret = 0;
	struct i2c_msg msgs[2];

	const struct device *const i2c_master_dev = DEVICE_DT_GET(I2C_MASTER);

	ret = device_is_ready(i2c_master_dev);
	if (!ret) {
		printk("i2c: Master Device is not ready.\n");
		return ret;
	}
	tx_data[0] = 0xAA;
	tx_data[1] = 0xAB;
	tx_data[2] = 0xAC;
	tx_data[3] = 0xAD;

	/* Setup i2c messages
	 * Data to be written, and STOP after this.
	 */
	msgs[0].buf = tx_data;
	msgs[0].len = 4U;
	msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

	/* Setup i2c receive buffers
	 * Data to be received to this buffer
	 */
	msgs[1].buf = rx_data;
	msgs[1].len = 1U;
	msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;
	while (1) {

		/* Writing data from Master to Slave */
		ret = i2c_transfer(i2c_master_dev, &msgs[0], 1, SLV_I2C_ADDR);
		if (ret) {
			printk("error on transfer with : %d\n", ret);
			return ret;
		}
		printk("Master wrote 0x%x 0x%x 0x%x 0x%x to slave\n",
			tx_data[0], tx_data[1], tx_data[2], tx_data[3]);

		/* Reading data from Slave by master */
		ret = i2c_transfer(i2c_master_dev, &msgs[1], 1, SLV_I2C_ADDR);
		if (ret) {
			printk("error on Reception with : %d\n", ret);
			return ret;
		}
		printk("Master received Data 0x%x From Slave\n",
			msgs[1].buf[0]);
	}
}

/* Slave call back function */
int i2c_target_write_requested_cb(struct i2c_target_config *config)
{
	return 0;
}
int i2c_target_read_requested_cb(struct i2c_target_config *config, uint8_t *val)
{
	*val = 0x60;
	printk("Read requested from Master and send 0x%x from slave\n", *val);
	return 0;
}
int i2c_target_write_received_cb(struct i2c_target_config *config, uint8_t val)
{
	printk("Received a byte in slave : 0x%x\n", val);
	return 0;
}
int i2c_target_read_processed_cb(struct i2c_target_config *config, uint8_t *val)
{
	printk("Read processed_cb called\n");
	return 0;
}

/* Registering the i2c instance as slave
 * passing the i2c_target_config structure with call back apis
 */
void register_slave_i2c(struct i2c_target_config *cfg)
{
	int ret = 0;
	const struct device *const i2c_slave_dev = DEVICE_DT_GET(I2C_SLAVE);

	if (!device_is_ready(i2c_slave_dev)) {
		printk("i2c: Slave Device is not ready.\n");
		return;
	}
	cfg->flags = 0;
	cfg->address = SLV_I2C_ADDR;
	ret = i2c_target_register(i2c_slave_dev, cfg);
}

int main(void)
{
	int ret = 0;
	struct i2c_target_callbacks i2c_t_cb = {
		.write_requested = &i2c_target_write_requested_cb,
		.read_requested = &i2c_target_read_requested_cb,
		.write_received = &i2c_target_write_received_cb,
		.read_processed = &i2c_target_read_processed_cb,
	};
	struct i2c_target_config tcfg = {
		.callbacks = &i2c_t_cb
	};
	register_slave_i2c(&tcfg);
	ret = start_i2c_transfer();
	if (ret) {
		printk("Transmission stopped due to error in transfer\n");
	}
	return 0;
}
