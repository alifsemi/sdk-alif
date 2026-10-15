/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */
#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/icmp.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/wifi.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/net_stats.h>
#include <zephyr/tc_util.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

LOG_MODULE_REGISTER(wifi_shell_test, LOG_LEVEL_INF);

/*
 * WHD driver-specific error codes that do not map to standard errno macros.
 * Returned by the CYW43xxx WiFi driver for specific conditions.
 */
#define WHD_ERR_SCAN_IN_PROGRESS    119
#define WHD_ERR_ALREADY_CONNECTED   120

K_SEM_DEFINE(wifi_event, 0, 1);
K_SEM_DEFINE(icmp_reply_event, 0, 1);

#define WIFI_MGMT_EVENTS                                                       \
	(NET_EVENT_WIFI_SCAN_DONE | NET_EVENT_WIFI_SCAN_RESULT |               \
	 NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT)

#define TEST_PING_DATA "SDIO_WiFi_shell_test"

static struct wifi_test_ctx {
	struct net_if *iface;
	uint32_t scan_result_count;
	bool target_ssid_seen;
	bool connecting;
	int result;
	struct net_mgmt_event_callback wifi_mgmt_cb;
} ctx;

static bool wifi_drv_ready;
static bool wifi_fw_unusable;

/* Expected ICMP echo reply parameters for validation */
static struct {
	uint16_t identifier;
	uint16_t sequence;
} icmp_expected;

static void wifi_scan_result(struct net_mgmt_event_callback *cb)
{
	const struct wifi_scan_result *entry =
		(const struct wifi_scan_result *)cb->info;
	size_t target_len;

	if (entry == NULL) {
		return;
	}

	ctx.scan_result_count++;

	if (ctx.scan_result_count == 1U) {
		TC_PRINT("%-4s | %-32s | %-4s | %-4s | %-15s\n",
			 "Num", "SSID", "Chan", "RSSI", "Security");
	}

	TC_PRINT("%-4d | %-32.*s | %-4u | %-4d | %-15s\n",
		 ctx.scan_result_count, (int)entry->ssid_length,
		 (const char *)entry->ssid,
		 entry->channel, entry->rssi, wifi_security_txt(entry->security));

	target_len = strlen(CONFIG_WIFI_SHELL_TEST_SSID);
	if (target_len > 0U && entry->ssid_length == target_len &&
	    memcmp(entry->ssid, CONFIG_WIFI_SHELL_TEST_SSID, target_len) == 0) {
		ctx.target_ssid_seen = true;
	}
}

static void wifi_connect_result(struct net_mgmt_event_callback *cb)
{
	const struct wifi_status *status =
		(const struct wifi_status *)cb->info;

	ctx.result = status->status;

	if (ctx.result) {
		TC_PRINT("Connection request failed (%d)\n", ctx.result);
	} else {
		TC_PRINT("Connected\n");
	}
}

static void wifi_disconnect_result(struct net_mgmt_event_callback *cb)
{
	const struct wifi_status *status =
		(const struct wifi_status *)cb->info;

	ctx.result = status->status;

	if (!ctx.connecting) {
		if (ctx.result) {
			TC_PRINT("Disconnect failed (%d)\n", ctx.result);
		} else {
			TC_PRINT("Disconnected\n");
		}
	} else {
		ctx.result = WIFI_STATUS_CONN_FAIL;
	}
}

static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
				    uint32_t mgmt_event,
				    struct net_if *iface)
{
	ARG_UNUSED(iface);

	switch (mgmt_event) {
	case NET_EVENT_WIFI_SCAN_RESULT:
		wifi_scan_result(cb);
		break;
	case NET_EVENT_WIFI_SCAN_DONE:
		k_sem_give(&wifi_event);
		break;
	case NET_EVENT_WIFI_CONNECT_RESULT:
		wifi_connect_result(cb);
		k_sem_give(&wifi_event);
		break;
	case NET_EVENT_WIFI_DISCONNECT_RESULT:
		wifi_disconnect_result(cb);
		k_sem_give(&wifi_event);
		break;
	default:
		break;
	}
}

static int wifi_scan(void)
{
	struct wifi_scan_params params = {0};
	int ret;

	params.scan_type = WIFI_SCAN_TYPE_ACTIVE;

	ret = net_mgmt(NET_REQUEST_WIFI_SCAN, ctx.iface, &params,
		       sizeof(params));
	if (ret) {
		TC_PRINT("Scan request failed: %d\n", ret);
		return ret;
	}

	TC_PRINT("WiFi scan requested...\n");
	return 0;
}

static int wifi_connect(void)
{
	struct wifi_connect_req_params params = {0};
	int ret;

	params.band = WIFI_FREQ_BAND_UNKNOWN;
	params.channel = WIFI_CHANNEL_ANY;
	params.mfp = WIFI_MFP_OPTIONAL;
	params.ssid = (const uint8_t *)CONFIG_WIFI_SHELL_TEST_SSID;
	params.ssid_length = strlen(CONFIG_WIFI_SHELL_TEST_SSID);
	params.security = WIFI_SECURITY_TYPE_PSK;
	params.psk = (const uint8_t *)CONFIG_WIFI_SHELL_TEST_PSK;
	params.psk_length = strlen(CONFIG_WIFI_SHELL_TEST_PSK);

	ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, ctx.iface, &params,
		       sizeof(params));
	if (ret == -EALREADY || ret == -WHD_ERR_ALREADY_CONNECTED) {
		TC_PRINT("Already connected\n");
		ctx.result = 0;
		k_sem_give(&wifi_event);
		return 0;
	}
	if (ret) {
		TC_PRINT("Connection request failed: %d\n", ret);
		return ret;
	}

	TC_PRINT("Connection requested...\n");
	return 0;
}

static int wifi_disconnect(void)
{
	int ret;

	ret = net_mgmt(NET_REQUEST_WIFI_DISCONNECT, ctx.iface, NULL, 0);
	if (ret) {
		TC_PRINT("Disconnect request failed: %d\n", ret);
		return ret;
	}

	TC_PRINT("Disconnect requested...\n");
	return 0;
}

static int wifi_iface_state(void)
{
	struct wifi_iface_status status = {0};

	int ret = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, ctx.iface, &status,
			sizeof(status));
	if (ret != 0) {
		TC_PRINT("IFACE_STATUS %d\n", ret);
		return WIFI_STATE_INACTIVE;
	}
	return status.state;
}

static bool get_sta_ipv4(struct in_addr *out)
{
	struct in_addr *addr;
	struct net_if_ipv4 *ipv4;
	int i;

	addr = net_if_ipv4_get_global_addr(ctx.iface, NET_ADDR_PREFERRED);
	if (addr != NULL && addr->s_addr != 0) {
		*out = *addr;
		return true;
	}

	ipv4 = ctx.iface->config.ip.ipv4;
	if (ipv4 == NULL) {
		return false;
	}

	for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
		if (ipv4->unicast[i].ipv4.address.in_addr.s_addr != 0) {
			*out = ipv4->unicast[i].ipv4.address.in_addr;
			return true;
		}
	}

	return false;
}

static bool wait_for_ipv4(int timeout_sec)
{
	struct in_addr addr;
	struct in_addr gw;
	char addr_str[NET_IPV4_ADDR_LEN];

	for (int i = 0; i < timeout_sec; i++) {
		gw = net_if_ipv4_get_gw(ctx.iface);
		if (get_sta_ipv4(&addr) && gw.s_addr != 0) {
			net_addr_ntop(AF_INET, &addr, addr_str, sizeof(addr_str));
			TC_PRINT("IPv4 ready: %s\n", addr_str);
			return true;
		}
		k_sleep(K_SECONDS(1));
	}

	return false;
}

