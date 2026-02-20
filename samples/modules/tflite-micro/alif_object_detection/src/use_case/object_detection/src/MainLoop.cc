/*
 * Copyright (c) 2021 - 2022 Arm Limited. All rights reserved.
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

#include "YoloFastestModel.hpp"     /* Model class for running inference. */
#include "UseCaseHandler.hpp"       /* Handlers for different user options. */
#include "BufAttributes.hpp"        /* Buffer attributes to be applied */
#include <zephyr/logging/log.h>

#include <zephyr/kernel.h>

LOG_MODULE_REGISTER(MainLoop);

namespace arm {
namespace app {
    static uint8_t tensorArena[ACTIVATION_BUF_SZ] ACTIVATION_BUF_ATTRIBUTE;
    namespace object_detection {
        extern uint8_t* GetModelPointer();
        extern size_t GetModelLen();
    } /* namespace object_detection */
} /* namespace app */
} /* namespace arm */

//using ImgClassClassifier = arm::app::Classifier;

void main_loop()
{
    arm::app::YoloFastestModel model;  /* Model wrapper object. */

    if (!alif::app::ObjectDetectionInit()) {
        LOG_ERR("Failed to initialise use case handler");
    }

    /* Load the model. */
    if (!model.Init(arm::app::tensorArena,
                    sizeof(arm::app::tensorArena),
                    arm::app::object_detection::GetModelPointer(),
                    arm::app::object_detection::GetModelLen())) {
        LOG_ERR("Failed to initialise model");
        return;
    }

    LOG_INF("model.Init done");
    /* Instantiate application context. */
    arm::app::ApplicationContext caseContext;

    //arm::app::Profiler profiler{"object_detection"};
    //caseContext.Set<arm::app::Profiler&>("profiler", profiler);
    caseContext.Set<arm::app::Model&>("model", model);

    LOG_INF("Entering main loop");
    /* Loop. */
    do {
        bool executionSuccessful = alif::app::ObjectDetectionHandler(caseContext);
        if(!executionSuccessful) {
            LOG_ERR("ObjectDetectionHandler failed");
        }
    } while (1);
}
