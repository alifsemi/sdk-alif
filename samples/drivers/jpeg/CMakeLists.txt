# Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(jpeg_test)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
target_compile_definitions(app PRIVATE INCBIN_BASE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
