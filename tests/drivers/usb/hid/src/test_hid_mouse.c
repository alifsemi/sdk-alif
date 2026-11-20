/* Copyright Alif Semiconductor - All Rights Reserved.
 * USB HID mouse QA (device-side). No driver changes.
 */

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usb_hid.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/usb/class/hid.h>
#include <zephyr/drivers/usb/udc_buf.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <sample_usbd.h>

#define MOUSE_BTN_LEFT		0
#define MOUSE_BTN_RIGHT		1
#define MOUSE_MOVE		10
#define PERF_INJECT_COUNT	200
#define PERF_SUBMIT_COUNT	8
#define IFACE_READY_TIMEOUT_MS	3000
#define HID_HOST_SETTLE_MS	3000
#define MANUAL_PERIOD_MS	800

enum mouse_report_idx {
	MOUSE_BTN = 0,
	MOUSE_X,
	MOUSE_Y,
	MOUSE_WHEEL,
	MOUSE_REPORT_COUNT,
};

static const uint8_t hid_report_desc[] = HID_MOUSE_REPORT_DESC(2);
static const uint8_t report_neutral[MOUSE_REPORT_COUNT];

struct hid_mouse_qa_fixture {
	const struct device *hid_dev;
	struct usbd_context *usbd_ctx;
	int register_ret;
};

static struct hid_mouse_qa_fixture *g_hid;
UDC_STATIC_BUF_DEFINE(report, MOUSE_REPORT_COUNT);
UDC_STATIC_BUF_DEFINE(in_report, MOUSE_REPORT_COUNT);
static atomic_t hid_iface_ready;
static atomic_t hid_in_flight;
static K_SEM_DEFINE(ep_write_sem, 0, 1);
K_THREAD_STACK_DEFINE(manual_stack, 2048);
static struct k_thread manual_tid;

static void iface_ready_cb(const struct device *dev, const bool ready)
{
	ARG_UNUSED(dev);
	atomic_set(&hid_iface_ready, ready ? 1 : 0);
	TC_PRINT("HID iface ready=%d\n", ready ? 1 : 0);
}

static void input_report_done_cb(const struct device *dev, const uint8_t *const rpt)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(rpt);
	atomic_set(&hid_in_flight, 0);
	k_sem_give(&ep_write_sem);
}

static int hid_get_report(const struct device *dev, const uint8_t type, const uint8_t id,
			  const uint16_t len, uint8_t *const buf)
{
	size_t n = MIN(len, (uint16_t)MOUSE_REPORT_COUNT);

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
	case INPUT_KEY_0:
		WRITE_BIT(report[MOUSE_BTN], MOUSE_BTN_LEFT, evt->value);
		break;
	case INPUT_KEY_1:
		WRITE_BIT(report[MOUSE_BTN], MOUSE_BTN_RIGHT, evt->value);
		break;
	case INPUT_KEY_2:
		report[MOUSE_X] = evt->value ? MOUSE_MOVE : 0;
		break;
	case INPUT_KEY_3:
		report[MOUSE_Y] = evt->value ? MOUSE_MOVE : 0;
		break;
	case INPUT_KEY_4:
		report[MOUSE_WHEEL] = evt->value ? 1 : 0;
		break;
	default:
		return;
	}
}

INPUT_CALLBACK_DEFINE(NULL, input_cb, NULL);

static void skip_if_no_host(void)
{
	if (!atomic_get(&hid_iface_ready)) {
		TC_PRINT("HID iface not ready (no host), skipping\n");
		ztest_test_skip();
	}
}

static void inject_key(uint16_t code, int32_t value)
{
	int err = input_report_key(NULL, code, value, true, K_FOREVER);

	zassert_equal(err, 0, "input_report_key(%u,%d) %d", code, value, err);
}

/*
 * One IN at a time, dedicated DMA buffer. Reusing `report` for many
 * outstanding submits wedges DWC3 (pool empty, evtest silent).
 * Do not wait for ACK in ztest; return -EBUSY if the previous IN is live.
 */
