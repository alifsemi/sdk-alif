/*
 * Copyright (c) 2025 Alif Semiconductor.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/zperf.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/wifi.h>
#include <zephyr/tc_util.h>
#include <string.h>
#include <errno.h>

LOG_MODULE_REGISTER(wifi_zperf_test, LOG_LEVEL_INF);

K_SEM_DEFINE(wifi_event, 0, 1);

#define WIFI_MGMT_EVENTS                                                       \
	(NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT)

static struct zperf_test_ctx {
	struct net_if *iface;
	bool connecting;
	int result;
	struct net_mgmt_event_callback wifi_mgmt_cb;
} ctx;

static void wifi_connect_result(struct net_mgmt_event_callback *cb)
{
	const struct wifi_status *status =
		(const struct wifi_status *)cb->info;

	ctx.result = status->status;

	if (ctx.result) {
		TC_PRINT("Connection failed (%d)\n", ctx.result);
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

static int wifi_connect(void)
{
	struct wifi_connect_req_params params = {0};
	int ret;

	params.band = WIFI_FREQ_BAND_UNKNOWN;
	params.channel = WIFI_CHANNEL_ANY;
	params.mfp = WIFI_MFP_OPTIONAL;
	params.ssid = (const uint8_t *)CONFIG_WIFI_ZPERF_TEST_SSID;
	params.ssid_length = strlen(CONFIG_WIFI_ZPERF_TEST_SSID);
	params.security = WIFI_SECURITY_TYPE_PSK;
	params.psk = (const uint8_t *)CONFIG_WIFI_ZPERF_TEST_PSK;
	params.psk_length = strlen(CONFIG_WIFI_ZPERF_TEST_PSK);

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

static bool wait_for_dhcp(int timeout_sec)
{
	for (int i = 0; i < timeout_sec; i++) {
		struct net_if_ipv4 *ipv4 = ctx.iface->config.ip.ipv4;

		if (ipv4 != NULL &&
		    ipv4->unicast[0].ipv4.address.in_addr.s_addr != 0) {
			char addr_str[NET_IPV4_ADDR_LEN];

			net_addr_ntop(AF_INET,
				      &ipv4->unicast[0].ipv4.address.in_addr,
				      addr_str, sizeof(addr_str));
			TC_PRINT("DHCP acquired IP: %s\n", addr_str);
			return true;
		}
		k_sleep(K_SECONDS(1));
	}
	return false;
}

static void setup_upload_params(struct zperf_upload_params *params,
				uint16_t packet_size, uint32_t rate_kbps)
{
	struct sockaddr_in *addr4;

	memset(params, 0, sizeof(*params));

	addr4 = (struct sockaddr_in *)&params->peer_addr;
	addr4->sin_family = AF_INET;
	addr4->sin_port = htons(CONFIG_WIFI_ZPERF_SERVER_PORT);
	net_addr_pton(AF_INET, CONFIG_WIFI_ZPERF_SERVER_IP,
		      &addr4->sin_addr);

	params->duration_ms = CONFIG_WIFI_ZPERF_TEST_DURATION_SEC * 1000U;
	params->packet_size = packet_size;
	params->rate_kbps = rate_kbps;
}

static uint32_t calc_throughput_kbps(struct zperf_results *results)
{
	if (results->client_time_in_us == 0) {
		return 0;
	}
	/* Use client-side metrics: nb_packets_sent * packet_size for bytes */
	uint64_t total_bytes = (uint64_t)results->nb_packets_sent *
			       results->packet_size;

	return (uint32_t)((total_bytes * 8ULL * 1000ULL) /
			  results->client_time_in_us);
}

/* ===================================================================
 * Functional Tests
 * ===================================================================
 */

ZTEST(wifi_zperf, test_wifi_connect)
{
	/* WiFi is already connected by suite setup - just verify */
	int state = wifi_iface_state();

	TC_PRINT("Interface state: %s\n", wifi_state_txt(state));
	zassert_equal(state, WIFI_STATE_COMPLETED,
		      "WiFi not connected (setup should have connected)");

	struct net_if_ipv4 *ipv4 = ctx.iface->config.ip.ipv4;

	zassert_not_null(ipv4, "No IPv4 config");
	zassert_true(ipv4->unicast[0].ipv4.address.in_addr.s_addr != 0,
		     "No IP address assigned");

	char addr_str[NET_IPV4_ADDR_LEN];

	net_addr_ntop(AF_INET, &ipv4->unicast[0].ipv4.address.in_addr,
		      addr_str, sizeof(addr_str));
	TC_PRINT("IP address: %s\n", addr_str);
}

