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
#define TEST_JPEG_BASIC_ENCODING   1
#define TEST_JPEG_THUMB_RGB24      0

#if TEST_JPEG_THUMB_RGB24
/* Thumbnail dimensions */
#define THUMB_WIDTH      64
#define THUMB_HEIGHT     48
#define THUMB_RGB24_SIZE (THUMB_WIDTH * THUMB_HEIGHT * 3)

static uint8_t thumb_rgb24_buf[THUMB_RGB24_SIZE];
#endif

/**
 * @brief Header bytes reserved in the output buffer.
 *
 * An RGB24 thumbnail has a fixed size, so this matches the value the
 * driver gets from jpeg_header_get_size() at encode time.
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
#endif

	return jpeg_header_get_size(thumb, NULL);
}

/* Video buffers - only need 1 of each for single-frame encoding */
static struct video_buffer *input_buffer;
static struct video_buffer *output_buffer;

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

#if TEST_JPEG_THUMB_RGB24
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

/**
 * @brief Configure the encoder for one frame.
 *
 * NV12 is set on VIDEO_EP_IN. JPEG is set on VIDEO_EP_OUT at the same
 * resolution. Quality, thumbnail, and the input buffer address follow.
 * A NULL thumbnail clears any thumbnail left by an earlier test.
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

	ret = video_set_ctrl(dev, VIDEO_CID_JPEG_INPUT_BUFFER, cfg->input_addr);
	if (ret < 0) {
		LOG_ERR("jpeg_configure: set input buffer failed: %d", ret);
		return ret;
	}

	return 0;
}

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

	input_buffer = video_buffer_alloc(INPUT_BUFFER_SIZE, K_NO_WAIT);
	if (input_buffer == NULL) {
		LOG_ERR("Failed to allocate input buffer");
		return -ENOMEM;
	}
	LOG_INF("Allocated input buffer at %p with (%u bytes)\n",
		(void *)input_buffer->buffer, INPUT_BUFFER_SIZE);

	copy_testimg(&testimg[0], (uint8_t *)input_buffer->buffer);

#if TEST_JPEG_THUMB_RGB24
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

	/* Output buffer stays allocated so the dump address remains valid. */
	video_buffer_release(input_buffer);

	LOG_INF("=== Jpeg Test Completed Successfully ===");
	return 0;
}
