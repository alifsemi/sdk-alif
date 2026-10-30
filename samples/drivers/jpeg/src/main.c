/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/video/video_alif.h>
#include <zephyr/drivers/video-controls.h>
#include <zephyr/logging/log.h>
#include <jpeg_hantro_vc9000e_sw.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/cache.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(jpeg_test, LOG_LEVEL_INF);

#define JPEG_DEVICE_NODE DT_NODELABEL(jpeg0)

/* Test image dimensions */
#define TEST_WIDTH  1280
#define TEST_HEIGHT 720

/* Buffer sizes */
#define INPUT_BUFFER_SIZE     (TEST_WIDTH * TEST_HEIGHT * 3 / 2) /* YUV420 */
#define OUTPUT_BUFFER_SIZE    (INPUT_BUFFER_SIZE / 2)            /* JPEG compressed */
#define JPEG_COMPRESS_QUALITY 10

/* Macros to JPEG tests */
#define TEST_JPEG_BASIC_ENCODING   0
#define TEST_JPEG_THUMB_RGB24      0
#define TEST_JPEG_THUMB_JPEG       0
#define TEST_JPEG_THUMB_PALETTE    0
#define TEST_JPEG_QUALITY_LEVELS   0
#define TEST_JPEG_CROPPING         0
#define TEST_JPEG_SLICE            0
#define TEST_JPEG_MJPEG            0
#define TEST_JPEG_AVI              1

#if TEST_JPEG_AVI
#include "avi_mux.h"
#endif

#define TEST_JPEG_FRAME (TEST_JPEG_BASIC_ENCODING || TEST_JPEG_THUMB_RGB24 || \
			 TEST_JPEG_THUMB_JPEG || TEST_JPEG_THUMB_PALETTE || \
			 TEST_JPEG_QUALITY_LEVELS || TEST_JPEG_CROPPING)

#if TEST_JPEG_SLICE
#define SLICE_ROWS             4
#define SLICE_HEIGHT_PX        (SLICE_ROWS * 16)
#define SLICE_INPUT_SIZE       (TEST_WIDTH * SLICE_HEIGHT_PX * 3 / 2)
#define SLICE_OUTPUT_SIZE      (128 * 1024)
#define SLICE_ACCUMULATOR_SIZE (1024 * 1024)
#endif

#if TEST_JPEG_MJPEG
#define MJPEG_NUM_FRAMES      30
#define MJPEG_FPS             30
#define MJPEG_STREAM_MAX_SIZE (1024 * 1024)
#endif

#if TEST_JPEG_AVI
#define AVI_NUM_FRAMES    30
#define AVI_FPS           30
#define AVI_FILE_MAX_SIZE (1024 * 1024)
#endif

#if (TEST_JPEG_THUMB_RGB24 || TEST_JPEG_THUMB_JPEG || TEST_JPEG_THUMB_PALETTE)
/* Thumbnail dimensions */
#define THUMB_WIDTH        64
#define THUMB_HEIGHT       48
#define THUMB_RGB24_SIZE   (THUMB_WIDTH * THUMB_HEIGHT * 3)
#define THUMB_YUV420_SIZE  (THUMB_WIDTH * THUMB_HEIGHT * 3 / 2)
#define THUMB_JPEG_MAX_SIZE 8192
#endif

#if (TEST_JPEG_THUMB_RGB24 || TEST_JPEG_THUMB_PALETTE)
static uint8_t thumb_rgb24_buf[THUMB_RGB24_SIZE];
#endif

#if TEST_JPEG_THUMB_JPEG
static uint8_t thumb_jpeg_buf[THUMB_JPEG_MAX_SIZE];
#endif

#if TEST_JPEG_THUMB_PALETTE
#define THUMB_PALETTE_BUF_SIZE (768 + THUMB_WIDTH * THUMB_HEIGHT)
static uint8_t thumb_palette_buf[THUMB_PALETTE_BUF_SIZE];
#endif

#if (TEST_JPEG_FRAME || TEST_JPEG_MJPEG || TEST_JPEG_AVI)
/**
 * @brief Header bytes reserved in the output buffer.
 *
 * RGB24 and palette thumbnails have a fixed size, so this matches the
 * value the driver gets from jpeg_header_get_size() at encode time.
 *
 * A JPEG thumbnail is different. jpeg_header_get_size() adds the
 * compressed length, and that length is thumb_jpeg_size from pass 1.
 * It does not exist yet when the buffer is allocated, and pass 1 itself
 * encodes with no thumbnail. The driver queries the real header on
 * pass 2. The base header is reserved here; the THUMB_RGB24_SIZE tail
 * leaves room for a thumbnail up to THUMB_JPEG_MAX_SIZE.
 *
 * The same call also sizes a slice header. jpeg_header_get_size() adds
 * the DRI segment only when the slice pointer is non-NULL. To reserve
 * that header, fill a jpeg_slice_info with
 * restart_interval = SLICE_ROWS * (TEST_WIDTH / 16) and pass it here.
 */
static uint32_t jpeg_sample_header_size(void)
{
	const struct jpeg_thumbnail_info *thumb = NULL;

#if TEST_JPEG_THUMB_RGB24
	struct jpeg_thumbnail_info thumb_info = {
		.format      = JPEGENC_THUMB_RGB24,
		.width       = THUMB_WIDTH,
		.height      = THUMB_HEIGHT,
		.data        = NULL,
		.data_length = THUMB_RGB24_SIZE,
	};

	thumb = &thumb_info;
#elif TEST_JPEG_THUMB_PALETTE
	struct jpeg_thumbnail_info thumb_info = {
		.format      = JPEGENC_THUMB_PALETTE_RGB8,
		.width       = THUMB_WIDTH,
		.height      = THUMB_HEIGHT,
		.data        = NULL,
		.data_length = THUMB_PALETTE_BUF_SIZE,
	};

	thumb = &thumb_info;
#endif

	return jpeg_header_get_size(thumb, NULL);
}
#endif

#if (TEST_JPEG_FRAME || TEST_JPEG_MJPEG || TEST_JPEG_AVI)
/* Video buffers - only need 1 of each for single-frame encoding */
static struct video_buffer *input_buffer;
static struct video_buffer *output_buffer;
#endif

#if TEST_JPEG_MJPEG
static struct video_buffer *mjpeg_stream_buffer;
#endif

#if TEST_JPEG_AVI
static struct video_buffer *avi_file_buffer;
#endif

#if TEST_JPEG_SLICE
static struct video_buffer *slice_in_buf;
static struct video_buffer *slice_out_buf;
static struct video_buffer *slice_accum_buf;
#endif

/* External symbols from incbin.c */
extern uint8_t testimg_start[];
extern uint8_t testimg_end[];

typedef struct _jpeg_testimg_t {
	uint8_t *buffer_start;
	uint8_t *buffer_end;
	uint32_t row_length;
	uint32_t luma_stride;
	uint32_t chroma_stride;
	uint32_t column_length;
} jpeg_testimg_t;

/** Jpeg encoder configuration for jpeg_configure(). */
struct jpeg_encode_cfg {
	uint32_t width;
	uint32_t height;
	uint16_t quality;
	const struct jpeg_thumbnail_info *thumbnail; /**< NULL = clear */
	const struct jpeg_crop_config *crop;        /**< NULL = clear */
	void *input_addr;
};

jpeg_testimg_t testimg[] = {
	{testimg_start, testimg_end, 1280, 1280, 1280, 720},
};

/**
 * @brief Copy test image data from flash to a destination buffer.
 *
 * @param testimg Pointer to the test image descriptor.
 * @param dest Pointer to the destination buffer.
 */