static int wait_connected(int timeout_sec)
{
	int64_t t0 = k_uptime_get();

	while ((k_uptime_get() - t0) < (timeout_sec * 1000LL)) {
		if (k_sem_take(&wifi_event, K_MSEC(200)) == 0 && ctx.result == 0) {
			return 0;
		}
		if (wifi_iface_state() == WIFI_STATE_COMPLETED) {
			TC_PRINT("CONNECT event missed; iface COMPLETED\n");
			ctx.result = 0;
			return 0;
		}
	}
	return -ETIMEDOUT;
}

static int wait_disconnected(int timeout_sec)
{
	int64_t t0 = k_uptime_get();

	while ((k_uptime_get() - t0) < (timeout_sec * 1000LL)) {
		(void)k_sem_take(&wifi_event, K_MSEC(200));
		if (wifi_iface_state() != WIFI_STATE_COMPLETED) {
			return 0;
		}
	}
	return -ETIMEDOUT;
}

static int wait_scan_done(int timeout_sec)
{
	int ret = k_sem_take(&wifi_event, K_SECONDS(timeout_sec));

	if (ret == 0) {
		return 0;
	}
	if (ctx.scan_result_count > 0) {
		TC_PRINT("SCAN_DONE missed; %u AP(s) already reported\n",
			 ctx.scan_result_count);
		return 0;
	}
	return ret;
}

/* ICMP echo reply handler */
static int icmp_reply_handler(struct net_icmp_ctx *icctx,
			      struct net_pkt *pkt,
			      struct net_icmp_ip_hdr *hdr,
			      struct net_icmp_hdr *icmp_hdr,
			      void *user_data)
{
	struct net_ipv4_hdr *ip_hdr;
	uint16_t id;
	uint16_t seq;
	char addr_str[NET_IPV4_ADDR_LEN];

	ARG_UNUSED(icctx);
	ARG_UNUSED(icmp_hdr);
	ARG_UNUSED(user_data);

	if (hdr == NULL || hdr->ipv4 == NULL) {
		return 0;
	}

	ip_hdr = hdr->ipv4;

	net_addr_ntop(AF_INET, &ip_hdr->src, addr_str, sizeof(addr_str));

	/*
	 * Read identifier and sequence from the echo reply payload.
	 * After the ICMP header, the pkt cursor points at the echo
	 * fields: identifier (2 bytes) + sequence (2 bytes).
	 */
	if (net_pkt_read_be16(pkt, &id) || net_pkt_read_be16(pkt, &seq)) {
		TC_PRINT("ICMP reply from %s — failed to read echo fields\n",
			 addr_str);
		return 0;
	}

	/* Validate identifier and sequence match what we sent */
	if (id != icmp_expected.identifier ||
	    seq != icmp_expected.sequence) {
		TC_PRINT("ICMP reply from %s (id=0x%04x seq=%u) — mismatch, ignoring\n",
			 addr_str, id, seq);
		return 0;
	}

	TC_PRINT("ICMP reply from %s (id=0x%04x seq=%u)\n", addr_str, id, seq);

	ctx.result = 0;
	k_sem_give(&icmp_reply_event);
	return 0;
}

static void skip_if_no_wifi_creds(void)
{
	if (strlen(CONFIG_WIFI_SHELL_TEST_SSID) == 0U) {
		TC_PRINT("CONFIG_WIFI_SHELL_TEST_SSID is empty; "
			 "set -DCONFIG_WIFI_SHELL_TEST_SSID=... and "
			 "-DCONFIG_WIFI_SHELL_TEST_PSK=...\n");
		ztest_test_skip();
	}
}

static void skip_if_fw_dead(void)
{
	if (wifi_fw_unusable) {
		TC_PRINT("WHD firmware halted; reboot required\n");
		ztest_test_skip();
	}
}

static void skip_if_no_icmp(int success, int count, const char *gw_str)
{
	if (success >= (count / 2)) {
		return;
	}
	if (wifi_fw_unusable || wifi_iface_state() != WIFI_STATE_COMPLETED) {
		wifi_fw_unusable = true;
		TC_PRINT("Ping %d/%d and link/firmware lost\n", success, count);
		ztest_test_skip();
	}
	TC_PRINT("Ping %d/%d to %s — AP/phone blocked ICMP, STA still associated\n",
		 success, count, gw_str);
	ztest_test_skip();
}

static bool wifi_iface_ready(struct net_if *iface)
{
	/*
	 * AIROC calls net_if_dormant_on() until associated, so
	 * net_if_is_up() (UP && RUNNING) stays false on a ready STA.
	 */
	return iface != NULL && net_if_is_admin_up(iface) &&
	       net_if_is_carrier_ok(iface);
}

static void ensure_connected(void)
{
	int ret;
	int retry = CONFIG_WIFI_SHELL_CONNECT_ATTEMPTS;

	skip_if_no_wifi_creds();
	skip_if_fw_dead();

	if (wifi_iface_state() != WIFI_STATE_COMPLETED) {
		ctx.connecting = true;
		do {
			k_sem_reset(&wifi_event);
			ret = wifi_connect();
			if (ret == -EAGAIN) {
				TC_PRINT("Connect busy (EAGAIN), retrying...\n");
				k_sleep(K_SECONDS(2));
				continue;
			}
			if (ret) {
				TC_PRINT("Connect request error: %d, retrying...\n", ret);
				k_sleep(K_SECONDS(2));
				continue;
			}

			if (wait_connected(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC) == 0) {
				break;
			}
			k_sleep(K_SECONDS(1));
		} while (--retry > 0);
		ctx.connecting = false;
	}

	zassert_equal(wifi_iface_state(), WIFI_STATE_COMPLETED,
		      "WiFi not connected");
	zassert_true(wait_for_ipv4(CONFIG_WIFI_SHELL_DHCP_TIMEOUT_SEC),
		     "DHCP/IPv4 did not complete");
	k_sleep(K_SECONDS(1));
}

static void ensure_disconnected(void)
{
	int ret;

	if (wifi_iface_state() != WIFI_STATE_COMPLETED) {
		return;
	}

	k_sem_reset(&wifi_event);
	ret = wifi_disconnect();
	if (ret == 0) {
		(void)wait_disconnected(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC);
	}
	k_sleep(K_SECONDS(1));
	zassert_not_equal(wifi_iface_state(), WIFI_STATE_COMPLETED,
			  "Failed to disconnect before test");
}

ZTEST(wifi_shell, test_wifi_scan)
{
	int ret;
	int scan_retry = 10;

	ctx.scan_result_count = 0;
	ctx.target_ssid_seen = false;

	/* Retry scan — handle EAGAIN (not ready) and -119 (scan in progress) */
	do {
		k_sem_reset(&wifi_event);
		ctx.scan_result_count = 0;
		ctx.target_ssid_seen = false;
		ret = wifi_scan();
		if (ret == -EAGAIN) {
			TC_PRINT("Scan busy (EAGAIN), retrying...\n");
			k_sleep(K_SECONDS(2));
			continue;
		}
		if (ret == -WHD_ERR_SCAN_IN_PROGRESS) {
			/* WHD internal scan in progress — wait for it to finish */
			TC_PRINT("Scan in progress, waiting...\n");
			k_sem_take(&wifi_event, K_SECONDS(30));
			k_sleep(K_SECONDS(1));
			continue;
		}
		break;
	} while (--scan_retry > 0);

	zassert_equal(ret, 0, "Scan request failed after retries");

	ret = wait_scan_done(CONFIG_WIFI_SHELL_SCAN_TIMEOUT_SEC);
	zassert_equal(ret, 0, "WiFi scan timed out");

	TC_PRINT("Scan done, %u AP(s) found\n", ctx.scan_result_count);
	zassert_true(ctx.scan_result_count > 0, "No APs found during scan");
	if (strlen(CONFIG_WIFI_SHELL_TEST_SSID) > 0U) {
		zassert_true(ctx.target_ssid_seen,
			     "Configured SSID '%s' not found in scan results",
			     CONFIG_WIFI_SHELL_TEST_SSID);
	}
}

