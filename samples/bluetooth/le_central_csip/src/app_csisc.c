/* Copyright (C) Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include "alif_ble.h"
#include "gapm.h"
#include "gap_le.h"
#include "gapc_le.h"
#include "gapc_sec.h"
#include "gapm_le.h"
#include "gapm_le_adv.h"
#include "co_buf.h"
#include "address_verification.h"

#include "prf.h"
#include "atc_csisc.h"
#include "atc_csisc_msg.h"
#include "csis.h"
#include "gapm_api.h"
#include "rwip_task.h"
#include "app_csisc.h"
#include "app_connection.h"

LOG_MODULE_REGISTER(csisc, LOG_LEVEL_DBG);

#define BUTTON_NODELABEL DT_NODELABEL(button0)

#define NUMBER_OF_SETS 3
#define NUMBER_OF_RSI  3

struct app_csisc_set_t {
	uint8_t set_lid;
	uint8_t key_lid;
	uint8_t conidx;
	bool sirk_valid;
	uint8_t sirk[CSIS_SIRK_LEN];
	uint8_t char_type_size;
	uint8_t char_type_rank;
	uint8_t char_type_lock;
	csisc_csis_info_t csis_info;
};

struct app_csisc_env_t {
	struct app_csisc_set_t sets[NUMBER_OF_SETS];
	uint16_t active_sets;
	sys_slist_t dev_free_list;
	sys_slist_t dev_rsi_list;
};

struct app_coordinator_rsi {
	/* List header */
	sys_snode_t node;
	/* RSI to resolve */
	atc_csis_rsi_t rsi;
	uint8_t key_lid;
	uint8_t set_lid;
	uint8_t lid;
	uint8_t conidx;
	bool resolved;
	bool connected;
	bool sirk_valid;
	bool rsi_valid;
	gap_bdaddr_t addr;
};

static struct app_coordinator_rsi app_rsi_available[NUMBER_OF_RSI];

K_SEM_DEFINE(csisc_procedure_sem, 0, 1);
static uint16_t csisc_completed_status;
static uint8_t csisc_completed_lid;
static uint8_t csisc_completed_char_type;

static peer_ltk_getter_t peer_ltk_get;
static struct app_csisc_env_t env;

void joystick_press(struct k_work *work)
{
	ARG_UNUSED(work);
	LOG_INF("Toggle csisc lock state for first device in the list!");
	csisc_lock_toggle(0, 0);
}
static K_WORK_DEFINE(joystick_press_work, joystick_press);

void joystick_up(struct k_work *work)
{
	int dev_count;
	struct app_coordinator_rsi *entry;

	ARG_UNUSED(work);
	alif_ble_mutex_lock(K_FOREVER);
	dev_count = sys_slist_len(&env.dev_rsi_list);
	alif_ble_mutex_unlock();

	if (!dev_count) {
		LOG_INF("No devices to disconnect");

		return;
	}

	alif_ble_mutex_lock(K_FOREVER);
	entry = (struct app_coordinator_rsi *)sys_slist_get(&env.dev_rsi_list);
	alif_ble_mutex_unlock();

	while (entry) {
		if (entry->conidx != GAP_INVALID_CONIDX) {
			LOG_INF("Disconnecting peer with conidx %u", entry->conidx);
			app_disconnect(entry->conidx, false);
		}
		/* Release entry */
		alif_ble_mutex_lock(K_FOREVER);
		sys_slist_append(&env.dev_free_list, &entry->node);
		entry->connected = false;
		entry->resolved = false;
		entry->conidx = GAP_INVALID_CONIDX;
		alif_ble_mutex_unlock();

		alif_ble_mutex_lock(K_FOREVER);
		entry = (struct app_coordinator_rsi *)sys_slist_get(&env.dev_rsi_list);
		alif_ble_mutex_unlock();
	}
}

static K_WORK_DEFINE(joystick_up_work, joystick_up);

