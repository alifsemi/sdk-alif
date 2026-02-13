# Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(alif_object_detection)

# Silence "changed in GCC 7.1" warnigns from C++ headers
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-psabi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

# Add include directories
zephyr_include_directories(
	src/aipl/
)

add_subdirectory(src/math)
add_subdirectory(src/application/api/common)
add_subdirectory(src/application/api/use_case/object_detection)
add_subdirectory(src/application/api/use_case/alif_ui)

target_include_directories(app PRIVATE
    src/application/main/include
    src/use_case/object_detection/include
    src/application/api/use_case/object_detection/include
    src/generated/object_detection
    src/application/api/use_case/alif_ui/include
)

target_compile_definitions(app PRIVATE ACTIVATION_BUF_SZ=CONFIG_ACTIVATION_BUF_SZ)

target_sources(app PRIVATE
    src/application/main/Main.cc
    src/use_case/object_detection/src/MainLoop.cc
    src/use_case/object_detection/src/UseCaseHandler.cc
    src/use_case/object_detection/src/image_ensemble.c
    src/use_case/object_detection/src/image_processing.c
)

if (CONFIG_ARM_ETHOS_U55_256)
target_sources(app PRIVATE src/generated/object_detection/yolo-fastest_192_face_v4_vela_H256.tflite.cc)
else()
target_sources(app PRIVATE src/generated/object_detection/yolo-fastest_192_face_v4_vela_H128.tflite.cc)
endif()

target_link_libraries(app PRIVATE
    arm_math
    common_api
    object_detection_api
    alif_ui_api
)
