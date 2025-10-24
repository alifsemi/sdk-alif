/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
#include "RTE_Components.h"
#include "RTE_Device.h"

#include "log_macros.h"
#include "image_data.h"
#include "image_processing.h"
#include "bayer.h"
#include "tiff.h"
#include "camera.h"
#include "board.h"
#include "base_def.h"
#include "timer_ensemble.h"
#include "delay.h"
*/


#include "image_ensemble.h"
#include "image_processing.h"
#include "bayer.h"

#include <tgmath.h>
#include <string.h>
#include <inttypes.h>

#include <zephyr/drivers/video.h>
#include <zephyr/drivers/video-controls.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(image_ensemble);



/* Camera fills the raw_image buffer.
 * Bayer->RGB conversion transfers into the rgb_image buffer.
 * With MT9M114 camera this can be a RGB565 to RGB conversion.
 * Following steps (crop, interpolate, colour correct) all occur in the rgb_image buffer in-place.
 */
static struct {
	//tiff_header_t tiff_header;
	uint8_t image_data[CIMAGE_RGB_WIDTH_MAX * CIMAGE_RGB_HEIGHT_MAX * RGB_BYTES];
} rgb_image __attribute__((section("SRAM0.camera_frame_bayer_to_rgb_buf")));

/*
static uint8_t raw_image[CIMAGE_X * CIMAGE_Y + CIMAGE_USE_RGB565 * CIMAGE_X * CIMAGE_Y]
    __attribute__((aligned(32),section("SRAM1.camera_frame_buf")));
*/

#define BAYER_FORMAT DC1394_COLOR_FILTER_GRBG

static const struct device *video_dev;
struct video_buffer *buffer, *vbuf;

int image_init()
{
    //struct video_buffer *buffers[N_VID_BUFF], *vbuf;
	struct video_format fmt = { 0 };
	struct video_caps caps;
	//const struct device *video;
	unsigned int frame = 0;
	size_t bsize;
	int i = 0;
	int ret;

    video_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_camera));
	if (!device_is_ready(video_dev)) {
		LOG_ERR("%s: device not ready.", video_dev->name);
		return -1;
	}
	printk("- Device name: %s\n", video_dev->name);

	/* Get capabilities */
	if (video_get_caps(video_dev, VIDEO_EP_OUT, &caps)) {
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

	ret = video_set_format(video_dev, VIDEO_EP_OUT, &fmt);
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

    buffer = video_buffer_alloc(bsize);
    if (buffer == NULL) {
        LOG_ERR("Unable to alloc video buffer");
        return -1;
    }

    /* Allocated Buffer Information */
    printk("- addr - 0x%x, size - %d, bytesused - %d\n",
        (uint32_t)buffer->buffer,
        bsize,
        buffer->bytesused);

    memset(buffer->buffer, 0, sizeof(char) * bsize);
    video_enqueue(video_dev, VIDEO_EP_OUT, buffer);

    ret = video_stream_start(video_dev);
	if (ret) {
		LOG_ERR("Unable to start capture (interface). ret - %d", ret);
		return -1;
	}

    /*
    ret = video_set_ctrl(video_dev, VIDEO_CID_CAMERA_GAIN, (void *)-1);
    if (ret) {
        LOG_ERR("Unable set camera gain. ret - %d", ret);
		return -1;
    }
    */

    //k_msleep(7000);

	printk("Capture started\n");
    
    return 0;
}


static float current_log_gain = 0.0;
static int32_t current_api_gain = 1;
static int32_t last_requested_api_gain = 1;
static float minimum_log_gain = -INFINITY;
static float maximum_log_gain = +INFINITY;

float get_image_gain(void)
{
    return current_api_gain * 0x1p-16f;
}

static float api_gain_to_log(int32_t api)
{
    return logf(api * 0x1p-16f);
}

static int32_t log_gain_to_api(float gain)
{
    return expf(gain) * 0x1p16f;
}

