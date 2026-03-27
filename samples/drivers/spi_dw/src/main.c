/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <string.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/dt-bindings/dma/alif_dma_event_router.h>
#include <soc_common.h>

#define Mhz		1000000
#define Khz		1000

/* master_spi and slave_spi aliases are defined in
 * overlay files to use different SPI instance if needed.
 */

#define SPIDW_NODE	DT_ALIAS(master_spi)

#define S_SPIDW_NODE	DT_ALIAS(slave_spi)

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* default SPI master SS(slave select) is H/W controlled,
 * enable this to use as S/W controlled using gpio.
 */
#define SPI_MASTER_SS_SW_CONTROLLED_GPIO   0


/* scheduling priority used by each thread,
 * as we are testing Loopback on the same board,
 * make sure master priority is higher than slave priority.
 */
#define MASTER_PRIORITY 6
#define SLAVE_PRIORITY  7

/* delay between greetings (in ms) */
#define SLEEPTIME 1000

K_THREAD_STACK_DEFINE(MasterT_stack, STACKSIZE);
static struct k_thread MasterT_data;

K_THREAD_STACK_DEFINE(SlaveT_stack, STACKSIZE);
static struct k_thread SlaveT_data;

/* Master and Slave buffer size */
#define BUFF_SIZE  200

/* Master and Slave buffer word size */
#define SPI_WORD_SIZE 8

/* Master and Slave buffer frequency */
#define SPI_FREQUENCY (1 * Mhz)

/* Master and Slave buffer transfers */
#define SPI_NUM_TRANSFERS  10

/* Master and Slave buffers */
static uint32_t master_txdata[BUFF_SIZE];
static uint32_t master_rxdata[BUFF_SIZE];
static uint32_t slave_txdata[BUFF_SIZE];
static uint32_t slave_rxdata[BUFF_SIZE];

/*
 * Send/Receive data through slave spi
 */
int slave_spi_transceive(const struct device *dev)
{
	struct spi_config cnfg;
	int ret;

	cnfg.frequency = SPI_FREQUENCY;
	cnfg.operation = SPI_OP_MODE_SLAVE | SPI_WORD_SET(SPI_WORD_SIZE);
	cnfg.slave = 0;

	int length = (BUFF_SIZE) * sizeof(slave_rxdata[0]);

	struct spi_buf rx_buf = {
		.buf = slave_rxdata,
		.len = length
	};
	struct spi_buf_set rx_bufset = {
		.buffers = &rx_buf,
		.count = 1
	};
	struct spi_buf tx_buf = {
		.buf = slave_txdata,
		.len = length
	};
	struct spi_buf_set tx_bufset = {
		.buffers = &tx_buf,
		.count = 1
	};

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret < 0) {
		printk("ERROR: Slave SPI Transceive: %d\n", ret);
	}

	printk("slave wrote: %08x %08x %08x %08x %08x\n",
		slave_txdata[0], slave_txdata[1], slave_txdata[2],
		slave_txdata[3], slave_txdata[4]);

	printk("slave read: %08x %08x %08x %08x %08x\n",
		slave_rxdata[0], slave_rxdata[1], slave_rxdata[2],
		slave_rxdata[3], slave_rxdata[4]);

	ret = memcmp(master_txdata, slave_rxdata, length);
	if (ret) {
		printk("ERROR: SPI Master TX & Slave RX DATA NOT MATCHING: %d\n", ret);
	} else {
		printk("SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: %d\n", ret);
	}

	return ret;
}

/*
 * Send/Receive data through master spi
 */
int master_spi_transceive(const struct device *dev,
				 struct spi_cs_control *cs)
{
	struct spi_config cnfg;
	int ret;

	cnfg.frequency = SPI_FREQUENCY;
	cnfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(SPI_WORD_SIZE);
	cnfg.slave = 0;
	cnfg.cs = *cs;

	int length = (BUFF_SIZE) * sizeof(master_txdata[0]);

	struct spi_buf tx_buf = {
		.buf = master_txdata,
		.len = length
	};

	struct spi_buf_set tx_bufset = {
		.buffers = &tx_buf,
		.count = 1
	};

