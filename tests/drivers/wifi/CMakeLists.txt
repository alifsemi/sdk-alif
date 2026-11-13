# Copyright Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement.

cmake_minimum_required(VERSION 3.20.0)

# E8 DK overlay (gpio14.7 WL_REG_ON) is not valid on E7.
if("${BOARD}" MATCHES "^alif_e7_dk" AND
   "${DTC_OVERLAY_FILE}" MATCHES "alif_e8_dk")
  set(DTC_OVERLAY_FILE
      "${CMAKE_CURRENT_LIST_DIR}/boards/alif_e7_dk_ae722f80f55d5xx_rtss_hp.overlay"
      CACHE STRING "Devicetree overlay" FORCE)
  message(WARNING "alif_e7_dk cannot use the E8 wifi overlay; "
          "switched to alif_e7_dk_ae722f80f55d5xx_rtss_hp.overlay")
endif()

# Suite extras when selected via -DCONFIG_WIFI_TEST_*=y (no overlay .conf).
if(CONFIG_WIFI_TEST_P2P)
  list(APPEND EXTRA_CONF_FILE "${CMAKE_CURRENT_LIST_DIR}/extras-p2p.conf")
endif()
if(CONFIG_WIFI_TEST_ZPERF)
  list(APPEND EXTRA_CONF_FILE "${CMAKE_CURRENT_LIST_DIR}/extras-zperf.conf")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_wifi_qa)

if(CONFIG_WIFI_TEST_AP)
  target_sources(app PRIVATE src/test_wifi_ap.c)
elseif(CONFIG_WIFI_TEST_P2P)
  target_sources(app PRIVATE src/test_wifi_p2p.c)
elseif(CONFIG_WIFI_TEST_ZPERF)
  target_sources(app PRIVATE src/test_wifi_zperf.c)
else()
  target_sources(app PRIVATE src/test_wifi_shell.c)
endif()
