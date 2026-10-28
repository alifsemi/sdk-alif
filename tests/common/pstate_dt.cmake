# Devicetree overlays are preprocessed before Kconfig writes autoconf.h.
# Forward the P-state selections, and which RTSS core this board is,
# so pstate_common.overlay can see them.

list(APPEND DTS_EXTRA_CPPFLAGS
     "-I${CMAKE_CURRENT_LIST_DIR}")

if(BOARD MATCHES "rtss_hp")
  list(APPEND DTS_EXTRA_CPPFLAGS "-DCONFIG_RTSS_HP=1")
endif()

set(alif_pstate_syms
    ALIF_TEST_PSTATE_ALL
    ALIF_TEST_PSTATE_PLL_200
    ALIF_TEST_PSTATE_PLL_100
    ALIF_TEST_PSTATE_PLL_120
    ALIF_TEST_PSTATE_PLL_80
    ALIF_TEST_PSTATE_PLL_60
    ALIF_TEST_PSTATE_HFRC_76P8
    ALIF_TEST_PSTATE_HFRC_38P4
    ALIF_TEST_PSTATE_HFRC_19P2
    ALIF_TEST_PSTATE_HFRC_9P6
    ALIF_TEST_PSTATE_HFRC_4P8
    ALIF_TEST_PSTATE_HFRC_2P4
    ALIF_TEST_PSTATE_HFRC_1P2
    ALIF_TEST_PSTATE_HFRC_0P6
)

foreach(sym ${alif_pstate_syms})
  if("${CONFIG_${sym}}" STREQUAL "y")
    list(APPEND DTS_EXTRA_CPPFLAGS "-DCONFIG_${sym}=1")
  endif()
endforeach()

set(alif_pstate_confs "${CMAKE_CURRENT_SOURCE_DIR}/prj.conf")
if(CONF_FILE)
  list(APPEND alif_pstate_confs ${CONF_FILE})
endif()
if(OVERLAY_CONFIG)
  separate_arguments(alif_pstate_overlay_list UNIX_COMMAND "${OVERLAY_CONFIG}")
  list(APPEND alif_pstate_confs ${alif_pstate_overlay_list})
endif()
if(EXTRA_CONF_FILE)
  separate_arguments(alif_pstate_extra_list UNIX_COMMAND "${EXTRA_CONF_FILE}")
  list(APPEND alif_pstate_confs ${alif_pstate_extra_list})
endif()

foreach(conf ${alif_pstate_confs})
  if(NOT EXISTS "${conf}")
    continue()
  endif()
  file(READ "${conf}" alif_pstate_conf_text)
  foreach(sym ${alif_pstate_syms})
    if(alif_pstate_conf_text MATCHES "(^|\n)[ \t]*CONFIG_${sym}=y([ \t]|$)")
      list(APPEND DTS_EXTRA_CPPFLAGS "-DCONFIG_${sym}=1")
    endif()
  endforeach()
endforeach()
