# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lvgl_dave2d_test)

target_sources(app PRIVATE src/main.c)

# LVGL's D/AVE 2D draw unit includes bsp_api.h. The shim has to be visible to
# the LVGL library build, not only this test.
zephyr_include_directories(include)
zephyr_compile_definitions(ARM_CORTEX_M55_M85=1 BSP_CFG_DCACHE_ENABLED=1)

zephyr_linker_sources_ifdef(CONFIG_LV_Z_VDB_CUSTOM_SECTION
    SECTIONS lvgl_sram.ld)
