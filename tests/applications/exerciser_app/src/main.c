/*
 * Copyright (c) Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "exerciser_app.h"

LOG_MODULE_REGISTER(exerciser_main, LOG_LEVEL_INF);

/* Thread stack size and priority */
#define STACK_SIZE 1024

#if BLINKY_NODE_OKAY
#define BLINKY_PRIORITY 4
K_THREAD_STACK_DEFINE(blinky_stack, 512);
struct k_thread blinky_thread_data;
#endif

#if OSPI_NODE_OKAY
#define OSPI_PRIORITY 4
K_THREAD_STACK_DEFINE(ospi_stack, STACK_SIZE * 2);
struct k_thread ospi_thread_data;
#endif

#if SPI_NODE_OKAY
#define SPI_PRIORITY 4
K_THREAD_STACK_DEFINE(spi_stack, STACK_SIZE * 1);
struct k_thread spi_thread_data;
#endif

#if SD_NODE_OKAY
#define SD_PRIORITY 7
K_THREAD_STACK_DEFINE(sd_stack, STACK_SIZE * 4);
struct k_thread sd_thread_data;
#endif

#if USB_NODE_OKAY
#define USB_PRIORITY 2
K_THREAD_STACK_DEFINE(usb_stack, STACK_SIZE * 4);
struct k_thread usb_thread_data;
#endif

#if CODEC_NODE_OKAY
#define CODEC_PRIORITY 4
K_THREAD_STACK_DEFINE(codec_stack, STACK_SIZE * 8);
struct k_thread codec_thread_data;
#endif

#if BMI323_NODE_OKAY
#define BMI323_PRIORITY 4
K_THREAD_STACK_DEFINE(bmi323_stack, STACK_SIZE * 2);
struct k_thread bmi323_thread_data;
#endif

#if VIDEO_NODE_OKAY
#define VIDEO_PRIORITY 5
K_THREAD_STACK_DEFINE(video_stack, STACK_SIZE * 16);
struct k_thread video_thread_data;
#endif

#if ETH_NODE_OKAY
#define ETH_PRIORITY 4
K_THREAD_STACK_DEFINE(eth_stack, STACK_SIZE * 4);
struct k_thread eth_thread_data;
#endif

/* Function to start all exerciser threads */
void start_all_exerciser_threads(void)
{
	static atomic_t exerciser_started;

	if (!atomic_cas(&exerciser_started, 0, 1)) {
		LOG_WRN("Exerciser threads already started");
		return;
	}
#if BLINKY_NODE_OKAY
	k_tid_t tidb = k_thread_create(&blinky_thread_data, blinky_stack,
			K_THREAD_STACK_SIZEOF(blinky_stack),
			(k_thread_entry_t)blinky_thread,
			NULL, NULL, NULL, BLINKY_PRIORITY, 0, K_NO_WAIT);
	if (tidb == NULL) {
		LOG_ERR("Error creating Blinky Thread");
	} else {
		LOG_INF("Blinky thread started");
	}
#endif

#if OSPI_NODE_OKAY
	k_tid_t tido = k_thread_create(&ospi_thread_data, ospi_stack,
			K_THREAD_STACK_SIZEOF(ospi_stack),
			(k_thread_entry_t)ospi_thread,
			NULL, NULL, NULL, OSPI_PRIORITY, 0, K_NO_WAIT);
	if (tido == NULL) {
		LOG_ERR("Error creating Ospi Thread");
	} else {
		LOG_INF("OSPI thread started");
	}
#endif

#if SPI_NODE_OKAY
	k_tid_t tids = k_thread_create(&spi_thread_data, spi_stack, STACK_SIZE * 1,
			(k_thread_entry_t)spi_thread,
			NULL, NULL, NULL, SPI_PRIORITY, 0, K_NO_WAIT);
	if (tids == NULL) {
		LOG_ERR("Error creating SPI Master Thread");
	} else {
		LOG_INF("SPI thread started");
	}
#endif

#if USB_NODE_OKAY
	k_tid_t tidu = k_thread_create(&usb_thread_data, usb_stack, STACK_SIZE * 4,
			(k_thread_entry_t)usb_thread,
			NULL, NULL, NULL, USB_PRIORITY, 0, K_NO_WAIT);
	if (tidu == NULL) {
		LOG_ERR("Error creating USB thread");
	} else {
		LOG_INF("USB thread started");
	}
#endif

#if BMI323_NODE_OKAY
	k_tid_t tidbmi = k_thread_create(&bmi323_thread_data, bmi323_stack, STACK_SIZE * 2,
			(k_thread_entry_t)bmi323_thread,
			NULL, NULL, NULL, BMI323_PRIORITY, 2, K_NO_WAIT);
	if (tidbmi == NULL) {
		LOG_ERR("Error creating BMI323 thread");
	} else {
		LOG_INF("BMI323 thread started");
	}
#endif

#if CODEC_NODE_OKAY
	k_tid_t tidcodec = k_thread_create(&codec_thread_data, codec_stack, STACK_SIZE * 8,
			(k_thread_entry_t)codec_thread,
			NULL, NULL, NULL, CODEC_PRIORITY, K_FP_REGS, K_NO_WAIT);
	if (tidcodec == NULL) {
		LOG_ERR("Error creating Codec thread");
	} else {
		LOG_INF("Codec thread started");
	}
#endif

#if VIDEO_NODE_OKAY
	k_tid_t tidv = k_thread_create(&video_thread_data, video_stack,
			K_THREAD_STACK_SIZEOF(video_stack),
			(k_thread_entry_t)video_thread,
			NULL, NULL, NULL, VIDEO_PRIORITY, 0, K_NO_WAIT);
	if (tidv == NULL) {
		LOG_ERR("Error creating Video thread");
	} else {
		LOG_INF("Video thread started");
	}
#endif

#if ETH_NODE_OKAY
	k_tid_t tide = k_thread_create(&eth_thread_data, eth_stack,
			K_THREAD_STACK_SIZEOF(eth_stack),
			(k_thread_entry_t)eth_thread,
			NULL, NULL, NULL, ETH_PRIORITY, 0, K_NO_WAIT);
	if (tide == NULL) {
		LOG_ERR("Error creating DHCP thread");
	} else {
		LOG_INF("DHCP thread started");
	}
#endif

#if SD_NODE_OKAY
	k_tid_t tidsd = k_thread_create(&sd_thread_data, sd_stack,
			K_THREAD_STACK_SIZEOF(sd_stack),
			(k_thread_entry_t)sd_thread,
			NULL, NULL, NULL, SD_PRIORITY, 0, K_NO_WAIT);
	if (tidsd == NULL) {
		LOG_ERR("Error creating SD Thread");
	} else {
		LOG_INF("SD thread started");
	}
#endif

	LOG_INF("All exerciser threads started successfully!");
}

int main(void)
{
#ifdef CONFIG_SHELL
	/* Shell control mode - wait for user command */
	LOG_INF("Exerciser App - Shell Control Mode");
	LOG_INF("Use 'start_exerciser' command to start all threads");
	LOG_INF("Shell ready. Type 'help' for available commands.");
#else
	/* Auto-start mode - start all threads immediately */
	LOG_INF("Exerciser App - Auto-start Mode");
	LOG_INF("Starting all exerciser threads...");
	start_all_exerciser_threads();
#endif

	return 0;
}
