/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include "test_spi_common.h"

#include <zephyr/sys/atomic.h>

LOG_MODULE_REGISTER(alif_spi_common, LOG_LEVEL_INF);

#define SPI_TEST_CFG_SHADOW_DEPTH 1024U


K_THREAD_STACK_DEFINE(controller_stack, SPI_TEST_STACKSIZE);
struct k_thread controller_thread_data;

K_THREAD_STACK_DEFINE(target_stack, SPI_TEST_STACKSIZE);
struct k_thread target_thread_data;

#if !IS_ENABLED(CONFIG_TEST_SPI_PERFORMANCE)
uint32_t __aligned(4) ctrl_txdata[SPI_TEST_BUFF_SIZE];
uint32_t __aligned(4) tgt_txdata[SPI_TEST_BUFF_SIZE];
uint32_t __aligned(4) ctrl_rxdata[SPI_TEST_BUFF_SIZE];
uint32_t __aligned(4) tgt_rxdata[SPI_TEST_BUFF_SIZE];
#else
uint32_t __aligned(4) ctrl_txdata[SPI_TEST_BUFF_SIZE_PERF];
uint32_t __aligned(4) ctrl_rxdata[SPI_TEST_BUFF_SIZE_PERF];
uint32_t __aligned(4) tgt_txdata[SPI_TEST_BUFF_SIZE_PERF];
uint32_t __aligned(4) tgt_rxdata[SPI_TEST_BUFF_SIZE_PERF];
#endif

atomic_t err_count;
atomic_val_t transceive_word;
uint32_t spi_test_op_flags;
atomic_t spi_test_async_mode;

uint32_t ctrl_ret_tx;
uint32_t tgt_ret_tx;
uint32_t ctrl_ret_rx;
uint32_t tgt_ret_rx;

static struct spi_config spi_test_cfg_shadow[SPI_TEST_CFG_SHADOW_DEPTH];
static atomic_t spi_test_cfg_idx;
static struct k_mutex cfg_shadow_mutex;


/* ---------------------------------------------------------------------------
 * Utility functions
 * ---------------------------------------------------------------------------
 */
int spi_test_prepare_data(uint32_t *data, uint16_t pattern, uint32_t count)
{
	if (data == NULL) {
		LOG_ERR("%s: NULL data pointer", __func__);
		return -EINVAL;
	}
	if (count == 0) {
		return 0;
	}
	for (uint32_t i = 0; i < count; i++) {
		data[i] = ((uint32_t)pattern << 16) | i;
	}
	return 0;
}

int spi_test_reset_buffer(uint32_t *buffer, size_t count)
{
	if (buffer == NULL) {
		LOG_ERR("%s: NULL received data pointer", __func__);
		return -EINVAL;
	}
	memset(buffer, 0, count * sizeof(uint32_t));
	return 0;
}

uint32_t spi_test_word_operation(uint32_t base_op, uint32_t word_size)
{
	uint32_t operation = 0U;

	switch (word_size) {
	case 32U:
		operation = base_op | (uint32_t)SPI_WORD_SET(32);
		break;
	case 16U:
		operation = base_op | (uint32_t)SPI_WORD_SET(16);
		break;
	default:
		operation = base_op | (uint32_t)SPI_WORD_SET(8);
		break;
	}

	return operation;
}

bool spi_test_device_ready(const struct device *dev, const char *role)
{
	bool ready = false;

	if ((dev != NULL) && (device_is_ready(dev))) {
		ready = true;
	} else if (role != NULL) {
		LOG_ERR("%s device not ready", role);
	} else {
		LOG_ERR("SPI device not ready");
	}

	return ready;
}

void spi_test_controller_cs_init(struct spi_cs_control *cs)
{
	if (cs == NULL) {
		return;
	}

#if DT_NODE_HAS_PROP(SPI_CONTROLLER_NODE, cs_gpios)
	*cs = (struct spi_cs_control){
		.gpio = GPIO_DT_SPEC_GET(SPI_CONTROLLER_NODE, cs_gpios),
		.delay = SPI_TEST_CS_DELAY_US,
	};
	LOG_DBG("Controller CS GPIO initialized: pin=%d, delay=%d us",
		cs->gpio.pin, cs->delay);
#else
	(void)memset(cs, 0, sizeof(*cs));
#endif
}

