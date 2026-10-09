/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/input/input.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#if !DT_NODE_EXISTS(DT_NODELABEL(gt911))
#error "GT911 node gt911 is missing; build with -S alif-gt911"
#endif


#define GT911_PRODUCT_ID     0x00313139U
#define GT911_REG_ID         0x8140U
#define GT911_REG_CONFIG     0x8047U
#define GT911_CONFIG_SIZE    186U
#define GT911_TOUCH_NUM_OFF  5U
#define GT911_I2C_ADDR_5D    0x5d

#define TOUCH_TIMEOUT_MS     30000
#define IDLE_WAIT_MS         15000
#define IDLE_SAMPLE_MS       400
#define RATE_HOLD_MS         2500  /* 5-finger hold measurement window (> 2 s) */
#define RATE_MIN_SCANS       5
#define RATE_MAX_INTERVAL_MS 20
#define RAPID_TAP_TARGET     10
#define DRAG_DELTA           20
#define CORNER_ZONE_DIV      4     /* corner zone = outer 1/this of the panel, in X and Y */
#define CORNER_INVERT_X      1     /* 1: raw X is highest at the physical LEFT edge */
#define CORNER_INVERT_Y      1     /* 1: raw Y is highest at the physical TOP edge */
#define CORNER_CHECK_NAMES   1     /* 1: match named corner (after INVERT), 0: any 4 corners */
#define MAX_SLOTS            10
/* 5 fingers x 100 Hz x 2.5 s = 1250 frames, so keep some margin above that */
#define MAX_FRAMES           1536

/* Multi-finger / stability tests */
#define HOLD_FINGERS         5
#define SLIDE_FINGERS        3     /* fingers used by the slide tests */
#define EDGE_MARGIN          20    /* raw units: a touch this close to an edge is unreliable */
#define SCAN_GROUP_MS        3     /* reports closer than this belong to one scan */
#define JITTER_MAX           40    /* max X/Y wander of a still finger (raw units) */
#define LONG_HOLD_MS         3000
#define GHOST_WATCH_MS       3000

/* Slide / swipe / progressive-finger tests */
#define SLIDE_SPAN_DIV       4     /* each finger must travel > panel_max / this */
#define SWIPE_MAX_GAP_MS     50    /* max time between reports during a swipe */
#define PROGRESSIVE_GAP_MS   1000  /* delay between adding each finger */

static const struct device *const touch_dev = DEVICE_DT_GET(DT_NODELABEL(gt911));
static const struct i2c_dt_spec gt911_i2c = I2C_DT_SPEC_GET(DT_NODELABEL(gt911));
static const struct gpio_dt_spec irq_gpio =
	GPIO_DT_SPEC_GET(DT_NODELABEL(gt911), irq_gpios);

static uint16_t panel_max_x;
static uint16_t panel_max_y;

struct touch_sample {
	int32_t x;
	int32_t y;
	int32_t pressed;
	int32_t slot;
	bool has_x;
	bool has_y;
	bool has_btn;
	bool has_slot;
	int64_t ts_ms;
};

/* acc  : accumulator filled by the input callback
 * last : most recent sample dequeued by the test thread
 * Kept separate so the callback and k_msgq_get() never write the same struct.
 */
static struct touch_sample acc;
static struct touch_sample last;
static struct touch_sample frames[MAX_FRAMES];
static uint32_t frame_count;
static uint32_t sample_count;
static uint32_t tap_presses;
static uint32_t tap_releases;
static uint32_t incomplete_frames;
static int prev_pressed = -1;
static bool slot_down[MAX_SLOTS];
K_MSGQ_DEFINE(touch_msgq, sizeof(struct touch_sample), MAX_FRAMES, 4);

static int gt911_read(uint16_t reg, void *buf, size_t len)
{
	uint16_t addr = BSWAP_16(reg);

	return i2c_write_read_dt(&gt911_i2c, &addr, sizeof(addr), buf, len);
}

static uint8_t slots_down_count(void)
{
	uint8_t n = 0;
	uint8_t i;

	for (i = 0; i < MAX_SLOTS; i++) {
		if (slot_down[i]) {
			n++;
		}
	}

	return n;
}

static uint32_t slots_down_mask(void)
{
	uint32_t mask = 0;
	uint8_t i;

	for (i = 0; i < MAX_SLOTS; i++) {
		if (slot_down[i]) {
			mask |= BIT(i);
		}
	}

	return mask;
}

/* Ring buffer of the last raw input events as the driver emitted them.
 * Dumped when a test sees an unexpected release, so the exact event order
 * (MT_SLOT / X / Y / BTN_TOUCH) from the driver is visible.
 */
#define TRACE_LEN       128
#define TRACE_DUMP_N    40

struct raw_evt {
	int64_t ts;
	int32_t value;
	uint16_t type;
	uint16_t code;
	bool sync;
};

