/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/mgmt/mcumgr/mgmt/mgmt.h>
#include <mgmt/mcumgr/transport/smp_internal.h>

#include "alif_ble.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_endian.h"
#include "gatt_db.h"
#include "batt_svc.h"
#include "shared_control.h"
#include "address_verification.h"
#include <alif/bluetooth/bt_adv_data.h>
#include <alif/bluetooth/bt_scan_rsp.h>
#include "gapm_api.h"

extern void service_conn(struct shared_control *ctrl);
struct shared_control ctrl = { false, 0, 0 };

/* Define advertising address type */
#define SAMPLE_ADDR_TYPE	ALIF_PUBLIC_ADDR

/* Store and share advertising address type */
static uint8_t adv_type;

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define DEVICE_NAME CONFIG_BLE_DEVICE_NAME

/* Standard GATT 16 bit UUIDs must be extended to 128 bits when using gatt_att_desc_t */
#define GATT_DECL_PRIMARY_SERVICE_UUID128                                                          \
	{GATT_DECL_PRIMARY_SERVICE & 0xFF,                                                         \
	 (GATT_DECL_PRIMARY_SERVICE >> 8) & 0xFF,                                                  \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0}

#define GATT_DECL_CHARACTERISTIC_UUID128                                                           \
	{GATT_DECL_CHARACTERISTIC & 0xFF,                                                          \
	 (GATT_DECL_CHARACTERISTIC >> 8) & 0xFF,                                                   \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0}

#define GATT_DESC_CLIENT_CHAR_CFG_UUID128                                                          \
	{GATT_DESC_CLIENT_CHAR_CFG & 0xFF,                                                         \
	 (GATT_DESC_CLIENT_CHAR_CFG >> 8) & 0xFF,                                                  \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0,                                                                                        \
	 0}

/* SMP service 8D53DC1D-1DB7-4CD3-868B-8A527460AA84 */
#define SMP_SERVICE_UUID128                                                                        \
	{0x84, 0xAA, 0x60, 0x74, 0x52, 0x8A, 0x8B, 0x86,                                           \
	 0xD3, 0x4C, 0xB7, 0x1D, 0x1D, 0xDC, 0x53, 0x8D}

/* SMP characteristic DA2E7828-FBCE-4E01-AE9E-261174997C48 */
#define SMP_CHARACTERISTIC_UUID128                                                                 \
	{0x48, 0x7C, 0x99, 0x74, 0x11, 0x26, 0x9E, 0xAE,                                           \
	 0x01, 0x4E, 0xCE, 0xFB, 0x28, 0x78, 0x2E, 0xDA}

enum smp_gatt_id {
	SMP_GATT_ID_SERVICE = 0,
	SMP_GATT_ID_CHAR,
	SMP_GATT_ID_VAL,
	SMP_GATT_ID_NTF_CFG,
	SMP_GATT_ID_END,
};

struct smp_environment {
	uint8_t conidx;
	uint8_t adv_actv_idx;
	uint16_t ntf_cfg;
	uint16_t start_hdl;
	uint8_t user_lid;
	struct k_sem ntf_sem;
	struct smp_transport transport;
};

static struct smp_environment env;