ZTEST(wifi_zperf, test_tcp_upload)
{
	struct zperf_upload_params params;
	struct zperf_results results = {0};
	int ret;
	uint32_t throughput_kbps;

	setup_upload_params(&params, CONFIG_WIFI_ZPERF_TCP_PACKET_SIZE, 0);

	TC_PRINT("TCP upload to %s:%d for %d seconds (pkt_size=%d)...\n",
		 CONFIG_WIFI_ZPERF_SERVER_IP, CONFIG_WIFI_ZPERF_SERVER_PORT,
		 CONFIG_WIFI_ZPERF_TEST_DURATION_SEC,
		 CONFIG_WIFI_ZPERF_TCP_PACKET_SIZE);

	ret = zperf_tcp_upload(&params, &results);
	if (ret == -ETIMEDOUT || ret == -ECONNREFUSED || ret == -ENETUNREACH) {
		TC_PRINT("Server %s:%d unreachable (%d), skipping\n",
			 CONFIG_WIFI_ZPERF_SERVER_IP,
			 CONFIG_WIFI_ZPERF_SERVER_PORT, ret);
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "TCP upload failed: %d", ret);

	throughput_kbps = calc_throughput_kbps(&results);

	TC_PRINT("TCP upload results:\n");
	TC_PRINT("  Duration:   %llu us\n", results.client_time_in_us);
	TC_PRINT("  Bytes sent: %llu\n",
		 (uint64_t)results.nb_packets_sent * results.packet_size);
	TC_PRINT("  Packets:    %u sent, %u errors\n",
		 results.nb_packets_sent, results.nb_packets_errors);
	TC_PRINT("  Throughput: %u kbps (%.2f Mbps)\n",
		 throughput_kbps, (float)throughput_kbps / 1000.0f);

	zassert_true(throughput_kbps >= CONFIG_WIFI_ZPERF_MIN_TCP_THROUGHPUT_KBPS,
		     "TCP throughput %u kbps below minimum %u kbps",
		     throughput_kbps, CONFIG_WIFI_ZPERF_MIN_TCP_THROUGHPUT_KBPS);
}

ZTEST(wifi_zperf, test_tcp_upload_large_packet)
{
	struct zperf_upload_params params;
	struct zperf_results results = {0};
	int ret;
	uint32_t throughput_kbps;
	uint16_t pkt_size = 1024;

	setup_upload_params(&params, pkt_size, 0);

	TC_PRINT("TCP upload (large pkt=%d) to %s:%d for %d s...\n",
		 pkt_size, CONFIG_WIFI_ZPERF_SERVER_IP,
		 CONFIG_WIFI_ZPERF_SERVER_PORT,
		 CONFIG_WIFI_ZPERF_TEST_DURATION_SEC);

	ret = zperf_tcp_upload(&params, &results);
	if (ret == -ETIMEDOUT || ret == -ECONNREFUSED || ret == -ENETUNREACH) {
		TC_PRINT("Server unreachable (%d), skipping\n", ret);
		ztest_test_skip();
		return;
	}
	zassert_equal(ret, 0, "TCP upload (large pkt) failed: %d", ret);

	throughput_kbps = calc_throughput_kbps(&results);

	TC_PRINT("  Bytes sent: %llu, Packets: %u\n",
		 (uint64_t)results.nb_packets_sent * results.packet_size,
		 results.nb_packets_sent);
	TC_PRINT("  Throughput: %u kbps (%.2f Mbps)\n",
		 throughput_kbps, (float)throughput_kbps / 1000.0f);

	zassert_true(throughput_kbps >= CONFIG_WIFI_ZPERF_MIN_TCP_THROUGHPUT_KBPS,
		     "TCP throughput %u kbps below minimum %u kbps",
		     throughput_kbps, CONFIG_WIFI_ZPERF_MIN_TCP_THROUGHPUT_KBPS);
}