static struct raw_evt trace[TRACE_LEN];
static uint32_t trace_head;

static void trace_reset(void)
{
	trace_head = 0;
}

static void trace_add(const struct input_event *evt)
{
	struct raw_evt *e = &trace[trace_head % TRACE_LEN];

	e->ts = k_uptime_get();
	e->type = evt->type;
	e->code = evt->code;
	e->value = evt->value;
	e->sync = evt->sync;
	trace_head++;
}

static const char *trace_code_name(uint16_t code)
{
	switch (code) {
	case INPUT_ABS_MT_SLOT:
		return "MT_SLOT";
	case INPUT_ABS_X:
		return "ABS_X";
	case INPUT_ABS_Y:
		return "ABS_Y";
	case INPUT_BTN_TOUCH:
		return "BTN_TOUCH";
	default:
		return "other";
	}
}

static void trace_dump(uint32_t n)
{
	uint32_t total = MIN(trace_head, (uint32_t)TRACE_LEN);
	uint32_t i;

	if (n > total) {
		n = total;
	}

	TC_PRINT("--- last %u raw input events (oldest first) ---\n", n);
	for (i = trace_head - n; i < trace_head; i++) {
		const struct raw_evt *e = &trace[i % TRACE_LEN];

		TC_PRINT("[%8lld] type=%u %-9s val=%d%s\n", e->ts, e->type,
			 trace_code_name(e->code), e->value, e->sync ? "  <sync>" : "");
	}
	TC_PRINT("--- end of trace ---\n");
}

static void touch_cb(struct input_event *evt, void *user_data)
{
	ARG_UNUSED(user_data);

	if (evt->dev != touch_dev) {
		return;
	}

	trace_add(evt);

	switch (evt->code) {
	case INPUT_ABS_MT_SLOT:
		acc.slot = evt->value;
		acc.has_slot = true;
		break;
	case INPUT_ABS_X:
		acc.x = evt->value;
		acc.has_x = true;
		break;
	case INPUT_ABS_Y:
		acc.y = evt->value;
		acc.has_y = true;
		break;
	case INPUT_BTN_TOUCH:
		acc.pressed = evt->value;
		acc.has_btn = true;
		acc.ts_ms = k_uptime_get();

		if (!acc.has_x || !acc.has_y) {
			incomplete_frames++;
		}

		if (acc.has_slot && acc.slot >= 0 && acc.slot < MAX_SLOTS) {
			slot_down[acc.slot] = (acc.pressed != 0);
		}

		sample_count++;
		if (acc.pressed) {
			if (prev_pressed != 1) {
				tap_presses++;
			}
		} else if (prev_pressed != 0) {
			tap_releases++;
		}
		prev_pressed = acc.pressed;

		if (frame_count < MAX_FRAMES) {
			frames[frame_count++] = acc;
		}

		(void)k_msgq_put(&touch_msgq, &acc, K_NO_WAIT);
		acc.has_x = false;
		acc.has_y = false;
		acc.has_btn = false;
		break;
	default:
		break;
	}
}
INPUT_CALLBACK_DEFINE(touch_dev, touch_cb, NULL);

/* Full reset: forgets everything, including which fingers are down. */
static void capture_reset(void)
{
	memset(&acc, 0, sizeof(acc));
	memset(&last, 0, sizeof(last));
	memset(frames, 0, sizeof(frames));
	memset(slot_down, 0, sizeof(slot_down));
	trace_reset();
	frame_count = 0;
	sample_count = 0;
	tap_presses = 0;
	tap_releases = 0;
	incomplete_frames = 0;
	prev_pressed = -1;
	k_msgq_purge(&touch_msgq);
}

/* Restart only the measurement window. slot_down[] and prev_pressed are
 * kept so fingers that are already down stay counted.
 */
static void capture_window_reset(void)
{
	memset(frames, 0, sizeof(frames));
	frame_count = 0;
	sample_count = 0;
	incomplete_frames = 0;
	k_msgq_purge(&touch_msgq);
}

static int wait_touch(k_timeout_t timeout)
{
	return k_msgq_get(&touch_msgq, &last, timeout);
}

static k_timeout_t remain_timeout(int64_t deadline_ms)
{
	int64_t remain = deadline_ms - k_uptime_get();

	if (remain <= 0) {
		return K_NO_WAIT;
	}

	return K_MSEC((uint32_t)remain);
}

static int wait_press(int32_t timeout_ms)
{
	int64_t deadline = k_uptime_get() + timeout_ms;
	int ret;

	while (k_uptime_get() < deadline) {
		ret = wait_touch(remain_timeout(deadline));
		if (ret != 0) {
			return ret;
		}
		if (last.pressed) {
			return 0;
		}
	}

	return -EAGAIN;
}

