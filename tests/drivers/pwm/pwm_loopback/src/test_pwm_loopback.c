/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <zephyr/kernel.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/ztest.h>

#include "test_pwm_loopback.h"

#define TEST_PWM_PERIOD_NSEC	100000000
#define TEST_PWM_PULSE_NSEC		15000000
#define TEST_PWM_PERIOD_USEC	100000
#define TEST_PWM_PULSE_USEC		75000

enum test_pwm_unit {
	TEST_PWM_UNIT_NSEC,
	TEST_PWM_UNIT_USEC,
};

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led6));
static const struct pwm_dt_spec pwm_led1 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led7));


void get_test_pwms(struct test_pwm *out, struct test_pwm *in)
{
	/* PWM generator device (output) */
	out->dev = pwm_led0.dev;
	out->pwm = pwm_led0.channel;
	out->flags = pwm_led0.flags;
	zassert_true(device_is_ready(out->dev), "pwm loopback output device is not ready");

	/* PWM capture device (input) */
	in->dev = pwm_led1.dev;
	in->pwm = pwm_led1.channel;
	in->flags = pwm_led1.flags;
	zassert_true(device_is_ready(in->dev), "pwm loopback input device is not ready");
}

static void test_capture(uint32_t period, uint32_t pulse, enum test_pwm_unit unit,
		pwm_flags_t flags)
{
	struct test_pwm in;
	struct test_pwm out;
	uint64_t period_capture = 0;
	uint64_t pulse_capture = 0;
	int err = 0;

	get_test_pwms(&out, &in);

	pwm_flags_t out_flags = (out.flags & ~PWM_POLARITY_MASK) |
				(flags & PWM_POLARITY_MASK);

	switch (unit) {
	case TEST_PWM_UNIT_NSEC:
		TC_PRINT("Testing PWM capture @ %u/%u nsec\n",
				pulse, period);
		err = pwm_set(out.dev, out.pwm, period, pulse, out_flags);
		break;

	case TEST_PWM_UNIT_USEC:
		TC_PRINT("Testing PWM capture @ %u/%u usec\n",
				pulse, period);
		err = pwm_set(out.dev, out.pwm, PWM_USEC(period),
		PWM_USEC(pulse), out_flags);
		break;

	default:
		TC_PRINT("Unsupported test unit");
		ztest_test_fail();
	}

	zassert_equal(err, 0, "failed to set pwm output (err %d)", err);

	switch (unit) {
	case TEST_PWM_UNIT_NSEC:
		err = pwm_capture_nsec(in.dev, in.pwm, flags, &period_capture,
				&pulse_capture, K_NSEC(period * 10));
		break;

	case TEST_PWM_UNIT_USEC:
		err = pwm_capture_usec(in.dev, in.pwm, flags, &period_capture,
				&pulse_capture, K_USEC(period * 10));
		break;

	default:
		TC_PRINT("Unsupported test unit");
		ztest_test_fail();
	}

	if (err == -ENOTSUP || err == -ENOSYS) {
		TC_PRINT("capture type not supported\n");
		ztest_test_skip();
	}

	zassert_equal(err, 0, "failed to capture pwm (err %d)", err);

	if (flags & PWM_CAPTURE_TYPE_PERIOD) {
		zassert_within(period_capture, period, period / 100,
				"period capture off by more than 1%");
	}

	if (flags & PWM_CAPTURE_TYPE_PULSE) {
		zassert_within(pulse_capture, pulse, pulse / 100,
				"pulse capture off by more than 1%");
	}
}

/*
 *	PWM pulse capture test
 */
ZTEST_USER(pwm_loopback, test_pulse_capture)
{
	test_capture(TEST_PWM_PERIOD_NSEC, TEST_PWM_PULSE_NSEC,
			TEST_PWM_UNIT_NSEC,
			PWM_CAPTURE_TYPE_PULSE | PWM_POLARITY_NORMAL);
	test_capture(TEST_PWM_PERIOD_USEC, TEST_PWM_PULSE_USEC,
			TEST_PWM_UNIT_USEC,
			PWM_CAPTURE_TYPE_PULSE | PWM_POLARITY_NORMAL);
}

/*
 *	PWM pulse_capture_inverted test()
 */
ZTEST_USER(pwm_loopback, test_pulse_capture_inverted)
{
	test_capture(TEST_PWM_PERIOD_NSEC, TEST_PWM_PULSE_NSEC,
			TEST_PWM_UNIT_NSEC,
			PWM_CAPTURE_TYPE_PULSE | PWM_POLARITY_INVERTED);
	test_capture(TEST_PWM_PERIOD_USEC, TEST_PWM_PULSE_USEC,
			TEST_PWM_UNIT_USEC,
			PWM_CAPTURE_TYPE_PULSE | PWM_POLARITY_INVERTED);
}

