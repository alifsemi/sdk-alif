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
 *
 * The dual camera build captures the two sensors in turn and has to stop the
 * pipeline between frames to switch sensor, so a single NV12 buffer is enough
 * there; the pool instead holds one JPEG output buffer per camera.
 */
#if UVC_DUAL_CAMERA
#define N_CAPTURE_BUFFERS 1
#else
#define N_CAPTURE_BUFFERS 2
#endif

/*
 * Upper bound on how long a dequeue may block. In continuous streaming a frame
 * arrives every sensor period; the generous 2 s ceiling only trips if capture
 * stalled or the host closed the stream, in which case the session is torn down
 * and restarted.
 */
#define CAPTURE_TIMEOUT_MS 2000
#define CAPTURE_TIMEOUT K_MSEC(CAPTURE_TIMEOUT_MS)

#if UVC_OUTPUT_MJPEG
static const struct device *jpeg_dev;
/* One JPEG output buffer per camera, so a camera can capture while the other's
 * frame is still being streamed.
 */
static struct video_buffer *jpeg_op_bufs[UVC_NUM_CAMERAS];
#endif

#if UVC_DUAL_CAMERA
/* UVC device per camera, indexed by the CSI sensor id. */
static const struct device *const uvc_cam_devs[UVC_NUM_CAMERAS] = {
	DEVICE_DT_GET(DT_NODELABEL(uvc)),
	DEVICE_DT_GET(DT_NODELABEL(uvc_standard)),
};
static const char *const camera_names[UVC_NUM_CAMERAS] = { "selfie", "standard" };
/* Camera controller (CPI) between the CSI receiver and the ISP. */
static const struct device *const cam_ctrl = DEVICE_DT_GET(DT_NODELABEL(cam));
#endif

/*
 * Cleared while the cable is unplugged or the bus is suspended. The host cannot
 * read frames then, but the UVC class only learns that the stream ended at the
 * next bus reset, so the capture loop checks this as well.
 */
static atomic_t usb_bus_active = ATOMIC_INIT(1);

static void usbd_msg_cb(struct usbd_context *const ctx, const struct usbd_msg *const msg)
{
	ARG_UNUSED(ctx);

	switch (msg->type) {
	case USBD_MSG_VBUS_REMOVED:
	case USBD_MSG_SUSPEND:
		if (atomic_set(&usb_bus_active, 0) != 0) {
			LOG_INF("USB %s",
				msg->type == USBD_MSG_SUSPEND ? "suspended" : "unplugged");
		}
		break;
	case USBD_MSG_VBUS_READY:
	case USBD_MSG_RESUME:
		atomic_set(&usb_bus_active, 1);
		break;
	default:
		break;
	}
}

/* True while the host has an open (committed) stream it can read on @p uvc. */
static inline bool uvc_stream_open(const struct device *uvc)
{
	struct video_format neg;

	return atomic_get(&usb_bus_active) != 0 &&
	       video_get_format(uvc, VIDEO_EP_OUT, &neg) == 0;
}

#if UVC_DUAL_CAMERA
static int select_camera(const struct device *video, uint8_t cam)
{
	int ret = video_set_ctrl(video, VIDEO_CID_ALIF_CSI_CURR_CAM, &cam);

	if (ret) {
		LOG_ERR("Failed to select the %s camera: %d", camera_names[cam], ret);
	}
	return ret;
}

/*
 * Program the selfie sensor and bring up its D-PHY once the standard camera has
 * been configured through the ISP. The ISP caches its input format and skips a
 * set_format matching it, so the (identical) sensor format is pushed through
 * the camera controller directly while the selfie camera is selected.
 */
static int configure_selfie_camera(const struct device *video,
				   const struct video_format *in_fmt)
{
	struct video_format fmt = *in_fmt;
	int ret;

	ret = select_camera(video, 0);
	if (ret) {
		return ret;
	}

	ret = video_set_format(cam_ctrl, VIDEO_EP_OUT, &fmt);
	if (ret) {
		LOG_ERR("Failed to set the selfie camera format: %d", ret);
	}
	return ret;
}
#endif /* UVC_DUAL_CAMERA */

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

