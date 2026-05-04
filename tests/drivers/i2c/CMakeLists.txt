cmake_minimum_required(VERSION 3.20.0)

# ==================== ZEPHYR PROJECT SETUP ==================
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_i2c)

# ==================== TEST CONFIGURATION ============
# Tests are now controlled via Kconfig (CONFIG_I2C_*_TESTS)

# ==================== SOURCE FILE DEFINITIONS ===============
set(I2C_CORE_SOURCES
  src/test_i2c.c
)

set(I2C_PERFORMANCE_SOURCES
  src/test_i2c_performance.c
)

set(I2C_BOUNDARY_SOURCES
  src/test_i2c_boundary.c
)

set(I2C_HARDWARE_AWARE_SOURCES
  src/test_i2c_hardware_aware.c
)

set(I2C_FAULT_INJECTION_SOURCES
  src/test_i2c_fault_injection.c
)

set(I2C_STRESS_SOURCES
  src/test_i2c_stress.c
)

set(I2C_BME680_SOURCES
  src/test_i2c_bme680.c
)

# ==================== CORE TEST BUILD =======================
# Always include core functional tests
target_sources(app PRIVATE ${I2C_CORE_SOURCES})

# ==================== CONDITIONAL TEST SUITES ===============
# Include test suites based on Kconfig settings
if(CONFIG_I2C_PERFORMANCE_TESTS)
  target_sources(app PRIVATE ${I2C_PERFORMANCE_SOURCES})
endif()

if(CONFIG_I2C_BOUNDARY_TESTS)
  target_sources(app PRIVATE ${I2C_BOUNDARY_SOURCES})
endif()

if(CONFIG_I2C_HARDWARE_AWARE_TESTS)
  target_sources(app PRIVATE ${I2C_HARDWARE_AWARE_SOURCES})
endif()

if(CONFIG_I2C_FAULT_INJECTION_TESTS)
  target_sources(app PRIVATE ${I2C_FAULT_INJECTION_SOURCES})
endif()

if(CONFIG_I2C_STRESS_TESTS OR CONFIG_I2C_ENHANCED_STRESS)
  target_sources(app PRIVATE ${I2C_STRESS_SOURCES})
endif()

if(CONFIG_I2C_BME680_TESTS)
  target_sources(app PRIVATE ${I2C_BME680_SOURCES})
endif()

# ==================== COMPILATION FLAGS ====================
# Enable additional diagnostics for debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(app PRIVATE -Wall -Wextra -Werror)
endif()

# ==================== VALIDATION MESSAGES ====================
message(STATUS "  Test Type: I2C Driver Validation")
message(STATUS "  Hardware Overlay: ${DTC_OVERLAY_FILE}")

if(CONFIG_I2C_PERFORMANCE_TESTS)
  message(STATUS "  Enabled: Performance Tests")
endif()

if(CONFIG_I2C_BOUNDARY_TESTS)
  message(STATUS "  Enabled: Boundary Tests")
endif()

if(CONFIG_I2C_HARDWARE_AWARE_TESTS)
  message(STATUS "  Enabled: Hardware-Aware Tests")
endif()

if(CONFIG_I2C_FAULT_INJECTION_TESTS)
  message(STATUS "  Enabled: Fault Injection Tests")
endif()

if(CONFIG_I2C_STRESS_TESTS)
  message(STATUS "  Enabled: Basic Stress Tests")
endif()

if(CONFIG_I2C_ENHANCED_STRESS)
  message(STATUS "  Enabled: Enhanced Stress Tests")
endif()

if(CONFIG_I2C_BME680_TESTS)
  message(STATUS "  Enabled: BME680 Sensor Integration Tests")
endif()
