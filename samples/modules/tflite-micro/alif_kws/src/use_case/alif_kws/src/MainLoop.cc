/*
 * Copyright (c) 2021 Arm Limited. All rights reserved.
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
#include "mlek/use_case/kws/KwsClassifier.hpp"  /* Classifier. */
#include "mlek/fwk/tflm/MicroNetKwsModel.hpp"   /* Model class for running inference. */
#include "Labels.hpp"                            /* For label strings. */
#include "UseCaseHandler.hpp"                    /* Handlers for different user options. */
#include "BufAttributes.hpp"                     /* Buffer attributes to be applied */

#include <zephyr/console/console.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(MainLoop);

namespace arm
{
namespace app
{
static uint8_t tensorArena[ACTIVATION_BUF_SZ] ACTIVATION_BUF_ATTRIBUTE;
namespace kws
{
extern uint8_t *GetModelPointer();
extern size_t GetModelLen();
extern const int g_AudioRate;
} /* namespace kws */
} /* namespace app */
} /* namespace arm */

enum opcodes {
	MENU_OPT_RUN_ONCE = 1,
	MENU_OPT_RUN_CONTINUOUS,
};

static void DisplayMenu()
{
	LOG_INF("User input required");
	LOG_INF("Enter option number from:");
	LOG_INF("%u. Run classification on one audio window", MENU_OPT_RUN_ONCE);
	LOG_INF("%u. Run classification continuously", MENU_OPT_RUN_CONTINUOUS);
	LOG_INF("Choice:");
	fflush(stdout);
}

void main_loop()
{
	arm::app::fwk::tflm::MicroNetKwsModel model; /* Model wrapper object. */

	arm::app::fwk::iface::MemoryRegion computeMem{arm::app::tensorArena,
							sizeof(arm::app::tensorArena)};
	arm::app::fwk::iface::MemoryRegion modelMem{arm::app::kws::GetModelPointer(),
							arm::app::kws::GetModelLen()};

	/* Load the model. */
	if (!model.Init(computeMem, modelMem)) {
		LOG_ERR("Failed to initialise model");
		return;
	}

	/* Instantiate application context. */
	arm::app::ApplicationContext caseContext;

	caseContext.Set<arm::app::fwk::iface::Model &>("model", model);
	caseContext.Set<int>("frameLength", arm::app::kws::g_FrameLength);
	caseContext.Set<int>("frameStride", arm::app::kws::g_FrameStride);
	caseContext.Set<int>("audioRate", arm::app::kws::g_AudioRate);
	caseContext.Set<float>("scoreThreshold",
			       arm::app::kws::g_ScoreThreshold); /* Normalised score threshold. */

	arm::app::KwsClassifier classifier; /* classifier wrapper object. */
	caseContext.Set<arm::app::KwsClassifier &>("classifier", classifier);

	std::vector<std::string> labels;
	GetLabelsVector(labels);

	caseContext.Set<const std::vector<std::string> &>("labels", labels);

	bool executionSuccessful = true;

#if USE_APP_MENU
	constexpr bool bUseMenu = true;
#else
	constexpr bool bUseMenu = false;
#endif

	/* Loop. */
	do {
		int menuOption = MENU_OPT_RUN_CONTINUOUS;
		if (bUseMenu) {
			DisplayMenu();
			menuOption = atoi(console_getline());
		}
		switch (menuOption) {
		case MENU_OPT_RUN_ONCE:
			executionSuccessful = alif::app::ClassifyAudioHandler(caseContext, true);
			break;
		case MENU_OPT_RUN_CONTINUOUS:
			executionSuccessful = alif::app::ClassifyAudioHandler(caseContext, false);
			break;
		default:
			LOG_ERR("Incorrect choice, try again.");
			break;
		}
	} while (executionSuccessful || bUseMenu);
	LOG_INF("Main loop terminated.");
}
