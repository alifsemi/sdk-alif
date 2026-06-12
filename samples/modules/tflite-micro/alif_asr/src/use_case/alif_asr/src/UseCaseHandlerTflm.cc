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
 * SPDX-FileCopyrightText: Copyright 2021-2022, 2024-2025 Arm Limited and/or its
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
#if defined(MLEK_FWK_TFLM)

#include "UseCaseHandler.hpp"

#include "AudioBackend.hpp"

#include "AsrClassifier.hpp"
#include "AsrResult.hpp"
#include "AudioSlidingWindow.hpp"
#include "OutputDecode.hpp"
#include "Wav2LetterModel.hpp"
#include "Wav2LetterPostprocess.hpp"
#include "Wav2LetterPreprocess.hpp"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/audio/codec.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/pm/device.h>

#include <stdint.h>
#include <unistd.h>
#include <cstdio>
#include <malloc.h>
#include <cstdlib>
#include <cstring>

LOG_MODULE_REGISTER(UseCaseHandlerTflm);

#define AUDIO_RATE               (16000)
#define AUDIO_CHUNK_SIZE_SAMPLES (8000) // 0.5s of audio at 16KHz
#define AUDIO_CHUNKS             (20)   // ~10s max
#define AUDIO_SAMPLES            (AUDIO_CHUNK_SIZE_SAMPLES * AUDIO_CHUNKS)

#define SAMPLE_SIZE    sizeof(int16_t)
#define WORD_SIZE      (SAMPLE_SIZE * 8)
#define AUDIO_CHANNELS CONFIG_AUDIO_CHANNELS

#if (IS_ENABLED(CONFIG_SOC_SERIES_E1C) || IS_ENABLED(CONFIG_SOC_SERIES_B1))
static int16_t audio_inf[AUDIO_SAMPLES + AUDIO_CHUNK_SIZE_SAMPLES];
#else
static int16_t audio_inf[AUDIO_SAMPLES + AUDIO_CHUNK_SIZE_SAMPLES]
	__attribute__((section("SRAM0.audio_inf")));
#if CONFIG_OUTPUT_TO_LINE_OUT
static char __aligned(8) my_slab_buffer[AUDIO_CHANNELS * WORD_SIZE * AUDIO_CHUNK_SIZE_SAMPLES]
	__attribute__((section("SRAM0.audio_slab")));
#endif
#endif

volatile bool button_pressed = false;

#if CONFIG_OUTPUT_TO_LINE_OUT

#define CODEC_CFG_NODE DT_ALIAS(audio_codec)
#define I2S_TX_NODE    DT_ALIAS(i2s_output)

static struct k_mem_slab mem_slab;

static bool configure_streams(const struct device *i2s_dev_tx)
{
	struct i2s_config config;

	config.word_size = WORD_SIZE;
	config.channels = AUDIO_CHANNELS;
	config.format = I2S_FMT_DATA_FORMAT_I2S;
	config.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
	config.frame_clk_freq = AUDIO_RATE;
	config.mem_slab = &mem_slab;
	config.block_size = AUDIO_CHUNK_SIZE_SAMPLES * AUDIO_CHANNELS * SAMPLE_SIZE;
	config.timeout = 1000;

	int ret = i2s_configure(i2s_dev_tx, I2S_DIR_TX, &config);
	if (ret < 0) {
		LOG_ERR("Failed to configure TX stream: %d\n", ret);
		return false;
	}

	audio_codec_stop_output(DEVICE_DT_GET(CODEC_CFG_NODE));

	struct audio_codec_cfg codec_cfg = {
		.dai_type = AUDIO_DAI_TYPE_I2S,
		.dai_cfg =
			{
				.i2s =
					{
						.word_size = AUDIO_PCM_WIDTH_16_BITS,
						.channels = 2,
						.format = I2S_FMT_DATA_FORMAT_I2S,
						.options = 0,
						.frame_clk_freq = AUDIO_RATE,
						.mem_slab = NULL,
						.block_size = 0,
						.timeout = 0,
					},
			},
	};
	ret = audio_codec_configure(DEVICE_DT_GET(CODEC_CFG_NODE), &codec_cfg);
	if (ret) {
		LOG_ERR("audio_codec_configure failed with error: %d", ret);
		return false;
	}

	return true;
}
#endif // CONFIG_OUTPUT_TO_LINE_OUT

