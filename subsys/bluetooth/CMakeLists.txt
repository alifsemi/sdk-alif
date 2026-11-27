# Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

set(SDK_ALIF_BLE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to sdk-alif BLE root")

add_subdirectory_ifdef(CONFIG_ALIF_ROM_LC3_CODEC lc3)
add_subdirectory_ifdef(CONFIG_ALIF_BLE_HOST ble)

add_subdirectory_ifdef(CONFIG_ALIF_BLE_AUDIO le_audio)
add_subdirectory_ifdef(CONFIG_ALIF_BT_SHELL shell)

add_subdirectory_ifdef(CONFIG_ALIF_BLE_HOST host)
add_subdirectory_ifdef(CONFIG_ALIF_BLE_HOST common)