#if UVC_DUAL_CAMERA
	/*
	 * Bring up the standard camera's D-PHY (the DSI D-PHY in RX mode)
	 * before the selfie camera's CSI D-PHY.
	 */
	ret = select_camera(video, 1);
	if (ret) {
		return ret;
	}
#endif

	ret = video_set_format(video, VIDEO_EP_IN, &in_fmt);
	if (ret) {
		LOG_ERR("Failed to set ISP input format: %d", ret);
		return ret;
	}
	LOG_INF("- ISP input format: Y10P %ux%u", ISP_INPUT_WIDTH, ISP_INPUT_HEIGHT);
#endif

#if UVC_DUAL_CAMERA
	ret = configure_selfie_camera(video, &in_fmt);
	if (ret) {
		return ret;
	}
	LOG_INF("- standard and selfie cameras configured");
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

	for (size_t i = 0; i < UVC_NUM_CAMERAS; i++) {
		jpeg_op_bufs[i] = video_buffer_aligned_alloc(JPEG_OUTPUT_MAX_BYTES,
							     CAPTURE_BUF_ALIGN, K_NO_WAIT);
		if (jpeg_op_bufs[i] == NULL) {
			LOG_ERR("Unable to alloc JPEG output buffer %zu", i);
			return -ENOMEM;
		}
		LOG_INF("- JPEG output buffer %zu: %u bytes at 0x%08x", i,
			(unsigned int)JPEG_OUTPUT_MAX_BYTES, (uint32_t)jpeg_op_bufs[i]->buffer);
	}

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
 * Set when capture could not be stopped or a buffer could not be reclaimed. A
 * driver may then still own a buffer, so capture ends for good: nothing is
 * enqueued again and no other sensor is selected.
 */
static bool capture_halted;

/*
 * video_stream_stop() with the result of its cancelling flush checked:
 * video_stream_stop() discards it, and with a single ISP buffer that flush does
 * the remaining ISP shutdown. The flush cancels because after a failed start the
 * pipeline may be stopped with a buffer still armed, and a draining flush would
 * wait for it forever.
 */
static int video_stop_checked(const struct device *dev)
{
	const struct video_driver_api *api = dev->api;
	int flush_ret;
	int ret;

	if (api->set_stream == NULL) {
		return -ENOSYS;
	}

	ret = api->set_stream(dev, false);
	flush_ret = video_flush(dev, VIDEO_EP_ALL, true);

	return ret ? ret : flush_ret;
}

/*
 * Stop the capture pipeline. On the dual camera build the other sensor may only
 * be selected once the camera controller and sensor have stopped. With a single
 * buffer the ISP's frame-end worker stops them itself without reporting a
 * failure, so the controller stop is repeated: it returns 0 once the controller
 * and sensor are stopped, and retries the stop and returns its error otherwise.
 */
static int capture_stream_stop(const struct device *video)
{
	int ret = video_stop_checked(video);

#if UVC_DUAL_CAMERA
	int cam_ret = video_stop_checked(cam_ctrl);

	if (ret == 0) {
		ret = cam_ret;
	}
#endif
	return ret;
}

/* Stop capture and reclaim every buffer still held by the capture driver. */
static int capture_stop_and_drain(const struct device *video)
{
	struct video_buffer *vbuf;
	int ret;

	ret = capture_stream_stop(video);
	while (video_dequeue(video, VIDEO_EP_OUT, &vbuf, K_NO_WAIT) == 0) {
		/* Drain the driver FIFOs; buffers live in the caller's array. */
	}

	if (ret) {
		LOG_ERR("Capture recovery failed (%d), capture halted", ret);
		capture_halted = true;
	}
	return ret;
}

#if !UVC_DUAL_CAMERA
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

