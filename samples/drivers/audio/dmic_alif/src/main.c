/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/audio/dmic.h>
#include <string.h>

#define SAMPLE_BIT_WIDTH 16
#define TIMEOUT     1000

/* The number of channels tested - changes with the test case */
#define NUM_CHANNELS (2)
/*
 * Driver will allocate blocks from this slab to receive audio data into them.
 * Application, after getting a given block from the driver and processing its
 * data, needs to free that block.
 */
#define PCMJ_BLOCK_SIZE   (1024)

/* Number of blocks in the slab */
#define MEM_SLAB_NUM_BLOCKS  12

#define DATA_BLOCK_COUNT (10 * 10 * 2)

/* size of buffer where the whole data is stored */
#define DATA_SIZE        (PCMJ_BLOCK_SIZE * DATA_BLOCK_COUNT)

static unsigned char pcmj_data[DATA_SIZE];

/*
 * The list of channels to test
 * The number of channels should match the NUM_CHANNELS
 */
static unsigned int channel_list[8] = {4, 5};

static struct k_mem_slab mem_slab;
/* buffers for the mem slab */
static char buffers[PCMJ_BLOCK_SIZE * MEM_SLAB_NUM_BLOCKS];

static int config_channel(const struct device *dmic_dev,
			  struct dmic_cfg *cfg,
			  size_t block_count)
{
	int rc;
	int i;
	int k = 0;
	uint32_t data;

	if (dmic_dev == NULL || cfg == NULL) {
		printk("%s: input invalid\n", __func__);
		return -1;
	}

	dmic_configure(dmic_dev, cfg);

	rc = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
	if (rc < 0) {
		printk("dmic_trigger error\n");
		return rc;
	}

	k = 0;

	for (i = 0; i < block_count; ++i) {
		void *buffer;

		rc = dmic_read(dmic_dev, 0, &buffer, &data, TIMEOUT);
		if (rc < 0) {
			printk("block_count: %d\n", i);
			dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
			return rc;
		}

		/* copy the data from the buffer to the pcmj data */
		if ((k + data) <= DATA_SIZE) {
			memcpy(pcmj_data + k, buffer, data);
			k += data;
		}

		k_mem_slab_free(&mem_slab, &buffer);
	}

	printk("read okay\n");

	rc = dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);

	return rc;

}

void print_data(void)
{
	int32_t i;

	/* print the poitner where the data is stored and */
	/* first 80 bytes of data */

	printk("pcm data : %p\n", pcmj_data);

	for (i = 0; i < 80; i += 8) {
		printk("%x %x %x %x %x %x %x %x\n",
			pcmj_data[i], pcmj_data[i+1], pcmj_data[i+2],
			pcmj_data[i+3], pcmj_data[i+4], pcmj_data[i+5],
			pcmj_data[i+6], pcmj_data[i+7]);
	}
}

void init_slab(void)
{
	int rc;

	rc = k_mem_slab_init(&mem_slab, buffers,
			     PCMJ_BLOCK_SIZE, MEM_SLAB_NUM_BLOCKS);

	if (rc == 0) {
		printk("%s:slab_init okay\n", __func__);
	} else {
		printk("init mem slab error\n");
	}
}

void set_config(struct dmic_cfg *cfg, struct pcm_stream_cfg *stream)
{
	uint32_t channel_map = 0;
	int i;

	stream->pcm_width = SAMPLE_BIT_WIDTH;

	cfg->streams = stream;
	cfg->streams[0].mem_slab = &mem_slab;
	cfg->channel.req_num_streams = 1;
	cfg->channel.req_num_chan = NUM_CHANNELS;
	cfg->streams[0].block_size = PCMJ_BLOCK_SIZE;

	for (i = 0; i < NUM_CHANNELS; i++) {
		channel_map = channel_map | 1 << channel_list[i];
	}

	cfg->channel.req_chan_map_lo = channel_map;

	printk("memslab: %p\n", cfg->streams[0].mem_slab);
	printk("channel_map %x block size: %d\n",
		channel_map, PCMJ_BLOCK_SIZE);
}

void init_pdm(void)
{
	const struct device *pcmj_device;
	struct dmic_cfg cfg;
	struct pcm_stream_cfg stream;

	pcmj_device = DEVICE_DT_GET(DT_NODELABEL(pdm0));

	if (!pcmj_device) {
		printk("pcmj_device not found\n");
		return;
	}

	if (!device_is_ready(pcmj_device)) {
		printk("device not ready\n");
		return;
	}

	init_slab();

	set_config(&cfg, &stream);

	config_channel(pcmj_device, &cfg, DATA_BLOCK_COUNT);

}

int main(void *b1, void *b2, void *b3)
{
	init_pdm();
	print_data();

	return 0;
}