static int submit_hid_report(const struct device *hid_dev)
{
	int err;

	if (atomic_get(&hid_in_flight)) {
		if (k_sem_take(&ep_write_sem, K_NO_WAIT) != 0) {
			return -EBUSY;
		}
		atomic_set(&hid_in_flight, 0);
	}
	memcpy(in_report, report, MOUSE_REPORT_COUNT);
	err = hid_device_submit_report(hid_dev, MOUSE_REPORT_COUNT, in_report);
	if (err == 0) {
		atomic_set(&hid_in_flight, 1);
	}
	return err;
}

/* After SUCCESS: one REL_X per host ACK. Open evtest; do not reset. */
static void manual_host_thread(void *p1, void *p2, void *p3)
{
	const struct device *hid_dev = p1;
	int err;
	unsigned int n;
	unsigned int waits = 0;

	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
	k_sleep(K_MSEC(HID_HOST_SETTLE_MS));
	TC_PRINT("Manual host demo: open evtest now (Alif HID Mouse); no reset\n");
	for (n = 1; atomic_get(&hid_iface_ready); n++) {
		memset(report, 0, MOUSE_REPORT_COUNT);
		report[MOUSE_X] = MOUSE_MOVE;
		err = submit_hid_report(hid_dev);
		if (err == -EBUSY) {
			if (k_sem_take(&ep_write_sem, K_MSEC(MANUAL_PERIOD_MS)) != 0) {
				waits++;
				if ((waits % 8U) == 1U) {
					TC_PRINT("demo waiting for host IN ACK (evtest open?)\n");
				}
				continue;
			}
			atomic_set(&hid_in_flight, 0);
			err = submit_hid_report(hid_dev);
		}
		if (err != 0) {
			TC_PRINT("demo submit %u %d\n", n, err);
			k_sleep(K_MSEC(MANUAL_PERIOD_MS));
			continue;
		}
		TC_PRINT("demo IN %u queued (REL_X 10)\n", n);
		if (k_sem_take(&ep_write_sem, K_MSEC(2000)) == 0) {
			atomic_set(&hid_in_flight, 0);
			TC_PRINT("demo IN %u ACKed — check evtest\n", n);
		}
		k_sleep(K_MSEC(MANUAL_PERIOD_MS));
	}
}

static void hid_suite_teardown(void *f)
{
	struct hid_mouse_qa_fixture *fix = f;

	if (fix == NULL || !atomic_get(&hid_iface_ready)) {
		TC_PRINT("No host; skip manual demo\n");
		return;
	}
	k_thread_create(&manual_tid, manual_stack, K_THREAD_STACK_SIZEOF(manual_stack),
			manual_host_thread, (void *)fix->hid_dev, NULL, NULL,
			K_PRIO_PREEMPT(8), 0, K_NO_WAIT);
	k_thread_name_set(&manual_tid, "hid_mouse_manual");
}

static void *hid_suite_setup(void)
{
	static struct hid_mouse_qa_fixture fixture;
	int err;

	TC_PRINT("\n*** hid_mouse_qa ztest (not the hid-mouse sample) ***\n");
	memset(report, 0, sizeof(report));
	atomic_set(&hid_iface_ready, 0);
	atomic_set(&hid_in_flight, 0);

	fixture.hid_dev = DEVICE_DT_GET_ONE(zephyr_hid_device);
	zassert_true(device_is_ready(fixture.hid_dev), "HID not ready");

	fixture.register_ret = hid_device_register(fixture.hid_dev, hid_report_desc,
						   sizeof(hid_report_desc), &hid_ops);
	zassert_equal(fixture.register_ret, 0, "register %d", fixture.register_ret);

	fixture.usbd_ctx = sample_usbd_init_device(NULL);
	zassert_not_null(fixture.usbd_ctx, "USB init failed");
	err = usbd_enable(fixture.usbd_ctx);
	zassert_equal(err, 0, "usbd_enable %d", err);

	TC_PRINT("USB HID mouse stack initialized and enabled\n");
	{
		int waited = 0;

		while (!atomic_get(&hid_iface_ready) && waited < IFACE_READY_TIMEOUT_MS) {
			k_sleep(K_MSEC(50));
			waited += 50;
		}
	}
	if (atomic_get(&hid_iface_ready)) {
		TC_PRINT("Waiting %u ms for host usbhid bind\n", HID_HOST_SETTLE_MS);
		k_sleep(K_MSEC(HID_HOST_SETTLE_MS));
	}
	g_hid = &fixture;
	return &fixture;
}

