/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/drivers/i2s.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/sys_io.h>
#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

LOG_MODULE_REGISTER(i2s_dma1, LOG_LEVEL_INF);

#if DT_NODE_EXISTS(DT_NODELABEL(i2s_rxtx))
#define I2S_NODE DT_NODELABEL(i2s_rxtx)
#define I2S_BASE DT_REG_ADDR(I2S_NODE)
#define I2S_DMA_READY \
	(IS_ENABLED(CONFIG_I2S_DW_USE_DMA) && DT_NODE_HAS_PROP(I2S_NODE, dmas))
#else
#define I2S_DMA_READY 0
#endif

#define I2S_REG_DMACR_OFFSET 0x200U
#define I2S_DMACR_RX_EN_BIT  BIT(16)
#define I2S_DMACR_TX_EN_BIT  BIT(17)

#define SAMPLE_FREQUENCY   48000U
#define SAMPLE_BIT_WIDTH   16U
#define NUMBER_OF_CHANNELS 2U
#define BLOCK_DURATION_MS  10U
#define FRAMES_PER_BLOCK   (SAMPLE_FREQUENCY / (1000U / BLOCK_DURATION_MS))
#define BLOCK_SIZE         (FRAMES_PER_BLOCK * NUMBER_OF_CHANNELS * sizeof(int16_t))
#define INITIAL_BLOCKS     4U
#define VERIFY_BLOCKS      4U
#define MAX_RX_BLOCKS      12U
#define TIMEOUT_MS         1000U
#define SLAB_COUNT         16U
#define RAMP_L_BASE        0x1000U
#define RAMP_R_BASE        0xA000U

#define OPT_MASTER (I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER)

static uint8_t __nocache dma_slab_buf[BLOCK_SIZE * SLAB_COUNT] __aligned(4);
static struct k_mem_slab dma_slab;
static bool dma_slab_ready;

static int slab_ensure(void)
{
	int ret;

	if (dma_slab_ready) {
		return 0;
	}

	ret = k_mem_slab_init(&dma_slab, dma_slab_buf, BLOCK_SIZE, SLAB_COUNT);
	if (ret == 0) {
		dma_slab_ready = true;
	}
	return ret;
}

static void skip_unless_dma(void)
{
	if (!I2S_DMA_READY) {
		LOG_INF("SKIP: I2S DMA not enabled in Kconfig/DT");
		ztest_test_skip();
	}
}

static int configure_dir(const struct device *dev, enum i2s_dir dir, uint8_t channels)
{
	struct i2s_config cfg = {
		.word_size = SAMPLE_BIT_WIDTH,
		.channels = channels,
		.format = I2S_FMT_DATA_FORMAT_I2S,
		.options = OPT_MASTER,
		.frame_clk_freq = SAMPLE_FREQUENCY,
		.mem_slab = &dma_slab,
		.block_size = BLOCK_SIZE,
		.timeout = TIMEOUT_MS,
	};

	return i2s_configure(dev, dir, &cfg);
}

static void stream_drop(const struct device *dev)
{
	struct i2s_config reset = {0};

	(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	(void)i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_DROP);
	(void)i2s_configure(dev, I2S_DIR_TX, &reset);
	(void)i2s_configure(dev, I2S_DIR_RX, &reset);
	k_sleep(K_MSEC(20));
}

static void fill_pattern(uint16_t *buf, uint32_t seq)
{
	for (uint32_t f = 0U; f < FRAMES_PER_BLOCK; f++) {
		uint32_t g = seq * FRAMES_PER_BLOCK + f;

		buf[2U * f]     = (uint16_t)(RAMP_L_BASE + g);
		buf[2U * f + 1] = (uint16_t)(RAMP_R_BASE + g);
	}
}

static int prefill(const struct device *dev, uint32_t *seq, uint32_t n)
{
	int ret;

	for (uint32_t i = 0U; i < n; i++) {
		void *blk;

		ret = k_mem_slab_alloc(&dma_slab, &blk, K_NO_WAIT);
		if (ret < 0) {
			return ret;
		}
		fill_pattern(blk, (*seq)++);
		ret = i2s_write(dev, blk, BLOCK_SIZE);
		if (ret < 0) {
			k_mem_slab_free(&dma_slab, blk);
			return ret;
		}
	}
	return 0;
}

static int start_tx_rx(const struct device *dev)
{
	int ret;

	ret = i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_START);
	if (ret < 0) {
		return ret;
	}
	ret = i2s_trigger(dev, I2S_DIR_RX, I2S_TRIGGER_START);
	if (ret < 0) {
		(void)i2s_trigger(dev, I2S_DIR_TX, I2S_TRIGGER_DROP);
	}
	return ret;
}

static int find_sync(const uint16_t *buf, uint32_t sent_frames,
		     uint32_t *g_out, uint32_t *frame_out)
{
	for (uint32_t f = 0U; f < FRAMES_PER_BLOCK; f++) {
		uint16_t l = buf[2U * f];
		uint16_t r = buf[2U * f + 1];
		uint32_t g;

		if (l < RAMP_L_BASE) {
			continue;
		}
		g = (uint32_t)l - RAMP_L_BASE;
		if (g >= sent_frames) {
			continue;
		}
		if (r == (uint16_t)(RAMP_R_BASE + g)) {
			*g_out = g;
			*frame_out = f;
			return 0;
		}
	}
	return -ENOENT;
}

