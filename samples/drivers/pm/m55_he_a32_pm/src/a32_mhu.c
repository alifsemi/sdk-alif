/* Copyright (C) 2026 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <aipm.h>
#include <se_service.h>
#include <zephyr/device.h>
#include <zephyr/drivers/ipm.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/policy.h>

#include "a32_going_off.h"
#include "a32_mhu.h"

LOG_MODULE_REGISTER(a32_mhu, LOG_LEVEL_INF);

#if DT_NODE_EXISTS(DT_ALIAS(apssmhu0r)) && DT_NODE_EXISTS(DT_ALIAS(apssmhu0s))
#define A32_MHU_RX_NODE DT_ALIAS(apssmhu0r)
#define A32_MHU_TX_NODE DT_ALIAS(apssmhu0s)
#elif DT_NODE_EXISTS(DT_ALIAS(apsshemhu0r)) && DT_NODE_EXISTS(DT_ALIAS(apsshemhu0s))
#define A32_MHU_RX_NODE DT_ALIAS(apsshemhu0r)
#define A32_MHU_TX_NODE DT_ALIAS(apsshemhu0s)
#else
#error "Need HE<->APSS MHU0 aliases (apssmhu0r/s or apsshemhu0r/s)"
#endif

#define A32_TX_TIMEOUT_MS		20000
#define A32_REQ_ACK_TIMEOUT_MS		20000
#define A32_REQ_SEND_RETRIES		5
#define A32_PD9_POLL_TIMEOUT_MS		30000
#define A32_PD9_POLL_SLICE_MS		100
#define A32_LINUX_CLAIM_WAIT_S		20

static const struct device *a32_mhu_rx;
static const struct device *a32_mhu_tx;

static K_SEM_DEFINE(a32_sem_sent, 0, 1);
static K_SEM_DEFINE(a32_sem_req_ack, 0, 1);

static volatile bool a32_going_off_seen;
static volatile enum a32_cluster_state a32_state = A32_STATE_ON;
static uint32_t a32_tx_msg;

static void a32_recv_cb(const struct device *dev, void *user_data,
			uint32_t id, volatile void *data)
{
	uint32_t msg;
	uint32_t ack;
	int ret;

	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);

	if ((id != A32_GOING_OFF_CHANNEL) || (data == NULL)) {
		return;
	}

	msg = *((volatile uint32_t *)data);
	LOG_INF("RX MHU0 Ch%u = 0x%x", id, msg);

	if (msg == M55_PERIPH_OFF_REQ_ACK) {
		k_sem_give(&a32_sem_req_ack);
		return;
	}

	if (msg != A32_GOING_OFF) {
		return;
	}

	/* ACK immediately: TF-A has a bounded ACK window. */
	a32_state = A32_STATE_SHUTDOWN;
	a32_going_off_seen = true;
	ack = A32_GOING_OFF_ACK;
	ret = ipm_send(a32_mhu_tx, 0, A32_GOING_OFF_CHANNEL, &ack, sizeof(ack));
	if (ret != 0) {
		LOG_ERR("A32_GOING_OFF_ACK send failed: %d", ret);
	}
}

static void a32_send_cb(const struct device *dev, void *user_data,
			uint32_t id, volatile void *data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(user_data);
	ARG_UNUSED(data);
	ARG_UNUSED(id);

	k_sem_give(&a32_sem_sent);
}

static int a32_send_word(uint32_t value)
{
	int ret;

	a32_tx_msg = value;
	k_sem_reset(&a32_sem_sent);
	LOG_INF("sending 0x%x on MHU0 Ch%u", value, A32_GOING_OFF_CHANNEL);

	ret = ipm_send(a32_mhu_tx, 0, A32_GOING_OFF_CHANNEL, &a32_tx_msg,
		       sizeof(a32_tx_msg));
	if (ret != 0) {
		LOG_ERR("ipm_send failed: %d", ret);
		return ret;
	}

	if (k_sem_take(&a32_sem_sent, K_MSEC(A32_TX_TIMEOUT_MS)) != 0) {
		LOG_ERR("TX completion timeout for 0x%x", value);
		return -EAGAIN;
	}

	return 0;
}

static int a32_pd9_is_off(void)
{
	run_profile_t runp;
	int ret;

	ret = se_service_get_run_cfg(&runp);
	if (ret != 0) {
		LOG_ERR("GET_RUN failed: %d", ret);
		return ret;
	}

	LOG_INF("GET_RUN domains=0x%x PD9(APSS)=%s", runp.power_domains,
		(runp.power_domains & PD2_APPS_MASK) ? "ON" : "OFF");

	return ((runp.power_domains & PD2_APPS_MASK) == 0U) ? 1 : 0;
}

