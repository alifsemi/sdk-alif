cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(uart_tests)

target_sources(app PRIVATE
    src/main.c
)

# RTS/CTS lives in uart_internal_loopback.c (guarded by CONFIG_TEST_UART_RTSCTS).
# Compile that file when either the internal-loopback or the RTS/CTS suite
# is enabled so uart_RTSCTS_suite is not registered with zero tests.
if(CONFIG_TEST_INTERNAL_LB OR CONFIG_TEST_UART_RTSCTS)
  target_sources(app PRIVATE src/uart_internal_loopback.c)
endif()

target_sources_ifdef(CONFIG_TEST_EXTERNAL_LB app PRIVATE src/uart_external_loopback.c)

target_sources_ifdef(CONFIG_TEST_UART_DMA app PRIVATE src/uart_dma_tests.c)

target_sources_ifdef(CONFIG_TEST_STRESS app PRIVATE src/uart_stress.c)

target_sources_ifdef(CONFIG_TEST_PERFORMANCE app PRIVATE src/uart_perf.c)

target_sources_ifdef(CONFIG_TEST_UART_LINE_ERRORS app PRIVATE src/uart_line_errors.c)

if(CONFIG_TEST_INTERNAL_LB OR CONFIG_TEST_EXTERNAL_LB OR CONFIG_TEST_UART_FIFO)
  target_sources(app PRIVATE src/uart_fifo_depth.c)
endif()
target_sources_ifdef(CONFIG_TEST_UART_LPUART app PRIVATE src/uart_lpuart.c)
target_sources_ifdef(CONFIG_TEST_UART_HOTPLUG app PRIVATE src/uart_hotplug.c)
