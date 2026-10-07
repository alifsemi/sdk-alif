/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * Shared SRAM1 cross-core heap layout for the MHU doorbell sample.
 *
 * Each shared heap is a struct sys_heap control block immediately followed by
 * the memory it manages, both placed in shared SRAM1 so the two participating
 * cores operate on the same heap instance. The control block plus managed
 * memory occupy one SHARED_HEAP_SPAN region; the managed area itself is
 * SHARED_HEAP_SIZE bytes. There is one heap per MHU channel.
 *
 * The addresses are a fixed contract shared by every participant (M55-HE,
 * M55-HP and the A32/APSS side), so they live in one header rather than being
 * duplicated per test case. The A32 (HE<->A32) heaps are page-aligned and each
 * whole span fits in one 4 KB page so the A32 side can reach every block
 * through a single mapping.
 *
 * The region sits 0x24000 below the end of SRAM1, whose location differs per
 * SoC series:
 *   E8: SRAM1 0x02400000 (4 MB)   -> SHARED_HEAP_BASE 0x027DC000
 *   E7: SRAM1 0x08000000 (2.5 MB) -> SHARED_HEAP_BASE 0x0825C000
 * The base is selected by SoC series rather than taken from the devicetree
 * because the APSS devicetree has no sram1 node.
 *
 *   SRAM1 layout relative to SHARED_HEAP_BASE (ctrl / mem, each span 0x800):
 *     HE<->HP   MHU0: ctrl +0x0000, mem +0x0100
 *     HE<->HP   MHU1: ctrl +0x0800, mem +0x0900
 *     HE<->A32  MHU0: ctrl +0x1000, mem +0x1100
 *     HE<->A32  MHU1: ctrl +0x2000, mem +0x2100
 */

#ifndef MHU_DOORBELL_HEAP_H
#define MHU_DOORBELL_HEAP_H

#if defined(CONFIG_SOC_SERIES_E8)
#define SHARED_HEAP_BASE  0x027DC000
#elif defined(CONFIG_SOC_SERIES_E7)
#define SHARED_HEAP_BASE  0x0825C000
#else
#error "Shared SRAM1 heap location is not defined for this SoC"
#endif

#define SHARED_HEAP_SIZE  0x700   /* managed memory size */
#define SHARED_HEAP_SPAN  0x800   /* ctrl block + managed memory */

/* M55-HE <-> M55-HP heaps (one per MHU channel). */
#define SHARED_HEAP_HEHP_MHU0_CTRL  (SHARED_HEAP_BASE + 0x0000)
#define SHARED_HEAP_HEHP_MHU0_MEM   (SHARED_HEAP_BASE + 0x0100)
#define SHARED_HEAP_HEHP_MHU1_CTRL  (SHARED_HEAP_BASE + 0x0800)
#define SHARED_HEAP_HEHP_MHU1_MEM   (SHARED_HEAP_BASE + 0x0900)

/* M55-HE <-> A32 (APSS) heaps (one per MHU channel). */
#define SHARED_HEAP_HEA32_MHU0_CTRL (SHARED_HEAP_BASE + 0x1000)
#define SHARED_HEAP_HEA32_MHU0_MEM  (SHARED_HEAP_BASE + 0x1100)
#define SHARED_HEAP_HEA32_MHU1_CTRL (SHARED_HEAP_BASE + 0x2000)
#define SHARED_HEAP_HEA32_MHU1_MEM  (SHARED_HEAP_BASE + 0x2100)

#endif /* MHU_DOORBELL_HEAP_H */
