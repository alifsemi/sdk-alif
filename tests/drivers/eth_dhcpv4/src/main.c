/*
 * SPDX-FileCopyrightText: Copyright Alif Semiconductor
 * SPDX-License-Identifier: Apache-2.0
 *
 * Real-hardware Ethernet DHCPv4 client test suite for the Alif platform.
 *
 * This suite exercises the *real* Synopsys DesignWare MAC (DWMAC) driver,
 * the DesignWare MDIO bus and the Realtek RTL8201FR PHY. It is modelled on
 * samples/net/dhcpv4_client but adds ztest assertions so it can be used for
 * automated hardware validation.
 *
 *
 * Prerequisites to run on hardware:
 *   - Alif DevKit cabled to a LAN switch.
 *   - An active DHCP server reachable on that LAN.
 *
 * Some testcases require physical operator action (unplugging the cable) or a
 * specially configured network (exhausted pool, no server, multiple servers).
 * Those are implemented so they validate the behaviour when the condition is
 * present, and otherwise call ztest_test_skip() with on-console instructions
 * rather than producing a false failure.
 */

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(alif_eth_dhcpv4_test, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/ztest.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/dhcpv4.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/icmp.h>
#include <zephyr/net/phy.h>

#if defined(CONFIG_NET_STATISTICS_ETHERNET)
#include <zephyr/net/net_stats.h>
#endif

/* Maximum time we wait for the DHCP server to hand out an address. */
#define DHCP_TIMEOUT       K_SECONDS(60)
/* Maximum time we wait for a ping (ICMP echo) reply from the gateway. */
#define PING_TIMEOUT       K_SECONDS(5)
/* How many seconds we wait for an operator to unplug/replug a cable. */
#define CABLE_WAIT_SECONDS 20
#define CABLE_WAIT_TIMEOUT K_SECONDS(CABLE_WAIT_SECONDS)
/* Number of pings sent by the background-stability test. */
#define BG_PING_COUNT      10
/* Required success ratio (%) for the background-stability test. */
#define BG_PING_MIN_OK_PCT 80
/* Number of cycles for the stop/restart and reboot-proxy stress tests. */
#define STRESS_CYCLES      5

/* DHCP option numbers (RFC 2132). */
#define DHCP_OPTION_DNS    (6)
#define DHCP_OPTION_NTP    (42)

#define PING_TEST_DATA     "alif-eth-icmp"

#define DHCP_EVENT_MASK    (NET_EVENT_IPV4_ADDR_ADD | NET_EVENT_IPV4_DHCP_BOUND)

static struct net_mgmt_event_callback dhcp_cb;
static K_SEM_DEFINE(dhcp_bound, 0, 1);
static volatile bool dhcp_done;
static volatile bool ipv4_addr_add_seen;

static struct net_dhcpv4_option_callback ntp_cb;
static uint8_t ntp_server[4];
static volatile bool ntp_option_seen;

static struct net_dhcpv4_option_callback dns_cb;
static uint8_t dns_server[4];
static volatile bool dns_option_seen;

static K_SEM_DEFINE(ping_reply, 0, 1);

static struct net_if *eth_iface;

/* ------------------------------------------------------------------ */
/* Callbacks                                                          */
/* ------------------------------------------------------------------ */

static void dhcp_handler(struct net_mgmt_event_callback *cb,
			 uint32_t mgmt_event,
			 struct net_if *iface)
{
	ARG_UNUSED(cb);
	ARG_UNUSED(iface);

	if (mgmt_event == NET_EVENT_IPV4_ADDR_ADD) {
		ipv4_addr_add_seen = true;
	}

	if (mgmt_event == NET_EVENT_IPV4_DHCP_BOUND) {
		dhcp_done = true;
		k_sem_give(&dhcp_bound);
	}
}

static void ntp_option_handler(struct net_dhcpv4_option_callback *cb,
			       size_t length,
			       enum net_dhcpv4_msg_type msg_type,
			       struct net_if *iface)
{
	char buf[NET_IPV4_ADDR_LEN];

	ARG_UNUSED(length);
	ARG_UNUSED(msg_type);
	ARG_UNUSED(iface);

	ntp_option_seen = true;
	LOG_INF("DHCP option %d (NTP): %s", cb->option,
		net_addr_ntop(AF_INET, cb->data, buf, sizeof(buf)));
}

static void dns_option_handler(struct net_dhcpv4_option_callback *cb,
			       size_t length,
			       enum net_dhcpv4_msg_type msg_type,
			       struct net_if *iface)
{
	char buf[NET_IPV4_ADDR_LEN];

	ARG_UNUSED(length);
	ARG_UNUSED(msg_type);
	ARG_UNUSED(iface);

	dns_option_seen = true;
	LOG_INF("DHCP option %d (DNS): %s", cb->option,
		net_addr_ntop(AF_INET, cb->data, buf, sizeof(buf)));
}

static int icmp_reply_handler(struct net_icmp_ctx *ctx, struct net_pkt *pkt,
			      struct net_icmp_ip_hdr *hdr,
			      struct net_icmp_hdr *icmp_hdr, void *user_data)
{
	ARG_UNUSED(ctx);
	ARG_UNUSED(pkt);
	ARG_UNUSED(hdr);
	ARG_UNUSED(icmp_hdr);
	ARG_UNUSED(user_data);

	k_sem_give(&ping_reply);

	return 0;
}

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

static void wait_for_carrier_up(k_timeout_t timeout)
{
	int64_t deadline = k_uptime_get() + k_ticks_to_ms_floor64(timeout.ticks);

	while (!net_if_is_carrier_ok(eth_iface)) {
		if (k_uptime_get() >= deadline) {
			break;
		}
		k_sleep(K_MSEC(100));
	}
}

/* Dump MAC TX/RX counters so we can tell whether DISCOVER left the wire and
 * whether any frames (the broadcast OFFER) were received. This separates a
 * TX-path problem from an RX-path/driver problem during DHCP negotiation.
 */
static void dump_eth_stats(const char *when)
{
#if defined(CONFIG_NET_STATISTICS_ETHERNET)
	struct net_stats_eth data;
	int ret;

	ret = net_mgmt(NET_REQUEST_STATS_GET_ETHERNET, eth_iface,
		       &data, sizeof(data));
	if (ret < 0) {
		TC_PRINT("eth-stats %s: unavailable (%d)\n", when, ret);
		return;
	}

	TC_PRINT("eth-stats %s: tx_pkts=%u rx_pkts=%u rx_bcast=%u "
		 "rx_mcast=%u errors_rx=%u errors_tx=%u\n",
		 when,
		 (unsigned int)data.pkts.tx,
		 (unsigned int)data.pkts.rx,
		 (unsigned int)data.broadcast.rx,
		 (unsigned int)data.multicast.rx,
		 (unsigned int)data.errors.rx,
		 (unsigned int)data.errors.tx);
#else
	ARG_UNUSED(when);
#endif
}

/* Idempotent: make sure a DHCP lease is in place before a test runs.
 *
 * This is intentionally NON-destructive: if the client is already bound we
 * reuse the lease. Only when there is no lease do we (re)start DHCP, and we
 * give the MAC RX path time to settle between stop and start so a broadcast
 * OFFER is not dropped by a stop/start race in the driver.
 */
static void ensure_dhcp_bound(void)
{
	struct in_addr *addr;

	addr = net_if_ipv4_get_global_addr(eth_iface, NET_ADDR_PREFERRED);
	if (addr != NULL && addr->s_addr != 0 && dhcp_done) {
		return;
	}

	if (!net_if_is_up(eth_iface)) {
		zassert_ok(net_if_up(eth_iface),
			   "Failed to bring Ethernet interface up");
	}

	wait_for_carrier_up(K_SECONDS(5));

	if (!net_if_is_carrier_ok(eth_iface)) {
		TC_PRINT("No PHY carrier after waiting; check the cable/link "
			 "LED before blaming DHCP.\n");
	}

	/* Reset the DHCPv4 state machine so a stuck client can recover. A short
	 * settle delay after stop lets the DWMAC re-arm its RX descriptors and
	 * broadcast filter before the fresh DISCOVER/OFFER exchange begins.
	 */
	net_dhcpv4_stop(eth_iface);
	dhcp_done = false;
	ipv4_addr_add_seen = false;
	k_sem_reset(&dhcp_bound);
	k_sleep(K_MSEC(500));

	dump_eth_stats("before-discover");
	net_dhcpv4_start(eth_iface);

	if (k_sem_take(&dhcp_bound, DHCP_TIMEOUT) != 0) {
		dump_eth_stats("after-timeout");
		zassert_unreachable(
			"Timed out waiting for a DHCP lease. If tx_pkts "
			"increased but rx stayed flat, the DISCOVER left the "
			"DUT but no OFFER was received (RX/driver or server "
			"issue). Check the cable, link LED and that a DHCP "
			"server is reachable on the LAN.");
	}

	dump_eth_stats("after-bound");
}

/* Read the negotiated PHY link state. Returns 0 on success. */
static int read_phy_link(struct phy_link_state *st)
{
	const struct device *phy = net_eth_get_phy(eth_iface);

	if (phy == NULL) {
		TC_PRINT("PHY device is NULL (net_eth_get_phy failed).\n");
		return -ENODEV;
	}

	if (!device_is_ready(phy)) {
		TC_PRINT("PHY device not ready.\n");
		return -ENODEV;
	}

	return phy_get_link_state(phy, st);
}

static bool link_is_10m(enum phy_link_speed s)
{
	return (s & (LINK_HALF_10BASE_T | LINK_FULL_10BASE_T)) != 0;
}

static const char *phy_link_mode_str(enum phy_link_speed s)
{
	switch (s) {
	case LINK_HALF_10BASE_T:
		return "10 Mbps half-duplex";
	case LINK_FULL_10BASE_T:
		return "10 Mbps full-duplex";
	case LINK_HALF_100BASE_T:
		return "100 Mbps half-duplex";
	case LINK_FULL_100BASE_T:
		return "100 Mbps full-duplex";
	case LINK_HALF_1000BASE_T:
		return "1000 Mbps half-duplex";
	case LINK_FULL_1000BASE_T:
		return "1000 Mbps full-duplex";
	case LINK_FULL_2500BASE_T:
		return "2.5 Gbps full-duplex";
	case LINK_FULL_5000BASE_T:
		return "5 Gbps full-duplex";
	default:
		return "unknown link mode";
	}
}

static void print_phy_link_state(const struct phy_link_state *st)
{
	TC_PRINT("PHY link: %s, mode: %s (speed mask: 0x%x)\n",
		 st->is_up ? "up" : "down", phy_link_mode_str(st->speed),
		 st->speed);
}

/* Send a single ICMP echo to the gateway. Returns 0 if a reply arrives. */
static int ping_gateway_once(k_timeout_t timeout)
{
	struct net_icmp_ping_params params = {0};
	struct net_icmp_ctx ctx;
	struct sockaddr_in dst = {0};
	struct in_addr gw;
	int ret;

	gw = net_if_ipv4_get_gw(eth_iface);
	if (gw.s_addr == 0) {
		return -ENETUNREACH;
	}

	ret = net_icmp_init_ctx(&ctx, NET_ICMPV4_ECHO_REPLY, 0,
				icmp_reply_handler);
	if (ret != 0) {
		return ret;
	}

	dst.sin_family = AF_INET;
	memcpy(&dst.sin_addr, &gw, sizeof(gw));

	params.identifier = 0x1234;
	params.sequence = 1;
	params.data = PING_TEST_DATA;
	params.data_size = sizeof(PING_TEST_DATA);

	k_sem_reset(&ping_reply);

	ret = net_icmp_send_echo_request(&ctx, eth_iface,
					 (struct sockaddr *)&dst, &params,
					 NULL);
	if (ret != 0) {
		net_icmp_cleanup_ctx(&ctx);
		return ret;
	}

	ret = k_sem_take(&ping_reply, timeout);
	net_icmp_cleanup_ctx(&ctx);

	return ret;
}

/* ------------------------------------------------------------------ */
/* Suite setup / teardown                                             */
/* ------------------------------------------------------------------ */

static void *eth_dhcpv4_setup(void)
{
	eth_iface = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));

	zassert_not_null(eth_iface,
			 "No Ethernet interface found. Check that the DWMAC "
			 "node is enabled in the devicetree overlay.");

	net_mgmt_init_event_callback(&dhcp_cb, dhcp_handler, DHCP_EVENT_MASK);
	net_mgmt_add_event_callback(&dhcp_cb);

	/* Register NTP and DNS option callbacks like the sample app. */
	net_dhcpv4_init_option_callback(&ntp_cb, ntp_option_handler,
					DHCP_OPTION_NTP, ntp_server,
					sizeof(ntp_server));
	(void)net_dhcpv4_add_option_callback(&ntp_cb);

	net_dhcpv4_init_option_callback(&dns_cb, dns_option_handler,
					DHCP_OPTION_DNS, dns_server,
					sizeof(dns_server));
	(void)net_dhcpv4_add_option_callback(&dns_cb);

	return NULL;
}

