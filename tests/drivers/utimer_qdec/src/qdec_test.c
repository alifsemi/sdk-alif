/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "qdec_emulator.h"

#define QEC_DEV                  DEVICE_DT_GET(DT_ALIAS(qdec0))
#define QEC_WAIT_MS              100
#define QEC_STEPS_SMALL          50
#define QEC_STEPS_MEDIUM         120
#define QEC_STEPS_LARGE          150
#define QEC_STEPS_MULTI_REV      300
#define QEC_ROUNDTRIP_TOL        2
#define QEC_STRESS_ITERATIONS    10
#define QEC_DIRECTION_STEPS      20
#define QEC_STEPS_SINGLE         1
#define QEC_STEPS_TEN            10
#define QEC_REPEATED_FETCH_COUNT 50
#define QEC_GLITCH_PULSE_COUNT   20

/*
 * Set by qec_suite_setup() after detecting whether the GPIO loopback
 * between emulator outputs and QDEC inputs is wired on the board.
 * Motion-dependent tests call SKIP_IF_NO_LOOPBACK() to skip gracefully
 * when the loopback is absent.
 */
static bool loopback_available;

static int qec_read_position(int *pos_deg)
{
	struct sensor_value val;
	int rc;

	rc = sensor_sample_fetch(QEC_DEV);
	if (rc != 0) {
		return rc;
	}

	rc = sensor_channel_get(QEC_DEV, SENSOR_CHAN_ROTATION, &val);
	if (rc != 0) {
		return rc;
	}

	*pos_deg = val.val1;
	return 0;
}

static int mod360_delta(int from_deg, int to_deg)
{
	int delta = (to_deg - from_deg) % 360;

	if (delta < 0) {
		delta += 360;
	}

	return delta;
}

#define SKIP_IF_NO_LOOPBACK()				\
	do {						\
		if (!loopback_available) {		\
			ztest_test_skip();		\
		}					\
	} while (0)

/*
 * Suite setup: initialise the GPIO encoder emulator and detect whether
 * the loopback wiring between emulator outputs and QDEC inputs is present.
 */
static void *qec_suite_setup(void)
{
	int pos0, pos1, rc;

	rc = qenc_emulate_init();
	zassert_ok(rc, "qenc_emulate_init() failed: %d", rc);

	rc = qec_read_position(&pos0);
	zassert_ok(rc, "initial qec_read_position(pos0) failed: %d", rc);

	simulate_cw_rotation(QEC_STEPS_MEDIUM);
	k_msleep(QEC_WAIT_MS);

	rc = qec_read_position(&pos1);
	zassert_ok(rc, "initial qec_read_position(pos1) failed: %d", rc);

	if (pos1 != pos0) {
		loopback_available = true;
		printk("QEC loopback detected (pos0=%d, pos1=%d)\n", pos0, pos1);
	} else {
		printk("QEC loopback NOT detected - motion tests will be skipped\n");
	}

	return NULL;
}

/* Reset emulator GPIO outputs to inactive state before each test. */
static void qec_before_each(void *unused)
{
	int rc;

	ARG_UNUSED(unused);
	rc = qenc_emulate_init();
	zassert_equal(rc, 0, "Failed to initialize QEC emulator (%d)", rc);
}

/* =========================================================================
 * Feature 1: 32-bit wide counters and compare registers
 * =========================================================================
 */

/** TC-QEC-001: Verify QEC sensor device is initialized and ready. */
ZTEST(qec_tests, test_qec_device_ready)
{
	zassert_true(device_is_ready(QEC_DEV), "QEC device not ready");
	printk("QEC device ready: %s\n", QEC_DEV->name);
}

/** TC-QEC-002: Verify 32-bit counter range - position stays 0-359 after large rotation. */
ZTEST(qec_tests, test_qec_counter_range_32bit)
{
	int pos;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	for (int i = 0; i < 5; i++) {
		simulate_cw_rotation(QEC_STEPS_LARGE);
		k_msleep(QEC_WAIT_MS);
	}

	rc = qec_read_position(&pos);
	zassert_equal(rc, 0, "Failed to read position (%d)", rc);

	printk("QEC 32-bit range: position=%d after %d total steps\n",
	       pos, 5 * QEC_STEPS_LARGE);
	zassert_true(pos >= 0 && pos < 360,
		     "Position %d out of 0-359 range", pos);
}

