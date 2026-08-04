/*
 * Copyright (C) 2026 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Camera capture exposed as a USB Video Class (UVC) webcam. The capture
 * pipeline is chosen at build time from the devicetree (see pipeline.h):
 *
 *   - MJPEG (E8 DK):  <sensor> -> CSI -> ISP (NV12) -> Hantro JPEG encoder,
 *     streamed as MJPEG over the UVC bulk endpoint.
 *   - RGB565 (E1C / B1 SK): OV5640 parallel RGB565 480x272 off the LP-CAM,
 *     streamed uncompressed.
 *
 * The in-tree UVC class (CONFIG_USBD_VIDEO_CLASS, devicetree node "uvc")
 * generates its USB descriptors from the video device it is bound to: the JPEG
 * encoder (advertising MJPEG) on the E8, or the LP-CAM (advertising RGB565) on
 * the E1C / B1. The application configures the capture pipeline, then shuttles
 * finished frames into the UVC device with video_enqueue()/video_dequeue();
 * the host pulls frames at its own pace, which throttles the capture loop.
 */

#include "pipeline.h"
#if UVC_OUTPUT_MJPEG
#include "jpeg_encode.h"
#endif

#include <sample_usbd.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/cache.h>
#include <zephyr/drivers/video.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usbd_uvc.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(video_usbout_uvc, LOG_LEVEL_INF);

/* The in-tree UVC class instance (devicetree node "uvc"). */
static const struct device *const uvc_dev = DEVICE_DT_GET(DT_NODELABEL(uvc));

/*
 * Number of capture buffers kept in flight. Both the ISP and the LP-CAM stop
 * the sensor as soon as their input FIFO drains, so with a single buffer the
 * sensor is restarted every frame and the rate collapses to ~1 fps. Two buffers
 * keep one armed in the driver while the other is encoded and streamed over USB,
 * so capture overlaps with encode+send and runs continuously.
 *
 * On the MJPEG targets these two NV12 buffers plus the separate JPEG output
 * buffer all come from the video buffer pool, so CONFIG_VIDEO_BUFFER_POOL_NUM_MAX
 * must be >= 3.
 */
#define N_CAPTURE_BUFFERS 2

/*
 * Upper bound on how long a dequeue may block. In continuous streaming a frame
 * arrives every sensor period; the generous 2 s ceiling only trips if capture
 * stalled or the host closed the stream, in which case the session is torn down
 * and restarted.
 */
#define CAPTURE_TIMEOUT K_MSEC(2000)

#if UVC_OUTPUT_MJPEG
static const struct device *jpeg_dev;
static struct video_buffer *jpeg_op_buf;
#endif

/* True while the host has an open (committed) UVC stream. */
static inline bool uvc_stream_open(void)
{
	struct video_format neg;

	return video_get_format(uvc_dev, VIDEO_EP_OUT, &neg) == 0;
}

/*
 * Configure the capture format. On MJPEG targets the ISP input (sensor) format
 * must be set before the output format, or stream_start fails with -EINVAL.
 */
static int configure_capture_pipeline(const struct device *video)
{
	struct video_format fmt = {
		.pixelformat = CAPTURE_FORMAT,
		.width = CAPTURE_WIDTH,
		.height = CAPTURE_HEIGHT,
		.pitch = CAPTURE_PITCH,
	};
	int ret;

#if UVC_OUTPUT_MJPEG
	struct video_format in_fmt = {
		.pixelformat = ISP_INPUT_FORMAT,
		.width = ISP_INPUT_WIDTH,
		.height = ISP_INPUT_HEIGHT,
		.pitch = ISP_INPUT_PITCH,
	};

	ret = video_set_format(video, VIDEO_EP_IN, &in_fmt);
	if (ret) {
		LOG_ERR("Failed to set ISP input format: %d", ret);
		return ret;
	}
	LOG_INF("- ISP input format: Y10P %ux%u", ISP_INPUT_WIDTH, ISP_INPUT_HEIGHT);
#endif

	ret = video_set_format(video, VIDEO_EP_OUT, &fmt);
	if (ret) {
		LOG_ERR("Failed to set capture format: %d", ret);
		return ret;
	}
	LOG_INF("- capture format: %s %ux%u",
		VIDEO_FOURCC_TO_STR(CAPTURE_FORMAT), fmt.width, fmt.height);

	return 0;
}

#if UVC_OUTPUT_MJPEG
/*
 * Bring up the JPEG encoder and allocate its output buffer. The encoder's input
 * format is published before USB is enabled: the UVC class reads the encoder's
 * MJPEG output capability during usbd_enable() to build its descriptors, so this
 * makes the class advertise MJPEG at exactly CAPTURE_WIDTH x CAPTURE_HEIGHT.
 */
