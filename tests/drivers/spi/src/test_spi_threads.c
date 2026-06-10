/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_spi_threads.h"

LOG_MODULE_REGISTER(alif_spi_threads, LOG_LEVEL_INF);

static K_SEM_DEFINE(perf_sem_target_ready, 0, 1);
static K_SEM_DEFINE(perf_sem_controller_done, 0, 1);

static inline k_timeout_t perf_sync_timeout(uint32_t freq_hz)
{
	return spi_test_transfer_timeout(SPI_TEST_BUFF_SIZE_PERF * sizeof(uint32_t),
					 freq_hz);
}

#if IS_ENABLED(CONFIG_TEST_SPI_PERFORMANCE)
#define SPI_THREAD_TIMEOUT_FREQ_MIN ((uint32_t)CONFIG_TEST_SPI_PERF_FREQ_MIN_MHZ)
#define SPI_THREAD_TIMEOUT_FREQ_MAX ((uint32_t)CONFIG_TEST_SPI_PERF_FREQ_MAX_MHZ)
#define SPI_THREAD_TIMEOUT_FREQ_STEP ((uint32_t)CONFIG_TEST_SPI_PERF_FREQ_STEP_MHZ)
#define SPI_THREAD_TIMEOUT_BYTES (SPI_TEST_BUFF_SIZE_PERF * sizeof(uint32_t))
#else
#define SPI_THREAD_TIMEOUT_BYTES (SPI_TEST_BUFF_SIZE * sizeof(uint32_t))
#endif

static k_timeout_t spi_threads_join_timeout(void)
{
#if IS_ENABLED(CONFIG_TEST_SPI_PERFORMANCE)
	uint64_t timeout_ms = 0U;

	for (uint32_t freq = SPI_THREAD_TIMEOUT_FREQ_MIN;
	     freq <= SPI_THREAD_TIMEOUT_FREQ_MAX;) {
		timeout_ms += spi_test_transfer_timeout_ms(
			SPI_THREAD_TIMEOUT_BYTES, freq * SPI_FREQ_MHZ);

		if (freq >= SPI_THREAD_TIMEOUT_FREQ_MAX) {
			break;
		}
		freq += SPI_THREAD_TIMEOUT_FREQ_STEP;
		if (freq > SPI_THREAD_TIMEOUT_FREQ_MAX) {
			freq = SPI_THREAD_TIMEOUT_FREQ_MAX;
		}
	}

	return K_MSEC((timeout_ms * 2U) + 2000U);
#else
	return K_MSEC((spi_test_transfer_timeout_ms(SPI_THREAD_TIMEOUT_BYTES,
						    SPI_FREQ_MHZ) * 2U) +
		      2000U);
#endif
}

/* Controller-side sync/async selector for thread-based tests.
 * Target-side always stays sync — the slave must have its RX armed
 * synchronously before the controller clocks data.
 */
static int controller_xfer(const struct device *dev,
			   const struct spi_config *cfg,
			   const struct spi_buf_set *tx,
			   const struct spi_buf_set *rx)
{
	LOG_INF("ctrl_xfer: %s op=0x%08x ti=%d ws=%u freq=%u",
		dev->name, cfg->operation,
		!!(cfg->operation & SPI_FRAME_FORMAT_TI),
		SPI_WORD_SIZE_GET(cfg->operation),
		cfg->frequency);
	/* Always force reconfig: local cnfg stack addresses can be reused
	 * across tests, causing spi_context_configured() to falsely skip
	 * reconfiguration when operation flags change (e.g. Motorola -> TI).
	 */
	if (atomic_get(&spi_test_async_mode)) {
		return spi_test_transceive_async(dev, cfg, tx, rx, true);
	}
	return spi_test_transceive(dev, cfg, tx, rx, true);
}

/* Target-side transceive. Always sync, but must also force reconfig so the
 * slave's CTRLR0 (frame format, mode, word size) is reprogrammed when a new
 * config is supplied.  The local cnfg lives on the static target thread stack,
 * so its address is reused across scenarios; without force_reconfig the driver
 * sees the same pointer and skips reconfiguration (e.g. stays Motorola when the
 * test selected TI).
 */
