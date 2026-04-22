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
#include "prf_types.h"
#include "cgmss.h"
#include "shared_control.h"
#include "co_time.h"

LOG_MODULE_REGISTER(cgms, LOG_LEVEL_DBG);

K_SEM_DEFINE(conn_sem, 0, 1);

#define CGMS_RUN_TIME_HOURS  (5)
#define CGMS_CGM_TYPE        CGMS_TYPE_CAPILLARY_WHOLE_BLOOD
#define CGMS_SAMPLE_LOCATION CGMS_SAMPLE_LOCATION_FINGER
#define CGMS_MIN_INTERVAL    1

typedef struct {
	prf_date_time_t date_time;
	int8_t time_zone;
	uint8_t dst_offset;
} app_start_time_t;

typedef struct {
	uint8_t cccd_state_bf;
} app_bond_data_t;

typedef struct {
	uint16_t time_offset;
	uint16_t glucose;
} cgms_record_t;

typedef struct {
	app_bond_data_t bond_data;
	app_start_time_t start_time;
	bool ready_to_send;
	bool sensor_annotations;
	bool racp_report_needed;
	uint16_t filter_time_offset;
	uint8_t communication_interval;
} app_env_t;

/* Dummy starting values for glucose and time offset */
uint16_t glucose = 0x00AA;
uint16_t time_offset_minutes = 0x00BB;

/* Dummy record store */
#define DEFAULT_REPORT_COUNT 3

#define DEFAULT_REPORT_MAX_COUNT 25000

static uint16_t record_read_index;
static uint16_t record_stored_index = DEFAULT_REPORT_COUNT;

static cgms_record_t record_store[DEFAULT_REPORT_MAX_COUNT];

/* BLE definitions */
#define local_sec_level GAP_SEC1_AUTH_PAIR_ENC

struct shared_control *s_shared_ptr;

const char cgms_char_name[CGMS_CHAR_TYPE_MAX][31] = {
	"CGM Measurement",
	"CGM Feature",
	"Record Access Control Point",
	"CGM Specific Ops Control Point",
	"CGM Status",
	"CGM Session Start Time",
	"CGM Session Run Time",
};

typedef struct {
	uint8_t op_code;
	uint8_t operator;
	uint16_t operand;
} racp_resp_t;

typedef struct {
	uint8_t op_code;
	uint8_t op_code_req;
	uint8_t operand;
} socp_resp_t;

static app_env_t app_env;

static void racp_response_opcode_pack(racp_resp_t *racp_response, uint8_t opcode, uint8_t status)
{
	racp_response->op_code = CGMS_RA_OPCODE_RSP_CODE;
	racp_response->operator = CGMS_RA_OPERATOR_NULL;
	racp_response->operand = opcode | (status << 8);
}

static void socp_response_opcode_pack(socp_resp_t *socp_response, uint8_t opcode, uint8_t operand)
{
	if (opcode == CGMS_SPECIFIC_OPCODE_GET_INTERVAL) {
		socp_response->op_code = CGMS_SPECIFIC_OPCODE_INTERVAL_RSP;
	} else {
		socp_response->op_code = CGMS_SPECIFIC_OPCODE_RSP;
	}
	socp_response->op_code_req = opcode;
	socp_response->operand = operand;
}

static void send_racp_resp(uint8_t conidx, racp_resp_t *p_response)
{
	co_buf_t *p_buf;
	uint8_t *p_data;
	uint8_t length = 4;

	cgms_buf_alloc(&p_buf, length);
	p_data = co_buf_data(p_buf);

	*p_data++ = p_response->op_code;
	*p_data++ = p_response->operator;

	co_write16(p_data, co_htole16(p_response->operand));
	LOG_INF("Send RACP response opcode %u operator %u operand %u", p_response->op_code,
		p_response->operator, p_response->operand);
	cgmss_send_control_response(conidx, CGMS_CHAR_TYPE_RACP, p_buf);

	co_buf_release(p_buf);
}

