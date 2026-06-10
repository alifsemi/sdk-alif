/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
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
#include <zephyr/devicetree.h>
#include <string.h>
#include <zephyr/drivers/video.h>
#include <zephyr/drivers/gpio.h>
#include <soc_common.h>
#include <se_service.h>
#include <zephyr/drivers/video/video_alif.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/policy.h>
#include <zephyr/logging/log.h>

#include "camera_test.h"

LOG_MODULE_DECLARE(cam_pm, LOG_LEVEL_DBG);

/* Number of frames to capture per PM cycle */
#define N_FRAMES		10
#define N_VID_BUFF		2

/* ISP enabled check */
#define ISP_ENABLED DT_NODE_HAS_STATUS_OKAY(DT_NODELABEL(isp))

/* Pipeline format */
#define PIPELINE_FORMAT	VIDEO_PIX_FMT_Y10P

#if ISP_ENABLED
#define OUTPUT_FORMAT	VIDEO_PIX_FMT_RGB888_PLANAR_PRIVATE
#endif

#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
#define NUM_CAMS DT_PROP_LEN(DT_NODELABEL(csi), phy_if)
#else
#define NUM_CAMS 1
#endif

/* size of stack area used by each thread */
#define STACKSIZE 4096

/* Thread Priority */
#define CAMERA_PRIORITY 7

K_THREAD_STACK_DEFINE(CameraT_stack, STACKSIZE);
static struct k_thread thread_camera;

/* Semaphore for PM suspend/resume synchronization */
K_SEM_DEFINE(camera_pm_resume_sem, 0, 1);

/* Semaphore to signal that camera thread has suspended */
K_SEM_DEFINE(camera_pm_suspended_sem, 0, 1);

/* Semaphore to signal that a capture cycle has completed */
K_SEM_DEFINE(camera_capture_done_sem, 0, 1);

/* Thread control flags */
static volatile uint8_t THREAD_TO_BE_SUSPEND;
static volatile uint8_t THREAD_SUSPENDED;
static volatile uint8_t THREAD_TO_BE_STOPPED;

/* Video buffers */
static struct video_buffer *buffers[N_VID_BUFF];

/* Video format */
static struct video_format fmt;

/* Video device */
static const struct device *video_dev;

/* Sensor device for PM actions */
static const struct device *sensor_dev;

/* CSI2 device for PM actions */
static const struct device *csi_dev;

static const struct device *dphy_dev;

/* Buffers allocated flag */
static bool buffers_allocated;

/* Tracks whether capture was active when the PM suspend was requested,
 * so we only auto-restart capture on resume if it was streaming before.
 */
static bool was_streaming;

static bool capture_active;

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

/*
 * Initialize camera and allocate buffers (called once at startup)
 */
