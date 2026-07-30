# SPDX-FileCopyrightText: Copyright Alif Semiconductor
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# Locate the alif-mlek west module. Its Zephyr port lives in a sub-directory,
# so the module is not auto-discovered and must be registered explicitly. Ask
# west for the module path instead of hardcoding the workspace layout.
execute_process(
    COMMAND west list --format "{abspath}" alif-mlek
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE MLEK_ROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE MLEK_LIST_RESULT
    ERROR_QUIET
)
if (MLEK_LIST_RESULT OR NOT EXISTS ${MLEK_ROOT})
    message(FATAL_ERROR
        "alif-mlek module not found. Enable and fetch it first:\n"
        "  west config manifest.project-filter -- +alif-mlek\n"
        "  west update\n")
endif()

# Register the MLEK Zephyr port as an extra Zephyr module
list(APPEND ZEPHYR_EXTRA_MODULES ${MLEK_ROOT}/source/lib/ports/zephyr)

# ---------------------------------------------------------------------------
# Ethos-U NPU selection
# ---------------------------------------------------------------------------
# When ETHOSU_TARGET_NPU_CONFIG is provided on the command line it is the single
# source of truth for the NPU. We must act on it BEFORE find_package(Zephyr) so
# that the matching Kconfig choice (CONFIG_ARM_ETHOS_U<arch>_<macs>) is fed into
# the build and the Ethos-U driver targets the requested NPU. This overrides the
# board default (E8 rtss_hp defaults to Ethos-U55 with 256 MACs).
#
# The parsed value is also used to pick the correct Vela-compiled model below.
# Vela config-ID prefix: U55 -> H, U65 -> Y, U85 -> Z (e.g. ethos-u85-256 -> Z256).
if (DEFINED ETHOSU_TARGET_NPU_CONFIG)
    string(TOLOWER "${ETHOSU_TARGET_NPU_CONFIG}" _npu_cfg)
    if (_npu_cfg MATCHES "^ethos-(u[0-9]+)-([0-9]+)$")
        set(_npu_arch "${CMAKE_MATCH_1}")
        set(_npu_macs "${CMAKE_MATCH_2}")
        if (_npu_arch STREQUAL "u55")
            set(_npu_prefix "H")
        elseif (_npu_arch STREQUAL "u65")
            set(_npu_prefix "Y")
        elseif (_npu_arch STREQUAL "u85")
            set(_npu_prefix "Z")
        else()
            message(FATAL_ERROR
                "Unsupported Ethos-U architecture '${_npu_arch}' in "
                "ETHOSU_TARGET_NPU_CONFIG='${ETHOSU_TARGET_NPU_CONFIG}'")
        endif()
        set(ETHOS_U_NPU_CONFIG_ID "${_npu_prefix}${_npu_macs}")

        # Feed the matching Kconfig choice into the Zephyr configuration.
        string(TOUPPER "${_npu_arch}" _npu_arch_uc)
        set(CONFIG_ARM_ETHOS_${_npu_arch_uc}_${_npu_macs} y
            CACHE INTERNAL "Selected Ethos-U NPU configuration" FORCE)
        message(STATUS
            "Ethos-U: ETHOSU_TARGET_NPU_CONFIG=${ETHOSU_TARGET_NPU_CONFIG} -> "
            "CONFIG_ARM_ETHOS_${_npu_arch_uc}_${_npu_macs}=y, model ID ${ETHOS_U_NPU_CONFIG_ID}")
    else()
        message(FATAL_ERROR
            "Invalid ETHOSU_TARGET_NPU_CONFIG='${ETHOSU_TARGET_NPU_CONFIG}'.\n"
            "Expected format: ethos-u<arch>-<macs>, e.g. ethos-u55-256 or ethos-u85-256")
    endif()
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(alif_img_class)

# Silence "changed in GCC 7.1" warnigns from C++ headers
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-psabi")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

# Add include directories
zephyr_include_directories(
	src/aipl/
)

