cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_ospi_flash)

target_include_directories(app PRIVATE
  ${ZEPHYR_BASE}/drivers/flash
)

if(CONFIG_ALIF_OSPI_FLASH_XIP)
  target_sources(app PRIVATE
    src/test_ospi_xip_tests.c
    src/test_ospi_xip_reg_tests.c
  )
else()
  target_sources(app PRIVATE
    src/test_ospi_main.c
    src/test_ospi_boundary_tests.c
    src/test_ospi_negative_tests.c
    src/test_ospi_ctrl_tests.c
    src/test_ospi_xip_reg_tests.c
  )

  if(CONFIG_TEST_OSPI_FLASH_PERF)
    target_sources(app PRIVATE src/test_ospi_perf_tests.c)
  endif()

  if(CONFIG_TEST_OSPI_FLASH_EXTRA)
    target_sources(app PRIVATE
      src/test_ospi_flash_dev_tests.c
      src/test_ospi_irq_tests.c
      src/test_ospi_xip_transition_tests.c
    )
  endif()
endif()
