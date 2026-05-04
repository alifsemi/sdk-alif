cmake_minimum_required(VERSION 3.20.0)

# ==================== BOARD AUTO-DETECTION ==================
# Set ALIF_BOARD macro for overlay preprocessor so every overlay
# automatically picks the right pins / interrupts / console.
# Override on the command line:  west build -b <board> -- -DALIF_BOARD=ALIF_BOARD_E7_HP
if(NOT DEFINED ALIF_BOARD)
  if(BOARD MATCHES "e7.*rtss_he")
    set(ALIF_BOARD "ALIF_BOARD_E7_HE")
  elseif(BOARD MATCHES "e7.*rtss_hp")
    set(ALIF_BOARD "ALIF_BOARD_E7_HP")
  elseif(BOARD MATCHES "e8.*rtss_he")
    set(ALIF_BOARD "ALIF_BOARD_E8_HE")
  elseif(BOARD MATCHES "e8.*rtss_hp")
    set(ALIF_BOARD "ALIF_BOARD_E8_HP")
  elseif(BOARD MATCHES "a5")
    set(ALIF_BOARD "ALIF_BOARD_A5")
  elseif(BOARD MATCHES "e1c.*rtss_he")
    set(ALIF_BOARD "ALIF_BOARD_E1C")
  elseif(BOARD MATCHES "b1.*rtss_he")
    set(ALIF_BOARD "ALIF_BOARD_B1")
  else()
    set(ALIF_BOARD "ALIF_BOARD_E8_HP")
    message(WARNING "Unknown board '${BOARD}' — defaulting to E8 HP pin mapping.
            Set -DALIF_BOARD=... to override.")
  endif()
endif()

# Pass to the devicetree preprocessor so overlays resolve the macros.
# C code does not need ALIF_BOARD, but the C preprocessor that runs on .overlay
# files uses DTS_EXTRA_CPPFLAGS.
list(APPEND DTS_EXTRA_CPPFLAGS "-DALIF_BOARD=${ALIF_BOARD}")
message(STATUS "SPI test overlay board: ${ALIF_BOARD}")

# Default overlay for plain west builds; Twister/CLI overlays take precedence.
if(NOT DTC_OVERLAY_FILE)
  set(DTC_OVERLAY_FILE
      ${CMAKE_CURRENT_SOURCE_DIR}/boards/spi.overlay
      CACHE STRING "Default SPI test overlay" FORCE)
endif()

# ==================== ZEPHYR PROJECT SETUP ==================
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_spi)



# ==================== STACK SIZES ==========================
# Stack sizes are defined in prj.conf (20KB default for all tests)

target_include_directories(app PRIVATE src)

# ==================== SOURCE FILE DEFINITIONS ================
set(SPI_CORE_SOURCES
  src/test_spi_common.c
  src/test_spi_threads.c
)

set(SPI_LOOPBACK_SOURCES
  src/test_spi_loopback.c
)

set(SPI_NEGATIVE_SOURCES
  src/test_spi_negative.c
)

set(SPI_BOUNDARY_SOURCES
  src/test_spi_boundary.c
)

set(SPI_STRESS_SOURCES
  src/test_spi_stress.c
)

set(SPI_PERF_SOURCES
  src/test_spi_perf.c
)

# ==================== CORE TEST BUILD ========================
set(SPI_TEST_SOURCES ${SPI_CORE_SOURCES})

# ==================== CONDITIONAL TEST SUITES ==============
# Include test sources based on Kconfig selections
if(CONFIG_TEST_SPI_LOOPBACK)
  list(APPEND SPI_TEST_SOURCES ${SPI_LOOPBACK_SOURCES})
endif()

if(CONFIG_TEST_SPI_NEGATIVE)
  list(APPEND SPI_TEST_SOURCES ${SPI_NEGATIVE_SOURCES})
endif()

if(CONFIG_TEST_SPI_BOUNDARY)
  list(APPEND SPI_TEST_SOURCES ${SPI_BOUNDARY_SOURCES})
endif()

if(CONFIG_TEST_SPI_STRESS)
  list(APPEND SPI_TEST_SOURCES ${SPI_STRESS_SOURCES})
endif()

if(CONFIG_TEST_SPI_PERFORMANCE)
  list(APPEND SPI_TEST_SOURCES ${SPI_PERF_SOURCES})
endif()

target_sources(app PRIVATE ${SPI_TEST_SOURCES})

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
