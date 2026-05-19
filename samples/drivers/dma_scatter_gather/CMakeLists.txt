# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

# Apply the alif-dma snippet automatically for manual builds.
# Twister/CI builds use extra_args: -S alif-dma in sample.yaml instead.
if(NOT alif-dma IN_LIST SNIPPET)
  set(SNIPPET alif-dma ${SNIPPET} CACHE STRING "" FORCE)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dma_scatter_gather)

target_sources(app PRIVATE src/main.c)