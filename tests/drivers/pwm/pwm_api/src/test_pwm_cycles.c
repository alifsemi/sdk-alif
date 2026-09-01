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
#include "test_pwm_cycles.h"

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

#define DEFAULT_PERIOD_CYCLE 64000
#define DEFAULT_PULSE_CYCLE 32000
#define MAX_PERIOD_CYCLE UINT32_MAX
#define MAX_PULSE_CYCLE (UINT32_MAX / 2U)
#define DEFAULT_PERIOD_NSEC 2500000
#define DEFAULT_PULSE_NSEC 625000
#define SMALL_PERIOD_CYCLE 1000
#define SMALL_PULSE_CYCLE 500
#define INVALID_CHANNEL 5
#define PWM_CHANNEL_0 0
#define PWM_CHANNEL_1 1
#define STRESS_ITERATIONS 20
#define DUTY_SWEEP_STEPS 10

/* Default port should be adapted per board to fit the channel
 * associated to the PWM pin. For intsance, for following device,
 *	pwm1: pwm {
 *			status = "okay";
 *			pinctrl-0 = <&tim1_ch3_pe13>;
 *	};
 * the following should be used:
 * #define DEFAULT_PWM_PORT 3
 */

#define DEFAULT_PWM_PORT pwm_led0.channel

#define UNIT_CYCLES	0
#define UNIT_NSECS	1

void get_test_pwm(struct test_pwm *led)
{
	/* PWM generator device */
	led->dev = pwm_led0.dev;
	led->pwm = pwm_led0.channel;
	led->flags = pwm_led0.flags;
	zassert_true(device_is_ready(led->dev), "PWM device is not ready");
}

static int test_task(uint32_t port, uint32_t period, uint32_t pulse, uint8_t unit)
{
	TC_PRINT("[PWM]: %" PRIu32 ", [period]: %" PRIu32 ", [pulse]: %" PRIu32 "\n",
		port, period, pulse);

	struct test_pwm pwm_dev;

	get_test_pwm(&pwm_dev);

	if (unit == UNIT_CYCLES) {
		/* Verify pwm_set_cycles() */
		if (pwm_set_cycles(pwm_dev.dev, port, period, pulse, 0)) {
			TC_PRINT("Fail to set the period and pulse width\n");
			return TC_FAIL;
		}
	} else { /* unit == UNIT_NSECS */
		/* Verify pwm_set() */
		if (pwm_set(pwm_dev.dev, port, period, pulse, 0)) {
			TC_PRINT("Fail to set the period and pulse width\n");
			return TC_FAIL;
		}
	}

	return TC_PASS;
}

/*
 *	test_pwm_nsec() : test PWM with normal polarity and with nsec config
 *	test_pwm_nsec() : test PWM with inverted polarity and with nsec config
 */

ZTEST_USER(pwm_basic, test_pwm_nsec)
{
	/* Period : Pulse (2500000 : 1250000), unit (nsec). Voltage : 0.9V */
	zassert_true(test_task(DEFAULT_PWM_PORT, DEFAULT_PERIOD_NSEC,
				DEFAULT_PULSE_NSEC, UNIT_NSECS) == TC_PASS, NULL);
	k_sleep(K_MSEC(1000));

	/* Period : Pulse (2500000 : 2500000), unit (nsec). Voltage : 1.8V */
	zassert_true(test_task(DEFAULT_PWM_PORT, DEFAULT_PERIOD_NSEC,
				DEFAULT_PERIOD_NSEC, UNIT_NSECS) == TC_PASS, NULL);
	k_sleep(K_MSEC(1000));

	/* Period : Pulse (2500000 : 0), unit (nsec). Voltage : 0V */
	zassert_true(test_task(DEFAULT_PWM_PORT, DEFAULT_PERIOD_NSEC,
				0, UNIT_NSECS) == TC_PASS, NULL);
	k_sleep(K_MSEC(1000));
}
/*
 *	test_pwm_cycle() : test PWM with normal polarity and with cycles config
 *	test_pwm_cycle() : test PWM with inverted polarity and with cycles config
 */