static int target_xfer(const struct device *dev,
		       const struct spi_config *cfg,
		       const struct spi_buf_set *tx,
		       const struct spi_buf_set *rx)
{
	LOG_INF("tgt_xfer:  %s op=0x%08x ti=%d ws=%u freq=%u",
		dev->name, cfg->operation,
		!!(cfg->operation & SPI_FRAME_FORMAT_TI),
		SPI_WORD_SIZE_GET(cfg->operation),
		cfg->frequency);
	return spi_test_transceive(dev, cfg, tx, rx, true);
}


void run_spi_test_threads(void (*controller_func)(void *, void *, void *),
			  void (*target_func)(void *, void *, void *))
{
	int higher_prio =
		MIN(SPI_TEST_PRIORITY_CONTROLLER, SPI_TEST_PRIORITY_TARGET);
	int lower_prio =
		MAX(SPI_TEST_PRIORITY_CONTROLLER, SPI_TEST_PRIORITY_TARGET);
	int controller_prio;
	int target_prio;
	k_timeout_t join_timeout = spi_threads_join_timeout();
	int ret;

	/* The slave must arm its hardware BEFORE the master starts clocking,
	 * regardless of PIO or DMA mode.  Give the target thread higher
	 * scheduling priority so that after it calls k_sem_give() it
	 * continues uninterrupted into spi_transceive(), arms the slave
	 * DMA/FIFO, and only then blocks — allowing the controller thread
	 * (lower priority) to run and start the transfer.
	 */
	controller_prio = lower_prio;
	target_prio = higher_prio;

	TC_PRINT("Creating SPI test threads (controller: %p, target: %p)\n",
		 controller_func, target_func);
	TC_PRINT("Thread scheduling: mode=%s controller_prio=%d target_prio=%d\n",
		 IS_ENABLED(CONFIG_TEST_SPI_USE_DMA) ? "dma" : "pio",
		 controller_prio, target_prio);

	k_sem_reset(&perf_sem_target_ready);
	k_sem_reset(&perf_sem_controller_done);

	k_tid_t tids = k_thread_create(&target_thread_data, target_stack,
				       SPI_TEST_STACKSIZE, target_func,
				       (void *)1, NULL, NULL,
				       target_prio, 0, K_FOREVER);
	zassert_not_null(tids, "Failed to create target thread");

	k_tid_t tidm = k_thread_create(
		&controller_thread_data, controller_stack,
		SPI_TEST_STACKSIZE, controller_func,
		(void *)1, NULL, NULL,
		controller_prio, 0, K_FOREVER);
	zassert_not_null(tidm, "Failed to create controller thread");

	LOG_DBG("Creating SPI test threads (controller: %p, target: %p)",
		 &controller_thread_data, &target_thread_data);

	LOG_DBG("Starting target thread");
	k_thread_start(&target_thread_data);

	k_msleep(100);

	LOG_DBG("Starting controller thread");
	k_thread_start(&controller_thread_data);

	LOG_DBG("Waiting for threads to complete");
	ret = k_thread_join(&controller_thread_data, join_timeout);
	if (ret != 0) {
		LOG_ERR("Controller thread did not finish cleanly: %d", ret);
		k_thread_abort(&controller_thread_data);
		k_thread_abort(&target_thread_data);
		zassert_equal(ret, 0, "Controller thread join failed: %d", ret);
	}

	ret = k_thread_join(&target_thread_data, join_timeout);
	if (ret != 0) {
		LOG_ERR("Target thread did not finish cleanly: %d", ret);
		k_thread_abort(&target_thread_data);
		zassert_equal(ret, 0, "Target thread join failed: %d", ret);
	}

	TC_PRINT("SPI test threads completed (errors: %ld)\n",
		 atomic_get(&err_count));
	zassert_equal(atomic_get(&err_count), 0,
		      "Test case failed with %ld errors",
		      atomic_get(&err_count));
}

#if !IS_ENABLED(CONFIG_TEST_SPI_PERFORMANCE)