static void hid_suite_before(void *f)
{
	ARG_UNUSED(f);
	memset(report, 0, sizeof(report));
}

ZTEST_F(hid_mouse_qa, test_hid_device_ready)
{
	zassert_true(device_is_ready(fixture->hid_dev), "HID not ready");
	TC_PRINT("HID device %s ready\n", fixture->hid_dev->name);
}

ZTEST_F(hid_mouse_qa, test_hid_device_register)
{
	zassert_equal(fixture->register_ret, 0);
}

ZTEST_F(hid_mouse_qa, test_hid_report_descriptor)
{
	zassert_equal(hid_report_desc[0], 0x05);
	zassert_equal(hid_report_desc[1], HID_USAGE_GEN_DESKTOP);
	zassert_equal(hid_report_desc[3], HID_USAGE_GEN_DESKTOP_MOUSE);
	TC_PRINT("HID mouse descriptor size=%u\n", (unsigned int)sizeof(hid_report_desc));
}

ZTEST_F(hid_mouse_qa, test_hid_iface_ready)
{
	if (!atomic_get(&hid_iface_ready)) {
		ztest_test_skip();
	}
}

ZTEST_F(hid_mouse_qa, test_usb_speed_capability)
{
	enum usbd_speed speed = usbd_caps_speed(fixture->usbd_ctx);

	TC_PRINT("USB speed: %s\n",
		 speed == USBD_SPEED_HS ? "High-Speed" : "Full-Speed");
	zassert_true(speed == USBD_SPEED_FS || speed == USBD_SPEED_HS);
}

ZTEST_F(hid_mouse_qa, test_mouse_left_button)
{
	inject_key(INPUT_KEY_0, 1);
	zassert_equal(report[MOUSE_BTN] & BIT(MOUSE_BTN_LEFT), BIT(MOUSE_BTN_LEFT));
	TC_PRINT("Left button 0x%02x\n", report[MOUSE_BTN]);
}

ZTEST_F(hid_mouse_qa, test_mouse_right_button)
{
	inject_key(INPUT_KEY_1, 1);
	zassert_equal(report[MOUSE_BTN] & BIT(MOUSE_BTN_RIGHT), BIT(MOUSE_BTN_RIGHT));
}

ZTEST_F(hid_mouse_qa, test_mouse_move_x)
{
	inject_key(INPUT_KEY_2, 1);
	zassert_equal(report[MOUSE_X], MOUSE_MOVE);
}

ZTEST_F(hid_mouse_qa, test_mouse_move_y)
{
	inject_key(INPUT_KEY_3, 1);
	zassert_equal(report[MOUSE_Y], MOUSE_MOVE);
}

ZTEST_F(hid_mouse_qa, test_mouse_wheel)
{
	inject_key(INPUT_KEY_4, 1);
	zassert_equal(report[MOUSE_WHEEL], 1);
}

ZTEST_F(hid_mouse_qa, test_mouse_combo)
{
	inject_key(INPUT_KEY_0, 1);
	inject_key(INPUT_KEY_2, 1);
	inject_key(INPUT_KEY_3, 1);
	zassert_equal(report[MOUSE_BTN] & BIT(0), BIT(0));
	zassert_equal(report[MOUSE_X], MOUSE_MOVE);
	zassert_equal(report[MOUSE_Y], MOUSE_MOVE);
	TC_PRINT("Combo %02x %02x %02x %02x\n",
		 report[0], report[1], report[2], report[3]);
}

