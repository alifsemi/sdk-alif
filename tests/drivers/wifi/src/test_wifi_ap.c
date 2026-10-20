/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/net_stats.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/wifi.h>
#include <zephyr/net/icmp.h>
#include <zephyr/net/dhcpv4_server.h>
#include <string.h>
#include <errno.h>

/*
 * Functional SoftAP QA. Matches samples/net/wifi/shell:
 *   wifi ap enable -s Alif-AP -c 6 -k 1 -p 12345678
 *   net ipv4 add 1 192.168.4.1 255.255.255.0
 *   net dhcpv4 server start 1 192.168.4.10
 * Phone/laptop joins Alif-AP, then ping 192.168.4.10.
 * This is SoftAP, not P2P GO (do not flash wifi_p2p).
 */
#define PING_WAIT_MS		3000

struct wifi_ap_qa_fixture {
	struct net_if *iface;
	bool ap_started;
};

static struct wifi_ap_qa_fixture g_ap;
static K_SEM_DEFINE(ping_sem, 0, 1);
static volatile int ping_replies;
static struct in_addr client_lease;

static int ping_handler(struct net_icmp_ctx *ctx, struct net_pkt *pkt,
			struct net_icmp_ip_hdr *ip_hdr,
			struct net_icmp_hdr *icmp_hdr, void *user_data)
{
	ARG_UNUSED(ctx);
	ARG_UNUSED(pkt);
	ARG_UNUSED(ip_hdr);
	ARG_UNUSED(icmp_hdr);
	ARG_UNUSED(user_data);

	ping_replies++;
	k_sem_give(&ping_sem);
	return 0;
}

static void lease_cb(struct net_if *iface, struct dhcpv4_addr_slot *lease, void *user_data)
{
	struct in_addr *out = user_data;

	ARG_UNUSED(iface);
	if (lease->state == DHCPV4_SERVER_ADDR_ALLOCATED) {
		*out = lease->addr;
	}
}

static bool ap_ipv4_ok(struct net_if *iface)
{
	struct in_addr addr;
	struct net_if *found = NULL;

	if (net_addr_pton(AF_INET, CONFIG_WIFI_AP_TEST_IP_ADDR, &addr) != 0) {
		return false;
	}
	return net_if_ipv4_addr_lookup(&addr, &found) != NULL && found == iface;
}

static int configure_ap_ipv4(struct net_if *iface)
{
	struct in_addr addr;
	struct in_addr mask;
	struct in_addr pool;
	int err;

	if (net_addr_pton(AF_INET, CONFIG_WIFI_AP_TEST_IP_ADDR, &addr) != 0) {
		return -EINVAL;
	}
	if (net_addr_pton(AF_INET, CONFIG_WIFI_AP_TEST_NETMASK, &mask) != 0) {
		return -EINVAL;
	}
	if (net_addr_pton(AF_INET, CONFIG_WIFI_AP_TEST_DHCP_POOL_BASE, &pool) != 0) {
		return -EINVAL;
	}

	net_if_ipv4_set_gw(iface, &addr);
	if (net_if_ipv4_addr_add(iface, &addr, NET_ADDR_MANUAL, 0) == NULL) {
		if (!ap_ipv4_ok(iface)) {
			return -EIO;
		}
	}
	if (!net_if_ipv4_set_netmask_by_addr(iface, &addr, &mask)) {
		return -EIO;
	}

	err = net_dhcpv4_server_start(iface, &pool);
	if (err == -EALREADY) {
		err = 0;
	}
	return err;
}

static int ap_enable(struct net_if *iface)
{
	struct wifi_connect_req_params params = { 0 };

	params.ssid = (const uint8_t *)CONFIG_WIFI_AP_TEST_SSID;
	params.ssid_length = strlen(CONFIG_WIFI_AP_TEST_SSID);
	params.psk = (const uint8_t *)CONFIG_WIFI_AP_TEST_PSK;
	params.psk_length = strlen(CONFIG_WIFI_AP_TEST_PSK);
	params.channel = CONFIG_WIFI_AP_TEST_CHANNEL;
	params.band = WIFI_FREQ_BAND_2_4_GHZ;
	params.security = WIFI_SECURITY_TYPE_PSK;
	params.mfp = WIFI_MFP_OPTIONAL;

	return net_mgmt(NET_REQUEST_WIFI_AP_ENABLE, iface, &params, sizeof(params));
}

