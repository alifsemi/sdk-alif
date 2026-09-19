/*
 * Copyright (C) 2026 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Hantro VC9000E hardware JPEG-encoder helper. Kept out of main.c because the
 * encode hand-off is encoder-specific plumbing rather than part of the UVC
 * sample story. The whole file compiles away on the RGB565 (non-JPEG) targets.
 */

#include "pipeline.h"

#if UVC_OUTPUT_MJPEG

#include "jpeg_encode.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/video-controls.h>	/* VIDEO_CID_JPEG_COMPRESSION_QUALITY */
#include <zephyr/drivers/video/video_alif.h>	/* VIDEO_CID_JPEG_INPUT_BUFFER */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(uvc_jpeg, LOG_LEVEL_INF);

int jpeg_encode(const struct device *jdev, struct video_buffer *nv12,
		struct video_buffer *out, size_t *out_len)
{
	struct video_format jfmt = {
		.pixelformat = CAPTURE_FORMAT,
		.width = CAPTURE_WIDTH,
		.height = CAPTURE_HEIGHT,
		.pitch = JPEG_INPUT_PITCH,
	};
	struct video_buffer *deq = NULL;
	uint8_t quality = JPEG_QUALITY;
	int ret;

	ret = video_set_format(jdev, VIDEO_EP_OUT, &jfmt);
	if (ret) {
		LOG_ERR("set_format failed: %d", ret);
		return ret;
	}

	ret = video_set_ctrl(jdev, VIDEO_CID_JPEG_COMPRESSION_QUALITY, &quality);
	if (ret) {
		LOG_ERR("set quality failed: %d", ret);
		return ret;
	}

	/* The encoder takes the input frame by address, not via the queue. */
	ret = video_set_ctrl(jdev, VIDEO_CID_JPEG_INPUT_BUFFER, nv12->buffer);
	if (ret) {
		LOG_ERR("set input buffer failed: %d", ret);
		return ret;
	}

	/*
	 * Tell the encoder the exact compressed-data capacity of the output
	 * buffer (total size minus the space reserved for the JPEG header), so
	 * it never writes past the buffer.
	 */
	out->bytesused = out->size - CONFIG_VIDEO_JPEG_HANTRO_VC9000E_HEADER_SIZE;

	ret = video_enqueue(jdev, VIDEO_EP_OUT, out);
	if (ret) {
		LOG_ERR("enqueue failed: %d", ret);
		return ret;
	}

	ret = video_stream_start(jdev);
	if (ret) {
		LOG_ERR("stream_start failed: %d", ret);
		return ret;
	}

	ret = video_dequeue(jdev, VIDEO_EP_OUT, &deq, K_SECONDS(5));
	if (ret) {
		LOG_ERR("dequeue failed: %d", ret);
		video_stream_stop(jdev);
		return ret;
	}

	video_stream_stop(jdev);

	*out_len = deq->bytesused;
	return 0;
}

#endif /* UVC_OUTPUT_MJPEG */
