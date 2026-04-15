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
#include <string.h>
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
#define JPEG_HEADER_SIZE      CONFIG_VIDEO_JPEG_HANTRO_VC9000E_HEADER_SIZE

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

/**
 * @brief Test basic JPEG encoding.
 *
 * Encodes the full test image at the configured quality and saves
 * the output to the filesystem.
 *
 * @param jpeg_dev Pointer to the JPEG encoder device.
 *
 * @return 0 on success, negative errno on failure.
 */
static int test_jpeg_encode(const struct device *jpeg_dev)
{
	struct video_format fmt = {0};
	struct video_buffer *dequeued_buf = NULL;
	int ret;

	LOG_INF("Starting JPEG encoding test...");

	/* Set video format */
	fmt.pixelformat = VIDEO_PIX_FMT_NV12;
	fmt.width = TEST_WIDTH;
	fmt.height = TEST_HEIGHT;
	fmt.pitch = TEST_WIDTH;

	ret = video_set_format(jpeg_dev, VIDEO_EP_OUT, &fmt);
	if (ret < 0) {
		LOG_ERR("Failed to set format: %d", ret);
		return ret;
	}

	LOG_INF("Format set: %ux%u, format: NV12", fmt.width, fmt.height);

	/* Set JPEG quality */
	uint16_t quality = JPEG_COMPRESS_QUALITY;

	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_COMPRESSION_QUALITY, &quality);
	if (ret < 0) {
		LOG_ERR("Failed to set quality: %d", ret);
		return ret;
	}

	LOG_INF("Quality set to: %u", quality);

	/* Set input buffer address */
	ret = video_set_ctrl(jpeg_dev, VIDEO_CID_JPEG_INPUT_BUFFER, input_buffer->buffer);
	if (ret < 0) {
		LOG_ERR("Failed to set input buffer: %d", ret);
		return ret;
	}

	/* Get output buffer */
	output_buffer->bytesused = OUTPUT_BUFFER_SIZE;

	/* Enqueue output buffer */
	ret = video_enqueue(jpeg_dev, VIDEO_EP_OUT, output_buffer);
	if (ret < 0) {
		LOG_ERR("Failed to enqueue buffer: %d", ret);
		return ret;
	}

	LOG_INF("Buffer enqueued");

	ret = video_stream_start(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Failed to start stream: %d", ret);
		return ret;
	}

	LOG_INF("Stream started, waiting for encoding...");

	/* Dequeue encoded buffer */
	ret = video_dequeue(jpeg_dev, VIDEO_EP_OUT, &dequeued_buf, K_SECONDS(5));
	if (ret < 0) {
		LOG_ERR("Failed to dequeue buffer: %d", ret);
		video_stream_stop(jpeg_dev);
		return ret;
	}

	/* Stop streaming */
	ret = video_stream_stop(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Failed to stop stream: %d", ret);
		return ret;
	}

	/* Print results */
	LOG_INF("=== JPEG Encoding Results ===");
	LOG_INF("Input size:  %u bytes (%ux%u YUV420)", INPUT_BUFFER_SIZE, TEST_WIDTH, TEST_HEIGHT);
	LOG_INF("Output size: %u bytes (JPEG)", dequeued_buf->bytesused);
	LOG_INF("Compression ratio: %.2f:1", (double)INPUT_BUFFER_SIZE / dequeued_buf->bytesused);

	/* Verify JPEG header */
	if (dequeued_buf->bytesused > 2) {
		uint8_t *jpeg_data = (uint8_t *)dequeued_buf->buffer;

		if (jpeg_data[0] == 0xFF && jpeg_data[1] == 0xD8) {
			LOG_INF("JPEG header verified (SOI marker found)");
		} else {
			LOG_WRN("JPEG header invalid: 0x%02X%02X", jpeg_data[0], jpeg_data[1]);
		}
	}

	LOG_INF("Jpeg: Capture Image: dump memory "
		"\"/home/$USER/capture_img_q10.jpg\" 0x%08x 0x%08x\n",
		(uint32_t)output_buffer->buffer,
		(uint32_t)output_buffer->buffer + dequeued_buf->bytesused);

	LOG_INF(" Jpeg Basic Test completed successfully!");
	return 0;
}

/**
 * @brief Main application entry point.
 *
 * Initializes the JPEG encoder device, allocates video buffers, loads
 * the test image, and runs the configured encoding test functions.
 *
 * @return 0 on success, negative errno on failure.
 */
int main(void)
{
	const struct device *jpeg_dev;
	struct video_caps caps;
	int ret;

	LOG_INF("=== VeriSilicon Hantro VC9000E JPEG Encoder Test ===");

	/* Get JPEG device */
	jpeg_dev = DEVICE_DT_GET(JPEG_DEVICE_NODE);
	if (!device_is_ready(jpeg_dev)) {
		LOG_ERR("JPEG device not ready");
		return -ENODEV;
	}

	LOG_INF("JPEG device ready: %s", jpeg_dev->name);

	/* Allocate input buffer from video buffer pool */
	input_buffer = video_buffer_alloc(INPUT_BUFFER_SIZE, K_NO_WAIT);
	if (input_buffer == NULL) {
		LOG_ERR("Failed to allocate input buffer");
		return -ENOMEM;
	}
	LOG_INF("Allocated input buffer at %p with (%u bytes)\n", (void *)input_buffer->buffer,
		INPUT_BUFFER_SIZE);

	/* Get input buffer and copy test image */
	copy_testimg(&testimg[0], (uint8_t *)input_buffer->buffer);

	/* Allocate output buffer from video buffer pool */
	output_buffer = video_buffer_alloc(OUTPUT_BUFFER_SIZE + JPEG_HEADER_SIZE, K_NO_WAIT);
	if (output_buffer == NULL) {
		LOG_ERR("Failed to allocate output buffer");
		video_buffer_release(input_buffer);
		return -ENOMEM;
	}
	LOG_INF("Allocated output buffer at %p with (%u bytes)\n", (void *)output_buffer->buffer,
		OUTPUT_BUFFER_SIZE + JPEG_HEADER_SIZE);

	/* Get capabilities */
	ret = video_get_caps(jpeg_dev, VIDEO_EP_OUT, &caps);
	if (ret == 0) {
		LOG_INF("JPEG Encoder Capabilities:");
		for (int i = 0; caps.format_caps[i].pixelformat != 0; i++) {
			LOG_INF("  Format: 0x%08x, Size: %ux%u to %ux%u",
				caps.format_caps[i].pixelformat, caps.format_caps[i].width_min,
				caps.format_caps[i].height_min, caps.format_caps[i].width_max,
				caps.format_caps[i].height_max);
		}
	}
	/* Run basic encoding test */
	ret = test_jpeg_encode(jpeg_dev);
	if (ret < 0) {
		LOG_ERR("Basic encoding test failed: %d", ret);
		return ret;
	}

	/* Free allocated buffers */
	video_buffer_release(input_buffer);
	video_buffer_release(output_buffer);

	LOG_INF("Buffers released");
	return 0;
}