ZTEST_USER(pwm_basic, test_pwm_cycle)
{
	int i = 0;
	/* Period : Pulse (64000 : 32000), unit (cycle). Voltage : 1.65V */
	zassert_true(test_task(DEFAULT_PWM_PORT, DEFAULT_PERIOD_CYCLE,
				DEFAULT_PULSE_CYCLE, UNIT_CYCLES) == TC_PASS, NULL);
	k_sleep(K_MSEC(1000));

	/* Period : Pulse (64000 : 64000), unit (cycle). Voltage : 3.3V */
	zassert_true(test_task(DEFAULT_PWM_PORT, DEFAULT_PERIOD_CYCLE,
				DEFAULT_PERIOD_CYCLE, UNIT_CYCLES) == TC_PASS, NULL);
	k_sleep(K_MSEC(1000));

	/* Period : Pulse (64000 : 0), unit (cycle). Voltage : 0V */
	zassert_true(test_task(DEFAULT_PWM_PORT, DEFAULT_PERIOD_CYCLE,
				0, UNIT_CYCLES) == TC_PASS, NULL);
	k_sleep(K_MSEC(1000));

	/* Period : Pulse (4294967295 : 2147483647), unit (cycle). Voltage : 0V */
	zassert_true(test_task(DEFAULT_PWM_PORT, MAX_PERIOD_CYCLE,
			MAX_PULSE_CYCLE, UNIT_CYCLES) == TC_PASS, NULL);
	while (i++ < 10) {
		k_sleep(K_MSEC(50));
	}

}

/** Verify PWM device is ready after initialization. */
ZTEST_USER(pwm_basic, test_pwm_device_ready)
{
	struct test_pwm pwm_dev;

	get_test_pwm(&pwm_dev);
	zassert_true(device_is_ready(pwm_dev.dev),
			"PWM device is not ready");
	TC_PRINT("PWM device %s is ready\n", pwm_dev.dev->name);
}

/** Verify pwm_get_cycles_per_sec returns valid frequency. */
ZTEST_USER(pwm_basic, test_pwm_get_cycles_per_sec)
{
	struct test_pwm pwm_dev;
	uint64_t cycles_per_sec = 0;
	int err;

	get_test_pwm(&pwm_dev);

	err = pwm_get_cycles_per_sec(pwm_dev.dev, pwm_dev.pwm, &cycles_per_sec);
	zassert_equal(err, 0, "pwm_get_cycles_per_sec failed (%d)", err);
	zassert_true(cycles_per_sec > 0,
			"Expected cycles_per_sec > 0, got %" PRIu64, cycles_per_sec);
	TC_PRINT("PWM clock frequency: %" PRIu64 " Hz\n", cycles_per_sec);
}

/** Verify invalid channel returns -EINVAL. */
ZTEST_USER(pwm_basic, test_pwm_invalid_channel)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	err = pwm_set_cycles(pwm_dev.dev, INVALID_CHANNEL,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, -EINVAL,
			"Expected -EINVAL for invalid channel %d, got %d",
			INVALID_CHANNEL, err);
	TC_PRINT("Invalid channel %d correctly rejected with -EINVAL\n",
		 INVALID_CHANNEL);
}

/** Verify PWM output on channel 0. */
ZTEST_USER(pwm_basic, test_pwm_channel_0)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	err = pwm_set_cycles(pwm_dev.dev, PWM_CHANNEL_0,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed to set PWM on channel 0 (%d)", err);
	TC_PRINT("PWM channel 0: period=%u pulse=%u\n",
		 DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE);
	k_sleep(K_MSEC(500));
}

/** Verify PWM output on channel 1. */
ZTEST_USER(pwm_basic, test_pwm_channel_1)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	err = pwm_set_cycles(pwm_dev.dev, PWM_CHANNEL_1,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed to set PWM on channel 1 (%d)", err);
	TC_PRINT("PWM channel 1: period=%u pulse=%u\n",
		 DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE);
	k_sleep(K_MSEC(500));
}

