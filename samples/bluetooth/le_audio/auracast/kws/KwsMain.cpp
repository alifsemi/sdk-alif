/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <limits>
#include <vector>
#include <algorithm>

#include "KwsMain.h"
#include "KwsModel.h"
#include "LiveMicInput.h"
#include "mic_source.h"

LOG_MODULE_REGISTER(inference, CONFIG_MAIN_LOG_LEVEL);

#define INFERENCE_THREAD_STACK_SIZE 2048
#define INFERENCE_THREAD_PRIORITY   10

static 	struct k_thread inference_thread;
static K_THREAD_STACK_DEFINE(inference_stack, INFERENCE_THREAD_STACK_SIZE);

static void process_inference_output(const KWSModel::Result &result)
{
	const size_t keyword_idx = result.GetBiggestConfidenceIndex();
	const float confidence = result.GetBiggestConfidenceValue(keyword_idx);

	/* Handle only 70% or higher confidences */
	if (confidence < 0.7f) {
		return;
	}

	if (keyword_idx == KWSModel::KW_GO) {
		mic_start();
	} else if (keyword_idx == KWSModel::KW_STOP) {
		mic_stop();
	} else if (keyword_idx == KWSModel::KW_UP) {
		mic_adjust_gain(true);
	} else if (keyword_idx == KWSModel::KW_DOWN) {
		mic_adjust_gain(false);
	} else {
		return;
	}

	/* Just a debugging purposes */
	/*
	LOG_INF("%s: %f", result.GetLabelName(keyword_idx),
		static_cast<double>(confidence));
	*/
}

static void InferenceThreadEntry(void *ctx, void *, void *)
{
	static KWSModel model;
	size_t data_size;

	if (!model.Init()) {
		return;
	}

	/* Window size is valid only after the model tensors have been allocated. */
	static LiveMicInput input(model.GetAudioDataWindowSize());

	if (!input.Start()) {
		return;
	}

	while (true) {
		if (!model.PreProcess(input.GetInputData(data_size), data_size)) {
			break;
		}

		if (!model.RunInference()) {
			break;
		}

		process_inference_output(model.PostProcess());
	}

	input.Stop();
}

int kws_initialize(void)
{
	const k_tid_t tid =
		k_thread_create(&inference_thread, inference_stack,
				K_THREAD_STACK_SIZEOF(inference_stack),	InferenceThreadEntry,
				NULL, NULL, NULL, INFERENCE_THREAD_PRIORITY, 0, K_NO_WAIT);

	if (!tid) {
		LOG_ERR("Failed to create inference thread");
		return -ENOEXEC;
	}

	return 0;
}