ZTEST_F(hid_mouse_qa, test_mouse_release)
{
	inject_key(INPUT_KEY_0, 1);
	inject_key(INPUT_KEY_2, 1);
	inject_key(INPUT_KEY_0, 0);
	inject_key(INPUT_KEY_2, 0);
	zassert_mem_equal(report, report_neutral, MOUSE_REPORT_COUNT);
	TC_PRINT("Release 00 00 00 00\n");
}

ZTEST_F(hid_mouse_qa, test_hid_get_report)
{
	uint8_t buf[8];
	int n;

	inject_key(INPUT_KEY_2, 1);
	n = hid_get_report(fixture->hid_dev, 0, 0, sizeof(buf), buf);
	zassert_equal(n, MOUSE_REPORT_COUNT);
	zassert_equal(buf[MOUSE_X], MOUSE_MOVE);
	TC_PRINT("GET_REPORT %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3]);
}

ZTEST_F(hid_mouse_qa, test_perf_input_inject)
{
	uint32_t t0 = k_uptime_get_32();
	uint32_t ms, ev_s;

	for (int i = 0; i < PERF_INJECT_COUNT; i++) {
		inject_key(INPUT_KEY_2, i & 1);
	}
	ms = k_uptime_get_32() - t0;
	if (ms == 0U) {
		ms = 1U;
	}
	ev_s = (PERF_INJECT_COUNT * 1000U) / ms;
	TC_PRINT("Input inject %d in %u ms (%u ev/s)\n", PERF_INJECT_COUNT, ms, ev_s);
	zassert_true(ev_s > 0);
}

ZTEST_F(hid_mouse_qa, test_perf_submit_report)
{
	int ok = 0, err;
	uint32_t t0, ms;

	skip_if_no_host();
	t0 = k_uptime_get_32();
	for (int i = 0; i < PERF_SUBMIT_COUNT; i++) {
		inject_key(INPUT_KEY_0, i & 1);
		err = submit_hid_report(fixture->hid_dev);
		if (err == -EBUSY) {
			(void)k_sem_take(&ep_write_sem, K_MSEC(20));
			atomic_set(&hid_in_flight, 0);
			err = submit_hid_report(fixture->hid_dev);
		}
		if (err == -ENOMEM || err == -EBUSY) {
			break;
		}
		zassert_equal(err, 0, "submit %d %d", i, err);
		ok++;
	}
	ms = k_uptime_get_32() - t0;
	if (ms == 0U) {
		ms = 1U;
	}
	TC_PRINT("HID submit %d reports in %u ms (%u rps)\n",
		 ok, ms, (ok * 1000U) / ms);
	zassert_true(ok >= 1);
}

ZTEST(hid_mouse_basic, test_report_pack_buttons)
{
	uint8_t rpt[MOUSE_REPORT_COUNT] = {0};

	WRITE_BIT(rpt[MOUSE_BTN], MOUSE_BTN_LEFT, 1);
	WRITE_BIT(rpt[MOUSE_BTN], MOUSE_BTN_RIGHT, 1);
	zassert_equal(rpt[0], 0x03);
}

ZTEST(hid_mouse_basic, test_report_pack_move)
{
	uint8_t rpt[MOUSE_REPORT_COUNT] = {0};

	rpt[MOUSE_X] = MOUSE_MOVE;
	rpt[MOUSE_Y] = MOUSE_MOVE;
	zassert_equal(rpt[1], 10);
	zassert_equal(rpt[2], 10);
}

ZTEST(hid_mouse_basic, test_report_neutral)
{
	uint8_t rpt[MOUSE_REPORT_COUNT] = {1, 2, 3, 4};

	memset(rpt, 0, sizeof(rpt));
	zassert_mem_equal(rpt, report_neutral, MOUSE_REPORT_COUNT);
}

ZTEST_SUITE(hid_mouse_basic, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(hid_mouse_qa, NULL, hid_suite_setup, hid_suite_before, NULL,
	    hid_suite_teardown);