void copy_testimg(jpeg_testimg_t *testimg, uint8_t *dest)
{
	uint32_t total_iter = testimg->buffer_end - testimg->buffer_start;
	uint32_t src = (uint32_t)testimg->buffer_start;

	for (uint32_t i = 0; i < total_iter; i++) {
		*dest = *(uint8_t *)(src + i);
		dest++;
	}
}

#if (TEST_JPEG_THUMB_RGB24 || TEST_JPEG_THUMB_PALETTE)
/**
 * @brief Downscale NV12 input and convert to RGB24 using nearest-neighbor.
 *
 * @param yuv_src Pointer to the source NV12 image buffer.
 * @param src_w Source image width in pixels.
 * @param src_h Source image height in pixels.
 * @param rgb_dst Pointer to the destination RGB24 buffer.
 * @param dst_w Destination width in pixels.
 * @param dst_h Destination height in pixels.
 */
static void sw_downscale_yuv420_to_rgb24(const uint8_t *yuv_src,
					 uint32_t src_w, uint32_t src_h,
					 uint8_t *rgb_dst,
					 uint32_t dst_w, uint32_t dst_h)
{
	const uint8_t *y_plane = yuv_src;
	const uint8_t *uv_plane = yuv_src + src_w * src_h;

	for (uint32_t dy = 0; dy < dst_h; dy++) {
		uint32_t sy = dy * src_h / dst_h;
		for (uint32_t dx = 0; dx < dst_w; dx++) {
			uint32_t sx = dx * src_w / dst_w;

			int y = y_plane[sy * src_w + sx];
			uint32_t uv_idx = (sy / 2) * src_w + (sx & ~1u);
			int u = uv_plane[uv_idx] - 128;
			int v = uv_plane[uv_idx + 1] - 128;

			int r = y + ((v * 359) >> 8);
			int g = y - ((u * 88 + v * 183) >> 8);
			int b = y + ((u * 454) >> 8);

			*rgb_dst++ = (uint8_t)(r < 0 ? 0 : (r > 255 ? 255 : r));
			*rgb_dst++ = (uint8_t)(g < 0 ? 0 : (g > 255 ? 255 : g));
			*rgb_dst++ = (uint8_t)(b < 0 ? 0 : (b > 255 ? 255 : b));
		}
	}
}
#endif

#if TEST_JPEG_THUMB_JPEG
/**
 * @brief Downscale NV12 image to a smaller NV12 using nearest-neighbor.
 *
 * @param src Pointer to the source NV12 image buffer.
 * @param src_w Source image width in pixels.
 * @param src_h Source image height in pixels.
 * @param dst Pointer to the destination NV12 buffer.
 * @param dst_w Destination width in pixels.
 * @param dst_h Destination height in pixels.
 */
static void sw_downscale_nv12(const uint8_t *src, uint32_t src_w, uint32_t src_h,
			      uint8_t *dst, uint32_t dst_w, uint32_t dst_h)
{
	const uint8_t *src_y = src;
	const uint8_t *src_uv = src + src_w * src_h;
	uint8_t *dst_y = dst;
	uint8_t *dst_uv = dst + dst_w * dst_h;

	for (uint32_t dy = 0; dy < dst_h; dy++) {
		uint32_t sy = dy * src_h / dst_h;
		for (uint32_t dx = 0; dx < dst_w; dx++) {
			uint32_t sx = dx * src_w / dst_w;
			dst_y[dy * dst_w + dx] = src_y[sy * src_w + sx];
		}
	}

	for (uint32_t dy = 0; dy < dst_h / 2; dy++) {
		uint32_t sy = dy * (src_h / 2) / (dst_h / 2);
		for (uint32_t dx = 0; dx < dst_w / 2; dx++) {
			uint32_t sx = dx * (src_w / 2) / (dst_w / 2);
			dst_uv[dy * dst_w + dx * 2]     = src_uv[sy * src_w + sx * 2];
			dst_uv[dy * dst_w + dx * 2 + 1] = src_uv[sy * src_w + sx * 2 + 1];
		}
	}
}
#endif

#if TEST_JPEG_THUMB_PALETTE
/**
 * @brief Convert RGB24 image to 256-color palette format.
 *
 * Uses uniform quantization (R=8, G=8, B=4 levels = 256 colors)
 * to produce a 768-byte RGB palette and per-pixel index data.
 *
 * @param rgb Pointer to the source RGB24 pixel data.
 * @param num_pixels Number of pixels to convert.
 * @param palette_out Pointer to the 768-byte palette output buffer.
 * @param indices_out Pointer to the per-pixel index output buffer.
 */
static void sw_rgb24_to_palette(const uint8_t *rgb, uint32_t num_pixels,
				uint8_t *palette_out, uint8_t *indices_out)
{
	for (int r = 0; r < 8; r++) {
		for (int g = 0; g < 8; g++) {
			for (int b = 0; b < 4; b++) {
				int idx = (r * 32) + (g * 4) + b;

				palette_out[idx * 3]     = (uint8_t)(r * 255 / 7);
				palette_out[idx * 3 + 1] = (uint8_t)(g * 255 / 7);
				palette_out[idx * 3 + 2] = (uint8_t)(b * 255 / 3);
			}
		}
	}

	for (uint32_t i = 0; i < num_pixels; i++) {
		uint8_t ri = rgb[i * 3] * 7 / 255;
		uint8_t gi = rgb[i * 3 + 1] * 7 / 255;
		uint8_t bi = rgb[i * 3 + 2] * 3 / 255;

		indices_out[i] = (uint8_t)((ri * 32) + (gi * 4) + bi);
	}
}
#endif

/**
 * @brief Configure the encoder for one frame.
 *
 * NV12 is set on VIDEO_EP_IN. JPEG is set on VIDEO_EP_OUT at the same
 * resolution. Quality, thumbnail, crop, and the input buffer address follow.
 * A NULL thumbnail or crop clears one left by an earlier test.
 */
static int jpeg_configure(const struct device *dev,
			  const struct jpeg_encode_cfg *cfg)
{
	struct video_format in_fmt = {
		.pixelformat = VIDEO_PIX_FMT_NV12,
		.width  = cfg->width,
		.height = cfg->height,
		.pitch  = cfg->width,
	};
	struct video_format out_fmt = {
		.pixelformat = VIDEO_PIX_FMT_JPEG,
		.width  = cfg->width,
		.height = cfg->height,
	};
	uint16_t q = cfg->quality;
	int ret;

	ret = video_set_format(dev, VIDEO_EP_IN, &in_fmt);
	if (ret < 0) {
		LOG_ERR("jpeg_configure: set input format %ux%u failed: %d",
			cfg->width, cfg->height, ret);
		return ret;
	}

	ret = video_set_format(dev, VIDEO_EP_OUT, &out_fmt);
	if (ret < 0) {
		LOG_ERR("jpeg_configure: set JPEG format %ux%u failed: %d",
			cfg->width, cfg->height, ret);
		return ret;
	}

	ret = video_set_ctrl(dev, VIDEO_CID_JPEG_COMPRESSION_QUALITY, &q);
	if (ret < 0) {
		LOG_ERR("jpeg_configure: set quality=%u failed: %d", q, ret);
		return ret;
	}

	ret = video_set_ctrl(dev, VIDEO_CID_JPEG_THUMBNAIL_CONFIG,
			     (void *)cfg->thumbnail);
	if (ret < 0) {
		LOG_ERR("jpeg_configure: thumbnail %s failed: %d",
			cfg->thumbnail ? "set" : "clear", ret);
		return ret;
	}

	ret = video_set_ctrl(dev, VIDEO_CID_JPEG_CROP_CONFIG,
			     (void *)cfg->crop);
	if (ret < 0) {
		LOG_ERR("jpeg_configure: crop %s failed: %d",
			cfg->crop ? "set" : "clear", ret);
		return ret;
	}

	ret = video_set_ctrl(dev, VIDEO_CID_JPEG_INPUT_BUFFER, cfg->input_addr);
	if (ret < 0) {
		LOG_ERR("jpeg_configure: set input buffer failed: %d", ret);
		return ret;
	}

	return 0;
}