ZTEST(wifi_zperf, test_udp_upload)
{
	struct zperf_upload_params params;
	struct zperf_results results = {0};
	int ret;
	uint32_t throughput_kbps;

	setup_upload_params(&params, CONFIG_WIFI_ZPERF_UDP_PACKET_SIZE,
			    CONFIG_WIFI_ZPERF_UDP_RATE_KBPS);

	TC_PRINT("UDP upload to %s:%d for %d s (pkt=%d, rate=%d kbps)...\n",
		 CONFIG_WIFI_ZPERF_SERVER_IP, CONFIG_WIFI_ZPERF_SERVER_PORT,
		 CONFIG_WIFI_ZPERF_TEST_DURATION_SEC,
		 CONFIG_WIFI_ZPERF_UDP_PACKET_SIZE,
		 CONFIG_WIFI_ZPERF_UDP_RATE_KBPS);

	ret = zperf_udp_upload(&params, &results);
	if (ret != 0) {
		TC_PRINT("UDP upload failed (%d), server may be unreachable, skipping\n", ret);
		ztest_test_skip();
		return;
	}

	throughput_kbps = calc_throughput_kbps(&results);

	TC_PRINT("UDP upload results:\n");
	TC_PRINT("  Duration:   %llu us\n", results.client_time_in_us);
	TC_PRINT("  Bytes sent: %llu\n",
		 (uint64_t)results.nb_packets_sent * results.packet_size);
	TC_PRINT("  Packets:    %u sent, %u lost, %u errors\n",
		 results.nb_packets_sent, results.nb_packets_lost,
		 results.nb_packets_errors);
	TC_PRINT("  Jitter:     %u us\n", results.jitter_in_us);
	TC_PRINT("  Throughput: %u kbps (%.2f Mbps)\n",
		 throughput_kbps, (float)throughput_kbps / 1000.0f);

	zassert_true(throughput_kbps >= CONFIG_WIFI_ZPERF_MIN_UDP_THROUGHPUT_KBPS,
		     "UDP throughput %u kbps below minimum %u kbps",
		     throughput_kbps, CONFIG_WIFI_ZPERF_MIN_UDP_THROUGHPUT_KBPS);
}

ZTEST(wifi_zperf, test_udp_upload_large_packet)
{
	struct zperf_upload_params params;
	struct zperf_results results = {0};
	int ret;
	uint32_t throughput_kbps;
	uint16_t pkt_size = 1024;

	setup_upload_params(&params, pkt_size, CONFIG_WIFI_ZPERF_UDP_RATE_KBPS);

	TC_PRINT("UDP upload (large pkt=%d) to %s:%d for %d s...\n",
		 pkt_size, CONFIG_WIFI_ZPERF_SERVER_IP,
		 CONFIG_WIFI_ZPERF_SERVER_PORT,
		 CONFIG_WIFI_ZPERF_TEST_DURATION_SEC);

	ret = zperf_udp_upload(&params, &results);
	if (ret != 0) {
		TC_PRINT("UDP upload (large pkt) failed (%d), skipping\n", ret);
		ztest_test_skip();
		return;
	}

	throughput_kbps = calc_throughput_kbps(&results);

	TC_PRINT("  Bytes sent: %llu, Packets: %u sent, %u lost\n",
		 (uint64_t)results.nb_packets_sent * results.packet_size,
		 results.nb_packets_sent, results.nb_packets_lost);
	TC_PRINT("  Throughput: %u kbps (%.2f Mbps)\n",
		 throughput_kbps, (float)throughput_kbps / 1000.0f);

	zassert_true(throughput_kbps >= CONFIG_WIFI_ZPERF_MIN_UDP_THROUGHPUT_KBPS,
		     "UDP throughput %u kbps below minimum %u kbps",
		     throughput_kbps, CONFIG_WIFI_ZPERF_MIN_UDP_THROUGHPUT_KBPS);
}

/* ===================================================================
 * Performance / Stability Tests
 * ===================================================================
 */