void joystick_down(struct k_work *work)
{
	ARG_UNUSED(work);
	LOG_INF("Get SIRK of first device in the list!");
	csisc_get_char(0, 0, CSIS_CHAR_TYPE_SIRK);
}
static K_WORK_DEFINE(joystick_down_work, joystick_down);

#if !DT_NODE_EXISTS(BUTTON_NODELABEL)
#error "Button is mandatory!"
#endif

#include <zephyr/drivers/gpio.h>

struct gpio_data {
	const struct gpio_dt_spec spec;
	struct gpio_callback cb_data;
	struct k_work *execute_func;
	uint32_t last_clicked_ms;
};

#define BUTTON_DEBOUNCE_MS 15

static bool check_debounce(uint32_t *const p_last_time_ms)
{
	uint32_t const current_time_ms = k_uptime_get_32();

	if ((current_time_ms - *p_last_time_ms) < BUTTON_DEBOUNCE_MS) {
		return false;
	}

	*p_last_time_ms = current_time_ms;
	return true;
}

static bool is_bdaddr_valid(gap_bdaddr_t const *const p_addr)
{
	return (!!p_addr && p_addr->addr_type <= GAP_ADDR_RAND);
}

static struct app_coordinator_rsi *get_rsi_by_bdaddr(gap_bdaddr_t const *const p_addr)
{
	if (!is_bdaddr_valid(p_addr)) {
		return NULL;
	}

	struct app_coordinator_rsi *p_rsi;
	sys_snode_t *node = NULL;

	SYS_SLIST_ITERATE_FROM_NODE(&env.dev_rsi_list, node)
	{
		p_rsi = (struct app_coordinator_rsi *)node;
		if (!memcmp((p_rsi->addr.addr), p_addr->addr, sizeof(p_addr->addr))) {
			return p_rsi;
		}
	}

	return NULL;
}

static struct app_coordinator_rsi *get_rsi_by_conidx(uint8_t conidx)
{
	struct app_coordinator_rsi *p_rsi;
	sys_snode_t *node = NULL;

	if (conidx == GAP_INVALID_CONIDX) {
		return NULL;
	}

	SYS_SLIST_ITERATE_FROM_NODE(&env.dev_rsi_list, node)
	{
		p_rsi = (struct app_coordinator_rsi *)node;
		if (p_rsi->conidx == conidx) {
			return p_rsi;
		}
	}

	return NULL;
}

static void gpio_isr_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	struct gpio_data *p_data = CONTAINER_OF(cb, struct gpio_data, cb_data);
	gpio_port_value_t value;

	if (gpio_port_get(dev, &value)) {
		LOG_ERR("Failed to read button state");
		return;
	}

	if (!check_debounce(&p_data->last_clicked_ms)) {
		return;
	}

	if (value & pins) {
		/* ignore invalid state */
		return;
	}

	if (!p_data->execute_func) {
		return;
	}

	k_work_submit(p_data->execute_func);
}

static int configure_button(struct gpio_data *p_button)
{
	if (!p_button->spec.port) {
		LOG_ERR("Button is not valid!");
		return -EEXIST;
	}

	/* Configure button */
	if (!gpio_is_ready_dt(&p_button->spec)) {
		LOG_ERR("Button is not ready");
		return -EEXIST;
	}

	if (gpio_pin_configure_dt(&p_button->spec, GPIO_INPUT)) {
		LOG_ERR("Button configure failed");
		return -EIO;
	}

	if (gpio_pin_interrupt_configure_dt(&p_button->spec, GPIO_INT_EDGE_FALLING)) {
		LOG_ERR("button int conf failed");
		return -EIO;
	}

	gpio_init_callback(&p_button->cb_data, gpio_isr_handler, BIT(p_button->spec.pin));
	if (gpio_add_callback(p_button->spec.port, &p_button->cb_data)) {
		LOG_ERR("cb add failed");
		return -EIO;
	}

	return 0;
}