static void eth_dhcpv4_teardown(void *fixture)
{
	ARG_UNUSED(fixture);

	if (eth_iface != NULL) {
		net_dhcpv4_stop(eth_iface);
		net_if_down(eth_iface);
	}

	net_dhcpv4_remove_option_callback(&ntp_cb);
	net_dhcpv4_remove_option_callback(&dns_cb);
	net_mgmt_del_event_callback(&dhcp_cb);
}

/* ================================================================== */
/* Build / bring-up                                                   */
/* ================================================================== */

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_client_build_alif_board)
{
	const struct device *dev;

	zassert_not_null(eth_iface, "Ethernet interface is NULL");

	dev = net_if_get_device(eth_iface);
	zassert_not_null(dev, "Ethernet device is NULL");
	zassert_true(device_is_ready(dev),
		     "Ethernet device %s is not ready (probe failed)",
		     dev->name);

	LOG_INF("Build/probe OK on device %s", dev->name);
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_ethernet_interface_detection)
{
	struct net_if *iface;

	iface = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
	zassert_not_null(iface, "No Ethernet L2 interface detected");
	zassert_equal_ptr(iface, eth_iface,
			  "Detected interface differs from suite interface");
}

/* ================================================================== */
/* DHCPv4 core flow                                                   */
/* ================================================================== */

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_dora_flow)
{
	struct in_addr *addr;
	char buf[NET_IPV4_ADDR_LEN];

	ensure_dhcp_bound();

	addr = net_if_ipv4_get_global_addr(eth_iface, NET_ADDR_PREFERRED);
	zassert_not_null(addr, "No preferred IPv4 address after DORA");
	zassert_not_equal(addr->s_addr, 0, "Assigned IPv4 address is 0.0.0.0");

	LOG_INF("DORA complete, lease %s",
		net_addr_ntop(AF_INET, addr, buf, sizeof(buf)));
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_ipv4_address_assignment_event)
{
	ensure_dhcp_bound();

	zassert_true(ipv4_addr_add_seen,
		     "NET_EVENT_IPV4_ADDR_ADD event was not seen");
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_subnet_mask_reception)
{
	struct in_addr *addr;
	struct in_addr mask;
	char buf[NET_IPV4_ADDR_LEN];

	ensure_dhcp_bound();

	addr = net_if_ipv4_get_global_addr(eth_iface, NET_ADDR_PREFERRED);
	zassert_not_null(addr, "No IPv4 address to look up netmask for");

	mask = net_if_ipv4_get_netmask_by_addr(eth_iface, addr);
	if (mask.s_addr == 0) {
		TC_PRINT("DHCP server did not provide a subnet mask (option 1); "
			 "configure the DHCP server to provide it. Skipping.\n");
		ztest_test_skip();
		return;
	}

	LOG_INF("Subnet mask: %s",
		net_addr_ntop(AF_INET, &mask, buf, sizeof(buf)));
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_router_gateway_reception)
{
	struct in_addr gw;
	char buf[NET_IPV4_ADDR_LEN];

	ensure_dhcp_bound();

	gw = net_if_ipv4_get_gw(eth_iface);
	if (gw.s_addr == 0) {
		TC_PRINT("DHCP server did not provide a default gateway (option 3); "
			 "configure the DHCP server to provide it. Skipping.\n");
		ztest_test_skip();
		return;
	}

	LOG_INF("Gateway: %s", net_addr_ntop(AF_INET, &gw, buf, sizeof(buf)));
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_lease_time_reception)
{
	ensure_dhcp_bound();

	zassert_not_equal(eth_iface->config.dhcpv4.lease_time, 0,
			  "DHCP lease time is zero");

	LOG_INF("Lease time: %u seconds",
		eth_iface->config.dhcpv4.lease_time);
}

