# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

# Check if CONFIG_IUT_TESTER_ENABLED is set in prj.conf or as a CMake variable
set(IUT_TESTER_ENABLED OFF)

# First, check if it's set as a CMake variable
if(DEFINED CONFIG_IUT_TESTER_ENABLED AND CONFIG_IUT_TESTER_ENABLED)
  set(IUT_TESTER_ENABLED ON)
endif()

# Then, check if it's set in prj.conf
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/prj.conf)
  file(READ ${CMAKE_CURRENT_SOURCE_DIR}/prj.conf PRJ_CONF_CONTENT)
  if(PRJ_CONF_CONTENT MATCHES "CONFIG_IUT_TESTER_ENABLED=y")
    set(IUT_TESTER_ENABLED ON)
  endif()
endif()

# Enable button interface for PTS testing
if(IUT_TESTER_ENABLED)
  set(EXTRA_DTC_OVERLAY_FILE "test_button.overlay")
endif()

# Include BLE Power Management snippet by default
if(NOT IUT_TESTER_ENABLED AND NOT pm-ble IN_LIST SNIPPET)
  set(SNIPPET pm-ble ${SNIPPET} CACHE STRING "" FORCE)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(le_periph_plxp)

target_sources(app PRIVATE
    src/main.c
)