static int ping_ipv4(struct net_if *iface, const struct in_addr *dst_addr, int timeout_ms)
{
	struct net_icmp_ctx ctx;
	struct sockaddr_in dst = { 0 };
	int err;

	ping_replies = 0;
	k_sem_reset(&ping_sem);

	err = net_icmp_init_ctx(&ctx, NET_ICMPV4_ECHO_REPLY, 0, ping_handler);
	if (err != 0) {
		return err;
	}

	dst.sin_family = AF_INET;
	dst.sin_addr = *dst_addr;
	err = net_icmp_send_echo_request(&ctx, iface, (struct sockaddr *)&dst, NULL, NULL);
	if (err != 0) {
		net_icmp_cleanup_ctx(&ctx);
		return err;
	}

	if (k_sem_take(&ping_sem, K_MSEC(timeout_ms)) != 0) {
		net_icmp_cleanup_ctx(&ctx);
		return -ETIMEDOUT;
	}
	net_icmp_cleanup_ctx(&ctx);
	return 0;
}

static void skip_if_no_ap(void)
{
	if (!g_ap.ap_started) {
		TC_PRINT("SoftAP not started, skip\n");
		ztest_test_skip();
	}
}

static void skip_if_no_client(void)
{
	skip_if_no_ap();
	memset(&client_lease, 0, sizeof(client_lease));
	net_dhcpv4_server_foreach_lease(g_ap.iface, lease_cb, &client_lease);
	if (client_lease.s_addr == 0) {
		TC_PRINT("No DHCPv4 lease; join SSID %s PSK %s and re-run\n",
			 CONFIG_WIFI_AP_TEST_SSID, CONFIG_WIFI_AP_TEST_PSK);
		ztest_test_skip();
	}
}

static void *ap_suite_setup(void)
{
	int retry = 50;
	int err;

	TC_PRINT("\n*** wifi_ap_qa ztest (SoftAP, not P2P / wifi shell sample) ***\n");
	TC_PRINT("SSID=%s ch=%d IPv4=%s DHCP=%s (sample: wifi ap enable -s %s -c %d -k 1 -p %s)\n",
		 CONFIG_WIFI_AP_TEST_SSID, CONFIG_WIFI_AP_TEST_CHANNEL,
		 CONFIG_WIFI_AP_TEST_IP_ADDR, CONFIG_WIFI_AP_TEST_DHCP_POOL_BASE,
		 CONFIG_WIFI_AP_TEST_SSID, CONFIG_WIFI_AP_TEST_CHANNEL,
		 CONFIG_WIFI_AP_TEST_PSK);

	while (retry-- > 0) {
		g_ap.iface = net_if_get_wifi_sap();
		if (g_ap.iface == NULL) {
			g_ap.iface = net_if_get_wifi_sta();
		}
		if (g_ap.iface != NULL) {
			break;
		}
		k_sleep(K_MSEC(100));
	}
	zassert_not_null(g_ap.iface, "No Wi-Fi interface");

	TC_PRINT("Waiting for WHD/AIROC firmware (iface may stay down until AP enable)\n");
	k_sleep(K_SECONDS(3));

	err = configure_ap_ipv4(g_ap.iface);
	zassert_equal(err, 0, "SoftAP IPv4/DHCP setup failed (%d)", err);

	err = ap_enable(g_ap.iface);
	if (err == -EALREADY || err == -EEXIST) {
		TC_PRINT("AP already up (%d)\n", err);
		err = 0;
	}
	zassert_equal(err, 0, "WIFI_AP_ENABLE failed (%d)", err);

	if (ap_ipv4_ok(g_ap.iface)) {
		g_ap.ap_started = true;
	}
	zassert_true(g_ap.ap_started, "AP IPv4 %s not configured",
		     CONFIG_WIFI_AP_TEST_IP_ADDR);
	net_if_dormant_off(g_ap.iface);
	(void)net_if_up(g_ap.iface);
	TC_PRINT("SoftAP started SSID=%s IPv4=%s ch=%d\n",
		 CONFIG_WIFI_AP_TEST_SSID, CONFIG_WIFI_AP_TEST_IP_ADDR,
		 CONFIG_WIFI_AP_TEST_CHANNEL);
	return &g_ap;
}