static struct gpio_data joystick_conf[] = {
	{.spec = GPIO_DT_SPEC_GET_OR(BUTTON_NODELABEL, gpios, {0}),
	 .execute_func = &joystick_press_work},
	{.spec = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(button_up), gpios, {0}),
	 .execute_func = &joystick_up_work},
	{.spec = GPIO_DT_SPEC_GET_OR(DT_NODELABEL(button_down), gpios, {0}),
	 .execute_func = &joystick_down_work},
};

/* CSIP callbacks */
void app_csisc_cb_bond_data(uint8_t conidx, uint8_t set_lid, const csisc_csis_info_t *p_csis_info)
{
	struct app_coordinator_rsi *entry = get_rsi_by_conidx(conidx);

	if (!entry) {
		LOG_ERR("No entry found for conidx %u", conidx);
		return;
	}

	env.sets[set_lid].csis_info = *p_csis_info;

	entry->set_lid = set_lid;
	entry->conidx = conidx;

	LOG_DBG("Bond data for set_lid %u, conidx %u", set_lid, conidx);
	LOG_DBG("Service info: start_hdl %u, end_hdl %u", p_csis_info->svc_info.shdl,
		p_csis_info->svc_info.ehdl);
	for (uint8_t i = 0; i < CSIS_CHAR_TYPE_MAX; i++) {
		LOG_DBG("Char %u:property 0x%02X, val_hdl %u", i, p_csis_info->char_info[i].prop,
			p_csis_info->char_info[i].val_hdl);
	}
	for (uint8_t i = 0; i < CSIS_DESC_TYPE_MAX; i++) {
		LOG_DBG("Desc %u:desc_hdl %u", i, p_csis_info->desc_info[i].desc_hdl);
	}
}

void app_csisc_cb_ltk_req(uint8_t con_lid, uint8_t set_lid)
{
	struct app_coordinator_rsi *entry = get_rsi_by_conidx(con_lid);

	if (!entry) {
		LOG_ERR("No entry found for conidx %u", con_lid);
		return;
	}
	entry->set_lid = set_lid;

	LOG_DBG("LTK request for set_lid %u, con_lid %u", set_lid, con_lid);
	/* GET LTK KEY */
	if (!peer_ltk_get) {
		LOG_ERR("peer_ltk_get is NULL");
		return;
	}
	atc_csisc_ltk_cfm(peer_ltk_get(con_lid));
}

void app_csisc_cb_cmp_evt(uint16_t cmd_code, uint16_t status, uint8_t lid, uint8_t set_lid,
			  uint8_t char_type)
{
	switch (cmd_code) {
	case ATC_CSISC_CMD_TYPE_RESOLVE:
		LOG_INF("RSI resolution completed with status %u for set_lid %u, lid %u", status,
			set_lid, lid);
		break;
	case ATC_CSISC_CMD_TYPE_DISCOVER:
		LOG_INF("Discover completed with status %u for set_lid %u, lid %u", status, set_lid,
			lid);
		break;
	case ATC_CSISC_CMD_TYPE_LOCK:
		LOG_INF("Lock command completed with status %u for set_lid %u, lid %u", status,
			set_lid, lid);
		break;
	case ATC_CSISC_CMD_TYPE_GET:
		LOG_INF("Get command completed with status %u for set_lid %u, lid %u, char_type %u",
			status, set_lid, lid, char_type);
		break;
	case ATC_CSISC_CMD_TYPE_SET_CFG:
		LOG_INF("Set configuration command completed with status %u for set_lid %u, lid "
			"%u, char_type %u",
			status, set_lid, lid, char_type);
		break;
	default:
		LOG_INF("Command code %u completed with status %u for set_lid %u, lid %u, "
			"char_type %u",
			cmd_code, status, set_lid, lid, char_type);
		break;
	}

	csisc_completed_status = status;
	csisc_completed_lid = lid;
	csisc_completed_char_type = char_type;
	k_sem_give(&csisc_procedure_sem);
}

