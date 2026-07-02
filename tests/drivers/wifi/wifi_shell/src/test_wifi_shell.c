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
#include <zephyr/tc_util.h>
#include <string.h>

LOG_MODULE_REGISTER(wifi_shell_test, LOG_LEVEL_INF);

K_SEM_DEFINE(wifi_event, 0, 1);

#define WIFI_MGMT_EVENTS                                                       \
	(NET_EVENT_WIFI_SCAN_DONE | NET_EVENT_WIFI_SCAN_RESULT |               \
	 NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT)

#define TEST_PING_DATA "SDIO_WiFi_shell_test"

static struct wifi_test_ctx {
	struct net_if *iface;
	uint32_t scan_result_count;
	bool connecting;
	int result;
	struct net_mgmt_event_callback wifi_mgmt_cb;
} ctx;

static void wifi_scan_result(struct net_mgmt_event_callback *cb)
{
	const struct wifi_scan_result *entry =
		(const struct wifi_scan_result *)cb->info;

	ctx.scan_result_count++;

	if (ctx.scan_result_count == 1U) {
		TC_PRINT("%-4s | %-32s | %-4s | %-4s | %-15s\n",
			 "Num", "SSID", "Chan", "RSSI", "Security");
	}

	TC_PRINT("%-4d | %-32s | %-4u | %-4d | %-15s\n",
		 ctx.scan_result_count, entry->ssid, entry->channel,
		 entry->rssi, wifi_security_txt(entry->security));
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

	net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, ctx.iface, &status,
		 sizeof(status));
	return status.state;
}

/* ICMP echo reply handler */
static int icmp_reply_handler(struct net_icmp_ctx *icctx,
			      struct net_pkt *pkt,
			      struct net_icmp_ip_hdr *hdr,
			      struct net_icmp_hdr *icmp_hdr,
			      void *user_data)
{
	struct net_ipv4_hdr *ip_hdr = hdr->ipv4;

	ARG_UNUSED(icctx);
	ARG_UNUSED(pkt);
	ARG_UNUSED(icmp_hdr);
	ARG_UNUSED(user_data);

	char addr_str[NET_IPV4_ADDR_LEN];

	net_addr_ntop(AF_INET, &ip_hdr->src, addr_str, sizeof(addr_str));
	TC_PRINT("ICMP reply from %s\n", addr_str);

	ctx.result = 0;
	k_sem_give(&wifi_event);
	return 0;
}

static void ensure_connected(void)
{
	if (wifi_iface_state() == WIFI_STATE_COMPLETED) {
		return;
	}
	ctx.connecting = true;
	k_sem_reset(&wifi_event);
	int ret = wifi_connect();

	if (ret == 0) {
		k_sem_take(&wifi_event,
			   K_SECONDS(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC));
	}
	ctx.connecting = false;
}

static void ensure_disconnected(void)
{
	if (wifi_iface_state() != WIFI_STATE_COMPLETED) {
		return;
	}
	k_sem_reset(&wifi_event);
	wifi_disconnect();
	k_sem_take(&wifi_event, K_SECONDS(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC));
	k_sleep(K_SECONDS(1));
}

ZTEST(wifi_shell, test_wifi_scan)
{
	int ret;
	int scan_retry = 10;

	ctx.scan_result_count = 0;

	/* Retry scan — handle EAGAIN (not ready) and -119 (scan in progress) */
	do {
		k_sem_reset(&wifi_event);
		ret = wifi_scan();
		if (ret == -EAGAIN) {
			TC_PRINT("Scan busy (EAGAIN), retrying...\n");
			k_sleep(K_SECONDS(2));
			continue;
		}
		if (ret == -119) {
			/* WHD internal scan in progress — wait for it to finish */
			TC_PRINT("Scan in progress, waiting...\n");
			k_sem_take(&wifi_event, K_SECONDS(30));
			k_sleep(K_SECONDS(1));
			continue;
		}
		break;
	} while (--scan_retry > 0);

	zassert_equal(ret, 0, "Scan request failed after retries");

	ret = k_sem_take(&wifi_event, K_SECONDS(CONFIG_WIFI_SHELL_SCAN_TIMEOUT_SEC));
	zassert_equal(ret, 0, "WiFi scan timed out");

	TC_PRINT("Scan done, %u AP(s) found\n", ctx.scan_result_count);
	zassert_true(ctx.scan_result_count > 0, "No APs found during scan");
}

