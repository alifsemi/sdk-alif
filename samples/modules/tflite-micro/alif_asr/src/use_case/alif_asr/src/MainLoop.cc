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
 * SPDX-FileCopyrightText: Copyright 2021, 2024-2025 Arm Limited and/or its
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
#include "Labels.hpp"               /* For label strings. */
#include "UseCaseHandler.hpp"       /* Handlers for different user options. */
#include "BufAttributes.hpp"        /* Buffer attributes to be applied */

/** Based on ML framework, set up the model namespace. */
#if defined(MLEK_FWK_TFLM)
#include "Wav2LetterModel.hpp"    /* Model class for running inference. */
#include "AsrClassifier.hpp"  /* Classifier. */

using AsrModel = arm::app::fwk::tflm::Wav2LetterModel;
#elif defined(MLEK_FWK_EXECUTORCH)
#include "ConformerModel.hpp"

using AsrModel = arm::app::fwk::et::ConformerModel;
#endif /** MLEK_FWK_TFLM or MLEK_FWK_EXECUTORCH */

#include "power_mgr.h"             /* power management APIs */
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(MainLoop);

#define LPGPIO_NODE           DT_NODELABEL(lpgpio)
#define LPGPIO_WAKEUP_ENABLED DT_NODE_HAS_STATUS_OKAY(LPGPIO_NODE)

#if LPGPIO_WAKEUP_ENABLED
/* LPGPIO wake up source is used */
#define LPGPIO_EWIC_CFG EWIC_VBAT_GPIO

#if CONFIG_LPGPIO_WAKEUP_SOURCE == 4
#define LPGPIO_WAKEUP_EVENT WE_LPGPIO4
#elif CONFIG_LPGPIO_WAKEUP_SOURCE == 1
#define LPGPIO_WAKEUP_EVENT WE_LPGPIO1
#else
#define LPGPIO_WAKEUP_EVENT WE_LPGPIO0
#endif
#else
#define LPGPIO_EWIC_CFG 0
#define LPGPIO_WAKEUP_EVENT 0
#endif

namespace arm {
namespace app {
    static uint8_t activationBuf[ACTIVATION_BUF_SZ] ACTIVATION_BUF_ATTRIBUTE;

    namespace asr {
        extern uint8_t* GetModelPointer();
        extern size_t GetModelLen();
    } /* namespace asr */

} /* namespace app */
} /* namespace arm */


#if LPGPIO_WAKEUP_ENABLED
static const struct gpio_dt_spec lpgpio_config = GPIO_DT_SPEC_GET_BY_IDX_OR(
	DT_NODELABEL(wakeup_pins), lpgpios, CONFIG_LPGPIO_WAKEUP_SOURCE, {0});
#endif


extern volatile uint8_t button_pressed;
static struct gpio_callback button_cb_data;
static bool por_boot;
static struct k_sem button_wait_sem;

static void button_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	if (por_boot) {
		/* Ignore button press event if it's from POR boot, as the button state may be unstable during POR. */
		return;
	}

	int val = gpio_pin_get_dt(&lpgpio_config);  // 1 = pressed, 0 = released
    if (val < 0) {
        LOG_ERR("Failed to read pin: %d", val);
        return;
    }

    button_pressed = (val == 1);
    LOG_INF("Button %s", button_pressed ? "pressed" : "released");

	if (!k_sem_count_get(&button_wait_sem) && button_pressed) {
		k_sem_give(&button_wait_sem);
	}
}

/**
 * Configure LPGPIO0 and LPGPIO1 as inputs with interrupts
 * With B1/E1C need to connect wire between SW5 (P5_7) and P15_0 to use button as Push-to-talk button.
 * With B1/E1C devkit you need to also remove resistor R64. */