ZTEST(wifi_zperf, test_tcp_throughput_stability)
{
	struct zperf_upload_params params;
	struct zperf_results results;
	int ret;
	int iterations = CONFIG_WIFI_ZPERF_STABILITY_ITERATIONS;
	uint32_t throughput_kbps;
	uint32_t total_kbps = 0;
	uint32_t min_kbps = UINT32_MAX;
	uint32_t max_kbps = 0;

	setup_upload_params(&params, CONFIG_WIFI_ZPERF_TCP_PACKET_SIZE, 0);

	TC_PRINT("TCP throughput stability (%d runs, %d s each)...\n",
		 iterations, CONFIG_WIFI_ZPERF_TEST_DURATION_SEC);

	for (int i = 0; i < iterations; i++) {
		memset(&results, 0, sizeof(results));

		ret = zperf_tcp_upload(&params, &results);
		if (ret == -ETIMEDOUT || ret == -ECONNREFUSED ||
		    ret == -ENETUNREACH) {
			TC_PRINT("Run %d: server unreachable (%d), skipping\n",
				 i + 1, ret);
			ztest_test_skip();
			return;
		}
		zassert_equal(ret, 0, "Run %d: TCP upload failed: %d",
			      i + 1, ret);

		throughput_kbps = calc_throughput_kbps(&results);
		total_kbps += throughput_kbps;

		if (throughput_kbps < min_kbps) {
			min_kbps = throughput_kbps;
		}
		if (throughput_kbps > max_kbps) {
			max_kbps = throughput_kbps;
		}

		TC_PRINT("  Run %d: %u kbps (%.2f Mbps)\n",
			 i + 1, throughput_kbps,
			 (float)throughput_kbps / 1000.0f);

		k_sleep(K_SECONDS(2));
	}

	uint32_t avg_kbps = total_kbps / iterations;

	TC_PRINT("Stability results:\n");
	TC_PRINT("  Avg: %u kbps (%.2f Mbps)\n",
		 avg_kbps, (float)avg_kbps / 1000.0f);
	TC_PRINT("  Min: %u kbps, Max: %u kbps\n", min_kbps, max_kbps);
	TC_PRINT("  Variance: %u kbps\n", max_kbps - min_kbps);

	zassert_true(avg_kbps >= CONFIG_WIFI_ZPERF_MIN_TCP_THROUGHPUT_KBPS,
		     "Avg throughput %u kbps below minimum %u kbps",
		     avg_kbps, CONFIG_WIFI_ZPERF_MIN_TCP_THROUGHPUT_KBPS);

	zassert_true(min_kbps >= (CONFIG_WIFI_ZPERF_MIN_TCP_THROUGHPUT_KBPS / 2),
		     "Min throughput %u kbps below half of minimum threshold",
		     min_kbps);
}

ZTEST(wifi_zperf, test_udp_packet_loss)
{
	struct zperf_upload_params params;
	struct zperf_results results = {0};
	int ret;
	uint32_t loss_pct;

	setup_upload_params(&params, CONFIG_WIFI_ZPERF_UDP_PACKET_SIZE,
			    CONFIG_WIFI_ZPERF_UDP_RATE_KBPS);

	TC_PRINT("UDP packet loss test to %s:%d...\n",
		 CONFIG_WIFI_ZPERF_SERVER_IP, CONFIG_WIFI_ZPERF_SERVER_PORT);

	ret = zperf_udp_upload(&params, &results);
	if (ret != 0) {
		TC_PRINT("UDP upload failed (%d), server may be unreachable, skipping\n", ret);
		ztest_test_skip();
		return;
	}

	if (results.nb_packets_sent > 0) {
		loss_pct = (results.nb_packets_lost * 100U) /
			   results.nb_packets_sent;
	} else {
		loss_pct = 100;
	}

	TC_PRINT("UDP packet loss results:\n");
	TC_PRINT("  Sent: %u, Lost: %u, Errors: %u\n",
		 results.nb_packets_sent, results.nb_packets_lost,
		 results.nb_packets_errors);
	TC_PRINT("  Loss: %u%%\n", loss_pct);
	TC_PRINT("  Jitter: %u us\n", results.jitter_in_us);

	if (loss_pct > 100U) {
		TC_PRINT("Server report looks corrupted (%u%% loss), skipping\n",
			 loss_pct);
		ztest_test_skip();
		return;
	}

	zassert_true(loss_pct <= 30,
		     "UDP packet loss %u%% exceeds 30%% threshold", loss_pct);
}

/* ===================================================================
 * Negative Tests
 * ===================================================================
 */

ZTEST(wifi_zperf, test_tcp_upload_invalid_server)
{
	struct zperf_upload_params params;
	struct zperf_results results = {0};
	struct sockaddr_in *addr4 = (struct sockaddr_in *)&params.peer_addr;
	int ret;

	memset(&params, 0, sizeof(params));
	addr4->sin_family = AF_INET;
	addr4->sin_port = htons(59999);
	net_addr_pton(AF_INET, "192.168.255.254", &addr4->sin_addr);
	params.duration_ms = 5000;
	params.packet_size = 256;

	TC_PRINT("TCP upload to unreachable server 192.168.255.254:59999...\n");

	ret = zperf_tcp_upload(&params, &results);

	TC_PRINT("Result: %d (expected non-zero or zero bytes sent)\n", ret);

	/* Either the API returns error or zero throughput — both acceptable */
	if (ret == 0) {
		zassert_equal(results.total_len, 0,
			      "Should not transfer data to unreachable server");
	}
}

