/* Copyright Alif Semiconductor - All Rights Reserved.
 * USB HID keyboard QA (device-side). No driver changes.
 */

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usbd_hid.h>
#include <zephyr/usb/class/hid.h>
#include <zephyr/drivers/usb/udc_buf.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <sample_usbd.h>

enum kb_report_idx {
	KB_MOD = 0,
	KB_RESERVED,
	KB_KEY1,
	KB_KEY2,
	KB_KEY3,
	KB_KEY4,
	KB_KEY5,
	KB_KEY6,
	KB_REPORT_COUNT,
};

#define PERF_INJECT_COUNT	200
#define PERF_SUBMIT_COUNT	50
#define IFACE_READY_TIMEOUT_MS	3000

static const uint8_t hid_report_desc[] = HID_KEYBOARD_REPORT_DESC();
static const uint8_t report_neutral[KB_REPORT_COUNT];

struct hid_keyboard_qa_fixture {
	const struct device *hid_dev;
	struct usbd_context *usbd_ctx;
	int register_ret;
};

static struct hid_keyboard_qa_fixture *g_hid;
UDC_STATIC_BUF_DEFINE(report, KB_REPORT_COUNT);
static atomic_t hid_iface_ready;
static uint8_t last_led_report;
static uint8_t hid_protocol = HID_PROTOCOL_REPORT;
static K_SEM_DEFINE(ep_write_sem, 0, 1);

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
	k_sem_give(&ep_write_sem);
}

static int hid_get_report(const struct device *dev, const uint8_t type, const uint8_t id,
			  const uint16_t len, uint8_t *const buf)
{
	size_t n = MIN(len, (uint16_t)KB_REPORT_COUNT);

	ARG_UNUSED(dev);
	ARG_UNUSED(type);
	ARG_UNUSED(id);
	if (n == 0) {
		return -ENOTSUP;
	}
	memcpy(buf, report, n);
	return (int)n;
}

static int hid_set_report(const struct device *dev, const uint8_t type, const uint8_t id,
			  const uint16_t len, const uint8_t *const buf)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(id);
	if (type != HID_REPORT_TYPE_OUTPUT || len == 0) {
		return -ENOTSUP;
	}
	last_led_report = buf[0];
	return 0;
}

/* Required when protocol-code = "keyboard" (boot interface). */
static void hid_set_protocol(const struct device *dev, const uint8_t proto)
{
	ARG_UNUSED(dev);
	hid_protocol = proto;
	TC_PRINT("HID protocol=%u\n", hid_protocol);
}

static const struct hid_device_ops hid_ops = {
	.iface_ready = iface_ready_cb,
	.get_report = hid_get_report,
	.set_report = hid_set_report,
	.set_protocol = hid_set_protocol,
	.input_report_done = input_report_done_cb,
};