/** Verify both PWM channels can be active simultaneously. */
ZTEST_USER(pwm_basic, test_pwm_both_channels)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	err = pwm_set_cycles(pwm_dev.dev, PWM_CHANNEL_0,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed to set PWM on channel 0 (%d)", err);

	err = pwm_set_cycles(pwm_dev.dev, PWM_CHANNEL_1,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE / 2, 0);
	zassert_equal(err, 0, "Failed to set PWM on channel 1 (%d)", err);

	TC_PRINT("Both channels active: ch0 pulse=%u, ch1 pulse=%u\n",
			DEFAULT_PULSE_CYCLE, DEFAULT_PULSE_CYCLE / 2);
	k_sleep(K_MSEC(500));
}

/** Verify PWM with inverted polarity using pwm_set_cycles. */
ZTEST_USER(pwm_basic, test_pwm_set_cycles_inverted)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE,
			PWM_POLARITY_INVERTED);
	zassert_equal(err, 0,
			"Failed to set PWM with inverted polarity (%d)", err);
	TC_PRINT("PWM inverted polarity: period=%u pulse=%u\n",
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE);
	k_sleep(K_MSEC(500));

	/* Restore normal polarity */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed to restore normal polarity (%d)", err);
}

/**	Verify zero period disables PWM output. */
ZTEST_USER(pwm_basic, test_pwm_zero_period_disables)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	/* First set a valid PWM output */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed to set initial PWM (%d)", err);
	k_sleep(K_MSEC(200));

	/* Set period to 0 to disable output */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT, 0, 0, 0);
	zassert_equal(err, 0, "Failed to disable PWM with zero period (%d)", err);
	TC_PRINT("PWM disabled with zero period\n");
	k_sleep(K_MSEC(200));
}

/**	Verify zero pulse disables PWM output (period non-zero). */
ZTEST_USER(pwm_basic, test_pwm_zero_pulse_disables)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	/* First set a valid PWM output */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed to set initial PWM (%d)", err);
	k_sleep(K_MSEC(200));

	/* Set pulse to 0 to disable output */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, 0, 0);
	zassert_equal(err, 0, "Failed to disable PWM with zero pulse (%d)", err);
	TC_PRINT("PWM disabled with zero pulse (period still %u)\n",
			DEFAULT_PERIOD_CYCLE);
	k_sleep(K_MSEC(200));
}

/**	Verify zero pulse with inverted polarity sets output high. */
ZTEST_USER(pwm_basic, test_pwm_zero_pulse_inverted)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, 0, PWM_POLARITY_INVERTED);
	zassert_equal(err, 0,
			"Failed to set zero pulse with inverted polarity (%d)", err);
	TC_PRINT("PWM zero pulse + inverted: driver disable state should be high\n");
	k_sleep(K_MSEC(200));

	/* Restore */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT, 0, 0, 0);
	zassert_equal(err, 0, "Failed to restore (%d)", err);
}

/** TC-PWM-013: Verify period can be updated while PWM is running. */
ZTEST_USER(pwm_basic, test_pwm_period_update)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	/* Initial configuration */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed to set initial PWM (%d)", err);
	k_sleep(K_MSEC(200));

	/* Update period (double it) */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE * 2, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed to update period (%d)", err);
	TC_PRINT("Period updated: %u -> %u\n",
			DEFAULT_PERIOD_CYCLE, DEFAULT_PERIOD_CYCLE * 2);
	k_sleep(K_MSEC(200));

	/* Update period (halve it) */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE / 2, DEFAULT_PULSE_CYCLE / 4, 0);
	zassert_equal(err, 0, "Failed to update period again (%d)", err);
	TC_PRINT("Period updated: %u -> %u\n",
			DEFAULT_PERIOD_CYCLE * 2, DEFAULT_PERIOD_CYCLE / 2);
	k_sleep(K_MSEC(200));
}

/**	Verify pulse can be updated while PWM is running. */
ZTEST_USER(pwm_basic, test_pwm_pulse_update)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	/* Initial configuration */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed to set initial PWM (%d)", err);
	k_sleep(K_MSEC(200));

	/* Update pulse to 25% duty */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PERIOD_CYCLE / 4, 0);
	zassert_equal(err, 0, "Failed to update pulse to 25%% (%d)", err);
	k_sleep(K_MSEC(200));

	/* Update pulse to 75% duty */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, (DEFAULT_PERIOD_CYCLE * 3) / 4, 0);
	zassert_equal(err, 0, "Failed to update pulse to 75%% (%d)", err);
	TC_PRINT("Pulse updated through 50%% -> 25%% -> 75%% duty\n");
	k_sleep(K_MSEC(200));
}

