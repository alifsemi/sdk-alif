/* This file was ported to work on Alif Semiconductor devices. */

/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/*
 * SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its
 * affiliates <open-source-office@arm.com>
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

#include "AudioBackend.hpp"
#include "mlek/use_case/asr/ConformerProcessing.hpp"
#include "mlek/common/ImageUtils.hpp"
#include "mlek/fwk/executorch/EtModel.hpp"

#ifdef CONFIG_ENABLE_DISPLAY
#include "ScreenLayout.hpp"
#include "lv_paint_utils.h"
#include "lvgl.h"
#endif /* CONFIG_ENABLE_DISPLAY */

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/audio/codec.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/pm/device.h>

#include <stdint.h>
#include <unistd.h>
#include <cstdio>
#include <malloc.h>
#include <cstdlib>
#include <cstring>
#include <cmath>

LOG_MODULE_REGISTER(UseCaseHandlerEt);

#ifdef CONFIG_ENABLE_DISPLAY

#define LIMAGE_X (340)
#define LIMAGE_Y (80)
#define LV_ZOOM  (1.2 * 256)

namespace
{
lv_style_t boxStyle;
lv_color16_t lvgl_image[LIMAGE_Y][LIMAGE_X] __attribute__((section("SRAM1.lcd_image_buf")));
}; // namespace

static const int result_label_idx = 4;

K_THREAD_STACK_DEFINE(lvgl_thread_stack, 1024 * 4);
static struct k_thread lvgl_thread;
static K_MUTEX_DEFINE(lvgl_mutex);

static void lvgl_worker_thread(void *, void *, void *)
{
	uint32_t time;
	while (1) {
		k_mutex_lock(&lvgl_mutex, K_FOREVER);
		time = lv_task_handler();
		k_mutex_unlock(&lvgl_mutex);
		if (time < 1) {
			time = 1;
		} else if (time > LV_DEF_REFR_PERIOD) {
			time = LV_DEF_REFR_PERIOD;
		}
		k_msleep(time);
	}
}

#endif /* CONFIG_ENABLE_DISPLAY */

#define AUDIO_RATE               (16000)
#define AUDIO_CHUNK_SIZE_SAMPLES (2000) // Need to be multiple of 4 for stereo to mono conversion
#define AUDIO_CHUNKS             (56)   // ~7s max
#define AUDIO_SAMPLES            (AUDIO_CHUNK_SIZE_SAMPLES * AUDIO_CHUNKS)
static int16_t audio_inf[AUDIO_SAMPLES + AUDIO_CHUNK_SIZE_SAMPLES]
	__attribute__((section(".bss.NoInit.temp_buf_sram")));

static volatile bool button_pressed = false;

static const struct gpio_dt_spec sw5_button =
	GPIO_DT_SPEC_GET_BY_IDX_OR(DT_NODELABEL(wakeup_pins), lpgpios, 0, {0});
static struct gpio_callback button_cb_data;

static void button_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	int val = gpio_pin_get_dt(&sw5_button); // 1 = pressed, 0 = released
	if (val < 0) {
		LOG_ERR("Failed to read pin: %d", val);
		return;
	}

	button_pressed = (val == 1);
	LOG_INF("Button %s", button_pressed ? "pressed" : "released");
}

