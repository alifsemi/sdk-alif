/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef SCREEN_LAYOUT_HPP
#define SCREEN_LAYOUT_HPP

#include "lvgl.h"

namespace alif {
namespace app {

void ScreenLayoutInit(const void *imgData, size_t imgSize, int imgWidth, int imgHeight, unsigned short imgZoom);

lv_obj_t *ScreenLayoutImageObject();
lv_obj_t *ScreenLayoutImageHolderObject();
lv_obj_t *ScreenLayoutHeaderObject();
lv_obj_t *ScreenLayoutLabelObject(int);
lv_obj_t *ScreenLayoutTimeObject();
lv_obj_t *ScreenLayoutLEDObject();

} /* namespace app */
} /* namespace alif */

#endif /* SCREEN_LAYOUT_HPP */