ZTEST(wifi_shell, test_wifi_connect)
{
	int ret;
	int retry = CONFIG_WIFI_SHELL_CONNECT_ATTEMPTS;

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

		ret = k_sem_take(&wifi_event,
				 K_SECONDS(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC));
		if (ret) {
			TC_PRINT("Timeout, retry %d\n",
				 CONFIG_WIFI_SHELL_CONNECT_ATTEMPTS - retry + 1);
			k_sleep(K_SECONDS(1));
		} else if (ctx.result) {
			TC_PRINT("Failed (%d), retry %d\n", ctx.result,
				 CONFIG_WIFI_SHELL_CONNECT_ATTEMPTS - retry + 1);
			k_sleep(K_SECONDS(1));
		} else {
			break;
		}
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

	TC_PRINT("SSID: %s\n", status.ssid);
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
	params.data_size = sizeof(TEST_PING_DATA);

	char gw_str[NET_IPV4_ADDR_LEN];

	net_addr_ntop(AF_INET, &gw_addr, gw_str, sizeof(gw_str));
	TC_PRINT("Pinging gateway %s...\n", gw_str);

	do {
		ret = net_icmp_send_echo_request(&icmp_ctx, ctx.iface,
						 (struct sockaddr *)&dst4,
						 &params, NULL);
		zassert_equal(ret, 0, "ICMP send failed (%d)", ret);

		ret = k_sem_take(&wifi_event,
				 K_SECONDS(CONFIG_WIFI_SHELL_PING_TIMEOUT_SEC));
		if (ret) {
			zassert(--retry, "Ping timed out on all attempts");
			TC_PRINT("No reply, retry %d\n",
				 CONFIG_WIFI_SHELL_PING_ATTEMPTS - retry);
		} else {
			break;
		}
	} while (retry);

	zassert_equal(ctx.result, 0, "ICMP payload mismatch");

	net_icmp_cleanup_ctx(&icmp_ctx);
}

ZTEST(wifi_shell, test_wifi_disconnect)
{
	int ret;

	ensure_connected();
	k_sem_reset(&wifi_event);
	ret = wifi_disconnect();
	zassert_equal(ret, 0, "Disconnect request failed");

	ret = k_sem_take(&wifi_event,
			 K_SECONDS(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC));
	zassert_equal(ret, 0, "WiFi disconnect timed out");
	zassert_equal(ctx.result, 0, "Disconnect failed");
}

ZTEST(wifi_shell, test_wifi_reconnect)
{
	int ret;
	int retry = CONFIG_WIFI_SHELL_CONNECT_ATTEMPTS;

	ensure_disconnected();
	ctx.connecting = true;

	do {
		k_sem_reset(&wifi_event);
		ret = wifi_connect();
		if (ret == -EALREADY || ret == -120) {
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

		ret = k_sem_take(&wifi_event,
				 K_SECONDS(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC));
		if (ret) {
			TC_PRINT("Reconnect timeout, retry...\n");
			k_sleep(K_SECONDS(1));
		} else if (ctx.result) {
			TC_PRINT("Reconnect failed (%d), retry...\n", ctx.result);
			k_sleep(K_SECONDS(1));
		} else {
			break;
		}
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

ZTEST(wifi_shell, test_stress_repeated_scan)
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

		ret = k_sem_take(&wifi_event,
				 K_SECONDS(CONFIG_WIFI_SHELL_SCAN_TIMEOUT_SEC));
		zassert_equal(ret, 0, "Scan %d timed out", i + 1);

		TC_PRINT("  Scan %d: %u AP(s) found\n",
			 i + 1, ctx.scan_result_count);
		zassert_true(ctx.scan_result_count > 0,
			     "Scan %d found no APs", i + 1);

		/* Brief pause between scans */
		k_sleep(K_SECONDS(1));
	}

	TC_PRINT("All %d scans completed successfully\n", iterations);
}

ZTEST(wifi_shell, test_stress_connect_disconnect)
{
	int ret;
	int iterations = CONFIG_WIFI_SHELL_STRESS_CONN_ITERATIONS;
	int retry;

	TC_PRINT("Running %d connect/disconnect cycles...\n", iterations);

	/* Ensure disconnected before starting */
	k_sem_reset(&wifi_event);
	wifi_disconnect();
	k_sem_take(&wifi_event, K_SECONDS(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC));
	k_sleep(K_SECONDS(1));

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
			if (ret == 0) {
				ret = k_sem_take(&wifi_event,
					K_SECONDS(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC));
				if (ret == 0 && ctx.result == 0) {
					break;
				}
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

		ret = k_sem_take(&wifi_event,
				 K_SECONDS(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC));
		zassert_equal(ret, 0, "Cycle %d: disconnect timed out",
			      i + 1);

		TC_PRINT("  Cycle %d/%d: OK\n", i + 1, iterations);
		k_sleep(K_SECONDS(1));
	}

	TC_PRINT("All %d connect/disconnect cycles completed\n", iterations);
}

/* ===================================================================
 * Performance Tests
 * ===================================================================
 */

ZTEST(wifi_shell, test_perf_scan_timing)
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

		ret = k_sem_take(&wifi_event,
				 K_SECONDS(CONFIG_WIFI_SHELL_SCAN_TIMEOUT_SEC));
		end_ms = k_uptime_get_32();
		elapsed_ms = end_ms - start_ms;

		zassert_equal(ret, 0, "Scan %d timed out", i + 1);
		TC_PRINT("  Scan %d: %u ms, %u APs\n",
			 i + 1, elapsed_ms, ctx.scan_result_count);

		total_ms += elapsed_ms;
		k_sleep(K_SECONDS(1));
	}

	TC_PRINT("Average scan time: %u ms\n", total_ms / count);
}

