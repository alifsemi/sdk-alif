/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/shell/shell.h>
#include <zephyr/net/ieee802154_radio.h>
#include <alif_mac154_api.h>
#include <stdlib.h>
#include <inttypes.h>

#define LOG_MODULE_NAME alif_802154_shell
#define LOG_LEVEL       LOG_LEVEL_INFO

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

void rx_frame_handler(struct k_work *work);

static const struct device *const radio_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_ieee802154));
static struct ieee802154_radio_api *radio_api;
static uint8_t cca_mode;
static uint8_t cca_threshold = 80;
static int16_t power;
static uint8_t channel = 11;
K_WORK_DEFINE(frame_handler, rx_frame_handler);
static struct alif_rx_frame_received received_frame;
static uint32_t reply_delay;

/* PAN ID 0x1234
 * dest:0xffff
 * src: 5e:cf:dc:2c:92:ae:c4:7f
 * key:00112233445566778899aabbccddeeff
 */
static uint8_t packet_multicast[] = {
	0x41, 0xd8, 0x85, 0x34, 0x12, 0xff, 0xff, 0x7f, 0xc4, 0xae, 0x92, 0x2c, 0xdc, 0xcf,
	0x5e, 0x7f, 0x3b, 0x01, 0xf0, 0x4d, 0x4c, 0x4d, 0x4c, 0x66, 0x0a, 0x00, 0x15, 0xee,
	0x5a, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x34, 0xcd, 0xe0, 0xba, 0x0b, 0x82,
	0xa8, 0x46, 0x62, 0x43, 0x57, 0x75, 0x00, 0x21, 0xeb, 0x24, 0xfe, 0x49, 0x9d, 0x66,
	0x9a, 0x52, 0xdf, 0xb6, 0x54, 0x4e, 0x7d, 0x63, 0x70, 0xc5, 0x02};
/*PAN ID 0x1234
 * dest:aa:68:d1:05:5c:59:43:e1
 * src: 5e:cf:dc:2c:92:ae:c4:7f
 * key:00112233445566778899aabbccddeeff
 */
static uint8_t packet_unicast[] = {
	0x61, 0xdc, 0x86, 0x34, 0x12, 0xe1, 0x43, 0x59, 0x5c, 0x05, 0xd1, 0x68, 0xaa, 0x7f,
	0xc4, 0xae, 0x92, 0x2c, 0xdc, 0xcf, 0x5e, 0x7f, 0x33, 0xf0, 0x4d, 0x4c, 0x4d, 0x4c,
	0xf3, 0x53, 0x00, 0x15, 0xef, 0x5a, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xee,
	0xeb, 0x2f, 0x3f, 0xa2, 0xca, 0xf4, 0x6d, 0x86, 0xed, 0xb2, 0xd4, 0x37, 0xed, 0xb6,
	0xb4, 0x55, 0x49, 0x89, 0x2f, 0x62, 0x9c, 0xbc, 0xa8, 0x97, 0xbd, 0x63, 0x1c, 0xca,
	0x91, 0xe8, 0xd4, 0x09, 0x83, 0x60, 0x65, 0xdd, 0x77, 0xdc, 0x34, 0x75, 0x59, 0x8a,
	0x54, 0x8e, 0xc6, 0x40, 0x48, 0x2b, 0x7b, 0xdf, 0x04, 0xd7, 0x24, 0x28, 0x8f, 0xb4,
	0x16, 0x11, 0x4e, 0x44, 0x77, 0x03, 0x8f, 0x6d, 0x62, 0x9d, 0xe7, 0x6b, 0xd7};

static uint8_t packet_tx_buffer[127];

static int hex2array(char *str, uint8_t *p_array, uint8_t len)
{
	for (int n = 0; n < len; n++) {
		while (*str != 0 && (*str == ',' || *str == ' ' || *str == ':' || *str == '.')) {
			str++;
		}
		if (*str == 0) {
			return n;
		}
		p_array[n] = strtol(str, &str, 16);
	}
	return len;
}

