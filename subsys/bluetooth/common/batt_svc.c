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
#include "prf.h"
#include "bass.h"
#include "bas.h"
#include "batt_svc.h"
#include "shared_control.h"

static struct shared_control *s_shared_ptr = NULL;

LOG_MODULE_REGISTER(alif_batt, LOG_LEVEL_DBG);

#define BATT_INSTANCE 0x00

static uint8_t battery_level = 99;
static uint16_t battery_power_state;
static uint8_t battery_critical_status;
static uint8_t battery_health_info[BAS_HEALTH_INFO_SIZE_MAX];
static uint8_t battery_info[BAS_INFO_SIZE_MAX];
static uint8_t battery_energy_status[BAS_ENERGY_STATUS_SIZE_MAX];
static uint8_t battery_time_status[BAS_TIME_STATUS_SIZE_MAX];
static uint8_t battery_health_status[BAS_HEALTH_STATUS_SIZE_MAX];
static uint8_t battery_service_date[3]; /* Day, Month, Year */
static const char *manufacturer_name = "Alif Semiconductor";
static const char *model_number = "E7-DevKit";
static const char *serial_number = "SN-2026-001";

/* Notifications bit field */
uint16_t ccc_bf;

__STATIC co_buf_t *battery_server_prepare_buf_level(void)
{
	co_buf_t *p_buf;

	prf_buf_alloc(&p_buf, BAS_LEVEL_SIZE_MAX);
	*co_buf_data(p_buf) = battery_level;

	return p_buf;
}

__STATIC co_buf_t *battery_server_prepare_buf_level_status(void)
{
	co_buf_t *p_buf;
	uint8_t flags = 0;

	/* Enable Level and Additional Status fields */
	flags |= BAS_LEVEL_STATUS_FLAGS_LEVEL_PRESENT_BIT;
	flags |= BAS_LEVEL_STATUS_FLAGS_ADD_STATUS_PRESENT_BIT;

	/* Allocate buffer: Flags(1) + Power State(2) + Level(1) + Add Status(1) = 5 bytes */
	prf_buf_alloc(&p_buf, 5);

	/* Fill flags */
	co_buf_data(p_buf)[0] = flags;

	/* Fill power state (2 bytes) */
	co_write16(&co_buf_data(p_buf)[1], co_htole16(battery_power_state));

	/* Fill battery level */
	co_buf_data(p_buf)[3] = battery_level;

	/* Fill additional status (no service required, no fault) */
	co_buf_data(p_buf)[4] = 0;

	return p_buf;
}

__STATIC co_buf_t *battery_server_prepare_buf_critical_status(void)
{
	co_buf_t *p_buf;

	prf_buf_alloc(&p_buf, BAS_CRITICAL_STATUS_SIZE_MAX);
	*co_buf_data(p_buf) = battery_critical_status;

	return p_buf;
}

__STATIC co_buf_t *battery_server_prepare_buf_health_info(void)
{
	co_buf_t *p_buf;
	uint8_t flags = 0;
	uint8_t size = BAS_HEALTH_INFO_SIZE_FLAGS;

	/* Enable cycle count and temperature fields */
	flags |= BAS_HEALTH_INFO_FLAGS_CYCLE_COUNT_DESIGNED_LIFETIME_PRESENT_BIT;
	flags |= BAS_HEALTH_INFO_FLAGS_MIN_MAX_TEMPERATURE_PRESENT_BIT;

	/* Calculate total size */
	size += BAS_HEALTH_INFO_SIZE_CYCLE_COUNT_DESIGNED_LIFETIME;
	size += BAS_HEALTH_INFO_SIZE_MIN_TEMPERATURE;
	size += BAS_HEALTH_INFO_SIZE_MAX_TEMPERATURE;

	/* Fill the buffer: Flags(1) + Cycle Count(2) + Min Temp(1) + Max Temp(1) = 5 bytes */
	battery_health_info[0] = flags;
	/* Cycle count designed lifetime (example: 500 cycles) */
	co_write16(&battery_health_info[1], co_htole16(500));
	/* Minimum temperature (-10°C) */
	battery_health_info[3] = (uint8_t)(-10);
	/* Maximum temperature (60°C) */
	battery_health_info[4] = 60;

	prf_buf_alloc(&p_buf, size);
	memcpy(co_buf_data(p_buf), battery_health_info, size);

	return p_buf;
}