static void send_socp_resp(uint8_t conidx, socp_resp_t *p_response)
{
	co_buf_t *p_buf;
	uint8_t *p_data;
	uint16_t len = 2;

	if (p_response->op_code == CGMS_SPECIFIC_OPCODE_RSP) {
		/* Need 1 byte more for Requested Opcode */
		len += 1;
	}

	cgms_buf_alloc(&p_buf, len);
	p_data = co_buf_data(p_buf);

	*p_data++ = p_response->op_code;
	if (p_response->op_code == CGMS_SPECIFIC_OPCODE_RSP) {
		/* Requested Operation code */
		*p_data++ = p_response->op_code_req;
	}
	*p_data++ = p_response->operand;
	LOG_INF("Send SOCP response opcode %u operand %u", p_response->op_code,
		p_response->operand);
	cgmss_send_control_response(conidx, CGMS_CHAR_TYPE_SOCP, p_buf);
	co_buf_release(p_buf);
}

static void send_measurement(uint16_t timeoffset, uint16_t glucose_value)
{
	uint16_t err;

	co_buf_t *p_buf;
	uint8_t *p_data;
	uint16_t length = CGMS_MEASUREMENT_MIN_LEN;
	uint8_t flags = 0u;

	if (app_env.sensor_annotations) {
		length += 1;
		flags = CO_BIT(7); /* Sensor Annotation field present */
	}

	cgms_buf_alloc(&p_buf, length);
	p_data = co_buf_data(p_buf);

	/* Size field */
	*p_data++ = length;
	/* Flags field */
	*p_data++ = flags;
	/* CGM Glucose Concentration field */
	co_write16(p_data, co_htole16(glucose_value));
	p_data += 2;
	/* Time Offset field */
	co_write16(p_data, co_htole16(timeoffset));
	p_data += 2;
	if (app_env.sensor_annotations) {
		/* Sensor Status Annunciation field */
		*p_data++ = CO_BIT(3); /* Sensor result higher than the Hyper level Field */
	}
	err = cgmss_send_measurement(0, p_buf);
	co_buf_release(p_buf);

	if (err) {
		LOG_ERR("Error %u sending measurement", err);
	}
}

static void init_dummy_stored_records(void)
{
	uint16_t time_offset_val = 0x00;
	uint16_t glucose_val = 0x00AA;

	for (uint16_t i = 0; i < DEFAULT_REPORT_MAX_COUNT; i++) {
		record_store[i].time_offset = time_offset_val;
		record_store[i].glucose = glucose_val;
		glucose_val = (glucose_val >= 0x00DD ? 0x00A9 : glucose_val) + 1;
		time_offset_val += 1;
	}
}

static uint16_t number_of_report_stored(void)
{

	if (app_env.filter_time_offset == 0) {
		return record_stored_index;
	}
	uint16_t report_count = 0;

	for (uint16_t i = 0; i < record_stored_index; i++) {
		if (record_store[i].time_offset >= app_env.filter_time_offset) {
			report_count++;
		}
	}
	return report_count;
}

static cgms_record_t *read_entry_report_stored(void)
{
	for (uint16_t i = record_read_index; i < record_stored_index; i++) {
		if (record_store[i].time_offset >= app_env.filter_time_offset) {
			record_read_index = i + 1;
			return &record_store[i];
		}
	}

	return NULL;
}

void cgms_record_store_size_update(void)
{
	if (record_stored_index == DEFAULT_REPORT_COUNT) {
		record_stored_index = DEFAULT_REPORT_MAX_COUNT;
	} else {
		record_stored_index = DEFAULT_REPORT_COUNT;
	}
	LOG_INF("Number of stored records updated, new count: %u", record_stored_index);
}

static void racp_resp_number_of_records_pack(racp_resp_t *racp_response)
{
	racp_response->op_code = CGMS_RA_OPCODE_REPORT_NUMBER_RSP;
	racp_response->operator = CGMS_RA_OPERATOR_NULL;
	racp_response->operand = number_of_report_stored();
}

static void sensor_value_update(void)
{
	/* for the sample application, glucose value is updated every time this function is called
	 */
	glucose = (glucose >= 0x00DD ? 0x00A9 : glucose) + 1;
	time_offset_minutes = (time_offset_minutes >= 0x00EE ? 0x00BA : time_offset_minutes) + 1;
}

/*
 * CGMS callbacks
 */
