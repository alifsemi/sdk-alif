# Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

if(NOT ("${BOARD}" STREQUAL "alif_e7_ak_apss" OR "${BOARD}" STREQUAL "alif_e7_dk_apss"))
	message(FATAL_ERROR "${BOARD} was not supported for this sample")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mhuv2)

option(SE_SERVICES "Build SE services test case" ON)
option(RTSS_HE_MHU0 "Build RTSS-HE (CM55) MHU0 test case" OFF)
option(RTSS_HE_MHU1 "Build RTSS-HE (CM55) MHU1 test case" OFF)
option(RTSS_HP_MHU0 "Build RTSS-HP (CM55) MHU0 test case" OFF)
option(RTSS_HP_MHU1 "Build RTSS-HP (CM55) MHU1 test case" OFF)
if(RTSS_HE_MHU0)
	target_sources(app PRIVATE src/for_rtss_he_mhu0.c)
elseif(RTSS_HE_MHU1)
	target_sources(app PRIVATE src/for_rtss_he_mhu1.c)
elseif(RTSS_HP_MHU0)
	target_sources(app PRIVATE src/for_rtss_hp_mhu0.c)
elseif(RTSS_HP_MHU1)
	target_sources(app PRIVATE src/for_rtss_hp_mhu1.c)
elseif(SE_SERVICES)
	target_sources(app PRIVATE src/se_service_test.c)
endif()
