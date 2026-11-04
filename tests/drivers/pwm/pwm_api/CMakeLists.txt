cmake_minimum_required(VERSION 3.20.0)

option(OSPI "Enable to run pinctrl in ospi" OFF)
option(INVERTED "Enable to test PWM Inverted Polarity" OFF)

# Overlays are family-named (alif_ensemble / alif_balletto), not BOARD-named.
# Do not append boards/${BOARD}_pwm_*.overlay — that file does not exist for
# documented boards (alif_e8_dk, alif_b1_dk, ...) and would fail configure
# even when the caller already passed a valid family overlay.
if(BOARD MATCHES "alif_b1|alif_e1c|alif_balletto")
	set(PWM_FAMILY "alif_balletto")
	set(OSPI_INSTANCE "0")
else()
	set(PWM_FAMILY "alif_ensemble")
	set(OSPI_INSTANCE "1")
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

if(OSPI)
	set(OSPI_OVERLAY
		"${CMAKE_CURRENT_SOURCE_DIR}/../../../boards/${BOARD}_ospi${OSPI_INSTANCE}.overlay")
	if(EXISTS "${OSPI_OVERLAY}")
		list(APPEND DTC_OVERLAY_FILE "${OSPI_OVERLAY}")
	endif()
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pwm_api)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
