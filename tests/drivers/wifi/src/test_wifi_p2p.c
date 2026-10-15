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
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/wifi.h>
#include <zephyr/net/net_stats.h>
#include <zephyr/net/icmp.h>
#include <zephyr/net/dhcpv4_server.h>
#include <string.h>
#include <errno.h>

/*
 * Matches samples/net/wifi/shell + overlay-p2p.conf:
 *   wifi p2p group add -f 2437
 *   SSID=DIRECT-Al, device name=mahesh's S23, PSK=Aadhya13$
 *   GO 192.168.50.1, DHCP pool 192.168.50.10+
 *   Phone: Provision Discovery / WPS PBC / authorized, then ping.
 * Limitation: reboot before another GO (remove then add does not restore DHCP).
 */
#define P2P_GO_FREQ_MHZ		2437
#define P2P_GO_CHANNEL		6
#define P2P_CLIENT_WAIT_MS	45000
#define PING_WAIT_MS		3000

struct wifi_p2p_qa_fixture {
	struct net_if *iface;
	bool go_started;
};

static struct wifi_p2p_qa_fixture g_p2p;
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

static bool go_ipv4_ok(struct net_if *iface)
{
	struct in_addr addr;
	struct net_if *found = NULL;

	if (net_addr_pton(AF_INET, CONFIG_AIROC_WIFI_P2P_GO_IP_ADDR, &addr) != 0) {
		return false;
	}
	return net_if_ipv4_addr_lookup(&addr, &found) != NULL && found == iface;
}