__STATIC co_buf_t *battery_server_prepare_buf_info(void)
{
	co_buf_t *p_buf;
	uint16_t flags = 0;
	uint8_t features = 0;
	uint8_t size = BAS_INFO_SIZE_FLAGS + BAS_INFO_SIZE_FEATURES;
	uint8_t offset = 0;

	/* Set features: Rechargeable, non-replaceable */
	features |= BAS_FEATURES_RECHARGEABLE_BIT;

	/* Enable optional fields */
	flags |= BAS_INFO_FLAGS_DESIGNED_CAPACITY_PRESENT_BIT;
	flags |= BAS_INFO_FLAGS_CHEMISTRY_PRESENT_BIT;
	flags |= BAS_INFO_FLAGS_NOMINAL_VOLTAGE_PRESENT_BIT;
	flags |= BAS_INFO_FLAGS_LOW_ENERGY_PRESENT_BIT;
	flags |= BAS_INFO_FLAGS_CRITICAL_ENERGY_PRESENT_BIT;

	/* Calculate total size */
	size += BAS_INFO_SIZE_DESIGNED_CAPACITY;
	size += BAS_INFO_SIZE_CHEMISTRY;
	size += BAS_INFO_SIZE_NOMINAL_VOLTAGE;
	size += BAS_INFO_SIZE_LOW_ENERGY;
	size += BAS_INFO_SIZE_CRITICAL_ENERGY;

	/* Fill the buffer */
	/* Flags (2 bytes) */
	co_write16(&battery_info[offset], co_htole16(flags));
	offset += BAS_INFO_SIZE_FLAGS;

	/* Features (1 byte) */
	battery_info[offset] = features;
	offset += BAS_INFO_SIZE_FEATURES;

	/* Designed capacity (2 bytes) - 3000 mAh */
	co_write16(&battery_info[offset], co_htole16(3000));
	offset += BAS_INFO_SIZE_DESIGNED_CAPACITY;

	/* Low energy threshold (2 bytes) - 600 mAh (20%) */
	co_write16(&battery_info[offset], co_htole16(600));
	offset += BAS_INFO_SIZE_LOW_ENERGY;

	/* Critical energy threshold (2 bytes) - 150 mAh (5%) */
	co_write16(&battery_info[offset], co_htole16(150));
	offset += BAS_INFO_SIZE_CRITICAL_ENERGY;

	/* Chemistry (1 byte) - Lithium Ion */
	battery_info[offset] = BAS_CHEMISTRY_LITHIUM_ION_LI;
	offset += BAS_INFO_SIZE_CHEMISTRY;

	/* Nominal voltage (2 bytes) - 3700 mV (3.7V) */
	co_write16(&battery_info[offset], co_htole16(3700));
	offset += BAS_INFO_SIZE_NOMINAL_VOLTAGE;

	prf_buf_alloc(&p_buf, size);
	memcpy(co_buf_data(p_buf), battery_info, size);

	return p_buf;
}

__STATIC co_buf_t *battery_server_prepare_buf_energy_status(void)
{
	co_buf_t *p_buf;
	uint8_t flags = 0;
	uint8_t size = BAS_ENERGY_STATUS_SIZE_FLAGS;
	uint8_t offset = 0;

	/* Enable present voltage and available capacity */
	flags |= BAS_ENERGY_STATUS_FLAGS_PRESENT_VOLTAGE_PRESENT_BIT;
	flags |= BAS_ENERGY_STATUS_FLAGS_AVAILABLE_ENERGY_CAPA_PRESENT_BIT;

	size += BAS_ENERGY_STATUS_SIZE_PRESENT_VOLTAGE;
	size += BAS_ENERGY_STATUS_SIZE_AVAILABLE_CAPACITY;

	/* Flags */
	battery_energy_status[offset++] = flags;

	/* Present voltage (3700 mV = 3.7V) */
	co_write16(&battery_energy_status[offset], co_htole16(3700));
	offset += BAS_ENERGY_STATUS_SIZE_PRESENT_VOLTAGE;

	/* Available capacity based on battery level (3000 mAh * level / 100) */
	uint16_t capacity = (3000 * battery_level) / 100;

	co_write16(&battery_energy_status[offset], co_htole16(capacity));
	offset += BAS_ENERGY_STATUS_SIZE_AVAILABLE_CAPACITY;

	prf_buf_alloc(&p_buf, size);
	memcpy(co_buf_data(p_buf), battery_energy_status, size);

	return p_buf;
}

