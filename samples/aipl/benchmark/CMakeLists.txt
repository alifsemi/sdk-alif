# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(DTC_OVERLAY_FILE
    "boards/alif_dave2d.overlay"
    "boards/alif_serial_display.overlay"
    "boards/alif_utimer.overlay"
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(aipl_example)

# Add include directories
zephyr_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/assets
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/display
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu_usage/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fps_counter/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/objects/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/perf_tests/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utimer/)

# Add app sources
target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/cache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/video_alloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/display/display.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpu_usage/cpu_usage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fps_counter/fps_counter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/objects/image.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/objects/rectangle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/graphics.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/perf_tests/color_conversion_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/perf_tests/color_correction_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/perf_tests/cropping_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/perf_tests/draw_object_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/perf_tests/flipping_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/perf_tests/lut_transform_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/perf_tests/resize_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/perf_tests/rotation_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/perf_tests/white_balance_test.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utimer/utmr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
)

if(CONFIG_AIPL_PREBUILT)
    add_library(aipl_wrapper STATIC IMPORTED GLOBAL)
    add_dependencies(app aipl_wrapper)

    set_target_properties(aipl_wrapper PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/libaipl.a)
    set_target_properties(aipl_wrapper PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/include;${CMAKE_CURRENT_SOURCE_DIR}/lib")

    target_link_libraries(app PUBLIC aipl_wrapper)
else()
    if(NOT CONFIG_AIPL_DAVE2D_ACCELERATION)
        # Add D/AVE2D functions from directly to be able to use it
        target_sources(app PRIVATE ${ZEPHYR_AIPL_MODULE_DIR}/source/aipl_dave2d.c)
    endif()
endif()

# Relocate the code to ITCM
# The library names are relative paths from ${ZEPHYR_BASE} to CMakeLists.txt files
# where '/' are replaced with '__'
if(CONFIG_AIPL)
    zephyr_code_relocate(LIBRARY ..__modules__lib__aipl__zephyr LOCATION ITCM_TEXT)
endif()
zephyr_code_relocate(LIBRARY ..__modules__drivers__dave2d__zephyr LOCATION ITCM_TEXT)
zephyr_code_relocate(LIBRARY ..__modules__hal__alif__drivers__dave2d LOCATION ITCM_TEXT)
zephyr_code_relocate(FILES $<TARGET_PROPERTY:app,SOURCES> LOCATION ITCM_TEXT)

# Add asset files to the app
target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/assets/sample_photo_alpha8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/assets/sample_photo_argb1555.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/assets/sample_photo_argb4444.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/assets/sample_photo_argb8888.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/assets/sample_photo_rgb565.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/assets/sample_photo_rgb888.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/assets/sample_photo_rgba4444.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/assets/sample_photo_rgba5551.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/assets/sample_photo_rgba8888.c
)

set(EXPORT_COMPILE_COMMANDS on)
