/*
 * Copyright (C) 2026 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include <zephyr/drivers/video.h>
#include <soc_common.h>
#include <se_service.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/drivers/video/video_alif.h>

#ifdef CONFIG_DT_HAS_HIMAX_HM0360_ENABLED
#include <zephyr/drivers/video/hm0360-video-controls.h>
#endif /* CONFIG_DT_HAS_HIMAX_HM0360_ENABLED */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(video_app, LOG_LEVEL_INF);

#define N_FRAMES		10
#define N_VID_BUFF              MIN(CONFIG_VIDEO_BUFFER_POOL_NUM_MAX, N_FRAMES)

#define ISP_ENABLED DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(isp))

#ifdef CONFIG_DT_HAS_HIMAX_HM0360_ENABLED
#define PIPELINE_FORMAT	VIDEO_PIX_FMT_BGGR8
#else
#define PIPELINE_FORMAT	VIDEO_PIX_FMT_Y10P
#endif /* CONFIG_DT_HAS_HIMAX_HM0360_ENABLED */

#if ISP_ENABLED
#define OUTPUT_FORMAT	VIDEO_PIX_FMT_RGB888_PLANAR_PRIVATE
#endif

#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
#define NUM_CAMS DT_PROP_LEN(DT_NODELABEL(csi), phy_if)
#else
#define NUM_CAMS 1
#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */

static int fourcc_to_pitch(uint32_t fourcc, uint32_t width)
{
	int pitch;

	switch (fourcc) {
	case VIDEO_PIX_FMT_RGB888_PLANAR_PRIVATE:
	case VIDEO_PIX_FMT_NV24:
	case VIDEO_PIX_FMT_NV42:
		pitch = width * 3;
		break;
	case VIDEO_PIX_FMT_RGB565:
	case VIDEO_PIX_FMT_Y10P:
	case VIDEO_PIX_FMT_BGGR10:
	case VIDEO_PIX_FMT_GBRG10:
	case VIDEO_PIX_FMT_GRBG10:
	case VIDEO_PIX_FMT_RGGB10:
	case VIDEO_PIX_FMT_BGGR12:
	case VIDEO_PIX_FMT_GBRG12:
	case VIDEO_PIX_FMT_GRBG12:
	case VIDEO_PIX_FMT_RGGB12:
	case VIDEO_PIX_FMT_BGGR14:
	case VIDEO_PIX_FMT_GBRG14:
	case VIDEO_PIX_FMT_GRBG14:
	case VIDEO_PIX_FMT_RGGB14:
	case VIDEO_PIX_FMT_BGGR16:
	case VIDEO_PIX_FMT_GBRG16:
	case VIDEO_PIX_FMT_GRBG16:
	case VIDEO_PIX_FMT_RGGB16:
	case VIDEO_PIX_FMT_Y10:
	case VIDEO_PIX_FMT_Y12:
	case VIDEO_PIX_FMT_Y14:
	case VIDEO_PIX_FMT_YUYV:
	case VIDEO_PIX_FMT_YVYU:
	case VIDEO_PIX_FMT_VYUY:
	case VIDEO_PIX_FMT_UYVY:
	case VIDEO_PIX_FMT_NV16:
	case VIDEO_PIX_FMT_NV61:
	case VIDEO_PIX_FMT_YUV422P:
		pitch = width << 1;
		break;
	case VIDEO_PIX_FMT_NV12:
	case VIDEO_PIX_FMT_NV21:
	case VIDEO_PIX_FMT_YUV420:
	case VIDEO_PIX_FMT_YVU420:
		pitch = (width * 3) >> 1;
		break;
	case VIDEO_PIX_FMT_BGGR8:
	case VIDEO_PIX_FMT_GBRG8:
	case VIDEO_PIX_FMT_GRBG8:
	case VIDEO_PIX_FMT_RGGB8:
	case VIDEO_PIX_FMT_GREY:
	default:
		pitch = width;
		break;
	}

	return pitch;
}