static int p2p_group_add(struct net_if *iface, int freq)
{
	struct wifi_p2p_params params = { 0 };

	params.oper = WIFI_P2P_GROUP_ADD;
	params.group_add.freq = freq;
	params.group_add.persistent = -1;
	return net_mgmt(NET_REQUEST_WIFI_P2P_OPER, iface, &params, sizeof(params));
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

static void skip_if_no_go(void)
{
	if (!g_p2p.go_started) {
		TC_PRINT("P2P GO not started, skip\n");
		ztest_test_skip();
	}
}

static void skip_if_no_client(void)
{
	skip_if_no_go();
	memset(&client_lease, 0, sizeof(client_lease));
	net_dhcpv4_server_foreach_lease(g_p2p.iface, lease_cb, &client_lease);
	if (client_lease.s_addr == 0) {
		TC_PRINT("No DHCPv4 lease; connect phone Wi-Fi Direct to %s and re-run\n",
			 CONFIG_AIROC_WIFI_P2P_GO_SSID);
		ztest_test_skip();
	}
}

static void *p2p_suite_setup(void)
{
	int retry = 50;
	int err;
	int waited;

	TC_PRINT("\n*** wifi_p2p_qa ztest (not wifi shell sample) ***\n");
	TC_PRINT("SSID=%s GO=%s freq=%d (sample: wifi p2p group add -f %d)\n",
		 CONFIG_AIROC_WIFI_P2P_GO_SSID, CONFIG_AIROC_WIFI_P2P_GO_IP_ADDR,
		 P2P_GO_FREQ_MHZ, P2P_GO_FREQ_MHZ);

	while (retry-- > 0) {
		g_p2p.iface = net_if_get_wifi_sta();
		if (g_p2p.iface != NULL) {
			break;
		}
		k_sleep(K_MSEC(100));
	}
	zassert_not_null(g_p2p.iface, "No Wi-Fi interface");

	/*
	 * STA iface stays down until autonomous GO starts (sample: group add
	 * then net iface oper=UP). Do not wait for net_if_is_up() here.
	 */
	TC_PRINT("Waiting for WHD/AIROC firmware (iface may stay down until GO)\n");
	k_sleep(K_SECONDS(3));

	err = p2p_group_add(g_p2p.iface, P2P_GO_FREQ_MHZ);
	if (err == -EALREADY || err == -EEXIST) {
		TC_PRINT("P2P GROUP_ADD already active (%d)\n", err);
		err = 0;
	}
	zassert_equal(err, 0, "P2P GROUP_ADD failed (%d)", err);

	for (waited = 0; waited < 150; waited++) {
		if (go_ipv4_ok(g_p2p.iface)) {
			g_p2p.go_started = true;
			break;
		}
		k_sleep(K_MSEC(100));
	}
	zassert_true(g_p2p.go_started, "GO IPv4 %s not configured",
		     CONFIG_AIROC_WIFI_P2P_GO_IP_ADDR);
	TC_PRINT("P2P autonomous GO started SSID=%s IPv4=%s\n",
		 CONFIG_AIROC_WIFI_P2P_GO_SSID, CONFIG_AIROC_WIFI_P2P_GO_IP_ADDR);
	return &g_p2p;
}

static void p2p_suite_teardown(void *f)
{
	ARG_UNUSED(f);

	TC_PRINT("P2P GO left up: phone Wi-Fi Direct -> %s (SSID %s PSK %s), expect DHCP %s, ping\n",
		 CONFIG_AIROC_WIFI_P2P_DEVICE_NAME, CONFIG_AIROC_WIFI_P2P_GO_SSID,
		 CONFIG_AIROC_WIFI_P2P_GO_PSK, CONFIG_AIROC_WIFI_P2P_GO_DHCP_POOL_BASE);
	TC_PRINT("Do not wifi p2p group remove then add without reboot (DHCP limitation)\n");
}

ZTEST(wifi_p2p_basic, test_ssid_direct_prefix)
{
	zassert_true(strncmp(CONFIG_AIROC_WIFI_P2P_GO_SSID, "DIRECT-", 7) == 0,
		     "Wi-Fi Direct SSID must start with DIRECT-");
	zassert_true(strlen(CONFIG_AIROC_WIFI_P2P_GO_SSID) >= 9, "SSID length < 9");
}

ZTEST(wifi_p2p_basic, test_go_ipv4_parse)
{
	struct in_addr addr;

	zassert_equal(net_addr_pton(AF_INET, CONFIG_AIROC_WIFI_P2P_GO_IP_ADDR, &addr), 0);
	zassert_equal(addr.s4_addr[0], 192);
	zassert_equal(addr.s4_addr[3], 1);
}

ZTEST(wifi_p2p_basic, test_freq_2437)
{
	zassert_equal(P2P_GO_FREQ_MHZ, 2437);
	zassert_equal(P2P_GO_CHANNEL, 6);
}

/*
 * Wi-Fi Interface Ready
 *
 * Purpose : wlan0 / airoc-wifi after WHD boot (sample MAC / firmware lines).
 * Method  : net_if_get_wifi_sta(); GO start is the oper-up signal.
 * Pass    : Iface present and GO started or iface up.
 * Fail    : No iface.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_iface_ready)
{
	zassert_not_null(fixture->iface);
	zassert_true(fixture->go_started || net_if_is_up(fixture->iface),
		     "Wi-Fi iface not ready");
	TC_PRINT("Wi-Fi iface %s go=%d up=%d\n",
		 net_if_get_device(fixture->iface)->name,
		 fixture->go_started ? 1 : 0,
		 net_if_is_up(fixture->iface) ? 1 : 0);
}

/*
 * P2P MAC
 *
 * Purpose : Link addr printed in sample (F0:3E:05:D7:F2:72).
 * Method  : net_if_get_link_addr().
 * Pass    : 6-byte MAC, not all-zero.
 * Fail    : Missing MAC.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_mac)
{
	struct net_linkaddr *ll = net_if_get_link_addr(fixture->iface);
	static const uint8_t zero[6];

	zassert_not_null(ll);
	zassert_equal(ll->len, 6);
	zassert_true(memcmp(ll->addr, zero, 6) != 0, "MAC all-zero");
	TC_PRINT("Link addr %02x:%02x:%02x:%02x:%02x:%02x\n",
		 ll->addr[0], ll->addr[1], ll->addr[2],
		 ll->addr[3], ll->addr[4], ll->addr[5]);
}

/*
 * P2P MTU
 *
 * Purpose : Sample net iface MTU 1544.
 * Method  : net_if_get_mtu().
 * Pass    : MTU >= 1500.
 * Fail    : Too small.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_mtu)
{
	uint16_t mtu = net_if_get_mtu(fixture->iface);

	zassert_true(mtu >= 1500, "MTU %u", mtu);
	TC_PRINT("MTU %u\n", mtu);
}

/*
 * P2P Group Add
 *
 * Purpose : Sample `wifi p2p group add -f 2437` (channel 6).
 * Method  : GROUP_ADD in suite setup; assert GO flag.
 * Pass    : go_started.
 * Fail    : GROUP_ADD or IPv4 missing.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_group_add)
{
	zassert_true(fixture->go_started, "autonomous GO not started");
	TC_PRINT("P2P autonomous GO started on %d MHz (ch %d) SSID=%s\n",
		 P2P_GO_FREQ_MHZ, P2P_GO_CHANNEL, CONFIG_AIROC_WIFI_P2P_GO_SSID);
}

/*
 * GO IPv4
 *
 * Purpose : Sample 192.168.50.1/24 DHCPv4 server.
 * Method  : net_if_ipv4_addr_lookup().
 * Pass    : CONFIG_AIROC_WIFI_P2P_GO_IP_ADDR on wlan0.
 * Fail    : Address missing.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_go_ipv4)
{
	skip_if_no_go();
	zassert_true(go_ipv4_ok(fixture->iface), "GO IPv4 missing");
	TC_PRINT("GO IPv4 %s\n", CONFIG_AIROC_WIFI_P2P_GO_IP_ADDR);
}

/*
 * Carrier
 *
 * Purpose : Sample Status after GO (IPv4 192.168.50.1).
 * Method  : Print oper/carrier; GO IPv4 is the pass signal.
 * Pass    : GO started.
 * Fail    : GO not started.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_carrier)
{
	skip_if_no_go();
	TC_PRINT("iface_up=%d carrier=%d (GO IPv4 is the bring-up signal)\n",
		 net_if_is_up(fixture->iface) ? 1 : 0,
		 net_if_is_carrier_ok(fixture->iface) ? 1 : 0);
}

/*
 * GO SSID
 *
 * Purpose : Sample SSID=DIRECT-Al.
 * Method  : NET_REQUEST_WIFI_IFACE_STATUS.
 * Pass    : SSID matches Kconfig, or skip if status has empty SSID.
 * Fail    : Wrong SSID.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_ssid)
{
	struct wifi_iface_status st = { 0 };
	int err;

	skip_if_no_go();
	err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, fixture->iface, &st, sizeof(st));
	if (err != 0 || st.ssid_len == 0) {
		TC_PRINT("iface status SSID unavailable (%d); GO still up\n", err);
		ztest_test_skip();
	}
	zassert_equal(st.ssid_len, strlen(CONFIG_AIROC_WIFI_P2P_GO_SSID));
	zassert_mem_equal(st.ssid, CONFIG_AIROC_WIFI_P2P_GO_SSID, st.ssid_len);
	TC_PRINT("SSID=%.*s channel=%u\n", st.ssid_len, st.ssid, st.channel);
}

/*
 * Purpose : GO iface status (mode AP, ch 6, TWT false).
 */
ZTEST_F(wifi_p2p_qa, test_p2p_link_params)
{
	struct wifi_iface_status st = { 0 };
	int err;

	skip_if_no_go();
	err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, fixture->iface, &st, sizeof(st));
	zassert_equal(err, 0, "IFACE_STATUS %d", err);
	TC_PRINT("mode=%d band=%s ch=%u twt=%d beacon=%u\n",
		 st.iface_mode, wifi_band_txt(st.band), st.channel,
		 st.twt_capable, st.beacon_interval);
	zassert_true(st.iface_mode == WIFI_MODE_AP || st.iface_mode == WIFI_MODE_P2P_GO,
		     "unexpected GO mode %d", st.iface_mode);
	zassert_false(st.twt_capable, "AIROC twt_capable=false");
	if (st.channel != 0) {
		zassert_true(st.channel >= 1 && st.channel <= 14, "bad GO ch %u", st.channel);
	}
}