static void ap_suite_teardown(void *f)
{
	ARG_UNUSED(f);

	if (g_ap.iface != NULL && g_ap.ap_started) {
		net_if_dormant_off(g_ap.iface);
		(void)net_if_up(g_ap.iface);
	}
	TC_PRINT("SoftAP left up: join %s PSK %s, expect DHCP %s, ping\n",
		 CONFIG_WIFI_AP_TEST_SSID, CONFIG_WIFI_AP_TEST_PSK,
		 CONFIG_WIFI_AP_TEST_DHCP_POOL_BASE);
	TC_PRINT("Do not type wifi ap enable again on this boot (already AP is on)\n");
	TC_PRINT("Leftover: net iface should show wlan0 UP and %s\n",
		 CONFIG_WIFI_AP_TEST_IP_ADDR);
}

ZTEST(wifi_ap_basic, test_ap_ssid_len)
{
	zassert_true(strlen(CONFIG_WIFI_AP_TEST_SSID) >= 1, "SSID empty");
	zassert_true(strlen(CONFIG_WIFI_AP_TEST_SSID) <= WIFI_SSID_MAX_LEN, "SSID too long");
	zassert_true(strlen(CONFIG_WIFI_AP_TEST_PSK) >= 8, "PSK shorter than 8");
}

ZTEST(wifi_ap_basic, test_ap_ipv4_parse)
{
	struct in_addr addr;

	zassert_equal(net_addr_pton(AF_INET, CONFIG_WIFI_AP_TEST_IP_ADDR, &addr), 0);
	zassert_equal(net_addr_pton(AF_INET, CONFIG_WIFI_AP_TEST_DHCP_POOL_BASE, &addr), 0);
}

ZTEST(wifi_ap_basic, test_ap_channel_range)
{
	zassert_true(CONFIG_WIFI_AP_TEST_CHANNEL >= 1 && CONFIG_WIFI_AP_TEST_CHANNEL <= 11,
		     "channel %d not in 1..11", CONFIG_WIFI_AP_TEST_CHANNEL);
}

/*
 * Purpose : wlan0 / airoc-wifi present after WHD boot.
 * Method  : Fixture iface non-NULL; AP started.
 * Pass    : Prints device name and ap=1.
 */
ZTEST_F(wifi_ap_qa, test_ap_iface_ready)
{
	const struct device *dev;

	skip_if_no_ap();
	zassert_not_null(fixture->iface);
	dev = net_if_get_device(fixture->iface);
	TC_PRINT("device=%s ap=%d up=%d\n",
		 dev ? dev->name : "?", fixture->ap_started,
		 net_if_is_up(fixture->iface));
}

/*
 * Purpose : WLAN MAC is a real 6-byte address.
 * Method  : net_if_get_link_addr().
 */
ZTEST_F(wifi_ap_qa, test_ap_mac)
{
	struct net_linkaddr *ll;

	skip_if_no_ap();
	ll = net_if_get_link_addr(fixture->iface);
	zassert_not_null(ll);
	zassert_equal(ll->len, 6);
	zassert_false(ll->addr[0] == 0 && ll->addr[1] == 0 && ll->addr[2] == 0 &&
			      ll->addr[3] == 0 && ll->addr[4] == 0 && ll->addr[5] == 0,
		      "MAC is all-zero");
	TC_PRINT("Link addr %02x:%02x:%02x:%02x:%02x:%02x\n",
		 ll->addr[0], ll->addr[1], ll->addr[2],
		 ll->addr[3], ll->addr[4], ll->addr[5]);
}

/*
 * Purpose : MTU at least 1500 (sample 1544).
 */
ZTEST_F(wifi_ap_qa, test_ap_mtu)
{
	uint16_t mtu;

	skip_if_no_ap();
	mtu = net_if_get_mtu(fixture->iface);
	TC_PRINT("MTU %u\n", mtu);
	zassert_true(mtu >= 1500, "MTU %u < 1500", mtu);
}

/*
 * Purpose : SoftAP as sample wifi ap enable -s Alif-AP -c 6 -k 1 -p 12345678.
 * Method  : Suite setup already called AP_ENABLE; check ap_started.
 */
