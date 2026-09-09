# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lvgl_gui)

# The declarative UI in ui/screen_main.xml is exported to C by
# scripts/gen_ui_xml.py. Generate it into the build tree (never the source
# tree) so read-only-source checkouts and parallel twister builds cannot race
# on a shared file. src/ui_xml.h stays in src/ as the stable interface.
set(ui_xml_c ${CMAKE_CURRENT_BINARY_DIR}/ui_xml.c)

# Application sources: the display bring-up and the exported UI.
target_sources(app PRIVATE
    src/main.c
    ${ui_xml_c}
    src/Alif240_white.c
)

# The generated C includes "ui_xml.h", which lives in src/.
target_include_directories(app PRIVATE src)

# Runs only when the XML or the generator changes.
add_custom_command(
    OUTPUT  ${ui_xml_c}
    COMMAND ${PYTHON_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_ui_xml.py
            ${CMAKE_CURRENT_SOURCE_DIR}/ui/screen_main.xml
            ${ui_xml_c}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ui/screen_main.xml
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_ui_xml.py
    COMMENT "Generating ui_xml.c from ui/screen_main.xml"
    VERBATIM
)

# LVGL's upstream D/AVE 2D draw unit includes "bsp_api.h" (LVGL 9.5.0+). The
# include/ directory provides a compatibility shim that maps the TES D/AVE 2D
# driver headers. It must be visible to the LVGL module library build (not just
# the app), so add it to the global include path.
zephyr_include_directories(include)

# The D/AVE 2D draw unit gates its D-cache maintenance (clean/invalidate of the
# GPU source and destination buffers) behind ARM_CORTEX_M55_M85 and
# BSP_CFG_DCACHE_ENABLED. The Alif Cortex-M55 runs with D-cache enabled and the
# GPU buffers live in cacheable SRAM, so this maintenance is required for
# CPU/GPU coherency. Define the macros for the whole build (the LVGL module
# needs them too, not just the app).
zephyr_compile_definitions(ARM_CORTEX_M55_M85=1 BSP_CFG_DCACHE_ENABLED=1)

# The LVGL rendering buffers (.lvgl_buf) and memory pool (.lvgl_heap) are
# rendered into by the D/AVE 2D GPU as a bus master, so relocate them into
# globally addressable SRAM0. In DIRECT mode these full-screen VDBs also serve
# as the CDC200 scan-out framebuffers (zero-copy page-flip).
zephyr_linker_sources_ifdef(CONFIG_LV_Z_VDB_CUSTOM_SECTION
    SECTIONS lvgl_sram.ld)
