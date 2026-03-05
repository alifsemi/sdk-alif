cmake_minimum_required(VERSION 3.20.0)

if(NOT DEFINED DTC_OVERLAY_FILE)
  set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_LIST_DIR}/boards/alif_uart.overlay)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_uart)

target_sources(app PRIVATE
    src/main.c
)

target_sources_ifdef(CONFIG_TEST_INTERNAL_LB app PRIVATE src/uart_internal_loopback.c)
target_sources_ifdef(CONFIG_TEST_EXTERNAL_LB app PRIVATE src/uart_external_loopback.c)
target_sources_ifdef(CONFIG_TEST_UART_RTSCTS app PRIVATE src/uart_internal_loopback.c)