namespace arm
{
namespace app
{
bool ClassifyAudioInit()
{
	/* Configure the SW5 button and register the press callback. */
	if (!gpio_is_ready_dt(&sw5_button)) {
		LOG_ERR("SW5 button GPIO device is not ready");
		return false;
	}

	int ret = gpio_pin_configure_dt(&sw5_button, GPIO_INPUT);
	if (ret) {
		LOG_ERR("Failed to configure SW5 button as input: %d", ret);
		return false;
	}

	ret = gpio_pin_interrupt_configure_dt(&sw5_button, GPIO_INT_EDGE_BOTH);
	if (ret) {
		LOG_ERR("Failed to configure SW5 button interrupt: %d", ret);
		return false;
	}

	gpio_init_callback(&button_cb_data, button_cb, BIT(sw5_button.pin));
	ret = gpio_add_callback(sw5_button.port, &button_cb_data);
	if (ret) {
		LOG_ERR("Failed to add SW5 button callback: %d", ret);
		return false;
	}

#ifdef CONFIG_ENABLE_DISPLAY
	alif::app::ScreenLayoutInit(lvgl_image, sizeof(lvgl_image), LIMAGE_X, LIMAGE_Y, LV_ZOOM,
				    true);
	std::memset(lvgl_image, 0, sizeof(lvgl_image));

	lv_label_set_text_static(alif::app::ScreenLayoutHeaderObject(),
				 "Conformer ASR (ExecuTorch)");

	k_thread_create(&lvgl_thread, lvgl_thread_stack, K_THREAD_STACK_SIZEOF(lvgl_thread_stack),
			lvgl_worker_thread, NULL, NULL, NULL, 0, 0, K_NO_WAIT);

	k_thread_name_set(&lvgl_thread, "lvgl");

	k_mutex_lock(&lvgl_mutex, K_FOREVER);

	lv_style_init(&boxStyle);
	lv_style_set_bg_opa(&boxStyle, LV_OPA_TRANSP);
	lv_style_set_pad_all(&boxStyle, 0);
	lv_style_set_border_width(&boxStyle, 0);
	lv_style_set_outline_width(&boxStyle, 2);
	lv_style_set_outline_pad(&boxStyle, 0);
	lv_style_set_outline_color(
		&boxStyle, lv_theme_get_color_primary(alif::app::ScreenLayoutHeaderObject()));
	lv_style_set_radius(&boxStyle, 4);

	lv_obj_add_flag(alif::app::ScreenLayoutBarObject(), LV_OBJ_FLAG_HIDDEN);

	/* Square the progress bar's corners. The default theme gives the
	 * indicator pill-shaped (rounded) ends with a radius of half the bar
	 * height. At the start of a capture the value is tiny, so the indicator
	 * is narrower than that radius and the two rounded ends overlap, drawing
	 * a distorted blob until the value grows past the bar height. A zero
	 * radius makes the indicator render cleanly at every value. */
	lv_obj_set_style_radius(alif::app::ScreenLayoutBarObject(), 0, LV_PART_MAIN);
	lv_obj_set_style_radius(alif::app::ScreenLayoutBarObject(), 0, LV_PART_INDICATOR);

	lv_label_set_text_static(alif::app::ScreenLayoutLabelObject(0), "");
	lv_label_set_text_static(alif::app::ScreenLayoutLabelObject(result_label_idx), "");
	lv_obj_set_width(alif::app::ScreenLayoutLabelObject(result_label_idx), 460);
	lv_label_set_long_mode(alif::app::ScreenLayoutLabelObject(result_label_idx),
			       LV_LABEL_LONG_WRAP);

	k_mutex_unlock(&lvgl_mutex);
#endif /* CONFIG_ENABLE_DISPLAY */
	return true;
}

#ifdef CONFIG_ENABLE_DISPLAY
// from https://www.andrewnoske.com
void getHeatMapColor(float value, float *red, float *green, float *blue)
{
	const int NUM_COLORS = 4;
	static float color[NUM_COLORS][3] = {{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}};

	int idx1; // |-- Our desired color will be between these two indexes in "color".
	int idx2; // |
	float fractBetween = 0; // Fraction between "idx1" and "idx2" where our value is.

	if (value <= 0) {
		idx1 = idx2 = 0;
	} // accounts for an input <=0
	else if (value >= 1) {
		idx1 = idx2 = NUM_COLORS - 1;
	} // accounts for an input >=0
	else {
		value = value * (NUM_COLORS - 1);   // Will multiply value by 3.
		idx1 = floor(value);                // Our desired color will be after this index.
		idx2 = idx1 + 1;                    // ... and before this index (inclusive).
		fractBetween = value - float(idx1); // Distance between the two indexes (0-1).
	}

	*red = (color[idx2][0] - color[idx1][0]) * fractBetween + color[idx1][0];
	*green = (color[idx2][1] - color[idx1][1]) * fractBetween + color[idx1][1];
	*blue = (color[idx2][2] - color[idx1][2]) * fractBetween + color[idx1][2];
}

void drawMelSpec(fwk::iface::TensorIface &inputMelSpec, size_t numValidFrames)
{
	const int input_channels = inputMelSpec.Shape()[2];
	const size_t maxFrames = inputMelSpec.GetNumElements() / input_channels;
	float *mel_input = (float *)inputMelSpec.GetData();

	/* Only the frames generated from the captured audio hold valid data; the
	 * remainder of the input tensor is silence padding. Scale the spectrogram
	 * across the valid frames so the captured audio fills the image width
	 * instead of rendering the padding tail (which shows up as garbage). */
	if (numValidFrames == 0 || numValidFrames > maxFrames) {
		numValidFrames = maxFrames;
	}

	/* Hold the LVGL lock for the whole buffer fill (not just the invalidate).
	 * lvgl_image is the source bitmap for the (scaled) spectrogram image widget,
	 * which the GPU-backed renderer reads from the PendSV-driven lv_timer_handler.
	 * Writing it unlocked while a previous invalidate is being rendered lets the
	 * GPU read the buffer as it changes, corrupting the in-flight Dave2D dlist and
	 * stalling the GPU. Filling under the lock keeps the update atomic with respect
	 * to rendering, matching the object-detection use case. */
	k_mutex_lock(&lvgl_mutex, K_FOREVER);
	for (int xx = 0; xx < LIMAGE_X; xx++) {
		/* Map each image column onto a frame within the valid region so the
		 * spectrogram stretches to fill the width regardless of audio length. */
		const int frame = (LIMAGE_X > 1) ? static_cast<int>((static_cast<size_t>(xx) *
								     (numValidFrames - 1)) /
								    (LIMAGE_X - 1))
						 : 0;
		for (int yy = 0; yy < LIMAGE_Y; yy++) {
			float mel_value = (mel_input[(frame * input_channels) + yy] + 1.0f) / 2;

			float fr, fg, fb;
			getHeatMapColor(mel_value, &fr, &fg, &fb);

			lv_color16_t rgb = {static_cast<uint16_t>((uint16_t)(fb * 255) >> 3),
					    static_cast<uint16_t>((uint16_t)(fg * 255) >> 2),
					    static_cast<uint16_t>((uint16_t)(fr * 255) >> 3)};
			lvgl_image[yy][xx] = rgb;
		}
	}
	lv_obj_invalidate(alif::app::ScreenLayoutImageObject());
	k_mutex_unlock(&lvgl_mutex);
}
#endif /* CONFIG_ENABLE_DISPLAY */

bool ClassifyAudioHandler(ApplicationContext &ctx)
{
	auto &model = ctx.Get<fwk::iface::Model &>("model");
	auto &labels = ctx.Get<const std::vector<std::string> &>("labels");
	auto melSpecWindowSize = ctx.Get<uint32_t>("melSpecWindowSize");
	auto melSpecHopSize = ctx.Get<uint32_t>("melSpecHopSize");
	auto chunkSize = ctx.Get<uint32_t>("chunkSize");

	if (!model.IsInited()) {
		LOG_ERR("Model is not initialised! Terminating processing.\n");
		return false;
	}

	auto inputTensorMelSpec = model.GetInputTensor(0);
	auto inputTensorChunkSize = model.GetInputTensor(1);
	auto outputTensorLogits = model.GetOutputTensor(0);
	auto outputTensorChunkSize = model.GetOutputTensor(1);

	auto preProcess =
		ConformerPreProcess<int16_t>(inputTensorMelSpec, inputTensorChunkSize,
					     melSpecWindowSize, melSpecHopSize, chunkSize);

	std::string decodedResult;

	auto postProcess = ConformerPostProcess(outputTensorLogits, outputTensorChunkSize, labels,
						decodedResult);

	if (!ClassifyAudioInit()) {
		LOG_ERR("Failed to initialise button press handling.\n");
		return false;
	}

	/* Loop to process audio clips. */
	while (true) {
		/* (Re)start capture for this cycle. The mic is torn down again after
		 * each capture so it does not free-run and overrun the I2S RX
		 * stream during the inference that follows. */
		int err = audio_init(AUDIO_RATE);
		if (err) {
			LOG_ERR("audio_init failed with error: %d", err);
			return false;
		}

		// Wait for button press callback ...
		button_pressed = false;
		uint32_t idle_count = 0;
		LOG_INF("Waiting for button press to start audio capture...\n");
		while (!button_pressed) {
			// run audio in order to update the automatic gain
			get_audio_data(audio_inf, AUDIO_CHUNK_SIZE_SAMPLES);

#ifdef CONFIG_ENABLE_DISPLAY
			// Blink the UI 'LED'
			k_mutex_lock(&lvgl_mutex, K_FOREVER);
			if (idle_count & 0x08) {
				lv_led_on(alif::app::ScreenLayoutLEDObject());
			} else {
				lv_led_off(alif::app::ScreenLayoutLEDObject());
			}
			k_mutex_unlock(&lvgl_mutex);
#endif /* CONFIG_ENABLE_DISPLAY */
			err = wait_for_audio();
			if (err) {
				LOG_ERR("get_audio_data failed with error: %d", err);
				audio_uninit();
				return false;
			}
			audio_preprocessing(audio_inf, AUDIO_CHUNK_SIZE_SAMPLES);
			idle_count++;
		}

#ifdef CONFIG_ENABLE_DISPLAY
		k_mutex_lock(&lvgl_mutex, K_FOREVER);
		lv_label_set_text_static(alif::app::ScreenLayoutLabelObject(0), "");
		lv_obj_remove_flag(alif::app::ScreenLayoutBarObject(), LV_OBJ_FLAG_HIDDEN);
		lv_obj_invalidate(alif::app::ScreenLayoutBarObject());
		for (int ii = 0; ii <= result_label_idx; ii++) {
			lv_label_set_text_static(alif::app::ScreenLayoutLabelObject(ii), "");
		}
		k_mutex_unlock(&lvgl_mutex);
#endif /* CONFIG_ENABLE_DISPLAY */

		int16_t *audio_inf_ptr = &audio_inf[0];
		// Capture audio as long as button is kept pressed
		int audio_idx = 0;

		get_audio_data(audio_inf_ptr, AUDIO_CHUNK_SIZE_SAMPLES);

		while (1) {
			err = wait_for_audio();
			if (err) {
				LOG_ERR("get_audio_data failed with error: %d", err);
				audio_uninit();
				return false;
			}

			// Start next chunk
			if (audio_idx < (AUDIO_CHUNKS - 1)) {
				get_audio_data(audio_inf_ptr +
						       ((audio_idx + 1) * AUDIO_CHUNK_SIZE_SAMPLES),
					       AUDIO_CHUNK_SIZE_SAMPLES);
			}

#ifdef CONFIG_ENABLE_DISPLAY
			k_mutex_lock(&lvgl_mutex, K_FOREVER);
			lv_bar_set_value(alif::app::ScreenLayoutBarObject(),
					 100 * (audio_idx + 1) / AUDIO_CHUNKS, LV_ANIM_OFF);
			lv_obj_invalidate(alif::app::ScreenLayoutBarObject());
			k_mutex_unlock(&lvgl_mutex);
#endif /* CONFIG_ENABLE_DISPLAY */

			// Preprocess current chunk
			audio_preprocessing(audio_inf_ptr + (audio_idx * AUDIO_CHUNK_SIZE_SAMPLES),
					    AUDIO_CHUNK_SIZE_SAMPLES);

			if (!button_pressed) {
				break;
			}

			if (audio_idx >= (AUDIO_CHUNKS - 1)) {
				break;
			}
			audio_idx++;
		}

		int16_t *audioArr = audio_inf;
		uint32_t audioArrSize =
			AUDIO_CHUNK_SIZE_SAMPLES + (audio_idx + 1) * AUDIO_CHUNK_SIZE_SAMPLES;

		/* Stop the mic before running inference so it does not free-run and
		 * overrun while the NPU is busy; it is re-initialised next cycle. */
		audio_uninit();

#ifdef CONFIG_ENABLE_DISPLAY
		k_mutex_lock(&lvgl_mutex, K_FOREVER);
		lv_label_set_text_static(alif::app::ScreenLayoutLabelObject(result_label_idx), "");
		lv_obj_add_flag(alif::app::ScreenLayoutBarObject(), LV_OBJ_FLAG_HIDDEN);
		lv_led_on(alif::app::ScreenLayoutLEDObject());
		lv_bar_set_value(alif::app::ScreenLayoutBarObject(), 0, LV_ANIM_OFF);
		lv_obj_invalidate(alif::app::ScreenLayoutLabelObject(result_label_idx));
		k_mutex_unlock(&lvgl_mutex);
#endif /* CONFIG_ENABLE_DISPLAY */
		/* Clamp the captured audio to what the mel-spectrogram input tensor
		 * can hold. A long button press can capture more audio than the
		 * model's fixed-size input tensor; passing it unclamped would overflow
		 * the tensor during pre-processing and crash. */
		const size_t melBins = inputTensorMelSpec->Shape()[2];
		const size_t maxMelFrames = inputTensorMelSpec->GetNumElements() / melBins;
		const uint32_t maxAudioSamples =
			melSpecWindowSize + (maxMelFrames - 1) * melSpecHopSize;
		if (audioArrSize > maxAudioSamples) {
			LOG_WRN("Captured audio exceeds model capacity; truncating to %u samples\n",
				static_cast<unsigned>(maxAudioSamples));
			audioArrSize = maxAudioSamples;
		}

		/* Run the pre-processing, inference and post-processing. */
#ifdef CONFIG_ENABLE_DISPLAY
		const uint32_t ts_start_pre = k_cycle_get_32();
#endif /* CONFIG_ENABLE_DISPLAY */
		if (!preProcess.DoPreProcess(audioArr, audioArrSize)) {
			LOG_ERR("Pre-processing failed.");
			return false;
		}

#ifdef CONFIG_ENABLE_DISPLAY
		const uint32_t ts_done_pre = k_cycle_get_32();

		/* Number of mel-spectrogram frames produced from the captured audio,
		 * matching the sliding-window count used during pre-processing. */
		const size_t numValidMelFrames =
			(audioArrSize >= melSpecWindowSize)
				? (1 + (audioArrSize - melSpecWindowSize) / melSpecHopSize)
				: 0;
		drawMelSpec(*inputTensorMelSpec, numValidMelFrames);

		const uint32_t ts_start_inference = k_cycle_get_32();
#endif /* CONFIG_ENABLE_DISPLAY */

		if (!model.RunInference()) {
			LOG_ERR("Inference failed.");
			return false;
		}
#ifdef CONFIG_ENABLE_DISPLAY
		const uint32_t ts_start_post = k_cycle_get_32();
#endif /* CONFIG_ENABLE_DISPLAY */
		if (!postProcess.DoPostProcess()) {
			LOG_ERR("Post-processing failed.");
			return false;
		}

#ifdef CONFIG_ENABLE_DISPLAY
		k_mutex_lock(&lvgl_mutex, K_FOREVER);
		const uint32_t ts_done = k_cycle_get_32();
		lv_led_off(alif::app::ScreenLayoutLEDObject());

		/* Format floats via Zephyr's snprintk (uses CONFIG_CBPRINTF_FP_SUPPORT)
		 * instead of lv_label_set_text_fmt: LVGL's built-in printf drops %f
		 * unless LV_USE_FLOAT is set, and that path miscompiles on Cortex-M55. */
		char label_buf[64];

		snprintk(label_buf, sizeof(label_buf), "Input duration: %.1fs",
			 (double)(audioArrSize) / AUDIO_RATE);
		lv_label_set_text(alif::app::ScreenLayoutLabelObject(0), label_buf);

		snprintk(label_buf, sizeof(label_buf), "Inference time: %.2f ms",
			 (double)(ts_start_post - ts_start_inference) /
				 sys_clock_hw_cycles_per_sec() * 1000);
		lv_label_set_text(alif::app::ScreenLayoutLabelObject(1), label_buf);

		snprintk(label_buf, sizeof(label_buf), "Pre: %.2fms Post: %.2fms",
			 (double)(ts_done_pre - ts_start_pre) / sys_clock_hw_cycles_per_sec() *
				 1000,
			 (double)(ts_done - ts_start_post) / sys_clock_hw_cycles_per_sec() * 1000);
		lv_label_set_text(alif::app::ScreenLayoutLabelObject(2), label_buf);

		lv_label_set_text_static(alif::app::ScreenLayoutLabelObject(3), "Output:");
		lv_label_set_text(alif::app::ScreenLayoutLabelObject(result_label_idx),
				  decodedResult.c_str());
		lv_obj_invalidate(alif::app::ScreenLayoutLabelObject(result_label_idx));
		k_mutex_unlock(&lvgl_mutex);
#endif /* CONFIG_ENABLE_DISPLAY */

		LOG_INF("Decoded output: %s\n", decodedResult.c_str());
	}

	return true;
}

} /* namespace app */
} /* namespace arm */
