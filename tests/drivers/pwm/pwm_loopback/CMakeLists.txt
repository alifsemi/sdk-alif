cmake_minimum_required(VERSION 3.20.0)

if(NOT DTC_OVERLAY_FILE)
	set(DTC_OVERLAY_FILE "boards/alif_pwm.overlay")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pwm_loopback)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