static int configure_jpeg_encoder(void)
{
	struct video_format enc_in = {
		.pixelformat = CAPTURE_FORMAT,
		.width = CAPTURE_WIDTH,
		.height = CAPTURE_HEIGHT,
		.pitch = JPEG_INPUT_PITCH,
	};
	int ret;

	jpeg_dev = DEVICE_DT_GET(DT_NODELABEL(jpeg0));
	if (!device_is_ready(jpeg_dev)) {
		LOG_ERR("JPEG encoder not ready");
		return -ENODEV;
	}

	ret = video_set_format(jpeg_dev, VIDEO_EP_IN, &enc_in);
	if (ret) {
		LOG_ERR("Failed to set JPEG encoder input format: %d", ret);
		return ret;
	}

	jpeg_op_buf = video_buffer_aligned_alloc(JPEG_OUTPUT_MAX_BYTES,
						 CAPTURE_BUF_ALIGN, K_NO_WAIT);
	if (jpeg_op_buf == NULL) {
		LOG_ERR("Unable to alloc JPEG output buffer");
		return -ENOMEM;
	}
	LOG_INF("- JPEG output buffer: %u bytes at 0x%08x",
		(unsigned int)JPEG_OUTPUT_MAX_BYTES, (uint32_t)jpeg_op_buf->buffer);

	return 0;
}
#endif /* UVC_OUTPUT_MJPEG */

/* Allocate the capture buffers into the caller's array. */
static int alloc_capture_buffers(struct video_buffer *bufs[], size_t n)
{
	for (size_t i = 0; i < n; i++) {
		bufs[i] = video_buffer_aligned_alloc(CAPTURE_FRAME_BYTES,
						     CAPTURE_BUF_ALIGN, K_NO_WAIT);
		if (bufs[i] == NULL) {
			LOG_ERR("Unable to alloc capture buffer %zu", i);
			return -ENOMEM;
		}
		LOG_INF("- capture buffer %zu: %u bytes at 0x%08x",
			i, (unsigned int)CAPTURE_FRAME_BYTES, (uint32_t)bufs[i]->buffer);
	}

	return 0;
}

/*
 * Start a capture session: enqueue every buffer up front and start the stream
 * once. With more than one buffer armed the driver re-arms the next buffer at
 * end-of-frame instead of stopping the sensor, so capture runs continuously.
 */
static int capture_session_start(const struct device *video,
				 struct video_buffer *bufs[], size_t n)
{
	int ret;

	for (size_t i = 0; i < n; i++) {
		ret = video_enqueue(video, VIDEO_EP_OUT, bufs[i]);
		if (ret) {
			LOG_ERR("Unable to enqueue buffer %zu: %d", i, ret);
			return ret;
		}
	}

	ret = video_stream_start(video);
	if (ret) {
		LOG_ERR("Unable to start capture: %d", ret);
		return ret;
	}

	LOG_INF("Capture session started (%zu buffers)", n);
	return 0;
}

/*
 * Stop the current capture session and reclaim every buffer, both from the
 * capture driver and from the UVC class (a frame may be left in flight when the
 * host stops pulling). After this the buffers are owned by the application again
 * and can be re-enqueued next session. Safe whether or not either side is still
 * streaming.
 */
static void capture_session_teardown(const struct device *video)
{
	struct video_buffer *vbuf;

	video_stream_stop(video);
	video_flush(video, VIDEO_EP_OUT, false);
	while (video_dequeue(video, VIDEO_EP_OUT, &vbuf, K_NO_WAIT) == 0) {
		/* Drain the driver FIFOs; buffers live in the caller's array. */
	}

	/*
	 * video_flush() aborts the UVC transfer and moves every queued buffer to
	 * the class output queue; the short wait covers the async abort. Draining
	 * it relinquishes ownership so the (reused) buffers are safe next session.
	 */
	video_flush(uvc_dev, VIDEO_EP_IN, true);
	while (video_dequeue(uvc_dev, VIDEO_EP_OUT, &vbuf, K_MSEC(50)) == 0) {
		/* Discard; ownership returns to the application. */
	}
}

/*
 * Capture one frame, (encode it on MJPEG targets,) push it to the host, and
 * re-arm the buffer. Returns 0 to keep streaming, or a negative errno when the
 * session should be torn down (capture stalled or the host closed the stream).
 */