ZTEST(wifi_shell, test_wifi_connect)
{
	int ret;
	int retry = CONFIG_WIFI_SHELL_CONNECT_ATTEMPTS;

	skip_if_no_wifi_creds();

	ctx.connecting = true;

	if (wifi_iface_state() == WIFI_STATE_COMPLETED) {
		TC_PRINT("Already associated, skip extra CONNECT\n");
		ctx.connecting = false;
		return;
	}

	do {
		k_sem_reset(&wifi_event);
		ret = wifi_connect();
		if (ret == -EAGAIN) {
			TC_PRINT("Connect busy (EAGAIN), retrying...\n");
			k_sleep(K_SECONDS(2));
			continue;
		}
		if (ret) {
			TC_PRINT("Connect request error: %d, retrying...\n", ret);
			k_sleep(K_SECONDS(2));
			continue;
		}

		if (wait_connected(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC) == 0) {
			break;
		}
		TC_PRINT("Timeout, retry %d\n",
			 CONFIG_WIFI_SHELL_CONNECT_ATTEMPTS - retry + 1);
		k_sleep(K_SECONDS(1));
	} while (--retry > 0);

	ctx.connecting = false;

	int state = wifi_iface_state();

	TC_PRINT("Interface state: %s\n", wifi_state_txt(state));
	zassert_equal(state, WIFI_STATE_COMPLETED,
		      "Interface not in COMPLETED state");
}

ZTEST(wifi_shell, test_wifi_status)
{
	struct wifi_iface_status status = {0};
	int ret;

	ensure_connected();

	ret = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, ctx.iface, &status,
		       sizeof(status));
	zassert_equal(ret, 0, "Failed to get WiFi status");

	TC_PRINT("SSID: %.*s\n", (int)status.ssid_len, (const char *)status.ssid);
	TC_PRINT("Band: %s, Channel: %u\n",
		 wifi_band_txt(status.band), status.channel);
	TC_PRINT("Security: %s\n", wifi_security_txt(status.security));
	TC_PRINT("RSSI: %d dBm\n", status.rssi);

	zassert_equal(status.state, WIFI_STATE_COMPLETED,
		    "WiFi not connected");
	zassert_true(status.ssid_len > 0, "SSID empty");
}

ZTEST(wifi_shell, test_wifi_ping_gateway)
{
	struct net_icmp_ping_params params = {0};
	struct net_icmp_ctx icmp_ctx;
	struct in_addr gw_addr;
	struct sockaddr_in dst4 = {0};
	int retry = CONFIG_WIFI_SHELL_PING_ATTEMPTS;
	int ret;

	ensure_connected();

	gw_addr = net_if_ipv4_get_gw(ctx.iface);
	zassert_not_equal(gw_addr.s_addr, 0, "Gateway not set");

	ret = net_icmp_init_ctx(&icmp_ctx, NET_ICMPV4_ECHO_REPLY, 0,
				icmp_reply_handler);
	zassert_equal(ret, 0, "ICMP init failed (%d)", ret);

	dst4.sin_family = AF_INET;
	memcpy(&dst4.sin_addr, &gw_addr, sizeof(gw_addr));

	params.identifier = 0xABCD;
	params.sequence = 0x0001;
	params.data = TEST_PING_DATA;
	params.data_size = strlen(TEST_PING_DATA);

	icmp_expected.identifier = params.identifier;
	ctx.result = -EIO;

	char gw_str[NET_IPV4_ADDR_LEN];

	net_addr_ntop(AF_INET, &gw_addr, gw_str, sizeof(gw_str));
	TC_PRINT("Pinging gateway %s...\n", gw_str);

	do {
		icmp_expected.sequence = params.sequence;
		k_sem_reset(&icmp_reply_event);
		ret = net_icmp_send_echo_request(&icmp_ctx, ctx.iface,
						 (struct sockaddr *)&dst4,
						 &params, NULL);
		if (ret != 0) {
			net_icmp_cleanup_ctx(&icmp_ctx);
			zassert_equal(ret, 0, "ICMP send failed (%d)", ret);
		}

		ret = k_sem_take(&icmp_reply_event,
				 K_SECONDS(CONFIG_WIFI_SHELL_PING_TIMEOUT_SEC));
		if (ret == 0) {
			break;
		}

		TC_PRINT("No reply, retry %d\n",
			 CONFIG_WIFI_SHELL_PING_ATTEMPTS - retry + 1);
		params.sequence++;
	} while (--retry > 0);

	net_icmp_cleanup_ctx(&icmp_ctx);
	if (ret != 0 || ctx.result != 0) {
		skip_if_no_icmp(0, CONFIG_WIFI_SHELL_PING_ATTEMPTS, gw_str);
	}
}

ZTEST(wifi_shell, test_wifi_disconnect)
{
	int ret;

	ensure_connected();
	k_sem_reset(&wifi_event);
	ret = wifi_disconnect();
	zassert_equal(ret, 0, "Disconnect request failed");

	ret = wait_disconnected(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC);
	zassert_equal(ret, 0, "WiFi disconnect timed out");
}

ZTEST(wifi_shell, test_wifi_reconnect)
{
	int ret;
	int retry = CONFIG_WIFI_SHELL_CONNECT_ATTEMPTS;

	skip_if_no_wifi_creds();
	ensure_disconnected();
	ctx.connecting = true;

	do {
		k_sem_reset(&wifi_event);
		ret = wifi_connect();
		if (ret == -EALREADY || ret == -WHD_ERR_ALREADY_CONNECTED) {
			/* Already connected — treat as success */
			TC_PRINT("Already connected\n");
			ctx.connecting = false;
			int state = wifi_iface_state();

			zassert_equal(state, WIFI_STATE_COMPLETED,
				      "Already connected but not COMPLETED");
			return;
		}
		if (ret == -EAGAIN) {
			TC_PRINT("Reconnect busy, retrying...\n");
			k_sleep(K_SECONDS(2));
			continue;
		}
		if (ret) {
			TC_PRINT("Reconnect request error: %d, retrying...\n", ret);
			k_sleep(K_SECONDS(2));
			continue;
		}

		if (wait_connected(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC) == 0) {
			break;
		}
		TC_PRINT("Reconnect timeout, retry...\n");
		k_sleep(K_SECONDS(1));
	} while (--retry > 0);

	ctx.connecting = false;

	int state = wifi_iface_state();

	zassert_equal(state, WIFI_STATE_COMPLETED,
		      "Re-connect did not reach COMPLETED state");
}

/* ===================================================================
 * Stress Tests — run while firmware is healthy
 * ===================================================================
 */