int main(void)
{
	struct video_buffer *buffers[N_VID_BUFF], *vbuf;
	struct video_format fmt = { 0 };
	struct video_caps caps[NUM_CAMS];
	const struct device *video;
	enum video_endpoint_id ep;
	unsigned int frame = 0;
	size_t bsize;
	int i = 0;
	int ret;

#ifdef CONFIG_DT_HAS_HIMAX_HM0360_ENABLED
	uint32_t num_frames;
#endif /* CONFIG_DT_HAS_HIMAX_HM0360_ENABLED */

#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
	uint8_t current_sensor;
#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */
	int loop_ctr;

	uint32_t last_timestamp = 0;
	uint32_t frame_time = 0;

#if ISP_ENABLED
	video = DEVICE_DT_GET_ONE(vsi_isp_pico);
#else
	video = DEVICE_DT_GET_ONE(alif_cam);
#endif /* DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(isp), okay)*/

	if (!device_is_ready(video)) {
		LOG_ERR("%s: device not ready.", video->name);
		return -1;
	}
	LOG_INF("- Device name: %s", video->name);

	for (loop_ctr = NUM_CAMS - 1; loop_ctr >= 0; loop_ctr--) {
#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
		ret = video_get_ctrl(video, VIDEO_CID_ALIF_CSI_CURR_CAM, &current_sensor);
		if (ret) {
			LOG_ERR("Failed to get current camera!");
			return ret;
		}
		LOG_INF("Selected camera: %s", (current_sensor) ? "Standard" : "Selfie");
#endif /* CONFIG_VIDEO_ALIF_CAM_EXTENDED */

		if (IS_ENABLED(ISP_ENABLED)) {
			ep = VIDEO_EP_IN;
		} else {
			ep = VIDEO_EP_OUT;
		}

		/* Get capabilities */
		if (video_get_caps(video, ep, &caps[loop_ctr])) {
			LOG_ERR("Unable to retrieve video capabilities");
			return -1;
		}

		LOG_INF("- Capabilities:\n");
		while (caps[loop_ctr].format_caps[i].pixelformat) {
			const struct video_format_cap *fcap = &caps[loop_ctr].format_caps[i];
			/* fourcc to string */
			LOG_INF("  %c%c%c%c width (min, max, step)[%u; %u; %u] "
				"height (min, max, step)[%u; %u; %u]",
			       (char)fcap->pixelformat,
			       (char)(fcap->pixelformat >> 8),
			       (char)(fcap->pixelformat >> 16),
			       (char)(fcap->pixelformat >> 24),
			       fcap->width_min, fcap->width_max, fcap->width_step,
			       fcap->height_min, fcap->height_max, fcap->height_step);
			if (fcap->pixelformat == PIPELINE_FORMAT) {
				fmt.pixelformat = PIPELINE_FORMAT;
				if (IS_ENABLED(CONFIG_DT_HAS_HIMAX_HM0360_ENABLED)) {
					fmt.width = 320;
					fmt.height = 240;
				} else {
					fmt.width = fcap->width_min;
					fmt.height = fcap->height_min;
				}
			}
			i++;
		}

		if (fmt.pixelformat == 0) {
			LOG_ERR("Desired Pixel format is not supported.");
			return -1;
		}

		fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);

		ret = video_set_format(video, ep, &fmt);
		if (ret) {
			LOG_ERR("Failed to set video format. ret - %d", ret);
			return -1;
		}

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
		if (ret) {
			LOG_ERR("Failed to set video format. ret - %d", ret);
			return -1;
		}
#endif /*ISP_ENABLED */

	LOG_INF("- format: %c%c%c%c %ux%u", (char)fmt.pixelformat,
	       (char)(fmt.pixelformat >> 8),
	       (char)(fmt.pixelformat >> 16),
	       (char)(fmt.pixelformat >> 24),
	       fmt.width, fmt.height);

	/* Size to allocate for each buffer */
	bsize = fmt.pitch * fmt.height;

	LOG_INF("Width - %d, Pitch - %d, Height - %d, Buff size - %d",
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
			return -1;
		}

		/* Allocated Buffer Information */
		LOG_INF("- addr - 0x%x, size - %d, bytesused - %d",
			(uint32_t)buffers[i]->buffer,
			bsize,
			buffers[i]->bytesused);

		memset(buffers[i]->buffer, 0, sizeof(char) * bsize);
		video_enqueue(video, VIDEO_EP_OUT, buffers[i]);

		LOG_INF("capture buffer[%d]: dump binary memory "
			"\"/home/$USER/capture_%d.bin\" 0x%08x 0x%08x -r\n",
			i, i, (uint32_t)buffers[i]->buffer,
			(uint32_t)buffers[i]->buffer + bsize - 1);
	}

	/*
	 * TODO: Need to fix this delay.
	 * As per our observation, if we are not giving this much delay
	 * then mt9m114 camera sensor is not setup properly and images its
	 * sending out are not clear.
	 */
	k_msleep(7000);