	struct spi_buf rx_buf = {
		.buf = master_rxdata,
		.len = length
	};
	struct spi_buf_set rx_bufset = {
		.buffers = &rx_buf,
		.count = 1
	};

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret) {
		printk("ERROR: SPI=%p transceive: %d\n", dev, ret);
	}
	printk("Master wrote: %08x %08x %08x %08x %08x\n",
		master_txdata[0], master_txdata[1], master_txdata[2],
		master_txdata[3], master_txdata[4]);
	printk("Master receive: %08x %08x %08x %08x %08x\n",
		master_rxdata[0], master_rxdata[1], master_rxdata[2],
		master_rxdata[3], master_rxdata[4]);

	ret = memcmp(master_rxdata, slave_txdata, length);
	if (ret) {
		printk("ERROR: SPI Master RX & Slave TX DATA NOT MATCHING: %d\n", ret);
	} else {
		printk("SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: %d\n", ret);
	}

	return ret;
}

static void master_spi(void *p1, void *p2, void *p3)
{
	const struct device *const dev = DEVICE_DT_GET(SPIDW_NODE);
	uint32_t iterations = SPI_NUM_TRANSFERS;
	int ret;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!device_is_ready(dev)) {
		printk("%s: Master device not ready.\n", dev->name);
		return;
	}

#if SPI_MASTER_SS_SW_CONTROLLED_GPIO /* SPI master SS as S/W controlled using gpio */
	struct spi_cs_control cs_ctrl = (struct spi_cs_control){
		.gpio  = GPIO_DT_SPEC_GET(SPIDW_NODE, cs_gpios),
		.delay = 100u, /* k_busy_wait(uint32_t usec_to_wait) */
	};
#else /* SPI master SS as H/W controlled */
	struct spi_cs_control cs_ctrl = {0};
#endif /* SPI_MASTER_SS_SW_CONTROLLED_GPIO */

	while (iterations) {
		printk("Master Transceive Iter= %d\n", iterations);
		ret = master_spi_transceive(dev, &cs_ctrl);
		k_msleep(SLEEPTIME);
		if (ret < 0) {
			printk("Stopping the Master Thread due to error\n");
			return;
		}
		iterations--;
	}
	printk("Master Transfer Successfully Completed\n");
}

static void slave_spi(void *p1, void *p2, void *p3)
{
	const struct device *const slave_dev = DEVICE_DT_GET(S_SPIDW_NODE);
	uint32_t iterations = SPI_NUM_TRANSFERS;
	int ret;

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	if (!device_is_ready(slave_dev)) {
		printk("%s: Slave device not ready\n", slave_dev->name);
		return;
	}

	while (iterations) {
		printk("Slave Transceive Iter= %d\n", iterations);
		ret = slave_spi_transceive(slave_dev);
		if (ret < 0) {
			printk("Stopping the Slave Thread due to error\n");
			return;
		}
		iterations--;
	}
	printk("Slave Transfer Successfully Completed\n");
}

/* DMA configurations */
#define DMA_CTRL_ACK_TYPE_Pos          (16U)
#define DMA_CTRL_ENA                   (1U << 4)

#define HE_DMA_SEL_LPSPI_Pos           (4)
#define HE_DMA_SEL_LPSPI_Msk           (0x3U << HE_DMA_SEL_LPSPI_Pos)

#if defined(CONFIG_RTSS_HE)
/**
 * @brief Configure HE_DMA_SEL register for LPSPI
 *
 * @param dma_group DMA group value from DTS (extracted from ALIF_DMA_ENCODE)
 * @param use_dma2 True if using DMA2, False if using DMA0
 */
