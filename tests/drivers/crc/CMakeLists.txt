# Copyright Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_crc)

target_sources(app PRIVATE
	src/main.c
	src/software_crc.c
)

target_sources_ifdef(CONFIG_TEST_CRC_8 app PRIVATE src/test_crc_8.c)
target_sources_ifdef(CONFIG_TEST_CRC_16 app PRIVATE src/test_crc_16.c)
target_sources_ifdef(CONFIG_TEST_CRC_16_ALT app PRIVATE src/test_crc_16_alt.c)
target_sources_ifdef(CONFIG_TEST_CRC_32 app PRIVATE src/test_crc_32.c)
target_sources_ifdef(CONFIG_TEST_CRC_32C app PRIVATE src/test_crc_32c.c)
target_sources_ifdef(CONFIG_TEST_CRC_STRESS app PRIVATE
		     src/test_crc_stress.c)
