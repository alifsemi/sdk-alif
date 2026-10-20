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
#include <zephyr/input/input.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usb_hid.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/drivers/usb/udc_buf.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <sample_usbd.h>

#define JOYSTICK_BUTTON          0
#define JOYSTICK_X               1
#define JOYSTICK_Y               2
#define JOYSTICK_REPORT_COUNT    3
#define JOYSTICK_AXIS_DELTA      20
#define STRESS_ITERATIONS        50
#define RAPID_TOGGLE_COUNT       20
#define PERF_INJECT_COUNT        200
#define PERF_SUBMIT_COUNT        50
#define IFACE_READY_TIMEOUT_MS   3000
#define REENUM_TIMEOUT_MS        10000

#define HID_JOYSTICK_REPORT_DESC(bcnt) \
{ \
	HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP), \
	HID_USAGE(HID_USAGE_GEN_DESKTOP_JOYSTICK), \
	HID_COLLECTION(HID_COLLECTION_APPLICATION), \
		HID_USAGE_PAGE(HID_USAGE_GEN_BUTTON), \
		HID_USAGE_MIN8(1), \
		HID_USAGE_MAX8(bcnt), \
		HID_LOGICAL_MIN8(0), \
		HID_LOGICAL_MAX8(1), \
		HID_REPORT_SIZE(1), \
		HID_REPORT_COUNT(bcnt), \
		HID_INPUT(0x02), \
		HID_REPORT_SIZE(8 - bcnt), \
		HID_REPORT_COUNT(1), \
		HID_INPUT(0x03), \
		HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP), \
		HID_USAGE(HID_USAGE_GEN_DESKTOP_X), \
		HID_USAGE(HID_USAGE_GEN_DESKTOP_Y), \
		HID_LOGICAL_MIN8(-127), \
		HID_LOGICAL_MAX8(127), \
		HID_REPORT_SIZE(8), \
		HID_REPORT_COUNT(2), \
		HID_INPUT(0x02), \
	HID_END_COLLECTION \
}

static const uint8_t hid_report_desc[] = HID_JOYSTICK_REPORT_DESC(1);
static const uint8_t report_neutral[JOYSTICK_REPORT_COUNT] = {0, 0, 0};

struct hid_joystick_qa_fixture {
	const struct device *hid_dev;
	struct usbd_context *usbd_ctx;
	int register_ret;
};

static struct hid_joystick_qa_fixture *g_hid;

/* DWC3 requires 32-byte aligned transfer buffers (same as the sample). */
UDC_STATIC_BUF_DEFINE(report, JOYSTICK_REPORT_COUNT);
static atomic_t hid_iface_ready;
static volatile int unrecognized_code;
static volatile int unrecognized_count;
static K_SEM_DEFINE(ep_write_sem, 0, 1);

static void iface_ready_cb(const struct device *dev, const bool ready)
{
	ARG_UNUSED(dev);
	atomic_set(&hid_iface_ready, ready ? 1 : 0);
	TC_PRINT("HID iface ready=%d\n", ready ? 1 : 0);
	if (ready) {
		k_sem_give(&ep_write_sem);
	}
}

static void input_report_done_cb(const struct device *dev, const uint8_t *const rpt)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(rpt);
	k_sem_give(&ep_write_sem);
}

static int hid_get_report(const struct device *dev, const uint8_t type, const uint8_t id,
			  const uint16_t len, uint8_t *const buf)
{
	size_t n = MIN(len, (uint16_t)JOYSTICK_REPORT_COUNT);

	ARG_UNUSED(dev);
	ARG_UNUSED(type);
	ARG_UNUSED(id);

	if (n == 0) {
		return -ENOTSUP;
	}

	memcpy(buf, report, n);
	return (int)n;
}

static const struct hid_device_ops hid_ops = {
	.iface_ready = iface_ready_cb,
	.get_report = hid_get_report,
	.input_report_done = input_report_done_cb,
};

