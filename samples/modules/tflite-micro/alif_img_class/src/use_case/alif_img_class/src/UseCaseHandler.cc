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

#include <lvgl.h>
#include "lv_paint_utils.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(UseCaseHandler);

// Do we get LVGL to zoom the camera image, or do we double it up?
#define USE_LVGL_ZOOM

using ImgClassClassifier = arm::app::Classifier;

#define MIMAGE_X 224
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
        //LOG_INF("time: %u", time);
        k_mutex_unlock(&lvgl_mutex);
		//k_msleep(time);       // TODO: FIX
        k_msleep(LV_DISP_DEF_REFR_PERIOD);
    }
}


extern "C" {
extern uint32_t tprof1, tprof2, tprof3, tprof4, tprof5;
}

namespace {

lv_color_t  lvgl_image[LIMAGE_Y][LIMAGE_X] __attribute__((section(".bss.lcd_image_buf")));                      // 448x448x4 = 802,856
};

namespace alif {
namespace app {

    using namespace arm::app;

    bool PresentInferenceResult(const std::vector<arm::app::ClassificationResult>& results)
    {
        constexpr uint32_t dataPsnTxtStartX1 = 150;
        constexpr uint32_t dataPsnTxtStartY1 = 30;

        constexpr uint32_t dataPsnTxtStartX2 = 10;
        constexpr uint32_t dataPsnTxtStartY2 = 150;

        constexpr uint32_t dataPsnTxtYIncr = 16; /* Row index increment. */

        //hal_lcd_set_text_color(COLOR_GREEN);

        /* Display each result. */
        uint32_t rowIdx1 = dataPsnTxtStartY1 + 2 * dataPsnTxtYIncr;
        uint32_t rowIdx2 = dataPsnTxtStartY2;

        LOG_INF("Final results:");
        LOG_INF("Total number of inferences: 1");

        for (uint32_t i = 0; i < results.size(); ++i) {
            std::string resultStr = std::to_string(i + 1) + ") " +
                                    std::to_string(results[i].m_labelIdx) + " (" +
                                    std::to_string(results[i].m_normalisedVal) + ")";
            /*
            hal_lcd_display_text(
                resultStr.c_str(), resultStr.size(), dataPsnTxtStartX1, rowIdx1, false);
            rowIdx1 += dataPsnTxtYIncr;
            */

            resultStr = std::to_string(i + 1) + ") " + results[i].m_label;
            //hal_lcd_display_text(resultStr.c_str(), resultStr.size(), dataPsnTxtStartX2, rowIdx2, 0);
            rowIdx2 += dataPsnTxtYIncr;

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
        ScreenLayoutInit(lvgl_image, sizeof lvgl_image, LIMAGE_X, LIMAGE_Y, LV_ZOOM);
        //uint32_t lv_lock_state = lv_port_lock();
        lv_label_set_text_static(ScreenLayoutHeaderObject(), "Image Classifier");
        //lv_port_unlock(lv_lock_state);

        k_thread_create(&lvgl_thread, lvgl_thread_stack,
                        K_THREAD_STACK_SIZEOF(lvgl_thread_stack),
                        lvgl_worker_thread,
                        NULL, NULL, NULL,
                        0, 0, K_NO_WAIT);       // TODO: priority define

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
                ctx.Get<ImgClassClassifier&>("classifier"), ctx.Get<std::vector<std::string>&>("labels"),
                results);

        const uint8_t *image_data = get_image_data(nCols, nRows);
        if(image_data == NULL) {
            return false;
        }

        /*
        uint8_t *image_data = (uint8_t *)0x08000000;
        if (!image_data) {
            LOG_ERR("hal_get_image_data failed");
            return false;
        }

        static int frame = 0;
        for (int i = 0; i < (nCols * nRows); ++i) {
            image_data[i * 3] = frame % 0xFF;   // R
            image_data[(i * 3) + 1] = 0x00;   // G
            image_data[(i * 3) + 2] = 0x00;   // B
        }

        frame++;
        */


        k_mutex_lock(&lvgl_mutex, K_FOREVER);
        //tprof5 = k_cycle_get_32();
        /* Display this image on the LCD. */
#ifdef USE_LVGL_ZOOM
        write_to_lvgl_buf(
#else
        write_to_lvgl_buf_doubled(
#endif
                MIMAGE_X, MIMAGE_Y, image_data, &lvgl_image[0][0]);
        //tprof5 = k_cycle_get_32() - tprof5;

        lv_obj_invalidate(ScreenLayoutImageObject());
/*
        if (!run_requested()) {
#if SHOW_PROFILING
            lv_label_set_text_fmt(ScreenLayoutLabelObject(0), "tprof1=%.3f ms", (double)tprof1 / sys_clock_hw_cycles_per_sec() * 1000);
            lv_label_set_text_fmt(ScreenLayoutLabelObject(1), "tprof2=%.3f ms", (double)tprof2 / sys_clock_hw_cycles_per_sec() * 1000);
            lv_label_set_text_fmt(ScreenLayoutLabelObject(2), "tprof3=%.3f ms", (double)tprof3 / sys_clock_hw_cycles_per_sec() * 1000);
            lv_label_set_text_fmt(ScreenLayoutLabelObject(3), "tprof4=%.3f ms", (double)tprof4 / sys_clock_hw_cycles_per_sec() * 1000);
            lv_label_set_text_fmt(ScreenLayoutLabelObject(4), "tprof5=%.3f ms", (double)tprof5 / sys_clock_hw_cycles_per_sec() * 1000);
#endif
#if SHOW_EXPOSURE
            lv_label_set_text_fmt(ScreenLayoutLabelObject(1), "low=%" PRIu32, exposure_low_count);
            lv_label_set_text_fmt(ScreenLayoutLabelObject(2), "high=%" PRIu32, exposure_high_count);
            lv_label_set_text_fmt(ScreenLayoutLabelObject(3), "gain=%.3f", get_image_gain());
#endif
            lv_led_off(ScreenLayoutLEDObject());
            lv_port_unlock(lv_lock_state);
            return true;
        }
        */

        lv_led_on(ScreenLayoutLEDObject());
        k_mutex_unlock(&lvgl_mutex);

        const size_t imgSz = inputTensor->bytes;

#if SHOW_INF_TIME
        uint32_t inf_prof = k_cycle_get_32();
#endif

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

#if SHOW_INF_TIME
        inf_prof = k_cycle_get_32() - inf_prof;
#endif

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

#if SHOW_INF_TIME
        lv_label_set_text_fmt(ScreenLayoutHeaderObject(), "%s - %.2f FPS", "Image Classifier", (double) SystemCoreClock / inf_prof);
        lv_label_set_text_fmt(ScreenLayoutTimeObject(), "%.3f ms", (double)inf_prof / SystemCoreClock * 1000);
#endif
        k_mutex_unlock(&lvgl_mutex);

        if (!PresentInferenceResult(results)) {
            return false;
        }

        return true;
    }

} /* namespace app */
} /* namespace alif */