#if CONFIG_DT_HAS_HIMAX_HM0360_ENABLED
	/* Video test SNAPSHOT capture. */
	num_frames = N_FRAMES;
	ret = video_set_ctrl(video, VIDEO_CID_SNAPSHOT_CAPTURE, &num_frames);
	if (ret) {
		LOG_INF("Snapshot mode not-supported by CMOS sensor.");
	}
#endif

	/* Start video capture */
	ret = video_stream_start(video);
	if (ret) {
		LOG_ERR("Unable to start capture (interface). ret - %d", ret);
		return -1;
	}

	LOG_INF("Capture started");

	for (int i = 0; i < N_FRAMES; i++) {
		ret = video_dequeue(video, VIDEO_EP_OUT, &vbuf, K_FOREVER);
		if (ret) {
			LOG_ERR("Unable to dequeue video buf");
			return -1;
		}

		LOG_INF("Got frame %u! size: %u; timestamp %u ms",
		       frame++, vbuf->bytesused, vbuf->timestamp);

		if (last_timestamp == 0) {
			LOG_INF("FPS: 0.0");
			last_timestamp = vbuf->timestamp;
		} else {
			frame_time = vbuf->timestamp - last_timestamp;
			last_timestamp = vbuf->timestamp;
			LOG_INF("FPS: %f", 1000.0/frame_time);
		}

		if (i < N_FRAMES - N_VID_BUFF) {
			ret = video_enqueue(video, VIDEO_EP_OUT, vbuf);
			if (ret) {
				LOG_ERR("Unable to requeue video buf");
				return -1;
			}

			ret = video_stream_start(video);
			if (ret && ret != -EBUSY) {
				LOG_ERR("Unable to restart capture (interface). ret - %d",
						ret);
				return -1;
			}
		}
	}

	LOG_INF("Calling video flush.");
	video_flush(video, VIDEO_EP_OUT, false);

	LOG_INF("Calling video stream stop.");
	ret = video_stream_stop(video);
	if (ret) {
		LOG_ERR("Unable to stop capture (interface). ret - %d", ret);
		return -1;
	}

	return 0;
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
	sys_write32(0x140001, EXPMST_CAMERA_PIXCLK_CTRL);
#endif

#if (DT_NODE_HAS_STATUS(DT_NODELABEL(lpcam), okay))
	/* Enable LPCAM controller Pixel Clock (XVCLK). */
	/*
	 * Not needed for the time being as LP-CAM supports only
	 * parallel data-mode of cature and only MT9M114 sensor is
	 * tested with parallel data capture which generates clock
	 * internally. But can be used to generate XVCLK from LP CAM
	 * controller.
	 * sys_write32(0x140001, M55HE_CFG_HE_CAMERA_PIXCLK);
	 */
#endif
	return 0;
}

SYS_INIT(app_set_parameters, PRE_KERNEL_1, 46);