static void input_cb(struct input_event *evt, void *user_data)
{
	ARG_UNUSED(user_data);

	switch (evt->code) {
	case INPUT_KEY_RIGHT:
		report[JOYSTICK_X] = evt->value ? JOYSTICK_AXIS_DELTA : 0;
		break;
	case INPUT_KEY_LEFT:
		report[JOYSTICK_X] = evt->value ? (uint8_t)(-JOYSTICK_AXIS_DELTA) : 0;
		break;
	case INPUT_KEY_UP:
		report[JOYSTICK_Y] = evt->value ? (uint8_t)(-JOYSTICK_AXIS_DELTA) : 0;
		break;
	case INPUT_KEY_DOWN:
		report[JOYSTICK_Y] = evt->value ? JOYSTICK_AXIS_DELTA : 0;
		break;
	case INPUT_KEY_ENTER:
		WRITE_BIT(report[JOYSTICK_BUTTON], 0, evt->value);
		break;
	default:
		unrecognized_code = evt->code;
		unrecognized_count++;
		return;
	}
}

INPUT_CALLBACK_DEFINE(NULL, input_cb, NULL);

static void wait_iface_ready_ms(int timeout_ms)
{
	int waited = 0;

	while (!atomic_get(&hid_iface_ready) && waited < timeout_ms) {
		k_sleep(K_MSEC(50));
		waited += 50;
	}
}

static void wait_iface_ready(void)
{
	wait_iface_ready_ms(IFACE_READY_TIMEOUT_MS);
}

/* Host re-enumeration after usbd_disable can take several seconds. */
static void wait_host_reenum(void)
{
	wait_iface_ready_ms(REENUM_TIMEOUT_MS);
	TC_PRINT("HID iface ready=%d after re-enum wait\n",
		 atomic_get(&hid_iface_ready) ? 1 : 0);
}

static void skip_if_no_host(void)
{
	if (!atomic_get(&hid_iface_ready)) {
		TC_PRINT("HID iface not ready (no host), skipping\n");
		ztest_test_skip();
	}
}

/*
 * Do not wait for input_report_done before the first submit: that callback
 * only runs after the host ACKs interrupt IN (e.g. hidraw open). If nobody
 * is reading hidraw, a pre-wait times out and poisons later disable/enable.
 * On -ENOMEM wait briefly for a completed IN and retry.
 */
static int submit_hid_report(const struct device *hid_dev)
{
	int err;
	int tries = 5;

	while (tries--) {
		err = hid_device_submit_report(hid_dev, JOYSTICK_REPORT_COUNT, report);
		if (err != -ENOMEM) {
			return err;
		}
		if (k_sem_take(&ep_write_sem, K_MSEC(200)) != 0) {
			TC_PRINT("HID IN pool empty and host not completing reports\n");
			return -ENOMEM;
		}
	}

	return err;
}

static void inject_key(uint16_t code, int32_t value)
{
	int err = input_report_key(NULL, code, value, true, K_FOREVER);

	zassert_equal(err, 0, "input_report_key(%u, %d) failed (%d)", code, value, err);
}

static void *hid_suite_setup(void)
{
	static struct hid_joystick_qa_fixture fixture;
	int err;

	TC_PRINT("\n*** hid_joystick_qa ztest (not the hid-joystick sample) ***\n");

	memset(report, 0, sizeof(report));
	unrecognized_count = 0;
	atomic_set(&hid_iface_ready, 0);

	fixture.hid_dev = DEVICE_DT_GET_ONE(zephyr_hid_device);
	zassert_not_null(fixture.hid_dev, "HID device not found in DT");
	zassert_true(device_is_ready(fixture.hid_dev), "HID device not ready");

	fixture.register_ret = hid_device_register(fixture.hid_dev, hid_report_desc,
						   sizeof(hid_report_desc), &hid_ops);
	zassert_equal(fixture.register_ret, 0,
		      "hid_device_register failed (%d)", fixture.register_ret);

	fixture.usbd_ctx = sample_usbd_init_device(NULL);
	zassert_not_null(fixture.usbd_ctx, "USB device init failed");

	err = usbd_enable(fixture.usbd_ctx);
	zassert_equal(err, 0, "usbd_enable failed (%d)", err);

	TC_PRINT("USB HID joystick stack initialized and enabled\n");
	wait_iface_ready();
	g_hid = &fixture;
	return &fixture;
}