int spi_test_transceive(const struct device *dev,
			const struct spi_config *cnfg,
			const struct spi_buf_set *tx_set,
			const struct spi_buf_set *rx_set,
			bool force_reconfig)
{
	struct spi_config *active_cfg;
	const struct spi_config *cfg_to_use;
	atomic_val_t idx;

	if (dev == NULL) {
		LOG_ERR("%s: NULL device", __func__);
		return -EINVAL;
	}
	if (cnfg == NULL) {
		LOG_ERR("%s: NULL config", __func__);
		return -EINVAL;
	}

	if (!force_reconfig) {
		return spi_transceive(dev, cnfg, tx_set, rx_set);
	}

	k_mutex_lock(&cfg_shadow_mutex, K_FOREVER);

	idx = atomic_inc(&spi_test_cfg_idx);
	if ((size_t)idx >= ARRAY_SIZE(spi_test_cfg_shadow)) {
		LOG_ERR("%s: cfg shadow exhausted (%ld >= %u)",
			__func__, (long)idx,
			(unsigned int)ARRAY_SIZE(spi_test_cfg_shadow));
		k_mutex_unlock(&cfg_shadow_mutex);
		return -ENOMEM;
	}

	active_cfg = &spi_test_cfg_shadow[idx];
	*active_cfg = *cnfg;
	cfg_to_use = active_cfg;

	k_mutex_unlock(&cfg_shadow_mutex);

	LOG_DBG("%s: forced reconfig ptr=%p idx=%ld",
		__func__, (void *)active_cfg, (long)idx);

	return spi_transceive(dev, cfg_to_use, tx_set, rx_set);
}

int spi_test_transceive_async(const struct device *dev,
			      const struct spi_config *cnfg,
			      const struct spi_buf_set *tx_set,
			      const struct spi_buf_set *rx_set,
			      bool force_reconfig)
{
	struct spi_config *active_cfg;
	const struct spi_config *cfg_to_use;
	atomic_val_t idx;
	struct k_poll_signal async_sig;
	struct k_poll_event async_evt;
	int ret;

	if (dev == NULL) {
		LOG_ERR("%s: NULL device", __func__);
		return -EINVAL;
	}
	if (cnfg == NULL) {
		LOG_ERR("%s: NULL config", __func__);
		return -EINVAL;
	}

	if (!force_reconfig) {
		cfg_to_use = cnfg;
	} else {
		k_mutex_lock(&cfg_shadow_mutex, K_FOREVER);

		idx = atomic_inc(&spi_test_cfg_idx);
		if ((size_t)idx >= ARRAY_SIZE(spi_test_cfg_shadow)) {
			LOG_ERR("%s: cfg shadow exhausted", __func__);
			k_mutex_unlock(&cfg_shadow_mutex);
			return -ENOMEM;
		}

		active_cfg = &spi_test_cfg_shadow[idx];
		*active_cfg = *cnfg;
		cfg_to_use = active_cfg;

		k_mutex_unlock(&cfg_shadow_mutex);
	}

	k_poll_signal_init(&async_sig);
	k_poll_event_init(&async_evt, K_POLL_TYPE_SIGNAL,
			  K_POLL_MODE_NOTIFY_ONLY, &async_sig);

	ret = spi_transceive_signal(dev, cfg_to_use, tx_set, rx_set, &async_sig);
	if (ret != 0) {
		return ret;
	}

	ret = k_poll(&async_evt, 1, K_MSEC(500));
	if (ret != 0) {
		LOG_ERR("async xfer poll timeout");
		return -EIO;
	}

	return async_sig.result;
}

