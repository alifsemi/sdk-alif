# Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
# Use, distribution and modification of this code is permitted under the
# terms stated in the Alif Semiconductor Software License Agreement
#
# You should have received a copy of the Alif Semiconductor Software
# License Agreement with this file. If not, please write to:
# contact@alifsemi.com, or visit: https://alifsemi.com/license

set(COMMON_SRCS
 	${CMAKE_CURRENT_LIST_DIR}/batt_svc.c
 	${CMAKE_CURRENT_LIST_DIR}/address_verification.c
	${CMAKE_CURRENT_LIST_DIR}/gapm_api.c
	${CMAKE_CURRENT_LIST_DIR}/gapm_sec.c
)

zephyr_library_sources_ifdef(CONFIG_GPIO ${CMAKE_CURRENT_LIST_DIR}/ble_gpio.c)
zephyr_library_sources_ifdef(CONFIG_SETTINGS ${CMAKE_CURRENT_LIST_DIR}/ble_storage.c)

if(pm-ble IN_LIST SNIPPET)
    message("BLE PM is enabled, including power_mgr.c in common sources.")
    set(COMMON_SRCS
	${COMMON_SRCS}
	${CMAKE_CURRENT_LIST_DIR}/power_mgr.c
    )
    zephyr_library_compile_options(-DSNIPPET_PM_BLE_USED=1)
endif()

set(COMMON_INC
	${CMAKE_CURRENT_LIST_DIR}
)
