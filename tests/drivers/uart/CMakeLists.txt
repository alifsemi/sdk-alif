cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_uart)

target_sources(app PRIVATE
    src/main.c
)

# Include uart_internal_loopback.c for either internal loopback or RTS/CTS tests
if(CONFIG_TEST_INTERNAL_LB OR CONFIG_TEST_UART_RTSCTS)
  target_sources(app PRIVATE src/uart_internal_loopback.c)
endif()

target_sources_ifdef(CONFIG_TEST_EXTERNAL_LB app PRIVATE src/uart_external_loopback.c)
