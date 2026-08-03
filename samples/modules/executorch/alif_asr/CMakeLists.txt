# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

# ExecuTorch emits install(EXPORT ExecuTorchTargets ...) rules that reference
# Zephyr-provided targets (zephyr_interface, ethosu_core_driver) which are not
# part of any export set, causing CMake generation to fail. We never install
# ExecuTorch in a Zephyr firmware build, so skip install rules entirely.
set(CMAKE_SKIP_INSTALL_RULES ON CACHE BOOL "" FORCE)

if(NOT ospi-flash IN_LIST SNIPPET)
  set(SNIPPET ospi-flash ${SNIPPET} CACHE STRING "" FORCE)
endif()

set(ML_FRAMEWORK ExecuTorch)

set(ETHOSU_ARCH_U85 y CACHE BOOL "U85 Flag")
add_compile_definitions(ETHOSU_ARCH_U85="${ETHOSU_ARCH_U85}")
set(CONFIG_ARM_ETHOS_U85_256 y CACHE BOOL "U85 256 MACs" FORCE)

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

project(alif_asr)

# The ExecuTorch temporary allocation pool (.bss.NoInit.temp_buf_sram) is too
# large for DTCM; place it in the SRAM1 memory region instead.
zephyr_linker_sources(SECTIONS sram1_temp_buf.ld)

if (CONFIG_ENABLE_DISPLAY)
# The LVGL rendering buffers (.lvgl_buf) and memory pool (.lvgl_heap) are too
# large for DTCM; relocate them into globally addressable SRAM0.
zephyr_linker_sources_ifdef(CONFIG_LV_Z_VDB_CUSTOM_SECTION SECTIONS lvgl_sram.ld)
endif()

# Silence "changed in GCC 7.1" warnigns from C++ headers
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-psabi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

set(PLATFORM_DRIVERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/hal/source/platform/zephyr)


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

message(STATUS "Using SRC_GEN_DIR:    ${SRC_GEN_DIR}")
message(STATUS "Using INC_GEN_DIR:    ${INC_GEN_DIR}")

# Model file path
set(ASR_MODEL_PATH ${RESOURCES_PATH}/asr/conformer_fp32_cln_wer_6_47_arm_delegate_ethos-u85-256.pte
    CACHE FILEPATH "Path to the ASR Executorch model (Vela-compiled)")

if (NOT EXISTS ${ASR_MODEL_PATH})
    message(FATAL_ERROR
        "ASR model not found at ${ASR_MODEL_PATH}\n"
        "Run set_up_default_resources.py first:\n"
        "  python3 ${MLEK_ROOT}/set_up_default_resources.py\n")
endif()

 set(EXTRA_MODEL_CODE
	"/* Model parameters for ${use_case} */"
	"extern const int   g_melSpecWindowSize = 512"
	"extern const int   g_melSpecHopSize    = 160"
	"extern const int   g_chunkSize         = 1500"
	)


# Generate model file
generate_model_code(
    MODEL_PATH  ${ASR_MODEL_PATH}
    DESTINATION ${SRC_GEN_DIR}
    EXPRESSIONS ${EXTRA_MODEL_CODE}
    NAMESPACE   "arm" "app" "asr")

# Generate labels C++ code
set(ASR_LABELS_FILE  ${MLEK_ROOT}/resources/asr/labels/librispeech_sp.pieces)

# Generate labels file
set(ASR_LABELS_CPP_FILE Labels)
generate_labels_code(
    INPUT           "${ASR_LABELS_FILE}"
    DESTINATION_SRC ${SRC_GEN_DIR}
    DESTINATION_HDR ${INC_GEN_DIR}
    OUTPUT_FILENAME "${ASR_LABELS_CPP_FILE}"
)

