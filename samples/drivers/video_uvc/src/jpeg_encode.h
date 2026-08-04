/*
 * Copyright (C) 2026 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VIDEO_UVC_JPEG_ENCODE_H_
#define VIDEO_UVC_JPEG_ENCODE_H_

#include <stddef.h>
#include <zephyr/device.h>
#include <zephyr/drivers/video.h>

/*
 * Compress one NV12 frame to JPEG with the Hantro hardware encoder.
 *
 * @param jdev     JPEG encoder device (jpeg0).
 * @param nv12     Dequeued ISP output buffer holding the NV12 frame.
 * @param out      Application-owned output buffer for the compressed frame.
 * @param out_len  Set to the compressed length on success.
 *
 * @return 0 on success or a negative errno on failure.
 */
int jpeg_encode(const struct device *jdev, struct video_buffer *nv12,
		struct video_buffer *out, size_t *out_len);

#endif /* VIDEO_UVC_JPEG_ENCODE_H_ */