static int camera_init(void)
{
	struct video_caps caps[NUM_CAMS];
	enum video_endpoint_id ep;
	size_t bsize;
	int ret;
	int i = 0;
	int loop_ctr;

#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
	uint8_t current_sensor;
#endif

#if ISP_ENABLED
	video_dev = DEVICE_DT_GET_ONE(vsi_isp_pico);
#else
	video_dev = DEVICE_DT_GET_ONE(alif_cam);
#endif

	if (!device_is_ready(video_dev)) {
		LOG_ERR("%s: device not ready.", video_dev->name);
		return -1;
	}
	LOG_INF("- Device name: %s", video_dev->name);

	/* Get sensor device for PM actions */
	sensor_dev = DEVICE_DT_GET_ONE(onnn_arx3a0);
	if (!device_is_ready(sensor_dev)) {
		LOG_WRN("Sensor device not ready for PM actions");
		sensor_dev = NULL;
	}

	csi_dev = DEVICE_DT_GET_ONE(snps_designware_csi);
	if (!device_is_ready(csi_dev)) {
		LOG_WRN("CSI2 device not ready for PM actions");
		csi_dev = NULL;
	}

	dphy_dev = DEVICE_DT_GET_ONE(snps_designware_dphy);
	if (!device_is_ready(dphy_dev)) {
		LOG_WRN("dphy device not ready for PM actions");
		dphy_dev = NULL;
	}

	for (loop_ctr = NUM_CAMS - 1; loop_ctr >= 0; loop_ctr--) {
		/* Reset both index AND fmt for each camera */
		i = 0;
		memset(&fmt, 0, sizeof(fmt));
#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
		ret = video_get_ctrl(video_dev, VIDEO_CID_ALIF_CSI_CURR_CAM, &current_sensor);
		if (ret) {
			LOG_ERR("Failed to get current camera!");
			return ret;
		}
		LOG_INF("Selected camera: %s", (current_sensor) ? "Standard" : "Selfie");
#endif

		if (IS_ENABLED(ISP_ENABLED)) {
			ep = VIDEO_EP_IN;
		} else {
			ep = VIDEO_EP_OUT;
		}

		/* Get capabilities */
		if (video_get_caps(video_dev, ep, &caps[loop_ctr])) {
			LOG_ERR("Unable to retrieve video capabilities");
			return -1;
		}

		LOG_INF("- Capabilities:");
		while (caps[loop_ctr].format_caps[i].pixelformat) {
			const struct video_format_cap *fcap = &caps[loop_ctr].format_caps[i];

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
				fmt.width = fcap->width_min;
				fmt.height = fcap->height_min;
			}
			i++;
		}

		if (fmt.pixelformat == 0) {
			LOG_ERR("Desired Pixel format is not supported.");
			return -1;
		}

		fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);

		LOG_INF("Setting format: %c%c%c%c %ux%u pitch=%d on ep=%d",
			(char)fmt.pixelformat,
			(char)(fmt.pixelformat >> 8),
			(char)(fmt.pixelformat >> 16),
			(char)(fmt.pixelformat >> 24),
			fmt.width, fmt.height, fmt.pitch, ep);

		ret = video_set_format(video_dev, ep, &fmt);
		if (ret) {
			LOG_ERR("Failed to set video format. ret - %d", ret);
			return -1;
		}

#if (CONFIG_VIDEO_ALIF_CAM_EXTENDED && CONFIG_VIDEO_MIPI_CSI2_DW)
		if (NUM_CAMS > 1) {
			current_sensor ^= 1;
			ret = video_set_ctrl(video_dev, VIDEO_CID_ALIF_CSI_CURR_CAM,
					&current_sensor);
			if (ret) {
				LOG_ERR("Unable to switch camera!");
			}
		}
#endif
	}

#if ISP_ENABLED
	/* Set Output Endpoint format for ISP */
	fmt.pixelformat = OUTPUT_FORMAT;
	fmt.width = 480;
	fmt.height = 480;
	fmt.pitch = fourcc_to_pitch(fmt.pixelformat, fmt.width);

	ret = video_set_format(video_dev, VIDEO_EP_OUT, &fmt);
	if (ret) {
		LOG_ERR("Failed to set video format. ret - %d", ret);
		return -1;
	}
#endif

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
		ret = video_set_ctrl(video_dev, VIDEO_CID_ALIF_CSI_CURR_CAM,
				&current_sensor);
		if (ret) {
			LOG_ERR("Unable to switch camera!");
		}
	}
#endif

	/* Alloc video buffers and enqueue for capture */
	for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		buffers[i] = video_buffer_alloc(bsize, K_NO_WAIT);
		if (buffers[i] == NULL) {
			LOG_ERR("Unable to alloc video buffer");
			return -1;
		}

		LOG_INF("- addr - 0x%x, size - %d, bytesused - %d",
			(uint32_t)buffers[i]->buffer,
			bsize,
			buffers[i]->bytesused);

		memset(buffers[i]->buffer, 0, sizeof(char) * bsize);
		video_enqueue(video_dev, VIDEO_EP_OUT, buffers[i]);

		LOG_INF("capture buffer[%d]: dump binary memory "
			"\"/home/$USER/capture_%d.bin\" 0x%08x 0x%08x -r",
			i, i, (uint32_t)buffers[i]->buffer,
			(uint32_t)buffers[i]->buffer + bsize - 1);
	}

	buffers_allocated = true;

	/* Wait for camera sensor to stabilize */
	k_msleep(7000);

	return 0;
}

/*
 * Camera capture thread function
 */