# Collect the generated source files
file(GLOB ASR_GENERATED_SOURCES ${SRC_GEN_DIR}/*.cc)

# Create static library
add_library(asr_iface INTERFACE)

## Common include directories - public
target_include_directories(asr_iface
    INTERFACE
	src/application/main/include)


target_include_directories(app PRIVATE
    src/application/main/include
    src/use_case/alif_asr/include
	../../alif_common
	../../alif_common/audio
	../../alif_common/audio/include
	 ${INC_GEN_DIR}
	# MLEK application-layer headers (UseCaseCommonUtils.hpp) and the profiler
	# (Profiler.hpp). These live in the MLEK 'app' tree, which is not part of the
	# Zephyr port. Listed after the sample's own include dirs so the vendored
	# AppContext.hpp/BufAttributes.hpp take precedence.
	${MLEK_ROOT}/source/app/main/include
)

target_compile_definitions(app PRIVATE ACTIVATION_BUF_SZ=CONFIG_ACTIVATION_BUF_SZ)

if (CONFIG_ENABLE_DISPLAY)
# initialize DSI before LVGL
math(EXPR APP_SET_DSI_CDC_PRIORITY "${CONFIG_LV_Z_INIT_PRIORITY} - 1")

target_compile_definitions(app PRIVATE APP_SET_DSI_CDC_PRIORITY=${APP_SET_DSI_CDC_PRIORITY})

add_subdirectory(src/application/api/use_case/alif_ui)
endif()


target_sources(app PRIVATE
    src/application/main/Main.cc
    src/use_case/alif_asr/src/MainLoop.cc
	src/use_case/alif_asr/src/UseCaseHandlerEt.cc
	../../alif_common/audio/AudioBackend.cpp
	${ASR_GENERATED_SOURCES}
)

set(SRC_MAIN "src/application/main/Main.cc")

list(APPEND SRC_MAIN_DEFS
    "PRJ_VER_STR=\"${CMAKE_PROJECT_VERSION}\""
    "PRJ_DES_STR=\"${CMAKE_PROJECT_DESCRIPTION}\""
    "PRJ_COMPILER=\"${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}\"")

set_source_files_properties(${SRC_MAIN}
    PROPERTIES COMPILE_DEFINITIONS
    "${SRC_MAIN_DEFS}")

target_link_libraries(app PRIVATE
	asr_iface
    mlek::asr_impl
)

if (CONFIG_ENABLE_DISPLAY)
target_link_libraries(app PRIVATE
	alif_ui_api
)
endif()

# The ExecuTorch Zephyr PAL (runtime/platform/default/zephyr.cpp, compiled into
# executorch_core) provides a strong et_pal_emit_log_message that collides with
# mlek's override in EtPal.cc. Weaken only that PAL definition file (not the rest
# of executorch_core, whose platform.cpp takes the addresses of these symbols to
# build the PAL table and must keep strong references) so the mlek override wins
# without patching the executorch submodule.
if(TARGET executorch_core)
    if(NOT DEFINED EXECUTORCH_DIR AND DEFINED ZEPHYR_EXECUTORCH_MODULE_DIR)
        set(EXECUTORCH_DIR ${ZEPHYR_EXECUTORCH_MODULE_DIR})
    endif()
    set_source_files_properties(
        ${EXECUTORCH_DIR}/runtime/platform/default/zephyr.cpp
        TARGET_DIRECTORY executorch_core
        PROPERTIES COMPILE_DEFINITIONS "ET_INTERNAL_PLATFORM_WEAKNESS=ET_WEAK")
endif()

# The EthosUBackend registers itself via a static initializer inside the
# executorch_delegate_ethos_u static library. Without --whole-archive the linker
# drops that object and the runtime reports "Backend EthosUBackend is not
# registered". Force the whole archive in so the registration runs.
if(TARGET executorch_delegate_ethos_u)
    if(NOT COMMAND executorch_target_link_options_shared_lib)
        if(NOT DEFINED EXECUTORCH_DIR AND DEFINED ZEPHYR_EXECUTORCH_MODULE_DIR)
            set(EXECUTORCH_DIR ${ZEPHYR_EXECUTORCH_MODULE_DIR})
        endif()
        include(${EXECUTORCH_DIR}/tools/cmake/Utils.cmake)
    endif()
    executorch_target_link_options_shared_lib(executorch_delegate_ethos_u)
    target_link_libraries(app PRIVATE executorch_delegate_ethos_u)
endif()

# The quantize/dequantize_per_tensor CPU kernels used at the delegate boundary
# are registered via static initializers in quantized_ops_lib. Same as above,
# force the whole archive in or resolve_operator() reports the ops as missing.
if(TARGET quantized_ops_lib)
    if(NOT COMMAND executorch_target_link_options_shared_lib)
        if(NOT DEFINED EXECUTORCH_DIR AND DEFINED ZEPHYR_EXECUTORCH_MODULE_DIR)
            set(EXECUTORCH_DIR ${ZEPHYR_EXECUTORCH_MODULE_DIR})
        endif()
        include(${EXECUTORCH_DIR}/tools/cmake/Utils.cmake)
    endif()
    executorch_target_link_options_shared_lib(quantized_ops_lib)
    target_link_libraries(app PRIVATE quantized_ops_lib)
endif()
