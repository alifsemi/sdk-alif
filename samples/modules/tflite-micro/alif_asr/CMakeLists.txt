# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

if(NOT ospi-flash IN_LIST SNIPPET)
  set(SNIPPET ospi-flash ${SNIPPET} CACHE STRING "" FORCE)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(alif_asr)

# Silence "changed in GCC 7.1" warnigns from C++ headers
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-psabi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

set(PLATFORM_DRIVERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/hal/source/platform/zephyr)

set(ML_FRAMEWORK TensorFlowLiteMicro)

# Create static library
add_library(asr_iface INTERFACE)

## Common include directories - public
target_include_directories(asr_iface
    INTERFACE
    src/iface
	src/application/main/include)

add_subdirectory(src/math)
add_subdirectory(src/common)
add_subdirectory(src/tflm)
add_subdirectory(src/application/api/use_case/asr)

target_include_directories(app PRIVATE
    src/application/main/include
    src/use_case/alif_asr/include
    src/application/api/use_case/asr/include
    src/generated/asr
	src/power
	../alif_common
	../alif_common/audio
	../alif_common/audio/include
)

target_compile_definitions(app PRIVATE ACTIVATION_BUF_SZ=CONFIG_ACTIVATION_BUF_SZ)

if (CONFIG_USE_APP_MENU)
target_compile_definitions(app PRIVATE USE_APP_MENU)
endif()

target_sources(app PRIVATE
    src/application/main/Main.cc
    src/use_case/alif_asr/src/MainLoop.cc
	src/use_case/alif_asr/src/UseCaseHandlerTflm.cc
	src/generated/asr/Labels.cc
	src/power/power_mgr.c
	../alif_common/audio/AudioBackend.cpp
)

set(SRC_MAIN "src/application/main/Main.cc")

list(APPEND SRC_MAIN_DEFS
    "PRJ_VER_STR=\"${CMAKE_PROJECT_VERSION}\""
    "PRJ_DES_STR=\"${CMAKE_PROJECT_DESCRIPTION}\""
    "PRJ_COMPILER=\"${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}\"")

set_source_files_properties(${SRC_MAIN}
    PROPERTIES COMPILE_DEFINITIONS
    "${SRC_MAIN_DEFS}")


if (CONFIG_ARM_ETHOS_U55_256)
target_sources(app PRIVATE src/generated/asr/wav2letter_pruned_int8_vela_H256.tflite.cc)
else()
target_sources(app PRIVATE src/generated/asr/wav2letter_pruned_int8_vela_H128.tflite.cc)
endif()

target_link_libraries(app PRIVATE
	asr_iface
    arm_math
    common_api
    asr_api
	ml_framework_tflm
)