static void camera_capture_thread(void *p1, void *p2, void *p3)
{
	struct video_buffer *vbuf;
	unsigned int frame = 0;
	uint32_t last_timestamp = 0;
	uint32_t frame_time = 0;
	size_t bsize;
	int ret;
	int i, j;
	bool pm_locks_held = false;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!was_streaming) {
		LOG_INF("skip starting a new capture cycle this round");
	}

	/* Initialize camera */
	ret = camera_init();
	if (ret) {
		LOG_ERR("Camera initialization failed: %d", ret);
		return;
	}

	bsize = fmt.pitch * fmt.height;

	while (1) {
		/* Check if thread should be stopped (PM sequence done) */
		if (THREAD_TO_BE_STOPPED) {
			LOG_INF("Camera: thread stopping (PM sequence completed)");
			return;
		}

		/* Re-enqueue buffers for capture */
		if (!capture_active) {
			/*
			 * Check if thread should be suspended (PM cycle).
			 * Only suspend when capture is not active.
			 */
			if (THREAD_TO_BE_SUSPEND) {
				THREAD_SUSPENDED = 1;
				LOG_INF("Camera: putting thread into wait for PM resume");

				/* Signal that we are now suspended */
				k_sem_give(&camera_pm_suspended_sem);

				/* Block on semaphore - does not wake system like k_msleep */
				k_sem_take(&camera_pm_resume_sem, K_FOREVER);

				LOG_INF("Camera: Woken up by PM resume");
				THREAD_SUSPENDED = 0;

				/*
				 * Lock deeper PM states during camera resume
				 * sequence to prevent S2RAM from trashing
				 * hardware state mid-reinit.
				 */
				pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_RAM,
							PM_ALL_SUBSTATES);
				pm_policy_state_lock_get(PM_STATE_SOFT_OFF,
							PM_ALL_SUBSTATES);
				pm_locks_held = true;

				/* PM-cycle the DPHY device first to zero all control registers
				 * back to POR state. Without this, stale register values from
				 * the initial boot-time DPHY setup prevent LP-11 detection.
				 */
				if (dphy_dev) {
					ret = pm_device_action_run(dphy_dev,
						PM_DEVICE_ACTION_SUSPEND);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("DPHY PM suspend failed: %d", ret);
					}
					ret = pm_device_action_run(dphy_dev,
						PM_DEVICE_ACTION_RESUME);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("DPHY PM resume failed: %d", ret);
					}
				}

				/* PM-cycle the CSI2 device to clear cached DPHY/format state.
				 * The CSI2 PM resume handler clears csi_cpi_settings[] and
				 * streaming_map, forcing full DPHY re-initialization on the
				 * next video_set_format() call.
				 */
				if (csi_dev) {
					ret = pm_device_action_run(csi_dev,
						PM_DEVICE_ACTION_SUSPEND);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("CSI2 PM suspend failed: %d", ret);
					}
					ret = pm_device_action_run(csi_dev,
						PM_DEVICE_ACTION_RESUME);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("CSI2 PM resume failed: %d", ret);
					}
				}

				/* PM-cycle the CPI device to reset internal FIFO state and
				 * re-apply full hardware configuration (CAM_CFG, watermarks)
				 * after CSI2 clocks were briefly disrupted.
				 */
				ret = pm_device_action_run(video_dev, PM_DEVICE_ACTION_SUSPEND);
				if (ret && ret != -EALREADY && ret != -ENOSYS) {
					LOG_ERR("CPI PM suspend failed: %d", ret);
				}
				ret = pm_device_action_run(video_dev, PM_DEVICE_ACTION_RESUME);
				if (ret && ret != -EALREADY && ret != -ENOSYS) {
					LOG_ERR("CPI PM resume failed: %d", ret);
				}

				/* Suspend and resume the sensor device through
				 * PM framework. This triggers full sensor
				 * re-initialization required for new frames.
				 */
				/* Resume the sensor device through PM framework.
				 * The sensor was PM-suspended in camera_pm_thread_suspend()
				 * to prevent k_msleep crash in idle thread context.
				 * Now resume it before starting capture.
				 */
				if (sensor_dev) {
					LOG_INF("Resuming sensor device via PM");
					ret = pm_device_action_run(sensor_dev,
							PM_DEVICE_ACTION_RESUME);
					if (ret && ret != -EALREADY && ret != -ENOSYS) {
						LOG_ERR("Sensor PM resume failed: %d", ret);
					}
				}

				/* After STOP, DBSS was fully powered off. Give DPHY analog
				 * (PLL, LDO, lane receivers) time to stabilize after power
				 * domain re-enable before attempting MIPI link setup.
				 */
				k_msleep(50);

				/* Re-apply format to reset ISP/CPI driver state
				 * after PM cycle, matching the working main.c
				 * iteration pattern.
				 */
				LOG_INF("Re-applying video format...");
				ret = video_set_format(video_dev, VIDEO_EP_OUT, &fmt);
				LOG_INF("video_set_format returned: %d", ret);
				if (ret) {
					LOG_ERR("Failed to re-set video format: %d", ret);
				}
				LOG_INF("Waiting for sensor to stabilize...");
				k_msleep(1000);
			}

			/* Flush any stale buffers from driver FIFOs before re-enqueue.
			 * Buffers from camera_init() are still in fifo_in since
			 * streaming was never started before suspend.
			 */
			/* cancel=true to drain immediately */
			video_flush(video_dev, VIDEO_EP_OUT, true);

			/* Drain buffers that flush moved to fifo_out.
			 * Without this, re-enqueueing them into fifo_in causes
			 * the same buffer nodes to be in both FIFOs, corrupting
			 * the linked lists.
			 */
			{
				struct video_buffer *tmp;

			while (video_dequeue(video_dev, VIDEO_EP_OUT,
					&tmp, K_NO_WAIT) == 0) {
				/* Just drain - already tracked in buffers[] */
				}
			}

			/* Clear and re-enqueue all buffers */
			for (i = 0; i < ARRAY_SIZE(buffers); i++) {
				if (buffers[i] != NULL) {
					memset(buffers[i]->buffer, 0,
					       sizeof(char) * bsize);
					buffers[i]->timestamp = 0;
					buffers[i]->bytesused = 0;
					video_enqueue(video_dev, VIDEO_EP_OUT,
						      buffers[i]);
				}
			}

			/* Start video capture */
			ret = video_stream_start(video_dev);
			if (ret) {
				LOG_ERR("Unable to start capture: %d", ret);
				/* Leave the video device in a known state on start failure. */
				video_stream_stop(video_dev);
				video_flush(video_dev, VIDEO_EP_OUT, true);
				/* Release PM policy locks acquired earlier before exiting. */
				if (pm_locks_held) {
					pm_policy_state_lock_put(PM_STATE_STANDBY,
								PM_ALL_SUBSTATES);
					pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM,
								PM_ALL_SUBSTATES);
					pm_policy_state_lock_put(PM_STATE_SOFT_OFF,
								PM_ALL_SUBSTATES);
					pm_locks_held = false;
				}

				return;
			}

			LOG_INF("Capture started");
			frame = 0;
			last_timestamp = 0;
			capture_active = true;
		}

		/*
		 * Bounded capture loop matching working main.c pattern:
		 * - Dequeue each frame
		 * - Re-enqueue only for first (N_FRAMES - N_VID_BUFF) frames
		 * - Let ISP IN-FIFO drain for last N_VID_BUFF frames
		 */
		for (j = 0; j < N_FRAMES; j++) {
			if (THREAD_TO_BE_SUSPEND) {
				LOG_INF("Camera: suspend requested mid-capture, breaking");
				break;
			}
			ret = video_dequeue(video_dev, VIDEO_EP_OUT, &vbuf, K_MSEC(2000));

			if (ret) {
				LOG_ERR("Unable to dequeue video buf: %d", ret);
				break;
			}

			LOG_INF("Got frame %u! size: %u; timestamp %u ms",
				frame++, vbuf->bytesused, vbuf->timestamp);

			/*
			 * Skip FPS for first N_VID_BUFF frames: these are
			 * stale ISP pipeline-flush frames with bogus
			 * timestamps (dequeued almost instantly).
			 */
			if (j < N_VID_BUFF) {
				last_timestamp = vbuf->timestamp;
			} else if (last_timestamp == 0) {
				LOG_INF("FPS: 0.0");
				last_timestamp = vbuf->timestamp;
			} else {
				frame_time = vbuf->timestamp - last_timestamp;
				last_timestamp = vbuf->timestamp;
				if (frame_time > 0) {
					LOG_INF("FPS: %f", 1000.0 / frame_time);
				} else {
					LOG_INF("FPS: N/A (same timestamp)");
				}
			}

			if (j < N_FRAMES - N_VID_BUFF) {
				ret = video_enqueue(video_dev, VIDEO_EP_OUT, vbuf);
				if (ret) {
					LOG_ERR("Unable to requeue video buf: %d",
						ret);
					break;
				}

				ret = video_stream_start(video_dev);
				if (ret && ret != -EBUSY) {
					LOG_ERR("Unable to restart capture: %d",
						ret);
					break;
				}
			}
		}

		/* Drain and stop: matching working main.c sequence */
		LOG_INF("Calling video flush.");
		video_flush(video_dev, VIDEO_EP_OUT, true);
		LOG_INF("Flush done.");

		LOG_INF("Calling video stream stop.");
		ret = video_stream_stop(video_dev);
		if (ret) {
			LOG_ERR("Unable to stop capture: %d", ret);
		}
		LOG_INF("Stream stop done.");

		/* Release PM state locks AFTER capture is fully done */
		if (pm_locks_held) {
			pm_policy_state_lock_put(PM_STATE_SOFT_OFF, PM_ALL_SUBSTATES);
			pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_RAM, PM_ALL_SUBSTATES);
			pm_locks_held = false;
		}

		/* Wait for hardware to fully reset */
		k_msleep(100);

		capture_active = false;
		LOG_INF("Capture completed after %u frames, waiting for PM cycle",
			frame);

		/* Signal main thread that capture cycle is done */
		k_sem_give(&camera_capture_done_sem);

		/* Wait for next PM suspend/resume cycle instead of free-running.
		 * Without this, the thread starts unsolicited captures without
		 * PM lock protection, which can corrupt hardware state if the
		 * system enters deep sleep mid-capture.
		 */
		THREAD_TO_BE_SUSPEND = 1;
	}
}

