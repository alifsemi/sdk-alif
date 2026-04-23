# Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(i2s_alif)

# Common sources (always included)
target_sources(app PRIVATE
    src/common/i2s_test_common.c
)

# Golden vectors: needed for loopback and golden-vector test builds
if(CONFIG_I2S_LOOPBACK OR CONFIG_I2S_GOLDEN_TESTS)
    target_sources(app PRIVATE
        src/common/i2s_golden_vectors.c
    )
endif()

target_include_directories(app PRIVATE src/common src/negative)

# Set when any functional-only flag is active — used to gate non-functional suites
set(I2S_FUNCTIONAL FALSE)
if(CONFIG_I2S_FUNCTIONAL_TESTS)
    if(CONFIG_I2S_FEATURES OR CONFIG_I2S_LOOPBACK OR
       CONFIG_I2S_PLAY_HELLO OR CONFIG_I2S_PLAY_HELLO_CODEC OR
       CONFIG_I2S_FULL_DUPLEX)
        set(I2S_FUNCTIONAL TRUE)
    endif()
endif()

# Functional tests
if(CONFIG_I2S_FUNCTIONAL_TESTS)
    if(CONFIG_I2S_FEATURES)
        # Only the i2s_features ZTEST_SUITE. The i2s_alif_functional
        # (lb_matrix_*) and i2s_functional (PlayHello) suites are
        # excluded by not compiling their translation units.
        target_sources(app PRIVATE
            src/functional/test_i2s_features.c
        )
    elseif(CONFIG_I2S_LOOPBACK)
        # Only the i2s_alif_functional ZTEST_SUITE (test_lb_matrix_*).
        # The i2s_features and i2s_functional suites are excluded by
        # not compiling their translation units.
        target_sources(app PRIVATE
            src/functional/test_i2s_loopback.c
        )
    elseif(CONFIG_I2S_PLAY_HELLO OR CONFIG_I2S_PLAY_HELLO_CODEC)
        # Only the i2s_functional ZTEST_SUITE (test_i2s_playback.c).
        # The i2s_alif_functional (lb_matrix_*) and i2s_features suites
        # are excluded by not compiling their translation units.
        # Non-functional suites (golden, negative, config) are unaffected
        # and still controlled by their own master switches below.
        target_sources(app PRIVATE
            src/functional/test_i2s_playback.c
        )
    elseif(CONFIG_I2S_FULL_DUPLEX)
        # Only test_full_duplex_same_device. Compiling test_i2s_features.c
        # with CONFIG_I2S_FULL_DUPLEX set #ifndef-excludes the matrix tests,
        # leaving only the full-duplex test. The i2s_alif_functional
        # (lb_matrix_*) and i2s_functional (PlayHello) suites are excluded
        # by not compiling their translation units.
        target_sources(app PRIVATE
            src/functional/test_i2s_features.c
        )
    else()
        target_sources(app PRIVATE
            src/functional/test_i2s_loopback.c
            src/functional/test_i2s_features.c
            src/functional/test_i2s_playback.c
        )
    endif()
endif()

# Golden vector tests — excluded when any functional-only config is active
if(CONFIG_I2S_GOLDEN_TESTS AND NOT I2S_FUNCTIONAL)
    target_sources(app PRIVATE
        src/golden/test_i2s_golden_tx.c
    )
endif()

# Negative tests — excluded when any functional-only config is active
if(CONFIG_I2S_NEGATIVE_TESTS AND NOT I2S_FUNCTIONAL)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/negative/test_i2s_negative.c)
        target_sources(app PRIVATE
            src/negative/test_i2s_negative.c
        )
    endif()
endif()

# Configuration tests — excluded when any functional-only config is active
if(CONFIG_I2S_CONFIG_TESTS AND NOT I2S_FUNCTIONAL)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/config/test_i2s_config.c)
        target_sources(app PRIVATE src/config/test_i2s_config.c)
    endif()
endif()
