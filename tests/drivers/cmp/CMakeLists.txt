# Copyright (C) 2024 Alif Semiconductor
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_cmp)

target_sources(app PRIVATE src/main.c)


target_sources(app PRIVATE
	src/test_cmp_basic.c
	src/test_cmp_filter.c
	src/test_cmp_polarity.c
	src/test_cmp_reference.c
	src/test_cmp_feature_ext.c
	src/test_cmp_trigger.c
	src/test_cmp_output.c
	src/test_cmp_negative.c
	src/test_cmp_response_time.c
)