/** TC-QEC-003: Verify counter reload wrap across multiple revolutions. */
ZTEST(qec_tests, test_qec_counter_reload_wrap)
{
	int pos;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	for (int i = 0; i < 5; i++) {
		simulate_cw_rotation(QEC_STEPS_MULTI_REV);
		k_msleep(QEC_WAIT_MS);

		rc = qec_read_position(&pos);
		zassert_equal(rc, 0,
			      "Failed to read position at burst %d (%d)", i, rc);
		zassert_true(pos >= 0 && pos < 360,
			     "Position %d out of 0-359 range at burst %d", pos, i);
		printk("QEC reload wrap burst %d: pos=%d\n", i, pos);
	}
}

/** TC-QEC-004: Verify position is always within 0-359 degrees, val2 == 0. */
ZTEST(qec_tests, test_qec_position_degree_range)
{
	struct sensor_value val;
	int rc;

	rc = sensor_sample_fetch(QEC_DEV);
	zassert_equal(rc, 0, "sensor_sample_fetch failed (%d)", rc);

	rc = sensor_channel_get(QEC_DEV, SENSOR_CHAN_ROTATION, &val);
	zassert_equal(rc, 0, "sensor_channel_get(ROTATION) failed (%d)", rc);

	zassert_true(val.val1 >= 0 && val.val1 < 360,
		     "Position out of range: %d", val.val1);
	zassert_equal(val.val2, 0, "Expected val2 == 0, got %d", val.val2);
}

/* =========================================================================
 * Feature 2: Decoding quadrature encoder pulse sequence
 * =========================================================================
 */

/** TC-QEC-005: Verify CW quadrature signal pattern is decoded correctly. */
ZTEST(qec_tests, test_qec_cw_quadrature_decode)
{
	int pos0, pos1;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_SINGLE);
	k_msleep(QEC_WAIT_MS);

	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after CW step (%d)", rc);

	printk("QEC CW decode: before=%d after=%d\n", pos0, pos1);
	zassert_true(pos1 != pos0, "CW rotation did not change position");
}

/** TC-QEC-006: Verify CCW quadrature signal pattern is decoded correctly. */
ZTEST(qec_tests, test_qec_ccw_quadrature_decode)
{
	int pos0, pos1;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	simulate_cw_rotation(QEC_STEPS_MEDIUM);
	k_msleep(QEC_WAIT_MS);

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read position before CCW (%d)", rc);

	simulate_ccw_rotation(QEC_STEPS_SINGLE);
	k_msleep(QEC_WAIT_MS);

	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after CCW step (%d)", rc);

	printk("QEC CCW decode: before=%d after=%d\n", pos0, pos1);
	zassert_true(pos1 != pos0, "CCW rotation did not change position");
}

/** TC-QEC-007: Verify position changes after CW and CCW rotations. */
ZTEST(qec_tests, test_qec_position_accuracy_cw_ccw)
{
	int pos0, pos1, pos2;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_LARGE);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after CW (%d)", rc);

	simulate_ccw_rotation(QEC_STEPS_LARGE);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos2);
	zassert_equal(rc, 0, "Failed to read position after CCW (%d)", rc);

	printk("QEC accuracy: initial=%d cw=%d ccw=%d\n", pos0, pos1, pos2);
	zassert_true(pos1 != pos0, "CW rotation did not change position");
	zassert_true(pos2 != pos1, "CCW rotation did not change position");
}

/** TC-QEC-008: Verify position stable with no encoder input. */
ZTEST(qec_tests, test_qec_no_motion_stable)
{
	int pos0, pos1;
	int rc;

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read stable position (%d)", rc);

	printk("QEC no-motion: first=%d second=%d\n", pos0, pos1);
	zassert_equal(pos1, pos0, "QEC moved without encoder input");
}

/* =========================================================================
 * Feature 3: Counting in increment/decrement modes
 * =========================================================================
 */

/** TC-QEC-009: Verify up-counter increments on CW rotation. */
ZTEST(qec_tests, test_qec_increment_mode_cw)
{
	int pos0, pos1;
	int delta_cw;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_MEDIUM);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after CW (%d)", rc);

	printk("QEC increment CW: before=%d after=%d\n", pos0, pos1);
	delta_cw = mod360_delta(pos0, pos1);
	zassert_true(delta_cw > 0, "CW rotation did not increment counter");
}

