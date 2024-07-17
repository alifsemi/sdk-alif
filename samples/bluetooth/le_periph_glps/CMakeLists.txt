cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(le_periph_glps)

# Include directories
zephyr_include_directories(includes)

target_sources(app PRIVATE
    src/main.c
    src/rtc_emulator.c
)
