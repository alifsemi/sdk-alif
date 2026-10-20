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

#include "mlek/use_case/object_detection/DetectorPostProcessing.hpp"
#include "mlek/use_case/object_detection/DetectorPreProcessing.hpp"
#include "mlek/fwk/tflm/YoloFastestModel.hpp"

#include "image_ensemble.h"

#include <cinttypes>
#include <cstring>
#include <cmath>
#include <algorithm>

#if defined(CONFIG_LVGL)
#include "ScreenLayout.hpp"
#include <lvgl.h>
#include "lv_paint_utils.h"
#include <zephyr/sys/util.h>
#include <zephyr/cache.h>
#if defined(CONFIG_APP_DISPLAY_DIRECT_MODE)
#include <zephyr/drivers/display/cdc200.h>
#include <zephyr/linker/section_tags.h>
#endif
#endif

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(UseCaseHandler);

// Do we get LVGL to zoom the camera image, or do we double it up?
#define USE_LVGL_ZOOM

#define ASSERT_DIVISIBLE_BY_16(V) \
    static_assert(((V) % 16) == 0, \
        "Value must be divisible by 16")

#define MIMAGE_X 192
ASSERT_DIVISIBLE_BY_16(MIMAGE_X);
#define MIMAGE_Y 192

#ifdef USE_LVGL_ZOOM
#define LIMAGE_X        MIMAGE_X
#define LIMAGE_Y        MIMAGE_Y
#define LV_ZOOM         (2 * 256)
#else
#define LIMAGE_X        (MIMAGE_X * 2)
#define LIMAGE_Y        (MIMAGE_Y * 2)
#define LV_ZOOM         (1 * 256)
#endif

/* The OV5675 selfie module is mounted upside down. Reverse the rows of the
 * packed RGB888 frame so the preview and the model see the same orientation.
 */
#if defined(CONFIG_DT_HAS_OVTI_OV5675_ENABLED)
namespace {
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
} /* namespace */
#endif

#if defined(CONFIG_LVGL)
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
lv_style_t boxStyle;
lv_color_t  lvgl_image[LIMAGE_Y][LIMAGE_X] __attribute__((section("SRAM1.lcd_image_buf"), aligned(128)));

/* Pre-allocated pool of detection box objects to avoid per-frame alloc/free */
static constexpr int MAX_DETECTION_BOXES = 10;
static lv_obj_t *boxPool[MAX_DETECTION_BOXES] = {};
static int boxPoolSize = 0;
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
#endif /* CONFIG_LVGL */

namespace alif {
namespace app {

    using namespace arm::app;

    /**
     * @brief           Presents inference results along using the data presentation
     *                  object.
     * @param[in]       results            Vector of detection results to be displayed.
     * @return          true if successful, false otherwise.
     **/
    static bool PresentInferenceResult(const std::vector<object_detection::DetectionResult>& results);

#if defined(CONFIG_LVGL)
    /**
     * @brief           Draw boxes directly on the LCD for all detected objects.
     * @param[in]       results            Vector of detection results to be displayed.
     **/
    static void DrawDetectionBoxes(
           const std::vector<object_detection::DetectionResult>& results,
           int imgInputCols, int imgInputRows);
#endif /* CONFIG_LVGL */

    bool ObjectDetectionInit()
    {
#if defined(CONFIG_LVGL)
#if defined(CONFIG_APP_DISPLAY_DIRECT_MODE)
        display_direct_mode_init();
#else
        display_partial_mode_init();
#endif

        LOG_DBG("ScreenLayoutInit");

        /* zero image data */
        memset(&lvgl_image[0][0], 0, sizeof(lvgl_image));

        ScreenLayoutInit(lvgl_image, sizeof lvgl_image, LIMAGE_X, LIMAGE_Y, LV_ZOOM);

        lv_label_set_text_static(ScreenLayoutHeaderObject(), "Face Detection");
        lv_label_set_text_static(ScreenLayoutLabelObject(0), "Faces Detected: 0");
        lv_label_set_text_static(ScreenLayoutLabelObject(1), "192px image (16-bit)");

        lv_style_init(&boxStyle);
        lv_style_set_bg_opa(&boxStyle, LV_OPA_TRANSP);
        lv_style_set_pad_all(&boxStyle, 0);
        lv_style_set_border_width(&boxStyle, 0);
        lv_style_set_outline_width(&boxStyle, 2);
        lv_style_set_outline_pad(&boxStyle, 0);
        lv_style_set_outline_color(&boxStyle, lv_theme_get_color_primary(ScreenLayoutHeaderObject()));
        lv_style_set_radius(&boxStyle, 4);

        k_thread_create(&lvgl_thread, lvgl_thread_stack,
                        K_THREAD_STACK_SIZEOF(lvgl_thread_stack),
                        lvgl_worker_thread,
                        NULL, NULL, NULL,
                        0, 0, K_NO_WAIT);       // TODO: priority define

	    k_thread_name_set(&lvgl_thread, "lvgl");
#endif /* CONFIG_LVGL */

        if (image_init(LIMAGE_X, LIMAGE_Y) != 0) {
            return false;
        }

        return true;
    }