namespace arm
{
namespace app
{

/**
 * @brief       Presents ASR inference results.
 * @param[in]   results   Vector of ASR classification results to be displayed.
 * @return      true if successful, false otherwise.
 **/
static bool PresentInferenceResult(std::vector<asr::AsrResult> &results);

/* ASR inference handler. */
bool ClassifyAudioHandler(ApplicationContext &ctx)
{
	auto &model = ctx.Get<fwk::iface::Model &>("model");
	auto mfccFrameLen = ctx.Get<uint32_t>("frameLength");
	auto mfccFrameStride = ctx.Get<uint32_t>("frameStride");
	auto scoreThreshold = ctx.Get<float>("scoreThreshold");
	auto inputCtxLen = ctx.Get<uint32_t>("ctxLen");

	if (!model.IsInited()) {
		LOG_ERR("Model is not initialised! Terminating processing.\n");
		return false;
	}

	const auto inputTensor = model.GetInputTensor(0);
	const auto outputTensor = model.GetOutputTensor(0);

	/* Get input shape. Dimensions of the tensor should have been verified by
	 * the callee. */
	auto inputShape = model.GetInputShape(0);

	const uint32_t inputRowsSize = inputShape[fwk::tflm::Wav2LetterModel::ms_inputRowsIdx];
	const uint32_t inputInnerLen = inputRowsSize - (2 * inputCtxLen);

	/* Audio data stride corresponds to inputInnerLen feature vectors. */
	const uint32_t audioDataWindowLen = (inputRowsSize - 1) * mfccFrameStride + (mfccFrameLen);
	const uint32_t audioDataWindowStride = inputInnerLen * mfccFrameStride;

	/* NOTE: This is only used for time stamp calculation. */
	const float secondsPerSample = (1.0 / audio::Wav2LetterMFCC::ms_defaultSamplingFreq);

	/* Set up pre and post-processing objects. */
	AsrPreProcess preProcess =
		AsrPreProcess(inputTensor, fwk::tflm::Wav2LetterModel::ms_numMfccFeatures,
			      inputShape[fwk::tflm::Wav2LetterModel::ms_inputRowsIdx], mfccFrameLen,
			      mfccFrameStride);

	std::vector<ClassificationResult> singleInfResult;
	AsrPostProcess postProcess =
		AsrPostProcess(model, ctx.Get<AsrClassifier &>("classifier"),
			       ctx.Get<std::vector<std::string> &>("labels"), singleInfResult,
			       inputCtxLen, fwk::tflm::Wav2LetterModel::ms_blankTokenIdx,
			       fwk::tflm::Wav2LetterModel::ms_outputRowsIdx);

#ifndef CONFIG_PUSH_TO_TALK
	// Application is not in push-to-talk mode. Start capturing audio once the button is pressed
	// for the first time, and keep capturing until we have the max number of samples or button
	// is released.
	button_pressed = false;
#endif

	while (!button_pressed) {
		k_sleep(K_MSEC(500));
		LOG_INF("Waiting for button press to start audio capture... (btn=%d)",
			(int)button_pressed);
	}

	LOG_INF("Button pressed. Init audio and start audio capture...\n");

	int err = audio_init(AUDIO_RATE);
	if (err) {
		LOG_ERR("hal_audio_init failed with error: %d", err);
		return false;
	}

	// Capture audio as long as button is kept pressed or until we have max samples
	int16_t *audio_inf_ptr = &audio_inf[0];
	int audio_idx = 0;

	get_audio_data(audio_inf_ptr, AUDIO_CHUNK_SIZE_SAMPLES);

	while (1) {
		err = wait_for_audio();
		if (err) {
			LOG_ERR("hal_get_audio_data failed with error: %d", err);
			audio_uninit();
			return false;
		}

		// Start next chunk
		if (audio_idx < (AUDIO_CHUNKS - 1)) {
			get_audio_data(audio_inf_ptr + ((audio_idx + 1) * AUDIO_CHUNK_SIZE_SAMPLES),
				       AUDIO_CHUNK_SIZE_SAMPLES);
		}

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
	LOG_INF("Audio capture finished.\n");

	int16_t *audioArr = audio_inf;
	uint32_t audioArrSize = (audio_idx + 1) * AUDIO_CHUNK_SIZE_SAMPLES;

#if CONFIG_OUTPUT_TO_LINE_OUT
	const struct device *const codec_dev = DEVICE_DT_GET(CODEC_CFG_NODE);
	const struct device *const i2s_dev_tx = DEVICE_DT_GET(I2S_TX_NODE);

	/* Each block is one stereo chunk: AUDIO_CHUNK_SIZE_SAMPLES * AUDIO_CHANNELS * SAMPLE_SIZE.
	 * With 2ch 16-bit: 8000 * 2 * 2 = 32000 bytes.
	 * num_blocks = sizeof(my_slab_buffer) / i2s_block_bytes
	 *            = (AUDIO_CHANNELS*WORD_SIZE*AUDIO_CHUNK_SIZE_SAMPLES) /
	 * (AUDIO_CHUNK_SIZE_SAMPLES*AUDIO_CHANNELS*SAMPLE_SIZE) = WORD_SIZE / SAMPLE_SIZE = 16/2
	 * = 8.
	 */
	const size_t i2s_block_bytes = AUDIO_CHUNK_SIZE_SAMPLES * AUDIO_CHANNELS * SAMPLE_SIZE;
	const uint32_t i2s_num_blocks = WORD_SIZE / SAMPLE_SIZE; /* = 8 */
#if (IS_ENABLED(CONFIG_SOC_SERIES_E1C) || IS_ENABLED(CONFIG_SOC_SERIES_B1))
	char *my_slab_buffer =
		(char *)malloc(AUDIO_CHANNELS * WORD_SIZE * AUDIO_CHUNK_SIZE_SAMPLES);
#endif
	k_mem_slab_init(&mem_slab, my_slab_buffer, i2s_block_bytes, i2s_num_blocks);

	if (!device_is_ready(i2s_dev_tx)) {
		LOG_ERR("%s is not ready", i2s_dev_tx->name);
		return 0;
	}

	if (!device_is_ready(codec_dev)) {
		LOG_ERR("%s is not ready", codec_dev->name);
		return 0;
	}
	if (!configure_streams(i2s_dev_tx)) {
		return 0;
	}

	audio_codec_start_output(codec_dev);

	int xx = 0;
	bool trigger_started = false;
	while (xx < audio_idx) {
		void *slab_block;
		/* Wait up to 2s for a block to be freed by the I2S ISR. */
		if (k_mem_slab_alloc(&mem_slab, &slab_block, K_MSEC(2000)) < 0) {
			LOG_ERR("Failed to allocate TX slab block");
			break;
		}

		// we have 16bit mono audio, but I2S is configured for stereo, so we need to
		// duplicate samples for left and right channels
		int16_t *stereo_buffer = (int16_t *)slab_block;
		for (size_t i = 0; i < AUDIO_CHUNK_SIZE_SAMPLES; i++) {
			stereo_buffer[2 * i] = audio_inf_ptr[xx * AUDIO_CHUNK_SIZE_SAMPLES + i];
			stereo_buffer[2 * i + 1] = audio_inf_ptr[xx * AUDIO_CHUNK_SIZE_SAMPLES + i];
		}

		err = i2s_write(i2s_dev_tx, slab_block, i2s_block_bytes);
		if (err < 0) {
			LOG_ERR("Failed to write data: %d", err);
			break;
		}
		xx++;

		/* Start the I2S output on the first chunk. */
		if (!trigger_started) {
			err = i2s_trigger(i2s_dev_tx, I2S_DIR_TX, I2S_TRIGGER_START);
			if (err < 0) {
				LOG_ERR("Failed to start I2S output: %d", err);
				return 0;
			}
			trigger_started = true;
		}
	}

	{
		uint32_t block_ms = (AUDIO_CHUNK_SIZE_SAMPLES * 1000U) / AUDIO_RATE;
		/* The feed loop above is paced by the i2s_num_blocks-deep slab
		 * (k_mem_slab_alloc blocks until the ISR frees a played block),
		 * so most of the audio has already played by the time we get
		 * here. At most i2s_num_blocks are still queued, so only wait
		 * for those remaining blocks rather than the whole stream. */
		uint32_t queued_blocks =
			((uint32_t)xx < i2s_num_blocks) ? (uint32_t)xx : i2s_num_blocks;
		k_sleep(K_MSEC(queued_blocks * block_ms + 200U));
	}

	/* Bring the TX stream back to the READY state. When the queued
	 * blocks finish playing the DW driver underruns and leaves the
	 * stream in I2S_STATE_ERROR (or RUNNING). i2s_configure() only
	 * accepts READY/NOT_READY, so without this the next button press
	 * would fail with "invalid state" (-EINVAL). I2S_TRIGGER_DROP is
	 * valid from any state except NOT_READY and resets it to READY. */
	if (trigger_started) {
		err = i2s_trigger(i2s_dev_tx, I2S_DIR_TX, I2S_TRIGGER_DROP);
		if (err < 0) {
			LOG_ERR("Failed to drop I2S output: %d", err);
		}
	}

#if (IS_ENABLED(CONFIG_SOC_SERIES_E1C) || IS_ENABLED(CONFIG_SOC_SERIES_B1))
	free(my_slab_buffer);
#endif
	audio_codec_stop_output(codec_dev);
	audio_codec_clear_errors(codec_dev);
#endif /* CONFIG_OUTPUT_TO_LINE_OUT */

	/* Audio clip needs enough samples to produce at least 1 MFCC feature. */
	if (audioArrSize < mfccFrameLen) {
		LOG_ERR("Not enough audio samples, minimum needed is %" PRIu32 "\n", mfccFrameLen);
		return false;
	}

	/* Creating a sliding window through the whole audio clip. */
	auto audioDataSlider = audio::FractionalSlidingWindow<const int16_t>(
		audioArr, audioArrSize, audioDataWindowLen, audioDataWindowStride);

	/* Declare a container for final results. */
	std::vector<asr::AsrResult> finalResults;
	size_t inferenceWindowLen = audioDataWindowLen;

	/* Start sliding through audio clip. */
	while (audioDataSlider.HasNext()) {

		/* If not enough audio, see how much can be sent for processing. */
		size_t nextStartIndex = audioDataSlider.NextWindowStartIndex();
		if (nextStartIndex + audioDataWindowLen > audioArrSize) {
			inferenceWindowLen = audioArrSize - nextStartIndex;
		}

		const int16_t *inferenceWindow = audioDataSlider.Next();

		LOG_INF("Inference %zu/%zu\n", audioDataSlider.Index() + 1,
			static_cast<size_t>(ceilf(audioDataSlider.FractionalTotalStrides() + 1)));

		/* Run the pre-processing, inference and post-processing. */
		if (!preProcess.DoPreProcess(inferenceWindow, inferenceWindowLen)) {
			LOG_ERR("Pre-processing failed.");
			return false;
		}

		if (!model.RunInference()) {
			LOG_ERR("Inference failed.");
			return false;
		}

		LOG_INF("NPU inference done.");

		/* Post processing needs to know if we are on the last audio window. */
		postProcess.m_lastIteration = !audioDataSlider.HasNext();
		if (!postProcess.DoPostProcess()) {
			LOG_ERR("Post-processing failed.");
			return false;
		}

		/* Add results from this window to our final results vector. */
		finalResults.emplace_back(asr::AsrResult(
			singleInfResult,
			(audioDataSlider.Index() * secondsPerSample * audioDataWindowStride),
			audioDataSlider.Index(), scoreThreshold));

	} /* while (audioDataSlider.HasNext()) */

	PresentInferenceResult(finalResults);
	audio_uninit();

	return true;
}

static bool PresentInferenceResult(std::vector<asr::AsrResult> &results)
{
	LOG_INF("Final results:\n");
	LOG_INF("Total number of inferences: %zu\n", results.size());

	/* Get each inference result string using the decoder. */
	std::string combinedResultStr;
	for (const auto &result : results) {
		std::string infResultStr = audio::asr::DecodeOutput(result.m_resultVec);

		LOG_INF("For timestamp: %f (inference #: %" PRIu32 "); label: %s\n",
			(double)result.m_timeStamp, result.m_inferenceNumber, infResultStr.c_str());
		combinedResultStr += infResultStr;
	}

	ClassificationResult res;
	res.m_label = combinedResultStr;
	std::vector<ClassificationResult> combinedResults;
	combinedResults.push_back(res);

	/* Get the decoded result for the combined result. */
	std::string finalResultStr = audio::asr::DecodeOutput(combinedResults);

	LOG_INF("Complete recognition: %s\n", finalResultStr.c_str());
	return true;
}

} /* namespace app */
} /* namespace arm */

#endif /* defined(MLEK_FWK_TFLM) */
