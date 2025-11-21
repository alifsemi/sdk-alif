# Copyright 2021-2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Detect NPU architecture from ETHOSU_TARGET_NPU_CONFIG
# This application requires Ethos-U85 with 256 MACs
if(ETHOSU_TARGET_NPU_CONFIG MATCHES "^ethos-(u[0-9]+)-([0-9]+)$")
    set(ETHOSU_ARCH "${CMAKE_MATCH_1}")
    set(ETHOSU_MACS "${CMAKE_MATCH_2}")

    message(STATUS "ETHOSU_TARGET_NPU_CONFIG: ${ETHOSU_TARGET_NPU_CONFIG}")
    message(STATUS "Detected NPU Architecture: ${ETHOSU_ARCH}, MACs: ${ETHOSU_MACS}")

    # Validate for U85 only
    if(ETHOSU_ARCH STREQUAL "u85")
        # Validate board supports U85 (E4, E8 boards)
        if(NOT (CONFIG_SOC_SERIES_E8 OR CONFIG_SOC_SERIES_E4))
            message(FATAL_ERROR 
                "BERT-Tiny transformer application requires Ethos-U85 on E4/E8 boards.\n"
                "Current board does not support U85.\n"
                "For this board, use the standard tflm_ethosu application instead.")
        endif()

        # Validate MAC configuration - U85 only supports 256 MACs on Alif boards
        if(NOT (ETHOSU_MACS STREQUAL "256"))
            message(FATAL_ERROR 
                "Unsupported U85 MAC configuration: ${ETHOSU_MACS}\n"
                "Alif Ensemble boards only support Ethos-U85 with 256 MACs.\n"
                "Use: -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256")
        endif()

        # Validate overlay file matches NPU type
        if(DEFINED EXTRA_DTC_OVERLAY_FILE)
            if(EXTRA_DTC_OVERLAY_FILE MATCHES "ethosu55\\.overlay")
                message(FATAL_ERROR 
                    "Overlay mismatch: ETHOSU_TARGET_NPU_CONFIG='${ETHOSU_TARGET_NPU_CONFIG}' (U85) "
                    "but using U55 overlay '${EXTRA_DTC_OVERLAY_FILE}'.\n"
                    "For U85, use: -DEXTRA_DTC_OVERLAY_FILE=\"boards/enable_ethosu85.overlay\"")
            endif()
        endif()

        # Set architecture flag
        set(ETHOSU_ARCH_U85 y CACHE BOOL "U85 Flag")
        add_compile_definitions(ETHOSU_ARCH_U85="${ETHOSU_ARCH_U85}")

        # Automatically set CONFIG_ARM_ETHOS_U85_256
        set(CONFIG_ARM_ETHOS_U85_256 y CACHE BOOL "U85 256 MACs" FORCE)
        message(STATUS "Auto-configured: CONFIG_ARM_ETHOS_U85_256=y")

        message(STATUS "Building BERT-Tiny for Ethos-U85 with ${ETHOSU_MACS} MACs")

    else()
        message(FATAL_ERROR 
            "BERT-Tiny transformer application requires Ethos-U85 NPU.\n"
            "Current NPU architecture: ${ETHOSU_ARCH}\n"
            "Use: -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256\n"
            "For U55 boards, please use the standard tflm_ethosu application.")
    endif()

else()
    message(FATAL_ERROR 
        "Invalid or missing ETHOSU_TARGET_NPU_CONFIG: '${ETHOSU_TARGET_NPU_CONFIG}'\n"
        "BERT-Tiny requires: -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256")
endif()

project(tflm_bert_tiny_app)

# Include directories
target_include_directories(app PRIVATE ../../../../include)
target_include_directories(app PRIVATE ../../../../include/ethosu/models/bert_tiny/u85)
target_include_directories(app PRIVATE ../../../../lib/ethosu_utils)

# Application sources (includes custom BERT-optimized inference_process)
target_sources(app PRIVATE 
    src/main.cpp
    src/inference_process_bert.cpp
)

# BERT-Tiny model sources
target_sources(app PRIVATE
    ../../../../include/ethosu/models/bert_tiny/u85/model_u85_256.c
    ../../../../include/ethosu/models/bert_tiny/u85/input.c
    ../../../../include/ethosu/models/bert_tiny/u85/output.c
)
