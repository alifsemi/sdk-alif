cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_dac)

target_sources(app PRIVATE
  src/main.c
  src/test_dac_basic.c
  src/test_dac_boundary.c
  src/test_dac_capacitance.c
  src/test_dac_continuous.c
  src/test_dac_data_format.c
)