static int wait_release(int32_t timeout_ms)
{
	int64_t deadline = k_uptime_get() + timeout_ms;
	int ret;

	while (k_uptime_get() < deadline) {
		ret = wait_touch(remain_timeout(deadline));
		if (ret != 0) {
			return ret;
		}
		if (!last.pressed) {
			return 0;
		}
	}

	return -EAGAIN;
}

/* Wait until at least n slots are down. */
static int wait_slots_down(uint8_t n, int32_t timeout_ms)
{
	int64_t deadline = k_uptime_get() + timeout_ms;
	struct touch_sample s;

	while (slots_down_count() < n) {
		if (k_msgq_get(&touch_msgq, &s, remain_timeout(deadline)) != 0) {
			return -EAGAIN;
		}
	}

	return 0;
}

/* Wait until every slot is released. */
static int wait_all_lifted(int32_t timeout_ms)
{
	int64_t deadline = k_uptime_get() + timeout_ms;
	struct touch_sample s;

	while (slots_down_count() > 0U) {
		if (k_msgq_get(&touch_msgq, &s, remain_timeout(deadline)) != 0) {
			return -EAGAIN;
		}
	}

	return 0;
}

static int wait_idle(bool announce)
{
	int64_t deadline = k_uptime_get() + IDLE_WAIT_MS;
	bool said = announce;

	if (announce) {
		TC_PRINT("Lift all fingers and leave the panel untouched\n");
	}

	while (k_uptime_get() < deadline) {
		capture_reset();
		k_sleep(K_MSEC(IDLE_SAMPLE_MS));
		if (sample_count == 0 && k_msgq_num_used_get(&touch_msgq) == 0) {
			return 0;
		}
		if (!said) {
			TC_PRINT("Lift all fingers and leave the panel untouched\n");
			said = true;
		} else {
			TC_PRINT("Still seeing touch reports — lift all fingers\n");
		}
	}

	return -EBUSY;
}

static int wait_until_idle(void)
{
	return wait_idle(true);
}

/* Count scans. Several finger reports within group_ms are ONE scan.
 * first_ts = start of first scan, last_ts = start of last scan.
 */
static uint32_t grouped_scan_count(int64_t group_ms, int64_t *first_ts, int64_t *last_ts)
{
	uint32_t n = 0;
	int64_t scan_start = -1;
	uint32_t i;

	*first_ts = 0;
	*last_ts = 0;

	for (i = 0; i < frame_count; i++) {
		if (!frames[i].pressed) {
			continue;
		}
		if (n > 0U && (frames[i].ts_ms - scan_start) <= group_ms) {
			continue;
		}
		scan_start = frames[i].ts_ms;
		if (n == 0U) {
			*first_ts = scan_start;
		}
		*last_ts = scan_start;
		n++;
	}

	return n;
}

static void skip_if_not_interactive(void)
{
	if (!IS_ENABLED(CONFIG_TEST_GT911_INTERACTIVE)) {
		TC_PRINT("SKIP: touch tests need CONFIG_TEST_GT911_INTERACTIVE=y\n");
		ztest_test_skip();
	}
}

static void skip_if_not_5_finger(void)
{
	if (CONFIG_INPUT_GT911_MAX_TOUCH_POINTS < HOLD_FINGERS) {
		TC_PRINT("SKIP: MAX_TOUCH_POINTS=%d < %d\n",
			 CONFIG_INPUT_GT911_MAX_TOUCH_POINTS, HOLD_FINGERS);
		ztest_test_skip();
	}
}

static void assert_in_range(int32_t x, int32_t y)
{
	zassert_true(x >= 0 && x <= panel_max_x, "X %d out of range 0..%u", x, panel_max_x);
	zassert_true(y >= 0 && y <= panel_max_y, "Y %d out of range 0..%u", y, panel_max_y);
}

/* Slide with SLIDE_FINGERS fingers in one direction. All of them must stay
 * down, keep their original slot IDs (no new or swapped IDs), and each travel
 * at least panel_max / SLIDE_SPAN_DIV along the chosen axis.
 */
