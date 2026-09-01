cmake_minimum_required(VERSION 3.20.0)

option(INVERTED "Enable to test PWM Inverted Polarity" OFF)

# Overlays are family-named (alif_ensemble / alif_balletto), not BOARD-named.
if(BOARD MATCHES "alif_b1|alif_e1c|alif_balletto")
	set(PWM_FAMILY "alif_balletto")
else()
	set(PWM_FAMILY "alif_ensemble")
endif()

if(INVERTED)
	set(PWM_POLARITY "inverted")
else()
	set(PWM_POLARITY "normal")
endif()

if(NOT DTC_OVERLAY_FILE)
	set(DTC_OVERLAY_FILE
		"boards/${PWM_FAMILY}_pwm_${PWM_POLARITY}.overlay")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pwm_api)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