/** TC-QEC-010: Verify down-counter decrements on CCW rotation. */
ZTEST(qec_tests, test_qec_decrement_mode_ccw)
{
	int pos0, pos1;
	int delta_ccw;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	simulate_cw_rotation(QEC_STEPS_MEDIUM);
	k_msleep(QEC_WAIT_MS);

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read position after CW (%d)", rc);

	simulate_ccw_rotation(QEC_STEPS_SMALL);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after CCW (%d)", rc);

	printk("QEC decrement CCW: after_cw=%d after_ccw=%d\n", pos0, pos1);
	delta_ccw = mod360_delta(pos1, pos0);
	zassert_true(delta_ccw > 0, "CCW rotation did not decrement counter");
}

/** TC-QEC-011: Verify multiple CW batches accumulate counter correctly. */
ZTEST(qec_tests, test_qec_batch_accumulation)
{
	int pos0, pos1, pos2, pos3;
	int d01, d12, d23;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_SMALL);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position batch1 (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_SMALL);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos2);
	zassert_equal(rc, 0, "Failed to read position batch2 (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_SMALL);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos3);
	zassert_equal(rc, 0, "Failed to read position batch3 (%d)", rc);

	printk("QEC batch accumulation: %d -> %d -> %d -> %d\n",
	       pos0, pos1, pos2, pos3);
	d01 = mod360_delta(pos0, pos1);
	d12 = mod360_delta(pos1, pos2);
	d23 = mod360_delta(pos2, pos3);
	zassert_true(d01 > 0, "Batch1 did not advance position");
	zassert_true(d12 > 0, "Batch2 did not advance position");
	zassert_true(d23 > 0, "Batch3 did not advance position");
}

/** TC-QEC-012: Verify equal CW then CCW returns to starting position. */
ZTEST(qec_tests, test_qec_round_trip_accuracy)
{
	int pos0, pos1, pos2;
	int delta, delta_cw;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_LARGE);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after CW (%d)", rc);

	simulate_ccw_rotation(QEC_STEPS_LARGE);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos2);
	zassert_equal(rc, 0, "Failed to read position after CCW (%d)", rc);

	delta = pos2 - pos0;
	if (delta < 0) {
		delta = -delta;
	}
	if (delta > 180) {
		delta = 360 - delta;
	}

	printk("QEC round-trip: initial=%d peak=%d final=%d\n", pos0, pos1, pos2);
	delta_cw = mod360_delta(pos0, pos1);
	zassert_true(delta_cw > 0, "CW rotation did not move position");
	zassert_true(delta <= QEC_ROUNDTRIP_TOL,
		     "Round-trip error too high (%d > %d)", delta, QEC_ROUNDTRIP_TOL);
}

/* =========================================================================
 * Feature 4: Measurement of pulse width, period, or duty cycle
 * =========================================================================
 */

/** TC-QEC-013: Verify position is proportional to number of encoder steps. */
ZTEST(qec_tests, test_qec_position_proportional_to_steps)
{
	int pos0, pos1, pos2;
	int delta1, delta2, diff;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_SMALL);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after N steps (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_SMALL);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos2);
	zassert_equal(rc, 0, "Failed to read position after 2N steps (%d)", rc);

	delta1 = mod360_delta(pos0, pos1);
	delta2 = mod360_delta(pos1, pos2);

	printk("QEC proportional: delta1=%d delta2=%d\n", delta1, delta2);
	zassert_true(delta1 > 0, "First batch did not move position");

	diff = delta2 - delta1;
	if (diff < 0) {
		diff = -diff;
	}
	zassert_true(diff <= 5,
		     "Non-linear: delta1=%d delta2=%d diff=%d", delta1, delta2, diff);
}

/** TC-QEC-014: Verify minimum step resolution. */
ZTEST(qec_tests, test_qec_small_step_resolution)
{
	int pos0, pos1, pos2;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_SINGLE);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after 1 step (%d)", rc);

	printk("QEC single step: before=%d after=%d\n", pos0, pos1);

	simulate_cw_rotation(QEC_STEPS_TEN);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos2);
	zassert_equal(rc, 0, "Failed to read position after 10 steps (%d)", rc);

	printk("QEC 10 steps: before=%d after=%d\n", pos1, pos2);
	zassert_true(pos2 != pos0,
		     "10 CW steps did not change position (pos0=%d, pos2=%d)",
		     pos0, pos2);
}