/* Set while the frame handed to the UVC class has not been dequeued back. */
static bool uvc_send_in_flight;

/*
 * Stop the current capture session and reclaim every buffer, both from the
 * capture driver and from the UVC class (a frame may be left in flight when the
 * host stops pulling). After this the buffers are owned by the application again
 * and can be re-enqueued next session. Safe whether or not either side is still
 * streaming. If either side keeps a buffer, capture is halted.
 */
static void capture_session_teardown(const struct device *video)
{
	struct video_buffer *vbuf;
	int ret;

	capture_stop_and_drain(video);

	if (!uvc_send_in_flight) {
		return;
	}

	/*
	 * Abort the UVC transfer. The class returns the frame from the transfer's
	 * completion callback, once the USB controller has stopped accessing it.
	 */
	ret = video_flush(uvc_dev, VIDEO_EP_IN, true);
	if (ret == 0) {
		ret = video_dequeue(uvc_dev, VIDEO_EP_OUT, &vbuf, CAPTURE_TIMEOUT);
	}
	if (ret) {
		LOG_ERR("UVC frame not reclaimed (%d), capture halted", ret);
		capture_halted = true;
		return;
	}
	uvc_send_in_flight = false;
}

/*
 * Re-arm a capture buffer. Re-issue stream_start too: it is a no-op (-EBUSY)
 * while the pipeline is still running, and restarts it if it stopped because
 * the FIFO briefly drained.
 */
static int rearm_capture(const struct device *video, struct video_buffer *vbuf)
{
	int ret;

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
	ret = jpeg_encode(jpeg_dev, vbuf, jpeg_op_bufs[0], &send_len);
	if (ret) {
		LOG_ERR("JPEG encode failed (%d), skipping frame", ret);
		/* Re-arm the buffer and keep the session running. */
		return rearm_capture(video, vbuf);
	}
	send_buf = jpeg_op_bufs[0];
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
	uvc_send_in_flight = true;

	ret = video_dequeue(uvc_dev, VIDEO_EP_OUT, &sent, CAPTURE_TIMEOUT);
	if (ret) {
		LOG_INF("UVC send timed out (%d), stream likely closed", ret);
		return ret;
	}
	uvc_send_in_flight = false;

	/* Re-arm this buffer for another capture. */
	return rearm_capture(video, vbuf);
}
#endif /* !UVC_DUAL_CAMERA */

#if UVC_DUAL_CAMERA
/*
 * Set while a camera's JPEG buffer is owned by its UVC device. It is cleared
 * only once the buffer has been dequeued back, so it is never re-encoded while
 * USB may still be reading it.
 */
static bool jpeg_in_flight[UVC_NUM_CAMERAS];
/* Uptime when the in-flight buffer was queued, or when its reclaim last failed. */
static int64_t jpeg_queued_at[UVC_NUM_CAMERAS];

/* Take back a camera's JPEG buffer if the UVC device has finished with it. */
static bool uvc_camera_buffer_free(uint8_t cam)
{
	struct video_buffer *vbuf;

	if (jpeg_in_flight[cam] &&
	    video_dequeue(uvc_cam_devs[cam], VIDEO_EP_OUT, &vbuf, K_NO_WAIT) == 0) {
		jpeg_in_flight[cam] = false;
	}
	return !jpeg_in_flight[cam];
}

/* True once a camera's JPEG buffer has been held by USB for CAPTURE_TIMEOUT. */
static bool uvc_camera_send_expired(uint8_t cam)
{
	return k_uptime_get() - jpeg_queued_at[cam] >= CAPTURE_TIMEOUT_MS;
}

/*
 * Abort a camera's UVC transfer and take its JPEG buffer back. The UVC class
 * only returns the buffer once the USB controller has stopped accessing it. If
 * the transfer cannot be cancelled or the buffer is not returned, it stays in
 * flight and the reset is retried after another CAPTURE_TIMEOUT.
 */