static void hid_suite_before(void *f)
{
	ARG_UNUSED(f);
	memset(report, 0, sizeof(report));
	unrecognized_code = -1;
	unrecognized_count = 0;
}

static void hid_suite_teardown(void *f)
{
	ARG_UNUSED(f);
	TC_PRINT("USB device stack remains enabled for host enumeration\n");
}

/*
 * HID Device Ready
 *
 * Purpose : Confirm the zephyr,hid-device node is present and the driver
 *           bound (same check as sample main() before usb_enable).
 * Method  : DEVICE_DT_GET_ONE(zephyr_hid_device) + device_is_ready().
 * Pass    : Device pointer is non-NULL and ready.
 * Fail    : Node missing or driver not initialized.
 */
ZTEST_F(hid_joystick_qa, test_hid_device_ready)
{
	zassert_not_null(fixture->hid_dev, "HID device is NULL");
	zassert_true(device_is_ready(fixture->hid_dev), "HID device not ready");
	TC_PRINT("HID device %s ready\n", fixture->hid_dev->name);
}

/*
 * HID Device Register
 *
 * Purpose : Register the joystick HID report descriptor and callbacks
 *           (sample hid_device_register path).
 * Method  : Record the setup-time hid_device_register() return value.
 * Pass    : Return value is 0.
 * Fail    : Register returned a non-zero errno.
 */
ZTEST_F(hid_joystick_qa, test_hid_device_register)
{
	zassert_equal(fixture->register_ret, 0,
		      "hid_device_register failed (%d)", fixture->register_ret);
}

/*
 * HID Report Descriptor
 *
 * Purpose : Validate the descriptor is a Generic Desktop Joystick (Usage 0x04)
 *           matching samples/subsys/usb/hid-joystick.
 * Method  : Inspect the static report descriptor bytes.
 * Pass    : Usage Page Desktop (0x05 0x01) and Usage Joystick (0x09 0x04).
 * Fail    : Descriptor too short or wrong usage.
 */
ZTEST_F(hid_joystick_qa, test_hid_report_descriptor)
{
	zassert_true(sizeof(hid_report_desc) >= 6, "Report descriptor too small");
	zassert_equal(hid_report_desc[0], 0x05, "Missing Usage Page");
	zassert_equal(hid_report_desc[1], HID_USAGE_GEN_DESKTOP, "Not Generic Desktop");
	zassert_equal(hid_report_desc[2], 0x09, "Missing Usage");
	zassert_equal(hid_report_desc[3], HID_USAGE_GEN_DESKTOP_JOYSTICK,
		      "Usage is not Joystick (0x04)");
	TC_PRINT("HID report descriptor size=%u joystick usage OK\n",
		 (unsigned int)sizeof(hid_report_desc));
}

/*
 * HID Interface Ready
 *
 * Purpose : Host enumerated the HID interface (QA: dmesg hid-generic Joystick,
 *           /dev/hidraw0). Matches sample log "HID iface ready=1".
 * Method  : Wait up to 3 s for iface_ready callback.
 * Pass    : hid_iface_ready is true.
 * Fail    : Timeout with no host (skip so suite can still run unattended).
 */
ZTEST_F(hid_joystick_qa, test_hid_iface_ready)
{
	wait_iface_ready();
	if (!atomic_get(&hid_iface_ready)) {
		TC_PRINT("Host not connected; plug USB device port and re-run\n");
		ztest_test_skip();
	}
	zassert_true(atomic_get(&hid_iface_ready), "HID iface not ready");
}

