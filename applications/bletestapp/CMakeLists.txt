# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ble_testapp)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../samples/bluetooth/common/CMakeLists.txt)

# Include directories
target_include_directories(app PRIVATE
	${COMMON_INC}
)

target_sources(app PRIVATE
    src/main.c
    src/appl_shell.c
    src/power_mgr.c
    src/ble_handler.c
    src/power_shell.c
    ${COMMON_SRCS}
)
