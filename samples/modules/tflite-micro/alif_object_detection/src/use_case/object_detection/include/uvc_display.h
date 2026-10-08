/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https: //alifsemi.com/license
 *
 */

#ifndef UVC_DISPLAY_H
#define UVC_DISPLAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * USB Video Class "display" for headless builds: the board enumerates as a
 * webcam streaming YUY2 frames of a fixed size. Frames are produced at the
 * application's pace; when the host is not streaming, or the previous frame is
 * still being transferred, new frames are silently dropped so the caller never
 * blocks on USB.
 *
 * Frames come either from the functions below, or, when an "alif,uvc-display"
 * node is the zephyr,display, from the Zephyr display API (e.g. LVGL). In the
 * latter case the display driver calls uvc_display_init() itself.
 */

/* Advertise a single YUY2 width x height format and allocate the frame. */
int uvc_display_init(int width, int height);

/* Enable the USB device; the board then enumerates as a webcam. */
int uvc_display_start(void);

/*
 * Convert an RGB888 frame of the initialised size into the UVC frame buffer.
 * Returns 0 if the frame was taken, or a negative errno if it was dropped.
 */
int uvc_display_capture(const uint8_t *rgb888);

/*
 * Draw a rectangle outline of an RGB565 color into the captured frame.
 * Coordinates are in frame pixels and are clipped to the frame. No-op if no
 * frame is pending.
 */
void uvc_display_draw_rect(int x, int y, int w, int h, uint16_t color);

/* Send the pending captured frame to the host. No-op if no frame is pending. */
void uvc_display_submit(void);

#ifdef __cplusplus
}
#endif

#endif /* UVC_DISPLAY_H */
