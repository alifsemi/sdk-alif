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

#include "DetectorPostProcessing.hpp"
#include "DetectorPreProcessing.hpp"
#include "YoloFastestModel.hpp"

#include "ScreenLayout.hpp"
#include "image_ensemble.h"

#include <cinttypes>
#include <cstring>
#include <cmath>
#include <algorithm>

#include <lvgl.h>
#include "lv_paint_utils.h"

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
lv_color_t  lvgl_image[LIMAGE_Y][LIMAGE_X] __attribute__((section("SRAM1.lcd_image_buf")));                      // 192x192x3 = 110,592 bytes

/* Pre-allocated pool of detection box objects to avoid per-frame alloc/free */
static constexpr int MAX_DETECTION_BOXES = 10;
static lv_obj_t *boxPool[MAX_DETECTION_BOXES] = {};
static int boxPoolSize = 0;
};

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

    /**
     * @brief           Draw boxes directly on the LCD for all detected objects.
     * @param[in]       results            Vector of detection results to be displayed.
     **/
    static void DrawDetectionBoxes(
           const std::vector<object_detection::DetectionResult>& results,
           int imgInputCols, int imgInputRows);

    bool ObjectDetectionInit()
    {
        LOG_DBG("ScreenLayoutInit");

        /* zero image data */
        memset(&lvgl_image[0][0], 0, sizeof(lvgl_image));

        ScreenLayoutInit(lvgl_image, sizeof lvgl_image, LIMAGE_X, LIMAGE_Y, LV_ZOOM);

        lv_label_set_text_static(ScreenLayoutHeaderObject(), "Face Detection");
        lv_label_set_text_static(ScreenLayoutLabelObject(0), "Faces Detected: 0");
        lv_label_set_text_static(ScreenLayoutLabelObject(1), "192px image (24-bit)");

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

        if (image_init(LIMAGE_X, LIMAGE_Y) != 0) {
            return false;
        }

        return true;
    }

    /* Object detection inference handler. */
    bool ObjectDetectionHandler(ApplicationContext& ctx)
    {
        auto& model = ctx.Get<Model&>("model");
        if (!model.IsInited()) {
            LOG_ERR("Model is not initialised! Terminating processing.");
            return false;
        }

        TfLiteTensor* inputTensor = model.GetInputTensor(0);
        TfLiteTensor* outputTensor0 = model.GetOutputTensor(0);
        TfLiteTensor* outputTensor1 = model.GetOutputTensor(1);

        if (!inputTensor->dims) {
            LOG_ERR("Invalid input tensor dims");
            return false;
        } else if (inputTensor->dims->size < 3) {
            LOG_ERR("Input tensor dimension should be >= 3");
            return false;
        }

        /* Get input shape for displaying the image. */
        TfLiteIntArray* inputShape = model.GetInputShape(0);

        const int inputImgCols = inputShape->data[YoloFastestModel::ms_inputColsIdx];
        const int inputImgRows = inputShape->data[YoloFastestModel::ms_inputRowsIdx];

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

        const size_t copySz = inputTensor->bytes;

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

        k_mutex_lock(&lvgl_mutex, K_FOREVER);

        lv_label_set_text_fmt(ScreenLayoutLabelObject(0), "Faces Detected: %i", results.size());

        /* Draw boxes. */
        DrawDetectionBoxes(results, inputImgCols, inputImgRows);

        k_mutex_unlock(&lvgl_mutex);

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

} /* namespace app */
} /* namespace alif */
