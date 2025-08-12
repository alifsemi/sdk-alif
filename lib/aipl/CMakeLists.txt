# Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license
#

if (CONFIG_AIPL)
  zephyr_interface_library_named(AIPL)
  zephyr_library_named(aipl)
  zephyr_include_directories(
    .
    include
    include/default
    include/dave2d
    include/helium
  )

  zephyr_library_sources(
    source/aipl_color_formats.c
    source/aipl_image.c
    source/aipl_error.c
  )

  zephyr_library_sources_ifdef(
    CONFIG_AIPL_DAVE2D_ACCELERATION
    source/aipl_dave2d.c
  )

  zephyr_library_sources_ifdef(
    CONFIG_AIPL_BASIC
    source/aipl_crop.c
    source/dave2d/aipl_crop_dave2d.c
    source/default/aipl_crop_default.c
    source/aipl_flip.c
    source/dave2d/aipl_flip_dave2d.c
    source/default/aipl_flip_default.c
    source/helium/aipl_flip_helium.c
    source/aipl_resize.c
    source/dave2d/aipl_resize_dave2d.c
    source/default/aipl_resize_default.c
    source/helium/aipl_resize_helium.c
    source/aipl_rotate.c
    source/dave2d/aipl_rotate_dave2d.c
    source/default/aipl_rotate_default.c
    source/helium/aipl_rotate_helium.c
  )

  zephyr_library_sources_ifdef(
    CONFIG_AIPL_COLOR_CORRECTION
    source/aipl_color_correction.c
    source/default/aipl_color_correction_default.c
    source/helium/aipl_color_correction_helium.c
    source/aipl_white_balance.c
    source/default/aipl_white_balance_default.c
    source/helium/aipl_white_balance_helium.c
    source/aipl_lut_transform.c
    source/default/aipl_lut_transform_default.c
    source/helium/aipl_lut_transform_helium.c
  )

  zephyr_library_sources_ifdef(
    CONFIG_AIPL_COLOR_CONVERSION
    source/aipl_color_conversion.c
    source/dave2d/aipl_color_conversion_dave2d.c
    source/helium/aipl_color_conversion_helium.c
    source/default/aipl_color_conversion_default.c
  )

  zephyr_library_sources_ifdef(
    CONFIG_AIPL_BAYER
    source/aipl_bayer.c
    source/default/aipl_bayer_default.c
    source/helium/aipl_bayer_helium.c
  )
endif()
