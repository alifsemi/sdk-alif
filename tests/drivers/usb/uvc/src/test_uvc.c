/*
 * Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement.
 *
 * Device-side ztest for the E8 DK UVC webcam pipeline. One sensor per image:
 *   MT9M114  Y10P 1288x728 -> NV12/JPEG 1280x720
 *   ARX3A0   Y10P 560x560  -> NV12/JPEG 560x560
 *   OV5675   Y10P 1296x972 -> NV12/JPEG 1024x768
 *
 * Uses the public video and UVC APIs. Does not call usbd_disable: on DWC3
 * that drops the host webcam. After the suite a thread waits for the host
 * COMMIT and streams, so ffplay can be opened once the UART shows SUCCESS.
 */

#include "pipeline.h"

#include <sample_usbd.h>

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/cache.h>
#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video-controls.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usbd_uvc.h>
#include <zephyr/sys/util.h>

#include <string.h>
#include <errno.h>

#if !UVC_OUTPUT_MJPEG
#error "UVC QA expects the E8 JPEG pipeline (jpeg0 okay)"
#endif

#if DT_NODE_EXISTS(DT_NODELABEL(mt9m114_selfie))
#define UVC_SENSOR_NAME       "MT9M114"
#define UVC_EXPECT_NV12_BYTES 1382400U
#define UVC_EXPECT_JPEG_BYTES 921600U
#define UVC_EXPECT_ISP_PITCH  2576U
#elif DT_NODE_EXISTS(DT_NODELABEL(ov5675_selfie))
#define UVC_SENSOR_NAME       "OV5675"
#define UVC_EXPECT_NV12_BYTES 1179648U
#define UVC_EXPECT_JPEG_BYTES 786432U
#define UVC_EXPECT_ISP_PITCH  2592U
#elif DT_NODE_EXISTS(DT_NODELABEL(arx3a0_selfie))
#define UVC_SENSOR_NAME       "ARX3A0"
#define UVC_EXPECT_NV12_BYTES 470400U
#define UVC_EXPECT_JPEG_BYTES 313600U
#define UVC_EXPECT_ISP_PITCH  1120U
#else
#error "Pass the ARX3A0, MT9M114, or OV5675 EXTRA_DTC_OVERLAY_FILE"
#endif

#define N_CAPTURE_BUFFERS 2
#define CAPTURE_TIMEOUT   K_MSEC(3000)
#define STRESS_POLLS      30
#define STRESS_RESTARTS   4
#define STRESS_JPEGS      8
#define PERF_FRAMES       4

struct uvc_ctx {
	const struct device *isp;
	const struct device *jpeg;
	const struct device *uvc;
	struct usbd_context *usbd;
	struct video_buffer *cap[N_CAPTURE_BUFFERS];
	struct video_buffer *jpeg_buf;
};

static struct uvc_ctx ctx;
static struct uvc_qa_fixture {
	struct uvc_ctx *ctx;
} qa_fixture;

static atomic_t usb_bus_active = ATOMIC_INIT(1);

static void usbd_msg_cb(struct usbd_context *const uds, const struct usbd_msg *const msg)
{
	ARG_UNUSED(uds);

	switch (msg->type) {
	case USBD_MSG_VBUS_REMOVED:
	case USBD_MSG_SUSPEND:
		atomic_set(&usb_bus_active, 0);
		break;
	case USBD_MSG_VBUS_READY:
	case USBD_MSG_RESUME:
		atomic_set(&usb_bus_active, 1);
		break;
	default:
		break;
	}
}

static bool caps_have(const struct video_caps *caps, uint32_t fourcc,
		      uint32_t width, uint32_t height)
{
	const struct video_format_cap *cap;

	if (caps == NULL || caps->format_caps == NULL) {
		return false;
	}

	for (cap = caps->format_caps; cap->pixelformat != 0; cap++) {
		if (cap->pixelformat == fourcc &&
		    width >= cap->width_min && width <= cap->width_max &&
		    height >= cap->height_min && height <= cap->height_max) {
			return true;
		}
	}

	return false;
}

