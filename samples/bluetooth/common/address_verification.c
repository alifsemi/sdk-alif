/* Copyright (C) 2025 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */


#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/random/random.h>
#include "address_verification.h"
#include "gapm_le.h"

LOG_MODULE_REGISTER(address, LOG_LEVEL_DBG);

gapm_config_t *local_gapm_cfg;

uint8_t address_verif(uint8_t SAMPLE_ADDR_TYPE, gapm_config_t *gapm_cfg)
{
	int8_t adv_type = GAPM_STATIC_ADDR; /*Default return value*/

	local_gapm_cfg = gapm_cfg;
	switch (SAMPLE_ADDR_TYPE) {
	case ALIF_STATIC_RAND_ADDR:
		local_gapm_cfg->privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT,
		sys_rand_get(gapm_cfg->private_identity.addr, GAP_BD_ADDR_LEN);
		gapm_cfg->private_identity.addr[5] |= 0xC0; /*MSB position*/
		adv_type = GAPM_STATIC_ADDR; /*Static random address*/
		break;
	case ALIF_PUBLIC_ADDR:
		local_gapm_cfg->privacy_cfg = 0;
		OG_DBG("Using public address");
		adv_type = GAPM_STATIC_ADDR; /*Public address*/
		break;
	default:
		break;
	case ALIF_GEN_RSLV_RAND_ADDR:
		local_gapm_cfg->privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT;
		gapm_cfg->irk = (gap_sec_key_t) {.key = {0x12, 0xCE,
		0xD2, 0x2F, 0x32, 0x5A,
		0x61, 0x2A, 0x7E, 0x1A, 0x1B, 0x3B,
		0x2A, 0x8D, 0xA1, 0xA4}};
		adv_type = GAPM_GEN_RSLV_ADDR; /*Resolvable random address*/
		break;
	case ALIF_GEN_NON_RSLV_RAND_ADDR:
		local_gapm_cfg->privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT;
		adv_type = GAPM_GEN_NON_RSLV_ADDR; /*Non-resolvable random address*/
		break;

	}
	return adv_type;
}