__STATIC co_buf_t *battery_server_prepare_buf_time_status(void)
{
	co_buf_t *p_buf;
	uint8_t flags = 0;
	uint8_t size = BAS_TIME_STATUS_SIZE_FLAGS + BAS_TIME_STATUS_TIME_UNTIL_DISCHARGED;
	uint8_t offset = 0;
	uint32_t time_remaining;

	/* Enable time until recharged if battery low */
	if (battery_level < 100) {
		flags |= BAS_TIME_STATUS_FLAGS_TIME_UNTIL_RECHARGED_PRESENT_BIT;
		size += BAS_TIME_STATUS_TIME_UNTIL_RECHARGED;
	}

	/* Flags */
	battery_time_status[offset++] = flags;

	/* Time until discharged in minutes (estimate: level * 2 minutes) */
	time_remaining = battery_level * 2;
	co_write24(&battery_time_status[offset], time_remaining);
	offset += BAS_TIME_STATUS_TIME_UNTIL_DISCHARGED;

	/* Time until recharged in minutes (if low) */
	if (battery_level < 100) {
		uint32_t charge_time = (100 - battery_level) * 1; /* 1 min per percent */

		co_write24(&battery_time_status[offset], charge_time);
		offset += BAS_TIME_STATUS_TIME_UNTIL_RECHARGED;
	}

	prf_buf_alloc(&p_buf, size);
	memcpy(co_buf_data(p_buf), battery_time_status, size);

	return p_buf;
}

__STATIC co_buf_t *battery_server_prepare_buf_service_date(void)
{
	co_buf_t *p_buf;

	/* Set estimated service date: Day=15, Month=12, Year=2026 (26 from 2000) */
	battery_service_date[0] = 15;  /* Day */
	battery_service_date[1] = 12;  /* Month */
	battery_service_date[2] = 26;  /* Year offset from 2000 */

	prf_buf_alloc(&p_buf, 3);
	memcpy(co_buf_data(p_buf), battery_service_date, 3);

	return p_buf;
}

__STATIC co_buf_t *battery_server_prepare_buf_health_status(void)
{
	co_buf_t *p_buf;
	uint8_t flags = 0;
	uint8_t size = BAS_HEALTH_STATUS_SIZE_FLAGS;
	uint8_t offset = 0;

	/* Enable all fields */
	flags |= BAS_HEALTH_STATUS_FLAGS_SUMMARY_PRESENT_BIT;
	flags |= BAS_HEALTH_STATUS_FLAGS_CYCLE_COUNT_PRESENT_BIT;
	flags |= BAS_HEALTH_STATUS_FLAGS_CURRENT_TEMPERATURE_PRESENT_BIT;

	size += BAS_HEALTH_STATUS_SIZE_SUMMARY;
	size += BAS_HEALTH_STATUS_SIZE_CYCLE_COUNT;
	size += BAS_HEALTH_STATUS_SIZE_CURRENT_TEMPERATURE;

	/* Flags */
	battery_health_status[offset++] = flags;

	/* Health summary (0=Unknown, 1=Good, 2=Fair, 3=Poor, 4=Replace) */
	battery_health_status[offset++] = 1; /* Good */

	/* Cycle count (example: 150 cycles) */
	co_write16(&battery_health_status[offset], co_htole16(150));
	offset += BAS_HEALTH_STATUS_SIZE_CYCLE_COUNT;

	/* Current temperature (25°C) */
	battery_health_status[offset++] = 25;

	prf_buf_alloc(&p_buf, size);
	memcpy(co_buf_data(p_buf), battery_health_status, size);

	return p_buf;
}

__STATIC co_buf_t *battery_server_prepare_buf_string(const char *str)
{
	co_buf_t *p_buf;
	uint16_t len = strlen(str);

	prf_buf_alloc(&p_buf, len);
	memcpy(co_buf_data(p_buf), str, len);

	return p_buf;
}

