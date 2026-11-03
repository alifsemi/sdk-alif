/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef ALIF_TEST_PDM_H_
#define ALIF_TEST_PDM_H_

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/drivers/pdm/pdm_alif.h>
#include <string.h>
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>
#define PDM	DT_ALIAS(test_pdm)
/*
 * Per-channel enable flags (1 if selected via Kconfig, else 0).
 * These are the single source of truth for the channel count and mask.
 */
#define PDM_CH0_EN IS_ENABLED(CONFIG_TEST_PDM_CH0)
#define PDM_CH1_EN IS_ENABLED(CONFIG_TEST_PDM_CH1)
#define PDM_CH2_EN IS_ENABLED(CONFIG_TEST_PDM_CH2)
#define PDM_CH3_EN IS_ENABLED(CONFIG_TEST_PDM_CH3)
#define PDM_CH4_EN IS_ENABLED(CONFIG_TEST_PDM_CH4)
#define PDM_CH5_EN IS_ENABLED(CONFIG_TEST_PDM_CH5)
#define PDM_CH6_EN IS_ENABLED(CONFIG_TEST_PDM_CH6)
#define PDM_CH7_EN IS_ENABLED(CONFIG_TEST_PDM_CH7)

/* Number of channels under test = count of enabled channels */
#define NUM_CHANNELS (PDM_CH0_EN + PDM_CH1_EN + PDM_CH2_EN + PDM_CH3_EN + \
		      PDM_CH4_EN + PDM_CH5_EN + PDM_CH6_EN + PDM_CH7_EN)

/* Channel bitmask derived from the enabled channels (req_chan_map_lo) */
#define PDM_CHANNELS ( \
	(PDM_CH0_EN ? PDM_MASK_CHANNEL_0 : 0) | \
	(PDM_CH1_EN ? PDM_MASK_CHANNEL_1 : 0) | \
	(PDM_CH2_EN ? PDM_MASK_CHANNEL_2 : 0) | \
	(PDM_CH3_EN ? PDM_MASK_CHANNEL_3 : 0) | \
	(PDM_CH4_EN ? PDM_MASK_CHANNEL_4 : 0) | \
	(PDM_CH5_EN ? PDM_MASK_CHANNEL_5 : 0) | \
	(PDM_CH6_EN ? PDM_MASK_CHANNEL_6 : 0) | \
	(PDM_CH7_EN ? PDM_MASK_CHANNEL_7 : 0))

#if DT_NODE_EXISTS(DT_NODELABEL(lppdm)) && \
	DT_SAME_NODE(DT_ALIAS(test_pdm), DT_NODELABEL(lppdm))
#define PDMTEST lppdm
#define PDM_STANDBY_KEEPS_CONTEXT 1
#elif DT_NODE_EXISTS(DT_NODELABEL(pdm)) && \
	DT_SAME_NODE(DT_ALIAS(test_pdm), DT_NODELABEL(pdm))
#define PDMTEST pdm
#define PDM_STANDBY_KEEPS_CONTEXT 0
#else
#error "test-pdm alias must point to pdm or lppdm (use -S pdm or -S lppdm)"
#endif

#if !IS_ENABLED(CONFIG_TEST_PDM_MONO_CH) && \
	!IS_ENABLED(CONFIG_TEST_PDM_STEREO_CH) && \
	!IS_ENABLED(CONFIG_TEST_PDM_MULTI_CH)
#error "No PDM channel group selected (TEST_PDM_MONO_CH/STEREO_CH/MULTI_CH)"
#endif

#if IS_ENABLED(CONFIG_TEST_PDM_MODE9)
#define PDM_MODE_HW PDM_MODE_ULTRASOUND_96_SAMPLING_RATE
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE8)
#define PDM_MODE_HW PDM_MODE_ULTRASOUND_4800_CLOCK_FRQ
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE7)
#define PDM_MODE_HW PDM_MODE_FULL_BANDWIDTH_AUDIO_3071_CLK_FRQ
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE6)
#define PDM_MODE_HW PDM_MODE_FULL_BANDWIDTH_AUDIO_2400_CLK_FRQ
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE5)
#define PDM_MODE_HW PDM_MODE_WIDE_BANDWIDTH_AUDIO_1536_CLK_FRQ
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE4)
#define PDM_MODE_HW PDM_MODE_HIGH_QUALITY_1024_CLK_FRQ
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE3)
#define PDM_MODE_HW PDM_MODE_HIGH_QUALITY_768_CLK_FRQ
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE2)
#define PDM_MODE_HW PDM_MODE_HIGH_QUALITY_512_CLK_FRQ
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE1)
#define PDM_MODE_HW PDM_MODE_STANDARD_VOICE_512_CLK_FRQ
#else
#error "No PDM mode selected (pass -DCONFIG_TEST_PDM_MODE1=y .. MODE9=y)"
#endif

#define PCMJ_BLOCK_SIZE   (40000)

#if IS_ENABLED(CONFIG_RTSS_HE)
#define MEM_SLAB_NUM_BLOCKS  3
#elif IS_ENABLED(CONFIG_RTSS_HP)
#define MEM_SLAB_NUM_BLOCKS  4
#else
#error "Set CONFIG_RTSS_HE or CONFIG_RTSS_HP"
#endif

#define SAMPLE_BIT_WIDTH 16
#define TIMEOUT          5000	/* dmic_read timeout in milliseconds */

#define CH0_PHASE             0x00000003
#define CH0_GAIN              0x00000013
#define CH1_PHASE             0x0000001F
#define CH1_GAIN              0x0000000D
#define CH2_PHASE             0x00000003
#define CH2_GAIN              0x00000013
#define CH3_PHASE             0x0000001F
#define CH3_GAIN              0x0000000D
#define CH4_PHASE             0x0000001F
#define CH4_GAIN              0x0000000D
#define CH5_PHASE             0x00000003
#define CH5_GAIN              0x00000013
#define CH6_PHASE             0x0000001F
#define CH6_GAIN              0x0000000D
#define CH7_PHASE             0x00000003
#define CH7_GAIN              0x00000013

#endif /* ALIF_TEST_PDM_H_ */
