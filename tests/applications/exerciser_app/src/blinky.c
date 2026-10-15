#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "exerciser_app.h"

LOG_MODULE_REGISTER(exerciser_blinky, LOG_LEVEL_INF);

#if BLINKY_NODE_OKAY
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* This thread will run in a loop */
void blinky_thread(void)
{
	int ret = 0;

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED GPIO not ready");
		return;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("LED GPIO configure failed");
		return;
	}

	while (1) {
		gpio_pin_toggle_dt(&led);
		k_msleep(SLEEP_TIME_MS);
	}
}

#endif /* BLINKY_NODE_OKAY */