static void on_value_req(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, uint16_t token)
{
	co_buf_t *p_buf = NULL;

	switch (char_type) {
	case BASS_CHAR_TYPE_LEVEL:
		p_buf = battery_server_prepare_buf_level();
		break;
	case BASS_CHAR_TYPE_LEVEL_STATUS:
		p_buf = battery_server_prepare_buf_level_status();
		break;
	case BASS_CHAR_TYPE_CRITICAL_STATUS:
		p_buf = battery_server_prepare_buf_critical_status();
		break;
	case BASS_CHAR_TYPE_HEALTH_INFO:
		p_buf = battery_server_prepare_buf_health_info();
		break;
	case BASS_CHAR_TYPE_INFO:
		p_buf = battery_server_prepare_buf_info();
		break;
	case BASS_CHAR_TYPE_ENERGY_STATUS:
		p_buf = battery_server_prepare_buf_energy_status();
		break;
	case BASS_CHAR_TYPE_TIME_STATUS:
		p_buf = battery_server_prepare_buf_time_status();
		break;
	case BASS_CHAR_TYPE_ESTIMATED_SERVICE_DATE:
		p_buf = battery_server_prepare_buf_service_date();
		break;
	case BASS_CHAR_TYPE_HEALTH_STATUS:
		p_buf = battery_server_prepare_buf_health_status();
		break;
	case BASS_CHAR_TYPE_MANUFACTURER_NAME:
		p_buf = battery_server_prepare_buf_string(manufacturer_name);
		break;
	case BASS_CHAR_TYPE_MODEL_NUMBER:
		p_buf = battery_server_prepare_buf_string(model_number);
		break;
	case BASS_CHAR_TYPE_SERIAL_NUMBER:
		p_buf = battery_server_prepare_buf_string(serial_number);
		break;
	default:
		LOG_WRN("REQUEST NOT SUPPORTED: 0x%02x", char_type);
		break;
	}

	if (p_buf != NULL) {
		bass_value_cfm(conidx, token, p_buf);
		co_buf_release(p_buf);
	}
}

static void on_get_cccd_req(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, uint16_t token)
{
	co_buf_t *p_buf;
	uint16_t value = ((ccc_bf & CO_BIT(char_type)) != 0u)
				? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

	prf_buf_alloc(&p_buf, PRF_CCC_DESC_LEN);
	co_write16(co_buf_data(p_buf), co_htole16(value));
	bass_value_cfm(conidx, token, p_buf);
	co_buf_release(p_buf);

	LOG_INF("Get CCCD request for 0x%02x characteristic",
		char_type);
}

static void on_set_cccd_req(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, uint16_t token,
			co_buf_t *p_buf)
{
	LOG_INF("Set char_type: 0x%02x", char_type);

	uint16_t value = co_letoh16(co_read16(co_buf_data(p_buf)));

	if (value != PRF_CLI_STOP_NTFIND) {
		LOG_INF("Enable CCCD request for %02x characteristic (0x%04X)", char_type, value);

		ccc_bf |= CO_BIT(char_type);
	} else {
		LOG_INF("Disable CCCD request for %02x characteristic", char_type);
		ccc_bf &= ~CO_BIT(char_type);
	}

	bass_set_cccd_cfm(conidx, GAP_ERR_NO_ERROR, token);
}

static void on_sent(uint8_t conidx, uint8_t instance_idx, uint8_t char_type, uint16_t status)
{
	if (status) {
		LOG_WRN("Value sent with status: 0x%02x", status);
	}
}

static const bass_cbs_t bass_cb = {
	.cb_value_req = on_value_req,
	.cb_get_cccd_req = on_get_cccd_req,
	.cb_set_cccd_req = on_set_cccd_req,
	.cb_sent = on_sent,
};

uint16_t get_batt_id(void)
{
	return GATT_SVC_BATTERY;
}

uint16_t config_battery_service(void)
{
	uint16_t err;
	uint16_t start_hdl = 0;
	uint16_t bass_cfg_bf = 0;

	/* Enable Indication support for characteristics (in addition to Notification) */
	bass_cfg_bf |= BASS_CONFIG_CRITICAL_BIT;

	/* Enable all Battery Service characteristics except Presentation Format */
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_LEVEL_STATUS);
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_CRITICAL_STATUS);
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_ENERGY_STATUS);
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_TIME_STATUS);
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_ESTIMATED_SERVICE_DATE);
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_HEALTH_STATUS);
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_HEALTH_INFO);
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_INFO);
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_MANUFACTURER_NAME);
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_MODEL_NUMBER);
	bass_cfg_bf |= (1 << BASS_CHAR_TYPE_SERIAL_NUMBER);

	err = prf_add_profile(TASK_ID_BASS, 0, 0, &bass_cfg_bf, &bass_cb, &start_hdl);

	if (err) {
		LOG_ERR("Error adding service: 0x%02x", err);
	}

	return err;
}