static void isp_reclaim(void)
{
	struct video_buffer *vbuf;

	if (ctx.isp == NULL) {
		return;
	}

	video_stream_stop(ctx.isp);
	video_flush(ctx.isp, VIDEO_EP_OUT, true);
	while (video_dequeue(ctx.isp, VIDEO_EP_OUT, &vbuf, K_NO_WAIT) == 0) {
	}
}

static void jpeg_reclaim(void)
{
	struct video_buffer *vbuf;

	if (ctx.jpeg == NULL) {
		return;
	}

	video_stream_stop(ctx.jpeg);
	while (video_dequeue(ctx.jpeg, VIDEO_EP_OUT, &vbuf, K_NO_WAIT) == 0) {
	}
}

/* Cancel path returns the buffer. Do not call video_stream_stop() here:
 * that sets the class paused bit. flush() clears it before returning.
 */
static void uvc_reclaim(void)
{
	struct video_buffer *vbuf;

	if (ctx.uvc == NULL) {
		return;
	}

	video_flush(ctx.uvc, VIDEO_EP_IN, true);
	while (video_dequeue(ctx.uvc, VIDEO_EP_OUT, &vbuf, K_NO_WAIT) == 0) {
	}
}

static int capture_one(struct video_buffer **out)
{
	int ret;

	for (size_t i = 0; i < N_CAPTURE_BUFFERS; i++) {
		ret = video_enqueue(ctx.isp, VIDEO_EP_OUT, ctx.cap[i]);
		if (ret) {
			return ret;
		}
	}

	ret = video_stream_start(ctx.isp);
	if (ret) {
		return ret;
	}

	ret = video_dequeue(ctx.isp, VIDEO_EP_OUT, out, CAPTURE_TIMEOUT);
	if (ret == 0) {
		sys_cache_data_invd_range((*out)->buffer, CAPTURE_FRAME_BYTES);
	}

	return ret;
}

static int jpeg_encode(struct video_buffer *nv12, struct video_buffer *out, size_t *out_len)
{
	struct video_format jfmt = {
		.pixelformat = CAPTURE_FORMAT,
		.width = CAPTURE_WIDTH,
		.height = CAPTURE_HEIGHT,
		.pitch = JPEG_INPUT_PITCH,
	};
	struct video_buffer *deq = NULL;
	uint16_t quality = JPEG_QUALITY;
	int ret;

	ret = video_set_format(ctx.jpeg, VIDEO_EP_IN, &jfmt);
	if (ret) {
		return ret;
	}

	ret = video_set_ctrl(ctx.jpeg, VIDEO_CID_JPEG_COMPRESSION_QUALITY, &quality);
	if (ret) {
		return ret;
	}

	ret = video_set_ctrl(ctx.jpeg, VIDEO_CID_JPEG_INPUT_BUFFER, nv12->buffer);
	if (ret) {
		return ret;
	}

	out->bytesused = out->size - CONFIG_VIDEO_JPEG_HANTRO_VC9000E_HEADER_SIZE;

	ret = video_enqueue(ctx.jpeg, VIDEO_EP_OUT, out);
	if (ret) {
		jpeg_reclaim();
		return ret;
	}

	ret = video_stream_start(ctx.jpeg);
	if (ret && ret != -EALREADY) {
		jpeg_reclaim();
		return ret;
	}

	ret = video_dequeue(ctx.jpeg, VIDEO_EP_OUT, &deq, K_SECONDS(5));
	if (ret == 0) {
		*out_len = deq->bytesused;
	}
	jpeg_reclaim();
	return ret;
}

static bool uvc_committed(struct video_format *fmt)
{
	return atomic_get(&usb_bus_active) != 0 &&
	       video_get_format(ctx.uvc, VIDEO_EP_OUT, fmt) == 0;
}

static int rearm_capture(struct video_buffer *vbuf)
{
	int ret;

	ret = video_enqueue(ctx.isp, VIDEO_EP_OUT, vbuf);
	if (ret) {
		return ret;
	}

	ret = video_stream_start(ctx.isp);
	if (ret && ret != -EBUSY) {
		return ret;
	}

	return 0;
}

