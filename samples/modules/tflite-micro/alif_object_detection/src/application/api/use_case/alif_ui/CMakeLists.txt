# SPDX-FileCopyrightText: Copyright Alif Semiconductor
# SPDX-License-Identifier: Apache-2.0

#########################################################
#                   ALIF UI API library                 #
#########################################################
cmake_minimum_required(VERSION 3.21.0)

set(ALIF_UI_API_TARGET alif_ui_api)
project(${ALIF_UI_API_TARGET}
        DESCRIPTION     "Alif UI API library"
        LANGUAGES       C CXX)

# Create static library
add_library(${ALIF_UI_API_TARGET} STATIC
        src/lv_paint_utils.c
        src/ScreenLayout.cc
        src/Alif240.c
        src/Alif240_white.c)

target_include_directories(${ALIF_UI_API_TARGET} PUBLIC include)

target_link_libraries(${ALIF_UI_API_TARGET} PUBLIC common_api PRIVATE zephyr_interface)

message(STATUS "*******************************************************")
message(STATUS "Library                                : " ${ALIF_UI_API_TARGET})
message(STATUS "CMAKE_SYSTEM_PROCESSOR                 : " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "*******************************************************")
