/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "i3c_target_service.h"

#include <zephyr/drivers/i3c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(i3c_target, LOG_LEVEL_INF);

/* No dynamic address: raise hot-join and wait for the controller to assign one.
 * Address already assigned: raise one target interrupt. ENEC must allow the
 * event, so a controller that has not yet enabled it is retried.
 */
static int raise_hotjoin(const struct device *dev)
{
	static int last_ret = 1;
	struct i3c_ibi req = {
		.ibi_type = I3C_IBI_HOTJOIN,
	};
	int ret = i3c_ibi_raise(dev, &req);

	if (ret == -EACCES) {
		return 0;
	}
	if (ret != last_ret) {
		if (ret == 0) {
			LOG_INF("Hot-join accepted");
		} else {
			LOG_INF("Hot-join: %d", ret);
		}
		last_ret = ret;
	}

	return ret;
}

static int signal_ready(void)
{
	static bool sent;
	uint8_t mdb = 0;
	struct i3c_config_target config = {0};
	struct i3c_ibi ibi = {
		.ibi_type = I3C_IBI_TARGET_INTR,
		.payload = &mdb,
		.payload_len = 1,
	};
	const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_i3c));
	int ret = i3c_config_get_target(dev, &config);

	if (ret != 0) {
		return ret;
	}
	if (config.dynamic_addr == 0U) {
		ret = raise_hotjoin(dev);
		if (ret != 0) {
			return ret;
		}
		ret = i3c_config_get_target(dev, &config);
		if (ret != 0 || config.dynamic_addr == 0U) {
			return 0;
		}
	}
	if (sent) {
		return 0;
	}

	ret = i3c_ibi_raise(dev, &ibi);
	if (ret == 0) {
		sent = true;
		LOG_INF("Ready IBI sent");
	}
	return ret;
}

int main(void)
{
	int ret = i3c_target_service_start();

	if (ret != 0) {
		return ret;
	}

	LOG_INF("Persistent target running");
	for (;;) {
		(void)signal_ready();
		k_sleep(K_MSEC(200));
	}
	return 0;
}
