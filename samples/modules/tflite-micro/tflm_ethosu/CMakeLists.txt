# Copyright 2021-2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(tflm_ethosu_app)

if (CONFIG_ETHOS_U_AXI0_ONLY)
target_compile_definitions(ethosu_core_driver PRIVATE
    NPU_QCONFIG=1
    NPU_REGIONCFG_0=1
    NPU_REGIONCFG_1=0
    NPU_REGIONCFG_2=1
    )
endif()

target_include_directories(app PRIVATE src/models/keyword_spotting_cnn_small_int8)

add_subdirectory(
    ../../../../lib/ethosu_utils
    ${CMAKE_BINARY_DIR}/lib/ethosu_utils
)

target_sources(app PRIVATE src/main.cpp)
