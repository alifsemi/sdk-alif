/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * S80KS2564 datasheet checks. Overlay/test only — does not change the MEMC
 * driver. Accesses are chunked so CS# can rise before tCSM (4 µs at ≤85 °C).
 */

 #include <zephyr/device.h>
 #include <zephyr/devicetree.h>
 #include <zephyr/kernel.h>
 #include <zephyr/logging/log.h>
 #include <zephyr/sys/barrier.h>
 #include <zephyr/sys/util.h>
 #include <zephyr/ztest.h>
 
 #if !DT_HAS_ALIAS(spi_psram)
 #error "Devicetree alias spi-psram is required (boards/alif_hex_s80ks.overlay)"
 #endif
 
 LOG_MODULE_DECLARE(xip);
 
 #define S80KS_NODE              DT_ALIAS(spi_psram)
 #define S80KS_CTRL              DT_PARENT(S80KS_NODE)
 #define S80KS_SIZE              ((uint32_t)DT_PROP(S80KS_NODE, size))
 #define S80KS_LATENCY           ((uint32_t)DT_PROP(S80KS_NODE, latency))
 #define S80KS_BUS_HZ            ((uint32_t)DT_PROP(S80KS_CTRL, bus_speed))
 #define S80KS_XIP_BASE          ((uint32_t)DT_PROP_BY_IDX(S80KS_CTRL, \
                               xip_base_address, 0))
 #define S80KS_XIP_WIN           ((uint32_t)DT_PROP_BY_IDX(S80KS_CTRL, \
                               xip_base_address, 1))
 
 /* 256 Mb device = 32 MB. DDR x16 @ 200 MHz = 800 MB/s. */
 #define S80KS_SIZE_256MBIT      (32U * 1024U * 1024U)
 #define S80KS_XIP_OSPI1         0xC0000000U
 
 /*
  * tCSM = 4 µs (grade 3, TA ≤ 85 °C). 64 words = 256 bytes:
  * ~0.3 µs at 200 MHz DDR x16, ~1.6 µs at 40 MHz.
  */
 #define S80KS_TCSM_CHUNK_WORDS  64U
 
 static uint32_t s80ks_max_hz_for_latency(uint32_t lc)
 {
     switch (lc) {
     case 3:
         return 85000000U;
     case 4:
         return 104000000U;
     case 5:
         return 133000000U;
     case 6:
         return 166000000U;
     case 7:
         return 200000000U;
     default:
         return 0U;
     }
 }
 
 static void s80ks_tcsm_gap(void)
 {
     barrier_dsync_fence_full();
 }
 
 static void s80ks_fill_u32(volatile uint32_t *ptr, uint32_t n, uint32_t seed)
 {
     for (uint32_t i = 0; i < n; i++) {
         ptr[i] = seed + i;
         if (((i + 1U) % S80KS_TCSM_CHUNK_WORDS) == 0U) {
             s80ks_tcsm_gap();
         }
     }
     s80ks_tcsm_gap();
 }
 
 static int s80ks_check_u32(volatile uint32_t *ptr, uint32_t n, uint32_t seed)
 {
     int errors = 0;
 
     for (uint32_t i = 0; i < n; i++) {
         uint32_t expected = seed + i;
         uint32_t got = ptr[i];
 
         if (got != expected) {
             errors++;
             if (errors <= 5) {
                 LOG_INF("word %u: exp 0x%08x got 0x%08x",
                     i, expected, got);
             }
         }
         if (((i + 1U) % S80KS_TCSM_CHUNK_WORDS) == 0U) {
             s80ks_tcsm_gap();
         }
     }
     s80ks_tcsm_gap();
     return errors;
 }
 
 /* Overlay must describe the 256 Mb S80KS2564 on OSPI1 XIP. */
 ZTEST(xip_tests, test_s80ks_dt_identity)
 {
     zassert_true(DT_NODE_HAS_COMPAT(S80KS_NODE, alif_infineon_s80ks2564),
              "spi-psram is not alif,infineon-s80ks2564");
     zassert_equal(S80KS_SIZE, S80KS_SIZE_256MBIT,
               "size %u, expected 32 MB (256 Mbit)", S80KS_SIZE);
     zassert_equal(S80KS_XIP_BASE, S80KS_XIP_OSPI1,
               "XIP base 0x%08x, expected OSPI1 0xC0000000", S80KS_XIP_BASE);
     zassert_true(S80KS_SIZE <= S80KS_XIP_WIN,
              "size %u exceeds XIP window %u", S80KS_SIZE, S80KS_XIP_WIN);
     zassert_true(S80KS_BUS_HZ > 0U && S80KS_BUS_HZ <= 200000000U,
              "bus-speed %u Hz outside 1..200 MHz", S80KS_BUS_HZ);
 }
 
 /*
  * CR0[7:4] vs clock (tACC 35 ns):
  * LC3 ≤85 MHz, LC4 ≤104, LC5 ≤133, LC6 ≤166, LC7 ≤200 (default).
  */
 ZTEST(xip_tests, test_s80ks_latency_vs_frequency)
 {
     uint32_t max_hz = s80ks_max_hz_for_latency(S80KS_LATENCY);
 
     LOG_INF("S80KS2564 DT: bus-speed %u Hz, latency %u, max for LC %u Hz",
         S80KS_BUS_HZ, S80KS_LATENCY, max_hz);
 
     zassert_true(S80KS_LATENCY >= 3U && S80KS_LATENCY <= 7U,
              "latency %u not in CR0 range 3-7", S80KS_LATENCY);
     zassert_true(max_hz > 0U, "reserved latency code %u", S80KS_LATENCY);
     zassert_true(S80KS_BUS_HZ <= max_hz,
              "bus-speed %u Hz exceeds LC%u max %u Hz "
              "(200 MHz requires latency 7)",
              S80KS_BUS_HZ, S80KS_LATENCY, max_hz);
 }
 
 /* Native bus is DQ[15:0]; halfword is one HyperBus beat. */
 ZTEST(xip_tests, test_s80ks_x16_walking_bits)
 {
     volatile uint16_t *ptr = (volatile uint16_t *)S80KS_XIP_BASE;
 
     LOG_INF("=== S80KS x16 walking-1/0 on DQ[15:0] ===");
 
     for (int bit = 0; bit < 16; bit++) {
         ptr[bit] = (uint16_t)BIT(bit);
         s80ks_tcsm_gap();
     }
     for (int bit = 0; bit < 16; bit++) {
         uint16_t expected = (uint16_t)BIT(bit);
         uint16_t got = ptr[bit];
 
         s80ks_tcsm_gap();
         zassert_equal(got, expected,
                   "DQ[%d] walking-1: wrote 0x%04x read 0x%04x",
                   bit, expected, got);
     }
 
     for (int bit = 0; bit < 16; bit++) {
         ptr[bit] = (uint16_t)(~BIT(bit) & 0xFFFF);
         s80ks_tcsm_gap();
     }
     for (int bit = 0; bit < 16; bit++) {
         uint16_t expected = (uint16_t)(~BIT(bit) & 0xFFFF);
         uint16_t got = ptr[bit];
 
         s80ks_tcsm_gap();
         zassert_equal(got, expected,
                   "DQ[%d] walking-0: wrote 0x%04x read 0x%04x",
                   bit, expected, got);
     }
 }
 
 /* DDR: two 16-bit beats per CK. Alternating 0x5555/0xAAAA stresses both edges. */
 ZTEST(xip_tests, test_s80ks_ddr_beat_patterns)
 {
     volatile uint32_t *ptr = (volatile uint32_t *)S80KS_XIP_BASE;
     const uint32_t patterns[] = {
         0x5555AAAAU, 0xAAAA5555U, 0x00FF00FFU, 0xFF00FF00U,
     };
     const uint32_t n = 256;
 
     LOG_INF("=== S80KS DDR beat patterns ===");
 
     for (int p = 0; p < ARRAY_SIZE(patterns); p++) {
         for (uint32_t i = 0; i < n; i++) {
             ptr[i] = patterns[p];
             if (((i + 1U) % S80KS_TCSM_CHUNK_WORDS) == 0U) {
                 s80ks_tcsm_gap();
             }
         }
         s80ks_tcsm_gap();
         for (uint32_t i = 0; i < n; i++) {
             uint32_t got = ptr[i];
 
             if (((i + 1U) % S80KS_TCSM_CHUNK_WORDS) == 0U) {
                 s80ks_tcsm_gap();
             }
             zassert_equal(got, patterns[p],
                       "DDR 0x%08x failed at word %u: got 0x%08x",
                       patterns[p], i, got);
         }
     }
 }
 
 /*
  * Linear burst (CA[45]=1): sequential words across row boundaries.
  * Chunked to keep each CS# window under tCSM.
  */
 ZTEST(xip_tests, test_s80ks_linear_burst_under_tcsm)
 {
     volatile uint32_t *ptr = (volatile uint32_t *)S80KS_XIP_BASE;
     const uint32_t n = 2048; /* 8 KB, several row crossings */
     const uint32_t seed = 0x1B000000U;
     int errors;
 
     LOG_INF("=== S80KS linear burst (%u words, tCSM gaps) ===", n);
     s80ks_fill_u32(ptr, n, seed);
     errors = s80ks_check_u32(ptr, n, seed);
     zassert_equal(errors, 0, "linear burst failed: %d errors", errors);
 }
 
 /*
  * Host must end the burst so refresh can run, then start a new transaction.
  * Gap is >> tCSM so distributed refresh must have had a chance.
  */
 ZTEST(xip_tests, test_s80ks_refresh_between_transactions)
 {
     volatile uint32_t *ptr = (volatile uint32_t *)S80KS_XIP_BASE;
     const uint32_t n = 512;
     const uint32_t seed = 0xDA7A0000U;
     int errors;
 
     LOG_INF("=== S80KS refresh gap (tCSM / distributed refresh) ===");
     s80ks_fill_u32(ptr, n, seed);
     k_busy_wait(20); /* several tCSM intervals with CS# idle */
     errors = s80ks_check_u32(ptr, n, seed);
     zassert_equal(errors, 0,
               "data lost across refresh gap: %d errors", errors);
 }
 
 /* RWDS is write-mask on writes; byte stores must not corrupt neighbors. */
 ZTEST(xip_tests, test_s80ks_rwds_byte_mask)
 {
     volatile uint8_t *bp = (volatile uint8_t *)S80KS_XIP_BASE;
     volatile uint32_t *wp = (volatile uint32_t *)S80KS_XIP_BASE;
 
     LOG_INF("=== S80KS RWDS byte mask ===");
 
     *wp = 0xA5A5A5A5U;
     s80ks_tcsm_gap();
     bp[1] = 0x5AU;
     s80ks_tcsm_gap();
 
     zassert_equal(bp[0], 0xA5, "byte0 corrupted by masked write");
     zassert_equal(bp[1], 0x5A, "byte1 mask write failed");
     zassert_equal(bp[2], 0xA5, "byte2 corrupted by masked write");
     zassert_equal(bp[3], 0xA5, "byte3 corrupted by masked write");
 }
 
 /*
  * If XIP uses wrapped burst (CA[45]=0) with 16-word wrap, word 16 overwrites
  * word 0. Linear INCR must not wrap.
  */
 ZTEST(xip_tests, test_s80ks_no_16word_wrap)
 {
     volatile uint16_t *ptr = (volatile uint16_t *)S80KS_XIP_BASE;
 
     LOG_INF("=== S80KS 16-word wrap vs linear ===");
 
     for (int i = 0; i < 32; i++) {
         ptr[i] = (uint16_t)(0xA000U + i);
         s80ks_tcsm_gap();
     }
     for (int i = 0; i < 32; i++) {
         uint16_t expected = (uint16_t)(0xA000U + i);
         uint16_t got = ptr[i];
 
         s80ks_tcsm_gap();
         zassert_equal(got, expected,
                   "halfword %d: exp 0x%04x got 0x%04x "
                   "(wrap would alias 16-word window)",
                   i, expected, got);
     }
 }
 
 /* 1/8, 1/4, 1/2, full array offsets (datasheet partial-array refresh regions). */
 ZTEST(xip_tests, test_s80ks_partial_array_offsets)
 {
     const uint32_t frac[] = {8, 4, 2, 1};
     int total = 0;
 
     LOG_INF("=== S80KS partial-array offsets ===");
 
     for (int f = 0; f < ARRAY_SIZE(frac); f++) {
         uint32_t region = S80KS_SIZE / frac[f];
         uint32_t off = (frac[f] == 1U) ? 0U : (S80KS_SIZE - region);
         volatile uint32_t *ptr = (volatile uint32_t *)(S80KS_XIP_BASE + off);
         uint32_t seed = 0x3A000000U | ((uint32_t)f << 16);
         int errors;
 
         LOG_INF("1/%u at offset 0x%08x", frac[f], off);
         s80ks_fill_u32(ptr, 64, seed);
         errors = s80ks_check_u32(ptr, 64, seed);
         total += errors;
     }
 
     zassert_equal(total, 0, "partial-array offset R/W failed: %d errors", total);
 }
 