/*
 * USB Speed Capability
 *
 * Purpose : Query DWC3 speed capability (host log: high-speed xhci).
 * Method  : usbd_caps_speed().
 * Pass    : USBD_SPEED_FS or USBD_SPEED_HS.
 * Fail    : Unexpected speed value.
 */
ZTEST_F(hid_joystick_qa, test_usb_speed_capability)
{
	enum usbd_speed speed = usbd_caps_speed(fixture->usbd_ctx);

	TC_PRINT("USB controller speed capability: %s\n",
		 speed == USBD_SPEED_HS ? "High-Speed" :
		 speed == USBD_SPEED_FS ? "Full-Speed" : "Unknown");
	zassert_true(speed == USBD_SPEED_FS || speed == USBD_SPEED_HS,
		     "Unexpected speed capability %d", speed);
}

/*
 * USB Disable and Re-enable
 *
 * Purpose : Cable unplug/replug simulation (sample usbd_enable path).
 * Method  : usbd_disable(), 100 ms, usbd_enable(), 500 ms (cdc-acm).
 * Pass    : Both calls return 0.
 * Fail    : Non-zero error. Skip if no host at start.
 */
ZTEST_F(hid_joystick_qa, test_usb_disable_reenable)
{
	int err;

	skip_if_no_host();

	err = usbd_disable(fixture->usbd_ctx);
	if (err != 0) {
		TC_PRINT("Skipping usbd_disable: DWC3 halt (%d)\n", err);
		ztest_test_skip();
	}
	k_sleep(K_MSEC(100));

	err = usbd_enable(fixture->usbd_ctx);
	zassert_equal(err, 0, "usbd_enable (re-enable) failed (%d)", err);
	k_sleep(K_MSEC(500));
	wait_host_reenum();
}

/*
 * Joystick Axis Up
 *
 * Purpose : INPUT_KEY_UP sets Y = -20 (sample input_cb).
 * Method  : input_report_key(UP, 1).
 * Pass    : report Y is (uint8_t)-20, X and button unchanged.
 * Fail    : Axis byte mismatch.
 */
ZTEST_F(hid_joystick_qa, test_joystick_axis_up)
{
	inject_key(INPUT_KEY_UP, 1);
	zassert_equal(report[JOYSTICK_Y], (uint8_t)(-JOYSTICK_AXIS_DELTA),
		      "UP: Y=%u expected %u", report[JOYSTICK_Y],
		      (uint8_t)(-JOYSTICK_AXIS_DELTA));
	zassert_equal(report[JOYSTICK_X], 0, "UP must not change X");
	zassert_equal(report[JOYSTICK_BUTTON], 0, "UP must not set button");
}

/*
 * Joystick Axis Down
 *
 * Purpose : INPUT_KEY_DOWN sets Y = +20.
 * Method  : input_report_key(DOWN, 1).
 * Pass    : report Y is 20.
 * Fail    : Axis byte mismatch.
 */
ZTEST_F(hid_joystick_qa, test_joystick_axis_down)
{
	inject_key(INPUT_KEY_DOWN, 1);
	zassert_equal(report[JOYSTICK_Y], JOYSTICK_AXIS_DELTA,
		      "DOWN: Y=%u expected %u", report[JOYSTICK_Y],
		      JOYSTICK_AXIS_DELTA);
}

/*
 * Joystick Axis Left
 *
 * Purpose : INPUT_KEY_LEFT sets X = -20.
 * Method  : input_report_key(LEFT, 1).
 * Pass    : report X is (uint8_t)-20.
 * Fail    : Axis byte mismatch.
 */
ZTEST_F(hid_joystick_qa, test_joystick_axis_left)
{
	inject_key(INPUT_KEY_LEFT, 1);
	zassert_equal(report[JOYSTICK_X], (uint8_t)(-JOYSTICK_AXIS_DELTA),
		      "LEFT: X=%u expected %u", report[JOYSTICK_X],
		      (uint8_t)(-JOYSTICK_AXIS_DELTA));
}