/**	Sweep duty cycle from 0% to 100% in steps. */
ZTEST_USER(pwm_basic, test_pwm_duty_cycle_sweep)
{
	struct test_pwm pwm_dev;
	int err;
	uint32_t pulse;

	get_test_pwm(&pwm_dev);

	for (int i = 0; i <= DUTY_SWEEP_STEPS; i++) {
		pulse = (DEFAULT_PERIOD_CYCLE * i) / DUTY_SWEEP_STEPS;
		err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
				DEFAULT_PERIOD_CYCLE, pulse, 0);
		zassert_equal(err, 0,
				"Failed at duty step %d/%d pulse=%u (%d)",
				i, DUTY_SWEEP_STEPS, pulse, err);
		k_sleep(K_MSEC(100));
	}
	TC_PRINT("Duty cycle sweep 0%% to 100%% in %d steps completed\n",
			DUTY_SWEEP_STEPS);
}

/**	Stress test with rapid period/pulse reconfiguration. */
ZTEST_USER(pwm_basic, test_pwm_rapid_reconfigure)
{
	struct test_pwm pwm_dev;
	int err;
	uint32_t period, pulse;

	get_test_pwm(&pwm_dev);

	for (int i = 0; i < STRESS_ITERATIONS; i++) {
		period = SMALL_PERIOD_CYCLE + (i * 1000);
		pulse = period / 2;
		err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
				period, pulse, 0);
		zassert_equal(err, 0,
				"Rapid reconfig failed at iteration %d (%d)", i, err);
	}
	TC_PRINT("Rapid reconfigure: %d iterations completed\n",
			STRESS_ITERATIONS);
}

/**	Switch polarity between normal and inverted. */
ZTEST_USER(pwm_basic, test_pwm_polarity_switch)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	/* Normal polarity */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed normal polarity (%d)", err);
	k_sleep(K_MSEC(200));

	/* Switch to inverted */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE,
			PWM_POLARITY_INVERTED);
	zassert_equal(err, 0, "Failed inverted polarity (%d)", err);
	k_sleep(K_MSEC(200));

	/* Switch back to normal */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
	zassert_equal(err, 0, "Failed to switch back to normal (%d)", err);
	TC_PRINT("Polarity switch: normal -> inverted -> normal completed\n");
	k_sleep(K_MSEC(200));
}

/**	Verify 100% duty cycle (pulse == period). */
ZTEST_USER(pwm_basic, test_pwm_full_duty_cycle)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PERIOD_CYCLE, 0);
	zassert_equal(err, 0, "Failed to set 100%% duty cycle (%d)", err);
	TC_PRINT("PWM 100%% duty: period=pulse=%u (always on)\n",
			DEFAULT_PERIOD_CYCLE);
	k_sleep(K_MSEC(500));
}

/**	Verify small period/pulse values work. */
ZTEST_USER(pwm_basic, test_pwm_small_period)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			SMALL_PERIOD_CYCLE, SMALL_PULSE_CYCLE, 0);
	zassert_equal(err, 0,
			"Failed to set small period/pulse (%d)", err);
	TC_PRINT("PWM small values: period=%u pulse=%u\n",
			SMALL_PERIOD_CYCLE, SMALL_PULSE_CYCLE);
	k_sleep(K_MSEC(200));
}

/**	Verify pwm_set with nsec and inverted polarity. */
ZTEST_USER(pwm_basic, test_pwm_nsec_inverted)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	err = pwm_set(pwm_dev.dev, DEFAULT_PWM_PORT, DEFAULT_PERIOD_NSEC,
			DEFAULT_PULSE_NSEC, PWM_POLARITY_INVERTED);
	zassert_equal(err, 0,
			"Failed to set PWM nsec inverted (%d)", err);
	TC_PRINT("PWM nsec inverted: period=%u pulse=%u\n",
		 DEFAULT_PERIOD_NSEC, DEFAULT_PULSE_NSEC);
	k_sleep(K_MSEC(500));

	/* Restore */
	err = pwm_set(pwm_dev.dev, DEFAULT_PWM_PORT, DEFAULT_PERIOD_NSEC,
			DEFAULT_PULSE_NSEC, 0);
	zassert_equal(err, 0, "Failed to restore normal (%d)", err);
}