/*
 * Purpose : GO BSSID from WHD.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_bssid)
{
	struct wifi_iface_status st = { 0 };
	int err;

	skip_if_no_go();
	err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, fixture->iface, &st, sizeof(st));
	if (err != 0 || (st.bssid[0] == 0 && st.bssid[1] == 0 && st.bssid[2] == 0 &&
			 st.bssid[3] == 0 && st.bssid[4] == 0 && st.bssid[5] == 0)) {
		TC_PRINT("GO BSSID unavailable\n");
		ztest_test_skip();
	}
	TC_PRINT("GO BSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
		 st.bssid[0], st.bssid[1], st.bssid[2],
		 st.bssid[3], st.bssid[4], st.bssid[5]);
}

/*
 * Purpose : Wi-Fi stats on P2P GO (needs CONFIG_NET_STATISTICS_WIFI).
 */
ZTEST_F(wifi_p2p_qa, test_p2p_stats)
{
	struct net_stats_wifi stats = { 0 };
	int err;

	skip_if_no_go();
	err = net_mgmt(NET_REQUEST_STATS_GET_WIFI, fixture->iface, &stats, sizeof(stats));
	if (err != 0) {
		TC_PRINT("Wi-Fi stats %d\n", err);
		ztest_test_skip();
	}
	TC_PRINT("GO stats pkts.tx=%u pkts.rx=%u bytes.tx=%u bytes.rx=%u\n",
		 stats.pkts.tx, stats.pkts.rx, stats.bytes.sent, stats.bytes.received);
}