static uint16_t create_adv_data(uint8_t actv_idx)
{
	const uint8_t svc_uuid[GATT_UUID_128_LEN] = SMP_SERVICE_UUID128;
	int ret;

	ret = bt_adv_data_set_tlv(GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID, svc_uuid,
				  GATT_UUID_128_LEN);
	if (ret) {
		LOG_ERR("AD profile set fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	ret = bt_adv_data_set_name_auto(DEVICE_NAME, strlen(DEVICE_NAME));

	if (ret) {
		LOG_ERR("AD device name data fail %d", ret);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	return bt_gapm_advertiment_data_set(actv_idx);
}

static void on_cb_event_sent(uint8_t conidx, uint8_t user_lid, uint16_t metainfo, uint16_t status)
{
	if (status != GAP_ERR_NO_ERROR) {
		LOG_ERR("Notification send callback failed, status: %u", status);
	}

	k_sem_give(&env.ntf_sem);
}

static void on_cb_att_read_get(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
			       uint16_t offset, uint16_t max_length)
{
	uint16_t rc;
	co_buf_t *p_buf = NULL;
	uint16_t idx = hdl - env.start_hdl;

	switch (idx) {
	case SMP_GATT_ID_NTF_CFG:
		rc = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, PRF_CCC_DESC_LEN,
				  GATT_BUFFER_TAIL_LEN);
		if (rc != CO_BUF_ERR_NO_ERROR) {
			rc = GAP_ERR_INSUFF_RESOURCES;
			break;
		}

		co_write16(co_buf_data(p_buf), co_htole16(env.ntf_cfg));

		LOG_INF("Value read notification configuration (conidx: %u), config: %u", conidx,
			env.ntf_cfg);
		rc = GAP_ERR_NO_ERROR;
		break;

	default:
		LOG_ERR("Value read to unknown characteristic (conidx: %u), idx: %u", conidx, idx);
		rc = ATT_ERR_REQUEST_NOT_SUPPORTED;
		break;
	}

	rc = gatt_srv_att_read_get_cfm(conidx, user_lid, token, rc, co_buf_data_len(p_buf), p_buf);
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to confirm value read (conidx: %u), error: %u", conidx, rc);
	}

	if (p_buf != NULL) {
		co_buf_release(p_buf);
	}
}

static uint16_t utils_process_smp_req(const void *p_data, uint16_t len)
{
	struct net_buf *nb;

	nb = smp_packet_alloc();
	if (!nb) {
		return ATT_ERR_INSUFF_RESOURCE;
	}

	if (net_buf_tailroom(nb) < len) {
		smp_packet_free(nb);
		return ATT_ERR_INSUFF_RESOURCE;
	}

	net_buf_add_mem(nb, p_data, len);
	smp_rx_req(&env.transport, nb);

	return GAP_ERR_NO_ERROR;
}

static uint16_t utils_process_ntf_cfq_req(const void *p_data, uint16_t len)
{
	uint16_t cfg;

	if (len != PRF_CCC_DESC_LEN) {
		return ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN;
	}

	memcpy(&cfg, p_data, PRF_CCC_DESC_LEN);

	if (cfg != PRF_CLI_START_NTF && cfg != PRF_CLI_STOP_NTFIND) {
		return ATT_ERR_REQUEST_NOT_SUPPORTED;
	}

	env.ntf_cfg = cfg;

	return GAP_ERR_NO_ERROR;
}

static void on_cb_att_val_set(uint8_t conidx, uint8_t user_lid, uint16_t token, uint16_t hdl,
			      uint16_t offset, co_buf_t *p_data)
{
	uint16_t rc;
	uint16_t idx = hdl - env.start_hdl;

	switch (idx) {
	case SMP_GATT_ID_VAL:
		rc = utils_process_smp_req(co_buf_data(p_data), co_buf_data_len(p_data));
		if (rc != GAP_ERR_NO_ERROR) {
			LOG_ERR("Failed to process SMP request (conidx: %u), error: %u", conidx,
				rc);
			break;
		}

		LOG_INF("Received SMP request (conidx: %u)", conidx);
		break;

	case SMP_GATT_ID_NTF_CFG:
		rc = utils_process_ntf_cfq_req(co_buf_data(p_data), co_buf_data_len(p_data));
		if (rc != GAP_ERR_NO_ERROR) {
			LOG_ERR("Failed to process notification configuration (conidx: %u), error: "
				"%u",
				conidx, rc);
			break;
		}

		LOG_INF("Received notification configuration (conidx: %u), config: %u", conidx,
			env.ntf_cfg);
		break;

	default:
		LOG_ERR("Value set to unknown characteristic (conidx: %u), idx: %u", conidx, idx);
		rc = ATT_ERR_REQUEST_NOT_SUPPORTED;
		break;
	}

	rc = gatt_srv_att_val_set_cfm(conidx, user_lid, token, rc);
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to confirm value set (conidx: %u), error: %u", conidx, rc);
	}
}

static uint16_t utils_add_service(void)
{
	uint16_t rc;

	static const gatt_srv_cb_t gatt_cbs = {
		.cb_event_sent = on_cb_event_sent,
		.cb_att_read_get = on_cb_att_read_get,
		.cb_att_val_set = on_cb_att_val_set,
	};

	static const gatt_att_desc_t att_desc[] = {
		[SMP_GATT_ID_SERVICE] = {GATT_DECL_PRIMARY_SERVICE_UUID128, ATT_UUID(16) | PROP(RD),
					 0},

		[SMP_GATT_ID_CHAR] = {GATT_DECL_CHARACTERISTIC_UUID128, ATT_UUID(16) | PROP(RD), 0},

		[SMP_GATT_ID_VAL] = {SMP_CHARACTERISTIC_UUID128, ATT_UUID(128) | PROP(WC) | PROP(N),
				     CFG_ATT_VAL_MAX | OPT(NO_OFFSET)},

		[SMP_GATT_ID_NTF_CFG] = {GATT_DESC_CLIENT_CHAR_CFG_UUID128,
					 ATT_UUID(16) | PROP(RD) | PROP(WR),
					 PRF_CCC_DESC_LEN | OPT(NO_OFFSET)},
	};

	static const uint8_t service_uuid[] = SMP_SERVICE_UUID128;

	LOG_INF("Registering GATT server");

	rc = gatt_user_srv_register(CFG_MAX_LE_MTU, 0, &gatt_cbs, &env.user_lid);
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to register gatt server, error: %u", rc);
		return rc;
	}

	LOG_INF("Adding GATT service");

	rc = gatt_db_svc_add(env.user_lid, SVC_UUID(128), service_uuid, SMP_GATT_ID_END, NULL,
			     att_desc, SMP_GATT_ID_END, &env.start_hdl);
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to add gatt service, error: %u", rc);
		return rc;
	}

	LOG_INF("GATT service added, start_hdl: %u", env.start_hdl);

	return GAP_ERR_NO_ERROR;
}