int spi_test_controller_receive_and_check(const struct device *dev,
				      const struct spi_config *cnfg,
				      const uint32_t *expected_tx,
				      uint32_t *rx_buffer,
				      size_t word_count)
{
	size_t length;
	struct spi_buf rx_buf;
	struct spi_buf_set rx_set;
	int ret;

	if ((dev == NULL) || (cnfg == NULL) || (expected_tx == NULL) ||
	    (rx_buffer == NULL) || (word_count == 0U)) {
		LOG_ERR("controller_receive_and_check: invalid argument");
		return -EINVAL;
	}

	length = word_count * sizeof(uint32_t);
	rx_buf = (struct spi_buf){
		.buf = rx_buffer,
		.len = length,
	};
	rx_set = (struct spi_buf_set){
		.buffers = &rx_buf,
		.count = 1U,
	};

	LOG_DBG("Controller RX: Starting transceive (freq=%u, len=%zu)",
		cnfg->frequency, length);
	ret = spi_test_transceive(dev, cnfg, NULL, &rx_set, false);

	if (ret < 0) {
		LOG_ERR("Controller RX: %s failed: %d", __func__, ret);
		return ret;
	}

	return memcmp(expected_tx, rx_buffer, length);
}

void spi_test_log_first_mismatch(const uint32_t *expected,
				 const uint32_t *actual,
				 size_t word_count,
				 const char *context)
{
	size_t length;
	const uint8_t *exp_bytes;
	const uint8_t *act_bytes;

	if ((expected == NULL) || (actual == NULL) || (word_count == 0U)) {
		LOG_ERR("%s mismatch details unavailable (invalid args)",
			(context != NULL) ? context : "SPI");
		return;
	}

	length = word_count * sizeof(uint32_t);
	exp_bytes = (const uint8_t *)expected;
	act_bytes = (const uint8_t *)actual;

	for (size_t i = 0U; i < length; i++) {
		if (exp_bytes[i] != act_bytes[i]) {
			LOG_ERR("%s: Mismatch at byte %u "
				"(expected: 0x%02X, received: 0x%02X, byte pattern 0x%02X)",
				(context != NULL) ? context : "SPI",
				(unsigned int)i, exp_bytes[i], act_bytes[i], 0);
			return;
		}
	}

	LOG_ERR("%s: mismatch reported but byte diff not found",
		(context != NULL) ? context : "SPI");
}

/*
 * DMA configuration functions for direct PL330 DMA controllers.
 */

#if DT_NODE_HAS_PROP(DT_NODELABEL(lpspi0), dmas) /* E1C/B1 LPSPI0 dma2 */
static void configure_lpspi0_for_dma2(void)
{
	#define LPSPI_DMA_RX_PERIPH_REQ		12
	#define LPSPI_DMA_TX_PERIPH_REQ		13
	#define LPSPI_DMA_GROUP             1

	uint32_t regdata;

	LOG_INF("Configuring lpspi0 for dma2");


	sys_clear_bits(M55HE_CFG_HE_DMA_SEL, HE_DMA_SEL_LPSPI_Msk);


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(LPSPI_DMA_GROUP),
			EVTRTRLOCAL_DMA_CTRL0 + (LPSPI_DMA_RX_PERIPH_REQ * 4));


	regdata = sys_read32(EVTRTRLOCAL_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));
	regdata |= (1 << LPSPI_DMA_RX_PERIPH_REQ);
	sys_write32(regdata, EVTRTRLOCAL_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(LPSPI_DMA_GROUP),
			EVTRTRLOCAL_DMA_CTRL0 + (LPSPI_DMA_TX_PERIPH_REQ * 4));


	regdata = sys_read32(EVTRTRLOCAL_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));
	regdata |= (1 << LPSPI_DMA_TX_PERIPH_REQ);
	sys_write32(regdata, EVTRTRLOCAL_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));
}
#endif /* E1C/B1 LPSPI0 dma2 */