add_subdirectory(src/application/api/use_case/alif_ui)

# ---------------------------------------------------------------------------
# Build-time model and labels generation from MLEK resources
# ---------------------------------------------------------------------------
set(MLEK_SCRIPTS_DIR ${MLEK_ROOT}/scripts)
set(RESOURCES_PATH ${MLEK_ROOT}/resources_downloaded CACHE PATH
    "Path to MLEK resources (models compiled with Vela)")
setup_source_generator()

# Source generation output directories
set(SRC_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/src)
set(INC_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/include)
file(MAKE_DIRECTORY ${SRC_GEN_DIR} ${INC_GEN_DIR})

# Determine NPU config ID based on Zephyr Kconfig.
# If ETHOSU_TARGET_NPU_CONFIG was supplied on the command line, ETHOS_U_NPU_CONFIG_ID
# has already been computed above. Otherwise fall back to the board Kconfig defaults.
if (NOT DEFINED ETHOS_U_NPU_CONFIG_ID)
    if (CONFIG_ARM_ETHOS_U85_256)
        set(ETHOS_U_NPU_CONFIG_ID "Z256")
    elseif (CONFIG_ARM_ETHOS_U55_256)
        set(ETHOS_U_NPU_CONFIG_ID "H256")
    else()
        set(ETHOS_U_NPU_CONFIG_ID "H128")
    endif()
endif()

# Model file path
set(IMG_CLASS_MODEL_PATH ${RESOURCES_PATH}/img_class/mobilenet_v2_1.0_224_INT8_vela_${ETHOS_U_NPU_CONFIG_ID}.tflite
    CACHE FILEPATH "Path to the image classification TFLite model (Vela-compiled)")

if (NOT EXISTS ${IMG_CLASS_MODEL_PATH})
    message(FATAL_ERROR
        "Image classification model not found at ${IMG_CLASS_MODEL_PATH}\n"
        "Run set_up_default_resources.py first:\n"
        "  python3 ${MLEK_ROOT}/set_up_default_resources.py\n")
endif()

# Generate model C array (no extra model code expressions for img_class TFLM)
generate_model_code(
    MODEL_PATH  ${IMG_CLASS_MODEL_PATH}
    DESTINATION ${SRC_GEN_DIR}
    NAMESPACE   "arm" "app" "img_class"
)

# Generate labels C++ code
set(IMG_CLASS_LABELS_FILE ${MLEK_ROOT}/resources/img_class/labels/labels_mobilenet_v2_1.0_224.txt)

generate_labels_code(
    INPUT           ${IMG_CLASS_LABELS_FILE}
    DESTINATION_SRC ${SRC_GEN_DIR}
    DESTINATION_HDR ${INC_GEN_DIR}
    OUTPUT_FILENAME Labels
)

# Collect the generated source files
file(GLOB IMG_CLASS_GENERATED_SOURCES ${SRC_GEN_DIR}/*.cc)

# ---------------------------------------------------------------------------

target_include_directories(app PRIVATE
    src/application/main/include
    src/use_case/alif_img_class/include
    ${INC_GEN_DIR}
)

# initialize DSI before LVGL
math(EXPR APP_SET_DSI_CDC_PRIORITY "${CONFIG_LV_Z_INIT_PRIORITY} - 1")

target_compile_definitions(app PRIVATE ACTIVATION_BUF_SZ=CONFIG_ACTIVATION_BUF_SZ APP_SET_DSI_CDC_PRIORITY=${APP_SET_DSI_CDC_PRIORITY})

target_sources(app PRIVATE
    src/application/main/Main.cc
    src/use_case/alif_img_class/src/MainLoop.cc
    src/use_case/alif_img_class/src/UseCaseHandler.cc
    src/use_case/alif_img_class/src/image_ensemble.c
    src/use_case/alif_img_class/src/image_processing.c
    ${IMG_CLASS_GENERATED_SOURCES}
)

target_link_libraries(app PRIVATE
    mlek::img_class_impl
    alif_ui_api
)
