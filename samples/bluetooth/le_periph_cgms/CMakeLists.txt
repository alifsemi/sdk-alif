# Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

# Define dummy default value for skip tester
if(NOT DEFINED CONFIG_IUT_TESTER_ENABLED)
  set(CONFIG_IUT_TESTER_ENABLED false)
endif(NOT DEFINED CONFIG_IUT_TESTER_ENABLED)

# Enable button interface for PTS testing
if (CONFIG_IUT_TESTER_ENABLED)
  set(EXTRA_DTC_OVERLAY_FILE "test_button.overlay")
endif()


# Include BLE Power Management snippet by default
if(NOT CONFIG_IUT_TESTER_ENABLED AND NOT pm-ble IN_LIST SNIPPET)
  set(SNIPPET pm-ble ${SNIPPET} CACHE STRING "" FORCE)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(le_periph_cgms)

# Include directories
target_include_directories(app PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_sources(app PRIVATE
	src/main.c
	src/cgms_rom_1_2.c
)

