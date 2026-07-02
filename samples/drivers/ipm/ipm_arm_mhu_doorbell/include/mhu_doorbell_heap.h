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
 * M55-HP and the A32/Linux side), so they live in one header rather than being
 * duplicated per test case. The A32 (HE<->A32) heaps are page-aligned and each
 * whole span fits in one 4 KB page so the Linux side can reach every block
 * through a single /dev/mem mmap; the Linux test application mirrors these.
 *
 *   SRAM1 layout (ctrl / mem, each span 0x800):
 *     HE<->HP   MHU0: ctrl 0x027DC000, mem 0x027DC100
 *     HE<->HP   MHU1: ctrl 0x027DC800, mem 0x027DC900
 *     HE<->A32  MHU0: ctrl 0x027DD000, mem 0x027DD100
 *     HE<->A32  MHU1: ctrl 0x027DE000, mem 0x027DE100
 */

#ifndef MHU_DOORBELL_HEAP_H
#define MHU_DOORBELL_HEAP_H

#define SHARED_HEAP_SIZE  0x700   /* managed memory size */
#define SHARED_HEAP_SPAN  0x800   /* ctrl block + managed memory */

/* M55-HE <-> M55-HP heaps (one per MHU channel). */
#define SHARED_HEAP_HEHP_MHU0_CTRL  0x027DC000
#define SHARED_HEAP_HEHP_MHU0_MEM   0x027DC100
#define SHARED_HEAP_HEHP_MHU1_CTRL  0x027DC800
#define SHARED_HEAP_HEHP_MHU1_MEM   0x027DC900

/* M55-HE <-> A32 (APSS) heaps (one per MHU channel). */
#define SHARED_HEAP_HEA32_MHU0_CTRL 0x027DD000
#define SHARED_HEAP_HEA32_MHU0_MEM  0x027DD100
#define SHARED_HEAP_HEA32_MHU1_CTRL 0x027DE000
#define SHARED_HEAP_HEA32_MHU1_MEM  0x027DE100

#endif /* MHU_DOORBELL_HEAP_H */
