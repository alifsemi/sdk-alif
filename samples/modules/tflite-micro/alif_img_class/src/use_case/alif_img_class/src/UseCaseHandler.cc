/* This file was ported to work on Alif Semiconductor Ensemble family of devices. */

/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 */

/*
 * Copyright (c) 2021-2022 Arm Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "UseCaseHandler.hpp"

#include "mlek/common/Classifier.hpp"
#include "mlek/fwk/tflm/MobileNetModel.hpp"
#include "mlek/common/ImageUtils.hpp"
#include "ScreenLayout.hpp"
#include "mlek/use_case/img_class/ImgClassProcessing.hpp"
#include "image_ensemble.h"

#include <cinttypes>
#include <cstring>
#include <cmath>

#include <lvgl.h>
#include "lv_paint_utils.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <zephyr/cache.h>

#if defined(CONFIG_APP_DISPLAY_DIRECT_MODE)
#include <zephyr/drivers/display/cdc200.h>
#include <zephyr/linker/section_tags.h>
#endif

LOG_MODULE_REGISTER(UseCaseHandler);

// Do we get LVGL to zoom the camera image, or do we double it up?
#define USE_LVGL_ZOOM

using ImgClassClassifier = arm::app::Classifier;

#define ASSERT_DIVISIBLE_BY_16(V) \
    static_assert(((V) % 16) == 0, \
        "Value must be divisible by 16")

#define MIMAGE_X 224
ASSERT_DIVISIBLE_BY_16(MIMAGE_X);
#define MIMAGE_Y 224

#ifdef USE_LVGL_ZOOM
#define LIMAGE_X        MIMAGE_X
#define LIMAGE_Y        MIMAGE_Y
#define LV_ZOOM         (2 * 256)
#else
#define LIMAGE_X        (MIMAGE_X * 2)
#define LIMAGE_Y        (MIMAGE_Y * 2)
#define LV_ZOOM         (1 * 256)
#endif

K_THREAD_STACK_DEFINE(lvgl_thread_stack, 1024 * 4);
static struct k_thread lvgl_thread;
static K_MUTEX_DEFINE(lvgl_mutex);

static void lvgl_worker_thread(void*, void*, void*)
{
    uint32_t time;
    while (1)
    {
        k_mutex_lock(&lvgl_mutex, K_FOREVER);
        time = lv_task_handler();
        k_mutex_unlock(&lvgl_mutex);
        if(time < 1) {
            time = 1;
        } else if (time > LV_DEF_REFR_PERIOD) {
            time = LV_DEF_REFR_PERIOD;
        }
        k_msleep(time);
    }
}

namespace {
lv_color_t  lvgl_image[LIMAGE_Y][LIMAGE_X] __attribute__((section("SRAM1.lcd_image_buf"), aligned(128)));

/* The OV5675 selfie module is mounted upside down. Reverse the rows of the
 * packed RGB888 frame so the preview and the model see the same orientation.
 */
#if defined(CONFIG_DT_HAS_OVTI_OV5675_ENABLED)
void rgb888_vflip(uint8_t *img, uint32_t width, uint32_t height)
{
    const uint32_t stride = width * 3;
    uint8_t row[MIMAGE_X * 3];

    if (width == 0 || height == 0 || stride > sizeof(row)) {
        LOG_ERR("Cannot flip %" PRIu32 "x%" PRIu32 " RGB frame", width, height);
        return;
    }

    uint8_t *top = img;
    uint8_t *bottom = img + (height - 1) * stride;

    while (top < bottom) {
        memcpy(row, top, stride);
        memcpy(top, bottom, stride);
        memcpy(bottom, row, stride);
        top += stride;
        bottom -= stride;
    }
}
#endif
};

/*
 * Partial mode copies this buffer into the CDC200 framebuffer. D/AVE 2D writes
 * it from the GPU, so drop stale CPU cache lines before that copy. Also used
 * when direct mode is selected but the CDC200 framebuffer cannot be adopted.
 */
extern "C" void lvgl_flush_cb_16bit(lv_display_t *display, const lv_area_t *area, uint8_t *px_map);

static void partial_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t h = (uint32_t)(area->y2 - area->y1 + 1);
    uint32_t stride = ROUND_UP(w * (LV_COLOR_DEPTH / 8), LV_DRAW_BUF_STRIDE_ALIGN);

    sys_cache_data_flush_and_invd_range(px_map, stride * h);
    lvgl_flush_cb_16bit(disp, area, px_map);
}

