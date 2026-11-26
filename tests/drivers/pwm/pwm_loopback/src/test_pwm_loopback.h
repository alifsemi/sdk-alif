#ifndef __TEST_PWM_LOOPBACK_H__
#define __TEST_PWM_LOOPBACK_H__

#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/ztest.h>

struct test_pwm {
	const struct device *dev;
	uint32_t pwm;
	pwm_flags_t flags;
};

struct test_pwm_callback_data {
	uint32_t *buffer;
	size_t buffer_len;
	size_t count;
	int status;
	struct k_sem sem;
	bool pulse_capture;
};

void get_test_pwms(struct test_pwm *out, struct test_pwm *in);

#endif /* __TEST_PWM_LOOPBACK_H__ */
