/**
 * @file
 * @brief Verify PWM can work well when configure through nsec,
 * or cycle.
 *
 * @details
 * - Test Steps
 *   -# Bind PWM_0 port 0.
 *   -# Set PWM period and pulse using pwm_set_cycles() or pwm_set().
 *   -# Use multimeter or other instruments to measure the output
 *	from PWM_OUT_0.
 * - Expected Results
 *   -# The output of PWM_OUT_0 will differ according to the value
 *	of period and pulse.
 *	Always on  ->  Period : Pulse (1 : 1)  ->  3.3V
 *	Half on  ->  Period : Pulse (2 : 1)  ->  1.65V
 *	Always off  ->  Period : Pulse (1 : 0)  ->  0V
 */

#include <zephyr/device.h>
#include <inttypes.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#if DT_NODE_HAS_STATUS(DT_ALIAS(pwm_led0), okay)
#define PWM_DEV_NODE DT_ALIAS(pwm_led0)
#else
#error "Define a PWM device"
#endif

#define DEFAULT_PERIOD_NSEC 2000000000U
#define QUARTER_VALUE_PERIOD_NSEC 500000000U
#define PULSE_HALF_OF_PERIOD_NSEC 1000000000U
#define MAX_PERIOD 4000000000U
#define MAX_PULSE 1000000000U

static int test_task(uint32_t period, uint32_t pulse)
{
	TC_PRINT("[period]: %" PRIu32 ", [pulse]: %" PRIu32 "\n", period, pulse);

	const struct pwm_dt_spec pwm_dev = PWM_DT_SPEC_GET(PWM_DEV_NODE);

	if (!pwm_is_ready_dt(&pwm_dev)) {
		TC_PRINT("PWM device is not ready\n");
		return TC_FAIL;
	}
	if (pwm_set_dt(&pwm_dev, period, pulse)) {
		TC_PRINT("Fail to set the period and pulse width\n");
		return TC_FAIL;
	}

	return TC_PASS;
}

/*
 *	generate_pwm_signals() with normal polarity
 *	generate_pwm_signals() with inverted polarity
 */
ZTEST_USER(pwm_basic_api, different_duty_cycle_pulse_test)
{
	/*	Period : Pulse (2000000000 : 500000000), unit (nsec). Voltage : 0.9V */
	zassert_true(test_task(DEFAULT_PERIOD_NSEC,
				QUARTER_VALUE_PERIOD_NSEC) == TC_PASS, NULL);
	k_sleep(K_MSEC(1000));

	/*	Period : Pulse (2000000000 : 2000000000), unit (nsec). Voltage : 1.8V */
	zassert_true(test_task(DEFAULT_PERIOD_NSEC,
				DEFAULT_PERIOD_NSEC) == TC_PASS, NULL);
	k_sleep(K_MSEC(1000));

	/*	Period : Pulse (2000000000 : 1000000000), unit (nsec). Voltage : 0.45V */
	zassert_true(test_task(DEFAULT_PERIOD_NSEC,
			PULSE_HALF_OF_PERIOD_NSEC) == TC_PASS, NULL);
	k_sleep(K_MSEC(1000));

	/*	Period : Pulse (2000000000 : 0), unit (nsec). Voltage : 0V */
	zassert_true(test_task(DEFAULT_PERIOD_NSEC,
				0) == TC_PASS, NULL);
	k_sleep(K_MSEC(1000));
}

ZTEST_USER(pwm_basic_api, z_max_period_test)
{
	int i = 0;
	/*	Period : Pulse (4000000000 : 1000000000), unit (nsec). Voltage : 0.9V */
	zassert_true(test_task(MAX_PERIOD,
			MAX_PULSE) == TC_PASS, NULL);
	while (i++ < 10) {
		k_sleep(K_MSEC(50));
	}

}

ZTEST_SUITE(pwm_basic_api, NULL, NULL, NULL, NULL, NULL);