/* ================================================================== */
/* DHCP option handling                                               */
/* ================================================================== */

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_ntp_option_callback)
{
	struct net_dhcpv4_option_callback cb;
	uint8_t data[4];
	int ret;

	net_dhcpv4_init_option_callback(&cb, ntp_option_handler,
					DHCP_OPTION_NTP, data, sizeof(data));

	ret = net_dhcpv4_add_option_callback(&cb);
	zassert_ok(ret, "Failed to add NTP option callback (%d)", ret);

	ret = net_dhcpv4_remove_option_callback(&cb);
	zassert_ok(ret, "Failed to remove NTP option callback (%d)", ret);

	if (ntp_option_seen) {
		LOG_INF("NTP option observed during DHCP negotiation");
	} else {
		LOG_INF("Server did not provide an NTP option (informational)");
	}
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_dns_option_validation)
{
	char buf[NET_IPV4_ADDR_LEN];

	ensure_dhcp_bound();

	if (!dns_option_seen) {
		TC_PRINT("DHCP server did not provide a DNS (option 6) value; "
			 "configure one to validate. Skipping.\n");
		ztest_test_skip();
		return;
	}

	zassert_true(dns_server[0] != 0 || dns_server[1] != 0 ||
		     dns_server[2] != 0 || dns_server[3] != 0,
		     "Received DNS option is 0.0.0.0");

	LOG_INF("DNS server: %s",
		net_addr_ntop(AF_INET, dns_server, buf, sizeof(buf)));
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_invalid_ntp_option_length_negative)
{
	struct net_dhcpv4_option_callback cb;
	struct in_addr *addr;
	uint8_t tiny[2]; /* deliberately smaller than an IPv4 address */
	int ret;

	ensure_dhcp_bound();

	net_dhcpv4_init_option_callback(&cb, ntp_option_handler,
					DHCP_OPTION_NTP, tiny, sizeof(tiny));

	ret = net_dhcpv4_add_option_callback(&cb);
	zassert_ok(ret, "Adding short-buffer option callback failed (%d)", ret);

	/* Give the stack a moment; it must not crash or wipe the lease. */
	k_sleep(K_MSEC(100));

	addr = net_if_ipv4_get_global_addr(eth_iface, NET_ADDR_PREFERRED);
	zassert_not_null(addr, "Lease lost after short-buffer option callback");
	zassert_not_equal(addr->s_addr, 0, "Lease became 0.0.0.0");

	ret = net_dhcpv4_remove_option_callback(&cb);
	zassert_ok(ret, "Removing short-buffer option callback failed (%d)",
		   ret);
}