#if DT_NODE_HAS_PROP(DT_NODELABEL(spi1), dmas) /* E1C/B1 SPI1 dma2 */
static void configure_spi1_for_dma2(void)
{
	uint32_t regdata;


	#define SPI1_DMA_RX_PERIPH_REQ         17
	#define SPI1_DMA_TX_PERIPH_REQ         21
	#define SPI1_DMA_GROUP                 2

	LOG_INF("Configuring spi1 for dma2");


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(SPI1_DMA_GROUP),
			EVTRTRLOCAL_DMA_CTRL0 + (SPI1_DMA_RX_PERIPH_REQ * 4));


	regdata = sys_read32(EVTRTRLOCAL_DMA_ACK_TYPE0 + (SPI1_DMA_GROUP * 4));
	regdata |= (1 << SPI1_DMA_RX_PERIPH_REQ);
	sys_write32(regdata, EVTRTRLOCAL_DMA_ACK_TYPE0 + (SPI1_DMA_GROUP * 4));


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(SPI1_DMA_GROUP),
			EVTRTRLOCAL_DMA_CTRL0 + (SPI1_DMA_TX_PERIPH_REQ * 4));


	regdata = sys_read32(EVTRTRLOCAL_DMA_ACK_TYPE0 + (SPI1_DMA_GROUP * 4));
	regdata |= (1 << SPI1_DMA_TX_PERIPH_REQ);
	sys_write32(regdata, EVTRTRLOCAL_DMA_ACK_TYPE0 + (SPI1_DMA_GROUP * 4));
}
#endif /* E1C/B1 SPI1 dma2 */

#if DT_NODE_HAS_PROP(DT_NODELABEL(spi0), dmas) /* SPI0 dma2 */
static void configure_spi0_for_dma2(void)
{
	uint32_t regdata;

	#define SPI0_DMA_RX_PERIPH_REQ         16
	#define SPI0_DMA_TX_PERIPH_REQ         20
	#define SPI0_DMA_GROUP                 2

	LOG_INF("Configuring spi0 for dma2");

	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(SPI0_DMA_GROUP),
			EVTRTRLOCAL_DMA_CTRL0 + (SPI0_DMA_RX_PERIPH_REQ * 4));

	regdata = sys_read32(EVTRTRLOCAL_DMA_ACK_TYPE0 + (SPI0_DMA_GROUP * 4));
	regdata |= (1 << SPI0_DMA_RX_PERIPH_REQ);
	sys_write32(regdata, EVTRTRLOCAL_DMA_ACK_TYPE0 + (SPI0_DMA_GROUP * 4));

	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(SPI0_DMA_GROUP),
			EVTRTRLOCAL_DMA_CTRL0 + (SPI0_DMA_TX_PERIPH_REQ * 4));

	regdata = sys_read32(EVTRTRLOCAL_DMA_ACK_TYPE0 + (SPI0_DMA_GROUP * 4));
	regdata |= (1 << SPI0_DMA_TX_PERIPH_REQ);
	sys_write32(regdata, EVTRTRLOCAL_DMA_ACK_TYPE0 + (SPI0_DMA_GROUP * 4));
}
#endif /* SPI0 dma2 */

#if DT_NODE_HAS_PROP(DT_NODELABEL(spi4), dmas) /* LPSPI(SPI4) */

/* dma2 */
#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(dma2), arm_dma_pl330, okay)
static void configure_lpspi_for_dma2(void)
{
	#define LPSPI_DMA_RX_PERIPH_REQ		12
	#define LPSPI_DMA_TX_PERIPH_REQ		13

	LOG_INF("Configuring lpspi for dma2");


	sys_clear_bits(M55HE_CFG_HE_DMA_SEL, HE_DMA_SEL_LPSPI_Msk);


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos),
			EVTRTRLOCAL_DMA_CTRL0 + (LPSPI_DMA_RX_PERIPH_REQ * 4));


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos),
			EVTRTRLOCAL_DMA_CTRL0 + (LPSPI_DMA_TX_PERIPH_REQ * 4));
}