static void uvc_camera_stream_reset(uint8_t cam)
{
	struct video_buffer *vbuf;
	int ret;

	if (!jpeg_in_flight[cam]) {
		return;
	}

	ret = video_flush(uvc_cam_devs[cam], VIDEO_EP_IN, true);
	if (ret) {
		LOG_ERR("%s camera: UVC flush failed (%d)", camera_names[cam], ret);
		jpeg_queued_at[cam] = k_uptime_get();
		return;
	}

	/* A cancelled transfer returns the buffer from its completion callback. */
	ret = video_dequeue(uvc_cam_devs[cam], VIDEO_EP_OUT, &vbuf, K_MSEC(50));
	if (ret) {
		LOG_ERR("%s camera: JPEG buffer not returned by UVC (%d)",
			camera_names[cam], ret);
		jpeg_queued_at[cam] = k_uptime_get();
		return;
	}
	jpeg_in_flight[cam] = false;
}

/*
 * Capture one frame from @p cam into @p capbuf. The ISP stops the pipeline once
 * its only buffer is filled, and only a stopped pipeline can be switched to the
 * other sensor, so every frame is a short start/capture/stop cycle.
 */
static int capture_camera_frame(const struct device *video, uint8_t cam,
				struct video_buffer *capbuf)
{
	struct video_buffer *vbuf;
	int ret;

	ret = select_camera(video, cam);
	if (ret) {
		return ret;
	}

	ret = video_enqueue(video, VIDEO_EP_OUT, capbuf);
	if (ret) {
		LOG_ERR("%s camera: capture enqueue failed (%d)", camera_names[cam], ret);
		return ret;
	}

	ret = video_stream_start(video);
	if (ret) {
		LOG_ERR("%s camera: capture start failed (%d)", camera_names[cam], ret);
		capture_stop_and_drain(video);
		return ret;
	}

	ret = video_dequeue(video, VIDEO_EP_OUT, &vbuf, CAPTURE_TIMEOUT);
	if (ret) {
		LOG_WRN("%s camera: capture dequeue failed (%d)", camera_names[cam], ret);
		capture_stop_and_drain(video);
		return ret;
	}

	sys_cache_data_invd_range(vbuf->buffer, CAPTURE_FRAME_BYTES);

	/*
	 * capture_stream_stop() stops the pipeline and flushes the driver. A
	 * switch to the other sensor is only valid once that has finished, so a
	 * failed stop drops the frame, and halts capture if the retry fails too.
	 * Drain anything the flush queued; the captured frame was already
	 * dequeued above.
	 */
	ret = capture_stream_stop(video);
	if (ret) {
		LOG_ERR("%s camera: capture stop failed (%d)", camera_names[cam], ret);
		capture_stop_and_drain(video);
		return ret;
	}
	while (video_dequeue(video, VIDEO_EP_OUT, &vbuf, K_NO_WAIT) == 0) {
	}

	return 0;
}

/*
 * Capture, encode and queue one frame of @p cam on its UVC device. The caller
 * makes sure the camera's JPEG buffer is free; the frame is then streamed while
 * the other camera captures.
 */
static int stream_camera_frame(const struct device *video, uint8_t cam,
			       struct video_buffer *capbuf)
{
	struct video_buffer *jpeg_buf = jpeg_op_bufs[cam];
	size_t len;
	int ret;

	ret = capture_camera_frame(video, cam, capbuf);
	if (ret) {
		return ret;
	}

	ret = jpeg_encode(jpeg_dev, capbuf, jpeg_buf, &len);
	if (ret) {
		LOG_ERR("%s camera: JPEG encode failed (%d), skipping frame",
			camera_names[cam], ret);
		return 0;
	}
	jpeg_buf->bytesused = len;

	ret = video_enqueue(uvc_cam_devs[cam], VIDEO_EP_IN, jpeg_buf);
	if (ret) {
		LOG_ERR("%s camera: UVC enqueue failed (%d)", camera_names[cam], ret);
		return ret;
	}
	jpeg_in_flight[cam] = true;
	jpeg_queued_at[cam] = k_uptime_get();

