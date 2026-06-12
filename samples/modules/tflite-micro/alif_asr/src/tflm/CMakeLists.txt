# This file was ported to work on Alif Semiconductor devices.

#  Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
#  Use, distribution and modification of this code is permitted under the
#  terms stated in the Alif Semiconductor Software License Agreement
#
#  You should have received a copy of the Alif Semiconductor Software
#  License Agreement with this file. If not, please write to:
#  contact@alifsemi.com, or visit: https://alifsemi.com/license

#----------------------------------------------------------------------------
#  SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its
#  affiliates <open-source-office@arm.com>
#  SPDX-License-Identifier: Apache-2.0
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#----------------------------------------------------------------------------

#########################################################
#               TensorFlow Lite Micro wrapper           #
#########################################################
cmake_minimum_required(VERSION 3.21.0)

set(ML_FWK_TFLM_TARGET   ml_framework_tflm)
project(${ML_FWK_TFLM_TARGET}
    DESCRIPTION     "TensorFlow Lite Micro framework module"
    LANGUAGES       C CXX)

if (NOT "TensorFlowLiteMicro" STREQUAL ${ML_FRAMEWORK})
    message(FATAL_ERROR "${CMAKE_CURRENT_SOURCE_DIR} should not be included for ${ML_FRAMEWORK}")
endif()

# Create static library
add_library(${ML_FWK_TFLM_TARGET} STATIC
    Wav2LetterModel.cc
    TflmTensor.cc
    TensorFlowLiteMicro.cc
    TflmModel.cc
)

## Include directories - public
target_include_directories(${ML_FWK_TFLM_TARGET}
    PUBLIC
    include
)

# Link time library targets:
target_link_libraries(${ML_FWK_TFLM_TARGET} PUBLIC
	arm_math                # Math functions
	zephyr_interface
	asr_iface
)

target_compile_definitions(${ML_FWK_TFLM_TARGET} INTERFACE MLEK_FWK_TFLM=1)

# Display status:
message(STATUS "*******************************************************")
message(STATUS "Library: "                ${ML_FWK_TFLM_TARGET})
message(STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "*******************************************************")
