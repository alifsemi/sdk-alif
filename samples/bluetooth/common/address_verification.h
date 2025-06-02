/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */



#include "gapm.h"

enum alif_addr_type
{
	/*Generated static random address*/
	ALIF_STATIC_RAND_ADDR = 0u,
	/*Generated resolvable private random address*/
	ALIF_GEN_RSLV_RAND_ADDR,
	/*Generated non-resolvable private random address*/
	ALIF_GEN_NON_RSLV_RAND_ADDR,
	/*Generated Public Address*/
	ALIF_PUBLIC_ADDR,
};
uint8_t address_verif(uint8_t SAMPLE_ADDR_TYPE, gapm_config_t *gapm_cfg);