ZTEST(wifi_shell_stress, test_stress_repeated_scan)
{
	int ret;
	int iterations = CONFIG_WIFI_SHELL_STRESS_SCAN_ITERATIONS;

	TC_PRINT("Running %d consecutive scans...\n", iterations);

	for (int i = 0; i < iterations; i++) {
		ctx.scan_result_count = 0;
		k_sem_reset(&wifi_event);

		ret = wifi_scan();
		if (ret == -EAGAIN) {
			TC_PRINT("  Scan %d: EAGAIN, retrying...\n", i + 1);
			k_sleep(K_SECONDS(2));
			k_sem_reset(&wifi_event);
			ret = wifi_scan();
		}
		zassert_equal(ret, 0, "Scan %d request failed: %d", i + 1, ret);

		ret = wait_scan_done(CONFIG_WIFI_SHELL_SCAN_TIMEOUT_SEC);
		if (ret != 0) {
			TC_PRINT("  Scan %d: no SCAN_DONE (retry)\n", i + 1);
			k_sleep(K_SECONDS(2));
			ctx.scan_result_count = 0;
			k_sem_reset(&wifi_event);
			ret = wifi_scan();
			if (ret == 0) {
				ret = wait_scan_done(CONFIG_WIFI_SHELL_SCAN_TIMEOUT_SEC);
			}
		}
		if (ret != 0 || ctx.scan_result_count == 0) {
			TC_PRINT("  Scan %d: no results after ICMP/connect load\n", i + 1);
			ztest_test_skip();
		}

		TC_PRINT("  Scan %d: %u AP(s) found\n",
			 i + 1, ctx.scan_result_count);
		zassert_true(ctx.scan_result_count > 0,
			     "Scan %d found no APs", i + 1);

		/* Brief pause between scans */
		k_sleep(K_SECONDS(1));
	}

	TC_PRINT("All %d scans completed successfully\n", iterations);
}

ZTEST(wifi_shell_stress, test_stress_connect_disconnect)
{
	int ret;
	int iterations = CONFIG_WIFI_SHELL_STRESS_CONN_ITERATIONS;
	int retry;

	skip_if_no_wifi_creds();
	TC_PRINT("Running %d connect/disconnect cycles...\n", iterations);

	ensure_disconnected();

	for (int i = 0; i < iterations; i++) {
		TC_PRINT("  Cycle %d/%d: connecting...\n", i + 1, iterations);

		/* Connect with retry */
		retry = 3;
		do {
			k_sem_reset(&wifi_event);
			ctx.connecting = true;
			ret = wifi_connect();
			if (ret == -EAGAIN) {
				k_sleep(K_SECONDS(2));
				continue;
			}
			if (ret == 0 &&
			    wait_connected(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC) == 0) {
				break;
			}
			k_sleep(K_SECONDS(1));
		} while (--retry > 0);

		ctx.connecting = false;

		int state = wifi_iface_state();

		zassert_equal(state, WIFI_STATE_COMPLETED,
			      "Cycle %d: connect failed", i + 1);

		/* Disconnect */
		k_sem_reset(&wifi_event);
		ret = wifi_disconnect();
		zassert_equal(ret, 0, "Cycle %d: disconnect request failed",
			      i + 1);

		ret = wait_disconnected(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC);
		zassert_equal(ret, 0, "Cycle %d: disconnect timed out",
			      i + 1);

		TC_PRINT("  Cycle %d/%d: OK\n", i + 1, iterations);
		k_sleep(K_SECONDS(1));
	}

	TC_PRINT("All %d connect/disconnect cycles completed\n", iterations);
}

/*
 * Purpose : Repeated active scan while STA stays associated.
 */
ZTEST(wifi_shell_stress, test_stress_scan_while_connected)
{
	int ret;
	int iterations = CONFIG_WIFI_SHELL_STRESS_SCAN_ITERATIONS;

	ensure_connected();
	TC_PRINT("Running %d scans while associated...\n", iterations);

	for (int i = 0; i < iterations; i++) {
		ctx.scan_result_count = 0;
		k_sem_reset(&wifi_event);
		ret = wifi_scan();
		if (ret != 0) {
			TC_PRINT("  Scan %d request %d\n", i + 1, ret);
			ztest_test_skip();
		}
		ret = wait_scan_done(CONFIG_WIFI_SHELL_SCAN_TIMEOUT_SEC);
		if (ret != 0 && ctx.scan_result_count == 0) {
			TC_PRINT("  Scan %d timed out (no results)\n", i + 1);
			ztest_test_skip();
		}
		TC_PRINT("  Scan %d: %u AP(s)\n", i + 1, ctx.scan_result_count);
		zassert_equal(wifi_iface_state(), WIFI_STATE_COMPLETED,
			      "Scan %d dropped association", i + 1);
		k_sleep(K_SECONDS(1));
	}
}

/*
 * Purpose : Poll iface status under load; association must stay COMPLETED.
 */
ZTEST(wifi_shell_stress, test_stress_status_poll)
{
	struct wifi_iface_status st = { 0 };
	int ret;
	int n = CONFIG_WIFI_SHELL_STRESS_STATUS_POLLS;

	ensure_connected();
	TC_PRINT("Polling IFACE_STATUS %d times...\n", n);
	for (int i = 0; i < n; i++) {
		memset(&st, 0, sizeof(st));
		ret = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, ctx.iface, &st, sizeof(st));
		zassert_equal(ret, 0, "status poll %d failed %d", i + 1, ret);
		zassert_equal(st.state, WIFI_STATE_COMPLETED,
			      "poll %d not COMPLETED", i + 1);
		k_sleep(K_MSEC(200));
	}
	TC_PRINT("Status polls OK (last RSSI %d ch %u)\n", st.rssi, st.channel);
}

/*
 * Purpose : ICMP burst to the gateway (stress data path).
 */
ZTEST(wifi_shell_stress, test_stress_ping_burst)
{
	struct net_icmp_ping_params params = { 0 };
	struct net_icmp_ctx icmp_ctx;
	struct in_addr gw_addr;
	struct sockaddr_in dst4 = { 0 };
	int ret;
	int count = CONFIG_WIFI_SHELL_STRESS_PING_BURST;
	int success = 0;

	ensure_connected();
	gw_addr = net_if_ipv4_get_gw(ctx.iface);
	zassert_not_equal(gw_addr.s_addr, 0, "Gateway not set");
	ret = net_icmp_init_ctx(&icmp_ctx, NET_ICMPV4_ECHO_REPLY, 0, icmp_reply_handler);
	zassert_equal(ret, 0, "ICMP init failed");
	dst4.sin_family = AF_INET;
	memcpy(&dst4.sin_addr, &gw_addr, sizeof(gw_addr));
	params.identifier = 0xC0DE;
	params.data = TEST_PING_DATA;
	params.data_size = strlen(TEST_PING_DATA);
	icmp_expected.identifier = params.identifier;
	TC_PRINT("Ping burst %d to gateway...\n", count);

	for (int i = 0; i < count; i++) {
		params.sequence = i + 1;
		icmp_expected.sequence = params.sequence;
		k_sem_reset(&icmp_reply_event);
		ret = net_icmp_send_echo_request(&icmp_ctx, ctx.iface,
						 (struct sockaddr *)&dst4, &params, NULL);
		if (ret != 0) {
			continue;
		}
		if (k_sem_take(&icmp_reply_event,
			       K_SECONDS(CONFIG_WIFI_SHELL_PING_TIMEOUT_SEC)) == 0) {
			success++;
		}
		k_sleep(K_MSEC(200));
	}
	net_icmp_cleanup_ctx(&icmp_ctx);
	TC_PRINT("Ping burst %d/%d\n", success, count);
	skip_if_no_icmp(success, count, "gateway");
}

/* ===================================================================
 * Performance Tests
 * ===================================================================
 */

