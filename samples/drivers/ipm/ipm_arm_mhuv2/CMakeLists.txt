# Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

if("${BOARD}" STREQUAL "alif_e3_dk/ae302f80f55d5xx/rtss_he" OR
	"${BOARD}" STREQUAL "alif_e4_dk/ae402fa0e5597xx0/rtss_he" OR
	"${BOARD}" STREQUAL "alif_e7_dk/ae722f80f55d5xx/rtss_he"  OR
	"${BOARD}" STREQUAL "alif_e1c_dk/ae1c1f4051920hh/rtss_he"  OR
	"${BOARD}" STREQUAL "alif_b1_dk/ab1c1f4m51820hh/rtss_he"  OR
	"${BOARD}" STREQUAL "alif_e8_dk/ae822fa0e5597xx0/rtss_he")
        add_subdirectory(rtss_he)
elseif("${BOARD}" STREQUAL "alif_e3_dk/ae302f80f55d5xx/rtss_hp"  OR
	"${BOARD}" STREQUAL "alif_e4_dk/ae402fa0e5597xx0/rtss_hp" OR
	"${BOARD}" STREQUAL "alif_e7_dk/ae722f80f55d5xx/rtss_hp"  OR
	"${BOARD}" STREQUAL "alif_e8_dk/ae822fa0e5597xx0/rtss_hp")
        add_subdirectory(rtss_hp)
endif()

project(mhuv2)
