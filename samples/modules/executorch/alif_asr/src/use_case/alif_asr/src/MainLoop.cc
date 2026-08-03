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

#include "mlek/fwk/executorch/ConformerModel.hpp"
using AsrModel = arm::app::fwk::et::ConformerModel;

#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(MainLoop);

namespace arm {
namespace app {
    static uint8_t activationBuf[ACTIVATION_BUF_SZ] ACTIVATION_BUF_ATTRIBUTE;

    namespace asr {
        extern uint8_t* GetModelPointer();
        extern size_t GetModelLen();
    } /* namespace asr */

} /* namespace app */
} /* namespace arm */

/** @brief   Verify input and output tensor are of certain min dimensions. */
static bool VerifyTensorDimensions(const arm::app::fwk::iface::Model& model);

void MainLoop()
{
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

    caseContext.Set<uint32_t>("melSpecWindowSize", arm::app::asr::g_melSpecWindowSize);
    caseContext.Set<uint32_t>("melSpecHopSize", arm::app::asr::g_melSpecHopSize);
    caseContext.Set<uint32_t>("chunkSize", arm::app::asr::g_chunkSize);
    caseContext.Set<arm::app::fwk::iface::Model&>("model", model);
    caseContext.Set<const std::vector <std::string>&>("labels", labels);

	while (1) {
		ClassifyAudioHandler(caseContext);
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