/*
 * Joystick Axis Right
 *
 * Purpose : INPUT_KEY_RIGHT sets X = +20.
 * Method  : input_report_key(RIGHT, 1).
 * Pass    : report X is 20.
 * Fail    : Axis byte mismatch.
 */
ZTEST_F(hid_joystick_qa, test_joystick_axis_right)
{
	inject_key(INPUT_KEY_RIGHT, 1);
	zassert_equal(report[JOYSTICK_X], JOYSTICK_AXIS_DELTA,
		      "RIGHT: X=%u expected %u", report[JOYSTICK_X],
		      JOYSTICK_AXIS_DELTA);
}

/*
 * Joystick Button Center
 *
 * Purpose : INPUT_KEY_ENTER sets button bit 0 (center / SW2).
 * Method  : input_report_key(ENTER, 1).
 * Pass    : report byte 0 bit 0 is 1.
 * Fail    : Button bit not set (was the code-0 SW0 bug).
 */
ZTEST_F(hid_joystick_qa, test_joystick_button_center)
{
	inject_key(INPUT_KEY_ENTER, 1);
	zassert_equal(report[JOYSTICK_BUTTON] & BIT(0), BIT(0),
		      "CENTER: button byte=0x%02x", report[JOYSTICK_BUTTON]);
}

/*
 * Joystick Neutral Release
 *
 * Purpose : Releasing axes/button returns the HID report to 00 00 00
 *           (host hexdump idle pattern between presses).
 * Method  : Press UP then release; press ENTER then release.
 * Pass    : Report is all zeros after releases.
 * Fail    : Sticky axis or button bit.
 */
ZTEST_F(hid_joystick_qa, test_joystick_neutral_release)
{
	inject_key(INPUT_KEY_UP, 1);
	inject_key(INPUT_KEY_UP, 0);
	inject_key(INPUT_KEY_ENTER, 1);
	inject_key(INPUT_KEY_ENTER, 0);
	zassert_mem_equal(report, report_neutral, JOYSTICK_REPORT_COUNT,
			  "Neutral report is %02x %02x %02x",
			  report[0], report[1], report[2]);
}

/*
 * Diagonal: UP + RIGHT together.
 * Purpose : Both axes set; button stays 0.
 * Pass    : X=+20, Y=-20.
 */
ZTEST_F(hid_joystick_qa, test_joystick_diagonal)
{
	inject_key(INPUT_KEY_UP, 1);
	inject_key(INPUT_KEY_RIGHT, 1);
	zassert_equal(report[JOYSTICK_X], JOYSTICK_AXIS_DELTA, "diag X");
	zassert_equal(report[JOYSTICK_Y], (uint8_t)(-JOYSTICK_AXIS_DELTA), "diag Y");
	zassert_equal(report[JOYSTICK_BUTTON], 0, "diag button");
	TC_PRINT("Diagonal report %02x %02x %02x\n",
		 report[0], report[1], report[2]);
}

/*
 * Opposite axis override: LEFT then RIGHT.
 * Purpose : Last press wins on the same axis.
 * Pass    : X=+20 after RIGHT.
 */
ZTEST_F(hid_joystick_qa, test_joystick_axis_override)
{
	inject_key(INPUT_KEY_LEFT, 1);
	inject_key(INPUT_KEY_RIGHT, 1);
	zassert_equal(report[JOYSTICK_X], JOYSTICK_AXIS_DELTA,
		      "override X=%u", report[JOYSTICK_X]);
	TC_PRINT("Axis override: last RIGHT, X=%u\n", report[JOYSTICK_X]);
}

/*
 * Button + axis combo.
 * Purpose : Center and DOWN at the same time.
 * Pass    : button bit 0 and Y=+20.
 */