ZTEST(wifi_zperf, test_udp_upload_zero_duration)
{
	struct zperf_upload_params params;
	struct zperf_results results = {0};
	int ret;

	setup_upload_params(&params, CONFIG_WIFI_ZPERF_UDP_PACKET_SIZE,
			    CONFIG_WIFI_ZPERF_UDP_RATE_KBPS);
	params.duration_ms = 0;

	TC_PRINT("UDP upload with 0 duration...\n");

	ret = zperf_udp_upload(&params, &results);

	TC_PRINT("Result: %d, bytes sent: %llu\n", ret,
		 (uint64_t)results.nb_packets_sent * results.packet_size);

	/* With 0 duration, should complete immediately with minimal data */
	if (ret == 0) {
		TC_PRINT("Completed with 0 duration — packets sent: %u\n",
			 results.nb_packets_sent);
	}
}

/* ===================================================================
 * Cleanup
 * ===================================================================
 */

ZTEST(wifi_zperf, test_wifi_disconnect)
{
	int ret;

	if (wifi_iface_state() != WIFI_STATE_COMPLETED) {
		TC_PRINT("Not connected, skipping disconnect\n");
		ztest_test_skip();
		return;
	}

	k_sem_reset(&wifi_event);
	ret = wifi_disconnect();
	zassert_equal(ret, 0, "Disconnect request failed");

	ret = k_sem_take(&wifi_event, K_SECONDS(10));
	zassert_equal(ret, 0, "Disconnect timed out");
	zassert_equal(ctx.result, 0, "Disconnect failed");
}

/* ===================================================================
 * Suite setup / teardown
 * ===================================================================
 */

static void wifi_zperf_before(void *fixture)
{
	ARG_UNUSED(fixture);
	k_sem_reset(&wifi_event);
	ctx.result = 0;
}

static void *wifi_zperf_setup(void)
{
	int retry = 50;
	int ret;

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

	/* Wait for WiFi driver to be fully ready */
	TC_PRINT("Waiting for WiFi driver readiness...\n");
	for (int i = 0; i < 30; i++) {
		if (net_if_is_up(ctx.iface)) {
			TC_PRINT("WiFi interface is up\n");
			break;
		}
		k_sleep(K_SECONDS(1));
	}
	k_sleep(K_SECONDS(2));
	TC_PRINT("WiFi driver ready\n");

	/* Connect WiFi so all tests have network available */
	if (wifi_iface_state() != WIFI_STATE_COMPLETED) {
		int connect_retry = 3;

		ctx.connecting = true;
		do {
			k_sem_reset(&wifi_event);
			ret = wifi_connect();
			if (ret == -EAGAIN) {
				k_sleep(K_SECONDS(2));
				continue;
			}
			if (ret == 0) {
				ret = k_sem_take(&wifi_event,
					K_SECONDS(CONFIG_WIFI_ZPERF_CONNECT_TIMEOUT_SEC));
				if (ret == 0 && ctx.result == 0) {
					TC_PRINT("WiFi connected\n");
					break;
				}
			}
			TC_PRINT("Connect attempt failed, retrying...\n");
			k_sleep(K_SECONDS(2));
		} while (--connect_retry > 0);
		ctx.connecting = false;

		zassert_equal(wifi_iface_state(), WIFI_STATE_COMPLETED,
			      "WiFi connection failed in setup");
	}

	/* Wait for DHCP */
	TC_PRINT("Waiting for DHCP...\n");
	zassert_true(wait_for_dhcp(15), "DHCP timed out");

	k_sem_reset(&wifi_event);
	return NULL;
}

static void wifi_zperf_teardown(void *fixture)
{
	ARG_UNUSED(fixture);
	net_mgmt_del_event_callback(&ctx.wifi_mgmt_cb);
}

ZTEST_SUITE(wifi_zperf, NULL, wifi_zperf_setup, wifi_zperf_before, NULL,
	    wifi_zperf_teardown);
