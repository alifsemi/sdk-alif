cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pdm)

target_sources(app PRIVATE src/alif_test_pdm.c)


