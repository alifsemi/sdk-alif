# Copyright 2021-2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Detect NPU architecture from ETHOSU_TARGET_NPU_CONFIG
# This is the SINGLE SOURCE OF TRUTH for NPU configuration
if(ETHOSU_TARGET_NPU_CONFIG MATCHES "^ethos-(u[0-9]+)-([0-9]+)$")
    set(ETHOSU_ARCH "${CMAKE_MATCH_1}")
    set(ETHOSU_MACS "${CMAKE_MATCH_2}")

    message(STATUS "ETHOSU_TARGET_NPU_CONFIG: ${ETHOSU_TARGET_NPU_CONFIG}")
    message(STATUS "Detected NPU Architecture: ${ETHOSU_ARCH}, MACs: ${ETHOSU_MACS}")

    # Validate and configure for U55/U65
    if((ETHOSU_ARCH STREQUAL "u55") OR (ETHOSU_ARCH STREQUAL "u65"))
        # Validate MAC configuration based on core type (HE vs HP)
        if(CONFIG_RTSS_HE)
            # HE core only supports 128 MACs for U55
            if(NOT (ETHOSU_MACS STREQUAL "128"))
                message(FATAL_ERROR 
                    "Core type mismatch: RTSS_HE core with U55 only supports 128 MACs.\n"
                    "You specified: ${ETHOSU_MACS} MACs\n"
                    "For HE core, use: -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128\n"
                    "For 256 MACs, use HP core board variant (rtss_hp)")
            endif()
            message(STATUS "Detected RTSS_HE core - U55 restricted to 128 MACs")
        elseif(CONFIG_RTSS_HP)
            # HP core only supports 256 MACs for U55
            if(NOT (ETHOSU_MACS STREQUAL "256"))
                message(FATAL_ERROR 
                    "Core type mismatch: RTSS_HP core with U55 only supports 256 MACs.\n"
                    "You specified: ${ETHOSU_MACS} MACs\n"
                    "For HP core, use: -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256\n"
                    "For 128 MACs, use HE core board variant (rtss_he)")
            endif()
            message(STATUS "Detected RTSS_HP core - U55 restricted to 256 MACs")
        else()
            # Fallback validation for other core types
            if(NOT (ETHOSU_MACS STREQUAL "64" OR ETHOSU_MACS STREQUAL "128" OR ETHOSU_MACS STREQUAL "256"))
                message(FATAL_ERROR "Unsupported U55/U65 MAC configuration: ${ETHOSU_MACS}. Valid: 64, 128, 256")
            endif()
        endif()

        # Validate overlay file matches NPU type
        if(DEFINED EXTRA_DTC_OVERLAY_FILE)
            if(EXTRA_DTC_OVERLAY_FILE MATCHES "ethosu85\\.overlay")
                message(FATAL_ERROR 
                    "Overlay mismatch: ETHOSU_TARGET_NPU_CONFIG='${ETHOSU_TARGET_NPU_CONFIG}' (U55) "
                    "but using U85 overlay '${EXTRA_DTC_OVERLAY_FILE}'.\n"
                    "For U55, use: -DEXTRA_DTC_OVERLAY_FILE=\"boards/enable_ethosu55.overlay\"")
            endif()
        endif()

        # Set architecture flag
        set(ETHOSU_ARCH_U55 y CACHE BOOL "U55 Flag")
        add_compile_definitions(ETHOSU_ARCH_U55="${ETHOSU_ARCH_U55}")

        # Automatically set the appropriate CONFIG_ARM_ETHOS_U55_* based on MACS
        if(ETHOSU_MACS STREQUAL "256")
            set(CONFIG_ARM_ETHOS_U55_256 y CACHE BOOL "U55 256 MACs" FORCE)
            message(STATUS "Auto-configured: CONFIG_ARM_ETHOS_U55_256=y")
        elseif(ETHOSU_MACS STREQUAL "128")
            set(CONFIG_ARM_ETHOS_U55_128 y CACHE BOOL "U55 128 MACs" FORCE)
            message(STATUS "Auto-configured: CONFIG_ARM_ETHOS_U55_128=y")
        elseif(ETHOSU_MACS STREQUAL "64")
            set(CONFIG_ARM_ETHOS_U55_64 y CACHE BOOL "U55 64 MACs" FORCE)
            message(STATUS "Auto-configured: CONFIG_ARM_ETHOS_U55_64=y")
        endif()

        message(STATUS "Building for Ethos-U55 with ${ETHOSU_MACS} MACs")

    # Validate and configure for U85
    elseif(ETHOSU_ARCH STREQUAL "u85")
        # Validate board supports U85 FIRST (before MAC check)
        # Note: E4, E8 boards have dual NPUs (U55 + U85)
        if(NOT (CONFIG_SOC_SERIES_E8 OR CONFIG_SOC_SERIES_E4))
            message(FATAL_ERROR 
                "Ethos-U85 is only available on E4, E8 boards.\n"
                "Current board does not support U85.\n"
                "For this board, use: -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256")
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

        # Automatically set CONFIG_ARM_ETHOS_U85_256 (only 256 MACs supported)
        set(CONFIG_ARM_ETHOS_U85_256 y CACHE BOOL "U85 256 MACs" FORCE)
        message(STATUS "Auto-configured: CONFIG_ARM_ETHOS_U85_256=y")

        message(STATUS "Building for Ethos-U85 with ${ETHOSU_MACS} MACs")

    else()
        message(FATAL_ERROR "Unsupported NPU architecture: ${ETHOSU_ARCH}. Valid: u55, u65, u85")
    endif()

else()
    message(FATAL_ERROR 
        "Invalid or missing ETHOSU_TARGET_NPU_CONFIG: '${ETHOSU_TARGET_NPU_CONFIG}'\n"
        "Required format: ethos-u<arch>-<macs>\n"
        "Examples:\n"
        "  -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128\n"
        "  -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256\n"
        "  -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256")
endif()

project(tflm_ethosu_app)

target_include_directories(app PRIVATE ../../../../include)

add_subdirectory(
    ../../../../lib/ethosu_utils
    ${CMAKE_BINARY_DIR}/lib/ethosu_utils
)

target_sources(app PRIVATE src/main.cpp)