static void multi_finger_slide(bool horizontal)
{
	struct slide_stat {
		bool seen;
		int32_t x0;
		int32_t y0;
		int32_t travel;
	} st[MAX_SLOTS] = {0};
	int32_t lx[MAX_SLOTS] = {0};
	int32_t ly[MAX_SLOTS] = {0};
	struct touch_sample s;
	uint32_t init_mask;
	uint32_t done_mask = 0;
	int32_t required;
	int32_t d;
	int64_t deadline;
	uint32_t i;
	int ret;

	/* Same travel on both axes, so a vertical slide is not 1.7x longer than a
	 * horizontal one on a tall panel (480x800 -> 120 units either way).
	 */
	required = MIN(panel_max_x, panel_max_y) / SLIDE_SPAN_DIV;

	TC_PRINT("Place %d fingers down (keep them away from the panel edges), then SLIDE "
		 "them all %s (at least %d units)\n"
		 "Keep all %d fingers on the panel until told to lift\n",
		 SLIDE_FINGERS, horizontal ? "LEFT <-> RIGHT" : "UP <-> DOWN", required,
		 SLIDE_FINGERS);

	ret = wait_slots_down(SLIDE_FINGERS, TOUCH_TIMEOUT_MS);
	zassert_ok(ret, "Did not see %d fingers down (got %u)",
		   SLIDE_FINGERS, slots_down_count());

	init_mask = slots_down_mask();
	capture_window_reset();

	deadline = k_uptime_get() + TOUCH_TIMEOUT_MS;
	while (done_mask != init_mask) {
		ret = k_msgq_get(&touch_msgq, &s, remain_timeout(deadline));
		zassert_ok(ret, "Timed out: only %u of %u fingers slid far enough",
			   POPCOUNT(done_mask), POPCOUNT(init_mask));

		if (!s.has_slot || s.slot < 0 || s.slot >= MAX_SLOTS) {
			continue;
		}

		if (!s.pressed) {
			TC_PRINT("Release on slot %d: down mask now 0x%x, initial mask 0x%x, "
				 "slid so far 0x%x\n",
				 s.slot, slots_down_mask(), init_mask, done_mask);
			TC_PRINT("Slot %d was last seen at %d,%d\n",
				 s.slot, lx[s.slot], ly[s.slot]);
			if (lx[s.slot] <= EDGE_MARGIN || lx[s.slot] >= panel_max_x - EDGE_MARGIN ||
			    ly[s.slot] <= EDGE_MARGIN || ly[s.slot] >= panel_max_y - EDGE_MARGIN) {
				TC_PRINT("  -> within %d units of the panel edge, where touches "
					 "can be lost. Keep fingers further inward.\n",
					 EDGE_MARGIN);
			}
			trace_dump(TRACE_DUMP_N);
		}
		zassert_true(s.pressed, "Finger on slot %d lifted before all fingers slid",
			     s.slot);
		zassert_true((init_mask & BIT(s.slot)) != 0,
			     "Unexpected slot %d appeared during slide (mask 0x%x)",
			     s.slot, init_mask);
		assert_in_range(s.x, s.y);
		lx[s.slot] = s.x;
		ly[s.slot] = s.y;

		if (!st[s.slot].seen) {
			st[s.slot].seen = true;
			st[s.slot].x0 = s.x;
			st[s.slot].y0 = s.y;
			continue;
		}

		d = horizontal ? abs(s.x - st[s.slot].x0) : abs(s.y - st[s.slot].y0);
		if (d > st[s.slot].travel) {
			st[s.slot].travel = d;
		}
		if (st[s.slot].travel >= required) {
			done_mask |= BIT(s.slot);
		}
	}

	for (i = 0; i < MAX_SLOTS; i++) {
		if (init_mask & BIT(i)) {
			TC_PRINT("slot %u travelled %d\n", i, st[i].travel);
		}
	}

	zassert_equal(incomplete_frames, 0, "%u incomplete frames", incomplete_frames);
	zassert_equal(slots_down_count(), SLIDE_FINGERS,
		      "Lost a finger during slide (%u down)", slots_down_count());

	TC_PRINT("Slide OK — lift all fingers\n");
	(void)wait_all_lifted(5000);
}

static void *gt911_setup(void)
{
	uint8_t cfg[GT911_CONFIG_SIZE];
	uint8_t touch_num;
	int ret;

	zassert_not_null(touch_dev, "GT911 device missing from DT");
	zassert_true(device_is_ready(touch_dev), "GT911 not ready");
	zassert_true(i2c_is_ready_dt(&gt911_i2c), "I2C bus not ready");

	ret = gt911_read(GT911_REG_CONFIG, cfg, sizeof(cfg));
	zassert_ok(ret, "Failed to read GT911 config (%d)", ret);

	panel_max_x = cfg[1] | ((uint16_t)cfg[2] << 8);
	panel_max_y = cfg[3] | ((uint16_t)cfg[4] << 8);
	touch_num = cfg[GT911_TOUCH_NUM_OFF];

	if (panel_max_x == 0U) {
		panel_max_x = 2048;
	}
	if (panel_max_y == 0U) {
		panel_max_y = 2048;
	}

	TC_PRINT("GT911 ready: max %ux%u, touch_num=%u, addr=0x%02x\n",
		 panel_max_x, panel_max_y, touch_num, gt911_i2c.addr);
	if (IS_ENABLED(CONFIG_TEST_GT911_INTERACTIVE)) {
		TC_PRINT("Interactive tests ENABLED — wait for each prompt, then touch\n");
	} else {
		TC_PRINT("Interactive tests DISABLED — touch cases will SKIP\n");
	}

	return NULL;
}

