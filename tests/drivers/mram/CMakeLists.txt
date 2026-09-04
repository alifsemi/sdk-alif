# Copyright Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mram)

if(CONFIG_ALIF_MRAM_CORE)
	target_sources(app PRIVATE
		src/functional/test_mram_functional.c
		src/performance/test_mram_performance.c
		src/stress/test_mram_stress.c
	)
endif()

if(CONFIG_ALIF_MRAM_FILESYSTEM)
	target_sources(app PRIVATE
		src/filesystem/test_mram_filesystem.c
	)
endif()