static int param_get_array(size_t argc, char **argv, char *p_param, uint8_t *p_array, uint8_t len)
{
	if (p_param && argc > 1) {
		for (int n = 0; n < (argc - 1); n++) {
			if (strcmp(argv[n], p_param) == 0) {
				return hex2array(argv[n + 1], p_array, len);
			}
		}
	}
	return 0;
}

static int64_t param_get_int(size_t argc, char **argv, char *p_param, int def_value)
{
	if (p_param && argc > 1) {
		for (int n = 0; n < (argc - 1); n++) {
			if (strcmp(argv[n], p_param) == 0) {
				return strtoll(argv[n + 1], NULL, 0);
			}
		}
	}
	return def_value;
}

static bool param_get_flag(size_t argc, char **argv, char *p_flag)
{
	if (p_flag && argc) {
		for (int n = 0; n < argc; n++) {
			if (strcmp(argv[n], p_flag) == 0) {
				return true;
			}
		}
	}
	return false;
}

static int cmd_info(const struct shell *shell, size_t argc, char **argv)
{
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
	uint64_t timestamp;
	int ret;

	if (!device_is_ready(radio_dev)) {
		shell_error(shell, "Device Not configured");
		return -EINVAL;
	}

	radio_api = (struct ieee802154_radio_api *)radio_dev->api;
	if (!radio_api) {
		shell_error(shell, "Driver not available");
		return -EINVAL;
	}
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Radio capabilities: ");
	shell_fprintf(shell, SHELL_VT100_COLOR_GREEN, "0x%x\n",
		      radio_api->get_capabilities(radio_dev));

	ret = alif_mac154_version_get(&major, &minor, &patch);
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Hal version (ret:%d): %d,%d,%d\n", ret,
		      major, minor, patch);

	ret = alif_mac154_timestamp_get(&timestamp);
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "LL Time: %" PRIu64 "\n", timestamp);

	uint8_t ext_addr[8];

	alif_mac154_get_extended_address(ext_addr);

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
		      "  Extended address:0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\n", ext_addr[7],
		      ext_addr[6], ext_addr[5], ext_addr[4], ext_addr[3], ext_addr[2], ext_addr[1],
		      ext_addr[0]);

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
		      "  PANID: 0x%x short address: 0x%x pwr:%d, last rssi:%d, promisc:%d\n",
		      alif_mac154_get_pan_id(), alif_mac154_get_short_address(),
		      alif_mac154_get_tx_power(), alif_mac154_get_last_rssi(),
		      alif_mac154_get_promiscuous_mode());

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "  cca mode:%d threshold:%d\n",
		      alif_mac154_get_cca_mode(), alif_mac154_get_cca_threshold());

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
		      "  prio TX:0x%x prio RX:0x%x prio ED:0x%x\n", alif_mac154_get_priority_tx(),
		      alif_mac154_get_priority_rx(), alif_mac154_get_priority_ed());

	return 0;
}

static int cmd_energy_detect(const struct shell *shell, size_t argc, char **argv)
{
	struct alif_energy_detect ed_req = {0};
	struct alif_energy_detect_response ed_resp = {0};

	ed_req.channel = param_get_int(argc, argv, "--ch", channel);
	ed_req.nb_tics = param_get_int(argc, argv, "--ticks", 10);
	ed_req.threshold = param_get_int(argc, argv, "--thr", 100);

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Energy detect ch:%d ticks:%d thr:%d\n",
		      ed_req.channel, ed_req.nb_tics, ed_req.threshold);

	int ret = alif_mac154_energy_detection(&ed_req, &ed_resp);

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
		      "Energy detect result ret:%d nb:%d avg:%d max:%d\n", ret, ed_resp.nb_measure,
		      ed_resp.average, ed_resp.max);
	return 0;
}

static int cmd_rx_start(const struct shell *shell, size_t argc, char **argv)
{
	struct alif_rx_enable rx_enable_req = {0};
	int ret;

	rx_enable_req.channel = param_get_int(argc, argv, "--ch", channel);
	rx_enable_req.frames = param_get_int(argc, argv, "--count", 0);
	rx_enable_req.timestamp = param_get_int(argc, argv, "--timestamp", 0);

	ret = alif_mac154_receive_start(&rx_enable_req);
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Start RX ret:%x ch:%d count:%d\n", ret,
		      rx_enable_req.channel, rx_enable_req.frames);
	return 0;
}