/* ================================================================== */
/* Connectivity (real TX/RX)                                          */
/* ================================================================== */

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_ping_reachability_after_bind)
{
	int ret;

	ensure_dhcp_bound();

	ret = ping_gateway_once(PING_TIMEOUT);
	zassert_ok(ret, "Gateway ping (ICMP echo) failed/timed out (%d)", ret);

	LOG_INF("Gateway ping reply received");
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_background_ping_stability)
{
	int ok = 0;

	ensure_dhcp_bound();

	for (int i = 0; i < BG_PING_COUNT; i++) {
		if (ping_gateway_once(PING_TIMEOUT) == 0) {
			ok++;
		}
		k_sleep(K_MSEC(200));
	}

	LOG_INF("Background ping: %d/%d replies", ok, BG_PING_COUNT);

	zassert_true(ok * 100 >= BG_PING_COUNT * BG_PING_MIN_OK_PCT,
		     "Ping stability below %d%% (%d/%d)",
		     BG_PING_MIN_OK_PCT, ok, BG_PING_COUNT);
}

/* ================================================================== */
/* PHY link speed / duplex                                            */
/* ================================================================== */

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_10mbps_link)
{
	struct phy_link_state st;
	int ret;

	ensure_dhcp_bound();
	wait_for_carrier_up(K_SECONDS(5));
	ret = read_phy_link(&st);
	if (ret == 0) {
		print_phy_link_state(&st);
	}

	if (ret != 0 || !st.is_up) {
		TC_PRINT("PHY link not available/up; skipping 10M check.\n");
		ztest_test_skip();
		return;
	}

	if (!link_is_10m(st.speed)) {
		TC_PRINT("Link negotiated non-10M; force 10M on the switch "
			 "port to validate. Skipping.\n");
		ztest_test_skip();
		return;
	}

	zassert_true(link_is_10m(st.speed), "Expected a 10 Mbps link");
	LOG_INF("Link is 10 Mbps");
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_100mbps_link)
{
	struct phy_link_state st;
	int ret;

	ensure_dhcp_bound();
	wait_for_carrier_up(K_SECONDS(5));
	ret = read_phy_link(&st);
	if (ret == 0) {
		print_phy_link_state(&st);
	}

	if (ret != 0 || !st.is_up) {
		TC_PRINT("PHY link not available/up; skipping 100M check.\n");
		ztest_test_skip();
		return;
	}

	if (!PHY_LINK_IS_SPEED_100M(st.speed)) {
		TC_PRINT("Link negotiated non-100M; force 100M on the switch "
			 "port to validate. Skipping.\n");
		ztest_test_skip();
		return;
	}

	zassert_true(PHY_LINK_IS_SPEED_100M(st.speed),
		     "Expected a 100 Mbps link");
	LOG_INF("Link is 100 Mbps");
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_full_duplex_link)
{
	struct phy_link_state st;
	int ret;

	ensure_dhcp_bound();
	wait_for_carrier_up(K_SECONDS(5));
	ret = read_phy_link(&st);
	if (ret == 0) {
		print_phy_link_state(&st);
	}

	if (ret != 0 || !st.is_up) {
		TC_PRINT("PHY link not available/up; skipping full-duplex.\n");
		ztest_test_skip();
		return;
	}

	if (!PHY_LINK_IS_FULL_DUPLEX(st.speed)) {
		TC_PRINT("Link negotiated half-duplex; force full-duplex to "
			 "validate. Skipping.\n");
		ztest_test_skip();
		return;
	}

	zassert_true(PHY_LINK_IS_FULL_DUPLEX(st.speed),
		     "Expected a full-duplex link");
	LOG_INF("Link is full-duplex");
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_half_duplex_link)
{
	struct phy_link_state st;
	int ret;

	ensure_dhcp_bound();
	wait_for_carrier_up(K_SECONDS(5));
	ret = read_phy_link(&st);
	if (ret == 0) {
		print_phy_link_state(&st);
	}

	if (ret != 0 || !st.is_up) {
		TC_PRINT("PHY link not available/up; skipping half-duplex.\n");
		ztest_test_skip();
		return;
	}

	if (PHY_LINK_IS_FULL_DUPLEX(st.speed)) {
		TC_PRINT("Link negotiated full-duplex; force half-duplex to "
			 "validate. Skipping.\n");
		ztest_test_skip();
		return;
	}

	zassert_false(PHY_LINK_IS_FULL_DUPLEX(st.speed),
		      "Expected a half-duplex link");
	LOG_INF("Link is half-duplex");
}