#if (TEST_JPEG_BASIC_ENCODING || TEST_JPEG_THUMB_RGB24 || \
     TEST_JPEG_THUMB_JPEG || TEST_JPEG_THUMB_PALETTE || \
     TEST_JPEG_CROPPING)
/**
 * @brief Single-frame encode: enqueue + stream_start + dequeue + stream_stop.
 *
 * Caller must have set @p output->bytesused to the buffer capacity before
 * calling. On success @p *dequeued points at the encoded buffer.
 */
static int jpeg_encode_one_shot(const struct device *dev,
				struct video_buffer *output,
				struct video_buffer **dequeued)
{
	int ret;

	ret = video_enqueue(dev, VIDEO_EP_OUT, output);
	if (ret < 0) {
		LOG_ERR("jpeg_encode_one_shot: enqueue failed: %d", ret);
		return ret;
	}

	ret = video_stream_start(dev);
	if (ret < 0) {
		LOG_ERR("jpeg_encode_one_shot: stream_start failed: %d", ret);
		return ret;
	}

	ret = video_dequeue(dev, VIDEO_EP_OUT, dequeued, K_SECONDS(5));
	if (ret < 0) {
		LOG_ERR("jpeg_encode_one_shot: dequeue failed: %d", ret);
		(void)video_stream_stop(dev);
		return ret;
	}

	ret = video_stream_stop(dev);
	if (ret < 0) {
		LOG_WRN("jpeg_encode_one_shot: stream_stop failed: %d", ret);
	}

	return 0;
}
#endif

#if (TEST_JPEG_QUALITY_LEVELS || TEST_JPEG_MJPEG || TEST_JPEG_AVI)
/**
 * @brief In-stream encode: enqueue + dequeue only.
 *
 * The caller has already started the stream and stops it after the loop.
 */
static int jpeg_encode_in_stream(const struct device *dev,
				 struct video_buffer *output,
				 struct video_buffer **dequeued)
{
	int ret;

	ret = video_enqueue(dev, VIDEO_EP_OUT, output);
	if (ret < 0) {
		LOG_ERR("jpeg_encode_in_stream: enqueue failed: %d", ret);
		return ret;
	}

	ret = video_dequeue(dev, VIDEO_EP_OUT, dequeued, K_SECONDS(5));
	if (ret < 0) {
		LOG_ERR("jpeg_encode_in_stream: dequeue failed: %d", ret);
		return ret;
	}

	return 0;
}
#endif

#if TEST_JPEG_FRAME
/**
 * @brief Validate JPEG SOI marker and log an OpenOCD dump-memory line.
 *
 * @param filename  File name to embed in the dump command (without path).
 * @param buf       Buffer containing the encoded JPEG.
 * @param bytesused Number of bytes actually written by the encoder.
 */
static void jpeg_verify_and_log_capture(const char *filename,
					const struct video_buffer *buf,
					uint32_t bytesused)
{
	const uint8_t *data = (const uint8_t *)buf->buffer;

	if (bytesused >= 2 && data[0] == 0xFF && data[1] == 0xD8) {
		LOG_INF("JPEG header verified (SOI marker found)");
	} else if (bytesused >= 2) {
		LOG_WRN("JPEG header invalid: 0x%02X%02X", data[0], data[1]);
	} else {
		LOG_WRN("JPEG output too small: %u bytes", bytesused);
	}

	LOG_INF("Jpeg: Capture Image: dump memory "
		"\"/home/$USER/%s\" 0x%08x 0x%08x\n",
		filename,
		(uint32_t)buf->buffer,
		(uint32_t)buf->buffer + bytesused);
}
#endif

#if (TEST_JPEG_BASIC_ENCODING)
/**
 * @brief Test basic JPEG encoding.
 *
 * Encodes the full test image at the configured quality.
 *
 * @param jpeg_dev Pointer to the JPEG encoder device.
 *
 * @return 0 on success, negative errno on failure.
 */
static int test_jpeg_encode(const struct device *jpeg_dev)
{
	struct jpeg_encode_cfg cfg = {
		.width      = TEST_WIDTH,
		.height     = TEST_HEIGHT,
		.quality    = JPEG_COMPRESS_QUALITY,
		.thumbnail  = NULL,
		.crop       = NULL,
		.input_addr = input_buffer->buffer,
	};
	struct video_buffer *dequeued = NULL;
	int ret;

	LOG_INF("Starting JPEG encoding test...");
	LOG_INF("Format: %ux%u NV12, quality=%u",
		cfg.width, cfg.height, cfg.quality);

	ret = jpeg_configure(jpeg_dev, &cfg);
	if (ret < 0) {
		return ret;
	}

	output_buffer->bytesused = OUTPUT_BUFFER_SIZE;

	ret = jpeg_encode_one_shot(jpeg_dev, output_buffer, &dequeued);
	if (ret < 0) {
		return ret;
	}

	LOG_INF("=== JPEG Encoding Results ===");
	LOG_INF("Input size:  %u bytes (%ux%u YUV420)",
		INPUT_BUFFER_SIZE, TEST_WIDTH, TEST_HEIGHT);
	LOG_INF("Output size: %u bytes (JPEG)", dequeued->bytesused);
	LOG_INF("Compression ratio: %.2f:1",
		(double)INPUT_BUFFER_SIZE / dequeued->bytesused);

	jpeg_verify_and_log_capture("capture_img_q10.jpg", dequeued, dequeued->bytesused);

	LOG_INF("Test completed successfully!");
	return 0;
}
#endif

#if TEST_JPEG_THUMB_RGB24
/**
 * @brief Test JPEG encoding with an RGB24 thumbnail.
 *
 * Downscales the input image to thumbnail size, converts to RGB24,
 * and encodes the full image with an embedded RGB24 JFIF thumbnail.
 *
 * @param jpeg_dev Pointer to the JPEG encoder device.
 *
 * @return 0 on success, negative errno on failure.
 */
static int test_jpeg_encode_with_thumbnail(const struct device *jpeg_dev)
{
	struct video_buffer *dequeued = NULL;
	int ret;

	LOG_INF("=== Testing JPEG Encoding with RGB24 Thumbnail ===");

	sw_downscale_yuv420_to_rgb24((const uint8_t *)input_buffer->buffer,
				     TEST_WIDTH, TEST_HEIGHT,
				     thumb_rgb24_buf,
				     THUMB_WIDTH, THUMB_HEIGHT);

	LOG_INF("Thumbnail prepared: %ux%u RGB24 (%u bytes)",
		THUMB_WIDTH, THUMB_HEIGHT, THUMB_RGB24_SIZE);

	struct jpeg_thumbnail_info thumb_info = {
		.format = JPEGENC_THUMB_RGB24,
		.width  = THUMB_WIDTH,
		.height = THUMB_HEIGHT,
		.data   = thumb_rgb24_buf,
		.data_length = THUMB_RGB24_SIZE,
	};
	struct jpeg_encode_cfg cfg = {
		.width      = TEST_WIDTH,
		.height     = TEST_HEIGHT,
		.quality    = JPEG_COMPRESS_QUALITY,
		.thumbnail  = &thumb_info,
		.crop       = NULL,
		.input_addr = input_buffer->buffer,
	};

	ret = jpeg_configure(jpeg_dev, &cfg);
	if (ret < 0) {
		return ret;
	}

	output_buffer->bytesused = OUTPUT_BUFFER_SIZE;

	ret = jpeg_encode_one_shot(jpeg_dev, output_buffer, &dequeued);
	if (ret < 0) {
		return ret;
	}

	LOG_INF("=== Thumbnail Encoding Results ===");
	LOG_INF("Output size: %u bytes (JPEG with %ux%u RGB24 thumbnail)",
		dequeued->bytesused, THUMB_WIDTH, THUMB_HEIGHT);

	jpeg_verify_and_log_capture("capture_thumb_rgb24.jpg", dequeued,
				    dequeued->bytesused);

	LOG_INF("Thumbnail encoding test completed!");
	return 0;
}
#endif