static int cmd_filter(const struct shell *shell, size_t argc, char **argv)
{
	uint16_t panid;
	uint16_t short_addr;
	uint8_t ext_addr[8] = {0};
	int ret;

	if (param_get_flag(argc, argv, "--panid")) {
		panid = param_get_int(argc, argv, "--panid", 0xfff);
		ret = alif_mac154_pan_id_set(panid);
		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "set filter panid:0x%x ret:%d\n",
			      panid, ret);
	}
	if (param_get_flag(argc, argv, "--short")) {
		short_addr = param_get_int(argc, argv, "--short", 0xfff);
		ret = alif_mac154_short_address_set(short_addr);
		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
			      "set filter short address:0x%x ret:%d\n", short_addr, ret);
	}
	if (param_get_flag(argc, argv, "--ext")) {
		param_get_array(argc, argv, "--ext", ext_addr, 8);
		/* Convert from big endian to little endian */
		for (int n = 0; n < 4; n++) {
			uint8_t a;

			a = ext_addr[n];
			ext_addr[n] = ext_addr[7 - n];
			ext_addr[7 - n] = a;
		}
		ret = alif_mac154_extended_address_set(ext_addr);
		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
			      "set filter extended address:0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x "
			      "ret:%d\n",
			      ext_addr[7], ext_addr[6], ext_addr[5], ext_addr[4], ext_addr[3],
			      ext_addr[2], ext_addr[1], ext_addr[0], ret);
	}
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "set filters\n");
	return 0;
}

static int cmd_pending(const struct shell *shell, size_t argc, char **argv)
{
	uint16_t short_addr;
	uint8_t ext_addr[8] = {0};
	int ret;

	if (param_get_flag(argc, argv, "--short")) {
		short_addr = param_get_int(argc, argv, "--short", 0xfff);
		if (param_get_flag(argc, argv, "--del")) {
			ret = alif_mac154_pendings_short_address_remove(short_addr);
		} else {
			ret = alif_mac154_pendings_short_address_insert(short_addr);
		}
		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
			      "set pendings short address:0x%x ret:%d\n", short_addr, ret);
	}
	if (param_get_flag(argc, argv, "--ext")) {
		param_get_array(argc, argv, "--ext", ext_addr, 8);
		/* Convert from big endian to little endian */
		for (int n = 0; n < 4; n++) {
			uint8_t a;

			a = ext_addr[n];
			ext_addr[n] = ext_addr[7 - n];
			ext_addr[7 - n] = a;
		}
		if (param_get_flag(argc, argv, "--del")) {
			ret = alif_mac154_pendings_long_address_remove(ext_addr);
		} else {
			ret = alif_mac154_pendings_long_address_insert(ext_addr);
		}

		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
			      "set Pendings extended "
			      "address:0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x ret:%d\n",
			      ext_addr[7], ext_addr[6], ext_addr[5], ext_addr[4], ext_addr[3],
			      ext_addr[2], ext_addr[1], ext_addr[0], ret);
	}
	return 0;
}

static int cmd_rx_stop(const struct shell *shell, size_t argc, char **argv)
{
	int ret;

	ret = alif_mac154_receive_stop();
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Stop RX ret:%x\n", ret);
	return 0;
}

void rx_frame_handler(struct k_work *work)
{
	LOG_INF("RX frame received size:%d, rssi:%d, fpb %d time:%" PRId64, received_frame.len,
		received_frame.rssi, received_frame.frame_pending, received_frame.timestamp);

	if (reply_delay) {
		struct alif_tx_req transmit_req = {0};
		struct alif_tx_ack_resp transmit_resp = {0};
		int ret;

		transmit_req.channel = channel;
		transmit_req.cca_requested = true;
		transmit_req.acknowledgment_asked = false;
		transmit_req.timestamp = received_frame.timestamp + reply_delay;
		transmit_req.msg_id = 0;
		transmit_req.length = sizeof(packet_unicast);
		transmit_req.p_payload = packet_unicast;
		ret = alif_mac154_transmit(&transmit_req, &transmit_resp);
	}
}

