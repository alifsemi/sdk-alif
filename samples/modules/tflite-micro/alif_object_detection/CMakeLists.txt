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

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(alif_object_detection)

# Silence "changed in GCC 7.1" warnigns from C++ headers
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-psabi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

# Add include directories
zephyr_include_directories(
	src/aipl/
)

add_subdirectory(src/application/api/use_case/alif_ui)

# ---------------------------------------------------------------------------
# Build-time model generation from MLEK resources
# ---------------------------------------------------------------------------
set(MLEK_SCRIPTS_DIR ${MLEK_ROOT}/scripts)
set(RESOURCES_PATH ${MLEK_ROOT}/resources_downloaded CACHE PATH
    "Path to MLEK resources (models compiled with Vela)")
setup_source_generator()

# Source generation output directories
set(SRC_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/src)
set(INC_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/include)
file(MAKE_DIRECTORY ${SRC_GEN_DIR} ${INC_GEN_DIR})

# Determine NPU config ID based on Zephyr Kconfig
if (CONFIG_ARM_ETHOS_U55_256)
    set(ETHOS_U_NPU_CONFIG_ID "H256")
else()
    set(ETHOS_U_NPU_CONFIG_ID "H128")
endif()

# Model file path
set(OD_MODEL_PATH ${RESOURCES_PATH}/object_detection/yolo-fastest_192_face_v4_vela_${ETHOS_U_NPU_CONFIG_ID}.tflite
    CACHE FILEPATH "Path to the object detection TFLite model (Vela-compiled)")

if (NOT EXISTS ${OD_MODEL_PATH})
    message(FATAL_ERROR
        "Object detection model not found at ${OD_MODEL_PATH}\n"
        "Run set_up_default_resources.py first:\n"
        "  python3 ${MLEK_ROOT}/set_up_default_resources.py\n")
endif()

# Generate model C array with anchor and image size parameters
set(EXTRA_MODEL_CODE
    "extern const int originalImageSize = 192;"
    "/* NOTE: anchors are different for any given input model size, estimated during training phase */"
    "extern const float anchor1[] = {38, 77, 47, 97, 61, 126};"
    "extern const float anchor2[] = {14, 26, 19, 37, 28, 55 };"
)

generate_model_code(
    MODEL_PATH  ${OD_MODEL_PATH}
    DESTINATION ${SRC_GEN_DIR}
    EXPRESSIONS ${EXTRA_MODEL_CODE}
    NAMESPACE   "arm" "app" "object_detection"
)

# Collect the generated source files
file(GLOB OD_GENERATED_SOURCES ${SRC_GEN_DIR}/*.cc)

# ---------------------------------------------------------------------------

target_include_directories(app PRIVATE
    src/application/main/include
    src/use_case/object_detection/include
    ${INC_GEN_DIR}
)

# initialize DSI before LVGL
math(EXPR APP_SET_DSI_CDC_PRIORITY "${CONFIG_LV_Z_INIT_PRIORITY} - 1")

target_compile_definitions(app PRIVATE ACTIVATION_BUF_SZ=CONFIG_ACTIVATION_BUF_SZ APP_SET_DSI_CDC_PRIORITY=${APP_SET_DSI_CDC_PRIORITY})

target_sources(app PRIVATE
    src/application/main/Main.cc
    src/use_case/object_detection/src/MainLoop.cc
    src/use_case/object_detection/src/UseCaseHandler.cc
    src/use_case/object_detection/src/image_ensemble.c
    src/use_case/object_detection/src/image_processing.c
    ${OD_GENERATED_SOURCES}
)

target_link_libraries(app PRIVATE
    mlek::object_detection_impl
    alif_ui_api
)