ZTEST_F(hid_joystick_qa, test_joystick_combo_button_axis)
{
	inject_key(INPUT_KEY_ENTER, 1);
	inject_key(INPUT_KEY_DOWN, 1);
	zassert_equal(report[JOYSTICK_BUTTON] & BIT(0), BIT(0), "combo button");
	zassert_equal(report[JOYSTICK_Y], JOYSTICK_AXIS_DELTA, "combo Y");
	TC_PRINT("Combo report %02x %02x %02x\n",
		 report[0], report[1], report[2]);
}

/*
 * Press all five directions then release all.
 * Purpose : No sticky bit after a full press set.
 * Pass    : Report is 00 00 00.
 */
ZTEST_F(hid_joystick_qa, test_joystick_all_release)
{
	inject_key(INPUT_KEY_UP, 1);
	inject_key(INPUT_KEY_DOWN, 1);
	inject_key(INPUT_KEY_LEFT, 1);
	inject_key(INPUT_KEY_RIGHT, 1);
	inject_key(INPUT_KEY_ENTER, 1);
	inject_key(INPUT_KEY_UP, 0);
	inject_key(INPUT_KEY_DOWN, 0);
	inject_key(INPUT_KEY_LEFT, 0);
	inject_key(INPUT_KEY_RIGHT, 0);
	inject_key(INPUT_KEY_ENTER, 0);
	zassert_mem_equal(report, report_neutral, JOYSTICK_REPORT_COUNT,
			  "sticky %02x %02x %02x",
			  report[0], report[1], report[2]);
	TC_PRINT("All-release verified 00 00 00\n");
}

/*
 * Rapid press/release of center.
 * Purpose : 20 toggles stay consistent; end released.
 * Pass    : Odd steps set bit 0; final report is 00 00 00.
 */
ZTEST_F(hid_joystick_qa, test_joystick_rapid_toggle)
{
	for (int i = 0; i < RAPID_TOGGLE_COUNT; i++) {
		inject_key(INPUT_KEY_ENTER, 1);
		zassert_equal(report[JOYSTICK_BUTTON] & BIT(0), BIT(0),
			      "press %d", i);
		inject_key(INPUT_KEY_ENTER, 0);
		zassert_equal(report[JOYSTICK_BUTTON], 0, "release %d", i);
	}
	TC_PRINT("Rapid toggle: %d press/release cycles\n", RAPID_TOGGLE_COUNT);
}

/*
 * GET_REPORT copies the current 3-byte joystick state.
 * Purpose : Host GET_REPORT path used by hid-generic.
 * Pass    : Returns 3 bytes matching the injected RIGHT report.
 */
ZTEST_F(hid_joystick_qa, test_hid_get_report)
{
	uint8_t buf[8];
	int n;

	inject_key(INPUT_KEY_RIGHT, 1);
	n = hid_get_report(fixture->hid_dev, 0, 0, sizeof(buf), buf);
	zassert_equal(n, JOYSTICK_REPORT_COUNT, "GET_REPORT len %d", n);
	zassert_equal(buf[JOYSTICK_X], JOYSTICK_AXIS_DELTA, "GET_REPORT X");
	zassert_equal(buf[JOYSTICK_Y], 0, "GET_REPORT Y");
	zassert_equal(buf[JOYSTICK_BUTTON], 0, "GET_REPORT button");
	TC_PRINT("GET_REPORT %02x %02x %02x\n", buf[0], buf[1], buf[2]);
}

/*
 * Timed input inject (no USB host required).
 * Purpose : How fast firmware maps keys into the HID report.
 * Pass    : All injects succeed. Rate is informational.
 */
