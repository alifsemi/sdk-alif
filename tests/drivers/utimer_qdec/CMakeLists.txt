cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(alif_utimer_qdec)

target_sources(app PRIVATE
	src/qdec_test.c
	src/qdec_emulator.c
)