/*
 *	PWM period_capture test()
 */
ZTEST_USER(pwm_loopback, test_period_capture)
{
	test_capture(TEST_PWM_PERIOD_NSEC, TEST_PWM_PULSE_NSEC,
			TEST_PWM_UNIT_NSEC,
			PWM_CAPTURE_TYPE_PERIOD | PWM_POLARITY_NORMAL);
	test_capture(TEST_PWM_PERIOD_USEC, TEST_PWM_PULSE_USEC,
			TEST_PWM_UNIT_USEC,
			PWM_CAPTURE_TYPE_PERIOD | PWM_POLARITY_NORMAL);
}

/*
 *	PWM period_capture_inverted test()
 */
ZTEST_USER(pwm_loopback, test_period_capture_inverted)
{
	test_capture(TEST_PWM_PERIOD_NSEC, TEST_PWM_PULSE_NSEC,
			TEST_PWM_UNIT_NSEC,
			PWM_CAPTURE_TYPE_PERIOD | PWM_POLARITY_INVERTED);
	test_capture(TEST_PWM_PERIOD_USEC, TEST_PWM_PULSE_USEC,
			TEST_PWM_UNIT_USEC,
			PWM_CAPTURE_TYPE_PERIOD | PWM_POLARITY_INVERTED);
}

/*
 *	PWM period and pulse capture test()
 */
ZTEST_USER(pwm_loopback, test_pulse_and_period_capture)
{
	test_capture(TEST_PWM_PERIOD_NSEC, TEST_PWM_PULSE_NSEC,
			TEST_PWM_UNIT_NSEC,
			PWM_CAPTURE_TYPE_BOTH | PWM_POLARITY_NORMAL);
	test_capture(TEST_PWM_PERIOD_USEC, TEST_PWM_PULSE_USEC,
			TEST_PWM_UNIT_USEC,
			PWM_CAPTURE_TYPE_BOTH | PWM_POLARITY_NORMAL);
}

/*
 *	test_capture_timeout()
 */
ZTEST_USER(pwm_loopback, test_capture_timeout)
{
	struct test_pwm in;
	struct test_pwm out;
	uint32_t period;
	uint32_t pulse;
	int err;

	get_test_pwms(&out, &in);

	err = pwm_set_cycles(out.dev, out.pwm, 100, 0, out.flags);
	zassert_equal(err, 0, "failed to set pwm output (err %d)", err);

	err = pwm_capture_cycles(in.dev, in.pwm, PWM_CAPTURE_TYPE_PULSE,
				 &period, &pulse, K_MSEC(1000));
	if (err == -ENOTSUP || err == -ENOSYS) {
		TC_PRINT("Pulse capture not supported, "
			 "trying period capture\n");
		err = pwm_capture_cycles(in.dev, in.pwm,
					 PWM_CAPTURE_TYPE_PERIOD, &period,
					 &pulse, K_MSEC(1000));
	}
	if (err == -ENOTSUP || err == -ENOSYS) {
		ztest_test_skip();
	}

	zassert_equal(err, -EAGAIN, "pwm capture did not timeout (err %d)",
			err);
}

static void continuous_capture_callback(const struct device *dev,
					uint32_t pwm,
					uint32_t period_cycles,
					uint32_t pulse_cycles,
					int status,
					void *user_data)
{
	struct test_pwm_callback_data *data = user_data;

	if (status != 0) {
		data->status = status;
		k_sem_give(&data->sem);
		return;
	}

	if (data->count >= data->buffer_len) {
		return;
	}

	data->buffer[data->count++] = data->pulse_capture ? pulse_cycles : period_cycles;
	if (data->count >= data->buffer_len) {
		data->status = 0;
		k_sem_give(&data->sem);
	}
}

/*
 *	test_continuous_capture()
 */