static void gt911_before(void *f)
{
	ARG_UNUSED(f);
	capture_reset();

	/* A failed test can leave a finger on the panel; make sure the next
	 * test starts from an idle panel instead of inheriting that touch.
	 */
	if (IS_ENABLED(CONFIG_TEST_GT911_INTERACTIVE)) {
		(void)wait_idle(false);
	}
}

ZTEST_SUITE(gt911_interactive, NULL, gt911_setup, gt911_before, NULL, NULL);

ZTEST(gt911_interactive, test_press_and_release)
{
	int ret;

	skip_if_not_interactive();

	TC_PRINT("Tap the screen once, then lift your finger\n");

	ret = wait_press(TOUCH_TIMEOUT_MS);
	zassert_ok(ret, "Timed out waiting for press");
	assert_in_range(last.x, last.y);

	ret = wait_release(TOUCH_TIMEOUT_MS);
	zassert_ok(ret, "Timed out waiting for release");
	zassert_equal(incomplete_frames, 0, "%u incomplete frames", incomplete_frames);
}

/* True if v lies in the outer 1/CORNER_ZONE_DIV of 0..max on the chosen side. */
static bool in_edge_zone(int32_t v, int32_t max, bool high_side)
{
	int32_t zone = max / CORNER_ZONE_DIV;

	return high_side ? (v >= max - zone) : (v <= zone);
}

/* Each tap must land inside a corner zone (near an edge in BOTH axes), the
 * four taps must hit four different corners, and (CORNER_CHECK_NAMES) each
 * must match the corner that was asked for. A tap in the middle or on an
 * edge centre fails.
 */
ZTEST(gt911_interactive, test_corners)
{
	static const struct {
		const char *name;
		bool right;
		bool bottom;
	} corners[] = {
		{ "TOP-LEFT", false, false },
		{ "TOP-RIGHT", true, false },
		{ "BOTTOM-RIGHT", true, true },
		{ "BOTTOM-LEFT", false, true },
	};
	uint32_t seen = 0;
	int32_t x, y;
	int32_t ex, ey;
	uint32_t quad;
	uint32_t want;
	bool near_x, near_y;
	int i;
	int ret;

	skip_if_not_interactive();

	zassert_ok(wait_until_idle(), "Lift all fingers before corner taps");

	for (i = 0; i < 4; i++) {
		TC_PRINT("Tap the %s corner (outer 1/%d of the panel), then lift\n",
			 corners[i].name, CORNER_ZONE_DIV);

		ret = wait_press(TOUCH_TIMEOUT_MS);
		zassert_ok(ret, "Timed out at %s", corners[i].name);
		x = last.x;
		y = last.y;
		assert_in_range(x, y);
		TC_PRINT("%s -> %d,%d\n", corners[i].name, x, y);

		near_x = in_edge_zone(x, panel_max_x, false) || in_edge_zone(x, panel_max_x, true);
		near_y = in_edge_zone(y, panel_max_y, false) || in_edge_zone(y, panel_max_y, true);
		zassert_true(near_x && near_y,
			     "%s tap at %d,%d is not in a corner (X must be <=%d or >=%d, "
			     "Y must be <=%d or >=%d)", corners[i].name, x, y,
			     panel_max_x / CORNER_ZONE_DIV,
			     panel_max_x - panel_max_x / CORNER_ZONE_DIV,
			     panel_max_y / CORNER_ZONE_DIV,
			     panel_max_y - panel_max_y / CORNER_ZONE_DIV);

		ex = CORNER_INVERT_X ? ((int32_t)panel_max_x - x) : x;
		ey = CORNER_INVERT_Y ? ((int32_t)panel_max_y - y) : y;
		quad = (ex > panel_max_x / 2 ? 1U : 0U) | (ey > panel_max_y / 2 ? 2U : 0U);
		zassert_false((seen & BIT(quad)) != 0U,
			      "%s tap at %d,%d is in a corner that was already tapped",
			      corners[i].name, x, y);
		seen |= BIT(quad);

#if CORNER_CHECK_NAMES
		want = (corners[i].right ? 1U : 0U) | (corners[i].bottom ? 2U : 0U);
		zassert_equal(quad, want,
			      "Asked for %s but tap at %d,%d is in a different corner "
			      "(check CORNER_INVERT_X/Y or set CORNER_CHECK_NAMES 0)",
			      corners[i].name, x, y);
#else
		ARG_UNUSED(want);
#endif

		ret = wait_release(TOUCH_TIMEOUT_MS);
		zassert_ok(ret, "Lift your finger after %s", corners[i].name);
	}

	zassert_equal(seen, 0xFU, "Did not tap four different corners (mask 0x%x)", seen);
}