int camera_pm_thread_init(void)
{
	k_tid_t tid = k_thread_create(&thread_camera, CameraT_stack, STACKSIZE,
			&camera_capture_thread, NULL, NULL, NULL,
			CAMERA_PRIORITY, 0, K_FOREVER);
	if (tid == NULL) {
		LOG_ERR("Error creating Camera Thread");
		return -1;
	}

	return 0;
}

int camera_pm_thread_start(void)
{
	THREAD_TO_BE_SUSPEND = 0;
	THREAD_SUSPENDED = 0;

	k_thread_start(&thread_camera);
	return 0;
}

int camera_pm_thread_suspend(void)
{
	was_streaming = capture_active;
	THREAD_TO_BE_SUSPEND = 1;

	/* Wait for thread to acknowledge suspend request */
	k_sem_take(&camera_pm_suspended_sem, K_FOREVER);

	LOG_INF("Camera thread is now suspended (polling for resume)");

	/* Explicitly PM-suspend the sensor device so the PM framework
	 * skips it during pm_suspend_devices(). The arx3a0 driver uses
	 * k_msleep() in I2C writes which cannot be called from the
	 * idle thread context during pm_system_suspend().
	 */
	if (sensor_dev) {
		int ret = pm_device_action_run(sensor_dev,
				PM_DEVICE_ACTION_SUSPEND);

		if (ret && ret != -EALREADY && ret != -ENOSYS) {
			LOG_ERR("Sensor PM suspend failed: %d", ret);
		}
	}

	return 0;
}