static int func_controller_transmit(const struct device *dev)
{
	int length = SPI_TEST_BUFF_SIZE * sizeof(uint32_t);
	struct spi_buf tx_buf = { .buf = ctrl_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = ctrl_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

	struct spi_config cnfg = spi_test_config(
		SPI_OP_MODE_MASTER | spi_test_op_flags |
		SPI_TEST_WORD_SIZE,
		1 * SPI_FREQ_MHZ);

	LOG_DBG("Controller: Starting simple transceive");
	int ret = controller_xfer(dev, &cnfg, &tx_set, &rx_set);

	if (ret >= 0) {
		TC_PRINT("Controller transceive success: %d\n", ret);
	} else {
		LOG_ERR("Controller transceive failed: %d", ret);
		atomic_inc(&err_count);
	}

	return ret;
}

static int func_target_receive(const struct device *dev)
{
	int length = SPI_TEST_BUFF_SIZE * sizeof(uint32_t);
	struct spi_buf tx_buf = { .buf = tgt_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = tgt_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

	struct spi_config cnfg = {
		.frequency = 1 * SPI_FREQ_MHZ,
		.operation = SPI_OP_MODE_SLAVE | spi_test_op_flags |
			SPI_TEST_WORD_SIZE,
		.slave = 0,
	};

	LOG_DBG("Target: Starting simple transceive");
	int ret = target_xfer(dev, &cnfg, &tx_set, &rx_set);

	if (ret >= 0) {
		TC_PRINT("Target transceive success: %d\n", ret);
		ret = memcmp(ctrl_txdata, tgt_rxdata, length);
		if (ret) {
			LOG_ERR("Target RX / Controller TX data mismatch: %d", ret);
			atomic_inc(&err_count);
		} else {
			TC_PRINT("Target RX / Controller TX data match\n");
		}
	} else {
		LOG_ERR("Target transceive failed: %d", ret);
		atomic_inc(&err_count);
	}

	return ret;
}

void controller_spi_transmit(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_CONTROLLER_NODE);

	zassert_true(spi_test_device_ready(dev, "Controller"),
		     "Controller device not ready");
	TC_PRINT("MOSI: controller TX (word %ld bits)\n",
		 transceive_word);

	func_controller_transmit(dev);
}

void target_spi_receive(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_TARGET_NODE);

	zassert_true(spi_test_device_ready(dev, "Target"),
		     "Target device not ready");
	TC_PRINT("MOSI: target RX (word %ld bits)\n",
		 transceive_word);
	func_target_receive(dev);
}

int func_target_transmit(const struct device *dev)
{
	int length = SPI_TEST_BUFF_SIZE * sizeof(uint32_t);
	struct spi_buf tx_buf = { .buf = tgt_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = tgt_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

	struct spi_config cnfg = {
		.frequency = 1 * SPI_FREQ_MHZ,
		.operation = SPI_OP_MODE_SLAVE | spi_test_op_flags |
			SPI_TEST_WORD_SIZE,
		.slave = 0,
	};

	LOG_DBG("Target: Starting simple transceive (word size: %ld)",
		IS_ENABLED(CONFIG_TEST_SPI_USE_DMA) ?
		SPI_TEST_DMA_WORD_SIZE : transceive_word);
	int ret = target_xfer(dev, &cnfg, &tx_set, &rx_set);

	if (ret >= 0) {
		TC_PRINT("Target transceive success: %d\n", ret);
	} else {
		LOG_ERR("Target transceive failed: %d", ret);
		atomic_inc(&err_count);
	}

	return ret;
}

int func_controller_receive(const struct device *dev)
{
	int length = SPI_TEST_BUFF_SIZE * sizeof(uint32_t);
	struct spi_buf tx_buf = { .buf = ctrl_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = ctrl_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

	struct spi_config cnfg = spi_test_config(
		SPI_OP_MODE_MASTER | spi_test_op_flags |
		SPI_TEST_WORD_SIZE,
		1 * SPI_FREQ_MHZ);

	LOG_DBG("Controller: Starting simple transceive");
	int ret = controller_xfer(dev, &cnfg, &tx_set, &rx_set);

	if (ret < 0) {
		LOG_ERR("Controller transceive failed: %d", ret);
		atomic_inc(&err_count);
	} else {
		int cmp_ret = memcmp(tgt_txdata, ctrl_rxdata, length);

		if (cmp_ret) {
			LOG_ERR("RX: %08x %08x %08x %08x %08x %08x %08x %08x",
				ctrl_rxdata[0], ctrl_rxdata[1],
				ctrl_rxdata[2], ctrl_rxdata[3],
				ctrl_rxdata[4], ctrl_rxdata[5],
				ctrl_rxdata[6], ctrl_rxdata[7]);
			LOG_ERR("TX: %08x %08x %08x %08x %08x %08x %08x %08x",
				tgt_txdata[0], tgt_txdata[1],
				tgt_txdata[2], tgt_txdata[3],
				tgt_txdata[4], tgt_txdata[5],
				tgt_txdata[6], tgt_txdata[7]);
			LOG_ERR("Controller RX / Target TX data mismatch: %d", cmp_ret);
			atomic_inc(&err_count);
		} else {
			TC_PRINT("Controller RX / Target TX data match\n");
		}
	}

	return ret;
}

void controller_receive(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_CONTROLLER_NODE);

	zassert_true(spi_test_device_ready(dev, "Controller"),
		     "Controller device not ready");
	TC_PRINT("MISO: controller RX (word %ld bits)\n",
		 transceive_word);

	func_controller_receive(dev);
}

void target_send(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_TARGET_NODE);

	zassert_true(spi_test_device_ready(dev, "Target"),
		     "Target device not ready");
	TC_PRINT("MISO: target TX (word %ld bits)\n",
		 transceive_word);
	func_target_transmit(dev);
}