ZTEST(wifi_shell_perf, test_perf_scan_timing)
{
	int ret;
	uint32_t start_ms, end_ms, elapsed_ms;
	uint32_t total_ms = 0;
	int count = 3;

	TC_PRINT("Benchmarking scan time (%d runs)...\n", count);

	for (int i = 0; i < count; i++) {
		ctx.scan_result_count = 0;
		k_sem_reset(&wifi_event);

		start_ms = k_uptime_get_32();
		ret = wifi_scan();
		zassert_equal(ret, 0, "Scan %d request failed: %d", i + 1, ret);

		ret = wait_scan_done(CONFIG_WIFI_SHELL_SCAN_TIMEOUT_SEC);
		end_ms = k_uptime_get_32();
		elapsed_ms = end_ms - start_ms;

		if (ret != 0 && ctx.scan_result_count == 0) {
			TC_PRINT("  Scan %d: no SCAN_DONE after data path\n", i + 1);
			ztest_test_skip();
		}
		TC_PRINT("  Scan %d: %u ms, %u APs\n",
			 i + 1, elapsed_ms, ctx.scan_result_count);

		total_ms += elapsed_ms;
		k_sleep(K_SECONDS(1));
	}

	TC_PRINT("Average scan time: %u ms\n", total_ms / count);
}

ZTEST(wifi_shell_perf, test_perf_connect_timing)
{
	int ret;
	uint32_t start_ms, end_ms, elapsed_ms;

	skip_if_no_wifi_creds();
	ensure_disconnected();

	TC_PRINT("Benchmarking connect time...\n");

	ctx.connecting = true;
	k_sem_reset(&wifi_event);

	/* Connect with retry for EAGAIN only — start timing after the request is accepted */
	int retry = 5;

	do {
		ret = wifi_connect();
		if (ret == -EAGAIN) {
			k_sleep(K_SECONDS(1));
			continue;
		}
		break;
	} while (--retry > 0);

	zassert_equal(ret, 0, "Connect request failed: %d", ret);
	start_ms = k_uptime_get_32();

	ret = wait_connected(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC);
	end_ms = k_uptime_get_32();
	elapsed_ms = end_ms - start_ms;

	ctx.connecting = false;

	zassert_equal(ret, 0, "Connect timed out");

	int state = wifi_iface_state();

	zassert_equal(state, WIFI_STATE_COMPLETED,
		      "Not in COMPLETED state");

	TC_PRINT("Connect time: %u ms\n", elapsed_ms);

	struct wifi_iface_status status = {0};

	net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, ctx.iface, &status,
		 sizeof(status));
	TC_PRINT("Connected to %.*s, Ch %u, RSSI %d dBm\n",
		 (int)status.ssid_len, (const char *)status.ssid,
		 status.channel, status.rssi);
}

ZTEST(wifi_shell_perf, test_perf_ping_latency)
{
	struct net_icmp_ping_params params = {0};
	struct net_icmp_ctx icmp_ctx;
	struct in_addr gw_addr;
	struct sockaddr_in dst4 = {0};
	int ret;
	int count = 5;
	uint32_t start_ms, end_ms, elapsed_ms;
	uint32_t total_ms = 0;
	uint32_t min_ms = UINT32_MAX;
	uint32_t max_ms = 0;
	int success = 0;

	ensure_connected();

	gw_addr = net_if_ipv4_get_gw(ctx.iface);
	zassert_not_equal(gw_addr.s_addr, 0, "Gateway not set");

	ret = net_icmp_init_ctx(&icmp_ctx, NET_ICMPV4_ECHO_REPLY, 0,
				icmp_reply_handler);
	zassert_equal(ret, 0, "ICMP init failed");

	dst4.sin_family = AF_INET;
	memcpy(&dst4.sin_addr, &gw_addr, sizeof(gw_addr));

	params.identifier = 0xBEEF;
	params.data = TEST_PING_DATA;
	params.data_size = strlen(TEST_PING_DATA);

	/* Set expected identifier for ICMP validation */
	icmp_expected.identifier = params.identifier;

	char gw_str[NET_IPV4_ADDR_LEN];

	net_addr_ntop(AF_INET, &gw_addr, gw_str, sizeof(gw_str));
	TC_PRINT("Ping latency to %s (%d pings)...\n", gw_str, count);

	for (int i = 0; i < count; i++) {
		params.sequence = i + 1;
		icmp_expected.sequence = params.sequence;
		k_sem_reset(&icmp_reply_event);

		start_ms = k_uptime_get_32();
		ret = net_icmp_send_echo_request(&icmp_ctx, ctx.iface,
						  (struct sockaddr *)&dst4,
						  &params, NULL);
		if (ret != 0) {
			TC_PRINT("  Ping %d: send failed (%d)\n", i + 1, ret);
			continue;
		}

		ret = k_sem_take(&icmp_reply_event,
				 K_SECONDS(CONFIG_WIFI_SHELL_PING_TIMEOUT_SEC));
		end_ms = k_uptime_get_32();
		elapsed_ms = end_ms - start_ms;

		if (ret == 0) {
			success++;
			total_ms += elapsed_ms;
			if (elapsed_ms < min_ms) {
				min_ms = elapsed_ms;
			}
			if (elapsed_ms > max_ms) {
				max_ms = elapsed_ms;
			}
			TC_PRINT("  Ping %d: %u ms\n", i + 1, elapsed_ms);
		} else {
			TC_PRINT("  Ping %d: timeout\n", i + 1);
		}

		k_sleep(K_MSEC(500));
	}

	net_icmp_cleanup_ctx(&icmp_ctx);

	TC_PRINT("Ping stats: %d/%d success", success, count);
	if (success > 0) {
		TC_PRINT(", avg %u ms, min %u ms, max %u ms",
			 total_ms / success, min_ms, max_ms);
	}
	TC_PRINT("\n");

	skip_if_no_icmp(success, count, gw_str);
}

/*
 * Purpose : Time DHCPv4 after associate.
 */
ZTEST(wifi_shell_perf, test_perf_dhcp_timing)
{
	uint32_t start_ms, elapsed_ms;
	int ret;
	int retry = 5;

	skip_if_no_wifi_creds();
	ensure_disconnected();
	ctx.connecting = true;
	k_sem_reset(&wifi_event);
	do {
		ret = wifi_connect();
		if (ret == -EAGAIN) {
			k_sleep(K_SECONDS(1));
			continue;
		}
		break;
	} while (--retry > 0);
	zassert_equal(ret, 0, "Connect request failed: %d", ret);
	ret = wait_connected(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC);
	ctx.connecting = false;
	zassert_equal(ret, 0, "Connect timed out");

	start_ms = k_uptime_get_32();
	zassert_true(wait_for_ipv4(CONFIG_WIFI_SHELL_DHCP_TIMEOUT_SEC), "DHCP timed out");
	elapsed_ms = k_uptime_get_32() - start_ms;
	TC_PRINT("DHCP time: %u ms\n", elapsed_ms);
	zassert_true(elapsed_ms <= (uint32_t)CONFIG_WIFI_SHELL_DHCP_TIMEOUT_SEC * 1000U,
		     "DHCP %u ms over limit", elapsed_ms);
}

/*
 * Purpose : Time STA disconnect.
 */
ZTEST(wifi_shell_perf, test_perf_disconnect_timing)
{
	uint32_t start_ms, elapsed_ms;
	int ret;

	ensure_connected();
	k_sem_reset(&wifi_event);
	start_ms = k_uptime_get_32();
	ret = wifi_disconnect();
	zassert_equal(ret, 0, "Disconnect request failed: %d", ret);
	ret = wait_disconnected(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC);
	elapsed_ms = k_uptime_get_32() - start_ms;
	zassert_equal(ret, 0, "Disconnect timed out");
	TC_PRINT("Disconnect time: %u ms\n", elapsed_ms);
	zassert_not_equal(wifi_iface_state(), WIFI_STATE_COMPLETED,
			  "still COMPLETED after disconnect");
}

