/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef KWSMODEL_H
#define KWSMODEL_H

#include <memory>
#include <vector>
#include <algorithm>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/micro/micro_mutable_op_resolver.h>

#include "mfcc/KwsProcessing.hpp"

class KWSModel
{
public:
	enum KeyWords {
		KW_DOWN = 0,
		KW_GO,
		KW_LEFT,
		KW_NO,
		KW_OFF,
		KW_ON,
		KW_RIGHT,
		KW_STOP,
		KW_UP,
		KW_YES,
		KW_SILENCE,
		KW_UNKNOWN,
		KW_COUNT,
	};

	class Result
	{
	public:
		void Clear(void)
		{
			confidences.clear();
		}

		void Add(const float confidence)
		{
			confidences.push_back(confidence);
		}

		size_t GetCount(void) const
		{
			return confidences.size();
		}

		auto &GetConfidences(void)
		{
			return confidences;
		}

		auto GetBiggestConfidence(void) const
		{
			return std::max_element(confidences.begin(), confidences.end());
		}

		size_t GetBiggestConfidenceIndex(void) const
		{
			return std::distance(confidences.begin(), GetBiggestConfidence());
		}

		float GetBiggestConfidenceValue(const size_t index) const
		{
			return static_cast<float>(confidences[index]);
		}
		const char *GetLabelName(size_t index) const;
	private:

		std::vector<float> confidences;
	};

	bool Init(void);
	size_t GetAudioDataWindowSize(void) const
	{
		return m_preProcess->GetAudioDataWindowSize();
	}
	bool PreProcess(void* buffer, size_t size);
	bool RunInference(void);
	const Result &PostProcess(void);

private:
	std::unique_ptr<tflite::MicroInterpreter> m_pInterpreter;
	std::unique_ptr<arm::app::KwsPreProcess> m_preProcess;
	tflite::MicroMutableOpResolver<1> m_resolver;
	int m_index = 0;
	Result m_output;
};

#endif
