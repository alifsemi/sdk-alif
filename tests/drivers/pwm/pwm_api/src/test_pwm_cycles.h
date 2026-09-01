#ifndef TEST_PWM_CYCLES_H_
#define TEST_PWM_CYCLES_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/ztest.h>

struct test_pwm {
	const struct device *dev;
	uint32_t pwm;
	pwm_flags_t flags;
};

#endif /* TEST_PWM_CYCLES_H_ */
