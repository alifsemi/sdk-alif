# Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

zephyr_library()
zephyr_include_directories(
    .
    ch201_lib
    ch201_lib/invn/soniclib
    ch201_lib/invn/soniclib/details
    ch201_lib/invn/soniclib/sensor_fw/ch101
    ch201_lib/invn/soniclib/sensor_fw/ch201
)
zephyr_library_sources(ch201.c
		       ch201_private.c)

zephyr_library_sources(
    # Core SonicLib Drivers
    ch201_lib/invn/soniclib/ch_api.c
    ch201_lib/invn/soniclib/ch_common.c
    ch201_lib/invn/soniclib/ch_driver.c
    ch201_lib/invn/soniclib/ch_log.c
    ch201_lib/invn/soniclib/ch_math_utils.c
    ch201_lib/invn/soniclib/ch_rangefinder.c
    ch201_lib/invn/soniclib/chbsp_dummy.c

    # Firmware for CH201
    ch201_lib/invn/soniclib/sensor_fw/ch201/_ch201_gprmt_plugin_version.c
    ch201_lib/invn/soniclib/sensor_fw/ch201/ch201_gprmt.c
    ch201_lib/invn/soniclib/sensor_fw/ch201/ch201_gprmt_fw.c
# Optional Utilities
    ch201_lib/invn/soniclib/extra/ch_extra_display_utils.c
)



