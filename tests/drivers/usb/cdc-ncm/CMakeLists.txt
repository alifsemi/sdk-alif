# Copyright Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement.

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_usb_cdc_ncm)

target_sources(app PRIVATE src/test_cdc_ncm.c)
include(${ZEPHYR_BASE}/samples/subsys/usb/common/common.cmake)