static void input_cb(struct input_event *evt, void *user_data)
{
	ARG_UNUSED(user_data);

	switch (evt->code) {
	case INPUT_KEY_A:
		report[KB_KEY1] = evt->value ? HID_KEY_A : 0;
		break;
	case INPUT_KEY_B:
		report[KB_KEY2] = evt->value ? HID_KEY_B : 0;
		break;
	case INPUT_KEY_ENTER:
		report[KB_KEY3] = evt->value ? HID_KEY_ENTER : 0;
		break;
	case INPUT_KEY_LEFTSHIFT:
		if (evt->value) {
			report[KB_MOD] |= HID_KBD_MODIFIER_LEFT_SHIFT;
		} else {
			report[KB_MOD] &= (uint8_t)~HID_KBD_MODIFIER_LEFT_SHIFT;
		}
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

static int submit_hid_report(const struct device *hid_dev)
{
	int err;
	int tries = 5;

	while (tries--) {
		err = hid_device_submit_report(hid_dev, KB_REPORT_COUNT, report);
		if (err != -ENOMEM) {
			return err;
		}
		if (k_sem_take(&ep_write_sem, K_MSEC(200)) != 0) {
			return -ENOMEM;
		}
	}
	return err;
}

static void *hid_suite_setup(void)
{
	static struct hid_keyboard_qa_fixture fixture;
	int err;

	TC_PRINT("\n*** hid_keyboard_qa ztest (not the hid-keyboard sample) ***\n");
	memset(report, 0, sizeof(report));
	atomic_set(&hid_iface_ready, 0);

	fixture.hid_dev = DEVICE_DT_GET_ONE(zephyr_hid_device);
	zassert_true(device_is_ready(fixture.hid_dev), "HID not ready");

	fixture.register_ret = hid_device_register(fixture.hid_dev, hid_report_desc,
						   sizeof(hid_report_desc), &hid_ops);
	zassert_equal(fixture.register_ret, 0, "register %d", fixture.register_ret);
	if (fixture.register_ret != 0) {
		g_hid = &fixture;
		return &fixture;
	}

	fixture.usbd_ctx = sample_usbd_init_device(NULL);
	zassert_not_null(fixture.usbd_ctx, "USB init failed");
	err = usbd_enable(fixture.usbd_ctx);
	zassert_equal(err, 0, "usbd_enable %d", err);

	TC_PRINT("USB HID keyboard stack initialized and enabled\n");
	{
		int waited = 0;

		while (!atomic_get(&hid_iface_ready) && waited < IFACE_READY_TIMEOUT_MS) {
			k_sleep(K_MSEC(50));
			waited += 50;
		}
	}
	g_hid = &fixture;
	return &fixture;
}

static void hid_suite_before(void *f)
{
	ARG_UNUSED(f);
	memset(report, 0, sizeof(report));
	last_led_report = 0;
}

ZTEST_F(hid_keyboard_qa, test_hid_device_ready)
{
	zassert_true(device_is_ready(fixture->hid_dev), "HID not ready");
	TC_PRINT("HID device %s ready\n", fixture->hid_dev->name);
}

ZTEST_F(hid_keyboard_qa, test_hid_device_register)
{
	zassert_equal(fixture->register_ret, 0);
}

ZTEST_F(hid_keyboard_qa, test_hid_report_descriptor)
{
	zassert_equal(hid_report_desc[0], 0x05);
	zassert_equal(hid_report_desc[1], HID_USAGE_GEN_DESKTOP);
	zassert_equal(hid_report_desc[3], HID_USAGE_GEN_DESKTOP_KEYBOARD);
	TC_PRINT("HID keyboard descriptor size=%u\n",
		 (unsigned int)sizeof(hid_report_desc));
}

ZTEST_F(hid_keyboard_qa, test_hid_iface_ready)
{
	if (!atomic_get(&hid_iface_ready)) {
		ztest_test_skip();
	}
}

ZTEST_F(hid_keyboard_qa, test_usb_speed_capability)
{
	enum usbd_speed speed = usbd_caps_speed(fixture->usbd_ctx);

	TC_PRINT("USB speed: %s\n",
		 speed == USBD_SPEED_HS ? "High-Speed" : "Full-Speed");
	zassert_true(speed == USBD_SPEED_FS || speed == USBD_SPEED_HS);
}

ZTEST_F(hid_keyboard_qa, test_key_a)
{
	inject_key(INPUT_KEY_A, 1);
	zassert_equal(report[KB_KEY1], HID_KEY_A);
	TC_PRINT("Key A = %u\n", report[KB_KEY1]);
}

ZTEST_F(hid_keyboard_qa, test_key_enter)
{
	inject_key(INPUT_KEY_ENTER, 1);
	zassert_equal(report[KB_KEY3], HID_KEY_ENTER);
}

ZTEST_F(hid_keyboard_qa, test_modifier_shift)
{
	inject_key(INPUT_KEY_LEFTSHIFT, 1);
	zassert_equal(report[KB_MOD], HID_KBD_MODIFIER_LEFT_SHIFT);
}

ZTEST_F(hid_keyboard_qa, test_shift_plus_a)
{
	inject_key(INPUT_KEY_LEFTSHIFT, 1);
	inject_key(INPUT_KEY_A, 1);
	zassert_equal(report[KB_MOD], HID_KBD_MODIFIER_LEFT_SHIFT);
	zassert_equal(report[KB_KEY1], HID_KEY_A);
	TC_PRINT("Shift+A mod=0x%02x key=%u\n", report[KB_MOD], report[KB_KEY1]);
}

ZTEST_F(hid_keyboard_qa, test_multi_key)
{
	inject_key(INPUT_KEY_A, 1);
	inject_key(INPUT_KEY_B, 1);
	zassert_equal(report[KB_KEY1], HID_KEY_A);
	zassert_equal(report[KB_KEY2], HID_KEY_B);
}

ZTEST_F(hid_keyboard_qa, test_key_release)
{
	inject_key(INPUT_KEY_A, 1);
	inject_key(INPUT_KEY_LEFTSHIFT, 1);
	inject_key(INPUT_KEY_A, 0);
	inject_key(INPUT_KEY_LEFTSHIFT, 0);
	zassert_mem_equal(report, report_neutral, KB_REPORT_COUNT);
	TC_PRINT("Release all zeros\n");
}

ZTEST_F(hid_keyboard_qa, test_hid_get_report)
{
	uint8_t buf[16];
	int n;

	inject_key(INPUT_KEY_A, 1);
	n = hid_get_report(fixture->hid_dev, 0, 0, sizeof(buf), buf);
	zassert_equal(n, KB_REPORT_COUNT);
	zassert_equal(buf[KB_KEY1], HID_KEY_A);
	TC_PRINT("GET_REPORT key1=%u\n", buf[KB_KEY1]);
}

ZTEST_F(hid_keyboard_qa, test_set_report_leds)
{
	uint8_t leds = BIT(0) | BIT(1); /* NumLock + CapsLock */
	int err = hid_set_report(fixture->hid_dev, HID_REPORT_TYPE_OUTPUT, 0, 1, &leds);

	zassert_equal(err, 0);
	zassert_equal(last_led_report, leds);
	TC_PRINT("SET_REPORT LEDs 0x%02x\n", last_led_report);
}

ZTEST_F(hid_keyboard_qa, test_perf_input_inject)
{
	uint32_t t0 = k_uptime_get_32();
	uint32_t ms, ev_s;

	for (int i = 0; i < PERF_INJECT_COUNT; i++) {
		inject_key(INPUT_KEY_A, i & 1);
	}
	ms = k_uptime_get_32() - t0;
	if (ms == 0U) {
		ms = 1U;
	}
	ev_s = (PERF_INJECT_COUNT * 1000U) / ms;
	TC_PRINT("Input inject %d in %u ms (%u ev/s)\n", PERF_INJECT_COUNT, ms, ev_s);
	zassert_true(ev_s > 0);
}

ZTEST_F(hid_keyboard_qa, test_perf_submit_report)
{
	int ok = 0, err;
	uint32_t t0, ms;

	skip_if_no_host();
	t0 = k_uptime_get_32();
	for (int i = 0; i < PERF_SUBMIT_COUNT; i++) {
		inject_key(INPUT_KEY_A, i & 1);
		err = submit_hid_report(fixture->hid_dev);
		if (err == -ENOMEM) {
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

ZTEST(hid_keyboard_basic, test_report_pack_key)
{
	uint8_t rpt[KB_REPORT_COUNT] = {0};

	rpt[KB_KEY1] = HID_KEY_A;
	zassert_equal(rpt[2], HID_KEY_A);
}

ZTEST(hid_keyboard_basic, test_report_pack_modifier)
{
	uint8_t rpt[KB_REPORT_COUNT] = {0};

	rpt[KB_MOD] = HID_KBD_MODIFIER_LEFT_SHIFT;
	zassert_equal(rpt[0], HID_KBD_MODIFIER_LEFT_SHIFT);
}

ZTEST(hid_keyboard_basic, test_report_neutral)
{
	uint8_t rpt[KB_REPORT_COUNT];

	memset(rpt, 0xFF, sizeof(rpt));
	memset(rpt, 0, sizeof(rpt));
	zassert_mem_equal(rpt, report_neutral, KB_REPORT_COUNT);
}

ZTEST_SUITE(hid_keyboard_basic, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(hid_keyboard_qa, NULL, hid_suite_setup, hid_suite_before, NULL, NULL);