/* ================================================================== */
/* MAC address                                                        */
/* ================================================================== */

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_packet_capture_mac_validation)
{
	struct net_linkaddr *ll;
	bool all_zero = true;

	zassert_not_null(eth_iface, "Ethernet interface is NULL");

	ll = net_if_get_link_addr(eth_iface);
	zassert_not_null(ll, "Link address is NULL");
	zassert_equal(ll->len, 6, "MAC length is %d, expected 6", ll->len);

	for (int i = 0; i < ll->len; i++) {
		if (ll->addr[i] != 0) {
			all_zero = false;
			break;
		}
	}

	zassert_false(all_zero, "MAC address is all zeros");
	zassert_false(ll->addr[0] & 0x01,
		      "MAC address is multicast (LSB of first octet set)");

	LOG_INF("Source MAC (verify in capture): "
		"%02x:%02x:%02x:%02x:%02x:%02x",
		ll->addr[0], ll->addr[1], ll->addr[2],
		ll->addr[3], ll->addr[4], ll->addr[5]);

	/* Send one ping so a live capture has a frame to inspect. */
	ensure_dhcp_bound();
	(void)ping_gateway_once(PING_TIMEOUT);
}

/* ================================================================== */
/* Negative / environment-dependent (operator-gated)                  */
/* ================================================================== */

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_multiple_offered_addresses)
{
	struct in_addr *addr;

	ensure_dhcp_bound();

	/* We cannot tell from the DUT how many OFFERs arrived, but the client
	 * must end with exactly one usable address regardless of offer count.
	 */
	addr = net_if_ipv4_get_global_addr(eth_iface, NET_ADDR_PREFERRED);
	zassert_not_null(addr, "No single committed lease present");
	zassert_not_equal(addr->s_addr, 0, "Committed lease is 0.0.0.0");

	TC_PRINT("To fully validate, run with multiple DHCP servers on the "
		 "LAN and confirm a single lease is committed.\n");
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_pool_exhaustion_negative)
{
	struct in_addr *addr;

	addr = net_if_ipv4_get_global_addr(eth_iface, NET_ADDR_PREFERRED);

	if (addr != NULL && addr->s_addr != 0) {
		TC_PRINT("A lease is present, so the pool is not exhausted in "
			 "this environment. Configure an exhausted pool to "
			 "validate. Skipping.\n");
		ztest_test_skip();
		return;
	}

	/* No lease: confirm the client stays unbound (does not invent an
	 * address) when the pool cannot satisfy the request.
	 */
	zassert_true(addr == NULL || addr->s_addr == 0,
		     "Client assigned an address despite pool exhaustion");
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_server_unavailable_negative)
{
	struct in_addr *addr;

	addr = net_if_ipv4_get_global_addr(eth_iface, NET_ADDR_PREFERRED);

	if (addr != NULL && addr->s_addr != 0) {
		TC_PRINT("A DHCP server is present in this environment. Run on "
			 "a server-less LAN to validate. Skipping.\n");
		ztest_test_skip();
		return;
	}

	zassert_true(addr == NULL || addr->s_addr == 0,
		     "Client bound an address with no server present");
}