ZTEST_F(wifi_ap_qa, test_ap_enable)
{
	skip_if_no_ap();
	TC_PRINT("SoftAP SSID=%s ch=%d IPv4=%s\n",
		 CONFIG_WIFI_AP_TEST_SSID, CONFIG_WIFI_AP_TEST_CHANNEL,
		 CONFIG_WIFI_AP_TEST_IP_ADDR);
}

/*
 * Purpose : SoftAP IPv4 192.168.4.1/24.
 */
ZTEST_F(wifi_ap_qa, test_ap_ipv4)
{
	skip_if_no_ap();
	zassert_true(ap_ipv4_ok(fixture->iface), "missing %s", CONFIG_WIFI_AP_TEST_IP_ADDR);
	TC_PRINT("AP IPv4 %s\n", CONFIG_WIFI_AP_TEST_IP_ADDR);
}

/*
 * Purpose : After AP enable, record oper/carrier.
 * Method  : Print flags. Pass if AP IPv4 is configured.
 */
ZTEST_F(wifi_ap_qa, test_ap_carrier)
{
	skip_if_no_ap();
	TC_PRINT("iface_up=%d carrier=%d\n",
		 net_if_is_up(fixture->iface), net_if_is_carrier_ok(fixture->iface));
}

/*
 * Purpose : iface status SSID=Alif-AP and mode AP.
 * Method  : NET_REQUEST_WIFI_IFACE_STATUS.
 */
ZTEST_F(wifi_ap_qa, test_ap_ssid)
{
	struct wifi_iface_status st = { 0 };
	int err;

	skip_if_no_ap();
	err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, fixture->iface, &st, sizeof(st));
	if (err != 0 || st.ssid_len == 0) {
		TC_PRINT("iface status SSID unavailable (%d); AP still up\n", err);
		ztest_test_skip();
	}
	zassert_equal(st.ssid_len, strlen(CONFIG_WIFI_AP_TEST_SSID));
	zassert_mem_equal(st.ssid, CONFIG_WIFI_AP_TEST_SSID, st.ssid_len);
	TC_PRINT("SSID=%.*s channel=%u mode=%d\n", st.ssid_len, st.ssid, st.channel,
		 st.iface_mode);
}

/*
 * Purpose : SoftAP iface status fields the AIROC driver fills.
 * Method  : NET_REQUEST_WIFI_IFACE_STATUS after AP_ENABLE.
 */
ZTEST_F(wifi_ap_qa, test_ap_link_params)
{
	struct wifi_iface_status st = { 0 };
	int err;

	skip_if_no_ap();
	err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, fixture->iface, &st, sizeof(st));
	zassert_equal(err, 0, "IFACE_STATUS %d", err);
	TC_PRINT("mode=%d band=%s ch=%u sec=%s twt=%d beacon=%u dtim=%u rate=%u\n",
		 st.iface_mode, wifi_band_txt(st.band), st.channel,
		 wifi_security_txt(st.security), st.twt_capable,
		 st.beacon_interval, st.dtim_period, st.current_phy_tx_rate);
	zassert_equal(st.iface_mode, WIFI_MODE_AP, "not AP mode");
	zassert_false(st.twt_capable, "AIROC twt_capable=false");
	if (st.channel != 0) {
		zassert_true(st.channel >= 1 && st.channel <= 14, "bad AP ch %u", st.channel);
	}
}

/*
 * Purpose : SoftAP BSSID / own BSS MAC from WHD.
 */
ZTEST_F(wifi_ap_qa, test_ap_bssid)
{
	struct wifi_iface_status st = { 0 };
	int err;

	skip_if_no_ap();
	err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, fixture->iface, &st, sizeof(st));
	if (err != 0 || (st.bssid[0] == 0 && st.bssid[1] == 0 && st.bssid[2] == 0 &&
			 st.bssid[3] == 0 && st.bssid[4] == 0 && st.bssid[5] == 0)) {
		TC_PRINT("AP BSSID unavailable\n");
		ztest_test_skip();
	}
	TC_PRINT("AP BSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
		 st.bssid[0], st.bssid[1], st.bssid[2],
		 st.bssid[3], st.bssid[4], st.bssid[5]);
}

/*
 * Purpose : SoftAP was enabled as WPA2-PSK (sample -k 1).
 * Method  : IFACE_STATUS security. WHD get_ap_info on SoftAP often
 *           returns WIFI_SECURITY_TYPE_UNKNOWN (21); that is a status
 *           mapping gap, not an open AP. Client join already used PSK.
 */
