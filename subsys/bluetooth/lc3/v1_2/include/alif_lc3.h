/*
 * Copyright (c) 2024 Alif Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _ALIF_LC3_H
#define _ALIF_LC3_H

#include "lc3_api.h"

/**
 * @brief Initialise the Alif LC3 codec. The codec must be initialised before any other functions
 *        from lc3_api.h can be used.
 * @return 0 on success, any other value indicates an error
 */
static inline int alif_lc3_init(void)
{
	void const *patch = NULL;

	return lc3_api_rom_init(patch);
}

#endif /* _ALIF_LC3_H */
