/*
 * Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Linker snippet: Place the TFLM tensor arena into the SRAM1 memory region.
 *
 * SRAM1 is mapped as non-cacheable in the MMU so the NPU and CPU share
 * a coherent view without explicit cache maintenance.
 *
 * The Zephyr build system generates a SRAM1 output section from the
 * "zephyr,memory-region" devicetree property.  We use an input-section
 * pattern that matches the attribute used in main.cpp:
 *   __attribute__((section("SRAM1.tflm_arena")))
 */

SECTION_DATA_PROLOGUE(SRAM1.tflm_arena, (NOLOAD),)
{
    . = ALIGN(16);
    *(.SRAM1.tflm_arena)
    *(.SRAM1.tflm_arena.*)
    . = ALIGN(16);
} GROUP_LINK_IN(SRAM1)