#if CIMAGE_SW_GAIN_CONTROL
static void process_autogain(void)
{
    /* Simple "auto-exposure" algorithm. We work a single "gain" value
     * and leave it up to the camera driver how this is produced through
     * adjusting exposure time, analogue gain or digital gain.
     *
     * We us a discrete velocity form of a PI controller to adjust the
     * current gain to try to make the difference between high pixels
     * and low pixels hit a target.
     *
     * The definition of "low" and "high" pixels has quite an effect on the
     * end result - this is set over in bayer2rgb.c, which does the analysis.
     * It gives us 4 counts for high/low (>80% / <20%) pixels and over/under-
     * exposed (255 or 0).
     *
     * We reduce to 2 counts by using high|low + (weight * over|under),
     * effectively counting the over/under-exposed pixels weight more times.
     */
    const float target_highlow_difference = 0.0f;
    const float overunder_weight = 4.0f;

    // Control constants - output is to the logarithm of gain (as if
    // we're working in decibels), so 1 here would mean a factor of e
    // adjustment if every pixel was high.
    const float Kp = 1.0f * 0.45f;
    const float Ki = 1.0f * 0.54f / 2.0f;
    const float tiny_error = 0x1p-4f;

    static float previous_error = 0;

    if (current_api_gain <= 0) {
        if (current_api_gain < 0) {
            return;
        }
        //current_api_gain = camera_gain(0); // Read initial gain
        if (current_api_gain < 0) {
            LOG_ERR("camera_gain(0) returned error %" PRId32 "; disabling autogain", current_api_gain);
            return;
        }
        current_log_gain = api_gain_to_log(current_api_gain);
    }

    /* Rescale high-low difference in pixel counts so that it's
     * in range [-1..+1], regardless of image size */
    float high_proportion = (exposure_high_count + overunder_weight * exposure_over_count) * (1.0f / (CIMAGE_X * CIMAGE_Y));
    float low_proportion = (exposure_low_count + overunder_weight * exposure_under_count) * (1.0f / (CIMAGE_X * CIMAGE_Y));
    float highlow_difference = high_proportion - low_proportion;
    float error = highlow_difference - target_highlow_difference;

    /* Ignore small errors, so we don't oscillate */
    if (fabsf(error) < tiny_error * fmaxf(high_proportion, low_proportion)) {
        error = 0;
    }

    float delta_controller_output = (Kp + Ki) * error - Kp * previous_error;

    previous_error = error;
    current_log_gain = current_log_gain - delta_controller_output;

    /* Clamp according to limits we've found from the gain API */
    current_log_gain = fminf(current_log_gain, maximum_log_gain);
    current_log_gain = fmaxf(current_log_gain, minimum_log_gain);

    int32_t desired_api_gain = log_gain_to_api(current_log_gain);
    if (desired_api_gain <= 0) {
        desired_api_gain = 1;
    }

    /* Apply the gain, if it's a new request */
    if (desired_api_gain != last_requested_api_gain) {
        //int32_t ret = camera_gain(desired_api_gain);
        int ret = video_set_ctrl(video_dev, VIDEO_CID_CAMERA_GAIN, (void *)desired_api_gain);
        if (ret < 0) {
            LOG_ERR("Camera gain error %" PRId32 "", ret);
            return;
        }
        last_requested_api_gain = desired_api_gain;
        current_api_gain = desired_api_gain;
        LOG_DBG("Camera gain changed to %.3f", current_api_gain * 0x1p-16f);

        /* Check for saturation, and record it. Knowing our limits avoids
         * making pointless calls to the gain changing API.
         */
        float deviation = ((float) current_api_gain - desired_api_gain) / desired_api_gain;
        if (fabsf(deviation) > 0.25f) {
            if (deviation < 0) {
                maximum_log_gain = api_gain_to_log(current_api_gain);
                LOG_DBG("Noted maximum gain %.3f", current_api_gain * 0x1p-16f);
            } else {
                minimum_log_gain = api_gain_to_log(current_api_gain);
                LOG_DBG("Noted minimum gain %.3f\n", current_api_gain * 0x1p-16f);
            }
            current_log_gain = api_gain_to_log(current_api_gain);
        }
    }
}
#endif