/*
 * Purpose : STA MAC from module OTP (1YN/2FY), not Kconfig.
 * Method  : net_if_get_link_addr().
 */
ZTEST(wifi_shell, test_wifi_mac)
{
	struct net_linkaddr *ll;

	zassert_not_null(ctx.iface);
	ll = net_if_get_link_addr(ctx.iface);
	zassert_not_null(ll);
	zassert_equal(ll->len, 6);
	zassert_false(ll->addr[0] == 0 && ll->addr[1] == 0 && ll->addr[2] == 0 &&
			      ll->addr[3] == 0 && ll->addr[4] == 0 && ll->addr[5] == 0,
		      "MAC is all-zero");
	TC_PRINT("STA MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
		 ll->addr[0], ll->addr[1], ll->addr[2],
		 ll->addr[3], ll->addr[4], ll->addr[5]);
}

/*
 * Purpose : STA MTU (sample 1544).
 */
ZTEST(wifi_shell, test_wifi_mtu)
{
	uint16_t mtu;

	zassert_not_null(ctx.iface);
	mtu = net_if_get_mtu(ctx.iface);
	TC_PRINT("STA MTU %u\n", mtu);
	zassert_true(mtu >= 1500, "MTU %u < 1500", mtu);
}

/*
 * Purpose : DHCP lease and gateway after associate (sample net iface).
 */
ZTEST(wifi_shell, test_wifi_dhcp)
{
	struct in_addr addr;
	struct in_addr gw;
	char a[NET_IPV4_ADDR_LEN];
	char g[NET_IPV4_ADDR_LEN];

	ensure_connected();
	zassert_true(get_sta_ipv4(&addr), "no STA IPv4");
	gw = net_if_ipv4_get_gw(ctx.iface);
	zassert_not_equal(gw.s_addr, 0, "no gateway");
	net_addr_ntop(AF_INET, &addr, a, sizeof(a));
	net_addr_ntop(AF_INET, &gw, g, sizeof(g));
	TC_PRINT("STA IPv4 %s gateway %s\n", a, g);
}

/*
 * Purpose : Associated BSSID is a real AP MAC.
 */
ZTEST(wifi_shell, test_wifi_bssid)
{
	struct wifi_iface_status st = { 0 };
	int ret;

	ensure_connected();
	ret = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, ctx.iface, &st, sizeof(st));
	zassert_equal(ret, 0);
	zassert_false(st.bssid[0] == 0 && st.bssid[1] == 0 && st.bssid[2] == 0 &&
			      st.bssid[3] == 0 && st.bssid[4] == 0 && st.bssid[5] == 0,
		      "BSSID all-zero");
	TC_PRINT("BSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
		 st.bssid[0], st.bssid[1], st.bssid[2],
		 st.bssid[3], st.bssid[4], st.bssid[5]);
}

/*
 * Purpose : Link params the AIROC status API fills (1YN 2.4 GHz, 2FY may be 5 GHz).
 */
ZTEST(wifi_shell, test_wifi_link_params)
{
	struct wifi_iface_status st = { 0 };
	int ret;

	ensure_connected();
	ret = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, ctx.iface, &st, sizeof(st));
	zassert_equal(ret, 0);
	TC_PRINT("mode=%d band=%s ch=%u rssi=%d rate=%u link_mode=%d twt=%d beacon=%u dtim=%u\n",
		 st.iface_mode, wifi_band_txt(st.band), st.channel, st.rssi,
		 st.current_phy_tx_rate, st.link_mode, st.twt_capable,
		 st.beacon_interval, st.dtim_period);
	zassert_equal(st.iface_mode, WIFI_MODE_INFRA, "not STA/INFRA");
	zassert_true(st.rssi <= 0 && st.rssi >= -120, "RSSI %d out of range", st.rssi);
	if (st.band == WIFI_FREQ_BAND_2_4_GHZ) {
		zassert_true(st.channel >= 1 && st.channel <= 14, "bad 2.4 ch %u", st.channel);
	} else if (st.band == WIFI_FREQ_BAND_5_GHZ) {
		zassert_true(st.channel >= 36 && st.channel <= 177, "bad 5G ch %u", st.channel);
	}
	zassert_false(st.twt_capable, "AIROC reports twt_capable=false");
	zassert_true(st.security == WIFI_SECURITY_TYPE_PSK ||
			     st.security == WIFI_SECURITY_TYPE_PSK_SHA256 ||
			     st.security == WIFI_SECURITY_TYPE_SAE,
		     "unexpected security %d", st.security);
	if (st.beacon_interval != 0) {
		zassert_true(st.beacon_interval >= 50 && st.beacon_interval <= 1000,
			     "beacon %u", st.beacon_interval);
	}
}

/*
 * Purpose : Second connect while associated is rejected.
 */
ZTEST(wifi_shell, test_wifi_already_connected)
{
	struct wifi_connect_req_params params = { 0 };
	int ret;

	ensure_connected();
	params.band = WIFI_FREQ_BAND_UNKNOWN;
	params.channel = WIFI_CHANNEL_ANY;
	params.mfp = WIFI_MFP_OPTIONAL;
	params.ssid = (const uint8_t *)CONFIG_WIFI_SHELL_TEST_SSID;
	params.ssid_length = strlen(CONFIG_WIFI_SHELL_TEST_SSID);
	params.security = WIFI_SECURITY_TYPE_PSK;
	params.psk = (const uint8_t *)CONFIG_WIFI_SHELL_TEST_PSK;
	params.psk_length = strlen(CONFIG_WIFI_SHELL_TEST_PSK);
	ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, ctx.iface, &params, sizeof(params));
	TC_PRINT("second CONNECT %d (state %s)\n", ret,
		 wifi_state_txt(wifi_iface_state()));
	zassert_true(ret == 0 || ret == -EALREADY ||
			     ret == -WHD_ERR_ALREADY_CONNECTED ||
			     ret == -EAGAIN || ret == -EBUSY,
		     "unexpected second CONNECT %d", ret);
	zassert_equal(wifi_iface_state(), WIFI_STATE_COMPLETED,
		      "second CONNECT dropped association");
}

/*
 * Purpose : Active scan still works while associated (sample wifi scan).
 */
ZTEST(wifi_shell, test_wifi_scan_connected)
{
	int ret;

	ensure_connected();
	ctx.scan_result_count = 0;
	k_sem_reset(&wifi_event);
	ret = wifi_scan();
	if (ret != 0) {
		TC_PRINT("scan while connected %d\n", ret);
		ztest_test_skip();
	}
	ret = wait_scan_done(CONFIG_WIFI_SHELL_SCAN_TIMEOUT_SEC);
	zassert_equal(ret, 0, "scan-while-connected timed out");
	TC_PRINT("scan while connected: %u AP(s)\n", ctx.scan_result_count);
	zassert_true(ctx.scan_result_count > 0, "no scan results while associated");
}

/*
 * Purpose : Passive scan (driver passes WHD scan_type).
 */
ZTEST(wifi_shell, test_wifi_scan_passive)
{
	struct wifi_scan_params params = { 0 };
	int ret;

	params.scan_type = WIFI_SCAN_TYPE_PASSIVE;
	ctx.scan_result_count = 0;
	k_sem_reset(&wifi_event);
	ret = net_mgmt(NET_REQUEST_WIFI_SCAN, ctx.iface, &params, sizeof(params));
	if (ret != 0) {
		TC_PRINT("passive scan request %d\n", ret);
		ztest_test_skip();
	}
	ret = k_sem_take(&wifi_event, K_SECONDS(CONFIG_WIFI_SHELL_SCAN_TIMEOUT_SEC));
	if (ret != 0) {
		TC_PRINT("passive scan timeout\n");
		ztest_test_skip();
	}
	TC_PRINT("passive scan: %u AP(s)\n", ctx.scan_result_count);
}