void battery_process(void)
{
	uint16_t err;

	/* Execute dummy measurement */	
	if (battery_level <= 1) {
		battery_level = 99;
	} else {
		battery_level--;
	}

	/* Update power state: Battery present, discharging, level based on percentage */
	battery_power_state = BAS_POWER_STATE_BATTERY_PRESENT_BIT;
	battery_power_state |= (BAS_BATTERY_CHARGE_STATE_DISCHARGING_ACTIVE
							<< BAS_POWER_STATE_BATTERY_CHARGE_LSB);

	if (battery_level > 20) {
		battery_power_state |= (BAS_BATTERY_CHARGE_LEVEL_GOOD
						<< BAS_POWER_STATE_BATTERY_CHARGE_LEVEL_LSB);
		battery_critical_status = 0; /* Not critical */
	} else if (battery_level > 5) {
		battery_power_state |= (BAS_BATTERY_CHARGE_LEVEL_LOW
						<< BAS_POWER_STATE_BATTERY_CHARGE_LEVEL_LSB);
		battery_critical_status = 0;
	} else {
		battery_power_state |= (BAS_BATTERY_CHARGE_LEVEL_CRITICAL
						<< BAS_POWER_STATE_BATTERY_CHARGE_LEVEL_LSB);
		battery_critical_status = BAS_CRITICAL_STATUS_CRITICAL_POWER_STATE_BIT;
	}

	/* Check if connection is available */
	if (!s_shared_ptr->connected) {
		ccc_bf = 0;
		return;
	}

	/* Proceed to send measurements */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_LEVEL)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_NOTIFY;
		p_buf = battery_server_prepare_buf_level();

		/* Sending dummy battery level to first battery instance*/
		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_LEVEL,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending battery level", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Battery Level Status notification if enabled */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_LEVEL_STATUS)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_NOTIFY;
		p_buf = battery_server_prepare_buf_level_status();

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_LEVEL_STATUS,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending battery level status", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Battery Critical Status indication if enabled and critical */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_CRITICAL_STATUS)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_INDICATE; /* Use indication for critical status */
		p_buf = battery_server_prepare_buf_critical_status();

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_CRITICAL_STATUS,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending battery critical status", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Battery Health Information indication if enabled */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_HEALTH_INFO)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_INDICATE;
		p_buf = battery_server_prepare_buf_health_info();

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_HEALTH_INFO,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending battery health info", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Battery Information indication if enabled */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_INFO)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_INDICATE;
		p_buf = battery_server_prepare_buf_info();

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_INFO,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending battery info", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Battery Energy Status notification if enabled */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_ENERGY_STATUS)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_NOTIFY;
		p_buf = battery_server_prepare_buf_energy_status();

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_ENERGY_STATUS,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending battery energy status", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Battery Time Status notification if enabled */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_TIME_STATUS)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_NOTIFY;
		p_buf = battery_server_prepare_buf_time_status();

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_TIME_STATUS,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending battery time status", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Battery Health Status indication if enabled */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_HEALTH_STATUS)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_INDICATE;
		p_buf = battery_server_prepare_buf_health_status();

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_HEALTH_STATUS,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending battery health status", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Estimated Service Date indication if enabled */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_ESTIMATED_SERVICE_DATE)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_INDICATE;
		p_buf = battery_server_prepare_buf_service_date();

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_ESTIMATED_SERVICE_DATE,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending service date", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Manufacturer Name indication if enabled */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_MANUFACTURER_NAME)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_INDICATE;
		p_buf = battery_server_prepare_buf_string(manufacturer_name);

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_MANUFACTURER_NAME,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending manufacturer name", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Model Number indication if enabled */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_MODEL_NUMBER)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_INDICATE;
		p_buf = battery_server_prepare_buf_string(model_number);

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_MODEL_NUMBER,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending model number", err);
		}

		co_buf_release(p_buf);
	}

	/* Send Serial Number indication if enabled */
	if ((ccc_bf & CO_BIT(BASS_CHAR_TYPE_SERIAL_NUMBER)) != 0u) {
		co_buf_t *p_buf;
		uint8_t evt_type;

		evt_type = GATT_INDICATE;
		p_buf = battery_server_prepare_buf_string(serial_number);

		err = bass_update_value(0, BATT_INSTANCE, BASS_CHAR_TYPE_SERIAL_NUMBER,
								evt_type, p_buf);

		if (err) {
			LOG_ERR("Error %u sending serial number", err);
		}

		co_buf_release(p_buf);
	}
}

void service_conn(struct shared_control *ctrl)
{
	s_shared_ptr = ctrl;
}

