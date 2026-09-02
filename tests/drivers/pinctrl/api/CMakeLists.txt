cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(pinctrl)

target_include_directories(app PRIVATE ../common)
target_sources(app PRIVATE src/main.c ../common/test_device.c)
