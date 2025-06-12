/*
 *   display.h
 */
#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <stdint.h>

int display_init(void);
void display_next_frame(void);
void *display_active_buffer(void);
void *display_inactive_buffer(void);
uint32_t display_width(void);
uint32_t display_height(void);

#endif /* __DISPLAY_ILI9806_H */
