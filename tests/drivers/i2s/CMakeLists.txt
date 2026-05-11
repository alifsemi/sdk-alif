# Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(i2s_alif)

# Common sources (always included)
target_sources(app PRIVATE
    src/common/i2s_test_common.c
    src/common/i2s_golden_vectors.c
)

target_include_directories(app PRIVATE src/common src/states src/negative)

# Functional tests
if(CONFIG_I2S_FUNCTIONAL_TESTS)
    if(CONFIG_I2S_FEATURES_ONLY)
        # Only the i2s_features ZTEST_SUITE. The i2s_alif_functional
        # (lb_matrix_*) and i2s_functional (PlayHello) suites are
        # excluded by not compiling their translation units.
        target_sources(app PRIVATE
            src/functional/test_i2s_features.c
        )
    elseif(CONFIG_I2S_LOOPBACK_ONLY)
        # Only the i2s_alif_functional ZTEST_SUITE (test_lb_matrix_*).
        # The i2s_features and i2s_functional suites are excluded by
        # not compiling their translation units.
        target_sources(app PRIVATE
            src/functional/test_i2s_loopback.c
        )
    else()
        target_sources(app PRIVATE
            src/functional/test_i2s_loopback.c
            src/functional/test_i2s_features.c
            src/functional/test_i2s_playback.c
        )
    endif()
endif()

# Golden vector tests
if(CONFIG_I2S_GOLDEN_TESTS)
    target_sources(app PRIVATE
        src/golden/test_i2s_golden_tx.c
    )
endif()

# Negative tests (includes state-machine negative tests moved from src/states/)
if(CONFIG_I2S_NEGATIVE_TESTS)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/negative/test_i2s_negative.c)
        target_sources(app PRIVATE
            src/negative/test_i2s_negative.c
            src/states/i2s_states_common.c
            src/states/test_i2s_states.c
        )
    endif()
endif()

# Configuration tests
if(CONFIG_I2S_CONFIG_TESTS)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/config/test_i2s_config.c)
        target_sources(app PRIVATE src/config/test_i2s_config.c)
    endif()
endif()