#if TEST_JPEG_THUMB_JPEG
/**
 * @brief Test JPEG encoding with a JPEG compressed thumbnail (two-pass).
 *
 * Pass 1 encodes a downscaled NV12 thumbnail. Pass 2 encodes the full
 * image with that JPEG thumbnail embedded in a JFXX APP0 segment.
 *
 * @param jpeg_dev Pointer to the JPEG encoder device.
 *
 * @return 0 on success, negative errno on failure.
 */
static int test_jpeg_encode_with_jpeg_thumbnail(const struct device *jpeg_dev)
{
	struct video_buffer *dequeued = NULL;
	uint32_t thumb_jpeg_size;
	int ret;

	LOG_INF("=== Testing JPEG Encoding with JPEG Thumbnail (two-pass) ===");

	/*
	 * Downscale NV12 into the tail of output_buffer (DMA-accessible SRAM).
	 * A static buffer can land in DTCM, which the JPEG DMA cannot read.
	 */
	uint8_t *thumb_nv12_dma = (uint8_t *)output_buffer->buffer +
				  OUTPUT_BUFFER_SIZE + jpeg_sample_header_size() +
				  THUMB_RGB24_SIZE - THUMB_YUV420_SIZE;

	sw_downscale_nv12((const uint8_t *)input_buffer->buffer,
			  TEST_WIDTH, TEST_HEIGHT,
			  thumb_nv12_dma,
			  THUMB_WIDTH, THUMB_HEIGHT);

	LOG_INF("Pass 1: Encoding %ux%u thumbnail...", THUMB_WIDTH, THUMB_HEIGHT);

	struct jpeg_encode_cfg pass1_cfg = {
		.width      = THUMB_WIDTH,
		.height     = THUMB_HEIGHT,
		.quality    = JPEG_COMPRESS_QUALITY,
		.thumbnail  = NULL,
		.crop       = NULL,
		.input_addr = thumb_nv12_dma,
	};

	ret = jpeg_configure(jpeg_dev, &pass1_cfg);
	if (ret < 0) {
		return ret;
	}

	output_buffer->bytesused = OUTPUT_BUFFER_SIZE;

	ret = jpeg_encode_one_shot(jpeg_dev, output_buffer, &dequeued);
	if (ret < 0) {
		return ret;
	}

	thumb_jpeg_size = dequeued->bytesused;
	if (thumb_jpeg_size > THUMB_JPEG_MAX_SIZE) {
		LOG_ERR("Thumb JPEG too large: %u > %u",
			thumb_jpeg_size, THUMB_JPEG_MAX_SIZE);
		return -ENOMEM;
	}
	memcpy(thumb_jpeg_buf, dequeued->buffer, thumb_jpeg_size);
	LOG_INF("Pass 1 done: thumb JPEG = %u bytes", thumb_jpeg_size);

	LOG_INF("Pass 2: Encoding %ux%u with JPEG thumbnail...",
		TEST_WIDTH, TEST_HEIGHT);

	struct jpeg_thumbnail_info thumb_info = {
		.format = JPEGENC_THUMB_JPEG,
		.width  = THUMB_WIDTH,
		.height = THUMB_HEIGHT,
		.data   = thumb_jpeg_buf,
		.data_length = thumb_jpeg_size,
	};
	struct jpeg_encode_cfg pass2_cfg = {
		.width      = TEST_WIDTH,
		.height     = TEST_HEIGHT,
		.quality    = JPEG_COMPRESS_QUALITY,
		.thumbnail  = &thumb_info,
		.crop       = NULL,
		.input_addr = input_buffer->buffer,
	};

	ret = jpeg_configure(jpeg_dev, &pass2_cfg);
	if (ret < 0) {
		return ret;
	}

	output_buffer->bytesused = OUTPUT_BUFFER_SIZE;

	ret = jpeg_encode_one_shot(jpeg_dev, output_buffer, &dequeued);
	if (ret < 0) {
		return ret;
	}

	LOG_INF("=== JPEG Thumbnail Results ===");
	LOG_INF("Output size: %u bytes (with %u-byte JPEG thumbnail)",
		dequeued->bytesused, thumb_jpeg_size);

	jpeg_verify_and_log_capture("capture_thumb_jpeg.jpg", dequeued,
				    dequeued->bytesused);

	LOG_INF("JPEG thumbnail test completed!");
	return 0;
}
#endif

#if TEST_JPEG_THUMB_PALETTE
/**
 * @brief Test JPEG encoding with a palette (RGB8) thumbnail.
 *
 * Downscales the input to thumbnail size, converts to RGB24, then
 * quantizes to a 256-color palette. Encodes the full image with the
 * palette thumbnail embedded in a JFXX APP0 extension segment.
 *
 * @param jpeg_dev Pointer to the JPEG encoder device.
 *
 * @return 0 on success, negative errno on failure.
 */
static int test_jpeg_encode_with_palette_thumbnail(const struct device *jpeg_dev)
{
	struct video_buffer *dequeued = NULL;
	int ret;

	LOG_INF("=== Testing JPEG Encoding with Palette Thumbnail ===");

	sw_downscale_yuv420_to_rgb24((const uint8_t *)input_buffer->buffer,
				     TEST_WIDTH, TEST_HEIGHT,
				     thumb_rgb24_buf,
				     THUMB_WIDTH, THUMB_HEIGHT);

	uint32_t num_pixels = THUMB_WIDTH * THUMB_HEIGHT;
	uint8_t *palette = thumb_palette_buf;
	uint8_t *indices = thumb_palette_buf + 768;

	sw_rgb24_to_palette(thumb_rgb24_buf, num_pixels, palette, indices);

	LOG_INF("Palette thumbnail prepared: %ux%u (%u palette + %u index bytes)",
		THUMB_WIDTH, THUMB_HEIGHT, 768, num_pixels);

	struct jpeg_thumbnail_info thumb_info = {
		.format = JPEGENC_THUMB_PALETTE_RGB8,
		.width  = THUMB_WIDTH,
		.height = THUMB_HEIGHT,
		.data   = thumb_palette_buf,
		.data_length = 768 + num_pixels,
	};
	struct jpeg_encode_cfg cfg = {
		.width      = TEST_WIDTH,
		.height     = TEST_HEIGHT,
		.quality    = JPEG_COMPRESS_QUALITY,
		.thumbnail  = &thumb_info,
		.crop       = NULL,
		.input_addr = input_buffer->buffer,
	};

	ret = jpeg_configure(jpeg_dev, &cfg);
	if (ret < 0) {
		return ret;
	}

	output_buffer->bytesused = OUTPUT_BUFFER_SIZE;

	ret = jpeg_encode_one_shot(jpeg_dev, output_buffer, &dequeued);
	if (ret < 0) {
		return ret;
	}

	LOG_INF("=== Palette Thumbnail Results ===");
	LOG_INF("Output size: %u bytes (with palette thumbnail)", dequeued->bytesused);

	jpeg_verify_and_log_capture("capture_thumb_palette.jpg", dequeued,
				    dequeued->bytesused);

	LOG_INF("Palette thumbnail test completed!");
	return 0;
}
#endif