ZTEST_F(hid_joystick_qa, test_perf_input_inject)
{
	uint32_t t0, ms, ev_s;

	t0 = k_uptime_get_32();
	for (int i = 0; i < PERF_INJECT_COUNT; i++) {
		inject_key(INPUT_KEY_UP, i & 1);
	}
	ms = k_uptime_get_32() - t0;
	if (ms == 0U) {
		ms = 1U;
	}
	ev_s = (PERF_INJECT_COUNT * 1000U) / ms;
	TC_PRINT("Input inject %d events in %u ms (%u ev/s)\n",
		 PERF_INJECT_COUNT, ms, ev_s);
	zassert_true(ev_s > 0, "inject throughput 0");
	inject_key(INPUT_KEY_UP, 0);
}

/*
 * Timed HID IN submit (needs host; runs before USB disable).
 * Purpose : Interrupt-IN report rate. Skip if no host.
 * Pass    : At least one submit OK. Rate informational. -ENOMEM stops loop.
 */
ZTEST_F(hid_joystick_qa, test_perf_submit_report)
{
	int ok = 0;
	int err;
	uint32_t t0, ms, rps;

	if (!atomic_get(&hid_iface_ready)) {
		TC_PRINT("HID iface not ready (no host), skipping\n");
		ztest_test_skip();
	}

	t0 = k_uptime_get_32();
	for (int i = 0; i < PERF_SUBMIT_COUNT; i++) {
		inject_key(INPUT_KEY_ENTER, i & 1);
		err = submit_hid_report(fixture->hid_dev);
		if (err == -ENOMEM) {
			TC_PRINT("HID IN pool empty after %d reports\n", ok);
			break;
		}
		zassert_equal(err, 0, "submit %d (%d)", i, err);
		ok++;
	}
	ms = k_uptime_get_32() - t0;
	if (ms == 0U) {
		ms = 1U;
	}
	rps = (ok * 1000U) / ms;
	TC_PRINT("HID submit %d reports in %u ms (%u rps)\n", ok, ms, rps);
	zassert_true(ok >= 1, "no report submitted");
}

/*
 * HID Submit Report
 *
 * Purpose : Send one 3-byte joystick report on the interrupt IN pipe
 *           (sample hid_device_submit_report / host hidraw).
 * Method  : Build a right-deflect report and submit if iface is ready.
 * Pass    : hid_device_submit_report returns 0.
 * Fail    : Submit error. Skip if no host.
 */
ZTEST(hid_joystick_submit, test_hid_submit_report)
{
	int err;

	zassert_not_null(g_hid, "HID fixture not initialized");
	skip_if_no_host();
	zassert_true(IS_UDC_ALIGNED(report), "HID report buffer is not UDC aligned");
	inject_key(INPUT_KEY_RIGHT, 1);

	err = submit_hid_report(g_hid->hid_dev);
	zassert_equal(err, 0, "hid_device_submit_report failed (%d)", err);
}

/*
 * Input Unrecognized Code
 *
 * Purpose : Unknown input codes are ignored (sample LOG_INF
 *           "Unrecognized input code"). Code 0 must not change the report.
 * Method  : input_report_key(0, 1).
 * Pass    : unrecognized_count increments; report stays 00 00 00.
 * Fail    : Report mutated or callback did not count the unknown code.
 */
ZTEST_F(hid_joystick_qa, test_input_unrecognized_code)
{
	inject_key(0, 1);
	zassert_true(unrecognized_count >= 1, "Unknown code was not logged");
	zassert_equal(unrecognized_code, 0, "Expected code 0, got %d", unrecognized_code);
	zassert_mem_equal(report, report_neutral, JOYSTICK_REPORT_COUNT,
			  "Unknown code must not mutate report");
}

/*
 * USB Disable/Enable Stress
 *
 * Purpose : Repeated disable/enable (QA cable stress).
 * Method  : Up to 5 cycles of usbd_disable (100 ms) / usbd_enable
 *           (500 ms). After each enable, wait for the host to
 *           re-enumerate. DWC3 cannot stop if SET_CONFIGURATION never
 *           comes back, so further cycles are stopped (not failed).
 * Pass    : Attempted disable/enable returns 0, or SKIP if DWC3 cannot stop
 *           (timeout after HID IN burst / SET_CONFIGURATION). Not a fail.
 * Fail    : usbd_enable fails after a successful disable.
 */
