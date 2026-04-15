# Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

# # Include BLE Power Management snippet by default
if(NOT pm-ble IN_LIST SNIPPET)
  set(SNIPPET pm-ble ${SNIPPET} CACHE STRING "" FORCE)
  set(EXTRA_DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/boards/lpgpio.overlay)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(le_periph_mcs)

target_sources(app PRIVATE
	src/main.c
)