void app_set_csisc_cb_sirk(uint8_t conidx, uint8_t set_lid, uint8_t key_lid,
			   const csis_sirk_t *p_sirk)
{
	struct app_coordinator_rsi *entry = get_rsi_by_conidx(conidx);

	if (!entry) {
		LOG_ERR("No entry found for conidx %u", conidx);
		return;
	}

	LOG_DBG("SIRK for set_lid %u, conidx %u, key_lid %u", set_lid, conidx, key_lid);
	LOG_INF("SIRK: "
		"%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
		p_sirk->sirk[0], p_sirk->sirk[1], p_sirk->sirk[2], p_sirk->sirk[3], p_sirk->sirk[4],
		p_sirk->sirk[5], p_sirk->sirk[6], p_sirk->sirk[7], p_sirk->sirk[8], p_sirk->sirk[9],
		p_sirk->sirk[10], p_sirk->sirk[11], p_sirk->sirk[12], p_sirk->sirk[13],
		p_sirk->sirk[14], p_sirk->sirk[15]);

	if (key_lid == GAF_INVALID_LID) {
		if (env.sets[set_lid].key_lid != GAF_INVALID_LID) {
			LOG_DBG("Removing SIRK with key_lid %u", env.sets[set_lid].key_lid);
			atc_csisc_remove_sirk(env.sets[set_lid].key_lid);
			env.sets[set_lid].key_lid = GAF_INVALID_LID;
			env.sets[set_lid].sirk_valid = false;
		}

		atc_csisc_add_sirk(p_sirk, &(env.sets[set_lid].key_lid));
		if (env.sets[set_lid].key_lid == GAF_INVALID_LID) {
			LOG_ERR("Failed to add SIRK");
		} else {
			LOG_INF("Added SIRK with key_lid %u", env.sets[set_lid].key_lid);
			entry->key_lid = env.sets[set_lid].key_lid;
			entry->set_lid = set_lid;

			env.sets[set_lid].set_lid = set_lid;
			env.sets[set_lid].sirk_valid = true;
			env.sets[set_lid].conidx = conidx;
			memcpy(env.sets[set_lid].sirk, p_sirk->sirk, CSIS_SIRK_LEN);
			env.active_sets |= BIT(set_lid);
		}
	}
}

void app_set_csisc_cb_info(uint8_t conidx, uint8_t set_lid, uint8_t char_type, uint8_t val)
{
	LOG_DBG("Info for set_lid %u, conidx %u, char_type %u, val %u", set_lid, conidx, char_type,
		val);
	switch (char_type) {
	case CSIS_CHAR_TYPE_SIZE:
		LOG_INF("Coordinated set size: %u", val);
		env.sets[set_lid].char_type_size = val;
		break;

	case CSIS_CHAR_TYPE_LOCK:
		env.sets[set_lid].char_type_lock = val;
		LOG_INF("Lock state: %s", val == CSIS_LOCK_UNLOCKED ? "UNLOCKED" : "LOCKED");
		break;

	case CSIS_CHAR_TYPE_RANK:
		env.sets[set_lid].char_type_rank = val;
		LOG_INF("Rank: %u", val);
		break;

	default:
		break;
	}
}

void app_set_csisc_cb_svc_changed(uint8_t conidx)
{
}

static atc_csisc_cb_t csisc_cbs = {
	.cb_bond_data = app_csisc_cb_bond_data,
	.cb_sirk = app_set_csisc_cb_sirk,
	.cb_info = app_set_csisc_cb_info,
	.cb_ltk_req = app_csisc_cb_ltk_req,
	.cb_cmp_evt = app_csisc_cb_cmp_evt,
	.cb_svc_changed = app_set_csisc_cb_svc_changed,
};

