/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef ALIF_JPEG_TEST_COMMON_H_
#define ALIF_JPEG_TEST_COMMON_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video/video_alif.h>
#include <zephyr/drivers/video-controls.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>
#include <stdint.h>
#include <stddef.h>

#define JPEG_DEVICE_NODE	DT_NODELABEL(jpeg0)

/* Reference image embedded via incbin (NV12 / YUV420SP, 1280x720). */
#define REF_IMG_WIDTH		1280
#define REF_IMG_HEIGHT		720
#define REF_IMG_PITCH		1280
#define REF_IMG_NV12_SIZE	(REF_IMG_WIDTH * REF_IMG_HEIGHT * 3 / 2)

#define JPEG_HEADER_SIZE	CONFIG_VIDEO_JPEG_HANTRO_VC9000E_HEADER_SIZE

/* Output buffer sized for the 1280x720 reference image. Generous to absorb
 * worst-case (low compression) high-quality output.
 */
#define DEFAULT_OUT_BUF_SIZE	(REF_IMG_NV12_SIZE + JPEG_HEADER_SIZE)

/* JPEG markers */
#define JPEG_MARKER_SOI_HI	0xFF
#define JPEG_MARKER_SOI_LO	0xD8
#define JPEG_MARKER_EOI_HI	0xFF
#define JPEG_MARKER_EOI_LO	0xD9

/* Embedded reference image (raw NV12 bytes). */
extern uint8_t testimg_start[];
extern uint8_t testimg_end[];

/* Shared device handle. */
extern const struct device *jpeg_dev;

/* Suite hook: acquires + validates the JPEG device. */
void jpeg_suite_before(void *fixture);

/* Helpers shared across tests. */
size_t jpeg_ref_image_size(void);
void jpeg_copy_ref_image(uint8_t *dest);

/* Fill a buffer with a deterministic NV12 gradient pattern of the given
 * dimensions. Buffer must be at least (pitch * height * 3 / 2) bytes.
 */
void jpeg_fill_nv12_gradient(uint8_t *dest, uint32_t width, uint32_t height,
			     uint32_t pitch);

/* Validate that buf[0..size) contains an SOI marker at offset 0 and an
 * EOI marker at the very end. Returns true if both checks pass.
 */
bool jpeg_validate_markers(const uint8_t *buf, size_t size);

/* Deep JFIF structural validation: walks the marker chain and checks
 * that all of the following are present in order:
 *   SOI (FFD8) -> APP0 with "JFIF\0" identifier (FFE0) ->
 *   at least one DQT (FFDB) -> SOF0 (FFC0) ->
 *   at least one DHT (FFC4) -> SOS (FFDA) -> EOI (FFD9).
 * Returns true on a structurally complete JFIF stream.
 */
bool jpeg_validate_jfif_structure(const uint8_t *buf, size_t size);

/* Extract width and height from the SOF0 (baseline) marker of an
 * encoded JPEG stream. SOF0 payload layout (after FFC0):
 *   [len_hi][len_lo][precision][height_hi][height_lo][width_hi][width_lo][nf]...
 * Returns true if SOF0 was found and width*height were populated.
 */
bool jpeg_extract_sof0_dimensions(const uint8_t *buf, size_t size,
				  uint16_t *width, uint16_t *height);

/* Encode helpers (shared across test files). */
void alloc_ref_buffers(struct video_buffer **in, struct video_buffer **out);
void release_buffers(struct video_buffer *in, struct video_buffer *out);
int do_single_encode(struct video_buffer *in, struct video_buffer *out,
		     uint16_t quality, uint32_t width, uint32_t height,
		     uint32_t pitch, struct video_buffer **deq);

/* Allocate a pair of video buffers (input and output). On allocation failure,
 * releases any partial allocations, prints a message, calls ztest_test_skip(),
 * and returns false. On success, returns true.
 */
bool alloc_pair_or_skip(size_t in_sz, size_t out_sz,
			struct video_buffer **in, struct video_buffer **out,
			const char *test_name);

#endif /* ALIF_JPEG_TEST_COMMON_H_ */
