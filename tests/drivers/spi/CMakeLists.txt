cmake_minimum_required(VERSION 3.20.0)

# ==================== BOARD OVERRIDE (optional) ==================
# ALIF_BOARD is auto-detected in board_common.overlay from Zephyr
# Kconfig symbols (CONFIG_SOC_SERIES_* / CONFIG_RTSS_*).
# To override manually: west build -b <board> -- -DALIF_BOARD=ALIF_BOARD_E7_HP
if(DEFINED ALIF_BOARD)
  list(APPEND DTS_EXTRA_CPPFLAGS "-DALIF_BOARD=${ALIF_BOARD}")
  message(STATUS "SPI test overlay board (manual override): ${ALIF_BOARD}")
endif()

# ==================== ZEPHYR PROJECT SETUP ==================
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_spi)

# ==================== STACK SIZES ==========================
# Stack sizes are defined in prj.conf (20KB default for all tests)

target_include_directories(app PRIVATE src)

# ==================== SOURCE FILES ==========================
target_sources(app PRIVATE
  src/test_spi_common.c
  src/test_spi_threads.c
)

target_sources_ifdef(CONFIG_TEST_SPI_LOOPBACK    app PRIVATE src/test_spi_loopback.c)
target_sources_ifdef(CONFIG_TEST_SPI_NEGATIVE   app PRIVATE src/test_spi_negative.c)
target_sources_ifdef(CONFIG_TEST_SPI_BOUNDARY   app PRIVATE src/test_spi_boundary.c)
target_sources_ifdef(CONFIG_TEST_SPI_STRESS     app PRIVATE src/test_spi_stress.c)
target_sources_ifdef(CONFIG_TEST_SPI_PERFORMANCE app PRIVATE src/test_spi_perf.c)

# ==================== COMPILATION FLAGS ======================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(app PRIVATE -Wall -Wextra -Werror)
endif()

# ==================== VALIDATION MESSAGES ====================
message(STATUS "  Test Type: SPI Driver Validation")
message(STATUS "  Hardware Overlay: ${DTC_OVERLAY_FILE}")

if(CONFIG_TEST_SPI_LOOPBACK)
  message(STATUS "  Enabled: Loopback Tests")
endif()

if(CONFIG_TEST_SPI_NEGATIVE)
  message(STATUS "  Enabled: Negative Tests")
endif()

if(CONFIG_TEST_SPI_BOUNDARY)
  message(STATUS "  Enabled: Boundary Tests")
endif()

if(CONFIG_TEST_SPI_STRESS)
  message(STATUS "  Enabled: Stress Tests")
endif()

if(CONFIG_TEST_SPI_PERFORMANCE)
  message(STATUS "  Enabled: Performance Tests")
endif()

if(CONFIG_TEST_SPI_USE_DMA)
  message(STATUS "  Enabled: DMA path")
endif()
