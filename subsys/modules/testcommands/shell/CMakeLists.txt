# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

include(git)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(testcommands_shell)

target_sources(app PRIVATE
		src/testcommands_shell.c
		src/memtestcommands_shell.c
)

git_describe(${APPLICATION_SOURCE_DIR} app_version)
target_compile_definitions(app PRIVATE APP_VERSION="${app_version}")