/* Hold FIVE fingers still and check the controller scan rate. */
ZTEST(gt911_interactive, test_hold_report_rate_5_fingers)
{
	int64_t first_ts;
	int64_t last_ts;
	int64_t dt;
	int64_t avg;
	uint32_t scans;
	int ret;

	skip_if_not_interactive();
	skip_if_not_5_finger();

	TC_PRINT("Place FIVE fingers on the panel and HOLD them still for %d.%d seconds\n",
		 RATE_HOLD_MS / 1000, (RATE_HOLD_MS % 1000) / 100);

	ret = wait_slots_down(HOLD_FINGERS, TOUCH_TIMEOUT_MS);
	zassert_ok(ret, "Did not see %d fingers down (got %u)",
		   HOLD_FINGERS, slots_down_count());

	/* Measure only while all five are held */
	capture_window_reset();
	k_sleep(K_MSEC(RATE_HOLD_MS));

	scans = grouped_scan_count(SCAN_GROUP_MS, &first_ts, &last_ts);
	if (scans < RATE_MIN_SCANS) {
		TC_PRINT("Only %u scans while held; controller may IRQ on change only\n", scans);
		ztest_test_skip();
	}

	dt = last_ts - first_ts;
	zassert_true(dt > 0, "Hold duration was 0");
	avg = dt / (scans - 1U);
	TC_PRINT("5-finger hold: samples=%u scans=%u duration=%lld ms avg interval=%lld ms\n",
		 frame_count, scans, dt, avg);
	/* Datasheet typical is 100 Hz. Several points in one scan must not
	 * look like a faster rate. Faster than 100 Hz is still a pass.
	 */
	zassert_true(avg <= RATE_MAX_INTERVAL_MS,
		     "Average scan interval %lld ms is slower than %d ms (50 Hz)",
		     avg, RATE_MAX_INTERVAL_MS);
	zassert_equal(slots_down_count(), HOLD_FINGERS,
		      "Lost a finger during hold (%u down)", slots_down_count());

	(void)wait_all_lifted(5000);
}

ZTEST(gt911_interactive, test_drag_track_id)
{
	int32_t x0, y0;
	bool moved = false;
	uint32_t i;
	int32_t slot_seen = -1;
	uint32_t unique_slots = 0;
	int ret;

	skip_if_not_interactive();

	TC_PRINT("Press with ONE finger, SLIDE across the panel, then lift\n");

	ret = wait_press(TOUCH_TIMEOUT_MS);
	zassert_ok(ret, "Timed out waiting for press");
	x0 = last.x;
	y0 = last.y;

	while (wait_touch(K_MSEC(TOUCH_TIMEOUT_MS)) == 0) {
		if (last.pressed == 0) {
			break;
		}
		if ((abs(last.x - x0) >= DRAG_DELTA) || (abs(last.y - y0) >= DRAG_DELTA)) {
			moved = true;
		}
	}

	zassert_true(moved, "X/Y did not change by at least %d during drag", DRAG_DELTA);
	zassert_true(frame_count >= 3, "Too few drag frames (%u)", frame_count);

	if (CONFIG_INPUT_GT911_MAX_TOUCH_POINTS > 1) {
		for (i = 0; i < frame_count; i++) {
			if (!frames[i].pressed || !frames[i].has_slot) {
				continue;
			}
			if (slot_seen < 0) {
				slot_seen = frames[i].slot;
				unique_slots = 1;
			} else if (frames[i].slot != slot_seen) {
				unique_slots++;
			}
		}
		if (unique_slots > 1U) {
			TC_PRINT("Multiple track IDs (%u) — use one finger for this test\n",
				 unique_slots);
		}
	}
}

ZTEST(gt911_interactive, test_rapid_taps_no_drop)
{
	int64_t deadline;
	int ret;

	skip_if_not_interactive();

	TC_PRINT("Tap and fully lift %d times (do not hold)\n", RAPID_TAP_TARGET);

	deadline = k_uptime_get() + 60000;
	while ((tap_presses < RAPID_TAP_TARGET || tap_releases < RAPID_TAP_TARGET) &&
	       (k_uptime_get() < deadline)) {
		ret = wait_touch(remain_timeout(deadline));
		if (ret != 0) {
			break;
		}
	}

	TC_PRINT("taps down=%u up=%u samples=%u incomplete=%u\n",
		 tap_presses, tap_releases, sample_count, incomplete_frames);
	zassert_true(tap_presses >= RAPID_TAP_TARGET, "Only %u tap presses", tap_presses);
	zassert_true(tap_releases >= RAPID_TAP_TARGET, "Only %u tap releases", tap_releases);
	zassert_true(abs((int)tap_presses - (int)tap_releases) <= 1,
		     "Unbalanced tap edges %u/%u", tap_presses, tap_releases);
	zassert_equal(incomplete_frames, 0, "%u incomplete frames", incomplete_frames);
}

