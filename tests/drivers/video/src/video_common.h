/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef ALIF_CPI_TEST_VIDEO_COMMON_H_
#define ALIF_CPI_TEST_VIDEO_COMMON_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video/video_alif.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>

/* CAM controller register offsets / bits used by test suite hooks. */
#define CAM_CTRL		0x00
#define CAM_CTRL_BUSY		BIT(2)

#define N_FRAMES		10
#define N_VID_BUFF		MIN(CONFIG_VIDEO_BUFFER_POOL_NUM_MAX, N_FRAMES)

#define ISP_ENABLED		DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(isp))

/*
 * Endpoint on which the sensor/pipeline pixel format (PIPELINE_FORMAT) is
 * negotiated and configured:
 *   - Direct-CAM  (ISP disabled): sensor feeds EP_OUT directly.
 *   - ISP-enabled: sensor feeds ISP at EP_IN; EP_OUT carries OUTPUT_FORMAT.
 */
#if ISP_ENABLED
#define CAPTURE_EP		VIDEO_EP_IN
#else
#define CAPTURE_EP		VIDEO_EP_OUT
#endif

#ifdef CONFIG_DT_HAS_HIMAX_HM0360_ENABLED
#define PIPELINE_FORMAT		VIDEO_PIX_FMT_BGGR8
#elif CONFIG_DT_HAS_OVTI_OV5640_ENABLED
#define PIPELINE_FORMAT		VIDEO_PIX_FMT_RGB565
#elif CONFIG_DT_HAS_APTINA_MT9M114_ENABLED
#define PIPELINE_FORMAT		VIDEO_PIX_FMT_GREY
#else
#define PIPELINE_FORMAT		VIDEO_PIX_FMT_Y10P
#endif

#if ISP_ENABLED
#define OUTPUT_FORMAT           VIDEO_PIX_FMT_RGB888_PLANAR_PRIVATE
/* Resolution presented to the application on VIDEO_EP_OUT when the ISP is in
 * the path. The ISP scaler down-samples the sensor frame to this size before
 * conversion to OUTPUT_FORMAT, so buffers enqueued on EP_OUT must be sized
 * for OUTPUT_FORMAT @ ISP_OUTPUT_WIDTH x ISP_OUTPUT_HEIGHT.
 */
#define ISP_OUTPUT_WIDTH        480
#define ISP_OUTPUT_HEIGHT       480
#endif

#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
#define NUM_CAMS		DT_PROP_LEN(DT_NODELABEL(csi), phy_if)
#else
#define NUM_CAMS		1
#endif

/* Shared device handle used by the manual test suite. */
extern const struct device *video;

/* Convert a fourcc pixel format to the pitch (bytes per row) for a given width. */
int fourcc_to_pitch(uint32_t fourcc, uint32_t width);


/**
 * Configure the capture pipeline format on the appropriate endpoint(s).
 *
 * On a direct-CAM build (ISP disabled), this calls
 *     video_set_format(dev, VIDEO_EP_OUT, fmt)
 * exactly once and leaves *fmt untouched.
 *
 * On an ISP-enabled build (e.g. arx3a0 + vsi,isp-pico), this:
 *   1. Sets the sensor / pipeline format on VIDEO_EP_IN (CAPTURE_EP),
 *   2. Rewrites *fmt in place to OUTPUT_FORMAT @ ISP_OUTPUT_WIDTH x
 *      ISP_OUTPUT_HEIGHT (with matching pitch),
 *   3. Sets that output format on VIDEO_EP_OUT.
 *
 * The in-place rewrite means callers can keep their existing
 * `bsize = fmt.pitch * fmt.height;` buffer-size math regardless of build
 * variant — `fmt` after the call always describes what EP_OUT expects.
 *
 * @param dev   video device handle (alif,cam on direct-CAM, vsi,isp-pico
 *              when the ISP is in the path).
 * @param fmt   on entry: pipeline format (PIPELINE_FORMAT, sensor-side
 *              w/h). On exit: identical on direct-CAM; OUTPUT_FORMAT
 *              dimensions on ISP-enabled builds.
 *
 * @return 0 on success, or the first non-zero return code from
 *         video_set_format().
 */
int video_set_capture_format(const struct device *dev,
			     struct video_format *fmt);

/* Suite "before" hook shared by manual + api test suites. */
void manual_suite_before(void *fixture);

/* cpi_api_testcase suite hooks (defined in video_api.c). */
void *api_suite_setup(void);
void api_suite_teardown(void *data);

#endif /* ALIF_CPI_TEST_VIDEO_COMMON_H_ */
