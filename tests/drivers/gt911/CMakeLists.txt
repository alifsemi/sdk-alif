cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(gt911)

target_sources(app PRIVATE src/test_gt911.c)
