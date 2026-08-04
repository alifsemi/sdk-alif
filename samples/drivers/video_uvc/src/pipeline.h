/*
 * Copyright (C) 2026 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Build-time pipeline configuration for the UVC webcam sample.
 *
 * The whole sample supports two capture pipelines, chosen automatically from
 * the target's devicetree. This header hides the resulting preprocessor
 * selection so main.c can stay focused on the capture/stream logic:
 *
 *   - MJPEG (E8 DK): a CSI-2 sensor -> ISP (NV12) -> Hantro JPEG encoder. The
 *     sensor and its resolutions are picked from the enabled sensor node.
 *   - RGB565 (E1C / B1 SK): an OV5640 parallel camera off the LP-CAM, streamed
 *     uncompressed.
 *
 * UVC_OUTPUT_MJPEG is the single predicate the rest of the sample switches on.
 */

#ifndef VIDEO_UVC_PIPELINE_H_
#define VIDEO_UVC_PIPELINE_H_

#include <zephyr/devicetree.h>
#include <zephyr/drivers/video.h>
#include <zephyr/sys/util.h>

/* MJPEG when the JPEG hardware encoder (jpeg0) is present, else RGB565. */
#define UVC_OUTPUT_MJPEG DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(jpeg0))

/* DMA frame-address alignment required by the camera/ISP masters. */
#define CAPTURE_BUF_ALIGN 32U

#if UVC_OUTPUT_MJPEG

/* VIDEO_PIX_FMT_NV12 (the ISP output / JPEG input) is Alif-specific. */
#include <zephyr/drivers/video/video_alif.h>

/*
 * MJPEG stream geometry, per sensor: the MT9M114 streams its native 1280x720,
 * the ARX3A0 its native 560x560, and the OV5675 its full-FOV 1296x972 mode
 * downscaled by the ISP to 1024x768 (4:3, 16-aligned).
 */
#if DT_NODE_EXISTS(DT_NODELABEL(mt9m114_selfie))
#define UVC_FRAME_WIDTH  1280U
#define UVC_FRAME_HEIGHT 720U
#elif DT_NODE_EXISTS(DT_NODELABEL(ov5675_selfie))
#define UVC_FRAME_WIDTH  1024U
#define UVC_FRAME_HEIGHT 768U
#else /* ARX3A0 */
#define UVC_FRAME_WIDTH  560U
#define UVC_FRAME_HEIGHT 560U
#endif

/* Capture = ISP output: NV12 at the stream resolution (1.5 bytes/pixel). */
#define CAPTURE_FORMAT VIDEO_PIX_FMT_NV12
#define CAPTURE_WIDTH  UVC_FRAME_WIDTH
#define CAPTURE_HEIGHT UVC_FRAME_HEIGHT
#define CAPTURE_PITCH  ((CAPTURE_WIDTH * 3U) / 2U)

/* The Hantro encoder consumes its NV12 input with a 16-pixel-aligned pitch. */
#define JPEG_INPUT_PITCH ROUND_UP(CAPTURE_WIDTH, 16)
#define JPEG_QUALITY     50

/*
 * A JPEG frame is always smaller than the uncompressed image, so the
 * uncompressed luma size (width*height) is a safe output-buffer ceiling that
 * also scales with resolution. Too small a buffer makes the encoder abort with
 * -ENOSPC and drop the frame.
 */
#define JPEG_OUTPUT_MAX_BYTES (CAPTURE_WIDTH * CAPTURE_HEIGHT)

/*
 * ISP input (sensor) format: RAW10 (Y10P) over MIPI CSI-2, at the sensor's
 * native resolution. The ISP debayers/scales it to the NV12 output above
 * (ARX3A0 and MT9M114 pass through 1:1; OV5675's full-FOV frame is downscaled).
 * The ISP keeps input and output formats separately and must have its input
 * set before stream_start, or it fails with -EINVAL.
 */
#define ISP_INPUT_FORMAT VIDEO_PIX_FMT_Y10P
#if DT_NODE_EXISTS(DT_NODELABEL(mt9m114_selfie))
#define ISP_INPUT_WIDTH  1288U
#define ISP_INPUT_HEIGHT 728U
#elif DT_NODE_EXISTS(DT_NODELABEL(ov5675_selfie))
#define ISP_INPUT_WIDTH  1296U
#define ISP_INPUT_HEIGHT 972U
#else /* ARX3A0 */
#define ISP_INPUT_WIDTH  560U
#define ISP_INPUT_HEIGHT 560U
#endif
#define ISP_INPUT_PITCH (ISP_INPUT_WIDTH * 2U)

#else /* uncompressed RGB565 */

#define UVC_FRAME_WIDTH  480U
#define UVC_FRAME_HEIGHT 272U

/* Capture = LP-CAM output: RGB565 straight from the parallel camera. */
#define CAPTURE_FORMAT      VIDEO_PIX_FMT_RGB565
#define CAPTURE_WIDTH       UVC_FRAME_WIDTH
#define CAPTURE_HEIGHT      UVC_FRAME_HEIGHT
#define UVC_BYTES_PER_PIXEL 2U
#define CAPTURE_PITCH       (CAPTURE_WIDTH * UVC_BYTES_PER_PIXEL)

#endif /* UVC_OUTPUT_MJPEG */

/* Bytes in one captured frame (NV12 or RGB565). */
#define CAPTURE_FRAME_BYTES (CAPTURE_PITCH * CAPTURE_HEIGHT)

#endif /* VIDEO_UVC_PIPELINE_H_ */
