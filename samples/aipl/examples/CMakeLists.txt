# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(DTC_OVERLAY_FILE
    "boards/alif_dave2d.overlay"
    "boards/alif_serial_display.overlay"
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(aipl_example)

# Add include directories
zephyr_include_directories(
    assets
    src/aipl/
    src/display/
    src/graphics/
)

# Add app sources
target_sources(app PRIVATE
    src/aipl/cache.c
    src/aipl/video_alloc.c
    src/display/display.c
    src/graphics/image.c
    src/main.c
)

if (NOT CONFIG_AIPL_DAVE2D_ACCELERATION)
    # Add D/AVE2D functions from directly to be able to use it
    target_sources(app PRIVATE ${ZEPHYR_AIPL_MODULE_DIR}/source/aipl_dave2d.c)
endif()

# Add asset files to the app
target_sources(app PRIVATE
    assets/sample_photo_alpha8.c
    assets/sample_photo_argb1555.c
    assets/sample_photo_argb4444.c
    assets/sample_photo_argb8888.c
    assets/sample_photo_rgb565.c
    assets/sample_photo_rgb888.c
    assets/sample_photo_rgba4444.c
    assets/sample_photo_rgba5551.c
    assets/sample_photo_rgba8888.c
)
