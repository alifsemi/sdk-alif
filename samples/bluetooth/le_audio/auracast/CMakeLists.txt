# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(le_audio_auracast)

target_sources(app PRIVATE
    src/main.c
    src/audio_datapath.c
    src/mic_source.c
    src/auracast_shell.c
)

add_subdirectory(auracast_source/)
add_subdirectory(auracast_sink/)
add_subdirectory(auracast_scan_delegator/)