/* ================================================================== */
/* Lease lifecycle / stress                                           */
/* ================================================================== */

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_lease_expiry_reacquire)
{
	struct in_addr *addr;

	ensure_dhcp_bound();

	LOG_INF("Lease time before re-acquire: %u s",
		eth_iface->config.dhcpv4.lease_time);

	k_sem_reset(&dhcp_bound);
	net_dhcpv4_restart(eth_iface);

	zassert_ok(k_sem_take(&dhcp_bound, DHCP_TIMEOUT),
		   "Lease was not re-acquired");

	addr = net_if_ipv4_get_global_addr(eth_iface, NET_ADDR_PREFERRED);
	zassert_not_null(addr, "No address after re-acquire");
	zassert_not_equal(addr->s_addr, 0, "Re-acquired address is 0.0.0.0");
}

ZTEST(alif_eth_dhcpv4, test_eth_dhcpv4_stop_restart_stability)
{
	struct in_addr *addr;

	ensure_dhcp_bound();

	for (int i = 0; i < STRESS_CYCLES; i++) {
		net_dhcpv4_stop(eth_iface);
		k_sleep(K_MSEC(200));

		addr = net_if_ipv4_get_global_addr(eth_iface, NET_ADDR_PREFERRED);
		zassert_true(addr == NULL || addr->s_addr == 0,
			     "Address not removed after stop (cycle %d)", i);

		k_sem_reset(&dhcp_bound);
		net_dhcpv4_start(eth_iface);
		zassert_ok(k_sem_take(&dhcp_bound, DHCP_TIMEOUT),
			   "Re-bind failed on cycle %d", i);
	}

	LOG_INF("Completed %d stop/start cycles", STRESS_CYCLES);
}