uint16_t csisc_configure(peer_ltk_getter_t peer_ltk_getter)
{
	uint16_t ret = 0;

	env.active_sets = 0;

	sys_slist_init(&env.dev_free_list);
	sys_slist_init(&env.dev_rsi_list);

	for (size_t iter = 0; iter < NUMBER_OF_SETS; iter++) {
		env.sets[iter].set_lid = iter;
		env.sets[iter].key_lid = GAF_INVALID_LID;
		env.sets[iter].conidx = GAP_INVALID_CONIDX;
		env.sets[iter].sirk_valid = false;
		env.sets[iter].char_type_lock = CSIS_LOCK_UNLOCKED;
	}

	for (size_t iter = 0; iter < NUMBER_OF_RSI; iter++) {
		app_rsi_available[iter].key_lid = GAF_INVALID_LID;
		app_rsi_available[iter].set_lid = 0;
		app_rsi_available[iter].conidx = GAP_INVALID_CONIDX;
		app_rsi_available[iter].resolved = false;
		app_rsi_available[iter].connected = false;
		app_rsi_available[iter].sirk_valid = false;
		app_rsi_available[iter].rsi_valid = false;

		sys_slist_append(&env.dev_free_list, &app_rsi_available[iter].node);
	}

	if (!peer_ltk_getter) {
		LOG_ERR("peer_ltk_getter is NULL");
		return -EINVAL;
	}

	peer_ltk_get = peer_ltk_getter;
	ret = atc_csisc_configure(3, &csisc_cbs);
	if (ret) {
		LOG_ERR("CSISM configuration failed %d", ret);
		return ret;
	}

	for (size_t iter = 0; iter < ARRAY_SIZE(joystick_conf); iter++) {
		ret = configure_button(&joystick_conf[iter]);
		if (ret) {
			return ret;
		}
	}

	return ret;
}

uint16_t csisc_dev_add(gap_bdaddr_t const *const p_addr, atc_csis_rsi_t const *const p_rsi)
{
	struct app_coordinator_rsi *p_dev = get_rsi_by_bdaddr(p_addr);

	if (p_dev) {

		if (p_rsi) {
			memcpy(&p_dev->rsi, p_rsi, sizeof(p_dev->rsi));
			p_dev->rsi_valid = true;
		} else {
			p_dev->rsi_valid = false;
		}

		return 0;
	}

	p_dev = (struct app_coordinator_rsi *)sys_slist_get(&env.dev_free_list);

	if (!p_dev) {
		LOG_ERR("No more free device context available");
		return -ENOMEM;
	}

	memcpy(&p_dev->addr, p_addr, sizeof(p_dev->addr));
	if (p_rsi) {
		memcpy(&p_dev->rsi, p_rsi, sizeof(p_dev->rsi));
		p_dev->rsi_valid = true;
		LOG_INF("Device rsi %02X:%02X:%02X:%02X:%02X:%02X added to CSISC context",
			p_rsi->rsi[0], p_rsi->rsi[1], p_rsi->rsi[2], p_rsi->rsi[3], p_rsi->rsi[4],
			p_rsi->rsi[5]);
	} else {
		p_dev->rsi_valid = false;
	}
	LOG_INF("new device with address %02X:%02X:%02X:%02X:%02X:%02X added to CSISC context",
		p_dev->addr.addr[5], p_dev->addr.addr[4], p_dev->addr.addr[3], p_dev->addr.addr[2],
		p_dev->addr.addr[1], p_dev->addr.addr[0]);
	sys_slist_append(&env.dev_rsi_list, &p_dev->node);
	return 0;
}

static uint16_t csisc_discovery(uint8_t conidx)
{
	uint16_t ret = 0;

	alif_ble_mutex_lock(K_FOREVER);
	ret = atc_csisc_discover(conidx, NUMBER_OF_RSI, GATT_INVALID_HDL, GATT_INVALID_HDL);
	if (ret) {
		LOG_ERR("CSISM discovery failed %d", ret);
		alif_ble_mutex_unlock();
		return ret;
	}
	alif_ble_mutex_unlock();
	k_sem_take(&csisc_procedure_sem, K_FOREVER);
	ret = csisc_completed_status;
	LOG_INF("CSISM discovery completed with status %u", ret);
	return ret;
}