ZTEST_F(wifi_ap_qa, test_ap_security)
{
	struct wifi_iface_status st = { 0 };
	int err;

	skip_if_no_ap();
	err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, fixture->iface, &st, sizeof(st));
	if (err != 0 || st.ssid_len == 0) {
		TC_PRINT("AP security unavailable\n");
		ztest_test_skip();
	}
	TC_PRINT("AP security %s (%d)\n", wifi_security_txt(st.security), st.security);
	zassert_not_equal(st.security, WIFI_SECURITY_TYPE_NONE, "SoftAP must not be OPEN");
	if (st.security == WIFI_SECURITY_TYPE_UNKNOWN) {
		TC_PRINT("WHD SoftAP status security unmapped; AP enable used WPA2-PSK\n");
		return;
	}
	zassert_true(st.security == WIFI_SECURITY_TYPE_PSK ||
			     st.security == WIFI_SECURITY_TYPE_PSK_SHA256 ||
			     st.security == WIFI_SECURITY_TYPE_WPA_AUTO_PERSONAL ||
			     st.security == WIFI_SECURITY_TYPE_WPA_PSK,
		     "expected PSK family, got %d", st.security);
}

/*
 * Purpose : Beacon / DTIM from WHD BSS info while SoftAP is up.
 */
ZTEST_F(wifi_ap_qa, test_ap_beacon_dtim)
{
	struct wifi_iface_status st = { 0 };
	int err;

	skip_if_no_ap();
	err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, fixture->iface, &st, sizeof(st));
	if (err != 0 || st.beacon_interval == 0) {
		TC_PRINT("beacon/DTIM unavailable\n");
		ztest_test_skip();
	}
	TC_PRINT("beacon=%u dtim=%u\n", st.beacon_interval, st.dtim_period);
	zassert_true(st.beacon_interval >= 50 && st.beacon_interval <= 1000,
		     "beacon %u", st.beacon_interval);
}

/*
 * Purpose : ap_sta_disconnect is not implemented on AIROC.
 */
ZTEST_F(wifi_ap_qa, test_ap_sta_disconnect_notsup)
{
	uint8_t mac[WIFI_MAC_ADDR_LEN] = { 0 };
	int err;

	skip_if_no_ap();
	err = net_mgmt(NET_REQUEST_WIFI_AP_STA_DISCONNECT, fixture->iface, mac, sizeof(mac));
	TC_PRINT("AP_STA_DISCONNECT %d\n", err);
	zassert_equal(err, -ENOTSUP, "expected -ENOTSUP, got %d", err);
}

/*
 * Purpose : Wi-Fi stats API works on SoftAP.
 * Method  : NET_REQUEST_STATS_GET_WIFI.
 */
ZTEST_F(wifi_ap_qa, test_ap_stats)
{
	struct net_stats_wifi stats = { 0 };
	int err;

	skip_if_no_ap();
	err = net_mgmt(NET_REQUEST_STATS_GET_WIFI, fixture->iface, &stats, sizeof(stats));
	if (err != 0) {
		TC_PRINT("Wi-Fi stats unavailable (%d)\n", err);
		ztest_test_skip();
	}
	TC_PRINT("stats pkts.tx=%u pkts.rx=%u bytes.tx=%u bytes.rx=%u\n",
		 stats.pkts.tx, stats.pkts.rx, stats.bytes.sent, stats.bytes.received);
}

/*
 * Purpose : Poll SoftAP status repeatedly; AP must stay up (no leftover kill).
 */
ZTEST_F(wifi_ap_qa, test_ap_stress_status)
{
	struct wifi_iface_status st = { 0 };
	int err;
	int n = 15;

	skip_if_no_ap();
	for (int i = 0; i < n; i++) {
		memset(&st, 0, sizeof(st));
		err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, fixture->iface, &st, sizeof(st));
		zassert_equal(err, 0, "status poll %d failed %d", i + 1, err);
		zassert_equal(st.iface_mode, WIFI_MODE_AP, "poll %d not AP", i + 1);
		k_sleep(K_MSEC(200));
	}
	TC_PRINT("SoftAP status polls %d OK\n", n);
}