int camera_pm_thread_resume(void)
{
	LOG_INF("Camera: Try to Resume...");

	/* Reset stale signal from interrupted capture cycles */
	k_sem_reset(&camera_capture_done_sem);

	/* Clear the suspend flag and signal semaphore to wake thread */
	THREAD_TO_BE_SUSPEND = 0;
	k_sem_give(&camera_pm_resume_sem);

	LOG_INF("Camera: resume signal sent");
	return 0;
}

int camera_pm_thread_stop(void)
{
	int ret;

	LOG_INF("Camera: requesting thread stop");

	THREAD_TO_BE_STOPPED = 1;

	/* Also clear suspend flag in case thread is currently suspended */
	THREAD_TO_BE_SUSPEND = 0;

	/* Wake thread if blocked on resume semaphore so it can check stop flag */
	k_sem_give(&camera_pm_resume_sem);

	/* Wait for thread to finish */
	ret = k_thread_join(&thread_camera, K_SECONDS(5));
	if (ret != 0) {
		LOG_ERR("Camera: thread join failed or timed out (err: %d)", ret);
		return ret;
	}

	LOG_INF("Camera: thread stopped");
	return 0;
}

int camera_pm_wait_capture_done(void)
{
	return k_sem_take(&camera_capture_done_sem, K_SECONDS(30));
}