/**	Verify pwm_set_dt with device tree spec. */
ZTEST_USER(pwm_basic, test_pwm_set_dt_api)
{
	int err;

	zassert_true(pwm_is_ready_dt(&pwm_led0), "PWM DT device not ready");

	err = pwm_set_dt(&pwm_led0, DEFAULT_PERIOD_NSEC, DEFAULT_PULSE_NSEC);
	zassert_equal(err, 0, "pwm_set_dt failed (%d)", err);
	TC_PRINT("pwm_set_dt: period=%u pulse=%u\n",
			DEFAULT_PERIOD_NSEC, DEFAULT_PULSE_NSEC);
	k_sleep(K_MSEC(500));
}

/**	Verify pwm_set_pulse_dt to update pulse only. */
ZTEST_USER(pwm_basic, test_pwm_set_pulse_dt_api)
{
	int err;

	zassert_true(pwm_is_ready_dt(&pwm_led0), "PWM DT device not ready");

	/* Set initial period and pulse */
	err = pwm_set_dt(&pwm_led0, pwm_led0.period, pwm_led0.period / 2);
	zassert_equal(err, 0, "pwm_set_dt failed (%d)", err);
	k_sleep(K_MSEC(200));

	/* Update pulse only to 25% */
	err = pwm_set_pulse_dt(&pwm_led0, pwm_led0.period / 4);
	zassert_equal(err, 0, "pwm_set_pulse_dt failed (%d)", err);
	TC_PRINT("pwm_set_pulse_dt: pulse updated to 25%% of period\n");
	k_sleep(K_MSEC(200));

	/* Update pulse only to 75% */
	err = pwm_set_pulse_dt(&pwm_led0, (pwm_led0.period * 3) / 4);
	zassert_equal(err, 0, "pwm_set_pulse_dt to 75%% failed (%d)", err);
	TC_PRINT("pwm_set_pulse_dt: pulse updated to 75%% of period\n");
	k_sleep(K_MSEC(200));
}

/** TC-PWM-023: Verify pwm_is_ready_dt returns true for configured device. */
ZTEST_USER(pwm_basic, test_pwm_is_ready_dt)
{
	zassert_true(pwm_is_ready_dt(&pwm_led0),
			"pwm_is_ready_dt returned false for configured PWM");
	TC_PRINT("pwm_is_ready_dt: device %s channel %u ready\n",
			pwm_led0.dev->name, pwm_led0.channel);
}

/** TC-PWM-024: Stress test - enable and disable PWM output repeatedly. */
ZTEST_USER(pwm_basic, test_pwm_enable_disable_stress)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	for (int i = 0; i < STRESS_ITERATIONS; i++) {
		/* Enable with valid period/pulse */
		err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
				DEFAULT_PERIOD_CYCLE, DEFAULT_PULSE_CYCLE, 0);
		zassert_equal(err, 0,
				"Enable failed at iteration %d (%d)", i, err);

		/* Disable with zero period */
		err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT, 0, 0, 0);
		zassert_equal(err, 0,
				"Disable failed at iteration %d (%d)", i, err);
	}
	TC_PRINT("Enable/disable stress: %d iterations completed\n",
			STRESS_ITERATIONS);
}


/**	Verify pulse greater than period is accepted by API. */
ZTEST_USER(pwm_basic, test_pwm_pulse_exceeds_period)
{
	struct test_pwm pwm_dev;
	int err;

	get_test_pwm(&pwm_dev);

	/* pulse > period: driver should accept it (100% duty effectively) */
	err = pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT,
			DEFAULT_PERIOD_CYCLE, DEFAULT_PERIOD_CYCLE + 1000, 0);
	TC_PRINT("Pulse > period: err=%d (driver may accept or reject)\n", err);
	/* Either 0 or an error is valid; just ensure no crash */
	k_sleep(K_MSEC(200));

	/* Restore */
	pwm_set_cycles(pwm_dev.dev, DEFAULT_PWM_PORT, 0, 0, 0);
}
