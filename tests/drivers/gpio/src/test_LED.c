/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_gpio.h"

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/*
 * \brief  Function: GPIO_Configure
 *
 * \details Configure a single pin from a gpio_dt_spec and some extra flags.
 *          This is equivalent to:
 *          gpio_pin_configure(spec->port, spec->pin, spec->dt_flags | extra_flags);
 */
static int GPIO_Configure(void)
{
	int ret;

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	zassert_equal(ret, 0, "Failed to configure interrupt");
	zassert_not_equal(ret, -EIO, "Operation is not implemented"
			" by the driver.\n");
	zassert_not_equal(ret, EWOULDBLOCK, "Operation would block.\n");

	return TC_PASS;
}

/*
 * \brief  Function: LED_toggle
 *
 * \details Toggle pin level from a gpio_dt_spec.
 *          This is equivalent to:
 *          gpio_pin_toggle(spec->port, spec->pin);
 */
static int LED_toggle(void)
{
	int ret;
	int i = 0;

	GPIO_Configure();

	while (i < 5) {
		ret = gpio_pin_toggle_dt(&led);
		zassert_equal(ret, 0, "Failed to configure interrupt");
		zassert_not_equal(ret, -EIO, "Operation is not implemented"
				" by the driver.\n");
		zassert_not_equal(ret, EWOULDBLOCK, "Operation would block.\n");

		k_msleep(SLEEP_TIME_MS);
		i++;
	}

	return TC_PASS;
}

/*
 * Test Case to test GPIO Configure
 *
 */
ZTEST(gpio_LED, test_b_GPIO_Configure)
{
	zassert_equal(GPIO_Configure(), TC_PASS, NULL);
}

/*
 * Test Case to test GPIO DE-BOUNCE
 *
 */
ZTEST(gpio_LED, test_d_gpio_toggle)
{
	zassert_equal(LED_toggle(), TC_PASS, NULL);
}