static uint16_t csisc_resolve_rsi(uint8_t *p_rsi)
{
	uint16_t ret = 0;
	csis_rsi_t rsi;

	alif_ble_mutex_lock(K_FOREVER);

	if (env.active_sets == 0) {
		LOG_ERR("SIRK is not valid");
		alif_ble_mutex_unlock();
		return -EINVAL;
	}

	memcpy(rsi.rsi, p_rsi, CSIS_RSI_LEN);

	ret = atc_csisc_resolve(&rsi);
	if (ret) {
		LOG_ERR("CSISM resolve failed %d", ret);
		alif_ble_mutex_unlock();
		return ret;
	}
	alif_ble_mutex_unlock();
	k_sem_take(&csisc_procedure_sem, K_FOREVER);
	ret = csisc_completed_status;
	LOG_INF("CSISM resolve completed with status %u", ret);
	return ret;
}

uint16_t csisc_lock_toggle(uint8_t conidx, uint8_t set_lid)
{
	uint16_t ret = 0;

	alif_ble_mutex_lock(K_FOREVER);

	uint8_t lock_state = (env.sets[set_lid].char_type_lock == CSIS_LOCK_UNLOCKED)
				     ? CSIS_LOCK_LOCKED
				     : CSIS_LOCK_UNLOCKED;

	LOG_INF("CSISM set lock state to %s",
		lock_state == CSIS_LOCK_UNLOCKED ? "UNLOCKED" : "LOCKED");
	ret = atc_csisc_lock(conidx, set_lid, lock_state);
	if (ret) {
		LOG_ERR("CSISM set lock failed %d", ret);
		alif_ble_mutex_unlock();
		return ret;
	}

	alif_ble_mutex_unlock();
	k_sem_take(&csisc_procedure_sem, K_FOREVER);
	alif_ble_mutex_lock(K_FOREVER);
	if (csisc_completed_status == 0) {
		env.sets[set_lid].char_type_lock = lock_state;
	}
	ret = csisc_completed_status;
	alif_ble_mutex_unlock();
	LOG_INF("CSISM set lock completed with status %u", ret);
	return ret;
}

uint16_t csisc_get_char(uint8_t conidx, uint8_t set_lid, uint8_t char_type)
{
	uint16_t ret = 0;

	LOG_INF("CSISM get char type %u", char_type);
	alif_ble_mutex_lock(K_FOREVER);

	ret = atc_csisc_get(conidx, set_lid, char_type);
	if (ret) {
		LOG_ERR("CSISM get char failed %d", ret);
		alif_ble_mutex_unlock();
		return ret;
	}
	alif_ble_mutex_unlock();
	k_sem_take(&csisc_procedure_sem, K_FOREVER);
	ret = csisc_completed_status;
	LOG_INF("CSISM get char completed with status %u", ret);
	return ret;
}