/* dma0 */
#elif DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(dma0), arm_dma_pl330, okay)
static void configure_lpspi_for_dma0(void)
{
	uint32_t regdata;


	#define LPSPI_DMA_RX_PERIPH_REQ        24
	#define LPSPI_DMA_TX_PERIPH_REQ        25
	#define LPSPI_DMA_GROUP                2

	LOG_INF("Configuring lpspi for dma0");


	regdata = sys_read32(M55HE_CFG_HE_DMA_SEL);
	regdata &= ~HE_DMA_SEL_LPSPI_Msk;
	regdata |= (0 << HE_DMA_SEL_LPSPI_Pos);
	sys_write32(regdata, M55HE_CFG_HE_DMA_SEL);


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(LPSPI_DMA_GROUP),
			EVTRTR0_DMA_CTRL0 + (LPSPI_DMA_RX_PERIPH_REQ * 4));


	regdata = sys_read32(EVTRTR0_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));
	regdata |= (1 << LPSPI_DMA_RX_PERIPH_REQ);
	sys_write32(regdata, EVTRTR0_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(LPSPI_DMA_GROUP),
			EVTRTR0_DMA_CTRL0 + (LPSPI_DMA_TX_PERIPH_REQ * 4));


	regdata = sys_read32(EVTRTR0_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));
	regdata |= (1 << LPSPI_DMA_TX_PERIPH_REQ);
	sys_write32(regdata, EVTRTR0_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));
}
#endif /* dma0 */
#endif /* LPSPI(SPI4) */


/* dma0 */
#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(dma0), arm_dma_pl330, okay)

#if DT_NODE_HAS_PROP(DT_NODELABEL(spi0), dmas) /* SPI0 dma0 */
static void configure_spi0_for_dma0(void)
{
	uint32_t regdata;


	#define SPI0_DMA_RX_PERIPH_REQ         16
	#define SPI0_DMA_TX_PERIPH_REQ         20
	#define SPI0_DMA_GROUP                 2

	LOG_INF("Configuring spi0 for dma0");


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(SPI0_DMA_GROUP),
			EVTRTR0_DMA_CTRL0 + (SPI0_DMA_RX_PERIPH_REQ * 4));


	regdata = sys_read32(EVTRTR0_DMA_ACK_TYPE0 + (SPI0_DMA_GROUP * 4));
	regdata |= (1 << SPI0_DMA_RX_PERIPH_REQ);
	sys_write32(regdata, EVTRTR0_DMA_ACK_TYPE0 + (SPI0_DMA_GROUP * 4));


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(SPI0_DMA_GROUP),
			EVTRTR0_DMA_CTRL0 + (SPI0_DMA_TX_PERIPH_REQ * 4));


	regdata = sys_read32(EVTRTR0_DMA_ACK_TYPE0 + (SPI0_DMA_GROUP * 4));
	regdata |= (1 << SPI0_DMA_TX_PERIPH_REQ);
	sys_write32(regdata, EVTRTR0_DMA_ACK_TYPE0 + (SPI0_DMA_GROUP * 4));
}
#endif /* SPI0 dma0 */

#if DT_NODE_HAS_PROP(DT_NODELABEL(spi1), dmas) /* SPI1 dma0 */
static void configure_spi1_for_dma0(void)
{
	uint32_t regdata;


	#define SPI1_DMA_RX_PERIPH_REQ         17
	#define SPI1_DMA_TX_PERIPH_REQ         21
	#define SPI1_DMA_GROUP                 2

	LOG_INF("Configuring spi1 for dma0");


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(SPI1_DMA_GROUP),
			EVTRTR0_DMA_CTRL0 + (SPI1_DMA_RX_PERIPH_REQ * 4));


	regdata = sys_read32(EVTRTR0_DMA_ACK_TYPE0 + (SPI1_DMA_GROUP * 4));
	regdata |= (1 << SPI1_DMA_RX_PERIPH_REQ);
	sys_write32(regdata, EVTRTR0_DMA_ACK_TYPE0 + (SPI1_DMA_GROUP * 4));


	sys_write32(DMA_CTRL_ENA |
			(0 << DMA_CTRL_ACK_TYPE_Pos)|
			(SPI1_DMA_GROUP),
			EVTRTR0_DMA_CTRL0 + (SPI1_DMA_TX_PERIPH_REQ * 4));


	regdata = sys_read32(EVTRTR0_DMA_ACK_TYPE0 + (SPI1_DMA_GROUP * 4));
	regdata |= (1 << SPI1_DMA_TX_PERIPH_REQ);
	sys_write32(regdata, EVTRTR0_DMA_ACK_TYPE0 + (SPI1_DMA_GROUP * 4));
}
#endif /* SPI1 dma0 */