ZTEST(pwm_loopback, test_continuous_capture)
{
	struct test_pwm in;
	struct test_pwm out;
	uint32_t buffer[10];
	struct test_pwm_callback_data data = {
		.buffer = buffer,
		.buffer_len = ARRAY_SIZE(buffer),
		.count = 0,
		.pulse_capture = true,
	};
	uint64_t usec = 0;
	int err;
	size_t i;

	get_test_pwms(&out, &in);

	memset(buffer, 0, sizeof(buffer));
	k_sem_init(&data.sem, 0, 1);

	err = pwm_set(out.dev, out.pwm, PWM_USEC(TEST_PWM_PERIOD_USEC),
			PWM_USEC(TEST_PWM_PULSE_USEC), out.flags);
	zassert_equal(err, 0, "failed to set pwm output (err %d)", err);

	err = pwm_configure_capture(in.dev, in.pwm,
			in.flags |
			PWM_CAPTURE_MODE_CONTINUOUS |
			PWM_CAPTURE_TYPE_PULSE,
			continuous_capture_callback, &data);
	if (err == -ENOTSUP || err == -ENOSYS) {
		TC_PRINT("Pulse capture not supported, "
				"trying period capture\n");
		err = pwm_configure_capture(in.dev, in.pwm,
				in.flags |
				PWM_CAPTURE_MODE_CONTINUOUS |
				PWM_CAPTURE_TYPE_PERIOD,
				continuous_capture_callback, &data);
		if (err == 0) {
			data.pulse_capture = false;
		}
	}
	if (err == -ENOTSUP || err == -ENOSYS) {
		ztest_test_skip();
	}
	zassert_equal(err, 0, "failed to configure pwm input (err %d)",
			err);

	err = pwm_enable_capture(in.dev, in.pwm);
	zassert_equal(err, 0, "failed to enable pwm capture (err %d)", err);

	err = k_sem_take(&data.sem, K_USEC(TEST_PWM_PERIOD_USEC * data.buffer_len * 10));
	zassert_equal(err, 0, "pwm capture timed out (err %d)", err);
	zassert_equal(data.status, 0, "pwm capture failed (status %d)", data.status);

	err = pwm_disable_capture(in.dev, in.pwm);
	zassert_equal(err, 0, "failed to disable pwm capture (err %d)", err);

	for (i = 0; i < data.buffer_len; i++) {
		err = pwm_cycles_to_usec(in.dev, in.pwm, buffer[i], &usec);
		zassert_equal(err, 0, "failed to calculate usec (err %d)", err);

		if (data.pulse_capture) {
			zassert_within(usec, TEST_PWM_PULSE_USEC, TEST_PWM_PULSE_USEC / 100,
					"pulse capture off by more than 1%");
		} else {
			zassert_within(usec, TEST_PWM_PERIOD_USEC, TEST_PWM_PERIOD_USEC / 100,
					"period capture off by more than 1%");
		}
	}
}

/*
 *	test_capture_busy()
 */
ZTEST(pwm_loopback, test_capture_busy)
{
	struct test_pwm in;
	struct test_pwm out;
	uint32_t buffer[10];
	struct test_pwm_callback_data data = {
		.buffer = buffer,
		.buffer_len = ARRAY_SIZE(buffer),
		.count = 0,
		.pulse_capture = true,
	};
	pwm_flags_t flags = PWM_CAPTURE_MODE_SINGLE |
		PWM_CAPTURE_TYPE_PULSE;
	int err;

	get_test_pwms(&out, &in);

	memset(buffer, 0, sizeof(buffer));
	k_sem_init(&data.sem, 0, 1);

	err = pwm_set_cycles(out.dev, out.pwm, 100, 0, out.flags);
	zassert_equal(err, 0, "failed to set pwm output (err %d)", err);

	err = pwm_configure_capture(in.dev, in.pwm, in.flags | flags,
			continuous_capture_callback, &data);
	if (err == -ENOTSUP || err == -ENOSYS) {
		TC_PRINT("Pulse capture not supported, "
				"trying period capture\n");
		flags = PWM_CAPTURE_MODE_SINGLE | PWM_CAPTURE_TYPE_PERIOD;
		err = pwm_configure_capture(in.dev, in.pwm, in.flags | flags,
				continuous_capture_callback, &data);
		if (err == 0) {
			data.pulse_capture = false;
		}
	}
	if (err == -ENOTSUP || err == -ENOSYS) {
		ztest_test_skip();
	}
	zassert_equal(err, 0, "failed to configure pwm input (err %d)",
			err);

	err = pwm_enable_capture(in.dev, in.pwm);
	zassert_equal(err, 0, "failed to enable pwm capture (err %d)", err);

	err = pwm_configure_capture(in.dev, in.pwm, in.flags | flags,
			continuous_capture_callback, &data);
	zassert_equal(err, -EBUSY, "pwm capture not busy (err %d)", err);

	err = pwm_enable_capture(in.dev, in.pwm);
	zassert_equal(err, -EBUSY, "pwm capture not busy (err %d)", err);

	err = pwm_disable_capture(in.dev, in.pwm);
	zassert_equal(err, 0, "failed to disable pwm capture (err %d)", err);
}