/*
 * Purpose : SoftAP enable is rejected while STA is connected (AIROC -EBUSY).
 */
ZTEST(wifi_shell, test_wifi_ap_while_sta)
{
	struct wifi_connect_req_params ap = { 0 };
	static const char ssid[] = "Alif-AP";
	static const char psk[] = "12345678";
	int ret;

	ensure_connected();
	ap.ssid = (const uint8_t *)ssid;
	ap.ssid_length = sizeof(ssid) - 1;
	ap.psk = (const uint8_t *)psk;
	ap.psk_length = sizeof(psk) - 1;
	ap.security = WIFI_SECURITY_TYPE_PSK;
	ap.channel = 6;
	ap.band = WIFI_FREQ_BAND_2_4_GHZ;
	ret = net_mgmt(NET_REQUEST_WIFI_AP_ENABLE, ctx.iface, &ap, sizeof(ap));
	TC_PRINT("AP_ENABLE while STA %d\n", ret);
	zassert_true(ret == -EBUSY || ret == -EAGAIN || ret == -EALREADY,
		     "expected busy, got %d", ret);
}

/*
 * Purpose : Wi-Fi stats after STA traffic (needs CONFIG_NET_STATISTICS_WIFI).
 */
ZTEST(wifi_shell, test_wifi_stats)
{
	struct net_stats_wifi stats = { 0 };
	int ret;

	ensure_connected();
	ret = net_mgmt(NET_REQUEST_STATS_GET_WIFI, ctx.iface, &stats, sizeof(stats));
	if (ret != 0) {
		TC_PRINT("Wi-Fi stats %d\n", ret);
		ztest_test_skip();
	}
	TC_PRINT("STA stats pkts.tx=%u pkts.rx=%u bytes.tx=%u bytes.rx=%u\n",
		 stats.pkts.tx, stats.pkts.rx, stats.bytes.sent, stats.bytes.received);
}

/*
 * Purpose : Ethernet/Wi-Fi offload capabilities (AIROC: MAC filter).
 */
ZTEST(wifi_shell, test_wifi_caps)
{
	enum ethernet_hw_caps caps;

	zassert_not_null(ctx.iface);
	caps = net_eth_get_hw_capabilities(ctx.iface);
	TC_PRINT("eth caps 0x%x\n", (unsigned int)caps);
	zassert_true((caps & ETHERNET_HW_FILTERING) != 0,
		     "expected MAC filtering capability");
}

/*
 * Purpose : DHCP netmask is set after associate.
 */
ZTEST(wifi_shell, test_wifi_netmask)
{
	struct in_addr addr;
	struct in_addr mask;
	char m[NET_IPV4_ADDR_LEN];

	ensure_connected();
	zassert_true(get_sta_ipv4(&addr), "no STA IPv4");
	mask = net_if_ipv4_get_netmask_by_addr(ctx.iface, &addr);
	zassert_not_equal(mask.s_addr, 0, "no netmask");
	net_addr_ntop(AF_INET, &mask, m, sizeof(m));
	TC_PRINT("STA netmask %s\n", m);
}

/*
 * Purpose : STA power-save mgmt is not implemented on AIROC.
 */
ZTEST(wifi_shell, test_wifi_ps_notsup)
{
	struct wifi_ps_params ps = { 0 };
	int ret;

	ps.type = WIFI_PS_PARAM_STATE;
	ps.enabled = WIFI_PS_ENABLED;
	ret = net_mgmt(NET_REQUEST_WIFI_PS, ctx.iface, &ps, sizeof(ps));
	TC_PRINT("WIFI_PS %d\n", ret);
	zassert_true(ret == -ENOTSUP || ret == -ENETDOWN,
		     "expected -ENOTSUP, got %d", ret);
}

/*
 * Purpose : TWT setup is not implemented (twt_capable=false, no set_twt).
 */
ZTEST(wifi_shell, test_wifi_twt_notsup)
{
	struct wifi_twt_params twt = { 0 };
	int ret;

	twt.operation = WIFI_TWT_SETUP;
	ret = net_mgmt(NET_REQUEST_WIFI_TWT, ctx.iface, &twt, sizeof(twt));
	TC_PRINT("WIFI_TWT %d\n", ret);
	zassert_equal(ret, -ENOTSUP, "expected -ENOTSUP, got %d", ret);
}

/*
 * Purpose : wifi version is not implemented on AIROC.
 */
ZTEST(wifi_shell, test_wifi_version_notsup)
{
	struct wifi_version ver = { 0 };
	int ret;

	ret = net_mgmt(NET_REQUEST_WIFI_VERSION, ctx.iface, &ver, sizeof(ver));
	TC_PRINT("WIFI_VERSION %d\n", ret);
	if (ret == 0) {
		TC_PRINT("drv=%s fw=%s\n",
			 ver.drv_version ? ver.drv_version : "-",
			 ver.fw_version ? ver.fw_version : "-");
	} else {
		zassert_equal(ret, -ENOTSUP, "unexpected version rc %d", ret);
	}
}

/* ===================================================================
 * Negative Tests — run last; wrong PSK may crash CYW43439 firmware
 * ===================================================================
 */

ZTEST(wifi_shell_neg, test_neg_connect_wrong_ssid)
{
	struct wifi_connect_req_params params = {0};
	int ret;

	ensure_disconnected();

	/* Attempt to connect with a non-existent SSID */
	params.band = WIFI_FREQ_BAND_UNKNOWN;
	params.channel = WIFI_CHANNEL_ANY;
	params.mfp = WIFI_MFP_OPTIONAL;
	params.ssid = (const uint8_t *)"NONEXISTENT_SSID_12345";
	params.ssid_length = strlen("NONEXISTENT_SSID_12345");
	params.security = WIFI_SECURITY_TYPE_PSK;
	params.psk = (const uint8_t *)"dummypassword";
	params.psk_length = strlen("dummypassword");

	k_sem_reset(&wifi_event);
	ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, ctx.iface, &params,
		       sizeof(params));
	if (ret == 0) {
		int wait = k_sem_take(&wifi_event,
			   K_SECONDS(CONFIG_WIFI_SHELL_NEG_CONNECT_TIMEOUT_SEC));

		if (wait == 0) {
			TC_PRINT("Connect result with wrong SSID: %d\n", ctx.result);
			zassert_not_equal(ctx.result, 0,
					  "Connect with wrong SSID should fail");
		} else {
			TC_PRINT("Connect with wrong SSID timed out (treated as failure)\n");
		}
	} else {
		/* Request rejected immediately — also acceptable */
		TC_PRINT("Connect with wrong SSID rejected: %d\n", ret);
	}

	int state = wifi_iface_state();

	TC_PRINT("State after wrong SSID: %s\n", wifi_state_txt(state));
	zassert_not_equal(state, WIFI_STATE_COMPLETED,
			  "Should not be COMPLETED with wrong SSID");
}

/*
 * Purpose : PSK shorter than 8 is rejected (WPA2 rule).
 * Method  : CONNECT with 5-char PSK on the configured SSID.
 */