static void cmd_rx_frame_callback(struct alif_rx_frame_received *p_frame_recv)

{
	LOG_INF("RX frame received size:%d, rssi:%d, fpb %d", p_frame_recv->len, p_frame_recv->rssi,
		p_frame_recv->frame_pending);
	received_frame.len = p_frame_recv->len;
	received_frame.timestamp = p_frame_recv->timestamp;
	received_frame.frame_pending = p_frame_recv->frame_pending;
	received_frame.rssi = p_frame_recv->rssi;
	k_work_submit(&frame_handler);
}

static void cmd_rx_status_callback(enum alif_mac154_status_code status)
{
	LOG_INF("RX status cb: %d", status);
}

static int cmd_sniffer(const struct shell *shell, size_t argc, char **argv)
{
	struct alif_mac154_api_cb api_cb = {0};

	reply_delay = param_get_int(argc, argv, "--reply", 0);

	api_cb.rx_frame_recv_cb = cmd_rx_frame_callback;
	api_cb.rx_status_cb = cmd_rx_status_callback;
	alif_mac154_init(&api_cb);
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Start Sniffer mode\n");
	return 0;
}

static int cmd_transmit(const struct shell *shell, size_t argc, char **argv)
{
	struct alif_tx_req transmit_req = {0};
	struct alif_tx_ack_resp transmit_resp = {0};
	int ret;
	int count;
	int delay;

	transmit_req.channel = param_get_int(argc, argv, "--ch", channel);
	transmit_req.cca_requested = param_get_flag(argc, argv, "--cca");
	transmit_req.acknowledgment_asked = param_get_flag(argc, argv, "--ack");
	transmit_req.timestamp = param_get_int(argc, argv, "--timestamp", 0);
	transmit_req.msg_id = 0;

	if (param_get_flag(argc, argv, "--pkt")) {
		transmit_req.length = param_get_array(argc, argv, "--pkt", packet_tx_buffer,
						      sizeof(packet_tx_buffer));
		transmit_req.p_payload = packet_tx_buffer;
	} else if (param_get_flag(argc, argv, "--pkt1")) {
		transmit_req.length = sizeof(packet_unicast);
		transmit_req.p_payload = packet_unicast;
	} else {
		transmit_req.length = sizeof(packet_multicast);
		transmit_req.p_payload = packet_multicast;
	}
	count = param_get_int(argc, argv, "--count", 1);
	delay = param_get_int(argc, argv, "--delay", 50);

	for (int n = 0; n < count; n++) {
		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
			      "transmit Packet[%d] len:%d ch:%d cca:%d ack:%d\n", n,
			      transmit_req.length, transmit_req.channel, transmit_req.cca_requested,
			      transmit_req.acknowledgment_asked);

		ret = alif_mac154_transmit(&transmit_req, &transmit_resp);

		shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "transmit resp:");
		if (ret == ALIF_MAC154_STATUS_CHANNEL_ACCESS_FAILURE) {
			shell_fprintf(shell, SHELL_VT100_COLOR_RED, "CCA Failed\n");
		} else if (ret == ALIF_MAC154_STATUS_NO_ACK) {
			shell_fprintf(shell, SHELL_VT100_COLOR_RED, "No ACK\n");
		} else {
			shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
				      "ret:%d rssi:%d ack: 0x%x,0x%x,0x%x\n", ret,
				      transmit_resp.ack_rssi, transmit_resp.ack_msg[0],
				      transmit_resp.ack_msg[1], transmit_resp.ack_msg[2]);
		}
		k_sleep(K_MSEC(delay));
	}

	return 0;
}

