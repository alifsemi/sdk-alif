/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/audio/dmic.h>
#include <zephyr/drivers/pdm/pdm_alif.h>
#include <string.h>
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>
#define PDM	DT_ALIAS(test_pdm)
/* The number of channels tested - changes with the test case */
#if CONFIG_TEST_PDM_MULTI_CH
	#define NUM_CHANNELS (4)
#elif CONFIG_TEST_PDM_MONO_CH
	#define NUM_CHANNELS (1)
#else
/* The number of channels tested - changes with the test case */
	#define NUM_CHANNELS (2)
#endif
/*
 * The list of channels to test
 * The number of channels should match the PDM_CHANNELS
 */
#if CONFIG_TEST_PDM_MULTI_CH
	#define PDM_CHANNELS (PDM_MASK_CHANNEL_4 | PDM_MASK_CHANNEL_5 | \
							PDM_MASK_CHANNEL_6 | PDM_MASK_CHANNEL_7)
#elif CONFIG_TEST_PDM_MONO_CH
	#if CONFIG_TEST_PDM_CH0
		#define PDM_CHANNELS PDM_MASK_CHANNEL_0
	#elif CONFIG_TEST_PDM_CH1
		#define PDM_CHANNELS PDM_MASK_CHANNEL_1
	#elif CONFIG_TEST_PDM_CH2
		#define PDM_CHANNELS PDM_MASK_CHANNEL_2
	#elif CONFIG_TEST_PDM_CH3
		#define PDM_CHANNELS PDM_MASK_CHANNEL_3
	#elif CONFIG_TEST_PDM_CH5
		#define PDM_CHANNELS PDM_MASK_CHANNEL_5
	#elif CONFIG_TEST_PDM_CH6
		#define PDM_CHANNELS PDM_MASK_CHANNEL_6
	#elif CONFIG_TEST_PDM_CH7
		#define PDM_CHANNELS PDM_MASK_CHANNEL_7
	#else
		#define PDM_CHANNELS PDM_MASK_CHANNEL_4
	#endif
#else
	#if (CONFIG_TEST_PDM_CH0 && CONFIG_TEST_PDM_CH1)
		#define PDM_CHANNELS (PDM_MASK_CHANNEL_0 | PDM_MASK_CHANNEL_1)
	#elif (CONFIG_TEST_PDM_CH2 && CONFIG_TEST_PDM_CH3)
		#define PDM_CHANNELS (PDM_MASK_CHANNEL_2 | PDM_MASK_CHANNEL_3)
	#elif (CONFIG_TEST_PDM_CH6 && CONFIG_TEST_PDM_CH7)
		#define PDM_CHANNELS (PDM_MASK_CHANNEL_6 | PDM_MASK_CHANNEL_7)
	#else
		#define PDM_CHANNELS (PDM_MASK_CHANNEL_4 | PDM_MASK_CHANNEL_5)
#endif
#endif
/*
 * Driver will allocate blocks from this slab to receive audio data into them.
 * Application, after getting a given block from the driver and processing its
 * data, needs to free that block.
 */
#define PCMJ_BLOCK_SIZE   (50000)
	#if defined(CONFIG_RTSS_HP)
		#define MEM_SLAB_NUM_BLOCKS  3
	#elif defined(CONFIG_RTSS_HE)
		#define MEM_SLAB_NUM_BLOCKS  4
#endif
#define DATA_BLOCK_COUNT (1)
/* size of buffer where the whole data is stored */
#define DATA_SIZE        (PCMJ_BLOCK_SIZE * MEM_SLAB_NUM_BLOCKS)
#define SAMPLE_BIT_WIDTH 16
#define CHANNEL_0  0
#define CHANNEL_1  1
#define CHANNEL_2  2
#define CHANNEL_3  3
#define CHANNEL_4  4
#define CHANNEL_5  5
#define CHANNEL_6  6
#define CHANNEL_7  7
#define TIMEOUT				5000
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
#define CH4_PHASE			0x0000001F
#define CH4_GAIN			0x0000000D
#define CH4_PEAK_DETECT_TH		0x00060002
#define CH4_PEAK_DETECT_ITV		0x0004002D
/* PDM Channel 5 configurations */
#define CH5_PHASE			0x00000003
#define CH5_GAIN			0x00000013
#define CH5_PEAK_DETECT_TH		0x00060002
#define CH5_PEAK_DETECT_ITV		0x00020027
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
