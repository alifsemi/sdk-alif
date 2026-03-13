/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**
 * @file video_alloc.c
 *
 */

#include <aipl_video_alloc.h>
#include <dave_d0lib.h>
#include <stdint.h>

#define CACHE_LINE_SIZE 32
#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))

void *aipl_video_alloc(uint32_t size)
{
	/* Allocate extra space for alignment and storing original pointer */
	uint32_t total_size = size + CACHE_LINE_SIZE + sizeof(void *);
	void *raw = d0_allocvidmem(total_size);

	if (raw == NULL) {
		return NULL;
	}

	/* Align the usable pointer to cache line boundary */
	uintptr_t raw_addr = (uintptr_t)raw + sizeof(void *);
	uintptr_t aligned_addr = ALIGN_UP(raw_addr, CACHE_LINE_SIZE);
	void *aligned = (void *)aligned_addr;

	/* Store original pointer just before the aligned address */
	((void **)aligned)[-1] = raw;

	return aligned;
}

void aipl_video_free(void *ptr)
{
	if (ptr != NULL) {
		/* Retrieve original pointer and free it */
		void *raw = ((void **)ptr)[-1];

		d0_freevidmem(raw);
	}
}