static int cmd_power(const struct shell *shell, size_t argc, char **argv)
{
	int16_t dbm;
	int ret;

	dbm = atoi(argv[argc - 1]);
	ret = alif_mac154_tx_power_set(dbm);
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Set tx power:%d ret:%d\n", dbm, ret);
	if (ret == 0) {
		power = dbm;
	}
	return 0;
}

static int cmd_promiscious(const struct shell *shell, size_t argc, char **argv)
{
	bool state = false;
	int ret;

	if (strcmp(argv[argc - 1], "on") == 0) {
		state = true;
	}

	ret = alif_mac154_promiscious_mode_set(state);
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Set promiscious mode:%d ret:%d\n", state,
		      ret);
	return 0;
}

static int cmd_config(const struct shell *shell, size_t argc, char **argv)
{
	uint32_t key;
	uint32_t value;
	uint32_t read_value;
	bool write;
	int ret;

	key = param_get_int(argc, argv, "--key", 0);
	value = param_get_int(argc, argv, "--write", 0);
	write = param_get_flag(argc, argv, "--write");
	ret = alif_mac154_dbg_rf(write, key, value, &read_value);

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT,
		      "config key:0x%x value:0x%x, read:%x ret:%d\n", key, value, read_value, ret);
	return 0;
}

static int cmd_reset(const struct shell *shell, size_t argc, char **argv)
{
	int ret = alif_mac154_reset();

	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "Reset Radio %x\n", ret);
	return 0;
}

static int cmd_cca_config(const struct shell *shell, size_t argc, char **argv)
{
	int ret;

	cca_mode = param_get_int(argc, argv, "--mode", cca_mode);
	ret = alif_mac154_cca_mode_set(cca_mode);
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "mode set: %d ret:%x\n", cca_mode, ret);

	cca_threshold = param_get_int(argc, argv, "--thr", cca_threshold);
	ret = alif_mac154_ed_threshold_set(cca_threshold);
	shell_fprintf(shell, SHELL_VT100_COLOR_DEFAULT, "threshold set: %d ret:%x\n", cca_threshold,
		      ret);

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_cmds, SHELL_CMD_ARG(info, NULL, "RF driver info", cmd_info, 1, 10),
	SHELL_CMD_ARG(transmit, NULL,
		      "Transmit packet --ch <channel> --count <amount> --pkt <byte array> --cca "
		      "--ack -delay <millisec>",
		      cmd_transmit, 1, 10),
	SHELL_CMD_ARG(rx-start, NULL, "start receiver --ch <channel> --count <amount>",
		      cmd_rx_start, 1, 10),
	SHELL_CMD_ARG(rx-stop, NULL, "stop receiver", cmd_rx_stop, 1, 10),
	SHELL_CMD_ARG(filter, NULL,
		      "set filters --panid <panid> --short <Short Address> --ext <8 byte array "
		      "long address>",
		      cmd_filter, 1, 10),
	SHELL_CMD_ARG(pending, NULL,
		      "set FPB --short <Short Address> --ext <8 byte array long address> --del",
		      cmd_pending, 1, 10),
	SHELL_CMD_ARG(energy, NULL,
		      "energy scan  --ch <channel> --ticks <128 us ticks> --thr <threshold>",
		      cmd_energy_detect, 1, 10),
	SHELL_CMD_ARG(reset, NULL, "Reset Radio", cmd_reset, 1, 10),
	SHELL_CMD_ARG(power, NULL, "power <level>", cmd_power, 2, 3),
	SHELL_CMD_ARG(sniffer, NULL, "start the sniffer mode. --reply <delay in ms>", cmd_sniffer,
		      1, 10),
	SHELL_CMD_ARG(config, NULL, "config RF --key <id> --write <value>", cmd_config, 1, 10),
	SHELL_CMD_ARG(promiscious, NULL, "promiscious <on/off>", cmd_promiscious, 2, 3),
	SHELL_CMD_ARG(cca, NULL,
		      "Configure CCA parameters  --mode [0 (ed),1 (CS),2 (ed or CS),3 (ed and CS)] "
		      "--thr <threshold>",
		      cmd_cca_config, 1, 10),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(rf, &sub_cmds, "IEEE802154 RF API test commands", NULL);
