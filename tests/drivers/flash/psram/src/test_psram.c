/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 * APS512XXN checks. Built only when CONFIG_TEST_PSRAM=y.
 */

 #include <zephyr/devicetree.h>
 #include <zephyr/logging/log.h>
 #include <zephyr/sys/util.h>
 #include <zephyr/ztest.h>
 
 #if !DT_HAS_ALIAS(spi_psram)
 #error "Devicetree alias spi-psram is required"
 #endif
 
 LOG_MODULE_DECLARE(xip);
 
 #define APS_NODE        DT_ALIAS(spi_psram)
 #define APS_CTRL        DT_PARENT(APS_NODE)
 #define APS_SIZE        ((uint32_t)DT_PROP(APS_NODE, size))
 #define APS_LATENCY     ((uint32_t)DT_PROP(APS_NODE, latency_code))
 #define APS_BUS_HZ      ((uint32_t)DT_PROP(APS_CTRL, bus_speed))
 #define APS_XIP_BASE    ((uint32_t)DT_PROP_BY_IDX(APS_CTRL, xip_base_address, 0))
 #define APS_XIP_WIN     ((uint32_t)DT_PROP_BY_IDX(APS_CTRL, xip_base_address, 1))
 #define APS_SIZE_512MBIT (64U * 1024U * 1024U)
 
 /* latency-code vs max CLK from the APS512XXN binding. */
 static uint32_t aps_max_hz_for_latency(uint32_t lc)
 {
     switch (lc) {
     case 3:
         return 66000000U;
     case 4:
         return 109000000U;
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
 
 ZTEST(xip_tests, test_aps512_dt_identity)
 {
     zassert_true(DT_NODE_HAS_COMPAT(APS_NODE, alif_apmemory_aps512xxn),
              "spi-psram is not alif,apmemory-aps512xxn");
     zassert_equal(APS_SIZE, APS_SIZE_512MBIT,
               "size %u, expected 64 MB (512 Mbit)", APS_SIZE);
     zassert_true(APS_XIP_BASE != 0U, "XIP base is 0");
     zassert_true(APS_SIZE <= APS_XIP_WIN,
              "size %u exceeds XIP window %u", APS_SIZE, APS_XIP_WIN);
     zassert_true(APS_BUS_HZ > 0U && APS_BUS_HZ <= 200000000U,
              "bus-speed %u Hz outside 1..200 MHz", APS_BUS_HZ);
 }
 
 ZTEST(xip_tests, test_aps512_latency_code_vs_frequency)
 {
     uint32_t max_hz = aps_max_hz_for_latency(APS_LATENCY);
 
     LOG_INF("APS512XXN DT: bus-speed %u Hz, latency-code %u, max %u Hz",
         APS_BUS_HZ, APS_LATENCY, max_hz);
 
     zassert_true(APS_LATENCY >= 3U && APS_LATENCY <= 7U,
              "latency-code %u not in 3-7", APS_LATENCY);
     zassert_true(max_hz > 0U, "reserved latency-code %u", APS_LATENCY);
     zassert_true(APS_BUS_HZ <= max_hz,
              "bus-speed %u Hz exceeds latency-code %u max %u Hz",
              APS_BUS_HZ, APS_LATENCY, max_hz);
 }
 