#if TEST_JPEG_QUALITY_LEVELS
/**
 * @brief Test encoding at multiple JPEG quality factors.
 *
 * Encodes the test image at each quality in one streaming session and
 * logs the output size and encode time.
 *
 * @param jpeg_dev Pointer to the JPEG encoder device.
 *
 * @return 0 on success, negative errno on failure.
 */
static int test_quality_levels(const struct device *jpeg_dev)
{
	uint16_t quality_levels[] = {25, 50, 75, 100};
	struct jpeg_encode_cfg cfg = {
		.width      = TEST_WIDTH,
		.height     = TEST_HEIGHT,
		.quality    = quality_levels[0],
		.thumbnail  = NULL,
		.crop       = NULL,
		.input_addr = input_buffer->buffer,
	};
	int ret;

	LOG_INF("=== Testing Multiple Quality Levels ===");

	ret = jpeg_configure(jpeg_dev, &cfg);
	if (ret < 0) {
		return ret;
	}

	ret = video_stream_start(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Failed to start stream: %d", ret);
		return ret;
	}

	LOG_INF("Quality, Size(bytes), Ratio");
	LOG_INF("----------------------------------------");

	for (int i = 0; i < ARRAY_SIZE(quality_levels); i++) {
		struct video_buffer *dequeued = NULL;
		uint16_t quality = quality_levels[i];
		char dump_name[32];

		ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_COMPRESSION_QUALITY,
				     &quality);
		if (ret < 0) {
			LOG_ERR("Failed to set quality=%u: %d", quality, ret);
			continue;
		}

		output_buffer->bytesused = OUTPUT_BUFFER_SIZE;

		ret = jpeg_encode_in_stream(jpeg_dev, output_buffer,
					    &dequeued);
		if (ret < 0) {
			continue;
		}

		LOG_INF("%3u,     %6u,      %.2f:1",
			quality, dequeued->bytesused,
			(double)INPUT_BUFFER_SIZE / dequeued->bytesused);

		snprintf(dump_name, sizeof(dump_name), "capture_img_q%u.jpg", quality);
		jpeg_verify_and_log_capture(dump_name, dequeued, dequeued->bytesused);
	}

	(void)video_stream_stop(jpeg_dev);

	LOG_INF("Quality level test completed!");
	return 0;
}
#endif

#if TEST_JPEG_CROPPING
/**
 * @brief Test JPEG encoding with hardware cropping.
 *
 * Encodes a center 640x360 window of the 1280x720 test image. The source
 * format stays the full frame; the crop window is the control.
 *
 * @param jpeg_dev Pointer to the JPEG encoder device.
 *
 * @return 0 on success, negative errno on failure.
 */
static int test_jpeg_encode_with_crop(const struct device *jpeg_dev)
{
	struct video_buffer *dequeued = NULL;
	int ret;
	struct jpeg_crop_config crop = {
		.x      = (TEST_WIDTH - 640) / 2,
		.y      = (TEST_HEIGHT - 360) / 2,
		.width  = 640,
		.height = 360,
	};
	struct jpeg_encode_cfg cfg = {
		.width      = TEST_WIDTH,
		.height     = TEST_HEIGHT,
		.quality    = JPEG_COMPRESS_QUALITY,
		.thumbnail  = NULL,
		.crop       = &crop,
		.input_addr = input_buffer->buffer,
	};

	LOG_INF("=== Testing JPEG Encoding with Hardware Crop ===");
	LOG_INF("Crop: x=%u y=%u w=%u h=%u",
		crop.x, crop.y, crop.width, crop.height);

	ret = jpeg_configure(jpeg_dev, &cfg);
	if (ret < 0) {
		return ret;
	}

	output_buffer->bytesused = OUTPUT_BUFFER_SIZE;

	ret = jpeg_encode_one_shot(jpeg_dev, output_buffer, &dequeued);
	if (ret < 0) {
		return ret;
	}

	LOG_INF("Crop encode complete: %u bytes", dequeued->bytesused);
	LOG_INF("Output JPEG should be 640x360 center crop");

	jpeg_verify_and_log_capture("capture_crop.jpg", dequeued, dequeued->bytesused);

	LOG_INF("Crop encoding test completed!");
	return 0;
}
#endif

#if TEST_JPEG_SLICE
/**
 * @brief Copy a horizontal slice from the full NV12 test image.
 *
 * @param src       Full NV12 source image.
 * @param dst       Destination slice buffer.
 * @param width     Image width in pixels.
 * @param full_h    Full image height in pixels.
 * @param slice_idx Index of the slice to copy (0-based).
 * @param slice_h   Slice height in pixels.
 */
static void copy_nv12_slice(const uint8_t *src, uint8_t *dst,
			    uint32_t width, uint32_t full_h,
			    uint32_t slice_idx, uint32_t slice_h)
{
	uint32_t y0 = slice_idx * slice_h;
	uint32_t this_h = slice_h;

	if (y0 + this_h > full_h) {
		this_h = full_h - y0;
	}

	memcpy(dst, src + y0 * width, this_h * width);

	uint32_t uv_offset = width * full_h + (y0 / 2) * width;

	memcpy(dst + slice_h * width, src + uv_offset, (this_h / 2) * width);
}

/**
 * @brief Encode the test image one slice at a time and concatenate the JPEG.
 *
 * Each enqueue produces one slice. Slice 0 carries the header. The app
 * copies every slice into one buffer. Play the result with ffplay.
 *
 * @param jpeg_dev Pointer to the JPEG encoder device.
 *
 * @return 0 on success, negative errno on failure.
 */
