/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include "inference_process.hpp"

#include "ethosu/models/keyword_spotting_cnn_small_int8/u55/input.h"
#include "ethosu/models/keyword_spotting_cnn_small_int8/u55/output.h"

#if defined(CONFIG_ARM_ETHOS_U55_256)
#include "ethosu/models/keyword_spotting_cnn_small_int8/u55/model_u55_256.h"
#else
#include "ethosu/models/keyword_spotting_cnn_small_int8/u55/model_u55_128.h"
#endif

#include <zephyr/shell/shell.h>
#include <inttypes.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <zephyr/kernel.h>

static K_THREAD_STACK_DEFINE(ethosu_stack, CONFIG_ALIF_ETHOSU_SHELL_THREAD_STACKSIZE);
static struct k_thread ethosu_thread;
static k_sem ethosu_sem;
static atomic_t ethosu_running = 0;
static atomic_t ethosu_verbose = CONFIG_ETHOSU_VERBOSE_LEVEL;

__attribute__((section(".bss.tflm_arena"),
	       aligned(16))) static uint8_t tensor_arena[TENSOR_ARENA_SIZE];

static unsigned int verbose_to_cnt()
{
	if (atomic_get(&ethosu_verbose) == 1) {
		return 100;
	} else if (atomic_get(&ethosu_verbose) == 2) {
		return 1000;
	} else if (atomic_get(&ethosu_verbose) == 3) {
		return 5000;
	} else if (atomic_get(&ethosu_verbose) == 4) {
		return 10000;
	} else {
		return 50000;
	}
}

static void ethosu_worker(void *, void *, void *)
{
	InferenceProcess::InferenceProcess npu(tensor_arena, TENSOR_ARENA_SIZE);
	uint32_t jobcnt = 0;
	uint32_t last_print_jobcnt = 0;
	int64_t last_print_ms = k_uptime_get();
	bool status;

	while (true) {
		if (k_sem_take(&ethosu_sem, K_NO_WAIT) == 0) {
			break;
		}

		InferenceProcess::InferenceJob job(
			modelName,
			InferenceProcess::DataPtr(const_cast<uint8_t *>(networkModelData),
						  sizeof(networkModelData)),
			{InferenceProcess::DataPtr(const_cast<uint8_t *>(inputData),
						   sizeof(inputData))},
			{},
			{InferenceProcess::DataPtr(const_cast<uint8_t *>(expectedOutputData),
						   sizeof(expectedOutputData))});

		status = npu.runJob(job);
		jobcnt++;

		if (atomic_get(&ethosu_verbose) && (jobcnt % verbose_to_cnt()) == 0) {
			int64_t now_ms = k_uptime_get();
			uint32_t delta_jobs = jobcnt - last_print_jobcnt;
			uint32_t jobs_per_sec = 0;

			if (now_ms > last_print_ms) {
				jobs_per_sec = (uint32_t)((delta_jobs * 1000ULL) /
							  (uint64_t)(now_ms - last_print_ms));
			}

			printk("jobcnt=%u status=%s rate=%u jobs/s\n", jobcnt,
			       status ? "failed" : "ok", jobs_per_sec);

			last_print_jobcnt = jobcnt;
			last_print_ms = now_ms;
		}
	}
}

static int cmd_start(const struct shell *shell, size_t, char **)
{
	if (atomic_set(&ethosu_running, 1) == 1) {
		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Ethos-U55 already inferencing\n");
		return -1;
	}

	k_sem_init(&ethosu_sem, 0, 1);

	k_thread_create(&ethosu_thread, ethosu_stack, K_THREAD_STACK_SIZEOF(ethosu_stack),
			ethosu_worker, NULL, NULL, NULL, CONFIG_ALIF_ETHOSU_SHELL_THREAD_PRIORITY,
			0, K_NO_WAIT);

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Start Ethos-U55 inferencing - model %s\n",
		      modelName);
	return 0;
}

static int cmd_stop(const struct shell *shell, size_t, char **)
{
	if (atomic_set(&ethosu_running, 0) == 0) {
		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Ethos-U55 already stopped\n");
		return -1;
	}

	k_sem_give(&ethosu_sem);
	k_thread_join(&ethosu_thread, K_FOREVER);

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Stop Ethos-U55 inferencing\n");
	return 0;
}

static int cmd_verbose(const struct shell *shell, size_t argc, char **argv)
{
	char *end = NULL;
	long level;

	if (argc < 2) {
		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Usage: ethosu verbose <0-5>\n");
		return -1;
	}

	level = strtol(argv[1], &end, 10);
	if ((end == argv[1]) || (*end != '\0') || (level < 0) || (level > 5)) {
		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
			      "Invalid verbose level, expected 0-5\n");
		return -1;
	}

	atomic_set(&ethosu_verbose, (atomic_val_t)level);
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "verbose=%ld\n", level);
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_cmds, SHELL_CMD_ARG(start, NULL, "start", cmd_start, 1, 10),
			       SHELL_CMD_ARG(stop, NULL, "stop", cmd_stop, 1, 10),
			       SHELL_CMD_ARG(verbose, NULL, "verbose <0-5>", cmd_verbose, 2, 0),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(ethosu, &sub_cmds, "Ethos-U55 commands", NULL);
