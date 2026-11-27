zephyr_include_directories(
  plf
)

zephyr_sources(
  plf/alif_ble.c
  plf/hci_uart.c
  plf/host_timer_kernel.c
  plf/sync_timer.c
)

if(CONFIG_ALIF_BLE_ROM_API_EXTERNAL)
  # An out-of-tree module supplies the BLE ROM API: the public headers, the ROM
  # symbol-address linker script and the host-stack patch. Expose the generic
  # patch section placement scripts from this HAL directory so the provider only
  # has to ship its headers, symbol addresses and patch ELF.
  set(ALIF_BLE_ROM_PATCH_LD_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  string(CONFIGURE "${CONFIG_ALIF_BLE_ROM_API_DIR}" ALIF_BLE_ROM_API_DIR)
  add_subdirectory(${ALIF_BLE_ROM_API_DIR} ble_rom_api_ext)
else()
  add_subdirectory_ifdef(CONFIG_ALIF_BLE_ROM_IMAGE_V1_2 v1_2)
endif()