	return 0;
}

/*
 * Serve both webcams, alternating frames between the cameras that are open. A
 * camera whose previous frame is still being read by the host is skipped rather
 * than waited for, so a slow or stalled reader does not hold back the other
 * webcam. Returns only if capture is halted.
 */
static int dual_camera_loop(const struct device *video, struct video_buffer *capbuf)
{
	bool was_open[UVC_NUM_CAMERAS] = { false };

	while (!capture_halted) {
		bool any_open = false;
		bool progressed = false;

		for (uint8_t cam = 0; cam < UVC_NUM_CAMERAS; cam++) {
			bool open = uvc_stream_open(uvc_cam_devs[cam]);
			bool closed_now = was_open[cam] && !open;

			if (open != was_open[cam]) {
				LOG_INF("%s camera stream %s", camera_names[cam],
					open ? "opened" : "closed");
				was_open[cam] = open;
			}

			if (!open) {
				if (jpeg_in_flight[cam] &&
				    (closed_now || uvc_camera_send_expired(cam))) {
					uvc_camera_stream_reset(cam);
				}
				continue;
			}
			any_open = true;

			if (!uvc_camera_buffer_free(cam)) {
				if (uvc_camera_send_expired(cam)) {
					LOG_INF("%s camera: UVC send timed out",
						camera_names[cam]);
					uvc_camera_stream_reset(cam);
				}
				continue;
			}

			if (stream_camera_frame(video, cam, capbuf)) {
				if (capture_halted) {
					break;
				}
				k_msleep(100);	/* Back off; don't busy-loop on failure. */
			} else {
				progressed = true;
			}
		}

		if (!any_open) {
			k_msleep(10);
		} else if (!progressed) {
			k_msleep(1);	/* Both open streams are still sending. */
		}
	}

	return -EIO;
}
#endif /* UVC_DUAL_CAMERA */

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
	 * bring up USB. The board now enumerates as a webcam (two on the dual
	 * camera build, both fed by the JPEG encoder).
	 */
#if UVC_DUAL_CAMERA
	for (size_t i = 0; i < UVC_NUM_CAMERAS; i++) {
		if (!device_is_ready(uvc_cam_devs[i])) {
			LOG_ERR("UVC device %s not ready", uvc_cam_devs[i]->name);
			return -ENODEV;
		}
		uvc_set_video_dev(uvc_cam_devs[i], uvc_src);
	}
#else
	uvc_set_video_dev(uvc_dev, uvc_src);
#endif

	sample_usbd = sample_usbd_init_device(usbd_msg_cb);
	if (sample_usbd == NULL) {
		LOG_ERR("Failed to initialize USB device");
		return -EIO;
	}

	ret = usbd_enable(sample_usbd);
	if (ret) {
		LOG_ERR("Failed to enable USB: %d", ret);
		return ret;
	}
#if UVC_DUAL_CAMERA
	LOG_INF("UVC webcams enabled - waiting for the host to open a stream.");
	return dual_camera_loop(video, buffers[0]);
#else
	LOG_INF("UVC webcam enabled - waiting for host to open the stream.");
	while (!capture_halted) {
		/*
		 * Wait for the host to commit/open the stream. The UVC class
		 * only reports a format once the host has issued COMMIT.
		 */
		if (!uvc_stream_open(uvc_dev)) {
			k_msleep(10);
			continue;
		}

		if (capture_session_start(video, buffers, N_CAPTURE_BUFFERS)) {
			capture_session_teardown(video);
			if (!capture_halted) {
				k_msleep(100);	/* Back off; don't busy-loop on failure. */
			}
			continue;
		}

		/* Stream frames until the host closes the stream or capture stalls. */
		while (uvc_stream_open(uvc_dev) && capture_and_send(video) == 0) {
		}

		capture_session_teardown(video);
		LOG_INF("Capture session stopped");
	}

	return -EIO;
#endif
}