ZTEST(wifi_shell, test_perf_connect_timing)
{
	int ret;
	uint32_t start_ms, end_ms, elapsed_ms;

	/* Ensure disconnected */
	k_sem_reset(&wifi_event);
	wifi_disconnect();
	k_sem_take(&wifi_event, K_SECONDS(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC));
	k_sleep(K_SECONDS(1));

	TC_PRINT("Benchmarking connect time...\n");

	ctx.connecting = true;
	k_sem_reset(&wifi_event);

	start_ms = k_uptime_get_32();

	/* Connect with retry for EAGAIN only */
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

	ret = k_sem_take(&wifi_event,
			 K_SECONDS(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC));
	end_ms = k_uptime_get_32();
	elapsed_ms = end_ms - start_ms;

	ctx.connecting = false;

	zassert_equal(ret, 0, "Connect timed out");
	zassert_equal(ctx.result, 0, "Connect failed: %d", ctx.result);

	int state = wifi_iface_state();

	zassert_equal(state, WIFI_STATE_COMPLETED,
		      "Not in COMPLETED state");

	TC_PRINT("Connect time: %u ms\n", elapsed_ms);

	struct wifi_iface_status status = {0};

	net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, ctx.iface, &status,
		 sizeof(status));
	TC_PRINT("Connected to %s, Ch %u, RSSI %d dBm\n",
		 status.ssid, status.channel, status.rssi);
}

ZTEST(wifi_shell, test_perf_ping_latency)
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

	/* Ensure connected */
	int state = wifi_iface_state();

	if (state != WIFI_STATE_COMPLETED) {
		TC_PRINT("Not connected, connecting first...\n");
		ctx.connecting = true;
		k_sem_reset(&wifi_event);
		ret = wifi_connect();
		if (ret == 0) {
			k_sem_take(&wifi_event,
				   K_SECONDS(CONFIG_WIFI_SHELL_CONNECT_TIMEOUT_SEC));
		}
		ctx.connecting = false;
	}

	gw_addr = net_if_ipv4_get_gw(ctx.iface);
	if (gw_addr.s_addr == 0) {
		TC_PRINT("No gateway — skipping ping latency test\n");
		ztest_test_skip();
		return;
	}

	ret = net_icmp_init_ctx(&icmp_ctx, NET_ICMPV4_ECHO_REPLY, 0,
				icmp_reply_handler);
	zassert_equal(ret, 0, "ICMP init failed");

	dst4.sin_family = AF_INET;
	memcpy(&dst4.sin_addr, &gw_addr, sizeof(gw_addr));

	params.identifier = 0xBEEF;
	params.data = TEST_PING_DATA;
	params.data_size = sizeof(TEST_PING_DATA);

	char gw_str[NET_IPV4_ADDR_LEN];

	net_addr_ntop(AF_INET, &gw_addr, gw_str, sizeof(gw_str));
	TC_PRINT("Ping latency to %s (%d pings)...\n", gw_str, count);

	for (int i = 0; i < count; i++) {
		k_sem_reset(&wifi_event);
		params.sequence = i + 1;

		start_ms = k_uptime_get_32();
		ret = net_icmp_send_echo_request(&icmp_ctx, ctx.iface,
						  (struct sockaddr *)&dst4,
						  &params, NULL);
		if (ret != 0) {
			TC_PRINT("  Ping %d: send failed (%d)\n", i + 1, ret);
			continue;
		}

		ret = k_sem_take(&wifi_event,
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

	zassert_true(success >= (count / 2),
		     "Too many ping failures: %d/%d", success, count);
}

/* ===================================================================
 * Negative Tests — run last; wrong PSK may crash CYW43439 firmware
 * ===================================================================
 */

ZTEST(wifi_shell, test_neg_connect_wrong_ssid)
{
	struct wifi_connect_req_params params = {0};
	int ret;

	/* First ensure we are disconnected */
	k_sem_reset(&wifi_event);
	wifi_disconnect();
	k_sem_take(&wifi_event, K_SECONDS(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC));
	k_sleep(K_SECONDS(1));

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
		/* Request accepted — wait for connect result (should fail) */
		k_sem_take(&wifi_event,
			   K_SECONDS(CONFIG_WIFI_SHELL_NEG_CONNECT_TIMEOUT_SEC));
		TC_PRINT("Connect result with wrong SSID: %d\n", ctx.result);
		zassert_not_equal(ctx.result, 0,
				  "Connect with wrong SSID should fail");
	} else {
		/* Request rejected immediately — also acceptable */
		TC_PRINT("Connect with wrong SSID rejected: %d\n", ret);
	}

	int state = wifi_iface_state();

	TC_PRINT("State after wrong SSID: %s\n", wifi_state_txt(state));
	zassert_not_equal(state, WIFI_STATE_COMPLETED,
			  "Should not be COMPLETED with wrong SSID");
}