static void on_set_session_start_time_req(uint8_t conidx, uint16_t token, co_buf_t *p_buf)
{
	/* for the sample application, a session is started during startup */
	uint16_t status = GAP_ERR_NO_ERROR;
	uint8_t *p_data = co_buf_data(p_buf);
	uint16_t length = co_buf_data_len(p_buf);
	app_start_time_t start_time;

	if (length < CGMS_SESSION_START_TIME_LEN) {
		status = PRF_ERR_INVALID_PARAM;
		cgmss_set_value_cfm(conidx, status, token);
		return;
	}
	start_time.date_time.year = co_letoh16(co_read16(p_data));
	p_data += 2u;
	start_time.date_time.month = *p_data++;
	start_time.date_time.day = *p_data++;
	start_time.date_time.hour = *p_data++;
	start_time.date_time.min = *p_data++;
	start_time.date_time.sec = *p_data++;
	start_time.time_zone = *p_data++;
	start_time.dst_offset = *p_data;

	if (start_time.time_zone < PRF_TIME_ZONE_MIN || start_time.time_zone > PRF_TIME_ZONE_MAX) {
		status = PRF_OUT_OF_RANGE;
	} else if (start_time.dst_offset > PRF_DST_OFFSET_DOUBLE_DAY) {
		status = PRF_OUT_OF_RANGE;
	} else {
		app_env.start_time = start_time;
	}

	LOG_DBG("Sample application continuously running a session");
	cgmss_set_value_cfm(conidx, status, token);
}

static void on_value_req(uint8_t conidx, uint8_t char_type, uint16_t token)
{
	co_buf_t *p_buf;
	uint8_t *p_data;
	uint16_t length;

	switch (char_type) {
	case CGMS_CHAR_TYPE_FEATURE:
		length = CGMS_FEATURE_LEN;
		break;

	case CGMS_CHAR_TYPE_STATUS:
		length = CGMS_STATUS_LEN;
		break;

	case CGMS_CHAR_TYPE_SESSION_START_TIME:
		length = CGMS_SESSION_START_TIME_LEN;
		break;

	default:
		length = CGMS_SESSION_RUN_TIME_LEN;
		break;
	}

	cgms_buf_alloc(&p_buf, length);
	p_data = co_buf_data(p_buf);

	switch (char_type) {
	case CGMS_CHAR_TYPE_FEATURE:
		/* CGM Feature field */
		*p_data++ = 0u;

		/* CGMSS_E2E_CRC */
		*p_data++ = 0u;
		*p_data++ = 0u;

		/* CGM Type-Sample Location field */
		*p_data = CGMS_CGM_TYPE | (CGMS_SAMPLE_LOCATION << 4);
		break;

	case CGMS_CHAR_TYPE_STATUS:
		/* Time offset field */
		co_write16(p_data, co_htole16(time_offset_minutes));
		p_data += 2u;
		/* CGM Status field */
		*p_data++ = 0u;
		*p_data++ = 0u;
		*p_data = 0u;
		break;

	case CGMS_CHAR_TYPE_SESSION_START_TIME:
		/* Session Start Time field */
		co_write16(p_data, co_htole16(app_env.start_time.date_time.year));
		p_data += 2u;
		*p_data++ = app_env.start_time.date_time.month;
		*p_data++ = app_env.start_time.date_time.day;
		*p_data++ = app_env.start_time.date_time.hour;
		*p_data++ = app_env.start_time.date_time.min;
		*p_data++ = app_env.start_time.date_time.sec;
		/* Time Zone field */
		*p_data++ = app_env.start_time.time_zone;
		/* DST Offset field */
		*p_data = app_env.start_time.dst_offset;
		break;

	default:
		uint16_t time = CGMS_RUN_TIME_HOURS;

		co_write16(p_data, co_htole16(time));
		break;
	}

	cgmss_value_cfm(conidx, token, char_type, p_buf);
	co_buf_release(p_buf);

	LOG_DBG("Read request for %s characteristic", cgms_char_name[char_type]);
}

static bool racp_report_number_greater_than_or_equal(racp_resp_t *racp_response, uint8_t op_code,
						     uint8_t *ptr, uint16_t length)
{
	if (length < 3u) {
		racp_response_opcode_pack(racp_response, op_code,
					  CGMS_RA_RSP_CODE_INVALID_OPERATOR);
		return false;
	}

	uint8_t filter_type = *ptr++;

	if (filter_type != CGMS_FILTER_TYPE_TIME_OFFSET) {
		racp_response_opcode_pack(racp_response, op_code,
					  CGMS_RA_RSP_CODE_OPERAND_NOT_SUPPORTED);
		return false;
	}

	app_env.filter_time_offset = co_letoh16(co_read16(ptr));
	return true;
}