static int func_controller_transceive(const struct device *dev)
{
	int length = SPI_TEST_BUFF_SIZE * sizeof(uint32_t);

	struct spi_buf tx_buf = { .buf = ctrl_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = ctrl_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

	struct spi_config cnfg = spi_test_config(
		SPI_OP_MODE_MASTER | spi_test_op_flags |
		SPI_TEST_WORD_SIZE,
		1 * SPI_FREQ_MHZ);

	LOG_DBG("Controller: Starting simple transceive (word size: %ld)",
		IS_ENABLED(CONFIG_TEST_SPI_USE_DMA) ?
		SPI_TEST_DMA_WORD_SIZE : transceive_word);
	int ret = controller_xfer(dev, &cnfg, &tx_set, &rx_set);

	if (ret < 0) {
		LOG_ERR("Controller transceive failed: %d", ret);
		atomic_inc(&err_count);
		return ret;
	}

	LOG_DBG("Controller TX: %08x %08x %08x %08x %08x",
		ctrl_txdata[0], ctrl_txdata[1], ctrl_txdata[2],
		ctrl_txdata[3], ctrl_txdata[4]);
	LOG_DBG("Controller RX: %08x %08x %08x %08x %08x",
		ctrl_rxdata[0], ctrl_rxdata[1], ctrl_rxdata[2],
		ctrl_rxdata[3], ctrl_rxdata[4]);

	ret = memcmp(ctrl_rxdata, tgt_txdata, length);
	if (ret) {
		LOG_ERR("Controller RX / Target TX data mismatch: %d", ret);
		atomic_inc(&err_count);
	} else {
		TC_PRINT("Controller RX / Target TX data match\n");
	}

	return ret;
}

static int func_target_transceive(const struct device *dev)
{
	int length = SPI_TEST_BUFF_SIZE * sizeof(uint32_t);

	struct spi_buf tx_buf = { .buf = tgt_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = tgt_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };

	struct spi_config cnfg = {
		.frequency = 1 * SPI_FREQ_MHZ,
		.operation = SPI_OP_MODE_SLAVE | spi_test_op_flags |
			SPI_TEST_WORD_SIZE,
		.slave = 0,
	};

	LOG_DBG("Target: Starting simple transceive");
	int ret = target_xfer(dev, &cnfg, &tx_set, &rx_set);

	if (ret < 0) {
		LOG_ERR("Target transceive failed: %d", ret);
		atomic_inc(&err_count);
		return ret;
	}

	LOG_DBG("Target TX: %08x %08x %08x %08x %08x",
		tgt_txdata[0], tgt_txdata[1], tgt_txdata[2],
		tgt_txdata[3], tgt_txdata[4]);
	LOG_DBG("Target RX: %08x %08x %08x %08x %08x",
		tgt_rxdata[0], tgt_rxdata[1], tgt_rxdata[2],
		tgt_rxdata[3], tgt_rxdata[4]);

	ret = memcmp(ctrl_txdata, tgt_rxdata, length);
	if (ret) {
		LOG_ERR("Target RX / Controller TX data mismatch: %d", ret);
		atomic_inc(&err_count);
	} else {
		TC_PRINT("Target RX / Controller TX data match\n");
	}

	return ret;
}

void controller_spi(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_CONTROLLER_NODE);

	zassert_true(spi_test_device_ready(dev, "Controller"),
		     "Controller device not ready");

	func_controller_transceive(dev);
}