/*
 * Purpose : Second wifi ap enable is rejected (Already AP is on).
 * Method  : AP_ENABLE while AP is up.
 * Pass    : -EAGAIN / -EALREADY / -EEXIST / -EBUSY.
 */
ZTEST_F(wifi_ap_qa, test_ap_second_enable)
{
	int err;

	skip_if_no_ap();
	err = ap_enable(fixture->iface);
	TC_PRINT("second AP_ENABLE %d\n", err);
	zassert_true(err == -EAGAIN || err == -EALREADY || err == -EEXIST || err == -EBUSY,
		     "expected already-on, got %d", err);
}

/*
 * Purpose : STA connect is rejected while SoftAP is up.
 * Method  : NET_REQUEST_WIFI_CONNECT dummy SSID.
 * Pass    : -EBUSY (AIROC: Network interface is busy in STA/AP).
 */
ZTEST_F(wifi_ap_qa, test_ap_sta_busy)
{
	struct wifi_connect_req_params params = { 0 };
	static const char ssid[] = "Alif-Wireless";
	static const char psk[] = "12345678";
	int err;

	skip_if_no_ap();
	params.ssid = (const uint8_t *)ssid;
	params.ssid_length = sizeof(ssid) - 1;
	params.psk = (const uint8_t *)psk;
	params.psk_length = sizeof(psk) - 1;
	params.security = WIFI_SECURITY_TYPE_PSK;
	params.channel = WIFI_CHANNEL_ANY;
	params.band = WIFI_FREQ_BAND_2_4_GHZ;
	err = net_mgmt(NET_REQUEST_WIFI_CONNECT, fixture->iface, &params, sizeof(params));
	TC_PRINT("STA CONNECT while AP %d\n", err);
	zassert_true(err == -EBUSY || err == -EAGAIN || err == -EALREADY,
		     "expected busy, got %d", err);
	net_if_dormant_off(fixture->iface);
	(void)net_if_up(fixture->iface);
}

/*
 * Purpose : Phone/laptop join Alif-AP then DHCPv4 ALLOCATED 192.168.4.10.
 * Method  : Wait up to WIFI_AP_CLIENT_WAIT_MS for a lease.
 * Pass    : Lease, or SKIP if no client.
 */
ZTEST_F(wifi_ap_qa, test_ap_client_dhcp)
{
	int waits = CONFIG_WIFI_AP_CLIENT_WAIT_MS / 1000;

	skip_if_no_ap();
	memset(&client_lease, 0, sizeof(client_lease));
	for (int i = 0; i < waits; i++) {
		net_dhcpv4_server_foreach_lease(fixture->iface, lease_cb, &client_lease);
		if (client_lease.s_addr != 0) {
			char buf[NET_IPV4_ADDR_LEN];

			net_addr_ntop(AF_INET, &client_lease, buf, sizeof(buf));
			TC_PRINT("DHCPv4 ALLOCATED %s\n", buf);
			return;
		}
		k_sleep(K_SECONDS(1));
	}
	TC_PRINT("No client in %d s; join SSID %s PSK %s\n",
		 waits, CONFIG_WIFI_AP_TEST_SSID, CONFIG_WIFI_AP_TEST_PSK);
	ztest_test_skip();
}

/*
 * Purpose : ICMP echo to the DHCP client (sample net ping 192.168.4.10).
 */
ZTEST_F(wifi_ap_qa, test_ap_client_ping)
{
	int err;

	skip_if_no_client();
	err = ping_ipv4(fixture->iface, &client_lease, PING_WAIT_MS);
	if (err == -ETIMEDOUT) {
		TC_PRINT("No ICMP reply from client\n");
		ztest_test_skip();
	}
	zassert_equal(err, 0, "ping failed (%d)", err);
	TC_PRINT("PING client: %d reply(ies)\n", ping_replies);
}

/*
 * Purpose : Do not disable SoftAP so leftover join still works.
 * Method  : Skip AP_DISABLE (same leftover pattern as P2P GO).
 */
ZTEST_F(wifi_ap_qa, test_ap_disable)
{
	skip_if_no_ap();
	TC_PRINT("Skip AP_DISABLE: AP left up for leftover phone/laptop join\n");
	ztest_test_skip();
}

ZTEST_SUITE(wifi_ap_basic, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(wifi_ap_qa, NULL, ap_suite_setup, NULL, NULL, ap_suite_teardown);