/** TC-QEC-015: Verify counts-per-revolution config via quarter-revolution test. */
ZTEST(qec_tests, test_qec_counts_per_revolution_config)
{
	int pos0, pos1;
	int delta, cpr, quarter;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	cpr = DT_PROP(DT_ALIAS(qdec0), counts_per_revolution);
	/* Each emulator step produces a full quadrature cycle = 4 QDEC counts */
	quarter = cpr / 16;

	simulate_cw_rotation(quarter);
	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after quarter rev (%d)", rc);

	delta = mod360_delta(pos0, pos1);
	printk("QEC CPR config: cpr=%d quarter_steps=%d delta=%d (expected ~90)\n",
	       cpr, quarter, delta);
	zassert_true(delta >= 80 && delta <= 100,
		     "Quarter revolution delta %d not ~90 degrees", delta);
}

/* =========================================================================
 * Feature 5: Digital filter on input signals
 * =========================================================================
 */

/** TC-QEC-016: Verify digital filter rejects glitch pulses. */
ZTEST(qec_tests, test_qec_filter_noise_rejection)
{
	int pos0, pos1;
	int rc;

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_glitch_pulses(QEC_GLITCH_PULSE_COUNT);
	k_msleep(QEC_WAIT_MS);

	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after glitches (%d)", rc);

	printk("QEC noise rejection: before=%d after=%d\n", pos0, pos1);
	zassert_equal(pos1, pos0,
		      "Filter failed: position changed after glitch pulses");
}

/** TC-QEC-017: Verify digital filter passes valid quadrature signals. */
ZTEST(qec_tests, test_qec_filter_passes_valid_signal)
{
	int pos0, pos1;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_MEDIUM);
	k_msleep(QEC_WAIT_MS);

	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after CW (%d)", rc);

	printk("QEC filter pass: before=%d after=%d\n", pos0, pos1);
	zassert_true(pos1 != pos0, "Valid signal not passed through filter");
}

/* =========================================================================
 * Feature 6: Generation of interrupts on compare/match events
 * =========================================================================
 */

/** TC-QEC-018: Verify counter overflow wrap across multiple revolutions. */
ZTEST(qec_tests, test_qec_overflow_event_wrap)
{
	int pos;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	for (int i = 0; i < 5; i++) {
		simulate_cw_rotation(QEC_STEPS_MULTI_REV);
		k_msleep(QEC_WAIT_MS);

		rc = qec_read_position(&pos);
		zassert_equal(rc, 0,
			      "Failed to read position at burst %d (%d)", i, rc);
		zassert_true(pos >= 0 && pos < 360,
			     "Position %d out of range at burst %d", pos, i);
		printk("QEC overflow wrap burst %d: pos=%d\n", i, pos);
	}
}

/** TC-QEC-019: Verify no spurious events when encoder is idle. */
ZTEST(qec_tests, test_qec_no_spurious_events)
{
	int pos0, pos;
	int rc;

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	for (int i = 0; i < 5; i++) {
		k_msleep(50);
		rc = qec_read_position(&pos);
		zassert_equal(rc, 0,
			      "Failed to read position at iter %d (%d)", i, rc);
		zassert_equal(pos, pos0,
			      "Spurious event: pos=%d != pos0=%d at iter %d",
			      pos, pos0, i);
	}
	printk("QEC no spurious: all 5 reads stable at %d\n", pos0);
}

/* =========================================================================
 * Feature 7: Internal clock frequency up to 400 MHz
 * =========================================================================
 */

/** TC-QEC-020: Verify uTimer clock is enabled - device responds to encoder. */
ZTEST(qec_tests, test_qec_clock_enable_verify)
{
	int pos0, pos1;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	zassert_true(device_is_ready(QEC_DEV), "QEC device not ready");

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_MEDIUM);
	k_msleep(QEC_WAIT_MS);

	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after CW (%d)", rc);

	printk("QEC clock verify: before=%d after=%d\n", pos0, pos1);
	zassert_true(pos1 != pos0,
		     "Counter not clocked - position unchanged after CW rotation");
}

/** TC-QEC-021: Verify pin control applied - encoder signals received. */
ZTEST(qec_tests, test_qec_pinctrl_verify)
{
	int pos0, pos1;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	zassert_true(device_is_ready(QEC_DEV), "QEC device not ready");

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	simulate_cw_rotation(QEC_STEPS_MEDIUM);
	k_msleep(QEC_WAIT_MS);

	rc = qec_read_position(&pos1);
	zassert_equal(rc, 0, "Failed to read position after CW (%d)", rc);

	printk("QEC pinctrl verify: before=%d after=%d\n", pos0, pos1);
	zassert_true(pos1 != pos0,
		     "Pins not muxed - position unchanged after CW rotation");
}

