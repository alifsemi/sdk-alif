cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(jpeg_hantro_vc9000e_test)

target_sources(app PRIVATE
	src/main.c
	src/jpeg_common.c
	src/incbin.c
	src/jpeg_perf.c
)
target_compile_definitions(app PRIVATE INCBIN_BASE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
