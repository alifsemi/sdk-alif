/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#ifndef TEST_SPI_COMMON_H_
#define TEST_SPI_COMMON_H_

#include <stdbool.h>
#include <zephyr/logging/log.h>
#include <zephyr/ztest.h>
#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util_macro.h>
#include <string.h>
#include <soc_common.h>

/* ---------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------------
 */
#define SPI_FREQ_MHZ                CONFIG_TEST_SPI_FREQUENCY

/* Match sample app for DMA compatibility */
#define SPI_TEST_BUFF_SIZE          200U
#define SPI_TEST_BUFF_SIZE_PERF     4096U

/* DMA-compatible word size - DMA controllers
 * typically only support 8-bit reliably
 */
#define SPI_TEST_DMA_WORD_SIZE      8U     /* Use 8-bit for DMA transfers */

/* Convenience macro to avoid repeating the long ternary in every test file */
#define SPI_TEST_WORD_SIZE \
	SPI_WORD_SET(IS_ENABLED(CONFIG_TEST_SPI_USE_DMA) ? \
		     SPI_TEST_DMA_WORD_SIZE : transceive_word)

#define SPI_TEST_STACKSIZE          (1024U * 20U)
#define SPI_TEST_MAX_FREQUENCY      25U
#define SPI_TEST_NUM_TRANSFERS      1U

#define SPI_TEST_SLEEPTIME_MS       500U
#define SPI_TEST_PRIORITY_CONTROLLER    7
#define SPI_TEST_PRIORITY_TARGET        6

#define SPI_TEST_CS_DELAY_US        100U
#define SPI_TEST_TARGET_HEADSTART_US 10000U  /* Increased for DMA timing */

/* Devicetree node aliases */
#define SPI_CONTROLLER_NODE         DT_ALIAS(controller_spi)
#define SPI_TARGET_NODE             DT_ALIAS(target_spi)

/* DMA event router control bits */
#ifndef DMA_CTRL_ACK_TYPE_Pos
#define DMA_CTRL_ACK_TYPE_Pos       16U
#endif
#ifndef DMA_CTRL_ENA
#define DMA_CTRL_ENA                (1U << 4)
#endif
#ifndef HE_DMA_SEL_LPSPI_Pos
#define HE_DMA_SEL_LPSPI_Pos        4U
#endif
#ifndef HE_DMA_SEL_LPSPI_Msk
#define HE_DMA_SEL_LPSPI_Msk        (0x3U << HE_DMA_SEL_LPSPI_Pos)
#endif

/* ---------------------------------------------------------------------------
 * Shared data (defined in test_spi_common.c)
 * ---------------------------------------------------------------------------
 */
K_THREAD_STACK_DECLARE(controller_stack, SPI_TEST_STACKSIZE);
extern struct k_thread controller_thread_data;

K_THREAD_STACK_DECLARE(target_stack, SPI_TEST_STACKSIZE);
extern struct k_thread target_thread_data;

#if !IS_ENABLED(CONFIG_TEST_SPI_PERFORMANCE)
extern uint32_t ctrl_txdata[SPI_TEST_BUFF_SIZE];
extern uint32_t tgt_txdata[SPI_TEST_BUFF_SIZE];
extern uint32_t ctrl_rxdata[SPI_TEST_BUFF_SIZE];
extern uint32_t tgt_rxdata[SPI_TEST_BUFF_SIZE];
#else
extern uint32_t ctrl_txdata[SPI_TEST_BUFF_SIZE_PERF];
extern uint32_t ctrl_rxdata[SPI_TEST_BUFF_SIZE_PERF];
extern uint32_t tgt_txdata[SPI_TEST_BUFF_SIZE_PERF];
extern uint32_t tgt_rxdata[SPI_TEST_BUFF_SIZE_PERF];
#endif

extern atomic_t err_count;
extern atomic_val_t transceive_word;
extern uint32_t spi_test_op_flags;  /* CPOL/CPHA*/
extern atomic_t spi_test_async_mode; /* 0 = sync, 1 = async */

/* Performance error counters */
extern uint32_t ctrl_ret_tx;
extern uint32_t tgt_ret_tx;
extern uint32_t ctrl_ret_rx;
extern uint32_t tgt_ret_rx;

/* ---------------------------------------------------------------------------
 * Utility functions (test_spi_common.c)
 * ---------------------------------------------------------------------------
 */
/* Generate a pattern seed that varies with the current word size */
static inline uint16_t spi_test_pattern(uint16_t seed)
{
	return (uint16_t)(seed ^ ((uint16_t)transceive_word << 8));
}

/* 32-bit seed derived from the word-size-aware pattern */
static inline uint32_t spi_test_seed32(uint32_t base)
{
	uint16_t p = spi_test_pattern((uint16_t)base);

	return ((uint32_t)p << 16) | (uint32_t)(~p & 0xFFFFU);
}

int spi_test_prepare_data(uint32_t *data, uint16_t pattern, uint32_t count);
int spi_test_reset_buffer(uint32_t *buffer, size_t count);
uint32_t spi_test_word_operation(uint32_t base_op, uint32_t word_size);
bool spi_test_device_ready(const struct device *dev, const char *role);
void spi_test_controller_cs_init(struct spi_cs_control *cs);

/* Inline helper: create spi_config with CS pre-configured from DT */
static inline struct spi_config spi_test_config(uint32_t operation,
						  uint32_t frequency)
{

#if DT_NODE_HAS_PROP(SPI_CONTROLLER_NODE, cs_gpios)
	struct spi_cs_control cs = {
		.gpio = GPIO_DT_SPEC_GET(SPI_CONTROLLER_NODE, cs_gpios),
		.delay = SPI_TEST_CS_DELAY_US,
	};
#else
	struct spi_cs_control cs = {0};
#endif
	return (struct spi_config){
		.frequency = frequency,
		.operation = operation,
		.slave = 0,
		.cs = cs,
	};
}
int spi_test_transceive(const struct device *dev,
			const struct spi_config *cnfg,
			const struct spi_buf_set *tx_set,
			const struct spi_buf_set *rx_set,
			bool force_reconfig);
int spi_test_transceive_async(const struct device *dev,
			      const struct spi_config *cnfg,
			      const struct spi_buf_set *tx_set,
			      const struct spi_buf_set *rx_set,
			      bool force_reconfig);
int spi_test_controller_receive_and_check(const struct device *dev,
				      const struct spi_config *cnfg,
				      const uint32_t *expected_tx,
				      uint32_t *rx_buffer,
				      size_t word_count);
void spi_test_log_first_mismatch(const uint32_t *expected,
				 const uint32_t *actual,
				 size_t word_count,
				 const char *context);

/* Ztest before-function: called by ZTEST_SUITE in each test file */
void test_before_func(void *fixture);

/* ---------------------------------------------------------------------------
 * Thread entry functions (test_spi_threads.c)
 * ---------------------------------------------------------------------------
 */

/* MOSI: controller transmit, target receive */
void controller_spi_transmit(void *p1, void *p2, void *p3);
void target_spi_receive(void *p1, void *p2, void *p3);

/* MISO: controller receive, target transmit */
void controller_receive(void *p1, void *p2, void *p3);
void target_send(void *p1, void *p2, void *p3);

/* Transceive: full-duplex controller and target */
void controller_spi(void *p1, void *p2, void *p3);
void target_spi(void *p1, void *p2, void *p3);

#endif /* TEST_SPI_COMMON_H_ */
