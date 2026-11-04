/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/logging/log.h>

#include "KwsModel.h"
#include "mfcc/PlatformMath.hpp"
#include "mfcc/MicroNetKwsModel.hpp"
#include "BufAttributes.hpp"

LOG_MODULE_REGISTER(KWSModel);

// By having these as global state prevents us from having multiple instances of KWSModel
// these can be move into the class, but then these must be allocated dynamically
static uint8_t tensorArena[CONFIG_ACTIVATION_BUF_SZ] ACTIVATION_BUF_ATTRIBUTE;

static const char *labelsVec[KWSModel::KW_COUNT] LABELS_ATTRIBUTE = {
	[KWSModel::KW_DOWN] = "down",
	[KWSModel::KW_GO] = "go",
	[KWSModel::KW_LEFT] = "left",
	[KWSModel::KW_NO] = "no",
	[KWSModel::KW_OFF] = "off",
	[KWSModel::KW_ON] = "on",
	[KWSModel::KW_RIGHT] = "right",
	[KWSModel::KW_STOP] = "stop",
	[KWSModel::KW_UP] = "up",
	[KWSModel::KW_YES] = "yes",
	[KWSModel::KW_SILENCE] = "_silence_",
	[KWSModel::KW_UNKNOWN] = "_unknown_",
};

namespace arm::app::kws
{
extern uint8_t *GetModelPointer();
} // namespace arm::app::kws

const char *KWSModel::Result::GetLabelName(const size_t index) const
{
	return labelsVec[index];
}

bool KWSModel::Init()
{
	m_resolver.AddEthosU();

	m_pInterpreter = std::make_unique<tflite::MicroInterpreter>(
		tflite::GetModel(arm::app::kws::GetModelPointer()), m_resolver, tensorArena,
		CONFIG_ACTIVATION_BUF_SZ);

	const auto rc = m_pInterpreter->AllocateTensors();
	if (rc != kTfLiteOk) {
		LOG_ERR("AllocateTensors failed: %i", rc);
		return false;
	}

	auto *inputTensor = m_pInterpreter->input(0);
	size_t numMfccFeatures =
		inputTensor->dims->data[arm::app::MicroNetKwsModel::ms_inputColsIdx];
	size_t numMfccFrames = inputTensor->dims->data[arm::app::MicroNetKwsModel::ms_inputRowsIdx];
	int mfccFrameLength = arm::app::kws::g_FrameLength;
	int mfccFrameStride = arm::app::kws::g_FrameStride;

	m_preProcess = std::make_unique<arm::app::KwsPreProcess>(
		inputTensor, numMfccFeatures, numMfccFrames, mfccFrameLength, mfccFrameStride);

	return true;
}

bool KWSModel::PreProcess(void* buffer, const size_t size)
{
	if (!buffer || !size) {
		LOG_ERR("Invalid buffer or size");
		return false;
	}

	if (!m_preProcess->DoPreProcess(buffer, m_index)) {
		LOG_ERR("DoPreProcess failed");
		return false;
	}

	++m_index;

	// move buffer down by one stride, clearing space at the end for the next stride
	//std::copy(audio_inf + (CONFIG_I2S_SAMPLE_RATE / 2),
	//	    audio_inf + CONFIG_I2S_SAMPLE_RATE,
	//	    audio_inf);

	return true;
}

bool KWSModel::RunInference()
{
	const auto rc = m_pInterpreter->Invoke();
	if (rc != kTfLiteOk) {
		LOG_ERR("Invoke failed: %i", rc);
		return false;
	}

	return true;
}

const KWSModel::Result &KWSModel::PostProcess()
{
	m_output.Clear();

	const auto *tensor = m_pInterpreter->output(0);

	// int8 dequantize, does not work if per-axis quantization is used.
	for (size_t i = 0; i < tensor->bytes; i++) {
		float value = tensor->params.scale * (static_cast<float>(tensor->data.int8[i]) -
						      tensor->params.zero_point);

		m_output.Add(value);
	}

	arm::app::math::MathUtils::SoftmaxF32(m_output.GetConfidences());

	return m_output;
}