static void uvc_demo(void *p1, void *p2, void *p3)
{
	struct video_buffer *vbuf;
	struct video_buffer *sent;
	size_t send_len;
	int ret;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	printk("UVC host demo: %s MJPEG %ux%u. Waiting for COMMIT.\n",
	       UVC_SENSOR_NAME, CAPTURE_WIDTH, CAPTURE_HEIGHT);

	while (1) {
		struct video_format neg;

		if (!uvc_committed(&neg)) {
			k_msleep(20);
			continue;
		}

		ret = 0;
		for (size_t i = 0; i < N_CAPTURE_BUFFERS; i++) {
			ret = video_enqueue(ctx.isp, VIDEO_EP_OUT, ctx.cap[i]);
			if (ret) {
				break;
			}
		}
		if (ret == 0) {
			ret = video_stream_start(ctx.isp);
		}
		if (ret) {
			isp_reclaim();
			k_msleep(100);
			continue;
		}

		printk("Capture session started (%u buffers)\n", N_CAPTURE_BUFFERS);

		while (uvc_committed(&neg)) {
			ret = video_dequeue(ctx.isp, VIDEO_EP_OUT, &vbuf, CAPTURE_TIMEOUT);
			if (ret) {
				break;
			}

			sys_cache_data_invd_range(vbuf->buffer, CAPTURE_FRAME_BYTES);
			ret = jpeg_encode(vbuf, ctx.jpeg_buf, &send_len);
			if (ret) {
				if (rearm_capture(vbuf)) {
					break;
				}
				continue;
			}

			ctx.jpeg_buf->bytesused = send_len;
			ret = video_enqueue(ctx.uvc, VIDEO_EP_IN, ctx.jpeg_buf);
			if (ret) {
				break;
			}

			ret = video_dequeue(ctx.uvc, VIDEO_EP_OUT, &sent, CAPTURE_TIMEOUT);
			if (ret) {
				break;
			}

			if (rearm_capture(vbuf)) {
				break;
			}
		}

		isp_reclaim();
		uvc_reclaim();
		printk("Capture session stopped\n");
	}
}

K_THREAD_STACK_DEFINE(uvc_demo_stack, 8192);
static struct k_thread uvc_demo_thread;

static void start_host_demo(void)
{
	if (ctx.jpeg_buf == NULL || ctx.cap[0] == NULL) {
		return;
	}

	k_thread_create(&uvc_demo_thread, uvc_demo_stack,
			K_THREAD_STACK_SIZEOF(uvc_demo_stack),
			uvc_demo, NULL, NULL, NULL,
			K_PRIO_PREEMPT(7), 0, K_NO_WAIT);
	k_thread_name_set(&uvc_demo_thread, "uvc_demo");
}

/*
 * Geometry and USB identity. These do not start the sensor.
 * One case covers the sensor name, fourccs, Y10P input, NV12 frame, and JPEG cap.
 */