#endif /* dma0 */

/*
 * DMA EVTRTR configuration for direct PL330 DMA controllers (dma0 / dma2).
 * Event-router DMA parents (evtrtr0 / evtrtr2) are handled by their driver.
 * Gated by CONFIG_TEST_SPI_USE_DMA.
 */
#if IS_ENABLED(CONFIG_TEST_SPI_USE_DMA)

/*
 * spi_test_configure_dma() — configures DMA routing
 * based on the DMA controller type.
 * Event Router DMA (evtrtr0/evtrtr2) is handled by the driver.
 * Direct PL330 DMA (dma0/dma2) requires manual event router configuration.
 */

#define SPI_TEST_NODE_DMA_IS(node, dma) \
	(DT_NODE_EXISTS(DT_NODELABEL(node)) && \
	 DT_NODE_HAS_PROP(DT_NODELABEL(node), dmas) && \
	 DT_SAME_NODE(DT_PHANDLE_BY_IDX(DT_NODELABEL(node), dmas, 0), DT_NODELABEL(dma)))

static int spi_test_configure_dma(void)
{
	LOG_INF("DMA configuration - align with spi_dw sample mode split");

	/* Validate DMA devices are ready before configuring */
#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(dma0), arm_dma_pl330, okay)
	const struct device *dma0_dev = DEVICE_DT_GET(DT_NODELABEL(dma0));

	if (!device_is_ready(dma0_dev)) {
		LOG_ERR("DMA0 (dma0) not ready");
		return -ENODEV;
	}
#endif

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(dma2), arm_dma_pl330, okay)
	const struct device *dma2_dev = DEVICE_DT_GET(DT_NODELABEL(dma2));

	if (!device_is_ready(dma2_dev)) {
		LOG_ERR("DMA2 (dma2) not ready");
		return -ENODEV;
	}
#endif

	/* Direct PL330 DMA needs manual event-router configuration;
	 * event-router DMA parents are handled by the driver.
	 */

#if DT_NODE_EXISTS(DT_NODELABEL(evtrtr2))
#if SPI_TEST_NODE_DMA_IS(lpspi0, evtrtr2)
	LOG_INF("lpspi0 dmas -> evtrtr2 (event-router)");
#endif
#if SPI_TEST_NODE_DMA_IS(spi1, evtrtr2)
	LOG_INF("spi1 dmas -> evtrtr2 (event-router)");
#endif
#if SPI_TEST_NODE_DMA_IS(spi4, evtrtr2)
	LOG_INF("spi4 dmas -> evtrtr2 (event-router)");
#endif
#endif /* evtrtr2 */

#if DT_NODE_EXISTS(DT_NODELABEL(evtrtr0))
#if SPI_TEST_NODE_DMA_IS(spi0, evtrtr0)
	LOG_INF("spi0 dmas -> evtrtr0 (event-router)");
#endif
#if SPI_TEST_NODE_DMA_IS(spi1, evtrtr0)
	LOG_INF("spi1 dmas -> evtrtr0 (event-router)");
#endif
#if SPI_TEST_NODE_DMA_IS(spi4, evtrtr0)
	LOG_INF("spi4 dmas -> evtrtr0 (event-router)");
#endif
#endif /* evtrtr0 */

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(dma2), arm_dma_pl330, okay)
#if SPI_TEST_NODE_DMA_IS(lpspi0, dma2)
	configure_lpspi0_for_dma2();
#endif

#if SPI_TEST_NODE_DMA_IS(spi0, dma2)
	configure_spi0_for_dma2();
#endif

#if SPI_TEST_NODE_DMA_IS(spi1, dma2)
	configure_spi1_for_dma2();
#endif

#if SPI_TEST_NODE_DMA_IS(spi4, dma2)
	configure_lpspi_for_dma2();
#endif
#endif /* dma2 */

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(dma0), arm_dma_pl330, okay)
#if SPI_TEST_NODE_DMA_IS(spi0, dma0)
	configure_spi0_for_dma0();
#endif

