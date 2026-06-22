# SPDX-FileCopyrightText: Copyright Alif Semiconductor
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lvgl_dave2d_benchmark)

set(LVGL_DIR ${ZEPHYR_LVGL_MODULE_DIR})

target_sources(app PRIVATE src/main.c)

target_include_directories(app PRIVATE ${LVGL_DIR}/demos/)

# LVGL's upstream D/AVE 2D draw unit was written for the Renesas FSP and pulls in
# "hal_data.h". The include/ directory provides a compatibility shim that maps
# the TES D/AVE 2D driver headers. It must be visible to the LVGL module library
# build (not just the app), so it is added to the global include path.
zephyr_include_directories(include)

# The upstream draw unit gates its D-cache maintenance (clean/invalidate of the
# GPU source and destination buffers) behind the Renesas FSP macros
# RENESAS_CORTEX_M85 and BSP_CFG_DCACHE_ENABLED. The guarded code itself is
# portable (CMSIS SCB_* and the Alif d1_cacheblockflush() helper). The Alif
# Cortex-M55 runs with D-cache enabled and the GPU buffers live in cacheable
# SRAM, so this maintenance is required for CPU/GPU coherency. Define the macros
# for the whole build (the LVGL module needs them too, not just the app).
zephyr_compile_definitions(RENESAS_CORTEX_M85=1 BSP_CFG_DCACHE_ENABLED=1)

# lv_demo_benchmark cycles through the widgets demo scenes, so both the
# benchmark and widgets demo sources are required.
target_sources_ifdef(CONFIG_LV_USE_DEMO_BENCHMARK app PRIVATE
    ${LVGL_DIR}/demos/benchmark/lv_demo_benchmark.c
    ${LVGL_DIR}/demos/benchmark/assets/img_benchmark_avatar.c
    ${LVGL_DIR}/demos/benchmark/assets/img_benchmark_lvgl_logo_argb.c
    ${LVGL_DIR}/demos/benchmark/assets/img_benchmark_lvgl_logo_rgb.c
    ${LVGL_DIR}/demos/benchmark/assets/lv_font_benchmark_montserrat_12_compr_az.c.c
    ${LVGL_DIR}/demos/benchmark/assets/lv_font_benchmark_montserrat_16_compr_az.c.c
    ${LVGL_DIR}/demos/benchmark/assets/lv_font_benchmark_montserrat_28_compr_az.c.c
)

target_sources_ifdef(CONFIG_LV_USE_DEMO_WIDGETS app PRIVATE
    ${LVGL_DIR}/demos/widgets/lv_demo_widgets.c
    ${LVGL_DIR}/demos/widgets/assets/img_clothes.c
    ${LVGL_DIR}/demos/widgets/assets/img_demo_widgets_avatar.c
    ${LVGL_DIR}/demos/widgets/assets/img_demo_widgets_needle.c
    ${LVGL_DIR}/demos/widgets/assets/img_lvgl_logo.c
)

# The LVGL rendering buffers (.lvgl_buf) and memory pool (.lvgl_heap) are
# rendered into by the D/AVE 2D GPU as a bus master, so relocate them into
# globally addressable SRAM0. In DIRECT mode these full-screen VDBs also
# serve as the CDC200 scan-out framebuffers (zero-copy page-flip).
zephyr_linker_sources_ifdef(CONFIG_LV_Z_VDB_CUSTOM_SECTION
    SECTIONS lvgl_sram.ld)