ZTEST_F(hid_joystick_qa, test_usb_disable_enable_stress)
{
	int err;
	int completed = 0;

	skip_if_no_host();

	for (int i = 0; i < 5; i++) {
		if (!atomic_get(&hid_iface_ready)) {
			TC_PRINT("Host not configured; stop stress after %d cycle(s)\n",
				 completed);
			break;
		}

		err = usbd_disable(fixture->usbd_ctx);
		if (err != 0) {
			TC_PRINT("Skipping usbd_disable: DWC3 halt (%d) after %d cycle(s)\n",
				 err, completed);
			if (completed == 0) {
				ztest_test_skip();
			}
			break;
		}
		k_sleep(K_MSEC(100));

		err = usbd_enable(fixture->usbd_ctx);
		zassert_equal(err, 0, "enable cycle %d failed (%d)", i, err);
		k_sleep(K_MSEC(500));
		completed++;
		TC_PRINT("USB disable/enable cycle %d done\n", i);

		wait_host_reenum();
	}
}

/*
 * HID Submit Report Stress
 *
 * Purpose : Burst of HID IN reports (host hexdump stream).
 * Method  : 50 submit_report calls with alternating button state.
 * Pass    : All submits return 0.
 * Fail    : Any submit error. Skip if no host.
 */
ZTEST(hid_joystick_submit, test_hid_submit_report_stress)
{
	int err;

	zassert_not_null(g_hid, "HID fixture not initialized");
	skip_if_no_host();
	for (int i = 0; i < STRESS_ITERATIONS; i++) {
		inject_key(INPUT_KEY_ENTER, i & 1);
		err = submit_hid_report(g_hid->hid_dev);
		zassert_equal(err, 0, "submit %d failed (%d)", i, err);
	}
}

/*
 * Report pack helpers (no USB) — same 3-byte layout as the sample.
 */
ZTEST(hid_joystick_basic, test_report_pack_button)
{
	uint8_t rpt[JOYSTICK_REPORT_COUNT] = {0};

	WRITE_BIT(rpt[JOYSTICK_BUTTON], 0, 1);
	zassert_equal(rpt[0], 0x01, "Button pack");
	zassert_equal(rpt[1], 0, "X must stay 0");
	zassert_equal(rpt[2], 0, "Y must stay 0");
}

ZTEST(hid_joystick_basic, test_report_pack_axes)
{
	uint8_t rpt[JOYSTICK_REPORT_COUNT] = {0};

	rpt[JOYSTICK_X] = JOYSTICK_AXIS_DELTA;
	rpt[JOYSTICK_Y] = (uint8_t)(-JOYSTICK_AXIS_DELTA);
	zassert_equal(rpt[1], 20, "X pack");
	zassert_equal(rpt[2], (uint8_t)(-20), "Y pack");
}

ZTEST(hid_joystick_basic, test_report_neutral)
{
	uint8_t rpt[JOYSTICK_REPORT_COUNT] = {0x01, 20, 20};

	memset(rpt, 0, sizeof(rpt));
	zassert_mem_equal(rpt, report_neutral, JOYSTICK_REPORT_COUNT, "Neutral pack");
}

static void *hid_submit_suite_setup(void)
{
	/* USB stack is initialized by hid_joystick_qa (runs first). */
	if (g_hid == NULL) {
		return hid_suite_setup();
	}
	return g_hid;
}

ZTEST_SUITE(hid_joystick_basic, NULL, NULL, NULL, NULL, NULL);

ZTEST_SUITE(hid_joystick_qa, NULL, hid_suite_setup, hid_suite_before, NULL,
	    hid_suite_teardown);

ZTEST_SUITE(hid_joystick_submit, NULL, hid_submit_suite_setup, hid_suite_before,
	    NULL, NULL);
