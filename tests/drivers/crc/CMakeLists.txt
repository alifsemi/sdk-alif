# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_crc)

# Common sources — always built
target_sources(app PRIVATE
	src/main.c
	src/software_crc.c
)

# Per-algorithm sources — built only when the corresponding Kconfig is enabled
target_sources_ifdef(CONFIG_TEST_CRC_8           app PRIVATE src/test_crc_8.c)
target_sources_ifdef(CONFIG_TEST_CRC_16          app PRIVATE src/test_crc_16.c)
target_sources_ifdef(CONFIG_TEST_CRC_32          app PRIVATE src/test_crc_32.c)
target_sources_ifdef(CONFIG_TEST_CRC_32_CUSTOM_POLY app PRIVATE src/test_crc_32_custom.c)
target_sources_ifdef(CONFIG_TEST_CRC_32C              app PRIVATE src/test_crc_32c.c)
target_sources_ifdef(CONFIG_TEST_CRC_STRESS         app PRIVATE src/test_crc_stress.c)
