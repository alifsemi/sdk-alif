#include <errno.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/spi.h>
#include <soc_common.h>
#include <zephyr/logging/log.h>

#include "exerciser_app.h"

LOG_MODULE_REGISTER(exerciser_spi, LOG_LEVEL_INF);

#if SPI_NODE_OKAY
#define Mhz 1000000

/* master_spi alias is defined in the overlay (lpspi0). */
#define SPIDW_NODE DT_ALIAS(master_spi)

/* Default SPI master SS is H/W controlled; set to 1 for GPIO CS. */
#define SPI_MASTER_SS_SW_CONTROLLED_GPIO 0

#define SLEEPTIME     1000
#define BUFF_SIZE     64
#define SPI_WORD_SIZE 8
#define SPI_FREQUENCY (1 * Mhz)

static uint32_t master_txdata[BUFF_SIZE];
static uint32_t master_rxdata[BUFF_SIZE];

/* DMA handshake register helpers (E1C/B1 LPSPI + dma2) */
#define DMA_CTRL_ACK_TYPE_Pos (16U)
#define DMA_CTRL_ENA          (1U << 4)

#ifndef HE_DMA_SEL_LPSPI_Pos
#define HE_DMA_SEL_LPSPI_Pos (4)
#endif
#define HE_DMA_SEL_LPSPI_Msk (0x3U << HE_DMA_SEL_LPSPI_Pos)

#define LPSPI_DMA_RX_PERIPH_REQ 12
#define LPSPI_DMA_TX_PERIPH_REQ 13
#define LPSPI_DMA_GROUP         1

static int master_spi_transceive(const struct device *dev, struct spi_cs_control *cs)
{
	struct spi_config cnfg;
	int ret;
	int length = BUFF_SIZE * sizeof(master_txdata[0]);

	struct spi_buf tx_buf = {
		.buf = master_txdata,
		.len = length,
	};
	struct spi_buf_set tx_bufset = {
		.buffers = &tx_buf,
		.count = 1,
	};
	struct spi_buf rx_buf = {
		.buf = master_rxdata,
		.len = length,
	};
	struct spi_buf_set rx_bufset = {
		.buffers = &rx_buf,
		.count = 1,
	};

	cnfg.frequency = SPI_FREQUENCY;
	/* Internal loopback: MOSI is fed back to MISO in the controller. */
	cnfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(SPI_WORD_SIZE) | SPI_MODE_LOOP;
	cnfg.slave = 0;
	cnfg.cs = *cs;

	ret = spi_transceive(dev, &cnfg, &tx_bufset, &rx_bufset);
	if (ret) {
		LOG_ERR("ERROR: SPI=%p transceive: %d", dev, ret);
		return ret;
	}

	LOG_INF("Master wrote: %08x %08x %08x %08x %08x",
		master_txdata[0], master_txdata[1], master_txdata[2],
		master_txdata[3], master_txdata[4]);
	LOG_INF("Master receive: %08x %08x %08x %08x %08x",
		master_rxdata[0], master_rxdata[1], master_rxdata[2],
		master_rxdata[3], master_rxdata[4]);

	ret = memcmp(master_txdata, master_rxdata, length);
	if (ret) {
		LOG_ERR("ERROR: SPI loopback TX/RX data mismatch: %d", ret);
		return -EIO;
	}

	LOG_INF("SUCCESS: SPI loopback TX/RX data matches");
	return 0;
}

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(dma2), arm_dma_pl330, okay)
#if (IS_ENABLED(CONFIG_SOC_SERIES_E1C) || IS_ENABLED(CONFIG_SOC_SERIES_B1))
#if DT_NODE_HAS_PROP(DT_NODELABEL(lpspi0), dmas)
static void configure_lpspi0_for_dma2(void)
{
	uint32_t regdata;

	LOG_INF("configure lpspi0 for dma2");

	/* Select DMA2 group 1 for LPSPI (default) */
	sys_clear_bits(M55HE_CFG_HE_DMA_SEL, HE_DMA_SEL_LPSPI_Msk);

	sys_write32(DMA_CTRL_ENA |
				(0 << DMA_CTRL_ACK_TYPE_Pos) |
				LPSPI_DMA_GROUP,
		    EVTRTRLOCAL_DMA_CTRL0 + (LPSPI_DMA_RX_PERIPH_REQ * 4));

	regdata = sys_read32(EVTRTRLOCAL_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));
	regdata |= (1 << LPSPI_DMA_RX_PERIPH_REQ);
	sys_write32(regdata, EVTRTRLOCAL_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));

	sys_write32(DMA_CTRL_ENA |
				(0 << DMA_CTRL_ACK_TYPE_Pos) |
				LPSPI_DMA_GROUP,
			EVTRTRLOCAL_DMA_CTRL0 + (LPSPI_DMA_TX_PERIPH_REQ * 4));

	regdata = sys_read32(EVTRTRLOCAL_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));
	regdata |= (1 << LPSPI_DMA_TX_PERIPH_REQ);
	sys_write32(regdata, EVTRTRLOCAL_DMA_ACK_TYPE0 + (LPSPI_DMA_GROUP * 4));
}
#endif /* lpspi0 dmas */
#endif /* E1C/B1 */
#endif /* dma2 */

static void prepare_data(uint32_t *data, uint16_t def_mask)
{
	for (uint32_t cnt = 0; cnt < BUFF_SIZE; cnt++) {
		data[cnt] = (def_mask << 16) | cnt;
	}
}

void spi_thread(void)
{
	const struct device *const dev = DEVICE_DT_GET(SPIDW_NODE);
	int ret;

#if DT_NODE_HAS_COMPAT_STATUS(DT_NODELABEL(dma2), arm_dma_pl330, okay)
#if (IS_ENABLED(CONFIG_SOC_SERIES_E1C) || IS_ENABLED(CONFIG_SOC_SERIES_B1))
#if DT_NODE_HAS_PROP(DT_NODELABEL(lpspi0), dmas)
	configure_lpspi0_for_dma2();
#endif
#endif
#endif

	if (!device_is_ready(dev)) {
		LOG_ERR("%s: Master device not ready.", dev->name);
		return;
	}

	prepare_data(master_txdata, 0xA5A5);

#if SPI_MASTER_SS_SW_CONTROLLED_GPIO
	struct spi_cs_control cs_ctrl = (struct spi_cs_control){
		.gpio = GPIO_DT_SPEC_GET(SPIDW_NODE, cs_gpios),
		.delay = 100u,
	};
#else
	struct spi_cs_control cs_ctrl = {0};
#endif

	while (1) {
		ret = master_spi_transceive(dev, &cs_ctrl);
		k_msleep(SLEEPTIME);
		if (ret != 0) {
			LOG_ERR("Stopping the SPI thread due to error");
		}
	}
}
#endif /* SPI_NODE_OKAY */