ZTEST(uvc_basic, test_pipeline_geometry)
{
	zassert_true(CAPTURE_WIDTH > 0 && ISP_INPUT_WIDTH > 0, "geometry");
	zassert_equal(VIDEO_PIX_FMT_NV12, VIDEO_FOURCC('N', 'V', '1', '2'), "NV12");
	zassert_equal(VIDEO_PIX_FMT_Y10P, VIDEO_FOURCC('Y', '1', '0', 'P'), "Y10P");
	zassert_equal(VIDEO_PIX_FMT_JPEG, VIDEO_FOURCC('J', 'P', 'E', 'G'), "JPEG");

	zassert_equal(ISP_INPUT_FORMAT, VIDEO_PIX_FMT_Y10P, "Y10P");
	zassert_equal(ISP_INPUT_PITCH, UVC_EXPECT_ISP_PITCH, "Y10P pitch");
	zassert_equal(ISP_INPUT_PITCH, ISP_INPUT_WIDTH * 2U, "width*2");

	zassert_equal(CAPTURE_FORMAT, VIDEO_PIX_FMT_NV12, "NV12 fmt");
	zassert_equal(CAPTURE_PITCH, (CAPTURE_WIDTH * 3U) / 2U, "NV12 pitch");
	zassert_equal(CAPTURE_FRAME_BYTES, UVC_EXPECT_NV12_BYTES, "NV12 bytes");
	zassert_equal(CAPTURE_FRAME_BYTES, CAPTURE_PITCH * CAPTURE_HEIGHT, "pitch*height");

	zassert_equal(JPEG_OUTPUT_MAX_BYTES, CAPTURE_WIDTH * CAPTURE_HEIGHT, "width*height");
	zassert_equal(JPEG_OUTPUT_MAX_BYTES, UVC_EXPECT_JPEG_BYTES, "JPEG cap");
	zassert_equal(JPEG_INPUT_PITCH, ROUND_UP(CAPTURE_WIDTH, 16), "encoder pitch");
	zassert_true(JPEG_QUALITY > 0 && JPEG_QUALITY <= 100, "quality");

	TC_PRINT("%s Y10P %ux%u -> NV12 %ux%u (%u bytes), JPEG cap %u quality %u\n",
		 UVC_SENSOR_NAME, ISP_INPUT_WIDTH, ISP_INPUT_HEIGHT,
		 CAPTURE_WIDTH, CAPTURE_HEIGHT, CAPTURE_FRAME_BYTES,
		 JPEG_OUTPUT_MAX_BYTES, JPEG_QUALITY);
}

ZTEST(uvc_basic, test_usb_identity)
{
	zassert_equal(CONFIG_SAMPLE_USBD_PID, 0x0009, "PID");
	zassert_true(strcmp(CONFIG_SAMPLE_USBD_PRODUCT, "Alif UVC Video Capture") == 0,
		     "product string");
	zassert_true(strcmp(CONFIG_SAMPLE_USBD_MANUFACTURER, "Alif Semiconductor") == 0,
		     "manufacturer");
	TC_PRINT("USB 2fe3:%04x %s\n", CONFIG_SAMPLE_USBD_PID, CONFIG_SAMPLE_USBD_PRODUCT);
}

ZTEST_SUITE(uvc_basic, NULL, NULL, NULL, NULL, NULL);

static void *uvc_qa_setup(void)
{
	struct video_format in_fmt = {
		.pixelformat = ISP_INPUT_FORMAT,
		.width = ISP_INPUT_WIDTH,
		.height = ISP_INPUT_HEIGHT,
		.pitch = ISP_INPUT_PITCH,
	};
	struct video_format fmt = {
		.pixelformat = CAPTURE_FORMAT,
		.width = CAPTURE_WIDTH,
		.height = CAPTURE_HEIGHT,
		.pitch = CAPTURE_PITCH,
	};
	struct video_format enc_in = {
		.pixelformat = CAPTURE_FORMAT,
		.width = CAPTURE_WIDTH,
		.height = CAPTURE_HEIGHT,
		.pitch = JPEG_INPUT_PITCH,
	};
	int ret;

	ctx.isp = DEVICE_DT_GET_ONE(vsi_isp_pico);
	ctx.jpeg = DEVICE_DT_GET(DT_NODELABEL(jpeg0));
	ctx.uvc = DEVICE_DT_GET(DT_NODELABEL(uvc));

	zassert_true(device_is_ready(ctx.isp), "ISP not ready");
	zassert_true(device_is_ready(ctx.jpeg), "JPEG not ready");
	zassert_true(device_is_ready(ctx.uvc), "UVC not ready");
	TC_PRINT("Device name: %s\n", ctx.isp->name);

	ret = video_set_format(ctx.isp, VIDEO_EP_IN, &in_fmt);
	zassert_equal(ret, 0, "ISP input format %d", ret);
	TC_PRINT("ISP input format: Y10P %ux%u\n", ISP_INPUT_WIDTH, ISP_INPUT_HEIGHT);

	ret = video_set_format(ctx.isp, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "capture format %d", ret);
	TC_PRINT("capture format: NV12 %ux%u\n", CAPTURE_WIDTH, CAPTURE_HEIGHT);

	ret = video_set_format(ctx.jpeg, VIDEO_EP_IN, &enc_in);
	zassert_equal(ret, 0, "JPEG input format %d", ret);

	for (size_t i = 0; i < N_CAPTURE_BUFFERS; i++) {
		ctx.cap[i] = video_buffer_aligned_alloc(CAPTURE_FRAME_BYTES,
							CAPTURE_BUF_ALIGN, K_NO_WAIT);
		zassert_not_null(ctx.cap[i], "capture buffer %zu", i);
		TC_PRINT("capture buffer %zu: %u bytes at %p\n",
			 i, CAPTURE_FRAME_BYTES, ctx.cap[i]->buffer);
	}

	ctx.jpeg_buf = video_buffer_aligned_alloc(JPEG_OUTPUT_MAX_BYTES,
						  CAPTURE_BUF_ALIGN, K_NO_WAIT);
	zassert_not_null(ctx.jpeg_buf, "JPEG buffer");
	TC_PRINT("JPEG output buffer: %u bytes at %p\n",
		 JPEG_OUTPUT_MAX_BYTES, ctx.jpeg_buf->buffer);

	uvc_set_video_dev(ctx.uvc, ctx.jpeg);

	ctx.usbd = sample_usbd_init_device(usbd_msg_cb);
	zassert_not_null(ctx.usbd, "USB init");
	ret = usbd_enable(ctx.usbd);
	zassert_equal(ret, 0, "usbd_enable %d", ret);
	TC_PRINT("UVC webcam enabled - waiting for host to open the stream.\n");

	qa_fixture.ctx = &ctx;
	return &qa_fixture;
}