static uint16_t create_advertising(void)
{
	gapm_le_adv_create_param_t adv_create_params = {
		.prop = GAPM_ADV_PROP_UNDIR_CONN_MASK,
		.disc_mode = GAPM_ADV_MODE_GEN_DISC,
		.tx_pwr = 0,
		.filter_pol = GAPM_ADV_ALLOW_SCAN_ANY_CON_ANY,
		.prim_cfg = {
				.adv_intv_min = 160,
				.adv_intv_max = 800,
				.ch_map = ADV_ALL_CHNLS_EN,
				.phy = GAPM_PHY_TYPE_LE_1M,
			},
	};

	return bt_gapm_le_create_advertisement_service(adv_type, &adv_create_params, NULL,
						      &env.adv_actv_idx);
}

void app_connection_status_update(enum gapm_connection_event con_event, uint8_t con_idx,
				  uint16_t status)
{
	switch (con_event) {
	case GAPM_API_SEC_CONNECTED_KNOWN_DEVICE:
		env.conidx = con_idx;
		ctrl.connected = true;
		break;
	case GAPM_API_DEV_CONNECTED:
		env.conidx = con_idx;
		ctrl.connected = true;
		break;
	case GAPM_API_DEV_DISCONNECTED:
		LOG_INF("Client disconnected (conidx: %u), restating advertising", con_idx);

		smp_rx_remove_invalid(&env.transport, NULL);

		env.conidx = GAP_INVALID_CONIDX;
		env.ntf_cfg = PRF_CLI_STOP_NTFIND;
		k_sem_give(&env.ntf_sem);

		ctrl.connected = false;
		LOG_INF("BLE disconnected conn:%d. Waiting new connection", con_idx);
		break;
	case GAPM_API_PAIRING_FAIL:
		LOG_INF("Connection pairing index %u fail for reason %u", con_idx, status);
		break;
	}
}

static gapm_user_cb_t gapm_user_cb = {
	.connection_status_update = app_connection_status_update,
};

static uint16_t utils_config_gapm(void)
{
	static gapm_config_t gapm_cfg = {
		.role = GAP_ROLE_LE_PERIPHERAL,
		.pairing_mode = GAPM_PAIRING_DISABLE,
		.pairing_min_req_key_size = 0,
		.privacy_cfg = GAPM_PRIV_CFG_PRIV_EN_BIT,
		.renew_dur = 1500,
		.private_identity.addr = {0xC0, 0x01, 0x23, 0x45, 0x67, 0x89},
		.irk.key = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		.gap_start_hdl = 0,
		.gatt_start_hdl = 0,
		.att_cfg = 0,
		.sugg_max_tx_octets = GAP_LE_MIN_OCTETS,
		.sugg_max_tx_time = GAP_LE_MIN_TIME,
		.tx_pref_phy = GAP_PHY_ANY,
		.rx_pref_phy = GAP_PHY_ANY,
		.tx_path_comp = 0,
		.rx_path_comp = 0,
		.class_of_device = 0,
		.dflt_link_policy = 0,
	};

	if (address_verification(SAMPLE_ADDR_TYPE, &adv_type, &gapm_cfg)) {
		LOG_ERR("Address verification failed");
		return -EADV;
	}

	return bt_gapm_init(&gapm_cfg, &gapm_user_cb, DEVICE_NAME, strlen(DEVICE_NAME));
}

static uint16_t utils_get_mtu(void)
{
	uint16_t mtu;

	alif_ble_mutex_lock(K_FOREVER);
	mtu = gatt_bearer_mtu_min_get(env.conidx);
	alif_ble_mutex_unlock();

	return mtu - GATT_NTF_HEADER_LEN;
}

