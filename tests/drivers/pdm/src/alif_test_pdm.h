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

/*
 * -------- Compile-time unique ZTEST name --------
 * Suite: &pdm → test_pdm, &lppdm → test_lppdm.
 * Case:   <suite>_<group>_mode<N>_ch[_<n>...]
 * Examples: test_pdm_multi_mode6_ch_0_1_4_5, test_lppdm_mono_mode1_ch_3
 */
#define PDM_CAT_(a, b) a##b
#define PDM_CAT(a, b) PDM_CAT_(a, b)

#if DT_NODE_EXISTS(DT_NODELABEL(lppdm)) && \
    DT_SAME_NODE(DT_ALIAS(test_pdm), DT_NODELABEL(lppdm))
#define PDM_SUITE_NAME test_lppdm
#elif DT_NODE_EXISTS(DT_NODELABEL(pdm)) && \
    DT_SAME_NODE(DT_ALIAS(test_pdm), DT_NODELABEL(pdm))
#define PDM_SUITE_NAME test_pdm
#else
#error "test-pdm alias must point to pdm or lppdm (use -S pdm or -S lppdm)"
#endif

#if IS_ENABLED(CONFIG_TEST_PDM_MONO_CH)
	#define PDM_GROUP_TAG mono
#elif IS_ENABLED(CONFIG_TEST_PDM_STEREO_CH)
	#define PDM_GROUP_TAG stereo
#elif IS_ENABLED(CONFIG_TEST_PDM_MULTI_CH)
	#define PDM_GROUP_TAG multi
#else
	#error "No PDM channel group selected (TEST_PDM_MONO_CH/STEREO_CH/MULTI_CH)"
#endif

#if IS_ENABLED(CONFIG_TEST_PDM_MODE9)
	#define PDM_MODE_TAG mode9
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE8)
	#define PDM_MODE_TAG mode8
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE7)
	#define PDM_MODE_TAG mode7
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE6)
	#define PDM_MODE_TAG mode6
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE5)
	#define PDM_MODE_TAG mode5
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE4)
	#define PDM_MODE_TAG mode4
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE3)
	#define PDM_MODE_TAG mode3
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE2)
	#define PDM_MODE_TAG mode2
#elif IS_ENABLED(CONFIG_TEST_PDM_MODE1)
	#define PDM_MODE_TAG mode1
#else
	#error "No PDM mode selected (pass -DCONFIG_TEST_PDM_MODE1=y .. MODE9=y)"
#endif

#if IS_ENABLED(CONFIG_TEST_PDM_CH0)
	#define PDM_CH0_TAG _0
#else
	#define PDM_CH0_TAG
#endif
#if IS_ENABLED(CONFIG_TEST_PDM_CH1)
	#define PDM_CH1_TAG _1
#else
	#define PDM_CH1_TAG
#endif
#if IS_ENABLED(CONFIG_TEST_PDM_CH2)
	#define PDM_CH2_TAG _2
#else
	#define PDM_CH2_TAG
#endif
#if IS_ENABLED(CONFIG_TEST_PDM_CH3)
	#define PDM_CH3_TAG _3
#else
	#define PDM_CH3_TAG
#endif
#if IS_ENABLED(CONFIG_TEST_PDM_CH4)
	#define PDM_CH4_TAG _4
#else
	#define PDM_CH4_TAG
#endif
#if IS_ENABLED(CONFIG_TEST_PDM_CH5)
	#define PDM_CH5_TAG _5
#else
	#define PDM_CH5_TAG
#endif
#if IS_ENABLED(CONFIG_TEST_PDM_CH6)
	#define PDM_CH6_TAG _6
#else
	#define PDM_CH6_TAG
#endif
#if IS_ENABLED(CONFIG_TEST_PDM_CH7)
	#define PDM_CH7_TAG _7
#else
	#define PDM_CH7_TAG
#endif

#define PDM_TEST_NAME \
	PDM_CAT(PDM_CAT(PDM_CAT(PDM_CAT(PDM_CAT(PDM_CAT(PDM_CAT( \
	PDM_CAT(PDM_CAT(PDM_CAT(PDM_CAT(PDM_CAT(PDM_CAT( \
	PDM_SUITE_NAME, _), PDM_GROUP_TAG), _), PDM_MODE_TAG), _ch), \
	PDM_CH0_TAG), PDM_CH1_TAG), PDM_CH2_TAG), PDM_CH3_TAG), \
	PDM_CH4_TAG), PDM_CH5_TAG), PDM_CH6_TAG), PDM_CH7_TAG)


#define PCMJ_BLOCK_SIZE   (40000)

#if IS_ENABLED(CONFIG_RTSS_HE)
#define MEM_SLAB_NUM_BLOCKS  3
#elif IS_ENABLED(CONFIG_RTSS_HP)
#define MEM_SLAB_NUM_BLOCKS  4
#else
#error "Set CONFIG_RTSS_HE or CONFIG_RTSS_HP"
#endif

/* size of buffer where the whole data is stored */
#define DATA_SIZE        (PCMJ_BLOCK_SIZE * MEM_SLAB_NUM_BLOCKS)
#define SAMPLE_BIT_WIDTH 16
#define TIMEOUT				5000	/* dmic_read timeout in milliseconds */

struct pdm_ch_hw_params {
	uint32_t phase;
	uint32_t gain;
	uint32_t peak_th;
	uint32_t peak_itv;
};

/* PDM Channel 0 configurations */
#define CH0_PHASE             0x00000003
#define CH0_GAIN              0x00000013
#define CH0_PEAK_DETECT_TH    0x00060002
#define CH0_PEAK_DETECT_ITV   0x00020027
/* PDM Channel 1 configurations */
#define CH1_PHASE             0x0000001F
#define CH1_GAIN              0x0000000D
#define CH1_PEAK_DETECT_TH    0x00060002
#define CH1_PEAK_DETECT_ITV   0x0004002D
/* PDM Channel 2 configurations */
#define CH2_PHASE             0x00000003
#define CH2_GAIN              0x00000013
#define CH2_PEAK_DETECT_TH    0x00060002
#define CH2_PEAK_DETECT_ITV   0x00020027
/* PDM Channel 3 configurations */
#define CH3_PHASE             0x0000001F
#define CH3_GAIN              0x0000000D
#define CH3_PEAK_DETECT_TH    0x00060002
#define CH3_PEAK_DETECT_ITV   0x0004002D
/* PDM Channel 4 configurations */
#define CH4_PHASE			  0x0000001F
#define CH4_GAIN			  0x0000000D
#define CH4_PEAK_DETECT_TH	  0x00060002
#define CH4_PEAK_DETECT_ITV	  0x0004002D
/* PDM Channel 5 configurations */
#define CH5_PHASE			  0x00000003
#define CH5_GAIN			  0x00000013
#define CH5_PEAK_DETECT_TH	  0x00060002
#define CH5_PEAK_DETECT_ITV	  0x00020027
/* PDM Channel 6 configurations */
#define CH6_PHASE             0x0000001F
#define CH6_GAIN              0x0000000D
#define CH6_PEAK_DETECT_TH    0x00060002
#define CH6_PEAK_DETECT_ITV   0x0004002D
/* PDM Channel 7 configurations */
#define CH7_PHASE             0x00000003
#define CH7_GAIN              0x00000013
#define CH7_PEAK_DETECT_TH    0x00060002
#define CH7_PEAK_DETECT_ITV   0x00020027

#endif /* ALIF_TEST_PDM_H_ */