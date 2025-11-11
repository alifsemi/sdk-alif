# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

set(sources
    auracast_sink.c
)

target_sources(app PRIVATE ${sources})

target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../src)