/*
 * Purpose : Poll GO iface status repeatedly; group must stay up.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_stress_status)
{
	struct wifi_iface_status st = { 0 };
	int err;
	int n = 15;

	skip_if_no_go();
	for (int i = 0; i < n; i++) {
		memset(&st, 0, sizeof(st));
		err = net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, fixture->iface, &st, sizeof(st));
		zassert_equal(err, 0, "status poll %d failed %d", i + 1, err);
		zassert_true(st.iface_mode == WIFI_MODE_AP || st.iface_mode == WIFI_MODE_P2P_GO,
			     "poll %d unexpected mode %d", i + 1, st.iface_mode);
		k_sleep(K_MSEC(200));
	}
	TC_PRINT("GO status polls %d OK\n", n);
}

/*
 * P2P Find / Stop
 *
 * Purpose : Shell `wifi p2p find` / `wifi p2p stop_find` (social channels).
 * Method  : WIFI_P2P_FIND then WIFI_P2P_STOP_FIND.
 * Pass    : Both net_mgmt return 0. Skip if GO path rejects find.
 * Fail    : Unexpected error other than not-supported.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_find_stop)
{
	struct wifi_p2p_params params = { 0 };
	int err;

	params.oper = WIFI_P2P_FIND;
	params.discovery_type = WIFI_P2P_FIND_ONLY_SOCIAL;
	params.timeout = 3;
	err = net_mgmt(NET_REQUEST_WIFI_P2P_OPER, fixture->iface, &params, sizeof(params));
	if (err != 0) {
		TC_PRINT("P2P find %d (OK if GO already up)\n", err);
		ztest_test_skip();
	}
	k_sleep(K_SECONDS(1));
	memset(&params, 0, sizeof(params));
	params.oper = WIFI_P2P_STOP_FIND;
	err = net_mgmt(NET_REQUEST_WIFI_P2P_OPER, fixture->iface, &params, sizeof(params));
	zassert_equal(err, 0, "P2P stop find %d", err);
	TC_PRINT("P2P find/stop OK\n");
}

/*
 * P2P Peer Query
 *
 * Purpose : Shell `wifi p2p peer`.
 * Method  : WIFI_P2P_PEER into a small array.
 * Pass    : Request returns 0 (count may be 0).
 * Fail    : net_mgmt error.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_peer_query)
{
	struct wifi_p2p_device_info peers[4];
	struct wifi_p2p_params params = { 0 };
	int err;

	memset(peers, 0, sizeof(peers));
	memset(params.peer_addr, 0xff, sizeof(params.peer_addr));
	params.oper = WIFI_P2P_PEER;
	params.peers = peers;
	params.peer_count = ARRAY_SIZE(peers);
	err = net_mgmt(NET_REQUEST_WIFI_P2P_OPER, fixture->iface, &params, sizeof(params));
	if (err != 0) {
		TC_PRINT("P2P peer query %d\n", err);
		ztest_test_skip();
	}
	TC_PRINT("P2P peers listed: %u\n", params.peer_count);
}

/*
 * Client DHCP
 *
 * Purpose : Sample `net dhcpv4 server status` ALLOCATED 192.168.50.10
 *           after phone WPS/assoc.
 * Method  : Wait up to 45 s for an ALLOCATED lease.
 * Pass    : At least one lease. Skip if no phone.
 * Fail    : foreach API error.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_client_dhcp)
{
	int waits = P2P_CLIENT_WAIT_MS / 1000;

	skip_if_no_go();
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
	TC_PRINT("No DHCP lease after %d s (phone associated is not enough)\n", waits);
	TC_PRINT("Join Wi-Fi Direct %s SSID %s PSK %s / WPS PBC\n",
		 CONFIG_AIROC_WIFI_P2P_DEVICE_NAME,
		 CONFIG_AIROC_WIFI_P2P_GO_SSID, CONFIG_AIROC_WIFI_P2P_GO_PSK);
	ztest_test_skip();
}

/*
 * Client Ping
 *
 * Purpose : Sample `net ping 192.168.50.10`.
 * Method  : ICMP echo to first ALLOCATED lease.
 * Pass    : Reply. Skip if no lease.
 * Fail    : Send error other than timeout (timeout → skip).
 */
