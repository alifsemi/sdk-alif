# Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license


zephyr_library_sources(batt_svc.c)
zephyr_library_sources(address_verification.c)
zephyr_library_sources(gapm_api.c)
zephyr_library_sources(gapm_sec.c)
zephyr_library_sources(batt_cli.c)
zephyr_library_sources(central_itf.c)

zephyr_library_sources_ifdef(CONFIG_GPIO ble_gpio.c)
zephyr_library_sources_ifdef(CONFIG_SETTINGS ble_storage.c)

if(pm-ble IN_LIST SNIPPET)
    message("BLE PM is enabled, including power_mgr.c in common sources.")
    zephyr_library_sources(power_mgr.c)
    zephyr_library_compile_options(-DSNIPPET_PM_BLE_USED=1)
endif()

zephyr_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
)