ZTEST(gt911_interactive, test_multi_touch_and_partial_lift)
{
	int64_t deadline;
	bool saw_two = false;

	skip_if_not_interactive();

	if (CONFIG_INPUT_GT911_MAX_TOUCH_POINTS < 2) {
		ztest_test_skip();
	}

	TC_PRINT("Place TWO fingers and keep both down\n");

	deadline = k_uptime_get() + 30000;
	while (k_uptime_get() < deadline) {
		if (wait_touch(remain_timeout(deadline)) != 0) {
			break;
		}
		if (slots_down_count() >= 2U) {
			saw_two = true;
			break;
		}
	}

	zassert_true(saw_two, "Did not see two active slots");

	TC_PRINT("Lift only ONE finger, keep the other down\n");

	deadline = k_uptime_get() + 30000;
	while (k_uptime_get() < deadline) {
		if (wait_touch(remain_timeout(deadline)) != 0) {
			break;
		}
		if (slots_down_count() == 1U) {
			return;
		}
	}

	zassert_equal(slots_down_count(), 1, "Expected one finger still down, got %u",
		      slots_down_count());
}

/* ------------------------------------------------------------------ */
/* New test cases                                                      */
/* ------------------------------------------------------------------ */

/* Slide three fingers horizontally. */
ZTEST(gt911_interactive, test_three_finger_slide_horizontal)
{
	skip_if_not_interactive();

	if (CONFIG_INPUT_GT911_MAX_TOUCH_POINTS < SLIDE_FINGERS) {
		TC_PRINT("SKIP: MAX_TOUCH_POINTS=%d < %d\n",
			 CONFIG_INPUT_GT911_MAX_TOUCH_POINTS, SLIDE_FINGERS);
		ztest_test_skip();
	}

	multi_finger_slide(true);
}

/* Slide three fingers vertically. */
ZTEST(gt911_interactive, test_three_finger_slide_vertical)
{
	skip_if_not_interactive();

	if (CONFIG_INPUT_GT911_MAX_TOUCH_POINTS < SLIDE_FINGERS) {
		TC_PRINT("SKIP: MAX_TOUCH_POINTS=%d < %d\n",
			 CONFIG_INPUT_GT911_MAX_TOUCH_POINTS, SLIDE_FINGERS);
		ztest_test_skip();
	}

	multi_finger_slide(false);
}

/* Progressive finger detection: add fingers one at a time (1 -> 2 -> 3 ->
 * 4 -> 5) with a 1 s gap after each. After every step exactly that many
 * fingers must be down, every earlier finger must keep its slot, and the new
 * finger must get a new slot ID.
 */
ZTEST(gt911_interactive, test_progressive_finger_detect)
{
	struct touch_sample ev;
	uint32_t prev_mask = 0;
	uint32_t mask;
	uint32_t added;
	int64_t t_press;
	int64_t deadline;
	uint8_t n;
	int ret;

	skip_if_not_interactive();
	skip_if_not_5_finger();

	zassert_ok(wait_until_idle(), "Lift all fingers before starting");
	capture_reset();

	for (n = 1; n <= HOLD_FINGERS; n++) {
		TC_PRINT("Place finger #%u now and keep it down (%u fingers total)\n",
			 n, n);

		ret = wait_slots_down(n, TOUCH_TIMEOUT_MS);
		zassert_ok(ret, "Timed out waiting for finger #%u (%u down)",
			   n, slots_down_count());

		/* 1 s gap: fingers must stay detected while held. Log every release
		 * seen so a real lift can be told apart from a driver glitch.
		 */
		t_press = k_uptime_get();
		deadline = t_press + PROGRESSIVE_GAP_MS;
		while (k_uptime_get() < deadline) {
			if (k_msgq_get(&touch_msgq, &ev, remain_timeout(deadline)) != 0) {
				break;
			}
			if (!ev.pressed) {
				TC_PRINT("  release: slot=%d x=%d y=%d at +%lld ms\n",
					 ev.slot, ev.x, ev.y, k_uptime_get() - t_press);
			}
		}

		mask = slots_down_mask();
		if (slots_down_count() != n) {
			trace_dump(TRACE_DUMP_N);
		}
		zassert_equal(slots_down_count(), n,
			      "Expected %u fingers down, got %u (mask 0x%x)",
			      n, slots_down_count(), mask);
		zassert_equal(mask & prev_mask, prev_mask,
			      "Earlier finger lost or changed slot (0x%x -> 0x%x)",
			      prev_mask, mask);

		added = mask & ~prev_mask;
		zassert_equal(POPCOUNT(added), 1,
			      "Expected one new slot for finger #%u, got mask 0x%x", n, added);
		TC_PRINT("Finger #%u detected on slot %u (mask 0x%x)\n",
			 n, (uint32_t)__builtin_ctz(added), mask);

		prev_mask = mask;
		k_msgq_purge(&touch_msgq);
	}

	zassert_equal(incomplete_frames, 0, "%u incomplete frames", incomplete_frames);

	TC_PRINT("All %d fingers detected — lift all fingers\n", HOLD_FINGERS);
	(void)wait_all_lifted(5000);
}