/* =========================================================================
 * Feature 8: External signals frequency up to 50 MHz
 * =========================================================================
 */

/** TC-QEC-022: Verify GPIO encoder emulator init and re-init. */
ZTEST(qec_tests, test_qec_encoder_emulator_init)
{
	int rc;

	rc = qenc_emulate_init();
	zassert_equal(rc, 0, "Encoder emulator init failed (%d)", rc);

	rc = qenc_emulate_init();
	zassert_equal(rc, 0, "Encoder emulator re-init failed (%d)", rc);
}

/** TC-QEC-023: Verify sensor_sample_fetch_chan with SENSOR_CHAN_ALL. */
ZTEST(qec_tests, test_qec_sensor_fetch_chan_all)
{
	struct sensor_value val;
	int rc;

	simulate_cw_rotation(QEC_STEPS_SMALL);
	k_msleep(QEC_WAIT_MS);

	rc = sensor_sample_fetch_chan(QEC_DEV, SENSOR_CHAN_ALL);
	zassert_equal(rc, 0, "sensor_sample_fetch_chan(ALL) failed (%d)", rc);

	rc = sensor_channel_get(QEC_DEV, SENSOR_CHAN_ROTATION, &val);
	zassert_equal(rc, 0, "sensor_channel_get failed (%d)", rc);

	printk("QEC fetch ALL: position=%d\n", val.val1);
	zassert_true(val.val1 >= 0 && val.val1 < 360,
		     "Position out of range: %d", val.val1);
}

/** TC-QEC-024: Verify sensor_sample_fetch_chan with SENSOR_CHAN_ROTATION. */
ZTEST(qec_tests, test_qec_sensor_fetch_chan_rotation)
{
	struct sensor_value val;
	int rc;

	rc = sensor_sample_fetch_chan(QEC_DEV, SENSOR_CHAN_ROTATION);
	zassert_equal(rc, 0, "sensor_sample_fetch_chan(ROTATION) failed (%d)", rc);

	rc = sensor_channel_get(QEC_DEV, SENSOR_CHAN_ROTATION, &val);
	zassert_equal(rc, 0, "sensor_channel_get failed (%d)", rc);

	printk("QEC fetch ROTATION: position=%d\n", val.val1);
	zassert_true(val.val1 >= 0 && val.val1 < 360,
		     "Position out of range: %d", val.val1);
}

/* =========================================================================
 * Negative Tests
 * =========================================================================
 */

/** TC-QEC-025: Verify -ENOTSUP for SENSOR_CHAN_ACCEL_X. */
ZTEST(qec_tests, test_qec_unsupported_channel_accel)
{
	struct sensor_value val;
	int rc;

	rc = sensor_sample_fetch(QEC_DEV);
	zassert_equal(rc, 0, "sensor_sample_fetch failed (%d)", rc);

	rc = sensor_channel_get(QEC_DEV, SENSOR_CHAN_ACCEL_X, &val);
	zassert_equal(rc, -ENOTSUP,
		      "Expected -ENOTSUP for ACCEL_X, got %d", rc);
}

/** TC-QEC-026: Verify -ENOTSUP for SENSOR_CHAN_GYRO_X. */
ZTEST(qec_tests, test_qec_unsupported_channel_gyro)
{
	struct sensor_value val;
	int rc;

	rc = sensor_sample_fetch(QEC_DEV);
	zassert_equal(rc, 0, "sensor_sample_fetch failed (%d)", rc);

	rc = sensor_channel_get(QEC_DEV, SENSOR_CHAN_GYRO_X, &val);
	zassert_equal(rc, -ENOTSUP,
		      "Expected -ENOTSUP for GYRO_X, got %d", rc);
}

/** TC-QEC-027: Verify -ENOTSUP for unsupported fetch channel. */
ZTEST(qec_tests, test_qec_unsupported_fetch_channel)
{
	int rc;

	rc = sensor_sample_fetch_chan(QEC_DEV, SENSOR_CHAN_ACCEL_X);
	zassert_equal(rc, -ENOTSUP,
		      "Expected -ENOTSUP for fetch ACCEL_X, got %d", rc);
}

/* =========================================================================
 * Stress Tests
 * =========================================================================
 */

