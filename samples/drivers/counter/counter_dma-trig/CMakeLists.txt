# Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

# One overlay per board family + DMA instance (HE/HP share when mux is the same).
if(BOARD MATCHES "alif_e8_dk" OR BOARD MATCHES "alif_e7_dk")
	set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/boards/lptimer_dma0_e7_e8.overlay)
elseif(BOARD MATCHES "alif_b1_dk" OR BOARD MATCHES "alif_e1c_dk")
	set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/boards/lptimer_dma2_b1_e1c.overlay)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(counter_dma_trig)

target_sources(app PRIVATE src/main.c)
