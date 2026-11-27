/*
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr/linker/sections.h>
#include <zephyr/devicetree.h>

#include <zephyr/linker/linker-defs.h>
#include <zephyr/linker/linker-tool.h>

#include <zephyr/arch/arm/cortex_m/scripts/linker.ld>

/* Symbols in ROM */

#if defined(CONFIG_ALIF_BLE_ROM_API_EXTERNAL) && CONFIG_ALIF_BLE_ROM_API_EXTERNAL
/* BLE/LC3 ROM symbol addresses supplied by an out-of-tree module (see
 * ALIF_BLE_ROM_API_* Kconfig). The paths are absolute and resolved from those
 * config strings. Takes precedence over the bundled set so a board can override
 * the default. */
#include CONFIG_ALIF_LC3_ROM_SYMBOLS_LDS
#include CONFIG_ALIF_BLE_ROM_SYMBOLS_LDS
#elif defined(CONFIG_ALIF_BLE_ROM_IMAGE_V1_2) && CONFIG_ALIF_BLE_ROM_IMAGE_V1_2
#include "lc3/v1_2/rom_symbols_lc3.lds"
#include "ble/v1_2/rom_symbols_ble.lds"
#else
#error "wrong ROM version"
#endif