void target_spi(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_TARGET_NODE);

	zassert_true(spi_test_device_ready(dev, "Target"),
		     "Target device not ready");
	func_target_transceive(dev);
}

#else /* CONFIG_TEST_SPI_PERFORMANCE */

#define PERF_FREQ_MIN_MHZ  ((uint32_t)CONFIG_TEST_SPI_PERF_FREQ_MIN_MHZ)
#define PERF_FREQ_MAX_MHZ  ((uint32_t)CONFIG_TEST_SPI_PERF_FREQ_MAX_MHZ)
#define PERF_FREQ_STEP_MHZ ((uint32_t)CONFIG_TEST_SPI_PERF_FREQ_STEP_MHZ)

/* Advance to the next sweep point, always landing exactly on the
 * configured maximum as the final point even when it is not on a step
 * boundary.  Returns a value above the max to terminate the loop.
 */
static inline uint32_t perf_freq_next(uint32_t freq)
{
	uint32_t next;

	if (freq >= PERF_FREQ_MAX_MHZ) {
		return PERF_FREQ_MAX_MHZ + 1U;
	}

	next = freq + PERF_FREQ_STEP_MHZ;
	return (next > PERF_FREQ_MAX_MHZ) ? PERF_FREQ_MAX_MHZ : next;
}

/* The target thread (higher priority) signals perf_sem_target_ready
 * BEFORE it arms the slave hardware inside its blocking transceive
 * (CTRLR0, SSIENR, TX prefill, IMR unmask / DMA channel start all happen
 * inside target_xfer).  The slave configure path can yield (clock_control
 * mutex), which would let this lower-priority controller clock the first
 * words before the slave RX FIFO/DMA is armed, dropping leading words and
 * shifting the received data.  Wait a deterministic head-start so the
 * higher-priority target reaches its armed, blocked state first.  Placed
 * outside the timing window so it does not skew throughput numbers.
 */
static inline void perf_wait_target_armed(void)
{
	k_usleep(SPI_TEST_TARGET_HEADSTART_US);
}

/* SPI controller (spi1) source clock = ALIF_SPI_CLK = HCLK = AXI/2.
 * On E1/E7/E8 AXI = 400 MHz, so the SPI ssi_clk is 200 MHz.
 */
#define SPI_CTRL_SRC_CLK_HZ 200000000U

/* Mirror the DesignWare baud divider: integer divide, then the HW
 * forces an even SCKDV (LSB is cleared). Returns the real SCLK in Hz.
 */
static inline uint32_t perf_actual_sclk(uint32_t req_hz)
{
	uint32_t div = SPI_CTRL_SRC_CLK_HZ / req_hz;

	div &= ~1U;             /* HW rounds down to even */
	if (div == 0U) {
		div = 2U;       /* minimum divider */
	}
	return SPI_CTRL_SRC_CLK_HZ / div;
}

static int perf_controller_send(const struct device *dev)
{
	int length = SPI_TEST_BUFF_SIZE_PERF * sizeof(uint32_t);
	struct spi_buf tx_buf = { .buf = ctrl_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = ctrl_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };
	static struct spi_config cnfg[2];

	for (uint32_t freq = PERF_FREQ_MIN_MHZ;
	     freq <= PERF_FREQ_MAX_MHZ; freq = perf_freq_next(freq)) {
		cnfg[freq % 2] = spi_test_config(
			SPI_OP_MODE_MASTER | SPI_TEST_WORD_SIZE,
			freq * SPI_FREQ_MHZ);
		memset(ctrl_rxdata, 0, length);
		TC_PRINT("Controller req %u Hz -> actual SCLK %u Hz\n",
			 freq * SPI_FREQ_MHZ,
			 perf_actual_sclk(freq * SPI_FREQ_MHZ));
		if (k_sem_take(&perf_sem_target_ready,
			       perf_sync_timeout(freq * SPI_FREQ_MHZ)) != 0) {
			LOG_ERR("Controller TX: target ready timeout @%u Hz",
				freq * SPI_FREQ_MHZ);
			break;
		}
		perf_wait_target_armed();

		uint32_t cyc_start = k_cycle_get_32();
		int64_t time_start = k_uptime_get();

		int ret = controller_xfer(dev, &cnfg[freq % 2], &tx_set, &rx_set);

		k_sem_give(&perf_sem_controller_done);

		int64_t time_end = k_uptime_get();
		uint32_t cyc_end = k_cycle_get_32();

		if (ret == 0) {
			TC_PRINT("Controller TX @%u Hz: OK\n",
				 freq * SPI_FREQ_MHZ);
		} else {
			LOG_ERR("Controller TX @%u Hz failed: %d",
				freq * SPI_FREQ_MHZ, ret);
			ctrl_ret_tx++;
		}

		TC_PRINT("  cycles: %u, time: %llu ms\n",
			cyc_end - cyc_start,
			(unsigned long long)(time_end - time_start));
		k_msleep(1);
	}
	return 0;
}

