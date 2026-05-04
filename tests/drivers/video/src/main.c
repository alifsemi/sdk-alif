/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <soc_common.h>
#include <se_service.h>
#include <zephyr/drivers/gpio.h>

#include <string.h>

#include "video_common.h"

#ifdef CONFIG_DT_HAS_HIMAX_HM0360_ENABLED
#include <zephyr/drivers/video/hm0360-video-controls.h>
#endif /* CONFIG_DT_HAS_HIMAX_HM0360_ENABLED */

#ifdef CONFIG_POLL
struct k_thread signal_thread;
struct k_poll_signal signal;
struct k_poll_event event;

#define STACKSIZE 1024
K_THREAD_STACK_DEFINE(thread_stack, 1024);
#endif
LOG_MODULE_REGISTER(video_app, LOG_LEVEL_INF);
const struct device *video;

/*
 * Test Case to Capture Image
 */

ZTEST(cpi_manual_testcase, video_test_image_capture)
{
	struct video_buffer *buffers[CONFIG_VIDEO_BUFFER_POOL_NUM_MAX], *vbuf;
	struct video_format fmt = { 0 };
	struct video_caps caps[NUM_CAMS];
	enum video_endpoint_id ep;
	unsigned int frame = 0;
	size_t bsize;
	int i;
	int ret;
	int loop_ctr;
	#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
		uint8_t current_sensor;
	#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */
	/* `video` is acquired and validated by manual_suite_before(). */
	TC_PRINT("- Device name: %s\n", video->name);
	for (loop_ctr = NUM_CAMS - 1; loop_ctr >= 0; loop_ctr--) {
		/* Reset per-camera iteration state: format accumulator and
		 * the caps-walk index must not leak across cameras, else the
		 * second pass would skip formats or walk past the end.
		 */
		i = 0;
		memset(&fmt, 0, sizeof(fmt));
	#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
		ret = video_get_ctrl(video, VIDEO_CID_ALIF_CSI_CURR_CAM, &current_sensor);
		zassert_equal(ret, 0, "Failed to get current camera: %d", ret);
		LOG_INF("Selected camera: %s", (current_sensor) ? "Standard" : "Selfie");
	#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */
		if (IS_ENABLED(ISP_ENABLED)) {
			ep = VIDEO_EP_IN;
		} else {
			ep = VIDEO_EP_OUT;
		}
	/* Get capabilities error check*/
	zassert_false(video_get_caps(video, ep, &caps[loop_ctr]),
					"Unable to retrieve video capabilities");

	TC_PRINT("- Capabilities:\n");
	while (caps[loop_ctr].format_caps[i].pixelformat) {
		const struct video_format_cap *fcap = &caps[loop_ctr].format_caps[i];
		/* fourcc to string */
		TC_PRINT("  %c%c%c%c width (min, max, step)[%u; %u; %u] "
			"height (min, max, step)[%u; %u; %u]\n",
		       (char)fcap->pixelformat,
		       (char)(fcap->pixelformat >> 8),
		       (char)(fcap->pixelformat >> 16),
		       (char)(fcap->pixelformat >> 24),
		       fcap->width_min, fcap->width_max, fcap->width_step,
		       fcap->height_min, fcap->height_max, fcap->height_step);

		if (fcap->pixelformat == PIPELINE_FORMAT) {
			fmt.pixelformat = PIPELINE_FORMAT;
			fmt.width = fcap->width_min;
			fmt.height = fcap->height_min;
			fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);
		}
		i++;
	}

	zassert_not_equal(fmt.pixelformat, 0, "Desired Pixel format is not supported.");
	ret = video_set_format(video, ep, &fmt);
	zassert_equal(ret, 0, "video_set_format on ep=%d failed: %d", ep, ret);
	#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
		if (NUM_CAMS > 1) {
			current_sensor ^= 1;
			ret = video_set_ctrl(video, VIDEO_CID_ALIF_CSI_CURR_CAM,
					&current_sensor);
			if (ret) {
				LOG_ERR("Unable to switch camera!");
			}
		}
	#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */
	}
		#if (ISP_ENABLED)
		/*
		 * Set Output Endpoint format. Ensure that ISP EP-out
		 * format is set while allocating the buffers used to
		 * capture images.
		 */
		fmt.pixelformat = OUTPUT_FORMAT;
		fmt.width = 480;
		fmt.height = 480;
		fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);

		ret = video_set_format(video, VIDEO_EP_OUT, &fmt);
		zassert_equal(ret, 0,
			"ISP EP_OUT video_set_format failed: %d", ret);
	#endif /*ISP_ENABLED */