static void display_partial_mode_init(void)
{
    lv_display_t *disp = lv_display_get_default();

    if (disp == nullptr) {
        LOG_ERR("No LVGL display; cannot install the partial flush");
        return;
    }

    lv_display_set_flush_cb(disp, partial_flush_cb);
    LOG_INF("Display in PARTIAL mode");
}

#if defined(CONFIG_APP_DISPLAY_DIRECT_MODE)
#define DISPLAY_NODE DT_CHOSEN(zephyr_display)
#define DISPLAY_FB_SIZE                                                    \
    (DT_PROP(DISPLAY_NODE, width) * DT_PROP(DISPLAY_NODE, height) *        \
     (LV_COLOR_DEPTH / 8))

static uint8_t __alif_sram0_section __aligned(128) lvgl_back_fb[DISPLAY_FB_SIZE];

static const struct device *const display_dev = DEVICE_DT_GET(DISPLAY_NODE);
static uint8_t *direct_fb_front;

static void direct_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    ARG_UNUSED(area);

    if (lv_display_flush_is_last(disp)) {
        struct cdc200_fb_desc fb;
        int ret;

        fb.fb_addr = px_map;
        fb.fb_size = sizeof(lvgl_back_fb);

        sys_cache_data_flush_range(px_map, fb.fb_size);
        cdc200_swap_fb(display_dev, 0, &fb);

        ret = cdc200_wait_vsync(display_dev, K_MSEC(50));
        if (ret != 0) {
            /* A missed semaphore can still leave the new buffer on screen. */
            cdc200_get_framebuffer(display_dev, 0, &fb);
            if (fb.fb_addr != px_map) {
                ret = cdc200_wait_vsync(display_dev, K_MSEC(50));
                cdc200_get_framebuffer(display_dev, 0, &fb);
            }
        }
        if (ret != 0 && fb.fb_addr != px_map) {
            uint8_t *scanned = (px_map == lvgl_back_fb) ? direct_fb_front : lvgl_back_fb;

            LOG_ERR("CDC200 vblank timed out (%d)", ret);
            /* LVGL activates buf2 when this callback returns. Keep the
             * scanned buffer in buf1 so the next render stays off screen.
             */
            lv_display_set_buffers(disp, scanned, px_map, sizeof(lvgl_back_fb),
                                   LV_DISPLAY_RENDER_MODE_DIRECT);
        }
    }

    lv_display_flush_ready(disp);
}

static void display_direct_mode_init(void)
{
    lv_display_t *disp = lv_display_get_default();
    struct cdc200_fb_desc front;

    cdc200_get_framebuffer(display_dev, 0, &front);

    if (disp == nullptr || front.fb_size != sizeof(lvgl_back_fb)) {
        LOG_WRN("CDC200 framebuffer is %zu bytes, expected %zu (RGB565 layer 1?); "
                "staying in PARTIAL mode", front.fb_size, sizeof(lvgl_back_fb));
        display_partial_mode_init();
        return;
    }

    direct_fb_front = front.fb_addr;

    lv_display_set_buffers(disp, lvgl_back_fb, front.fb_addr, front.fb_size,
                           LV_DISPLAY_RENDER_MODE_DIRECT);
    lv_display_set_flush_cb(disp, direct_flush_cb);

    LOG_INF("Display in DIRECT mode, framebuffers %p / %p",
            (void *)lvgl_back_fb, (void *)front.fb_addr);
}
#endif /* CONFIG_APP_DISPLAY_DIRECT_MODE */

namespace alif {
namespace app {

    using namespace arm::app;

    bool PresentInferenceResult(const std::vector<arm::app::ClassificationResult>& results)
    {

        LOG_INF("Final results:");
        LOG_INF("Total number of inferences: 1");

        for (uint32_t i = 0; i < results.size(); ++i) {
            LOG_INF("%" PRIu32 ") %" PRIu32 " (%f) -> %s",
                i,
                results[i].m_labelIdx,
                results[i].m_normalisedVal,
                results[i].m_label.c_str());
        }

        return true;
    }

    static std::string first_bit(const std::string &s)
    {
        std::string::size_type comma = s.find_first_of(',');
        return s.substr(0, comma);
    }

