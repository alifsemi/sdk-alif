# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
option(SERIAL "Enable for serial display" OFF)
if(SERIAL)
set(DTC_OVERLAY_FILE "boards/alif_serial_display.overlay")
set(OVERLAY_CONFIG "boards/alif_serial_display.conf")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(display)

target_sources(app PRIVATE src/main.c)