#if (ISP_ENABLED)
	zassert_false(fmt.pixelformat != OUTPUT_FORMAT, "Unsupported pixel format. fmt - %c%c%c%c",
		(char)fmt.pixelformat,
		(char)(fmt.pixelformat >> 8),
		(char)(fmt.pixelformat >> 16),
		(char)(fmt.pixelformat >> 24));
#else
	zassert_false(fmt.pixelformat != PIPELINE_FORMAT, "Unsupported pixel format. fmt "
		" - %c%c%c%c",
		(char)fmt.pixelformat,
		(char)(fmt.pixelformat >> 8),
		(char)(fmt.pixelformat >> 16),
		(char)(fmt.pixelformat >> 24));
#endif
	TC_PRINT("- format: %c%c%c%c %ux%u\n", (char)fmt.pixelformat,
	       (char)(fmt.pixelformat >> 8),
	       (char)(fmt.pixelformat >> 16),
	       (char)(fmt.pixelformat >> 24),
	       fmt.width, fmt.height);

	/* Size to allocate for each buffer */

	bsize = fmt.pitch * fmt.height;
	TC_PRINT("Width - %d, Pitch - %d, Height - %d, Buff size - %d\n",
			fmt.width, fmt.pitch, fmt.height, bsize);

	#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
		if (NUM_CAMS > 1) {
			current_sensor = 0;
			ret = video_set_ctrl(video, VIDEO_CID_ALIF_CSI_CURR_CAM,
					&current_sensor);
			if (ret) {
				LOG_ERR("Unable to switch camera!");
			}
		}
	#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */
	/* Alloc video buffers and enqueue for capture */

	for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		buffers[i] = video_buffer_alloc(bsize, K_NO_WAIT);
		if (buffers[i] == NULL) {
			LOG_ERR("Unable to alloc video buffer");
			return;
		}

		/* Allocated Buffer Information */

		TC_PRINT("- addr - 0x%x, size - %d, bytesused - %d\n",
			(uint32_t)buffers[i]->buffer,
			bsize,
			buffers[i]->bytesused);
		memset(buffers[i]->buffer, 0, sizeof(char) * bsize);
		ret = video_enqueue(video, VIDEO_EP_OUT, buffers[i]);
		zassert_equal(ret, 0, "video_enqueue buf %d failed: %d", i, ret);
		TC_PRINT("capture buffer[%d]: dump binary memory "
			"\"/home/$USER/path/capture_%d.bin\" 0x%08x 0x%08x -r\n\n",
			i, i, (uint32_t)buffers[i]->buffer,
			(uint32_t)buffers[i]->buffer + bsize - 1);
	}

	/* Start video capture */

	ret = video_stream_start(video);
	zassert_equal(ret, 0, "video_stream_start failed: %d", ret);
	TC_PRINT("Capture started\n");
	for (int f = 0; f < 1; f++) {
		ret = video_dequeue(video, VIDEO_EP_OUT, &vbuf, K_FOREVER);
		zassert_equal(ret, 0, "video_dequeue failed: %d", ret);
		TC_PRINT("\rGot frame %u! size: %u; timestamp %u ms\n",
		       frame++, vbuf->bytesused, vbuf->timestamp);
		zassert_true(vbuf->bytesused > 0,
			"bytesused is 0 — frame is empty");
		zassert_equal(vbuf->bytesused, bsize,
			"bytesused %u != expected bsize %u",
			vbuf->bytesused, (uint32_t)bsize);
		ret = video_enqueue(video, VIDEO_EP_OUT, vbuf);
		zassert_equal(ret, 0, "video_enqueue (re) failed: %d", ret);
	}

	k_msleep(10);

	LOG_INF("Calling video flush.");
	ret = video_flush(video, VIDEO_EP_OUT, false);
	zassert_equal(ret, 0, "video_flush failed: %d", ret);
	LOG_INF("Calling video stream stop.");
	ret = video_stream_stop(video);
	zassert_equal(ret, 0, "video_stream_stop failed: %d", ret);

	/*
	 * Drain any buffers left in the driver fifos and return them to the
	 * pool. Called with cancel=false, which per the Zephyr video API
	 * means "wait for completion" rather than "abort" — relying on the
	 * Alif CPI flush implementation draining fifo_in → fifo_out
	 * synchronously so the K_NO_WAIT dequeue loop below sees every
	 * buffer.
	 */
	video_flush(video, VIDEO_EP_OUT, false);
	LOG_INF("Calling video flush done.");
	for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		struct video_buffer *drained = NULL;

		video_dequeue(video, VIDEO_EP_OUT, &drained, K_NO_WAIT);
		if (drained) {
			video_buffer_release(drained);
		}
	}
}

#ifdef CONFIG_POLL