static int capture_and_send(const struct device *video)
{
	struct video_buffer *vbuf;
	struct video_buffer *send_buf;
	struct video_buffer *sent;
	size_t send_len;
	int ret;

	ret = video_dequeue(video, VIDEO_EP_OUT, &vbuf, CAPTURE_TIMEOUT);
	if (ret) {
		LOG_WRN("capture dequeue failed (%d), restarting session", ret);
		return ret;
	}

	/*
	 * The capture drivers do cache maintenance on enqueue only (before DMA),
	 * so invalidate to read the freshly DMA'd frame, not stale cache lines.
	 */
	sys_cache_data_invd_range(vbuf->buffer, CAPTURE_FRAME_BYTES);

#if UVC_OUTPUT_MJPEG
	ret = jpeg_encode(jpeg_dev, vbuf, jpeg_op_buf, &send_len);
	if (ret) {
		LOG_ERR("JPEG encode failed (%d), skipping frame", ret);
		/* Re-arm the buffer and keep the session running. */
		video_enqueue(video, VIDEO_EP_OUT, vbuf);
		video_stream_start(video);
		return 0;
	}
	send_buf = jpeg_op_buf;
#else
	send_buf = vbuf;
	send_len = CAPTURE_FRAME_BYTES;
#endif
	send_buf->bytesused = send_len;

	/*
	 * Hand the frame to the UVC class and wait for it to be streamed. The
	 * host pulls at its own pace, so this dequeue throttles the loop; a
	 * timeout means the host stalled or closed the stream.
	 */
	ret = video_enqueue(uvc_dev, VIDEO_EP_IN, send_buf);
	if (ret) {
		LOG_ERR("UVC enqueue failed (%d)", ret);
		return ret;
	}

	ret = video_dequeue(uvc_dev, VIDEO_EP_OUT, &sent, CAPTURE_TIMEOUT);
	if (ret) {
		LOG_INF("UVC send timed out (%d), stream likely closed", ret);
		return ret;
	}

	/*
	 * Re-arm this buffer for another capture. Re-issue stream_start too: it
	 * is a no-op (-EBUSY) while the pipeline is still running, and restarts
	 * it if it stopped because the FIFO briefly drained.
	 */
	ret = video_enqueue(video, VIDEO_EP_OUT, vbuf);
	if (ret) {
		LOG_ERR("capture re-enqueue failed (%d)", ret);
		return ret;
	}

	ret = video_stream_start(video);
	if (ret && ret != -EBUSY) {
		LOG_ERR("capture restart failed (%d)", ret);
		return ret;
	}

	return 0;
}

int main(void)
{
	struct video_buffer *buffers[N_CAPTURE_BUFFERS];
	struct usbd_context *sample_usbd;
	const struct device *video;
	const struct device *uvc_src;
	int ret;

#if UVC_OUTPUT_MJPEG
	video = DEVICE_DT_GET_ONE(vsi_isp_pico);
#else
	video = DEVICE_DT_GET_ONE(alif_cam);
#endif

	if (!device_is_ready(video) || !device_is_ready(uvc_dev)) {
		LOG_ERR("video or UVC device not ready");
		return -ENODEV;
	}
	LOG_INF("- Device name: %s", video->name);

	if (configure_capture_pipeline(video)) {
		return -EIO;
	}

#if UVC_OUTPUT_MJPEG
	if (configure_jpeg_encoder()) {
		return -EIO;
	}
	uvc_src = jpeg_dev;	/* The host sees the encoder's MJPEG output. */
#else
	uvc_src = video;	/* The host sees RGB565 straight from the LP-CAM. */
#endif

	if (alloc_capture_buffers(buffers, N_CAPTURE_BUFFERS)) {
		return -ENOMEM;
	}

	/* Bind the UVC class to the device whose format it advertises, then
	 * bring up USB. The board now enumerates as a webcam.
	 */
	uvc_set_video_dev(uvc_dev, uvc_src);

	sample_usbd = sample_usbd_init_device(NULL);
	if (sample_usbd == NULL) {
		LOG_ERR("Failed to initialize USB device");
		return -EIO;
	}

	ret = usbd_enable(sample_usbd);
	if (ret) {
		LOG_ERR("Failed to enable USB: %d", ret);
		return ret;
	}
	LOG_INF("UVC webcam enabled - waiting for host to open the stream.");

	while (1) {
		/*
		 * Wait for the host to commit/open the stream. The UVC class
		 * only reports a format once the host has issued COMMIT.
		 */
		if (!uvc_stream_open()) {
			k_msleep(10);
			continue;
		}

		if (capture_session_start(video, buffers, N_CAPTURE_BUFFERS)) {
			capture_session_teardown(video);
			k_msleep(100);	/* Back off; don't busy-loop on failure. */
			continue;
		}

		/* Stream frames until the host closes the stream or capture stalls. */
		while (uvc_stream_open() && capture_and_send(video) == 0) {
		}

		capture_session_teardown(video);
		LOG_INF("Capture session stopped");
	}

	return 0;
}