static void uvc_qa_before(void *fixture)
{
	ARG_UNUSED(fixture);
	isp_reclaim();
	jpeg_reclaim();
	uvc_reclaim();
}

static void uvc_qa_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	isp_reclaim();
	jpeg_reclaim();
	uvc_reclaim();
	start_host_demo();
}

ZTEST_F(uvc_qa, test_devices_ready)
{
	zassert_true(device_is_ready(fixture->ctx->isp), "ISP");
	zassert_true(device_is_ready(fixture->ctx->jpeg), "JPEG");
	zassert_true(device_is_ready(fixture->ctx->uvc), "UVC");
	zassert_not_null(fixture->ctx->isp->name, "ISP name");
	TC_PRINT("ISP %s, JPEG %s, UVC %s\n",
		 fixture->ctx->isp->name, fixture->ctx->jpeg->name, fixture->ctx->uvc->name);
}

ZTEST_F(uvc_qa, test_pipeline_formats)
{
	struct video_format isp_in = { 0 };
	struct video_format isp_out = { 0 };
	struct video_format jpeg_in = { 0 };
	struct video_format jpeg_out = { 0 };

	zassert_equal(video_get_format(fixture->ctx->isp, VIDEO_EP_IN, &isp_in), 0, "ISP in");
	zassert_equal(isp_in.pixelformat, VIDEO_PIX_FMT_Y10P, "Y10P");
	zassert_equal(isp_in.width, ISP_INPUT_WIDTH, "ISP in width");
	zassert_equal(isp_in.height, ISP_INPUT_HEIGHT, "ISP in height");

	zassert_equal(video_get_format(fixture->ctx->isp, VIDEO_EP_OUT, &isp_out), 0, "ISP out");
	zassert_equal(isp_out.pixelformat, VIDEO_PIX_FMT_NV12, "NV12");
	zassert_equal(isp_out.width, CAPTURE_WIDTH, "capture width");
	zassert_equal(isp_out.height, CAPTURE_HEIGHT, "capture height");

	zassert_equal(video_get_format(fixture->ctx->jpeg, VIDEO_EP_IN, &jpeg_in), 0, "JPEG in");
	zassert_equal(jpeg_in.pixelformat, VIDEO_PIX_FMT_NV12, "JPEG in NV12");
	zassert_equal(jpeg_in.width, CAPTURE_WIDTH, "JPEG in width");
	zassert_equal(jpeg_in.height, CAPTURE_HEIGHT, "JPEG in height");

	zassert_equal(video_get_format(fixture->ctx->jpeg, VIDEO_EP_OUT, &jpeg_out), 0, "JPEG out");
	zassert_equal(jpeg_out.pixelformat, VIDEO_PIX_FMT_JPEG, "JPEG");
	zassert_equal(jpeg_out.width, CAPTURE_WIDTH, "JPEG out width");
	zassert_equal(jpeg_out.height, CAPTURE_HEIGHT, "JPEG out height");

	TC_PRINT("Y10P %ux%u -> NV12 %ux%u -> JPEG %ux%u\n",
		 isp_in.width, isp_in.height, isp_out.width, isp_out.height,
		 jpeg_out.width, jpeg_out.height);
}

