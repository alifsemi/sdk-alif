cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(i3c)

set(I3C_COMMON_SOURCES src/i3c_common.c)

set(I3C_DISCOVERY_SOURCES src/i3c_discovery.c ${I3C_COMMON_SOURCES})

set(I3C_CCC_SOURCES src/i3c_ccc.c ${I3C_COMMON_SOURCES})

set(I3C_IBI_SOURCES src/i3c_ibi.c ${I3C_COMMON_SOURCES})

set(I3C_NEGATIVE_SOURCES src/i3c_negative.c ${I3C_COMMON_SOURCES})

set(I3C_STRESS_SOURCES src/i3c_stress.c ${I3C_COMMON_SOURCES})

set(I3C_SENSOR_SOURCES src/i3c_sensor.c ${I3C_COMMON_SOURCES})


if(CONFIG_I3C_DISCOVERY_TESTS)
  target_sources(app PRIVATE ${I3C_DISCOVERY_SOURCES})
endif()

if(CONFIG_I3C_CCC_TESTS)
  target_sources(app PRIVATE ${I3C_CCC_SOURCES})
endif()

if(CONFIG_I3C_IBI_TESTS)
  target_sources(app PRIVATE ${I3C_IBI_SOURCES})
endif()

if(CONFIG_I3C_NEGATIVE_TESTS)
  target_sources(app PRIVATE ${I3C_NEGATIVE_SOURCES})
endif()

if(CONFIG_I3C_STRESS_TESTS)
  target_sources(app PRIVATE ${I3C_STRESS_SOURCES})
endif()

if(CONFIG_I3C_SENSOR_TESTS)
  target_sources(app PRIVATE ${I3C_SENSOR_SOURCES})
endif()

