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

uint8_t address_verif(uint8_t addr_type, uint8_t *adv_type, gapm_config_t *gapm_cfg)
{
	if (!gapm_cfg) {
		LOG_ERR("no gapm configuration provided");
		return -EINVAL;
	}

	switch (addr_type) {
	case ALIF_STATIC_RAND_ADDR:
		gapm_cfg->privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT; /*Privacy address bit*/
		sys_rand_get(gapm_cfg->private_identity.addr, GAP_BD_ADDR_LEN);
		gapm_cfg->private_identity.addr[5] |= 0xC0; /*MSB position*/
		*adv_type = GAPM_STATIC_ADDR; /*Static random address*/
		break;
	case ALIF_PUBLIC_ADDR:
		gapm_cfg->privacy_cfg = 0;
		LOG_DBG("Using public address");
		*adv_type = GAPM_STATIC_ADDR; /*Public address*/
		break;
	case ALIF_GEN_RSLV_RAND_ADDR:
		gapm_cfg->privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT; /*Privacy address bit*/
		*adv_type = GAPM_GEN_RSLV_ADDR; /*Resolvable random address*/
		gapm_cfg->privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT; /*Privacy address bit*/
		sys_rand_get(gapm_cfg->private_identity.addr, GAP_BD_ADDR_LEN);
		gapm_cfg->private_identity.addr[5] |= 0xC0; /*MSB position*/
		break;
	case ALIF_GEN_NON_RSLV_RAND_ADDR:
		gapm_cfg->privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT; /*Privacy address bit*/
		*adv_type = GAPM_GEN_NON_RSLV_ADDR; /*Non-resolvable random address*/
		gapm_cfg->privacy_cfg = GAPM_PRIV_CFG_PRIV_ADDR_BIT; /*Privacy address bit*/
		sys_rand_get(gapm_cfg->private_identity.addr, GAP_BD_ADDR_LEN);
		gapm_cfg->private_identity.addr[5] |= 0xC0; /*MSB position*/
		break;
	default:
		LOG_ERR("Invalid address type %u", addr_type);
		return -EINVAL;
	}
	return 0;
}
