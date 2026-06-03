# Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mhuv2)

option(APSS_R "Build APSS (A32) shared memory&doorbell test case responder" OFF)
option(HE_HP_S "Build RTSS-HE (M55) to RTSS-HP (M55) shared memory&doorbell test case initiator/sender" OFF)
option(HP_HE_R "Build RTSS-HP (M55) to RTSS-HE (M55) shared memory&doorbell test case responder" OFF)
option(HE_A32_S "Build RTSS-HE (M55) to APSS (A32) shared memory&doorbell test case initiator/sender" OFF)

if(APSS_R)
	target_sources(app PRIVATE src/m55_to_a32_responder.c)
elseif(HE_HP_S)
	target_sources(app PRIVATE src/m55he_to_m55hp_initiator.c)
elseif(HP_HE_R)
	target_sources(app PRIVATE src/m55hp_to_m55he_responder.c)
elseif(HE_A32_S)
	target_sources(app PRIVATE src/m55he_to_a32_initiator.c)
else()
    target_sources(app PRIVATE src/m55_to_a32_responder.c)
endif()