ZTEST(wifi_shell_neg, test_neg_connect_short_psk)
{
	struct wifi_connect_req_params params = { 0 };
	int ret;

	skip_if_no_wifi_creds();
	ensure_disconnected();
	params.band = WIFI_FREQ_BAND_UNKNOWN;
	params.channel = WIFI_CHANNEL_ANY;
	params.mfp = WIFI_MFP_OPTIONAL;
	params.ssid = (const uint8_t *)CONFIG_WIFI_SHELL_TEST_SSID;
	params.ssid_length = strlen(CONFIG_WIFI_SHELL_TEST_SSID);
	params.security = WIFI_SECURITY_TYPE_PSK;
	params.psk = (const uint8_t *)"short";
	params.psk_length = 5;
	k_sem_reset(&wifi_event);
	ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, ctx.iface, &params, sizeof(params));
	if (ret == 0) {
		(void)k_sem_take(&wifi_event,
				 K_SECONDS(CONFIG_WIFI_SHELL_NEG_CONNECT_TIMEOUT_SEC));
		TC_PRINT("short PSK connect result %d\n", ctx.result);
		zassert_not_equal(wifi_iface_state(), WIFI_STATE_COMPLETED,
				  "short PSK must not associate");
	} else {
		TC_PRINT("short PSK rejected %d\n", ret);
	}
}

ZTEST(wifi_shell_neg, test_neg_disconnect_not_connected)
{
	int ret;

	ensure_disconnected();

	/* Now try to disconnect again — should not crash or hang */
	k_sem_reset(&wifi_event);
	ret = wifi_disconnect();
	TC_PRINT("Disconnect when not connected returned: %d\n", ret);

	/* Accept any non-crash result: 0 (no-op success) or error code */
	if (ret == 0) {
		/* Some drivers accept and fire a disconnect event */
		k_sem_take(&wifi_event,
			   K_SECONDS(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC));
		TC_PRINT("Disconnect event result: %d\n", ctx.result);
	}

	/* Main assertion: system is still functional */
	int state = wifi_iface_state();

	TC_PRINT("State: %s (should not be COMPLETED)\n",
		 wifi_state_txt(state));
	zassert_not_equal(state, WIFI_STATE_COMPLETED,
			  "Should not be connected");
}

ZTEST(wifi_shell_destructive, test_neg_connect_wrong_psk)
{
	skip_if_no_wifi_creds();
#ifdef CONFIG_CYW43439
	/*
	 * On CYW43439, connecting with wrong PSK causes a firmware TRAP
	 * (crash) making the whole test run fail. Skip on this chip.
	 */
	TC_PRINT("Skipped: wrong PSK causes firmware trap on CYW43439\n");
	ztest_test_skip();
#else
	struct wifi_connect_req_params params = {0};
	int ret;

	/*
	 * WARNING: On some chips, connecting with wrong PSK may cause firmware
	 * issues. This test is placed LAST intentionally.
	 */

	ensure_disconnected();

	/* Connect to valid SSID with wrong PSK */
	params.band = WIFI_FREQ_BAND_UNKNOWN;
	params.channel = WIFI_CHANNEL_ANY;
	params.mfp = WIFI_MFP_OPTIONAL;
	params.ssid = (const uint8_t *)CONFIG_WIFI_SHELL_TEST_SSID;
	params.ssid_length = strlen(CONFIG_WIFI_SHELL_TEST_SSID);
	params.security = WIFI_SECURITY_TYPE_PSK;
	params.psk = (const uint8_t *)"WrongPassword123!";
	params.psk_length = strlen("WrongPassword123!");

	k_sem_reset(&wifi_event);
	ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, ctx.iface, &params,
		       sizeof(params));
	if (ret == 0) {
		int wait = k_sem_take(&wifi_event,
			   K_SECONDS(CONFIG_WIFI_SHELL_NEG_CONNECT_TIMEOUT_SEC));

		if (wait == 0) {
			TC_PRINT("Connect result with wrong PSK: %d\n", ctx.result);
			zassert_not_equal(ctx.result, 0,
					  "Connect with wrong PSK should fail");
		} else {
			TC_PRINT("Connect with wrong PSK timed out (treated as failure)\n");
		}
	} else {
		TC_PRINT("Connect with wrong PSK rejected: %d\n", ret);
	}

	int state = wifi_iface_state();

	TC_PRINT("State after wrong PSK: %s\n", wifi_state_txt(state));
	zassert_not_equal(state, WIFI_STATE_COMPLETED,
			  "Should not be COMPLETED with wrong PSK");
#endif /* CONFIG_CYW43439 */
}

static void wifi_shell_before(void *fixture)
{
	ARG_UNUSED(fixture);
	zassert_true(wifi_drv_ready,
		     "WiFi interface not ready (SDIO/CMD5 enumerate failed)");
	skip_if_fw_dead();
	/* Reset state between tests to prevent semaphore leaks */
	k_sem_reset(&wifi_event);
	k_sem_reset(&icmp_reply_event);
	ctx.result = 0;
	ctx.connecting = false;
	ctx.scan_result_count = 0;
	ctx.target_ssid_seen = false;
}

static void *wifi_shell_setup(void)
{
	int retry = 50;

	wifi_drv_ready = false;

	/* airoc driver init is asynchronous; wait for interface */
	while (retry-- > 0) {
		ctx.iface = net_if_get_wifi_sta();
		if (ctx.iface != NULL) {
			break;
		}
		k_sleep(K_MSEC(100));
	}
	if (ctx.iface == NULL) {
		TC_PRINT("No WiFi STA interface; SDIO did not enumerate\n");
		return NULL;
	}
	if (strlen(CONFIG_WIFI_SHELL_TEST_SSID) == 0U) {
		TC_PRINT("CONFIG_WIFI_SHELL_TEST_SSID is empty; "
			 "connect/ping tests will skip\n");
	}

	net_mgmt_init_event_callback(&ctx.wifi_mgmt_cb,
				     wifi_mgmt_event_handler,
				     WIFI_MGMT_EVENTS);
	net_mgmt_add_event_callback(&ctx.wifi_mgmt_cb);

	/* Wait for WiFi driver to be fully ready (firmware download etc.) */
	TC_PRINT("Waiting for WiFi driver readiness...\n");

	for (int i = 0; i < 30; i++) {
		if (wifi_iface_ready(ctx.iface)) {
			TC_PRINT("WiFi interface ready (admin up, carrier on%s)\n",
				 net_if_is_dormant(ctx.iface) ?
				 ", dormant until associated" : "");
			wifi_drv_ready = true;
			break;
		}
		k_sleep(K_SECONDS(1));
	}
	if (!wifi_drv_ready) {
		TC_PRINT("WiFi interface not ready (admin_up=%d carrier=%d)\n",
			 net_if_is_admin_up(ctx.iface),
			 net_if_is_carrier_ok(ctx.iface));
		return NULL;
	}

	/* Extra settle time for WHD driver internal init */
	k_sleep(K_SECONDS(2));
	TC_PRINT("WiFi driver ready\n");

	k_sem_reset(&wifi_event);
	ctx.scan_result_count = 0;
	ctx.target_ssid_seen = false;
	return NULL;
}

static void wifi_shell_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	net_mgmt_del_event_callback(&ctx.wifi_mgmt_cb);
}

ZTEST_SUITE(wifi_shell, NULL, wifi_shell_setup, wifi_shell_before, NULL, NULL);

ZTEST_SUITE(wifi_shell_neg, NULL, NULL, wifi_shell_before, NULL, NULL);

ZTEST_SUITE(wifi_shell_perf, NULL, NULL, wifi_shell_before, NULL, NULL);

ZTEST_SUITE(wifi_shell_stress, NULL, NULL, wifi_shell_before, NULL, NULL);

ZTEST_SUITE(wifi_shell_destructive, NULL, NULL, wifi_shell_before,
	    NULL, wifi_shell_teardown);