static int test_jpeg_encode_slice(const struct device *jpeg_dev)
{
	uint32_t slice_rows = SLICE_ROWS;
	uint32_t total_slices = DIV_ROUND_UP(TEST_HEIGHT, SLICE_HEIGHT_PX);
	struct video_buffer *dequeued = NULL;
	uint32_t accum_offset = 0;
	uint32_t total_bytes = 0;
	int ret;
	struct jpeg_encode_cfg cfg = {
		.width      = TEST_WIDTH,
		.height     = TEST_HEIGHT,
		.quality    = JPEG_COMPRESS_QUALITY,
		.thumbnail  = NULL,
		.crop       = NULL,
		.input_addr = slice_in_buf->buffer,
	};

	LOG_INF("=== Starting JPEG Slice-Mode Test ===");
	LOG_INF("Format: %ux%u NV12, slice height=%u px (%u MCU rows), "
		"total slices=%u",
		TEST_WIDTH, TEST_HEIGHT, SLICE_HEIGHT_PX, slice_rows,
		total_slices);

	ret = jpeg_configure(jpeg_dev, &cfg);
	if (ret < 0) {
		return ret;
	}

	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_SLICE_HEIGHT, &slice_rows);
	if (ret < 0) {
		LOG_ERR("Failed to set slice height: %d", ret);
		return ret;
	}

	ret = video_stream_start(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Failed to start stream: %d", ret);
		return ret;
	}

	for (uint32_t i = 0; i < total_slices; i++) {
		copy_nv12_slice(testimg_start, slice_in_buf->buffer,
				TEST_WIDTH, TEST_HEIGHT, i, SLICE_HEIGHT_PX);

		(void)sys_cache_data_flush_and_invd_range(slice_in_buf->buffer,
							  SLICE_INPUT_SIZE);

		ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_INPUT_BUFFER,
				     slice_in_buf->buffer);
		if (ret < 0) {
			LOG_ERR("Slice %u: failed to set input buffer: %d", i, ret);
			(void)video_stream_stop(jpeg_dev);
			return ret;
		}

		slice_out_buf->bytesused = SLICE_OUTPUT_SIZE;

		ret = video_enqueue(jpeg_dev, VIDEO_EP_OUT, slice_out_buf);
		if (ret < 0) {
			LOG_ERR("Slice %u: enqueue failed: %d", i, ret);
			(void)video_stream_stop(jpeg_dev);
			return ret;
		}

		ret = video_dequeue(jpeg_dev, VIDEO_EP_OUT, &dequeued, K_MSEC(200));
		if (ret < 0) {
			LOG_ERR("Slice %u: dequeue failed: %d", i, ret);
			(void)video_stream_stop(jpeg_dev);
			return ret;
		}

		if (accum_offset + dequeued->bytesused > SLICE_ACCUMULATOR_SIZE) {
			LOG_ERR("Slice %u: accumulator overflow (%u + %u > %u)",
				i, accum_offset, dequeued->bytesused,
				SLICE_ACCUMULATOR_SIZE);
			(void)video_stream_stop(jpeg_dev);
			return -ENOSPC;
		}

		memcpy((uint8_t *)slice_accum_buf->buffer + accum_offset,
		       dequeued->buffer, dequeued->bytesused);
		accum_offset += dequeued->bytesused;
		total_bytes += dequeued->bytesused;
	}

	ret = video_stream_stop(jpeg_dev);
	if (ret < 0) {
		LOG_WRN("stream_stop failed: %d", ret);
	}

	(void)sys_cache_data_flush_and_invd_range(slice_accum_buf->buffer,
						  accum_offset);

	{
		const uint8_t *jpeg_data = (const uint8_t *)slice_accum_buf->buffer;

		if (accum_offset >= 2 && jpeg_data[0] == 0xFF && jpeg_data[1] == 0xD8) {
			LOG_INF("JPEG header verified (SOI marker found)");
		} else {
			LOG_WRN("JPEG header invalid or missing");
		}
	}

	LOG_INF("=== Slice-Mode Results ===");
	LOG_INF("Total slices:    %u", total_slices);
	LOG_INF("Total size:      %u bytes (%u KB)",
		total_bytes, total_bytes / 1024U);

	LOG_INF("Jpeg: Capture Slice JPEG: dump memory "
		"\"/home/$USER/capture_slice.jpg\" 0x%08x 0x%08x",
		(uint32_t)slice_accum_buf->buffer,
		(uint32_t)slice_accum_buf->buffer + accum_offset);

	LOG_INF("Verify RST markers: xxd capture_slice.jpg | grep -E 'ffd[0-7]'");
	LOG_INF("Play with: ffplay capture_slice.jpg");

	uint32_t zero = 0;

	(void)video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_SLICE_HEIGHT, &zero);

	LOG_INF("Slice-mode test completed!");
	return 0;
}
#endif

#if (TEST_JPEG_MJPEG || TEST_JPEG_AVI)
/**
 * @brief Paint a horizontal white band into an NV12 image.
 *
 * White is Y=235, U=128, V=128. The UV band is widened by one row so
 * chroma covers the luma band.
 */
static void nv12_paint_white_band(uint8_t *buf, uint32_t width, uint32_t height,
				  uint32_t band_y, uint32_t band_h)
{
	uint8_t *y_plane = buf;
	uint8_t *uv_plane = buf + width * height;
	uint32_t uv_y0;
	uint32_t uv_y1;

	if (band_y >= height) {
		return;
	}
	if (band_y + band_h > height) {
		band_h = height - band_y;
	}
	if (band_h == 0U) {
		return;
	}

	for (uint32_t y = band_y; y < band_y + band_h; y++) {
		memset(y_plane + y * width, 235, width);
	}

	uv_y0 = band_y / 2U;
	uv_y1 = (band_y + band_h + 1U) / 2U;
	for (uint32_t y = uv_y0; y < uv_y1; y++) {
		memset(uv_plane + y * width, 128, width);
	}
}
#endif

#if TEST_JPEG_MJPEG
/**
 * @brief Encode the test image repeatedly and concatenate a .mjpg stream.
 *
 * Each frame is a complete JPEG with a white band that moves down the
 * picture. Bytes after EOI are dropped so the concatenated file splits
 * cleanly at each SOI.
 *
 * @param jpeg_dev Pointer to the JPEG encoder device.
 *
 * @return 0 on success, negative errno on failure.
 */
static int test_jpeg_encode_mjpeg(const struct device *jpeg_dev)
{
	struct jpeg_encode_cfg cfg = {
		.width      = TEST_WIDTH,
		.height     = TEST_HEIGHT,
		.quality    = JPEG_COMPRESS_QUALITY,
		.thumbnail  = NULL,
		.crop       = NULL,
		.input_addr = input_buffer->buffer,
	};
	uint8_t *stream = (uint8_t *)mjpeg_stream_buffer->buffer;
	uint32_t stream_offset = 0;
	int ret;

	LOG_INF("=== Testing MJPEG Concat (.mjpg) Encoding ===");
	LOG_INF("Format: %ux%u NV12, quality=%u, frames=%u, fps=%u",
		cfg.width, cfg.height, cfg.quality,
		MJPEG_NUM_FRAMES, MJPEG_FPS);

	ret = jpeg_configure(jpeg_dev, &cfg);
	if (ret < 0) {
		return ret;
	}

	ret = video_stream_start(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Failed to start stream: %d", ret);
		return ret;
	}

	for (uint32_t i = 0; i < MJPEG_NUM_FRAMES; i++) {
		struct video_buffer *dequeued = NULL;
		const uint8_t *fb;
		uint32_t fsize;
		uint32_t trimmed;
		const uint32_t band_h = 60U;
		uint32_t denom = (MJPEG_NUM_FRAMES > 1U) ? (MJPEG_NUM_FRAMES - 1U) : 1U;
		uint32_t band_y = (i * (TEST_HEIGHT - band_h)) / denom;

		copy_testimg(&testimg[0], (uint8_t *)input_buffer->buffer);
		nv12_paint_white_band((uint8_t *)input_buffer->buffer,
				      TEST_WIDTH, TEST_HEIGHT, band_y, band_h);
		(void)sys_cache_data_flush_and_invd_range(input_buffer->buffer,
							  INPUT_BUFFER_SIZE);

		output_buffer->bytesused = OUTPUT_BUFFER_SIZE;

		ret = jpeg_encode_in_stream(jpeg_dev, output_buffer,
					    &dequeued);
		if (ret < 0) {
			LOG_ERR("Frame %u: encode failed: %d", i, ret);
			(void)video_stream_stop(jpeg_dev);
			return ret;
		}

		k_sleep(K_MSEC(500));

		/*
		 * Drop bytes past EOI. bytesused can overshoot the real marker,
		 * and those bytes corrupt the next frame once the JPEGs are
		 * concatenated.
		 */
		fb = (const uint8_t *)dequeued->buffer;
		fsize = dequeued->bytesused;
		trimmed = fsize;

		for (int32_t k = (int32_t)fsize - 2; k >= 0; k--) {
			if (fb[k] == 0xFF && fb[k + 1] == 0xD9) {
				trimmed = (uint32_t)k + 2;
				break;
			}
		}
		if (trimmed != fsize) {
			LOG_DBG("Frame %u: trimmed %u trailing byte(s) past EOI",
				i + 1, fsize - trimmed);
		} else if (fsize < 2 || fb[0] != 0xFF || fb[1] != 0xD8) {
			LOG_WRN("Frame %u: SOI/EOI not found, skipping", i + 1);
			continue;
		}

		if (stream_offset + trimmed > MJPEG_STREAM_MAX_SIZE) {
			LOG_ERR("Frame %u: stream buffer would overflow (%u + %u > %u)",
				i, stream_offset, trimmed, MJPEG_STREAM_MAX_SIZE);
			(void)video_stream_stop(jpeg_dev);
			return -ENOMEM;
		}

		memcpy(stream + stream_offset, fb, trimmed);
		stream_offset += trimmed;

		LOG_INF("Frame %2u: %6u bytes", i + 1, trimmed);
	}

	ret = video_stream_stop(jpeg_dev);
	if (ret < 0) {
		LOG_WRN("Failed to stop stream: %d", ret);
	}

	mjpeg_stream_buffer->bytesused = stream_offset;

	(void)sys_cache_data_flush_and_invd_range(stream, stream_offset);

	LOG_INF("=== MJPEG Concat Results ===");
	LOG_INF("Total frames:  %u", MJPEG_NUM_FRAMES);
	LOG_INF("Total size:    %u bytes (%u KB)",
		stream_offset, stream_offset / 1024U);
	LOG_INF("Avg per frame: %u bytes", stream_offset / MJPEG_NUM_FRAMES);

	LOG_INF("Jpeg: Capture MJPEG: dump memory "
		"\"/home/$USER/capture_video.mjpg\" 0x%08x 0x%08x\n",
		(uint32_t)stream,
		(uint32_t)stream + stream_offset);

	LOG_INF("Play with:  ffplay -framerate %u capture_video.mjpg", MJPEG_FPS);
	LOG_INF("Probe with: ffprobe capture_video.mjpg");
	LOG_INF("Verify   :  xxd capture_video.mjpg | grep -c 'ffd8'   # expect %u",
		MJPEG_NUM_FRAMES);

	LOG_INF("MJPEG concat test completed!");
	return 0;
}
#endif

