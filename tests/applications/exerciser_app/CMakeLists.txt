# Copyright (C) Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(exerciser_app)

target_sources(app PRIVATE src/main.c)
target_sources_ifdef(CONFIG_SHELL app PRIVATE src/exerciser_shell.c)

# Worker sources are compiled only when the matching DT node is okay.

dt_alias(led0_path PROPERTY led0)
if(led0_path)
  dt_node_has_status(blinky_okay PATH "${led0_path}" STATUS okay)
  if(blinky_okay)
    target_sources(app PRIVATE src/blinky.c)
  endif()
endif()

dt_alias(ospi_path PROPERTY spi-flash0)
if(ospi_path)
  dt_node_has_status(ospi_okay PATH "${ospi_path}" STATUS okay)
  if(ospi_okay)
    target_sources(app PRIVATE src/ospi.c)
  endif()
endif()

dt_alias(spi_path PROPERTY master-spi)
if(spi_path)
  dt_node_has_status(spi_okay PATH "${spi_path}" STATUS okay)
  if(spi_okay)
    target_sources(app PRIVATE src/spi.c)
  endif()
endif()

# Nodelabel may be missing on this board; skip status check if unresolved.
dt_nodelabel(sdhc_path NODELABEL sdhc)
if(sdhc_path)
  dt_node_has_status(sd_okay PATH "${sdhc_path}" STATUS okay)
  if(sd_okay)
    target_sources(app PRIVATE src/sd.c)
  endif()
endif()

if(CONFIG_DT_HAS_ZEPHYR_CDC_ACM_UART_ENABLED)
  include(${ZEPHYR_BASE}/samples/subsys/usb/common/common.cmake)
  target_sources(app PRIVATE src/usb.c)
endif()

if(CONFIG_DT_HAS_BOSCH_BMI323_ENABLED)
  target_sources(app PRIVATE src/bmi323.c)
endif()

dt_nodelabel(codec_path NODELABEL audio_codec)
if(codec_path)
  dt_node_has_status(codec_okay PATH "${codec_path}" STATUS okay)
  if(codec_okay)
    target_sources(app PRIVATE src/i2s_codec.c)
  endif()
endif()

# Nodelabel may be missing on this board; skip status check if unresolved.
dt_nodelabel(video_path NODELABEL isp)
if(video_path)
  dt_node_has_status(video_okay PATH "${video_path}" STATUS okay)
  if(video_okay)
    target_sources(app PRIVATE src/video.c src/alif_logo.c)
  endif()
endif()

dt_nodelabel(eth_path NODELABEL ethernet)
if(eth_path)
  dt_node_has_status(eth_okay PATH "${eth_path}" STATUS okay)
  if(eth_okay)
    target_sources(app PRIVATE src/eth.c)
  endif()
endif()