static int perf_target_receive(const struct device *dev)
{
	int length = SPI_TEST_BUFF_SIZE_PERF * sizeof(uint32_t);
	struct spi_buf tx_buf = { .buf = tgt_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = tgt_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };
	static struct spi_config cnfg[2];

	for (uint32_t freq = PERF_FREQ_MIN_MHZ;
	     freq <= PERF_FREQ_MAX_MHZ; freq = perf_freq_next(freq)) {
		cnfg[freq % 2] = (struct spi_config){
			.operation = SPI_OP_MODE_SLAVE | spi_test_op_flags |
				SPI_TEST_WORD_SIZE,
			.frequency = freq * SPI_FREQ_MHZ,
			.slave = 0,
		};
		memset(tgt_rxdata, 0, length);
		k_sem_give(&perf_sem_target_ready);

		uint32_t cyc_start = k_cycle_get_32();
		int64_t time_start = k_uptime_get();

		int ret = target_xfer(dev, &cnfg[freq % 2], &tx_set, &rx_set);

		if (k_sem_take(&perf_sem_controller_done,
			       perf_sync_timeout(freq * SPI_FREQ_MHZ)) != 0) {
			LOG_ERR("Target RX: controller done timeout @%u Hz",
				freq * SPI_FREQ_MHZ);
			break;
		}

		int64_t time_end = k_uptime_get();
		uint32_t cyc_end = k_cycle_get_32();

		if (ret >= 0) {
			ret = memcmp(ctrl_txdata, tgt_rxdata, length);
			if (ret != 0) {
				LOG_ERR("Target RX @%u Hz: data mismatch "
					"(word %ld)", freq * SPI_FREQ_MHZ,
					transceive_word);
				tgt_ret_rx++;
			} else {
				TC_PRINT("Target RX @%u Hz: data match\n",
					 freq * SPI_FREQ_MHZ);
			}
		} else {
			LOG_ERR("Target RX @%u Hz failed: %d",
				freq * SPI_FREQ_MHZ, ret);
			tgt_ret_rx++;
		}

		TC_PRINT("  cycles: %u, time: %llu ms\n",
			cyc_end - cyc_start,
			(unsigned long long)(time_end - time_start));
		k_msleep(1);
	}
	return 0;
}

void controller_spi_transmit(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_CONTROLLER_NODE);

	zassert_true(spi_test_device_ready(dev, "Controller"),
		     "Controller device not ready");

	perf_controller_send(dev);
}

void target_spi_receive(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_TARGET_NODE);

	zassert_true(spi_test_device_ready(dev, "Target"),
		     "Target device not ready");

	perf_target_receive(dev);
}

static int perf_target_transmit(const struct device *dev)
{
	int length = SPI_TEST_BUFF_SIZE_PERF * sizeof(uint32_t);
	struct spi_buf tx_buf = { .buf = tgt_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = tgt_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };
	static struct spi_config cnfg[2];

	for (uint32_t freq = PERF_FREQ_MIN_MHZ;
	     freq <= PERF_FREQ_MAX_MHZ; freq = perf_freq_next(freq)) {
		cnfg[freq % 2] = (struct spi_config){
			.operation = SPI_OP_MODE_SLAVE | spi_test_op_flags |
				SPI_TEST_WORD_SIZE,
			.frequency = freq * SPI_FREQ_MHZ,
			.slave = 0,
		};
		memset(tgt_rxdata, 0, length);
		k_sem_give(&perf_sem_target_ready);

		uint32_t cyc_start = k_cycle_get_32();

		int64_t time_start = k_uptime_get();

		int ret = target_xfer(dev, &cnfg[freq % 2], &tx_set, &rx_set);

		if (k_sem_take(&perf_sem_controller_done,
			       perf_sync_timeout(freq * SPI_FREQ_MHZ)) != 0) {
			LOG_ERR("Target TX: controller done timeout @%u Hz",
				freq * SPI_FREQ_MHZ);
			break;
		}

		int64_t time_end = k_uptime_get();
		uint32_t cyc_end = k_cycle_get_32();

		if (ret >= 0) {
			TC_PRINT("Target TX @%u Hz: OK\n",
				 freq * SPI_FREQ_MHZ);
		} else {
			LOG_ERR("Target TX @%u Hz failed: %d",
				freq * SPI_FREQ_MHZ, ret);
			tgt_ret_tx++;
		}

		TC_PRINT("  cycles: %u, time: %llu ms\n",
			cyc_end - cyc_start,
			(unsigned long long)(time_end - time_start));
		k_msleep(1);
	}
	return 0;
}