const uint8_t *get_image_data(int ml_width, int ml_height)
{
    extern uint32_t tprof1, tprof2, tprof3, tprof4, tprof5;
#ifndef USE_FAKE_CAMERA
    int ret;

    ret = video_dequeue(video_dev, VIDEO_EP_OUT, &vbuf, K_FOREVER);
    if (ret) {
        LOG_ERR("Unable to dequeue video buf");
        return NULL;
    }

    uint8_t* raw_image = vbuf->buffer;

    SCB_CleanInvalidateDCache();



    //camera_start(CAMERA_MODE_SNAPSHOT);
    //camera_wait(100);
    // It's a big buffer (313,600 bytes) - actually doing it by address can take 0.175ms, while
    // a global clean+invalidate is 0.023ms. (Although there will be a reload cost
    // on stuff we lost).
    // Notably, just invalidate is faster at 0.015ms, but we'd have to be sure
    // there were no writeback cacheable areas.
    // From that Breakeven point for ranged invalidate time = global clean+invalidate would be 43Kbyte.
    // So maybe go to global if >128K, considering cost of refills?
    //SCB_InvalidateDCache_by_Addr(raw_image, sizeof raw_image);
    //SCB_CleanInvalidateDCache();
#else
    static int roll = 0;
    for (int y = 0; y < CIMAGE_Y; y+=2) {
    	uint8_t *p = raw_image + y * CIMAGE_X;
    	int bar = (7 * ((y+roll) % CIMAGE_Y)) / CIMAGE_Y + 1;
    	float barb = bar & 1 ? 255 : 0;
    	float barr = bar & 2 ? 255 : 0;
    	float barg = bar & 4 ? 255 : 0;
    	for (int x = 0; x < CIMAGE_X; x+=2) {
    		float intensity = x * (1.0f/(CIMAGE_X-2));
    		float r = barr * intensity + 0.5f;
    		float g = barg * intensity + 0.5f;
    		float b = barb * intensity + 0.5f;
            if (BAYER_FORMAT == DC1394_COLOR_FILTER_BGGR) {
                p[0]        = b; p[1]            = g;
                p[CIMAGE_X] = g; p[CIMAGE_X + 1] = r;
            } else if (BAYER_FORMAT == DC1394_COLOR_FILTER_GRBG) {
                p[0]        = g; p[1]            = r;
                p[CIMAGE_X] = b; p[CIMAGE_X + 1] = g;
            }
    		p += 2;
    	}
    }
    roll = (roll + 1) % CIMAGE_Y;
#endif

#if !CIMAGE_USE_RGB565
    /* TIFF image can be dumped in Arm Development Studio using the command
     *
     *     dump value camera.tiff rgb_image
     *
     * while stopped at an appropriate breakpoint below.
     */
    //write_tiff_header(&rgb_image.tiff_header, CIMAGE_X, CIMAGE_Y);
    //tprof1 = Get_SysTick_Cycle_Count32();
    // RGB conversion and frame resize
    dc1394_bayer_Simple(raw_image, rgb_image.image_data, CIMAGE_X, CIMAGE_Y, BAYER_FORMAT);
    //tprof1 = Get_SysTick_Cycle_Count32() - tprof1;
#endif

#ifndef USE_FAKE_CAMERA
#if CIMAGE_SW_GAIN_CONTROL
    // Use pixel analysis from bayer_to_RGB to adjust gain
    process_autogain();
#endif
#endif

    // Cropping and scaling
#if CIMAGE_USE_RGB565
    if ((size_t) ml_width * ml_height * RGB_BYTES > sizeof rgb_image.image_data) {
        LOG_ERR("Requested image does not fit to RGB buffer.");
        return NULL;
    }
    crop_and_interpolate(raw_image, CIMAGE_X, CIMAGE_Y,
                         rgb_image.image_data, ml_width, ml_height,
                         RGB565_BYTES * 8);
#else
    if (ml_width > CIMAGE_X || ml_height > CIMAGE_Y) {
        LOG_ERR("Requested image can't be processed in place");
        return NULL;
    }
    crop_and_interpolate(rgb_image.image_data, CIMAGE_X, CIMAGE_Y,
                         rgb_image.image_data, ml_width, ml_height, RGB_BYTES * 8);
#endif
    // Rewrite the TIFF header for the new size
    //write_tiff_header(&rgb_image.tiff_header, ml_width, ml_height);

#if CIMAGE_COLOR_CORRECTION
    //tprof4 = Get_SysTick_Cycle_Count32();
    // Color correction for white balance
    white_balance(ml_width, ml_height, rgb_image.image_data, rgb_image.image_data);
    //tprof4 = Get_SysTick_Cycle_Count32() - tprof4;
#endif


    ret = video_enqueue(video_dev, VIDEO_EP_OUT, vbuf);
    if (ret) {
        LOG_ERR("Unable to requeue video buf");
        return NULL;
    }

    ret = video_stream_start(video_dev);
    if (ret) {
        printk("Unable to start capture (interface). ret - %d\n",
                ret);
        return NULL;
    }


    return rgb_image.image_data;
}
