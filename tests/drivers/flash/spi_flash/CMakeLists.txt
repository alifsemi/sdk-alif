cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_ospi_flash)

if(CONFIG_ALIF_OSPI_FLASH_XIP)
  target_sources(app PRIVATE
    src/test_ospi_xip_tests.c
  )
else()
  target_sources(app PRIVATE
    src/test_ospi_main.c
    src/test_ospi_boundary_tests.c
    src/test_ospi_negative_tests.c
  )

  if(CONFIG_TEST_PERF)
    target_sources(app PRIVATE src/test_ospi_perf_tests.c)
  endif()
endif()
