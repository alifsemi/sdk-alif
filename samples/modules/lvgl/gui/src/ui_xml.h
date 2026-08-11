/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#ifndef UI_XML_H
#define UI_XML_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Build the UI described by ui/screen_main.xml on the given parent.
 *
 * This is the generated C equivalent of the declarative XML: the Alif LVGL
 * fork has no runtime XML loader (lv_xml_*), so scripts/gen_ui_xml.py compiles
 * the XML offline into this function. Every widget here maps 1:1 to a tag in
 * screen_main.xml.
 */
void ui_xml_create(lv_obj_t *parent);

#ifdef __cplusplus
}
#endif

#endif /* UI_XML_H */