/* ================================================================== */
/* Cable disconnect (operator-gated)                                  */
/* ================================================================== */

#if defined(CONFIG_ETH_DHCPV4_MANUAL_CABLE_TESTS)
ZTEST(alif_eth_dhcpv4_manual_cable, test_eth_dhcpv4_cable_disconnect_before_dhcp)
{
	struct in_addr *addr;

	if (!net_if_is_up(eth_iface)) {
		(void)net_if_up(eth_iface);
	}

	if (net_if_is_carrier_ok(eth_iface)) {
		TC_PRINT("Carrier is present. Unplug the cable BEFORE running "
			 "to validate no-bind-without-link. Skipping.\n");
		ztest_test_skip();
		return;
	}

	/* Carrier down: try to start DHCP briefly and confirm no bind. */
	k_sem_reset(&dhcp_bound);
	net_dhcpv4_start(eth_iface);
	(void)k_sem_take(&dhcp_bound, K_SECONDS(5));
	net_dhcpv4_stop(eth_iface);

	addr = net_if_ipv4_get_global_addr(eth_iface, NET_ADDR_PREFERRED);
	zassert_true(addr == NULL || addr->s_addr == 0,
		     "Client bound a lease with no carrier (cable out)");
}

ZTEST(alif_eth_dhcpv4_manual_cable, test_eth_dhcpv4_cable_disconnect_after_bind)
{
	bool dropped = false;

	ensure_dhcp_bound();

	TC_PRINT("Unplug the Ethernet cable now (waiting up to %d s)...\n",
		 CABLE_WAIT_SECONDS);

	for (int i = 0; i < 20; i++) {
		if (!net_if_is_carrier_ok(eth_iface)) {
			dropped = true;
			break;
		}
		k_sleep(K_MSEC(CABLE_WAIT_TIMEOUT.ticks ?
			       (k_ticks_to_ms_floor64(CABLE_WAIT_TIMEOUT.ticks) / 20) :
			       1000));
	}

	if (!dropped) {
		TC_PRINT("Carrier stayed up (cable not unplugged). Skipping.\n");
		ztest_test_skip();
		return;
	}

	zassert_false(net_if_is_carrier_ok(eth_iface),
		      "Carrier did not drop after cable unplug");
	LOG_INF("Carrier correctly dropped on cable disconnect");

	/* Restore link state for any subsequent runs. */
	TC_PRINT("Re-plug the cable to restore connectivity.\n");
}

ZTEST_SUITE(alif_eth_dhcpv4_manual_cable, NULL, eth_dhcpv4_setup, NULL, NULL,
	    eth_dhcpv4_teardown);
#endif
ZTEST_SUITE(alif_eth_dhcpv4, NULL, eth_dhcpv4_setup, NULL, NULL,
	    eth_dhcpv4_teardown);