static int perf_controller_receive(const struct device *dev)
{
	int length = SPI_TEST_BUFF_SIZE_PERF * sizeof(uint32_t);
	struct spi_buf tx_buf = { .buf = ctrl_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = ctrl_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };
	static struct spi_config cnfg[2];

	for (uint32_t freq = PERF_FREQ_MIN_MHZ;
	     freq <= PERF_FREQ_MAX_MHZ; freq = perf_freq_next(freq)) {
		cnfg[freq % 2] = spi_test_config(
			SPI_OP_MODE_MASTER | SPI_TEST_WORD_SIZE,
			freq * SPI_FREQ_MHZ);
		memset(ctrl_rxdata, 0, length);
		if (k_sem_take(&perf_sem_target_ready,
			       perf_sync_timeout(freq * SPI_FREQ_MHZ)) != 0) {
			LOG_ERR("Controller RX: target ready timeout @%u Hz",
				freq * SPI_FREQ_MHZ);
			break;
		}
		perf_wait_target_armed();

		uint32_t cyc_start = k_cycle_get_32();

		int64_t time_start = k_uptime_get();

		int ret = controller_xfer(dev, &cnfg[freq % 2], &tx_set, &rx_set);

		k_sem_give(&perf_sem_controller_done);

		int64_t time_end = k_uptime_get();

		uint32_t cyc_end = k_cycle_get_32();

		if (ret < 0) {
			LOG_ERR("Controller RX @%u Hz failed: %d",
				freq * SPI_FREQ_MHZ, ret);
			ctrl_ret_rx++;
		} else {
			ret = memcmp(tgt_txdata, ctrl_rxdata, length);

			if (ret == 0) {
				TC_PRINT("Controller RX @%u Hz: data match\n",
					 freq * SPI_FREQ_MHZ);
			} else {
				LOG_ERR("Controller RX @%u Hz: data mismatch "
					"(word %ld)", freq * SPI_FREQ_MHZ,
					transceive_word);
				spi_test_log_first_mismatch(
					tgt_txdata, ctrl_rxdata,
					SPI_TEST_BUFF_SIZE_PERF,
					"Controller RX perf / Target TX");
				ctrl_ret_rx++;
			}
		}

		TC_PRINT("  cycles: %u, time: %llu ms\n",
			cyc_end - cyc_start,
			(unsigned long long)(time_end - time_start));
		k_msleep(1);
	}
	return 0;
}

void controller_receive(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_CONTROLLER_NODE);

	zassert_true(spi_test_device_ready(dev, "Controller"),
		     "Controller device not ready");

	perf_controller_receive(dev);
}

void target_send(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_TARGET_NODE);

	zassert_true(spi_test_device_ready(dev, "Target"),
		     "Target device not ready");

	perf_target_transmit(dev);
}

