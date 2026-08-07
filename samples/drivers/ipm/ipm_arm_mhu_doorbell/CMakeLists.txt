# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mhuv2)

target_include_directories(app PRIVATE include)

target_sources_ifdef(CONFIG_APSS_R   app PRIVATE src/m55_to_a32_responder.c)
target_sources_ifdef(CONFIG_HE_HP_S  app PRIVATE src/m55he_to_m55hp_initiator.c)
target_sources_ifdef(CONFIG_HP_HE_R  app PRIVATE src/m55hp_to_m55he_responder.c)
target_sources_ifdef(CONFIG_HE_A32_S app PRIVATE src/m55he_to_a32_initiator.c)