    bool ClassifyImageInit()
    {
#if defined(CONFIG_APP_DISPLAY_DIRECT_MODE)
        display_direct_mode_init();
#else
        display_partial_mode_init();
#endif

        LOG_DBG("ScreenLayoutInit");

        /* zero image data */
        memset(&lvgl_image[0][0], 0, sizeof(lvgl_image));

        ScreenLayoutInit(lvgl_image, sizeof lvgl_image, LIMAGE_X, LIMAGE_Y, LV_ZOOM);

        lv_label_set_text_static(ScreenLayoutHeaderObject(), "Image Classifier");

        k_thread_create(&lvgl_thread, lvgl_thread_stack,
                        K_THREAD_STACK_SIZEOF(lvgl_thread_stack),
                        lvgl_worker_thread,
                        NULL, NULL, NULL,
                        0, 0, K_NO_WAIT);

	    k_thread_name_set(&lvgl_thread, "lvgl");

        if (image_init(MIMAGE_X, MIMAGE_Y) != 0) {
            return false;
        }

        return true;
    }

    /* Image classification inference handler. */
    bool ClassifyImageHandler(ApplicationContext& ctx)
    {
        auto& model = ctx.Get<fwk::iface::Model&>("model");

        if (!model.IsInited()) {
            LOG_ERR("Model is not initialised! Terminating processing.");
            return false;
        }

        auto inputTensor = model.GetInputTensor(0);
        auto outputTensor = model.GetOutputTensor(0);
        const auto inputShape = inputTensor->Shape();
        if (inputShape.empty()) {
            LOG_ERR("Invalid input tensor dims");
            return false;
        } else if (inputShape.size() < 4) {
            LOG_ERR("Input tensor dimension should be = 4");
            return false;
        }

        /* Get input shape for displaying the image. */
        const uint32_t nCols       = inputShape[arm::app::fwk::tflm::MobileNetModel::ms_inputColsIdx];
        const uint32_t nRows       = inputShape[arm::app::fwk::tflm::MobileNetModel::ms_inputRowsIdx];

        /* Set up pre and post-processing. */
        ImgClassPreProcess preProcess = ImgClassPreProcess(inputTensor);

        std::vector<ClassificationResult> results;
        ImgClassPostProcess postProcess = ImgClassPostProcess(outputTensor,
                ctx.Get<ImgClassClassifier&>("classifier"), ctx.Get<const std::vector<std::string>&>("labels"),
                results);

        uint8_t* image_data = nullptr;
        if(get_image_data(&image_data) < 0) {
            LOG_ERR("Couldn't get image data");
            return false;
        }

#if defined(CONFIG_DT_HAS_OVTI_OV5675_ENABLED)
        rgb888_vflip(image_data, MIMAGE_X, MIMAGE_Y);
#endif

        k_mutex_lock(&lvgl_mutex, K_FOREVER);

        #if LV_COLOR_DEPTH == 16
        write_rgb888_to_rgb565_buf(
                MIMAGE_X, MIMAGE_Y, image_data, (uint16_t*) &lvgl_image[0][0]);
        #elif LV_COLOR_DEPTH == 24
            memcpy(&lvgl_image[0][0], image_data, MIMAGE_X * MIMAGE_Y * 3);
                #else
        #error "Unsupported LVGL color depth"
        #endif

        lv_obj_invalidate(ScreenLayoutImageObject());

        k_mutex_unlock(&lvgl_mutex);

        const size_t imgSz = inputTensor->Bytes();

        /* Run the pre-processing, inference and post-processing. */
        if (!preProcess.DoPreProcess(image_data, imgSz)) {
            LOG_ERR("Pre-processing failed.");
            return false;
        }

        if (!model.RunInference()) {
            LOG_ERR("Inference failed.");
            return false;
        }

        if (!postProcess.DoPostProcess()) {
            LOG_ERR("Post-processing failed.");
            return false;
        }

        /* Add results to context for access outside handler. */
        ctx.Set<std::vector<ClassificationResult>>("results", results);

        k_mutex_lock(&lvgl_mutex, K_FOREVER);
        for (int r = 0; r < 3; r++) {
            lv_obj_t *label = ScreenLayoutLabelObject(r);
            lv_label_set_text_fmt(label, "%s (%d%%)", first_bit(results[r].m_label).c_str(), (int)(results[r].m_normalisedVal * 100));
            if (results[r].m_normalisedVal >= 0.7) {
                lv_obj_add_state(label, LV_STATE_USER_1);
            } else {
                lv_obj_clear_state(label, LV_STATE_USER_1);
            }
            if (results[r].m_normalisedVal < 0.2) {
                lv_obj_add_state(label, LV_STATE_USER_2);
            } else {
                lv_obj_clear_state(label, LV_STATE_USER_2);
            }
        }

        k_mutex_unlock(&lvgl_mutex);

        if (!PresentInferenceResult(results)) {
            return false;
        }

        return true;
    }

} /* namespace app */
} /* namespace alif */