void csisc_process(void)
{
	int ret;
	uint16_t response;
	int dev_count;

	alif_ble_mutex_lock(K_FOREVER);
	dev_count = sys_slist_len(&env.dev_rsi_list);
	alif_ble_mutex_unlock();

	if (!dev_count) {
		return;
	}

	struct app_coordinator_rsi *entry, *tmp;
	sys_snode_t *prev_node = NULL;

	SYS_SLIST_FOR_EACH_CONTAINER_SAFE(&env.dev_rsi_list, entry, tmp, node) {
		if (entry->conidx == GAP_INVALID_CONIDX) {

			if (env.active_sets && entry->rsi_valid) {
				LOG_INF("Attempting to resolve RSI for device with address "
					"%02X:%02X:%02X:%02X:%02X:%02X",
					entry->addr.addr[5], entry->addr.addr[4],
					entry->addr.addr[3], entry->addr.addr[2],
					entry->addr.addr[1], entry->addr.addr[0]);
				response = csisc_resolve_rsi(entry->rsi.rsi);
				if (response) {
					LOG_ERR("CSIS RSI resolve failed for device with address "
						"%02X:%02X:%02X:%02X:%02X:%02X",
						entry->addr.addr[5], entry->addr.addr[4],
						entry->addr.addr[3], entry->addr.addr[2],
						entry->addr.addr[1], entry->addr.addr[0]);
					sys_slist_remove(&env.dev_rsi_list, prev_node,
							 &entry->node);
					sys_slist_append(&env.dev_free_list, &entry->node);
					continue;
				}
				entry->resolved = true;
			}
			ret = app_connect_device(&entry->addr, &entry->conidx);

			if (ret) {
				LOG_ERR("Failed to connect to device with address "
					"%02X:%02X:%02X:%02X:%02X:%02X",
					entry->addr.addr[5], entry->addr.addr[4],
					entry->addr.addr[3], entry->addr.addr[2],
					entry->addr.addr[1], entry->addr.addr[0]);
				sys_slist_remove(&env.dev_rsi_list, prev_node, &entry->node);
				sys_slist_append(&env.dev_free_list, &entry->node);
				continue;
			}
			LOG_INF("Connected to device with address %02X:%02X:%02X:%02X:%02X:%02X",
				entry->addr.addr[5], entry->addr.addr[4], entry->addr.addr[3],
				entry->addr.addr[2], entry->addr.addr[1], entry->addr.addr[0]);
			entry->connected = true;

			response = csisc_discovery(entry->conidx);
			if (response) {
				LOG_ERR("CSIS discovery failed for device with address "
					"%02X:%02X:%02X:%02X:%02X:%02X",
					entry->addr.addr[5], entry->addr.addr[4],
					entry->addr.addr[3], entry->addr.addr[2],
					entry->addr.addr[1], entry->addr.addr[0]);
				response = app_disconnect(entry->conidx, true);
				LOG_INF("Disconnected status %u", response);
				if (response) {
					LOG_ERR("Failed to disconnect from device with address "
						"%02X:%02X:%02X:%02X:%02X:%02X",
						entry->addr.addr[5], entry->addr.addr[4],
						entry->addr.addr[3], entry->addr.addr[2],
						entry->addr.addr[1], entry->addr.addr[0]);
					sys_slist_remove(&env.dev_rsi_list, prev_node,
							 &entry->node);
					sys_slist_append(&env.dev_free_list, &entry->node);
				}

				continue;
			}
			entry->lid = csisc_completed_lid;

			if (!entry->resolved && env.active_sets && entry->rsi_valid) {
				response = csisc_resolve_rsi(entry->rsi.rsi);
				if (response) {
					LOG_ERR("CSIS RSI resolve failed for device with address "
						"%02X:%02X:%02X:%02X:%02X:%02X",
						entry->addr.addr[5], entry->addr.addr[4],
						entry->addr.addr[3], entry->addr.addr[2],
						entry->addr.addr[1], entry->addr.addr[0]);
					response = app_disconnect(entry->conidx, true);
					LOG_INF("Disconnected status %u", response);
					if (response) {
						LOG_ERR("Failed to disconnect from device with "
							"address "
							"%02X:%02X:%02X:%02X:%02X:%02X",
							entry->addr.addr[5], entry->addr.addr[4],
							entry->addr.addr[3], entry->addr.addr[2],
							entry->addr.addr[1], entry->addr.addr[0]);
						sys_slist_remove(&env.dev_rsi_list, prev_node,
								 &entry->node);
						sys_slist_append(&env.dev_free_list, &entry->node);
					}
					continue;
				}
				entry->resolved = true;
			}
		}
		prev_node = &entry->node;
	}
}

uint16_t csisc_dev_disconnected(uint8_t conidx)
{

	struct app_coordinator_rsi *p_dev = get_rsi_by_conidx(conidx);

	if (!p_dev) {
		return -EINVAL;
	}
	LOG_INF("Device with conidx %u disconnected", conidx);
	p_dev->conidx = GAP_INVALID_CONIDX;
	p_dev->connected = false;
	p_dev->resolved = false;
	sys_slist_find_and_remove(&env.dev_rsi_list, &p_dev->node);
	sys_slist_append(&env.dev_free_list, &p_dev->node);
	return 0;
}