ZTEST(wifi_shell, test_neg_disconnect_not_connected)
{
	int ret;

	/* Ensure disconnected first */
	k_sem_reset(&wifi_event);
	wifi_disconnect();
	k_sem_take(&wifi_event, K_SECONDS(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC));
	k_sleep(K_SECONDS(1));

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
	struct wifi_connect_req_params params = {0};
	int ret;

	/*
	 * WARNING: On CYW43439, connecting with wrong PSK may cause a firmware
	 * TRAP (crash). This test is placed LAST intentionally because the
	 * firmware may become non-functional after this test.
	 */

	/* Ensure disconnected */
	k_sem_reset(&wifi_event);
	wifi_disconnect();
	k_sem_take(&wifi_event, K_SECONDS(CONFIG_WIFI_SHELL_DISCONNECT_TIMEOUT_SEC));
	k_sleep(K_SECONDS(1));

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
		k_sem_take(&wifi_event,
			   K_SECONDS(CONFIG_WIFI_SHELL_NEG_CONNECT_TIMEOUT_SEC));
		TC_PRINT("Connect result with wrong PSK: %d\n", ctx.result);
		zassert_not_equal(ctx.result, 0,
				  "Connect with wrong PSK should fail");
	} else {
		TC_PRINT("Connect with wrong PSK rejected: %d\n", ret);
	}

	int state = wifi_iface_state();

	TC_PRINT("State after wrong PSK: %s\n", wifi_state_txt(state));
	zassert_not_equal(state, WIFI_STATE_COMPLETED,
			  "Should not be COMPLETED with wrong PSK");
}

static void wifi_shell_before(void *fixture)
{
	ARG_UNUSED(fixture);
	/* Reset state between tests to prevent semaphore leaks */
	k_sem_reset(&wifi_event);
	ctx.result = 0;
}

static void *wifi_shell_setup(void)
{
	int retry = 50;

	/* airoc driver init is asynchronous; wait for interface */
	while (retry-- > 0) {
		ctx.iface = net_if_get_wifi_sta();
		if (ctx.iface != NULL) {
			break;
		}
		k_sleep(K_MSEC(100));
	}
	zassert_not_null(ctx.iface, "No WiFi STA interface found");

	net_mgmt_init_event_callback(&ctx.wifi_mgmt_cb,
				     wifi_mgmt_event_handler,
				     WIFI_MGMT_EVENTS);
	net_mgmt_add_event_callback(&ctx.wifi_mgmt_cb);

	/* Wait for WiFi driver to be fully ready (firmware download etc.) */
	TC_PRINT("Waiting for WiFi driver readiness...\n");
	for (int i = 0; i < 30; i++) {
		if (net_if_is_up(ctx.iface)) {
			TC_PRINT("WiFi interface is up\n");
			break;
		}
		k_sleep(K_SECONDS(1));
	}
	/* Extra settle time for WHD driver internal init */
	k_sleep(K_SECONDS(2));
	TC_PRINT("WiFi driver ready\n");

	k_sem_reset(&wifi_event);
	ctx.scan_result_count = 0;
	return NULL;
}

static void wifi_shell_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	net_mgmt_del_event_callback(&ctx.wifi_mgmt_cb);
}

ZTEST_SUITE(wifi_shell, NULL, wifi_shell_setup, wifi_shell_before, NULL,
	    wifi_shell_teardown);

ZTEST_SUITE(wifi_shell_destructive, NULL, NULL, wifi_shell_before, NULL, NULL);