static int check_ramp(const uint16_t *buf, uint32_t start_frame, uint32_t *g)
{
	for (uint32_t f = start_frame; f < FRAMES_PER_BLOCK; f++) {
		uint16_t exp_l = (uint16_t)(RAMP_L_BASE + *g);
		uint16_t exp_r = (uint16_t)(RAMP_R_BASE + *g);

		if (buf[2U * f] != exp_l || buf[2U * f + 1] != exp_r) {
			LOG_INF("mismatch g=%u frame=%u L exp=%04x got=%04x R exp=%04x got=%04x",
				*g, f, exp_l, buf[2U * f], exp_r, buf[2U * f + 1]);
			return -EIO;
		}
		(*g)++;
	}
	return 0;
}

ZTEST(i2s_dma1, test_dma_handshake)
{
	int ret;
	uint32_t seq = 0U;
	uint32_t dmacr;

	skip_unless_dma();
	zassert_equal(slab_ensure(), 0);

#if I2S_DMA_READY
	{
		const struct device *dev = DEVICE_DT_GET(I2S_NODE);

		zassert_true(device_is_ready(dev), "I2S not ready");
		ret = configure_dir(dev, I2S_DIR_RX, NUMBER_OF_CHANNELS);
		zassert_equal(ret, 0, "RX configure %d", ret);
		ret = configure_dir(dev, I2S_DIR_TX, NUMBER_OF_CHANNELS);
		zassert_equal(ret, 0, "TX configure %d", ret);
		ret = prefill(dev, &seq, INITIAL_BLOCKS);
		zassert_equal(ret, 0, "prefill %d", ret);
		ret = start_tx_rx(dev);
		zassert_equal(ret, 0, "START %d", ret);

		k_sleep(K_MSEC(5));
		dmacr = sys_read32(I2S_BASE + I2S_REG_DMACR_OFFSET);
		LOG_INF("DMACR 0x%08x", dmacr);
		stream_drop(dev);

		zassert_true((dmacr & I2S_DMACR_TX_EN_BIT) != 0U,
			     "DMACR TX DMA bit not set — IRQ path, not DMA");
		zassert_true((dmacr & I2S_DMACR_RX_EN_BIT) != 0U,
			     "DMACR RX DMA bit not set — IRQ path, not DMA");
		LOG_INF("PASS: DMA handshake");
	}
#endif
}

ZTEST(i2s_dma1, test_same_instance_loopback)
{
	uint32_t seq = 0U;
	uint32_t g = 0U;
	uint32_t verified = 0U;
	bool synced = false;
	const uint32_t need = VERIFY_BLOCKS * FRAMES_PER_BLOCK;
	int ret;

	skip_unless_dma();
	if (!IS_ENABLED(CONFIG_I2S_GPIO_LOOPBACK)) {
		LOG_INF("SKIP: CONFIG_I2S_GPIO_LOOPBACK=n");
		ztest_test_skip();
	}
	zassert_equal(slab_ensure(), 0);

#if I2S_DMA_READY
	{
		const struct device *dev = DEVICE_DT_GET(I2S_NODE);

		zassert_true(device_is_ready(dev), "I2S not ready");
		ret = configure_dir(dev, I2S_DIR_RX, NUMBER_OF_CHANNELS);
		zassert_equal(ret, 0, "RX configure %d", ret);
		ret = configure_dir(dev, I2S_DIR_TX, NUMBER_OF_CHANNELS);
		zassert_equal(ret, 0, "TX configure %d", ret);
		ret = prefill(dev, &seq, INITIAL_BLOCKS);
		zassert_equal(ret, 0, "prefill %d", ret);
		ret = start_tx_rx(dev);
		zassert_equal(ret, 0, "START %d", ret);

		for (uint32_t i = 0U; i < MAX_RX_BLOCKS && verified < need; i++) {
			void *rx_blk = NULL;
			void *tx_blk = NULL;
			size_t rx_size = 0U;
			uint16_t *samples;
			uint32_t start_frame = 0U;

			ret = i2s_read(dev, &rx_blk, &rx_size);
			if (ret < 0) {
				stream_drop(dev);
				zassert_equal(ret, 0, "i2s_read %u failed (%d)", i, ret);
			}
			zassert_equal(rx_size, BLOCK_SIZE, "short RX block");
			samples = rx_blk;

			if (!synced) {
				LOG_INF("rx blk %u head L/R %04x/%04x %04x/%04x",
					i, samples[0], samples[1], samples[2], samples[3]);
				ret = find_sync(samples, seq * FRAMES_PER_BLOCK, &g, &start_frame);
				if (ret == 0) {
					synced = true;
					LOG_INF("sync blk %u frame %u g=%u", i, start_frame, g);
				}
			}

			if (synced) {
				ret = check_ramp(samples, start_frame, &g);
				k_mem_slab_free(&dma_slab, rx_blk);
				if (ret < 0) {
					stream_drop(dev);
					zassert_equal(ret, 0, "bit-exact failed on block %u", i);
				}
				verified += FRAMES_PER_BLOCK - start_frame;
			} else {
				k_mem_slab_free(&dma_slab, rx_blk);
			}

			ret = k_mem_slab_alloc(&dma_slab, &tx_blk, K_NO_WAIT);
			if (ret < 0) {
				stream_drop(dev);
				zassert_equal(ret, 0, "TX alloc %u failed", i);
			}
			fill_pattern(tx_blk, seq++);
			ret = i2s_write(dev, tx_blk, BLOCK_SIZE);
			if (ret < 0) {
				k_mem_slab_free(&dma_slab, tx_blk);
				stream_drop(dev);
				zassert_equal(ret, 0, "i2s_write %u failed (%d)", i, ret);
			}
		}

		stream_drop(dev);
		zassert_true(synced, "never found ramp on RX — check SDO->SDI wire");
		zassert_true(verified >= need, "only %u/%u frames verified", verified, need);
		LOG_INF("PASS: loopback bit-exact, %u frames", verified);
	}
#endif
}

ZTEST_SUITE(i2s_dma1, NULL, NULL, NULL, NULL, NULL);