static int configure_lpgpio(void)
{
	int ret;
	const struct gpio_dt_spec *spec = &lpgpio_config;

	if (!spec || !spec->port) {
		/* Just ignore */
		LOG_ERR("lpgpio invalid");
		return -ENODEV;
	}

	/* Configure LPGPIO0 for wakeup */
	if (!gpio_is_ready_dt(spec)) {
		LOG_ERR("LPGPIO0 device is not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(spec, GPIO_INPUT | spec->dt_flags);
	if (ret != 0) {
		LOG_ERR("Failed to configure LPGPIO as input: %d", ret);
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(spec,
					      GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		LOG_ERR("Failed to configure LPGPIO interrupt: %d", ret);
		return ret;
	}

	gpio_init_callback(&button_cb_data, button_callback, BIT(spec->pin));
	ret = gpio_add_callback(spec->port, &button_cb_data);
	if (ret != 0) {
		LOG_ERR("Failed to add button callback: %d", ret);
		return ret;
	}

	LOG_INF("LPGPIO%d configured", spec->pin);

	return 0;
}

/** @brief   Verify input and output tensor are of certain min dimensions. */
static bool VerifyTensorDimensions(const arm::app::fwk::iface::Model& model);

void MainLoop()
{
	por_boot = true;
	k_sem_init(&button_wait_sem, 0, 1);

	if (configure_lpgpio()) {
		LOG_ERR("Failed to configure LPGPIO for button input");
		return;
	}

    AsrModel model; /* Model wrapper object. */

    arm::app::fwk::iface::MemoryRegion modelMem{arm::app::asr::GetModelPointer(),
                                                arm::app::asr::GetModelLen()};
    arm::app::fwk::iface::MemoryRegion computeMem{arm::app::activationBuf,
                                                  sizeof(arm::app::activationBuf)};

    /* Load the model. */
    if (!model.Init(computeMem, modelMem)) {
        LOG_ERR("Failed to initialise model");
        return;
    }

    if (!VerifyTensorDimensions(model)) {
        LOG_ERR("Model's input or output dimension verification failed");
        return;
    }

    /* Instantiate application context. */
    arm::app::ApplicationContext caseContext;
    std::vector <std::string> labels;
    GetLabelsVector(labels);

#if defined(MLEK_FWK_TFLM)
    arm::app::AsrClassifier classifier{
	arm::app::fwk::tflm::Wav2LetterModel::ms_inputRowsIdx,
	arm::app::fwk::tflm::Wav2LetterModel::ms_inputColsIdx,
	arm::app::fwk::tflm::Wav2LetterModel::ms_outputRowsIdx,
	arm::app::fwk::tflm::Wav2LetterModel::ms_outputColsIdx}; /* Classifier wrapper object. */
    caseContext.Set<uint32_t>("frameLength", arm::app::asr::g_FrameLength);
    caseContext.Set<uint32_t>("frameStride", arm::app::asr::g_FrameStride);
    caseContext.Set<float>("scoreThreshold", arm::app::asr::g_ScoreThreshold);  /* Score threshold. */
    caseContext.Set<uint32_t>("ctxLen", arm::app::asr::g_ctxLen);  /* Left and right context length (MFCC feat vectors). */
    caseContext.Set<arm::app::AsrClassifier&>("classifier", classifier);
#elif defined(MLEK_FWK_EXECUTORCH)
    caseContext.Set<uint32_t>("melSpecWindowSize", arm::app::asr::g_melSpecWindowSize);
    caseContext.Set<uint32_t>("melSpecHopSize", arm::app::asr::g_melSpecHopSize);
    caseContext.Set<uint32_t>("chunkSize", arm::app::asr::g_chunkSize);
#endif /** MLEK_FWK_TFLM or MLEK_FWK_EXECUTORCH */
    caseContext.Set<arm::app::fwk::iface::Model&>("model", model);
    caseContext.Set<const std::vector <std::string>&>("labels", labels);

	// gpio_debug_init_port(PORT_6);
	power_mgr_allow_sleep();
	por_boot = false;

	while (1) {
#if CONFIG_PUSH_TO_TALK
		LOG_INF("Push to talk start by going to STOP mode. Press button to start capturing audio.\n");
		LOG_INF("Release the button to stop capturing audio.\n");
		LOG_INF("NOTE: If the button is not released, audio will be captured ~10 seconds.\n");
		k_sem_reset(&button_wait_sem);
		k_sem_take(&button_wait_sem, K_FOREVER);
#else
		LOG_INF("Press the button to start capturing audio...\n");
		LOG_INF("Release the button to stop capturing audio.\n");
		LOG_INF("NOTE: If the button is not released, audio will be captured ~10 seconds.\n");
#endif

		power_mgr_disable_sleep();

		// Set button press to false to start capturing audio. In button callback, button_pressed will be set to true once the button is released.
		ClassifyAudioHandler(caseContext);
#if CONFIG_PUSH_TO_TALK
		power_mgr_allow_sleep();
		/* In sleep.overlay, min-residency-us is 1s, so sleep for longer than,
		   that to ensure we enter the deepest sleep state. Which in Alif
		   chip is STOP mode. */
#endif
	}
}

static bool VerifyTensorDimensions(const arm::app::fwk::iface::Model& model)
{
    /* Populate tensor related parameters. */
    auto inputTensor = model.GetInputTensor(0);
    if (inputTensor->Shape().empty()) {
        LOG_ERR("Invalid input tensor dims\n");
        return false;
    }
    if (inputTensor->Shape().size() < 3) {
        LOG_ERR("Input tensor dimension should be >= 3\n");
        return false;
    }

    auto outputTensor = model.GetOutputTensor(0);
    if (outputTensor->Shape().empty()) {
        LOG_ERR("Invalid output tensor dims\n");
        return false;
    }

    if (outputTensor->Shape().size() < 3) {
        LOG_ERR("Output tensor dimension should be >= 3\n");
        return false;
    }

    return true;
}