void test_signal_thread(void *arg1, void *arg2, void *arg3)
{
	int signaled, result;

	k_poll_event_init(&event, K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &signal);
	while (1) {
		k_poll(&event, 1, K_FOREVER);
		k_poll_signal_check(&signal, &signaled, &result);
		if (signaled) {
			switch (result) {
			case VIDEO_BUF_DONE:
				LOG_INF("Buffer captured correctly!");
				break;
			case VIDEO_BUF_ABORTED:
				LOG_INF("Flush operation with Abort completed successfully!");
				break;
			case VIDEO_BUF_ERROR:
				LOG_ERR("Issues with currently captured buffer. Will recapture. "
						"If problem persists, check timings!");
				break;
			}
			k_poll_signal_reset(&signal);
		}
	}
}
#endif

/* video_api_set_signal
 */
ZTEST(cpi_manual_testcase, video_api_set_signal)
{
	struct video_caps caps;
#ifdef CONFIG_POLL
	int ret;
#endif

	/* `video` is acquired and validated by manual_suite_before(). */
	TC_PRINT("- Device name: %s\n", video->name);
#ifdef CONFIG_POLL
	k_poll_signal_init(&signal);
	ret = video_set_signal(video, VIDEO_EP_OUT, &signal);
	zassert_equal(ret, 0, "Failed to set signal");
	k_thread_create(&signal_thread, thread_stack,
					K_THREAD_STACK_SIZEOF(thread_stack), test_signal_thread,
					NULL, NULL, NULL, 7, 0, K_NO_WAIT);
#endif
	/* Get capabilities error check*/
	zassert_false(video_get_caps(video, VIDEO_EP_OUT, &caps),
					"Unable to retrieve video capabilities");
}


/*
 * Test: capture N_FRAMES frames and verify each has non-zero bytesused
 * and monotonically non-decreasing timestamps.
 */
ZTEST(cpi_manual_testcase, video_z_capture_n_frames)
{
	struct video_buffer *buffers[CONFIG_VIDEO_BUFFER_POOL_NUM_MAX], *vbuf;
	struct video_format fmt = { 0 };
	struct video_caps caps;
	enum video_endpoint_id ep;
	size_t bsize;
	int i = 0;
	int ret;
	uint32_t prev_ts = 0;
	unsigned int frame = 0;

	/* `video` is acquired and validated by manual_suite_before(). */

	if (IS_ENABLED(ISP_ENABLED)) {
		ep = VIDEO_EP_IN;
	} else {
		ep = VIDEO_EP_OUT;
	}

	zassert_false(video_get_caps(video, ep, &caps),
		"Unable to retrieve video capabilities");

	while (caps.format_caps[i].pixelformat) {
		const struct video_format_cap *fcap = &caps.format_caps[i];

		if (fcap->pixelformat == PIPELINE_FORMAT) {
			fmt.pixelformat = PIPELINE_FORMAT;
			fmt.width = fcap->width_min;
			fmt.height = fcap->height_min;
			fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);
		}
		i++;
	}
	zassert_not_equal(fmt.pixelformat, 0, "Desired pixel format not supported.");

	ret = video_set_format(video, ep, &fmt);
	zassert_equal(ret, 0, "video_set_format failed: %d", ret);

#if ISP_ENABLED
	fmt.pixelformat = OUTPUT_FORMAT;
	fmt.width = 480;
	fmt.height = 480;
	fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);
	ret = video_set_format(video, VIDEO_EP_OUT, &fmt);
	zassert_equal(ret, 0, "ISP EP_OUT set_format failed: %d", ret);
#endif

	bsize = fmt.pitch * fmt.height;

	for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		buffers[i] = video_buffer_alloc(bsize, K_NO_WAIT);
		zassert_not_null(buffers[i], "Unable to alloc video buffer %d", i);
		memset(buffers[i]->buffer, 0, bsize);
		ret = video_enqueue(video, VIDEO_EP_OUT, buffers[i]);
		zassert_equal(ret, 0, "Enqueue failed for buffer %d: %d", i, ret);
	}

	ret = video_stream_start(video);
	zassert_equal(ret, 0, "stream_start failed: %d", ret);
	TC_PRINT("Capture started — expecting %d frames\n", N_FRAMES);

	for (int f = 0; f < N_FRAMES; f++) {
		ret = video_dequeue(video, VIDEO_EP_OUT, &vbuf, K_FOREVER);
		zassert_equal(ret, 0, "dequeue failed at frame %d: %d", f, ret);

		TC_PRINT("Frame %u: bytesused=%u timestamp=%u ms\n",
			frame, vbuf->bytesused, vbuf->timestamp);

		zassert_true(vbuf->bytesused > 0,
			"Frame %d: bytesused is 0 (empty frame)", f);
		zassert_true(vbuf->timestamp >= prev_ts,
			"Frame %d: non-monotonic timestamp %u < prev %u",
			f, vbuf->timestamp, prev_ts);

		prev_ts = vbuf->timestamp;
		frame++;

		ret = video_enqueue(video, VIDEO_EP_OUT, vbuf);
		zassert_equal(ret, 0, "Re-enqueue failed at frame %d: %d", f, ret);
	}

	TC_PRINT("All %d frames captured successfully\n", N_FRAMES);
	k_msleep(10);

	ret = video_stream_stop(video);
	zassert_equal(ret, 0, "video_stream_stop failed: %d", ret);
	ret = video_flush(video, VIDEO_EP_OUT, false);
	zassert_equal(ret, 0, "video_flush failed: %d", ret);

	for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		struct video_buffer *drained = NULL;

		ret = video_dequeue(video, VIDEO_EP_OUT, &drained, K_NO_WAIT);
		zassert_equal(ret, 0,
			"Cleanup dequeue failed at buf %d: %d", i, ret);
		zassert_not_null(drained,
			"Cleanup dequeue returned NULL buffer at %d", i);
		video_buffer_release(drained);
	}
}

