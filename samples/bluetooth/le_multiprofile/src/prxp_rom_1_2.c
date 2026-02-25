/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "alif_ble.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "shared_control.h"

/* Profiles definitions */
#include "llss.h"
#include "iass.h"
#include "tpss.h"
#include "prxp_app.h"

LOG_MODULE_REGISTER(prxp, LOG_LEVEL_DBG);

void ll_notify(void);
void ias_reset(void);

/* profile callbacks */
static uint8_t ll_level;
static uint8_t iass_level;

void ll_notify(void)
{
	if (ll_level != LLS_ALERT_LEVEL_NONE) {
		LOG_WRN("Link lost alert with level 0x%02x", ll_level);
		ll_level = LLS_ALERT_LEVEL_NONE;
	}
}

void ias_reset(void)
{
	iass_level = IAS_ALERT_LEVEL_NONE;
}

static void on_get_level_req(uint8_t conidx, uint16_t token)
{
	co_buf_t *p_buf;

	prf_buf_alloc(&p_buf, LLS_ALERT_LEVEL_SIZE);
	*co_buf_data(p_buf) = ll_level;
	llss_get_level_cfm(conidx, token, p_buf);
	co_buf_release(p_buf);

	LOG_DBG("Level requested");
}

static void on_set_level_req(uint8_t conidx, uint16_t token, co_buf_t *p_buf)
{
	uint8_t level = *co_buf_data(p_buf);
	uint16_t status;

	if (level < LLS_ALERT_LEVEL_MAX) {
		ll_level = level;
		status = GAP_ERR_NO_ERROR;
		LOG_INF("Set level requested: %d", level);
	} else {
		status = ATT_ERR_VALUE_NOT_ALLOWED;
	}

	llss_set_level_cfm(conidx, status, token);
}

static const llss_cbs_t llss_cb = {
	.cb_get_level_req = on_get_level_req,
	.cb_set_level_req = on_set_level_req,
};

/* Add profile to the stack */
void prxp_server_configure(void)
{
	uint16_t err;

	/* Dynamic allocation of service start handle */
	uint16_t start_hdl = 0;

	alif_ble_mutex_lock(K_FOREVER);
	err = prf_add_profile(TASK_ID_LLSS, 0, 0, NULL, &llss_cb, &start_hdl);
	alif_ble_mutex_unlock();

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}
}

void on_gapm_err(uint32_t metainfo, uint8_t code)
{
	LOG_ERR("gapm error %d", code);
}

static const gapm_cb_t gapm_err_cbs = {
	.cb_hw_error = on_gapm_err,
};

gapm_callbacks_t prxp_append_cbs(gapm_callbacks_t *gapm_append_cbs)
{
	gapm_callbacks_t cbs = *gapm_append_cbs;

	cbs.p_gapm_cbs = &gapm_err_cbs;
	return cbs;
}

void ias_process(void)
{
	/* IAS alert shall continue until disconnection or set to None */
	if (iass_level == IAS_ALERT_LEVEL_MILD) {
		LOG_WRN("IAS mild alert");
	} else if (iass_level == IAS_ALERT_LEVEL_HIGH) {
		LOG_WRN("IAS high alert");
	}
}

void prxp_disc_notify(uint16_t reason)
{
	if (reason != LL_ERR_REMOTE_USER_TERM_CON) {
		ll_notify();
	}
	ias_reset();
}