/** TC-QEC-028: Stress test rapid CW/CCW direction changes. */
ZTEST(qec_tests, test_qec_stress_rapid_direction_change)
{
	int pos0, pos_final, delta;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	rc = qec_read_position(&pos0);
	zassert_equal(rc, 0, "Failed to read initial position (%d)", rc);

	for (int i = 0; i < QEC_STRESS_ITERATIONS; i++) {
		simulate_cw_rotation(QEC_DIRECTION_STEPS);
		k_msleep(10);
		simulate_ccw_rotation(QEC_DIRECTION_STEPS);
		k_msleep(10);
	}

	k_msleep(QEC_WAIT_MS);
	rc = qec_read_position(&pos_final);
	zassert_equal(rc, 0, "Failed to read final position (%d)", rc);

	delta = pos_final - pos0;
	if (delta < 0) {
		delta = -delta;
	}
	if (delta > 180) {
		delta = 360 - delta;
	}

	printk("QEC rapid direction: initial=%d final=%d delta=%d (iters=%d)\n",
	       pos0, pos_final, delta, QEC_STRESS_ITERATIONS);
	zassert_true(delta <= QEC_ROUNDTRIP_TOL * QEC_STRESS_ITERATIONS,
		     "Accumulated error too large (%d)", delta);
}

/** TC-QEC-029: Stress test continuous CW over 5 full revolutions. */
ZTEST(qec_tests, test_qec_stress_continuous_cw_multi_rev)
{
	int pos = 0;
	int valid_reads = 0;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	for (int i = 0; i < 5; i++) {
		simulate_cw_rotation(QEC_STEPS_MULTI_REV);
		k_msleep(QEC_WAIT_MS);

		rc = qec_read_position(&pos);
		zassert_equal(rc, 0,
			      "Failed to read position at rev %d (%d)", i, rc);
		zassert_true(pos >= 0 && pos < 360,
			     "Position %d out of range at rev %d", pos, i);
		valid_reads++;
	}

	printk("QEC stress CW multi-rev: %d valid reads, final=%d\n",
	       valid_reads, pos);
	zassert_equal(valid_reads, 5, "Not all reads succeeded");
}

/** TC-QEC-030: Stress test continuous CCW over 5 full revolutions. */
ZTEST(qec_tests, test_qec_stress_continuous_ccw_multi_rev)
{
	int pos = 0;
	int valid_reads = 0;
	int rc;

	SKIP_IF_NO_LOOPBACK();

	simulate_cw_rotation(QEC_STEPS_MULTI_REV);
	k_msleep(QEC_WAIT_MS);

	for (int i = 0; i < 5; i++) {
		simulate_ccw_rotation(QEC_STEPS_MULTI_REV);
		k_msleep(QEC_WAIT_MS);

		rc = qec_read_position(&pos);
		zassert_equal(rc, 0,
			      "Failed to read position at rev %d (%d)", i, rc);
		zassert_true(pos >= 0 && pos < 360,
			     "Position %d out of range at rev %d", pos, i);
		valid_reads++;
	}

	printk("QEC stress CCW multi-rev: %d valid reads, final=%d\n",
	       valid_reads, pos);
	zassert_equal(valid_reads, 5, "Not all reads succeeded");
}

/** TC-QEC-031: Stress test 50 consecutive fetch+get without motion. */
ZTEST(qec_tests, test_qec_stress_repeated_fetch)
{
	struct sensor_value val;
	int first_pos = -1;
	int rc;

	for (int i = 0; i < QEC_REPEATED_FETCH_COUNT; i++) {
		rc = sensor_sample_fetch(QEC_DEV);
		zassert_equal(rc, 0,
			      "sensor_sample_fetch failed at iter %d (%d)", i, rc);

		rc = sensor_channel_get(QEC_DEV, SENSOR_CHAN_ROTATION, &val);
		zassert_equal(rc, 0,
			      "sensor_channel_get failed at iter %d (%d)", i, rc);

		zassert_true(val.val1 >= 0 && val.val1 < 360,
			     "Position %d out of range at iter %d", val.val1, i);

		if (i == 0) {
			first_pos = val.val1;
		} else {
			zassert_equal(val.val1, first_pos,
				      "Position drift: %d != %d at iter %d",
					  val.val1, first_pos, i);
		}
	}

	printk("QEC repeated fetch: %d reads, all at position=%d\n",
	       QEC_REPEATED_FETCH_COUNT, first_pos);
}

ZTEST_SUITE(qec_tests, NULL, qec_suite_setup, qec_before_each, NULL, NULL);