static void on_control_req(uint8_t conidx, uint8_t char_type, uint16_t token, co_buf_t *p_buf)
{
	/* No records stored for this sample application */
	uint16_t status = GAP_ERR_NO_ERROR;
	uint8_t *p_data = co_buf_data(p_buf);
	uint16_t length = co_buf_data_len(p_buf);
	uint8_t opcode;
	uint8_t operator;
	cgms_record_t *record = NULL;

	racp_resp_t racp_response = {0};
	socp_resp_t socp_response = {0};

	bool send_racp_response = false;
	bool start_report = false;
	bool snd_socp_rsp = false;

	switch (char_type) {
	case CGMS_CHAR_TYPE_RACP:
		/* Operand: Op code (1 byte) + Operator (1 byte) + optional filter parameters */
		if (length < CGMS_RACP_WRITTEN_MIN_LEN) {
			status = PRF_ERR_INVALID_PARAM;
			break;
		}

		opcode = *p_data++;
		operator = *p_data++;
		length -= 2u;

		if (opcode == CGMS_RA_OPCODE_REPORT_NUMBER) {
			LOG_INF("Received RACP Report Number command with operator %u len "
				"%u",
				operator, length);

			send_racp_response = true;
			if (operator == CGMS_RA_OPERATOR_ALL) {
				app_env.filter_time_offset = 0;
				racp_resp_number_of_records_pack(&racp_response);
			} else if (operator == CGMS_RA_OPERATOR_GREAT_THAN_OR_EQUAL) {
				if (racp_report_number_greater_than_or_equal(&racp_response, opcode,
									     p_data, length)) {
					racp_resp_number_of_records_pack(&racp_response);
				}

			} else {
				LOG_INF("Unsupported operator %u for Report Number command",
					operator);
				racp_response_opcode_pack(&racp_response, opcode,
							  CGMS_RA_RSP_CODE_OPERAND_NOT_SUPPORTED);
			}

		} else if (opcode == CGMS_RA_OPCODE_REPORT) {
			LOG_INF("Received RACP Report record command with operator %u len "
				"%u",
				operator, length);

			if (app_env.racp_report_needed) {
				LOG_INF("Report procedure already in progress, cannot "
					"start a new one");
				status = PRF_PROC_IN_PROGRESS;
				break;
			} else if (!app_env.ready_to_send) {
				LOG_INF("Report not allowed in current state");
				status = PRF_CCCD_IMPR_CONFIGURED;
				break;
			}

			if (operator == CGMS_RA_OPERATOR_GREAT_THAN_OR_EQUAL) {
				if (racp_report_number_greater_than_or_equal(&racp_response, opcode,
									     p_data, length)) {
					start_report = true;
				} else {
					send_racp_response = true;
				}
			} else if (operator == CGMS_RA_OPERATOR_ALL) {
				if (length) {
					racp_response_opcode_pack(&racp_response, opcode,
								  CGMS_RA_RSP_CODE_INVALID_OPERAND);
					send_racp_response = true;
				} else {
					app_env.filter_time_offset = 0;
					start_report = true;
				}
			} else {
				LOG_INF("Unsupported operator %u for Report Record command",
					operator);
				send_racp_response = true;
				if (operator < CGMS_RA_OPERATOR_ALL) {
					racp_response_opcode_pack(
						&racp_response, opcode,
						CGMS_RA_RSP_CODE_INVALID_OPERATOR);
				} else {
					racp_response_opcode_pack(
						&racp_response, opcode,
						CGMS_RA_RSP_CODE_OPERATOR_NOT_SUPPORTED);
				}
			}

			if (start_report) {
				record_read_index = 0;
				record = read_entry_report_stored();
				if (!record) {
					LOG_INF("No report to send");
					racp_response_opcode_pack(
						&racp_response, CGMS_RA_OPCODE_REPORT,
						CGMS_RA_RSP_CODE_NO_RECORDS_FOUND);
					start_report = false;
					send_racp_response = true;
				} else {
					app_env.racp_report_needed = true;
					app_env.ready_to_send = false;
					LOG_INF("Start report records");
				}
			}

		} else if (opcode == CGMS_RA_OPCODE_ABORT) {
			LOG_INF("Received RACP Abort command with operator %u len %u", operator,
				length);
			send_racp_response = true;
			if (app_env.racp_report_needed) {
				app_env.racp_report_needed = false;
				racp_response_opcode_pack(&racp_response, opcode,
							  CGMS_RA_RSP_CODE_SUCCESS);

			} else {
				LOG_INF("No procedure to abort");
				racp_response_opcode_pack(&racp_response, opcode,
							  CGMS_RA_RSP_CODE_ABORT_UNSUCCESSFUL);
			}

		} else {
			LOG_INF("Unsupported RACP opcode %u with operator %u and length %u", opcode,
				operator, length);
			send_racp_response = true;
			racp_response_opcode_pack(&racp_response, opcode,
						  CGMS_RA_RSP_CODE_OPCODE_NOT_SUPPORTED);
		}
		break;
	case CGMS_CHAR_TYPE_SOCP:
		/* Operand: Op code (1 byte) + optional parameters */
		if (length < CGMS_SOCP_WRITTEN_MIN_LEN) {
			status = PRF_ERR_INVALID_PARAM;
			break;
		}

		opcode = *p_data++;
		length -= 1u;
		snd_socp_rsp = true;

		if (opcode == CGMS_SPECIFIC_OPCODE_SET_INTERVAL) {
			LOG_INF("Received SOCP Set Communication Interval command len %u", length);
			if (length) {
				uint8_t interval = *p_data++;

				if (interval == CGMS_SET_FASTEST_INTERVAL_VALUE) {
					/* Use smallest possible value */
					interval = CGMS_MIN_INTERVAL;
				}
				app_env.communication_interval = interval;

				LOG_INF("Set Communication Interval to %u seconds",
					app_env.communication_interval);
				socp_response_opcode_pack(&socp_response, opcode,
							  CGMS_SPECIFIC_RSP_CODE_SUCCESS);
			} else {
				socp_response_opcode_pack(&socp_response, opcode,
							  CGMS_SPECIFIC_RSP_CODE_INVALID_OPERAND);
			}

		} else if (opcode == CGMS_SPECIFIC_OPCODE_GET_INTERVAL) {
			LOG_INF("Received SOCP Get Communication Interval command");
			socp_response_opcode_pack(&socp_response, opcode,
						  app_env.communication_interval);
		} else {
			LOG_INF("Unsupported SOCP opcode %u with length %u", opcode, length);
			socp_response_opcode_pack(&socp_response, opcode,
						  CGMS_SPECIFIC_RSP_CODE_OPCODE_NOT_SUPPORTED);
		}
		break;
	default:
		LOG_DBG("No records available %u, len %u", char_type, length);
		status = PRF_ERR_REQ_DISALLOWED;
		break;
	}

	cgmss_set_value_cfm(conidx, status, token);
	if (send_racp_response) {
		send_racp_resp(conidx, &racp_response);
	}

	if (start_report) {
		send_measurement(record->time_offset, record->glucose);
	}

	if (snd_socp_rsp) {
		send_socp_resp(conidx, &socp_response);
	}
}