#if SPI_TEST_NODE_DMA_IS(spi1, dma0)
	configure_spi1_for_dma0();
#endif

#if SPI_TEST_NODE_DMA_IS(spi4, dma0)
	configure_lpspi_for_dma0();
#endif
#endif /* dma0 */

	LOG_INF("DMA routing setup done");
	return 0;
}

#endif /* CONFIG_TEST_SPI_USE_DMA */

/* ---------------------------------------------------------------------------
 * Ztest before-function: runs before every test case
 * ---------------------------------------------------------------------------
 */
void test_before_func(void *fixture)
{
	ARG_UNUSED(fixture);

	TC_PRINT("Test setup: DMA enabled=%d\n",
		 IS_ENABLED(CONFIG_TEST_SPI_USE_DMA));

#if IS_ENABLED(CONFIG_TEST_SPI_USE_DMA)
	TC_PRINT("About to call spi_test_configure_dma()\n");
	int dma_ret = spi_test_configure_dma();

	zassert_equal(dma_ret, 0, "DMA configuration failed: %d", dma_ret);
#else
	TC_PRINT("DMA not enabled - skipping DMA configuration\n");
#endif
	TC_PRINT("SPI test setup: mode=%s path=%s\n",
		IS_ENABLED(CONFIG_TEST_SPI_PERFORMANCE) ?
			"performance" : "functional",
		IS_ENABLED(CONFIG_TEST_SPI_USE_DMA) ? "dma" : "pio");

	/* Reset config index to prevent exhaustion in stress tests */
	atomic_clear(&spi_test_cfg_idx);

	/* Initialize mutex for thread-safe shadow config access */
	k_mutex_init(&cfg_shadow_mutex);

	/* Clear shadow config array to ensure clean state */
	memset(spi_test_cfg_shadow, 0, sizeof(spi_test_cfg_shadow));

#if !IS_ENABLED(CONFIG_TEST_SPI_PERFORMANCE)
	(void)spi_test_reset_buffer(ctrl_txdata, SPI_TEST_BUFF_SIZE);
	(void)spi_test_reset_buffer(tgt_txdata, SPI_TEST_BUFF_SIZE);
	(void)spi_test_reset_buffer(ctrl_rxdata, SPI_TEST_BUFF_SIZE);
	(void)spi_test_reset_buffer(tgt_rxdata, SPI_TEST_BUFF_SIZE);
	(void)spi_test_prepare_data(ctrl_txdata, spi_test_pattern(0xBEEF),
				    SPI_TEST_BUFF_SIZE);
	(void)spi_test_prepare_data(tgt_txdata, spi_test_pattern(0xCAFE),
				    SPI_TEST_BUFF_SIZE);
#else
	(void)spi_test_reset_buffer(ctrl_txdata, SPI_TEST_BUFF_SIZE_PERF);
	(void)spi_test_reset_buffer(ctrl_rxdata, SPI_TEST_BUFF_SIZE_PERF);
	(void)spi_test_reset_buffer(tgt_txdata, SPI_TEST_BUFF_SIZE_PERF);
	(void)spi_test_reset_buffer(tgt_rxdata, SPI_TEST_BUFF_SIZE_PERF);
	(void)spi_test_prepare_data(ctrl_txdata, spi_test_pattern(0xBEEF),
				    SPI_TEST_BUFF_SIZE_PERF);
	(void)spi_test_prepare_data(tgt_txdata, spi_test_pattern(0xCAFE),
				    SPI_TEST_BUFF_SIZE_PERF);
#endif

	ctrl_ret_tx = 0;
	tgt_ret_tx = 0;
	ctrl_ret_rx = 0;
	tgt_ret_rx = 0;

	/* Reset atomic err_count */
	atomic_clear(&err_count);

	/* Set transceive_word: pinned value or default 8-bit */
	transceive_word = IS_ENABLED(CONFIG_TEST_SPI_WORD_SIZE_SWEEP) ?
				8 : CONFIG_TEST_SPI_WORD_SIZE;

	/* Reset async mode flag to default sync */
	atomic_clear(&spi_test_async_mode);

	LOG_INF("Test setup complete");
}