#if TEST_JPEG_AVI
/**
 * @brief Encode 30 frames into an MJPEG AVI, with a moving white band.
 *
 * @param jpeg_dev Pointer to the JPEG encoder device.
 *
 * @return 0 on success, negative errno on failure.
 */
static int test_jpeg_encode_avi(const struct device *jpeg_dev)
{
	struct jpeg_encode_cfg cfg = {
		.width      = TEST_WIDTH,
		.height     = TEST_HEIGHT,
		.quality    = JPEG_COMPRESS_QUALITY,
		.thumbnail  = NULL,
		.crop       = NULL,
		.input_addr = input_buffer->buffer,
	};
	struct avi_mux mux;
	uint8_t *avi_buf = (uint8_t *)avi_file_buffer->buffer;
	int ret;

	LOG_INF("=== Testing MJPEG-in-AVI (.avi) Encoding ===");
	LOG_INF("Format: %ux%u NV12, quality=%u, frames=%u, fps=%u",
		cfg.width, cfg.height, cfg.quality,
		AVI_NUM_FRAMES, AVI_FPS);

	ret = avi_mux_init(&mux, avi_buf, AVI_FILE_MAX_SIZE,
			   TEST_WIDTH, TEST_HEIGHT, AVI_FPS);
	if (ret < 0) {
		LOG_ERR("avi_mux_init failed: %d", ret);
		return ret;
	}

	ret = jpeg_configure(jpeg_dev, &cfg);
	if (ret < 0) {
		return ret;
	}

	ret = video_stream_start(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Failed to start stream: %d", ret);
		return ret;
	}

	for (uint32_t i = 0; i < AVI_NUM_FRAMES; i++) {
		struct video_buffer *dequeued = NULL;
		const uint8_t *fb;
		uint32_t fsize;
		uint32_t trimmed;
		const uint32_t band_h = 60U;
		uint32_t denom = (AVI_NUM_FRAMES > 1U) ? (AVI_NUM_FRAMES - 1U) : 1U;
		uint32_t band_y = (i * (TEST_HEIGHT - band_h)) / denom;

		copy_testimg(&testimg[0], (uint8_t *)input_buffer->buffer);
		nv12_paint_white_band((uint8_t *)input_buffer->buffer,
				      TEST_WIDTH, TEST_HEIGHT, band_y, band_h);
		(void)sys_cache_data_flush_and_invd_range(input_buffer->buffer,
							  INPUT_BUFFER_SIZE);

		output_buffer->bytesused = OUTPUT_BUFFER_SIZE;

		ret = jpeg_encode_in_stream(jpeg_dev, output_buffer,
					    &dequeued);
		if (ret < 0) {
			LOG_ERR("Frame %u: encode failed: %d", i, ret);
			(void)video_stream_stop(jpeg_dev);
			return ret;
		}

		k_sleep(K_MSEC(500));

		fb = (const uint8_t *)dequeued->buffer;
		fsize = dequeued->bytesused;
		trimmed = fsize;

		for (int32_t k = (int32_t)fsize - 2; k >= 0; k--) {
			if (fb[k] == 0xFF && fb[k + 1] == 0xD9) {
				trimmed = (uint32_t)k + 2;
				break;
			}
		}
		if (trimmed != fsize) {
			LOG_DBG("Frame %u: trimmed %u trailing byte(s) past EOI",
				i + 1, fsize - trimmed);
		} else if (fsize < 2 || fb[0] != 0xFF || fb[1] != 0xD8) {
			LOG_WRN("Frame %u: SOI/EOI not found, skipping", i + 1);
			continue;
		}

		ret = avi_mux_write_frame(&mux, fb, trimmed);
		if (ret < 0) {
			LOG_ERR("Frame %u: avi_mux_write_frame failed: %d", i + 1, ret);
			(void)video_stream_stop(jpeg_dev);
			return ret;
		}

		LOG_INF("Frame %2u: %6u bytes", i + 1, trimmed);
	}

	ret = video_stream_stop(jpeg_dev);
	if (ret < 0) {
		LOG_WRN("Failed to stop stream: %d", ret);
	}

	ret = avi_mux_finalize(&mux);
	if (ret < 0) {
		LOG_ERR("avi_mux_finalize failed: %d", ret);
		return ret;
	}

	avi_file_buffer->bytesused = mux.pos;

	(void)sys_cache_data_flush_and_invd_range(avi_buf, mux.pos);

	LOG_INF("=== AVI Mux Results ===");
	LOG_INF("Total frames:  %u", AVI_NUM_FRAMES);
	LOG_INF("File size:     %u bytes (%u KB)", mux.pos, mux.pos / 1024U);
	LOG_INF("Avg per frame: %u bytes", mux.pos / AVI_NUM_FRAMES);

	LOG_INF("Jpeg: Capture AVI: dump memory "
		"\"/home/$USER/capture_video.avi\" 0x%08x 0x%08x\n",
		(uint32_t)avi_buf,
		(uint32_t)avi_buf + mux.pos);

	LOG_INF("Play with:  ffplay capture_video.avi");
	LOG_INF("Probe with: ffprobe capture_video.avi");

	LOG_INF("AVI mux test completed!");
	return 0;
}
#endif

/**
 * @brief Main application entry point.
 *
 * Initializes the JPEG encoder device, allocates video buffers, loads
 * the test image, and runs the basic encoding test.
 *
 * @return 0 on success, negative errno on failure.
 */