static uint16_t utils_send_ntf(const void *p_data, uint16_t len)
{
	uint16_t rc;
	co_buf_t *p_buf;

	k_sem_reset(&env.ntf_sem);

	alif_ble_mutex_lock(K_FOREVER);

	do {
		rc = co_buf_alloc(&p_buf, GATT_BUFFER_HEADER_LEN, len, GATT_BUFFER_TAIL_LEN);
		if (rc != CO_BUF_ERR_NO_ERROR) {
			rc = GAP_ERR_INSUFF_RESOURCES;
			break;
		}

		memcpy(co_buf_data(p_buf), p_data, len);
		rc = gatt_srv_event_send(env.conidx, env.user_lid, 0, GATT_NOTIFY,
					 env.start_hdl + SMP_GATT_ID_VAL, p_buf);
		co_buf_release(p_buf);
	} while (false);

	alif_ble_mutex_unlock();

	if (rc == GAP_ERR_NO_ERROR) {
		k_sem_take(&env.ntf_sem, K_FOREVER);
	}

	return rc;
}

static int transport_out(struct net_buf *nb)
{
	int rc = MGMT_ERR_EOK;
	uint16_t off = 0;
	uint16_t mtu;
	uint16_t tx_size;
	uint16_t tx_rc;

	/* SMP response packet might be bigger than MTU, transmit response in MTU size chunks */

	mtu = utils_get_mtu();

	while (off < nb->len) {
		tx_size = MIN(nb->len - off, mtu);

		tx_rc = utils_send_ntf(&nb->data[off], tx_size);
		if (tx_rc != GAP_ERR_NO_ERROR) {
			LOG_ERR("Failed to send notification, error: %u", tx_rc);
			rc = MGMT_ERR_EUNKNOWN;
			break;
		}

		off += tx_size;
	}

	LOG_INF("Sent SMP response notification (conidx: %u)", env.conidx);

	smp_packet_free(nb);

	return rc;
}

static uint16_t transport_get_mtu(const struct net_buf *nb)
{
	ARG_UNUSED(nb);
	/* Seems that current SMP implementation does not call get_mtu at all, so output function
	 * needs to handle MTU by itself.
	 */
	return utils_get_mtu();
}

static bool transport_query_valid_check(struct net_buf *nb, void *arg)
{
	ARG_UNUSED(nb);
	ARG_UNUSED(arg);
	/* Mark all pending requests invalid when smp_rx_remove_invalid is called on disconnection.
	 */
	return false;
}

int main(void)
{
	int rc;
	uint16_t err;

	LOG_INF("Alif smp_svr build time: " __DATE__ " " __TIME__);

	env.conidx = GAP_INVALID_CONIDX;
	env.adv_actv_idx = GAP_INVALID_ACTV_IDX;
	env.ntf_cfg = PRF_CLI_STOP_NTFIND;
	env.start_hdl = GATT_INVALID_HDL;
	env.user_lid = GATT_INVALID_USER_LID;
	k_sem_init(&env.ntf_sem, 0, 1);

	env.transport.functions.output = transport_out;
	env.transport.functions.get_mtu = transport_get_mtu;
	env.transport.functions.query_valid_check = transport_query_valid_check;

	rc = smp_transport_init(&env.transport);
	if (rc != 0) {
		LOG_ERR("Failed to init transport");
		return -1;
	}

	LOG_INF("Enabling Alif BLE stack");
	rc = alif_ble_enable(NULL);
	if (rc) {
		LOG_ERR("Failed to enable Alif BLE stack, error: %i", rc);
		return -1;
	}

	err = utils_config_gapm();
	if (err != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to configure GAP, error: %u", err);
		return -1;
	}

	/* Share connection info */
	service_conn(&ctrl);
	/* Config Battery service */
	config_battery_service();

	LOG_INF("Creating service");
	rc = utils_add_service();
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to add service, error: %u", rc);
		return -1;
	}

	LOG_INF("Creating advertisement");
	rc = create_advertising();
	if (rc != GAP_ERR_NO_ERROR) {
		LOG_ERR("Failed to create advertising activity, error: %u", rc);
		return -1;
	}

	err = create_adv_data(env.adv_actv_idx);
	if (err) {
		LOG_ERR("Advertisement data set fail %u", err);
		return -1;
	}

	err = bt_gapm_scan_response_set(env.adv_actv_idx);
	if (err) {
		LOG_ERR("Scan response set fail %u", err);
		return -1;
	}

	err = bt_gapm_advertisement_start(env.adv_actv_idx);
	if (err) {
		LOG_ERR("Advertisement start fail %u", err);
		return -1;
	}

	print_device_identity();

	LOG_INF("Waiting for SMP requests...");
	while (1) {
		k_sleep(K_SECONDS(1));
		battery_process();
	}

	return 0;
}