ZTEST_F(uvc_qa, test_caps_and_buffers)
{
	struct video_caps jpeg_caps = { 0 };
	struct video_caps uvc_caps = { 0 };
	struct video_buffer *jbuf = fixture->ctx->jpeg_buf;

	zassert_equal(video_get_caps(fixture->ctx->jpeg, VIDEO_EP_OUT, &jpeg_caps), 0, "JPEG caps");
	zassert_true(caps_have(&jpeg_caps, VIDEO_PIX_FMT_JPEG, CAPTURE_WIDTH, CAPTURE_HEIGHT),
		     "JPEG caps missing the capture frame");
	zassert_equal(video_get_caps(fixture->ctx->uvc, VIDEO_EP_OUT, &uvc_caps), 0, "UVC caps");
	zassert_true(caps_have(&uvc_caps, VIDEO_PIX_FMT_JPEG, CAPTURE_WIDTH, CAPTURE_HEIGHT),
		     "UVC did not mirror the JPEG frame");

	for (size_t i = 0; i < N_CAPTURE_BUFFERS; i++) {
		struct video_buffer *buf = fixture->ctx->cap[i];

		zassert_not_null(buf, "buf %zu", i);
		zassert_not_null(buf->buffer, "data %zu", i);
		zassert_true(buf->size >= CAPTURE_FRAME_BYTES, "size %zu", i);
		zassert_equal((uintptr_t)buf->buffer % CAPTURE_BUF_ALIGN, 0, "align %zu", i);
	}

	zassert_not_null(jbuf, "JPEG buf");
	zassert_true(jbuf->size >= JPEG_OUTPUT_MAX_BYTES, "JPEG size");
	zassert_equal((uintptr_t)jbuf->buffer % CAPTURE_BUF_ALIGN, 0, "JPEG align");
	TC_PRINT("JPEG %ux%u, %u NV12 buffers + JPEG %u bytes, align %u\n",
		 CAPTURE_WIDTH, CAPTURE_HEIGHT, N_CAPTURE_BUFFERS,
		 jbuf->size, CAPTURE_BUF_ALIGN);
}

/*
 * USB speed, no committed stream yet, and a buffer round-trip on the UVC
 * queue. If the host already opened the camera, the committed size is checked
 * and the queue flush is skipped so the live stream is left alone.
 */