    /* Object detection inference handler. */
    bool ObjectDetectionHandler(ApplicationContext& ctx)
    {
        auto& model = ctx.Get<fwk::iface::Model&>("model");
        if (!model.IsInited()) {
            LOG_ERR("Model is not initialised! Terminating processing.");
            return false;
        }

        auto inputTensor = model.GetInputTensor(0);
        auto outputTensor0 = model.GetOutputTensor(0);
        auto outputTensor1 = model.GetOutputTensor(1);

        const auto inputShape = inputTensor->Shape();
        if (inputShape.empty()) {
            LOG_ERR("Invalid input tensor dims");
            return false;
        } else if (inputShape.size() < 3) {
            LOG_ERR("Input tensor dimension should be >= 3");
            return false;
        }

        /* Get input shape for displaying the image. */
        const int inputImgCols = inputShape[fwk::tflm::YoloFastestModel::ms_inputColsIdx];
        const int inputImgRows = inputShape[fwk::tflm::YoloFastestModel::ms_inputRowsIdx];

        /* Set up pre and post-processing. */
        DetectorPreProcess preProcess = DetectorPreProcess(inputTensor, true, model.IsDataSigned());

        std::vector<object_detection::DetectionResult> results;
        const object_detection::PostProcessParams postProcessParams{
            inputImgRows,
            inputImgCols,
            object_detection::originalImageSize,
            object_detection::anchor1,
            object_detection::anchor2};
        DetectorPostProcess postProcess =
            DetectorPostProcess(outputTensor0, outputTensor1, results, postProcessParams);

        uint8_t* imageDataPtr = nullptr;
        if(get_image_data(&imageDataPtr) < 0) {
            LOG_ERR("Couldn't get image data");
            return false;
        }

#if defined(CONFIG_DT_HAS_OVTI_OV5675_ENABLED)
        rgb888_vflip(imageDataPtr, MIMAGE_X, MIMAGE_Y);
#endif

#if defined(CONFIG_LVGL)
        k_mutex_lock(&lvgl_mutex, K_FOREVER);

        #if LV_COLOR_DEPTH == 16
        write_rgb888_to_rgb565_buf(
                MIMAGE_X, MIMAGE_Y, imageDataPtr, (uint16_t*) &lvgl_image[0][0]);
        #elif LV_COLOR_DEPTH == 24
            memcpy(&lvgl_image[0][0], imageDataPtr, MIMAGE_X * MIMAGE_Y * 3);
        #else
        #error "Unsupported LVGL color depth"
        #endif

        lv_obj_invalidate(ScreenLayoutImageObject());

        k_mutex_unlock(&lvgl_mutex);
#endif /* CONFIG_LVGL */

        const size_t copySz = inputTensor->Bytes();

        /* Run the pre-processing, inference and post-processing. */
        if (!preProcess.DoPreProcess(imageDataPtr, copySz)) {
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

#if defined(CONFIG_LVGL)
        k_mutex_lock(&lvgl_mutex, K_FOREVER);

        lv_label_set_text_fmt(ScreenLayoutLabelObject(0), "Faces Detected: %i", results.size());

        /* Draw boxes. */
        DrawDetectionBoxes(results, inputImgCols, inputImgRows);

        k_mutex_unlock(&lvgl_mutex);
#endif /* CONFIG_LVGL */

        if (!PresentInferenceResult(results)) {
            return false;
        }

        return true;
    }

    bool
    PresentInferenceResult(const std::vector<object_detection::DetectionResult>& results)
    {
        /* If profiling is enabled, and the time is valid. */
        LOG_DBG("Final results:");
        LOG_DBG("Total number of inferences: 1");

        for (uint32_t i = 0; i < results.size(); ++i) {
            LOG_INF("%" PRIu32 ") (%f) -> %s {x=%d,y=%d,w=%d,h=%d}",
                 i,
                 results[i].m_normalisedVal,
                 "Detection box:",
                 results[i].m_x0,
                 results[i].m_y0,
                 results[i].m_w,
                 results[i].m_h);
        }

        return true;
    }

#if defined(CONFIG_LVGL)
    static void InitBoxPool(lv_obj_t *frame)
    {
        for (int i = 0; i < MAX_DETECTION_BOXES; i++) {
            boxPool[i] = lv_obj_create(frame);
            lv_obj_add_style(boxPool[i], &boxStyle, LV_PART_MAIN);
            lv_obj_add_flag(boxPool[i], LV_OBJ_FLAG_HIDDEN);
        }
        boxPoolSize = MAX_DETECTION_BOXES;
    }

    static void DrawDetectionBoxes(const std::vector<object_detection::DetectionResult>& results,
                                   int imgInputCols, int imgInputRows)
    {
        lv_obj_t *frame = ScreenLayoutImageHolderObject();
        float xScale = (float) lv_obj_get_content_width(frame) / imgInputCols;
        float yScale = (float) lv_obj_get_content_height(frame) / imgInputRows;

        /* Lazily create the pool on first use */
        if (boxPoolSize == 0) {
            InitBoxPool(frame);
        }

        int numResults = std::min((int)results.size(), MAX_DETECTION_BOXES);

        /* Update active boxes */
        for (int i = 0; i < numResults; i++) {
            lv_obj_set_size(boxPool[i],
                            (int)ceil(results[i].m_w * xScale),
                            (int)ceil(results[i].m_h * yScale));
            lv_obj_set_pos(boxPool[i],
                           (int)floor(results[i].m_x0 * xScale),
                           (int)floor(results[i].m_y0 * yScale));
            lv_obj_clear_flag(boxPool[i], LV_OBJ_FLAG_HIDDEN);
        }

        /* Hide unused boxes */
        for (int i = numResults; i < MAX_DETECTION_BOXES; i++) {
            lv_obj_add_flag(boxPool[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
#endif /* CONFIG_LVGL */

} /* namespace app */
} /* namespace alif */