void controller_spi(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_CONTROLLER_NODE);

	zassert_true(spi_test_device_ready(dev, "Controller"),
		     "Controller device not ready");

	int length = SPI_TEST_BUFF_SIZE_PERF * sizeof(uint32_t);

	struct spi_buf tx_buf = { .buf = ctrl_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = ctrl_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };
	static struct spi_config cnfg[2];

	for (uint32_t freq = PERF_FREQ_MIN_MHZ;
	     freq <= PERF_FREQ_MAX_MHZ; freq = perf_freq_next(freq)) {
		cnfg[freq % 2] = spi_test_config(
			SPI_OP_MODE_MASTER | SPI_TEST_WORD_SIZE,
			freq * SPI_FREQ_MHZ);
		memset(ctrl_rxdata, 0, length);
		if (k_sem_take(&perf_sem_target_ready,
			       perf_sync_timeout(freq * SPI_FREQ_MHZ)) != 0) {
			LOG_ERR("Controller: target ready timeout @%u Hz",
				freq * SPI_FREQ_MHZ);
			break;
		}
		perf_wait_target_armed();

		uint32_t cyc_start = k_cycle_get_32();

		int64_t time_start = k_uptime_get();

		int ret = controller_xfer(dev, &cnfg[freq % 2], &tx_set, &rx_set);

		k_sem_give(&perf_sem_controller_done);

		int64_t time_end = k_uptime_get();

		uint32_t cyc_end = k_cycle_get_32();

		if (ret < 0) {
			LOG_ERR("Controller transceive @%u Hz failed: %d",
				freq * SPI_FREQ_MHZ, ret);
			ctrl_ret_tx++;
		} else {
			ret = memcmp(ctrl_rxdata, tgt_txdata, length);
			if (ret != 0) {
				LOG_ERR("Controller RX / Target TX mismatch @%u Hz",
					freq * SPI_FREQ_MHZ);
				ctrl_ret_rx++;
			} else {
				TC_PRINT("Controller transceive @%u Hz: data match\n",
					 freq * SPI_FREQ_MHZ);
			}
		}

		TC_PRINT("  cycles: %u, time: %llu ms\n",
			cyc_end - cyc_start,
			(unsigned long long)(time_end - time_start));
	}
	TC_PRINT("Controller transceive perf sweep complete\n");
}

void target_spi(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);

	const struct device *dev = DEVICE_DT_GET(SPI_TARGET_NODE);

	zassert_true(spi_test_device_ready(dev, "Target"),
		     "Target device not ready");

	int length = SPI_TEST_BUFF_SIZE_PERF * sizeof(uint32_t);

	struct spi_buf tx_buf = { .buf = tgt_txdata, .len = length };
	struct spi_buf rx_buf = { .buf = tgt_rxdata, .len = length };
	struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = 1 };
	struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = 1 };
	static struct spi_config cnfg[2];

	for (uint32_t freq = PERF_FREQ_MIN_MHZ;
	     freq <= PERF_FREQ_MAX_MHZ; freq = perf_freq_next(freq)) {
		cnfg[freq % 2] = (struct spi_config){
			.operation = SPI_OP_MODE_SLAVE | spi_test_op_flags |
				SPI_TEST_WORD_SIZE,
			.frequency = freq * SPI_FREQ_MHZ,
			.slave = 0,
		};
		memset(tgt_rxdata, 0, length);
		k_sem_give(&perf_sem_target_ready);

		uint32_t cyc_start = k_cycle_get_32();

		int64_t time_start = k_uptime_get();

		int ret = target_xfer(dev, &cnfg[freq % 2], &tx_set, &rx_set);

		if (k_sem_take(&perf_sem_controller_done,
			       perf_sync_timeout(freq * SPI_FREQ_MHZ)) != 0) {
			LOG_ERR("Target: controller done timeout @%u Hz",
				freq * SPI_FREQ_MHZ);
			break;
		}

		int64_t time_end = k_uptime_get();

		uint32_t cyc_end = k_cycle_get_32();

		if (ret < 0) {
			LOG_ERR("Target transceive @%u Hz failed: %d",
				freq * SPI_FREQ_MHZ, ret);
			tgt_ret_tx++;
		} else {
			ret = memcmp(ctrl_txdata, tgt_rxdata, length);
			if (ret != 0) {
				LOG_ERR("Target RX / Controller TX mismatch @%u Hz",
					freq * SPI_FREQ_MHZ);
				tgt_ret_rx++;
			} else {
				TC_PRINT("Target transceive @%u Hz: data match\n",
					 freq * SPI_FREQ_MHZ);
			}
		}

		TC_PRINT("  cycles: %u, time: %llu ms\n",
			cyc_end - cyc_start,
			(unsigned long long)(time_end - time_start));
	}
	TC_PRINT("Target transceive perf sweep complete\n");
}

#endif /* CONFIG_TEST_SPI_PERFORMANCE */
