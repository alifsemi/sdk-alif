/*
 *   display.h
 */
#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <stdint.h>
#include <zephyr/kernel.h>

#define DISPLAY_NODE DT_CHOSEN(zephyr_display)

#define DISPLAY_WIDTH  DT_PROP(DISPLAY_NODE, width)
#define DISPLAY_HEIGHT DT_PROP(DISPLAY_NODE, height)

int display_init(void);
void display_set_next_frame_duration(uint32_t duration);
void display_next_frame(void);
void *display_active_buffer(void);
void *display_inactive_buffer(void);
uint32_t display_width(void);
uint32_t display_height(void);

#endif /* __DISPLAY_ILI9806_H */
