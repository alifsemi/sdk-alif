cmake_minimum_required(VERSION 3.20.0)

option(OSPI "Enable to run pinctrl in ospi" OFF)
option(INVERTED "Enable to test PWM Inverted Polarity" OFF)


if(${BOARD} STREQUAL "alif_e1c_dk_rtss_he")
	set(OSPI_INSTANCE "0")
elseif(${BOARD} STREQUAL "alif_b1_dk_rtss_he")
	set(OSPI_INSTANCE "0")
else()
	set(OSPI_INSTANCE "1")
endif()

if(OSPI)
	list(APPEND DTC_OVERLAY_FILE "../../../boards/${BOARD}_ospi${OSPI_INSTANCE}.overlay")
endif()

if(INVERTED)
	list(APPEND DTC_OVERLAY_FILE "boards/${BOARD}_pwm_inverted.overlay")
else()
	list(APPEND DTC_OVERLAY_FILE "boards/${BOARD}_pwm_normal.overlay")
endif()


find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pwm_api)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