/*
 * Do application configurations.
 */
static int app_set_parameters(void)
{
#if (CONFIG_VIDEO_MIPI_CSI2_DW)
	run_profile_t runp;
	int ret;

#if (DT_NODE_HAS_STATUS(DT_NODELABEL(camera_select), okay))
	const struct gpio_dt_spec sel =
		GPIO_DT_SPEC_GET(DT_NODELABEL(camera_select), select_gpios);

	gpio_pin_configure_dt(&sel, GPIO_OUTPUT);
	gpio_pin_set_dt(&sel, 1);
#endif /* (DT_NODE_HAS_STATUS(DT_NODELABEL(camera_sensor), okay)) */

	/* Enable HFOSC (38.4 MHz) and CFG (100 MHz) clock. */
#if defined(CONFIG_SOC_SERIES_E8)
	sys_set_bits(CGU_CLK_ENA, BIT(23) | BIT(7));
#else
	sys_set_bits(CGU_CLK_ENA, BIT(23) | BIT(21));
#endif /* defined (CONFIG_SOC_SERIES_E7) */

	runp.power_domains = PD_SYST_MASK | PD_SSE700_AON_MASK;
	runp.dcdc_voltage  = 825;
	runp.dcdc_mode     = DCDC_MODE_PWM;
	runp.aon_clk_src   = CLK_SRC_LFXO;
	runp.run_clk_src   = CLK_SRC_PLL;
	runp.vdd_ioflex_3V3 = IOFLEX_LEVEL_1V8;
#if defined(CONFIG_RTSS_HP)
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_400MHZ;
#else
	runp.cpu_clk_freq  = CLOCK_FREQUENCY_160MHZ;
#endif

	runp.memory_blocks = MRAM_MASK;
#if DT_NODE_EXISTS(DT_NODELABEL(sram0))
	runp.memory_blocks |= SRAM0_MASK;
#endif

	runp.phy_pwr_gating |= MIPI_TX_DPHY_MASK | MIPI_RX_DPHY_MASK |
		MIPI_PLL_DPHY_MASK | LDO_PHY_MASK;
	runp.ip_clock_gating = CAMERA_MASK | MIPI_CSI_MASK | MIPI_DSI_MASK;

	ret = se_service_set_run_cfg(&runp);
	__ASSERT(ret == 0, "SE: set_run_cfg failed = %d", ret);

	/*
	 * CPI Pixel clock - Generate XVCLK. Used by ARX3A0
	 * TODO: parse this clock from DTS and set on board from camera
	 * controller driver.
	 */
	sys_write32(0x140001, CLKCTRL_PER_MST_CAMERA_PIXCLK_CTRL);
#endif

#if (DT_NODE_HAS_STATUS(DT_NODELABEL(lpcam), okay))
	/* Enable LPCAM controller Pixel Clock (XVCLK). */
	/*
	 * Not needed for the time being as LP-CAM supports only
	 * parallel data-mode of capture and only MT9M114 sensor is
	 * tested with parallel data capture which generates clock
	 * internally. But can be used to generate XVCLK from LP CAM
	 * controller.
	 * sys_write32(0x140001, M55HE_CFG_HE_CAMERA_PIXCLK);
	 */
#endif
	return 0;
}


ZTEST_SUITE(cpi_manual_testcase, NULL, NULL, manual_suite_before, NULL, NULL);
ZTEST_SUITE(cpi_api_testcase, NULL, api_suite_setup, manual_suite_before, NULL,
	    api_suite_teardown);

SYS_INIT(app_set_parameters, PRE_KERNEL_1, 46);
