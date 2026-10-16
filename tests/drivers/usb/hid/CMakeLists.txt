# Copyright Alif Semiconductor - All Rights Reserved.

cmake_minimum_required(VERSION 3.20.0)

# One suite per image. -DCONFIG_HID_TEST_* must be set before find_package
# so the matching overlay and product string are applied.
if(CONFIG_HID_TEST_KEYBOARD)
  list(APPEND EXTRA_DTC_OVERLAY_FILE
       "${CMAKE_CURRENT_LIST_DIR}/overlay-keyboard.overlay")
  list(APPEND EXTRA_CONF_FILE
       "${CMAKE_CURRENT_LIST_DIR}/extras-keyboard.conf")
elseif(CONFIG_HID_TEST_JOYSTICK)
  list(APPEND EXTRA_DTC_OVERLAY_FILE
       "${CMAKE_CURRENT_LIST_DIR}/overlay-joystick.overlay")
  list(APPEND EXTRA_CONF_FILE
       "${CMAKE_CURRENT_LIST_DIR}/extras-joystick.conf")
  if(BOARD MATCHES "alif_e[78]_dk")
    list(APPEND EXTRA_DTC_OVERLAY_FILE
         "${CMAKE_CURRENT_LIST_DIR}/boards/alif_ensemble_dk.overlay")
  elseif(BOARD MATCHES "alif_(e1c|b1)_dk")
    list(APPEND EXTRA_DTC_OVERLAY_FILE
         "${CMAKE_CURRENT_LIST_DIR}/boards/alif_balletto_dk.overlay")
  endif()
else()
  list(APPEND EXTRA_DTC_OVERLAY_FILE
       "${CMAKE_CURRENT_LIST_DIR}/overlay-mouse.overlay")
  list(APPEND EXTRA_CONF_FILE
       "${CMAKE_CURRENT_LIST_DIR}/extras-mouse.conf")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_usb_hid_qa)

if(CONFIG_HID_TEST_KEYBOARD)
  target_sources(app PRIVATE src/test_hid_keyboard.c)
elseif(CONFIG_HID_TEST_JOYSTICK)
  target_sources(app PRIVATE src/test_hid_joystick.c)
else()
  target_sources(app PRIVATE src/test_hid_mouse.c)
endif()

include(${ZEPHYR_BASE}/samples/subsys/usb/common/common.cmake)