int main(void)
{
	const struct device *jpeg_dev;
	struct video_caps caps;
	int ret;

	LOG_INF("=== VeriSilicon Hantro VC9000E JPEG Encoder Test ===");

	jpeg_dev = DEVICE_DT_GET(JPEG_DEVICE_NODE);
	if (!device_is_ready(jpeg_dev)) {
		LOG_ERR("JPEG device not ready");
		return -ENODEV;
	}

	LOG_INF("JPEG device ready: %s", jpeg_dev->name);

#if (TEST_JPEG_FRAME || TEST_JPEG_MJPEG || TEST_JPEG_AVI)
	input_buffer = video_buffer_alloc(INPUT_BUFFER_SIZE, K_NO_WAIT);
	if (input_buffer == NULL) {
		LOG_ERR("Failed to allocate input buffer");
		return -ENOMEM;
	}
	LOG_INF("Allocated input buffer at %p with (%u bytes)\n",
		(void *)input_buffer->buffer, INPUT_BUFFER_SIZE);

	copy_testimg(&testimg[0], (uint8_t *)input_buffer->buffer);

#if (TEST_JPEG_THUMB_RGB24 || TEST_JPEG_THUMB_JPEG || TEST_JPEG_THUMB_PALETTE)
	output_buffer = video_buffer_alloc(OUTPUT_BUFFER_SIZE + jpeg_sample_header_size() +
					   THUMB_RGB24_SIZE, K_NO_WAIT);
#else
	output_buffer = video_buffer_alloc(OUTPUT_BUFFER_SIZE + jpeg_sample_header_size(),
					   K_NO_WAIT);
#endif
	if (output_buffer == NULL) {
		LOG_ERR("Failed to allocate output buffer");
		video_buffer_release(input_buffer);
		return -ENOMEM;
	}
	LOG_INF("Allocated output buffer at %p with (%u bytes)\n",
		(void *)output_buffer->buffer, output_buffer->size);
#endif
#if TEST_JPEG_MJPEG
	mjpeg_stream_buffer = video_buffer_alloc(MJPEG_STREAM_MAX_SIZE, K_NO_WAIT);
	if (mjpeg_stream_buffer == NULL) {
		LOG_ERR("Failed to allocate MJPEG stream buffer");
		video_buffer_release(output_buffer);
		video_buffer_release(input_buffer);
		return -ENOMEM;
	}
	LOG_INF("Allocated MJPEG stream buffer at %p with (%u bytes)\n",
		(void *)mjpeg_stream_buffer->buffer, MJPEG_STREAM_MAX_SIZE);
#endif
#if TEST_JPEG_AVI
	avi_file_buffer = video_buffer_alloc(AVI_FILE_MAX_SIZE, K_NO_WAIT);
	if (avi_file_buffer == NULL) {
		LOG_ERR("Failed to allocate AVI file buffer");
		video_buffer_release(output_buffer);
		video_buffer_release(input_buffer);
		return -ENOMEM;
	}
	LOG_INF("Allocated AVI file buffer at %p with (%u bytes)\n",
		(void *)avi_file_buffer->buffer, AVI_FILE_MAX_SIZE);
#endif
#if TEST_JPEG_SLICE
	slice_in_buf = video_buffer_alloc(SLICE_INPUT_SIZE, K_NO_WAIT);
	if (slice_in_buf == NULL) {
		LOG_ERR("Failed to allocate slice input buffer");
		return -ENOMEM;
	}
	LOG_INF("Allocated slice input buffer at %p with (%u bytes)\n",
		(void *)slice_in_buf->buffer, SLICE_INPUT_SIZE);

	slice_out_buf = video_buffer_alloc(SLICE_OUTPUT_SIZE, K_NO_WAIT);
	if (slice_out_buf == NULL) {
		LOG_ERR("Failed to allocate slice output buffer");
		video_buffer_release(slice_in_buf);
		return -ENOMEM;
	}
	LOG_INF("Allocated slice output buffer at %p with (%u bytes)\n",
		(void *)slice_out_buf->buffer, SLICE_OUTPUT_SIZE);

	slice_accum_buf = video_buffer_alloc(SLICE_ACCUMULATOR_SIZE, K_NO_WAIT);
	if (slice_accum_buf == NULL) {
		LOG_ERR("Failed to allocate slice accumulator buffer");
		video_buffer_release(slice_out_buf);
		video_buffer_release(slice_in_buf);
		return -ENOMEM;
	}
	LOG_INF("Allocated slice accumulator buffer at %p with (%u bytes)\n",
		(void *)slice_accum_buf->buffer, SLICE_ACCUMULATOR_SIZE);
#endif

	ret = video_get_caps(jpeg_dev, VIDEO_EP_IN, &caps);
	if (ret == 0) {
		LOG_INF("JPEG Encoder Capabilities:");
		for (int i = 0; caps.format_caps[i].pixelformat != 0; i++) {
			LOG_INF("  Format: 0x%08x, Size: %ux%u to %ux%u",
				caps.format_caps[i].pixelformat,
				caps.format_caps[i].width_min,
				caps.format_caps[i].height_min,
				caps.format_caps[i].width_max,
				caps.format_caps[i].height_max);
		}
	}

#if (TEST_JPEG_BASIC_ENCODING)
	ret = test_jpeg_encode(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Basic encoding test failed: %d", ret);
		return ret;
	}
	k_sleep(K_MSEC(500));
#endif
#if TEST_JPEG_THUMB_RGB24
	ret = test_jpeg_encode_with_thumbnail(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("RGB24 thumbnail test failed: %d", ret);
		return ret;
	}
	k_sleep(K_MSEC(500));
#endif
#if TEST_JPEG_THUMB_JPEG
	ret = test_jpeg_encode_with_jpeg_thumbnail(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("JPEG thumbnail test failed: %d", ret);
		return ret;
	}
	k_sleep(K_MSEC(500));
#endif
#if TEST_JPEG_THUMB_PALETTE
	ret = test_jpeg_encode_with_palette_thumbnail(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Palette thumbnail test failed: %d", ret);
		return ret;
	}
	k_sleep(K_MSEC(500));
#endif
#if TEST_JPEG_QUALITY_LEVELS
	ret = test_quality_levels(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Quality level test failed: %d", ret);
		return ret;
	}
	k_sleep(K_MSEC(500));
#endif
#if TEST_JPEG_CROPPING
	ret = test_jpeg_encode_with_crop(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Crop encoding test failed: %d", ret);
		return ret;
	}
	k_sleep(K_MSEC(500));
#endif
#if TEST_JPEG_SLICE
	ret = test_jpeg_encode_slice(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Slice-mode test failed: %d", ret);
		return ret;
	}
	k_sleep(K_MSEC(500));
#endif
#if TEST_JPEG_MJPEG
	ret = test_jpeg_encode_mjpeg(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("MJPEG concat test failed: %d", ret);
		return ret;
	}
	k_sleep(K_MSEC(500));
#endif
#if TEST_JPEG_AVI
	ret = test_jpeg_encode_avi(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("AVI mux test failed: %d", ret);
		return ret;
	}
	k_sleep(K_MSEC(500));
#endif

#if (TEST_JPEG_FRAME || TEST_JPEG_MJPEG || TEST_JPEG_AVI)
	/* Output and stream buffers stay allocated so the dump address remains valid. */
	video_buffer_release(input_buffer);
#endif
#if TEST_JPEG_SLICE
	/* Accumulator stays allocated so the dump address remains valid. */
	video_buffer_release(slice_in_buf);
	video_buffer_release(slice_out_buf);
#endif

	LOG_INF("=== Jpeg Test Completed Successfully ===");
	return 0;
}
