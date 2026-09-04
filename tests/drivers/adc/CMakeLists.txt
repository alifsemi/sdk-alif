cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(adc_api)

target_sources(app PRIVATE
  src/test_adc.c
  src/temperature.c
)

if(CONFIG_TEST_ADC24)
  target_sources(app PRIVATE src/test_adc24.c)
else()
  target_sources(app PRIVATE src/test_adc12.c)
endif()