/*
 * GET_RUN is live SoC state (OR of every master's vote). Do not feed that
 * snapshot back into SET_RUN. Only change this core's last requested profile.
 */
static int a32_drop_se_pd9(void)
{
	run_profile_t live;
	run_profile_t requested;
	int ret;

	ret = se_service_get_run_cfg(&live);
	if (ret != 0) {
		LOG_ERR("GET_RUN failed: %d", ret);
		return ret;
	}

	ret = se_service_get_last_set_run_cfg(&requested);
	if (ret == 0) {
		if ((requested.power_domains & PD2_APPS_MASK) != 0U) {
			requested.power_domains &= ~PD2_APPS_MASK;
			ret = se_service_set_run_cfg(&requested);
			if (ret != 0) {
				LOG_ERR("SET_RUN clear PD9 failed: %d", ret);
				return ret;
			}
		}
	} else {
		LOG_WRN("no last SET_RUN cache (%d)", ret);
	}

	ret = se_service_get_run_cfg(&live);
	if (ret != 0) {
		return ret;
	}

	return ((live.power_domains & PD2_APPS_MASK) == 0U) ? 0 : -EIO;
}

int a32_mhu_shutdown(void)
{
	int waited;
	int pd9;

	a32_mhu_rx = DEVICE_DT_GET(A32_MHU_RX_NODE);
	a32_mhu_tx = DEVICE_DT_GET(A32_MHU_TX_NODE);

	if (!device_is_ready(a32_mhu_rx) || !device_is_ready(a32_mhu_tx)) {
		LOG_ERR("MHU0 not ready");
		return -ENODEV;
	}

	ipm_register_callback(a32_mhu_rx, a32_recv_cb, NULL);
	ipm_register_callback(a32_mhu_tx, a32_send_cb, NULL);

	if (ipm_set_enabled(a32_mhu_rx, true) != 0) {
		LOG_ERR("failed to enable MHU0 RX");
		return -EIO;
	}

	/*
	 * Snippet may enable SUSPEND_TO_IDLE. Lock IWIC for the handshake
	 * so the wait cannot stall in idle before the send.
	 */
	pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);

	LOG_INF("waiting %d s for Linux MHU0 client", A32_LINUX_CLAIM_WAIT_S);
	k_sleep(K_SECONDS(A32_LINUX_CLAIM_WAIT_S));

	a32_state = A32_STATE_OFF_REQ_SENT;
	k_sem_reset(&a32_sem_req_ack);
	for (waited = 0; waited < A32_REQ_SEND_RETRIES; waited++) {
		if (a32_send_word(M55_PERIPH_OFF_REQ) == 0) {
			break;
		}

		pd9 = a32_pd9_is_off();
		if (pd9 == 1) {
			a32_state = A32_STATE_OFF;
			break;
		}
		k_sleep(K_SECONDS(2));
	}

	if (a32_state == A32_STATE_OFF) {
		goto pd9_off_done;
	}

	if (waited == A32_REQ_SEND_RETRIES) {
		LOG_ERR("M55_PERIPH_OFF_REQ not accepted");
		a32_state = A32_STATE_FAIL;
		return -EAGAIN;
	}

	if (k_sem_take(&a32_sem_req_ack, K_MSEC(A32_REQ_ACK_TIMEOUT_MS)) != 0) {
		LOG_ERR("no M55_PERIPH_OFF_REQ_ACK from Linux");
		a32_state = A32_STATE_FAIL;
		return -ETIMEDOUT;
	}

	LOG_INF("got M55_PERIPH_OFF_REQ_ACK; waiting for PD9 OFF");

	waited = 0;
	while (waited < A32_PD9_POLL_TIMEOUT_MS) {
		if (a32_going_off_seen) {
			k_sleep(K_MSEC(200));
			if (a32_drop_se_pd9() == 0) {
				a32_state = A32_STATE_OFF;
				break;
			}
			LOG_ERR("SET_RUN did not clear PD9");
			a32_state = A32_STATE_FAIL;
			return -EIO;
		}

		pd9 = a32_pd9_is_off();
		if (pd9 == 1) {
			a32_state = A32_STATE_OFF;
			break;
		}
		k_sleep(K_MSEC(A32_PD9_POLL_SLICE_MS));
		waited += A32_PD9_POLL_SLICE_MS;
	}

	if (a32_state != A32_STATE_OFF) {
		LOG_ERR("PD9 still ON after %d s", A32_PD9_POLL_TIMEOUT_MS / 1000);
		a32_state = A32_STATE_FAIL;
		return -ETIMEDOUT;
	}

pd9_off_done:
	(void)ipm_set_enabled(a32_mhu_rx, false);
	LOG_INF("A32/PD9 OFF confirmed");
	return 0;
}