static void on_get_cccd_req(uint8_t conidx, uint8_t char_type, uint16_t token)
{
	co_buf_t *p_buf;
	uint16_t value;

	value = PRF_CLI_STOP_NTFIND;

	if ((app_env.bond_data.cccd_state_bf & CO_BIT(char_type)) != 0) {
		value = (char_type == CGMS_CHAR_TYPE_MEASUREMENT) ? PRF_CLI_START_NTF
								  : PRF_CLI_START_IND;
	}

	cgms_buf_alloc(&p_buf, PRF_CCC_DESC_LEN);
	co_write16(co_buf_data(p_buf), co_htole16(value));
	cgmss_get_cccd_cfm(conidx, token, p_buf);
	co_buf_release(p_buf);

	LOG_DBG("Get CCCD request for %s characteristic", cgms_char_name[char_type]);
}

static void on_set_cccd_req(uint8_t conidx, uint8_t char_type, uint16_t token, co_buf_t *p_buf)
{
	uint16_t status = GAP_ERR_NO_ERROR;
	uint16_t value;

	value = co_letoh16(co_read16(co_buf_data(p_buf)));

	if (value != PRF_CLI_STOP_NTFIND) {
		if (((char_type == CGMS_CHAR_TYPE_MEASUREMENT) && (value == PRF_CLI_START_NTF)) ||
		    ((char_type != CGMS_CHAR_TYPE_MEASUREMENT) && (value == PRF_CLI_START_IND))) {
			app_env.bond_data.cccd_state_bf |= CO_BIT(char_type);
			app_env.ready_to_send = true;
		} else {
			status = ATT_ERR_VALUE_NOT_ALLOWED;
		}
	} else {
		app_env.bond_data.cccd_state_bf &= ~CO_BIT(char_type);
		app_env.ready_to_send = false;
	}
	cgmss_set_value_cfm(conidx, status, token);
}