ZTEST_F(uvc_qa, test_usb_class)
{
	struct video_format fmt = { 0 };
	struct video_frmival frm = { 0 };
	struct video_buffer *back = NULL;
	enum usbd_speed speed = usbd_caps_speed(fixture->ctx->usbd);
	int ret;

	TC_PRINT("USB controller speed capability: %s\n",
		 speed == USBD_SPEED_HS ? "High-Speed" :
		 speed == USBD_SPEED_FS ? "Full-Speed" : "Unknown");
	zassert_true(speed == USBD_SPEED_FS || speed == USBD_SPEED_HS, "speed %d", speed);

	ret = video_get_format(fixture->ctx->uvc, VIDEO_EP_OUT, &fmt);
	if (ret == -EAGAIN) {
		zassert_equal(video_get_frmival(fixture->ctx->uvc, VIDEO_EP_OUT, &frm),
			      -EAGAIN, "frmival");
		ret = video_enqueue(fixture->ctx->uvc, VIDEO_EP_IN, fixture->ctx->jpeg_buf);
		zassert_equal(ret, 0, "enqueue %d", ret);
		ret = video_flush(fixture->ctx->uvc, VIDEO_EP_IN, true);
		zassert_equal(ret, 0, "flush %d", ret);
		ret = video_dequeue(fixture->ctx->uvc, VIDEO_EP_OUT, &back, K_MSEC(100));
		zassert_equal(ret, 0, "dequeue %d", ret);
		zassert_equal_ptr(back, fixture->ctx->jpeg_buf, "buffer returned");
		TC_PRINT("No host COMMIT; UVC queue flush returned the buffer\n");
		return;
	}

	zassert_equal(ret, 0, "get format %d", ret);
	zassert_equal(fmt.pixelformat, VIDEO_PIX_FMT_JPEG, "committed fourcc");
	zassert_equal(fmt.width, CAPTURE_WIDTH, "committed width");
	zassert_equal(fmt.height, CAPTURE_HEIGHT, "committed height");
	TC_PRINT("Host already committed JPEG %ux%u; queue flush skipped\n",
		 fmt.width, fmt.height);
}

ZTEST_F(uvc_qa, test_capture_jpeg)
{
	struct video_buffer *frame = NULL;
	size_t len = 0;
	int ret = capture_one(&frame);

	zassert_equal(ret, 0, "capture %d", ret);
	zassert_not_null(frame, "frame");
	zassert_not_null(frame->buffer, "data");
	zassert_true(frame->size >= CAPTURE_FRAME_BYTES, "size %u", frame->size);
	TC_PRINT("Captured NV12 frame, bytesused %u size %u\n",
		 frame->bytesused, frame->size);

	ret = jpeg_encode(frame, fixture->ctx->jpeg_buf, &len);
	zassert_equal(ret, 0, "encode %d", ret);
	zassert_true(len > 2 && len <= JPEG_OUTPUT_MAX_BYTES, "len %zu", len);
	zassert_equal(fixture->ctx->jpeg_buf->buffer[0], 0xff, "SOI0");
	zassert_equal(fixture->ctx->jpeg_buf->buffer[1], 0xd8, "SOI1");
	TC_PRINT("JPEG SOI ok, %zu bytes (cap %u)\n", len, JPEG_OUTPUT_MAX_BYTES);
	isp_reclaim();
}

ZTEST_F(uvc_qa, test_stress)
{
	struct video_buffer *frame = NULL;
	int ret;

	for (int i = 0; i < STRESS_POLLS; i++) {
		struct video_format in = { 0 };
		struct video_format out = { 0 };

		zassert_equal(video_get_format(fixture->ctx->isp, VIDEO_EP_IN, &in), 0, "in %d", i);
		zassert_equal(video_get_format(fixture->ctx->isp, VIDEO_EP_OUT, &out), 0, "out %d", i);
		zassert_equal(in.pixelformat, VIDEO_PIX_FMT_Y10P, "Y10P");
		zassert_equal(out.pixelformat, VIDEO_PIX_FMT_NV12, "NV12");
		zassert_equal(out.width, CAPTURE_WIDTH, "width");
		zassert_equal(out.height, CAPTURE_HEIGHT, "height");
	}

	for (int i = 0; i < STRESS_RESTARTS; i++) {
		ret = capture_one(&frame);
		zassert_equal(ret, 0, "restart %d capture %d", i, ret);
		zassert_not_null(frame, "frame %d", i);
		isp_reclaim();
		k_msleep(20);
	}

	ret = capture_one(&frame);
	zassert_equal(ret, 0, "stress capture %d", ret);
	for (int i = 0; i < STRESS_JPEGS; i++) {
		size_t len = 0;

		ret = jpeg_encode(frame, fixture->ctx->jpeg_buf, &len);
		zassert_equal(ret, 0, "encode %d -> %d", i, ret);
		zassert_true(len > 2 && len <= JPEG_OUTPUT_MAX_BYTES, "len %zu", len);
		zassert_equal(fixture->ctx->jpeg_buf->buffer[0], 0xff, "SOI");
		zassert_equal(fixture->ctx->jpeg_buf->buffer[1], 0xd8, "SOI");
	}

	TC_PRINT("Stress: format x%d, capture restart x%d, JPEG x%d\n",
		 STRESS_POLLS, STRESS_RESTARTS, STRESS_JPEGS);
	isp_reclaim();
}