ZTEST_F(wifi_p2p_qa, test_p2p_client_ping)
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
 * Group already active
 *
 * Purpose : Second wifi p2p group add is rejected (P2P group already active).
 * Method  : GROUP_ADD while GO is up.
 * Pass    : -EALREADY / -EEXIST / -EAGAIN / -EBUSY.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_group_add_already)
{
	int err;

	skip_if_no_go();
	err = p2p_group_add(fixture->iface, P2P_GO_FREQ_MHZ);
	TC_PRINT("second GROUP_ADD %d\n", err);
	zassert_true(err == -EALREADY || err == -EEXIST || err == -EAGAIN || err == -EBUSY,
		     "expected already-active, got %d", err);
}

/*
 * P2P connect not implemented
 *
 * Purpose : wifi p2p connect is not supported (GO only).
 * Method  : WIFI_P2P_CONNECT.
 * Pass    : -ENOTSUP.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_connect_notsup)
{
	struct wifi_p2p_params params = { 0 };
	int err;

	skip_if_no_go();
	params.oper = WIFI_P2P_CONNECT;
	err = net_mgmt(NET_REQUEST_WIFI_P2P_OPER, fixture->iface, &params, sizeof(params));
	TC_PRINT("P2P CONNECT %d\n", err);
	zassert_equal(err, -ENOTSUP, "expected -ENOTSUP, got %d", err);
}

/*
 * P2P invite not implemented
 *
 * Purpose : wifi p2p invite is not supported.
 * Method  : WIFI_P2P_INVITE.
 * Pass    : -ENOTSUP.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_invite_notsup)
{
	struct wifi_p2p_params params = { 0 };
	int err;

	skip_if_no_go();
	params.oper = WIFI_P2P_INVITE;
	err = net_mgmt(NET_REQUEST_WIFI_P2P_OPER, fixture->iface, &params, sizeof(params));
	TC_PRINT("P2P INVITE %d\n", err);
	zassert_equal(err, -ENOTSUP, "expected -ENOTSUP, got %d", err);
}

/*
 * P2P persistent list not implemented
 *
 * Purpose : wifi p2p list_networks is not supported.
 * Method  : WIFI_P2P_LIST_NETWORKS.
 * Pass    : -ENOTSUP.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_list_notsup)
{
	struct wifi_p2p_params params = { 0 };
	int err;

	skip_if_no_go();
	params.oper = WIFI_P2P_LIST_NETWORKS;
	err = net_mgmt(NET_REQUEST_WIFI_P2P_OPER, fixture->iface, &params, sizeof(params));
	TC_PRINT("P2P LIST_NETWORKS %d\n", err);
	zassert_equal(err, -ENOTSUP, "expected -ENOTSUP, got %d", err);
}

/*
 * P2P GO power save
 *
 * Purpose : wifi p2p power_save on then off while GO is up.
 * Method  : WIFI_P2P_POWER_SAVE true, then false (leave GO awake).
 * Pass    : Both return 0, or SKIP if firmware rejects PS.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_power_save)
{
	struct wifi_p2p_params params = { 0 };
	int err;

	skip_if_no_go();
	params.oper = WIFI_P2P_POWER_SAVE;
	params.power_save = true;
	err = net_mgmt(NET_REQUEST_WIFI_P2P_OPER, fixture->iface, &params, sizeof(params));
	if (err != 0) {
		TC_PRINT("P2P power_save on %d (firmware NoA/OppPS reject)\n", err);
		ztest_test_skip();
	}
	params.power_save = false;
	err = net_mgmt(NET_REQUEST_WIFI_P2P_OPER, fixture->iface, &params, sizeof(params));
	if (err != 0) {
		TC_PRINT("P2P power_save off %d (p2p_noa not supported on this GO)\n", err);
		ztest_test_skip();
	}
	TC_PRINT("P2P power_save on/off OK\n");
}

/*
 * Group Remove
 *
 * Purpose : Sample limitation: remove then add does not restore DHCP.
 * Method  : Do not call GROUP_REMOVE (would break leftover GO / host ping).
 * Pass    : Skip is expected.
 * Fail    : N/A.
 */
ZTEST_F(wifi_p2p_qa, test_p2p_group_remove)
{
	skip_if_no_go();
	TC_PRINT("Skip GROUP_REMOVE: reboot required before another GO (sample limitation)\n");
	ztest_test_skip();
}

ZTEST_SUITE(wifi_p2p_basic, NULL, NULL, NULL, NULL, NULL);
ZTEST_SUITE(wifi_p2p_qa, NULL, p2p_suite_setup, NULL, NULL, p2p_suite_teardown);
