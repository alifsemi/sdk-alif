/*
 * Copyright (C) 2024 Alif Semiconductor.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>

#include <zephyr/drivers/video.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(video_app, LOG_LEVEL_INF);

#define N_FRAMES		10
#define N_VID_BUFF		CONFIG_VIDEO_BUFFER_POOL_NUM_MAX

int main(void)
{
	struct video_buffer *buffers[N_VID_BUFF], *vbuf;
	struct video_format fmt = { 0 };
	struct video_caps caps;
	const struct device *video;
	unsigned int frame = 0;
	size_t bsize;
	int i = 0;
	int ret;

	video = DEVICE_DT_GET_ONE(alif_cam);
	if (!device_is_ready(video)) {
		LOG_ERR("%s: device not ready.", video->name);
		return -1;
	}
	printk("- Device name: %s\n", video->name);

	/* Get capabilities */
	if (video_get_caps(video, VIDEO_EP_OUT, &caps)) {
		LOG_ERR("Unable to retrieve video capabilities");
		return -1;
	}

	printk("- Capabilities:\n");
	while (caps.format_caps[i].pixelformat) {
		const struct video_format_cap *fcap = &caps.format_caps[i];
		/* fourcc to string */
		printk("  %c%c%c%c width (min, max, step)[%u; %u; %u] "
			"height (min, max, step)[%u; %u; %u]\n",
		       (char)fcap->pixelformat,
		       (char)(fcap->pixelformat >> 8),
		       (char)(fcap->pixelformat >> 16),
		       (char)(fcap->pixelformat >> 24),
		       fcap->width_min, fcap->width_max, fcap->width_step,
		       fcap->height_min, fcap->height_max, fcap->height_step);

		if (fcap->pixelformat == VIDEO_PIX_FMT_Y10P) {
			fmt.pixelformat = VIDEO_PIX_FMT_Y10P;
			fmt.width = fcap->width_min;
			fmt.height = fcap->height_min;
			fmt.pitch = fcap->width_min;
		}
		i++;
	}

	if (fmt.pixelformat == 0) {
		LOG_ERR("Desired Pixel format is not supported.");
		return -1;
	}

	ret = video_set_format(video, VIDEO_EP_OUT, &fmt);
	if (ret) {
		LOG_ERR("Failed to set video format. ret - %d", ret);
		return -1;
	}

	printk("- format: %c%c%c%c %ux%u\n", (char)fmt.pixelformat,
	       (char)(fmt.pixelformat >> 8),
	       (char)(fmt.pixelformat >> 16),
	       (char)(fmt.pixelformat >> 24),
	       fmt.width, fmt.height);

	/* Size to allocate for each buffer */
	bsize = fmt.pitch * fmt.height;

	printk("Width - %d, Pitch - %d, Height - %d, Buff size - %d\n",
			fmt.width, fmt.pitch, fmt.height, bsize);

	/* Alloc video buffers and enqueue for capture */
	for (i = 0; i < ARRAY_SIZE(buffers); i++) {
		buffers[i] = video_buffer_alloc(bsize);
		if (buffers[i] == NULL) {
			LOG_ERR("Unable to alloc video buffer");
			return -1;
		}

		/* Allocated Buffer Information */
		printk("- addr - 0x%x, size - %d, bytesused - %d\n",
			(uint32_t)buffers[i]->buffer,
			bsize,
			buffers[i]->bytesused);

		memset(buffers[i]->buffer, 0, sizeof(char) * bsize);
		video_enqueue(video, VIDEO_EP_OUT, buffers[i]);

		printk("capture buffer[%d]: dump binary memory "
			"\"/home/$USER/path/capture_%d.bin\" 0x%08x 0x%08x -r\n\n",
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

	/* Start video capture */
	ret = video_stream_start(video);
	if (ret) {
		LOG_ERR("Unable to start capture (interface). ret - %d", ret);
		return -1;
	}

	printk("Capture started\n");

	for (int i = 0; i < N_FRAMES; i++) {
		ret = video_dequeue(video, VIDEO_EP_OUT, &vbuf, K_FOREVER);
		if (ret) {
			LOG_ERR("Unable to dequeue video buf");
			return -1;
		}

		printk("\rGot frame %u! size: %u; timestamp %u ms\n",
		       frame++, vbuf->bytesused, vbuf->timestamp);

		if (i < N_FRAMES - N_VID_BUFF) {
			ret = video_enqueue(video, VIDEO_EP_OUT, vbuf);
			if (ret) {
				LOG_ERR("Unable to requeue video buf");
				return -1;
			}

			ret = video_stream_start(video);
			if (ret) {
				printk("Unable to start capture (interface). ret - %d\n",
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