ZTEST_F(uvc_qa, test_perf)
{
	struct video_buffer *frame = NULL;
	int64_t cap_total = 0;
	int64_t jpeg_total = 0;
	size_t last = 0;
	int ret;

	for (size_t i = 0; i < N_CAPTURE_BUFFERS; i++) {
		ret = video_enqueue(fixture->ctx->isp, VIDEO_EP_OUT, fixture->ctx->cap[i]);
		zassert_equal(ret, 0, "enqueue %d", ret);
	}
	ret = video_stream_start(fixture->ctx->isp);
	zassert_equal(ret, 0, "start %d", ret);

	for (int i = 0; i < PERF_FRAMES; i++) {
		int64_t t0 = k_uptime_get();
		int64_t ms;

		ret = video_dequeue(fixture->ctx->isp, VIDEO_EP_OUT, &frame, CAPTURE_TIMEOUT);
		ms = k_uptime_delta(&t0);
		cap_total += ms;
		zassert_equal(ret, 0, "frame %d dequeue %d", i, ret);
		TC_PRINT("capture frame %d: %lld ms\n", i, ms);
		sys_cache_data_invd_range(frame->buffer, CAPTURE_FRAME_BYTES);
		ret = rearm_capture(frame);
		zassert_equal(ret, 0, "rearm %d", ret);
	}
	isp_reclaim();

	ret = capture_one(&frame);
	zassert_equal(ret, 0, "perf capture %d", ret);
	for (int i = 0; i < PERF_FRAMES; i++) {
		int64_t t0 = k_uptime_get();
		int64_t ms;
		size_t len = 0;

		ret = jpeg_encode(frame, fixture->ctx->jpeg_buf, &len);
		ms = k_uptime_delta(&t0);
		jpeg_total += ms;
		zassert_equal(ret, 0, "encode %d -> %d", i, ret);
		zassert_true(len > 2, "empty jpeg");
		last = len;
		TC_PRINT("jpeg frame %d: %lld ms, %zu bytes\n", i, ms, len);
	}

	TC_PRINT("capture avg %lld ms, jpeg avg %lld ms, last %zu bytes (not USB wire)\n",
		 cap_total / PERF_FRAMES, jpeg_total / PERF_FRAMES, last);
	isp_reclaim();
}

ZTEST_F(uvc_qa, test_host_commit_mjpeg)
{
	struct video_format fmt = { 0 };
	int ret = video_get_format(fixture->ctx->uvc, VIDEO_EP_OUT, &fmt);

	if (ret == -EAGAIN) {
		TC_PRINT("Skip: host has not COMMIT. After SUCCESS run "
			 "ffplay -f v4l2 -input_format mjpeg -video_size %ux%u /dev/videoN\n",
			 CAPTURE_WIDTH, CAPTURE_HEIGHT);
		ztest_test_skip();
	}

	zassert_equal(ret, 0, "commit %d", ret);
	zassert_equal(fmt.pixelformat, VIDEO_PIX_FMT_JPEG, "JPEG");
	zassert_equal(fmt.width, CAPTURE_WIDTH, "width");
	zassert_equal(fmt.height, CAPTURE_HEIGHT, "height");
	TC_PRINT("Host COMMIT JPEG %ux%u\n", fmt.width, fmt.height);
}

ZTEST_F(uvc_qa, test_usb_disable_reenable)
{
	TC_PRINT("Skip usbd_disable: UVC+DWC3 halt drops the host webcam\n");
	ztest_test_skip();
}

ZTEST_SUITE(uvc_qa, NULL, uvc_qa_setup, uvc_qa_before, NULL, uvc_qa_teardown);
