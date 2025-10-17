/* This file was ported to work on Alif Semiconductor Ensemble family of devices. */

/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
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

#include "Classifier.hpp"
#include "MobileNetModel.hpp"
#include "ImageUtils.hpp"
#include "ScreenLayout.hpp"
#include "ImgClassProcessing.hpp"
#include "image_ensemble.h"

#include <cinttypes>
#include <cstring>
#include <cmath>

#include <lvgl.h>
#include "lv_paint_utils.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

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
lv_color_t  lvgl_image[LIMAGE_Y][LIMAGE_X] __attribute__((section("SRAM1.lcd_image_buf")));
};

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

        if (image_init() != 0) {
            return false;
        }

        return true;
    }

    /* Image classification inference handler. */
    bool ClassifyImageHandler(ApplicationContext& ctx)
    {
        auto& model = ctx.Get<Model&>("model");

        if (!model.IsInited()) {
            LOG_ERR("Model is not initialised! Terminating processing.");
            return false;
        }

        TfLiteTensor* inputTensor = model.GetInputTensor(0);
        TfLiteTensor* outputTensor = model.GetOutputTensor(0);
        if (!inputTensor->dims) {
            LOG_ERR("Invalid input tensor dims");
            return false;
        } else if (inputTensor->dims->size < 4) {
            LOG_ERR("Input tensor dimension should be = 4");
            return false;
        }

        /* Get input shape for displaying the image. */
        TfLiteIntArray* inputShape = model.GetInputShape(0);
        const uint32_t nCols       = inputShape->data[arm::app::MobileNetModel::ms_inputColsIdx];
        const uint32_t nRows       = inputShape->data[arm::app::MobileNetModel::ms_inputRowsIdx];

        /* Set up pre and post-processing. */
        ImgClassPreProcess preProcess = ImgClassPreProcess(inputTensor, model.IsDataSigned());

        std::vector<ClassificationResult> results;
        ImgClassPostProcess postProcess = ImgClassPostProcess(outputTensor,
                ctx.Get<ImgClassClassifier&>("classifier"), ctx.Get<const std::vector<std::string>&>("labels"),
                results);

        uint8_t* image_data = nullptr;
        if(get_image_data(nCols, nRows, &image_data) < 0) {
            LOG_ERR("Couldn't get image data");
            return false;
        }

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

        const size_t imgSz = inputTensor->bytes;

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