static void on_sent(uint8_t conidx, uint8_t char_type, uint16_t status)
{
	if (app_env.racp_report_needed && (char_type == CGMS_CHAR_TYPE_MEASUREMENT) &&
	    (status == GAP_ERR_NO_ERROR)) {
		cgms_record_t *record;

		record = read_entry_report_stored();

		if (record) {
			send_measurement(record->time_offset, record->glucose);
		} else {
			LOG_INF("All reports sent send Complete response");
			racp_resp_t racp_response;

			racp_response_opcode_pack(&racp_response, CGMS_RA_OPCODE_REPORT,
						  CGMS_RA_RSP_CODE_SUCCESS);
			send_racp_resp(conidx, &racp_response);
			app_env.racp_report_needed = false;
			app_env.ready_to_send = app_env.bond_data.cccd_state_bf != 0;
		}

	} else {
		app_env.ready_to_send = true;
	}
}

static const cgmss_cbs_t cgms_cb = {
	.cb_set_session_start_time_req = on_set_session_start_time_req,
	.cb_value_req = on_value_req,
	.cb_control_req = on_control_req,
	.cb_get_cccd_req = on_get_cccd_req,
	.cb_set_cccd_req = on_set_cccd_req,
	.cb_sent = on_sent,
};

static void set_start_time(void)
{
	/* dummy session start date and time */
	app_env.start_time.date_time.year = 2025;
	app_env.start_time.date_time.month = 1;
	app_env.start_time.date_time.day = 1;
	app_env.start_time.date_time.hour = 0;
	app_env.start_time.date_time.min = 0;
	app_env.start_time.date_time.sec = 0;
	app_env.start_time.time_zone = 10;
	app_env.start_time.dst_offset = 0;
}

void server_configure(void)
{
	uint16_t err;
	uint16_t start_hdl = 0;
	uint8_t cgmss_cfg_bf = 0;

	err = prf_add_profile(TASK_ID_CGMSS, local_sec_level, 0, &cgmss_cfg_bf, &cgms_cb,
			      &start_hdl);

	if (err) {
		LOG_ERR("Error %u adding profile", err);
	}

	/* Set sample start time for CGMS session */
	set_start_time();

	/* Enable sensor annotations for the sample application */
	app_env.sensor_annotations = IS_ENABLED(CONFIG_IUT_TESTER_ENABLED);
	app_env.racp_report_needed = false;
	app_env.filter_time_offset = 0;
	app_env.communication_interval = CGMS_MIN_INTERVAL;
	init_dummy_stored_records();
}

void cgms_process(void)
{
	/* Dummy measurement data */
	sensor_value_update();

	if (s_shared_ptr->connected && app_env.ready_to_send) {
		if (app_env.communication_interval) {
			send_measurement(time_offset_minutes, glucose);
			app_env.ready_to_send = false;
		} else {
			LOG_INF("Communication interval is 0, not sending measurement");
		}

	} else if (!s_shared_ptr->connected) {
		LOG_DBG("Waiting for peer connection...\n");
		k_sem_take(&conn_sem, K_FOREVER);
	}
}

void addr_res_done(void)
{
	/* Continue app */
	k_sem_give(&conn_sem);
}

void service_conn_cgms(struct shared_control *ctrl)
{
	s_shared_ptr = ctrl;
}

void disc_notify(uint16_t reason)
{
	app_env.ready_to_send = false;
}