static void configure_he_dma_sel_lpspi(uint8_t dma_group, bool use_dma2)
{
	uint32_t regdata;
	uint32_t he_dma_sel_value;

#if (IS_ENABLED(CONFIG_SOC_SERIES_E1C) || IS_ENABLED(CONFIG_SOC_SERIES_B1))
	/* B1/E1C Series: LPSPI only supports DMA2
	 * HE_DMA_SEL Register mapping:
	 *   0x0: Select DMA2 group 1
	 *   0x1, 0x2, 0x3: Select DMA2 group 2
	 */
	if (!use_dma2) {
		LOG_ERR("LPSPI on B1/E1C only supports DMA2, but DMA0 configured in DTS");
		return;
	}

	/* Map dma_group to HE_DMA_SEL value */
	if (dma_group == 1) {
		he_dma_sel_value = 0x0;  /* DMA2 group 1 */
	} else if (dma_group >= 2) {
		he_dma_sel_value = 0x1;  /* DMA2 group 2 (0x1, 0x2, or 0x3 all map to group 2) */
	} else {
		LOG_ERR("LPSPI on B1/E1C: invalid dma_group=%u (must be 1 or 2)", dma_group);
		return;
	}

#else /* E7/E5/E3/E4/E6/E8/E1 Series */
	/* E-Series: LPSPI supports both DMA2 and DMA0
	 * HE_DMA_SEL Register mapping:
	 *   0x0: Select DMA2
	 *   0x1: Select DMA0 group 1
	 *   0x2, 0x3: Select DMA0 group 2
	 */

	if (use_dma2) {
		/* Using DMA2: HE_DMA_SEL=0x0 regardless of dma_group */
		he_dma_sel_value = 0x0;
	} else {
		/* Using DMA0: HE_DMA_SEL depends on dma_group */
		if (dma_group == 1) {
			he_dma_sel_value = 0x1;  /* DMA0 group 1 */
		} else if (dma_group >= 2) {
			he_dma_sel_value = 0x2;  /* DMA0 group 2 (0x2 or 0x3) */
		} else {
			LOG_ERR("LPSPI on E-series DMA0: invalid dma_group=%u (must be 1 or 2)",
				dma_group);
			return;
		}
	}

#endif /* SoC series check */

	/* Program HE_DMA_SEL register */
	regdata = sys_read32(M55HE_CFG_HE_DMA_SEL);
	regdata &= ~HE_DMA_SEL_LPSPI_Msk;
	regdata |= ((he_dma_sel_value << HE_DMA_SEL_LPSPI_Pos) & HE_DMA_SEL_LPSPI_Msk);
	sys_write32(regdata, M55HE_CFG_HE_DMA_SEL);
}
#endif /* CONFIG_RTSS_HE */

static void prepare_data(uint32_t *data, uint16_t def_mask)
{
	for (uint32_t cnt = 0; cnt < BUFF_SIZE; cnt++) {
		data[cnt] = (def_mask << 16) | cnt;
	}
}

int main(void)
{

	/*
	 * Event Router Driver Configuration Section
	 * When using the event router driver, the event router registers are
	 * configured automatically via device tree. However, we still need to
	 * configure SoC-level mux registers like HE_DMA_SEL that select which
	 * DMA controller is routed to RTSS-HE peripherals.
	 */

	/* Configure HE_DMA_SEL for LPSPI based on DTS configuration */
#if DT_NODE_EXISTS(DT_NODELABEL(lpspi0)) && DT_NODE_HAS_PROP(DT_NODELABEL(lpspi0), dmas)
	/* B1/E1C: lpspi0 - always uses DMA2 via evtrtr2 */
	{
		/* Extract dma_group from encoded channel value */
		uint8_t dma_group = ALIF_DMA_DECODE_GROUP(
			DT_PHA_BY_IDX(DT_NODELABEL(lpspi0), dmas, 0, channel));
		configure_he_dma_sel_lpspi(dma_group, true);
	}
#elif DT_NODE_EXISTS(DT_NODELABEL(spi4)) && DT_NODE_HAS_PROP(DT_NODELABEL(spi4), dmas)
	/* E-series: spi4 (LPSPI) - check which event router is referenced
	 * - &evtrtr2 → DMA2
	 * - &evtrtr0 → DMA0
	 */
	{
		/* Check which event router phandle is used */
		bool use_dma2 = DT_SAME_NODE(
			DT_PHANDLE_BY_IDX(DT_NODELABEL(spi4), dmas, 0),
			DT_NODELABEL(evtrtr2));
		/* Extract dma_group from encoded channel value */
		uint8_t dma_group = ALIF_DMA_DECODE_GROUP(
			DT_PHA_BY_IDX(DT_NODELABEL(spi4), dmas, 0, channel));
		configure_he_dma_sel_lpspi(dma_group, use_dma2);
	}
#endif

	prepare_data(master_txdata, 0xA5A5);
	prepare_data(slave_txdata, 0x5A5A);

	k_tid_t tids = k_thread_create(&SlaveT_data, SlaveT_stack, STACKSIZE,
			&slave_spi, NULL, NULL, NULL,
			SLAVE_PRIORITY, 0, K_NO_WAIT);
	if (tids == NULL) {
		printk("Error creating Slave Thread\n");
	}

	/* as we are testing Loopback on the same board,
	 * make sure slave is ready before master starts.
	 */
	k_msleep(100);
	k_tid_t tidm = k_thread_create(&MasterT_data, MasterT_stack, STACKSIZE,
			&master_spi, NULL, NULL, NULL,
			MASTER_PRIORITY, 0, K_NO_WAIT);
	if (tidm == NULL) {
		printk("Error creating Master Thread\n");
	}

	k_thread_start(&SlaveT_data);
	k_thread_start(&MasterT_data);

	return 0;
}
