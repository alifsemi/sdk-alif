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

#include "DetectorPostProcessing.hpp"
#include "DetectorPreProcessing.hpp"
#include "YoloFastestModel.hpp"

#include "ScreenLayout.hpp"
#include "image_ensemble.h"

#include <cinttypes>

#include <lvgl.h>
#include "lv_paint_utils.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(UseCaseHandler);

// Do we get LVGL to zoom the camera image, or do we double it up?
#define USE_LVGL_ZOOM

#define MIMAGE_X 192
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
        //LOG_INF("time: %u", time);
        k_mutex_unlock(&lvgl_mutex);
		//k_msleep(time);       // TODO: FIX
        k_msleep(LV_DEF_REFR_PERIOD);
    }
}


extern "C" {
extern uint32_t tprof1, tprof2, tprof3, tprof4, tprof5;
}

namespace {
lv_style_t boxStyle;
//lv_color_t  lvgl_image[LIMAGE_Y][LIMAGE_X] __attribute__((section(".bss.lcd_image_buf")));                      // 192x192x3 = 110,592 bytes
lv_color_t  lvgl_image[LIMAGE_Y][LIMAGE_X] __attribute__((section(".alif_sram1.lcd_image_buf")));                 // 192x192x3 = 110,592 bytes
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
        LOG_INF("ScreenLayoutInit");
        ScreenLayoutInit(lvgl_image, sizeof lvgl_image, LIMAGE_X, LIMAGE_Y, LV_ZOOM);

        //uint32_t lv_lock_state = lv_port_lock();
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

    /* Object detection inference handler. */
    bool ObjectDetectionHandler(ApplicationContext& ctx)
    {
        //auto& profiler = ctx.Get<Profiler&>("profiler");

        // constexpr uint32_t dataPsnImgDownscaleFactor = 1;
        // constexpr uint32_t dataPsnImgStartX          = 10;
        // constexpr uint32_t dataPsnImgStartY          = 35;

        // constexpr uint32_t dataPsnTxtInfStartX = 20;
        // constexpr uint32_t dataPsnTxtInfStartY = 28;

        // hal_lcd_clear(COLOR_BLACK);

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

        const uint8_t *image_data = get_image_data(inputImgCols, inputImgRows);
        if(image_data == NULL) {
            return false;
        }
                
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

        //lv_led_on(ScreenLayoutLEDObject());
        k_mutex_unlock(&lvgl_mutex);
        const size_t copySz = inputTensor->bytes;

#if SHOW_INF_TIME
        uint32_t inf_prof = k_cycle_get_32();
#endif
        /* Run the pre-processing, inference and post-processing. */
        if (!preProcess.DoPreProcess(image_data, copySz)) {
            LOG_ERR("Pre-processing failed.");
            return false;
        }

        // /* Display image on the LCD. */
        // hal_lcd_display_image(
        //     (arm::app::object_detection::channelsImageDisplayed == 3) ? currImage : dstPtr,
        //     inputImgCols,
        //     inputImgRows,
        //     arm::app::object_detection::channelsImageDisplayed,
        //     dataPsnImgStartX,
        //     dataPsnImgStartY,
        //     dataPsnImgDownscaleFactor);

        // /* Display message on the LCD - inference running. */
        // hal_lcd_display_text(
        //     str_inf.c_str(), str_inf.size(), dataPsnTxtInfStartX, dataPsnTxtInfStartY, false);

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
        k_mutex_lock(&lvgl_mutex, K_FOREVER);
        // for (int r = 0; r < 3; r++) {
        //     lv_obj_t *label = ScreenLayoutLabelObject(r);
        //     lv_label_set_text_fmt(label, "%s (%d%%)", first_bit(results[r].m_label).c_str(), (int)(results[r].m_normalisedVal * 100));
        //     if (results[r].m_normalisedVal >= 0.7) {
        //         lv_obj_add_state(label, LV_STATE_USER_1);
        //     } else {
        //         lv_obj_clear_state(label, LV_STATE_USER_1);
        //     }
        //     if (results[r].m_normalisedVal < 0.2) {
        //         lv_obj_add_state(label, LV_STATE_USER_2);
        //     } else {
        //         lv_obj_clear_state(label, LV_STATE_USER_2);
        //     }
        // }
#if SHOW_INF_TIME
        lv_label_set_text_fmt(ScreenLayoutHeaderObject(), "%s - %.2f FPS", "Image Classifier", (double) SystemCoreClock / inf_prof);
        lv_label_set_text_fmt(ScreenLayoutTimeObject(), "%.3f ms", (double)inf_prof / SystemCoreClock * 1000);
#endif
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
        //hal_lcd_set_text_color(COLOR_GREEN);

        /* If profiling is enabled, and the time is valid. */
        LOG_INF("Final results:");
        LOG_INF("Total number of inferences: 1");

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

    static void DeleteBoxes(lv_obj_t *frame)
    {
        // Assume that child 0 of the frame is the image itself
        int children = lv_obj_get_child_cnt(frame);
        while (children > 1) {
            lv_obj_del(lv_obj_get_child(frame, 1));
            children--;
        }
    }

    static void CreateBox(lv_obj_t *frame, int x0, int y0, int w, int h)
    {
        lv_obj_t *box = lv_obj_create(frame);
        lv_obj_set_size(box, w, h);
        lv_obj_add_style(box, &boxStyle, LV_PART_MAIN);
        lv_obj_set_pos(box, x0, y0);
    }

    static void DrawDetectionBoxes(const std::vector<object_detection::DetectionResult>& results,
                                   int imgInputCols, int imgInputRows)
    {
        lv_obj_t *frame = ScreenLayoutImageHolderObject();
        float xScale = (float) lv_obj_get_content_width(frame) / imgInputCols;
        float yScale = (float) lv_obj_get_content_height(frame) / imgInputRows;

        DeleteBoxes(frame);

        for (const auto& result: results) {
            CreateBox(frame,
                      floor(result.m_x0 * xScale),
                      floor(result.m_y0 * yScale),
                      ceil(result.m_w * xScale),
                      ceil(result.m_h * yScale));
        }
    }

} /* namespace app */
} /* namespace alif */