/* One-finger fast swipe: reports must be continuous (no long gaps) and the
 * finger must cover a real distance.
 */
ZTEST(gt911_interactive, test_swipe_continuity)
{
	int32_t x0, y0;
	int32_t travel = 0;
	int32_t d;
	int64_t prev_ts;
	int64_t gap;
	int64_t max_gap = 0;
	uint32_t reports = 1;
	int32_t required;
	int ret;

	skip_if_not_interactive();

	required = MIN(panel_max_x, panel_max_y) / SLIDE_SPAN_DIV;

	TC_PRINT("Place ONE finger and swipe quickly across the panel (> %d units), then lift\n",
		 required);

	ret = wait_press(TOUCH_TIMEOUT_MS);
	zassert_ok(ret, "Timed out waiting for press");
	x0 = last.x;
	y0 = last.y;
	prev_ts = last.ts_ms;

	while (wait_touch(K_MSEC(TOUCH_TIMEOUT_MS)) == 0) {
		if (!last.pressed) {
			break;
		}
		assert_in_range(last.x, last.y);

		gap = last.ts_ms - prev_ts;
		if (gap > max_gap) {
			max_gap = gap;
		}
		prev_ts = last.ts_ms;
		reports++;

		d = MAX(abs(last.x - x0), abs(last.y - y0));
		if (d > travel) {
			travel = d;
		}
	}

	TC_PRINT("Swipe: reports=%u travel=%d max gap=%lld ms\n", reports, travel, max_gap);
	zassert_true(travel >= required, "Travel %d < %d; swipe further", travel, required);
	zassert_true(reports >= 5, "Too few reports during swipe (%u)", reports);
	zassert_true(max_gap <= SWIPE_MAX_GAP_MS,
		     "Report gap %lld ms > %d ms during swipe", max_gap, SWIPE_MAX_GAP_MS);
	zassert_equal(incomplete_frames, 0, "%u incomplete frames", incomplete_frames);
}

/* A still finger held for a few seconds must not wander or drop. */
ZTEST(gt911_interactive, test_long_hold_stability)
{
	int32_t min_x, max_x, min_y, max_y;
	uint32_t i;
	int ret;

	skip_if_not_interactive();

	TC_PRINT("Press ONE finger and keep it perfectly still for %d s\n",
		 LONG_HOLD_MS / 1000);

	ret = wait_press(TOUCH_TIMEOUT_MS);
	zassert_ok(ret, "Timed out waiting for press");

	capture_window_reset();
	k_sleep(K_MSEC(LONG_HOLD_MS));

	zassert_equal(incomplete_frames, 0, "%u incomplete frames", incomplete_frames);
	zassert_equal(slots_down_count(), 1, "Finger lost during hold (%u down)",
		      slots_down_count());

	if (frame_count == 0U) {
		TC_PRINT("No reports during hold; controller may IRQ on change only\n");
		(void)wait_release(5000);
		return;
	}

	min_x = max_x = frames[0].x;
	min_y = max_y = frames[0].y;
	for (i = 0; i < frame_count; i++) {
		zassert_true(frames[i].pressed, "Finger released during hold (frame %u)", i);
		min_x = MIN(min_x, frames[i].x);
		max_x = MAX(max_x, frames[i].x);
		min_y = MIN(min_y, frames[i].y);
		max_y = MAX(max_y, frames[i].y);
	}

	TC_PRINT("Jitter: X %d..%d  Y %d..%d (%u frames)\n",
		 min_x, max_x, min_y, max_y, frame_count);
	zassert_true((max_x - min_x) <= JITTER_MAX, "X jitter %d > %d",
		     max_x - min_x, JITTER_MAX);
	zassert_true((max_y - min_y) <= JITTER_MAX, "Y jitter %d > %d",
		     max_y - min_y, JITTER_MAX);

	(void)wait_release(5000);
}

/* Untouched panel must stay silent (no ghost touches). */
ZTEST(gt911_interactive, test_no_ghost_touches_when_idle)
{
	skip_if_not_interactive();

	zassert_ok(wait_until_idle(), "Panel not idle");

	capture_reset();
	TC_PRINT("Do NOT touch the panel for %d s\n", GHOST_WATCH_MS / 1000);
	k_sleep(K_MSEC(GHOST_WATCH_MS));

	zassert_equal(sample_count, 0, "%u ghost touch reports while idle", sample_count);
	zassert_equal(slots_down_count(), 0, "Ghost slot down while idle");
}
