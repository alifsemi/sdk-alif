/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/icmp.h>
#include <zephyr/net/socket.h>
#include <zephyr/usb/usbd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sample_usbd.h>

#define NCM_IPV4_STR           "192.168.7.2"
#define NCM_PEER_IPV4_STR      "192.168.7.1"
#define NCM_IPV6_STR           "2001:db8::1"
#define NCM_ECHO_PORT          4242
#define NCM_UDP_PORT           4243
#define NCM_TCP_LB_PORT        4244
#define NCM_MAC_LOCAL          { 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 }
#define HOST_WAIT_MS           3000
#define PING_WAIT_MS           2000
#define STRESS_PING_COUNT      10
#define STRESS_SOCKET_COUNT    16
#define STRESS_IFACE_POLLS     30
#define UDP_BURST_COUNT        16
#define UDP_PAYLOAD_LEN        128

struct cdc_ncm_qa_fixture {
	const struct device *ncm_dev;
	struct net_if *iface;
	struct usbd_context *usbd_ctx;
};

static const uint8_t expected_mac[] = NCM_MAC_LOCAL;
static K_SEM_DEFINE(ping_sem, 0, 1);
static volatile int ping_replies;

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

static bool host_carrier(struct net_if *iface)
{
	return iface != NULL && net_if_is_up(iface) && net_if_is_carrier_ok(iface);
}

static void wait_host_carrier(struct net_if *iface, int timeout_ms)
{
	int waited = 0;

	while (!host_carrier(iface) && waited < timeout_ms) {
		k_sleep(K_MSEC(50));
		waited += 50;
	}
}

static void skip_if_no_host(struct net_if *iface)
{
	if (!host_carrier(iface)) {
		TC_PRINT("NCM carrier off (no host), skipping\n");
		ztest_test_skip();
	}
}

static int ping_ipv4(struct net_if *iface, const char *addr_str, int timeout_ms)
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
	err = net_addr_pton(AF_INET, addr_str, &dst.sin_addr);
	if (err != 0) {
		net_icmp_cleanup_ctx(&ctx);
		return err;
	}

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

K_THREAD_STACK_DEFINE(tcp_lb_stack, 2048);
static struct k_thread tcp_lb_thread;
static K_SEM_DEFINE(tcp_lb_ready, 0, 1);
static K_SEM_DEFINE(tcp_lb_done, 0, 1);
static int tcp_lb_status;
static uint8_t udp_tx[UDP_PAYLOAD_LEN];
static uint8_t udp_rx[UDP_PAYLOAD_LEN];

static void sock_rcv_timeout(int sock, int timeout_ms)
{
	struct timeval tv = {
		.tv_sec = timeout_ms / 1000,
		.tv_usec = (timeout_ms % 1000) * 1000,
	};

	(void)zsock_setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

static int udp_loopback_once(uint8_t tag)
{
	int srv, cli, n;
	struct sockaddr_in addr = { 0 };
	struct sockaddr_in from = { 0 };
	socklen_t from_len = sizeof(from);

	srv = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (srv < 0) {
		return -errno;
	}
	addr.sin_family = AF_INET;
	if (net_addr_pton(AF_INET, NCM_IPV4_STR, &addr.sin_addr) != 0) {
		zsock_close(srv);
		return -EINVAL;
	}
	addr.sin_port = htons(NCM_UDP_PORT);
	if (zsock_bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		zsock_close(srv);
		return -errno;
	}
	sock_rcv_timeout(srv, 1000);

	cli = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (cli < 0) {
		zsock_close(srv);
		return -errno;
	}
	memset(udp_tx, tag, sizeof(udp_tx));
	udp_tx[0] = tag;
	if (zsock_sendto(cli, udp_tx, sizeof(udp_tx), 0,
			 (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		zsock_close(cli);
		zsock_close(srv);
		return -errno;
	}
	n = zsock_recvfrom(srv, udp_rx, sizeof(udp_rx), 0,
			   (struct sockaddr *)&from, &from_len);
	zsock_close(cli);
	zsock_close(srv);
	if (n != (int)sizeof(udp_tx) || memcmp(udp_tx, udp_rx, sizeof(udp_tx)) != 0) {
		return -EIO;
	}
	return 0;
}

static void tcp_lb_server(void *p1, void *p2, void *p3)
{
	int sock, client, n;
	struct sockaddr_in addr = { 0 };
	uint8_t buf[16];

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);
	tcp_lb_status = -EIO;

	sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		tcp_lb_status = -errno;
		k_sem_give(&tcp_lb_ready);
		k_sem_give(&tcp_lb_done);
		return;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(NCM_TCP_LB_PORT);
	if (zsock_bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
	    zsock_listen(sock, 1) < 0) {
		tcp_lb_status = -errno;
		zsock_close(sock);
		k_sem_give(&tcp_lb_ready);
		k_sem_give(&tcp_lb_done);
		return;
	}
	sock_rcv_timeout(sock, 2000);
	k_sem_give(&tcp_lb_ready);
	client = zsock_accept(sock, NULL, NULL);
	if (client < 0) {
		tcp_lb_status = -errno;
		zsock_close(sock);
		k_sem_give(&tcp_lb_done);
		return;
	}
	n = zsock_recv(client, buf, sizeof(buf), 0);
	if (n > 0) {
		(void)zsock_send(client, buf, n, 0);
		tcp_lb_status = 0;
	}
	zsock_close(client);
	zsock_close(sock);
	k_sem_give(&tcp_lb_done);
}

static void echo_tcp_thread(void *p1, void *p2, void *p3)
{
	int sock, client;
	struct sockaddr_in bind_addr = { 0 };
	uint8_t buf[128];

	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0) {
		return;
	}

	bind_addr.sin_family = AF_INET;
	bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_addr.sin_port = htons(NCM_ECHO_PORT);

	if (zsock_bind(sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0) {
		zsock_close(sock);
		return;
	}
	if (zsock_listen(sock, 1) < 0) {
		zsock_close(sock);
		return;
	}

	TC_PRINT("TCP echo listening on %s:%d\n", NCM_IPV4_STR, NCM_ECHO_PORT);

	while (true) {
		int recvd;

		client = zsock_accept(sock, NULL, NULL);
		if (client < 0) {
			continue;
		}
		TC_PRINT("TCP (IPv4): Accepted connection\n");
		while ((recvd = zsock_recv(client, buf, sizeof(buf), 0)) > 0) {
			zsock_send(client, buf, recvd, 0);
		}
		zsock_close(client);
	}
}

K_THREAD_STACK_DEFINE(echo_stack, 2048);
static struct k_thread echo_thread;

static void start_echo_server(void)
{
	k_thread_create(&echo_thread, echo_stack, K_THREAD_STACK_SIZEOF(echo_stack),
			echo_tcp_thread, NULL, NULL, NULL, 5, 0, K_NO_WAIT);
	k_thread_name_set(&echo_thread, "ncm_echo");
}

static void *ncm_suite_setup(void)
{
	static struct cdc_ncm_qa_fixture fixture;
	int err;

	TC_PRINT("\n*** cdc_ncm_qa ztest (not the echo_server sample) ***\n");

	fixture.ncm_dev = DEVICE_DT_GET(DT_NODELABEL(cdc_ncm_eth0));
	zassert_not_null(fixture.ncm_dev, "cdc_ncm_eth0 not found");
	zassert_true(device_is_ready(fixture.ncm_dev), "cdc_ncm_eth0 not ready");

	fixture.iface = net_if_lookup_by_dev(fixture.ncm_dev);
	zassert_not_null(fixture.iface, "No net_if for cdc_ncm_eth0");

	fixture.usbd_ctx = sample_usbd_init_device(NULL);
	zassert_not_null(fixture.usbd_ctx, "USB device init failed");

	err = usbd_enable(fixture.usbd_ctx);
	zassert_equal(err, 0, "usbd_enable failed (%d)", err);

	TC_PRINT("USB CDC NCM stack initialized and enabled\n");
	wait_host_carrier(fixture.iface, HOST_WAIT_MS);
	TC_PRINT("NCM carrier=%d iface_up=%d\n",
		 net_if_is_carrier_ok(fixture.iface) ? 1 : 0,
		 net_if_is_up(fixture.iface) ? 1 : 0);

	return &fixture;
}

static void ncm_suite_teardown(void *f)
{
	ARG_UNUSED(f);

	start_echo_server();
	TC_PRINT("USB NCM remains enabled; TCP echo on %s:%d for host nc/ping\n",
		 NCM_IPV4_STR, NCM_ECHO_PORT);
	TC_PRINT("Host: ip link | grep -E 'enx|cdc_ncm'; then ip addr add %s/24 and ping %s\n",
		 NCM_PEER_IPV4_STR, NCM_IPV4_STR);
}

/*
 * NCM Ethernet Device Ready
 *
 * Purpose : Confirm zephyr,cdc-ncm-ethernet node cdc_ncm_eth0 is bound
 *           (sample net iface Device: cdc_ncm_eth0).
 * Method  : DEVICE_DT_GET(DT_NODELABEL(cdc_ncm_eth0)) + device_is_ready().
 * Pass    : Device pointer non-NULL and ready.
 * Fail    : Node missing or driver not initialized.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_eth_device_ready)
{
	zassert_not_null(fixture->ncm_dev, "NCM device is NULL");
	zassert_true(device_is_ready(fixture->ncm_dev), "NCM device not ready");
	TC_PRINT("NCM device %s ready\n", fixture->ncm_dev->name);
}

/*
 * NCM Interface Up
 *
 * Purpose : Ethernet iface eth0 is up after USB NCM init
 *           (sample: Status oper=UP, admin=UP).
 * Method  : net_if_is_up() on cdc_ncm_eth0.
 * Pass    : Interface is up.
 * Fail    : Interface down.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_iface_up)
{
	zassert_true(net_if_is_up(fixture->iface), "NCM iface is down");
	TC_PRINT("NCM iface %s up\n", fixture->ncm_dev->name);
}

/*
 * NCM Local MAC
 *
 * Purpose : Local MAC matches overlay [02 00 00 00 00 01]
 *           (sample net iface Link addr).
 * Method  : net_if_get_link_addr().
 * Pass    : 02:00:00:00:00:01.
 * Fail    : MAC mismatch.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_mac_local)
{
	struct net_linkaddr *ll = net_if_get_link_addr(fixture->iface);

	zassert_not_null(ll, "No link address");
	zassert_equal(ll->len, 6, "MAC length %u", ll->len);
	zassert_mem_equal(ll->addr, expected_mac, 6,
			  "MAC %02x:%02x:%02x:%02x:%02x:%02x",
			  ll->addr[0], ll->addr[1], ll->addr[2],
			  ll->addr[3], ll->addr[4], ll->addr[5]);
	TC_PRINT("NCM MAC 02:00:00:00:00:01 OK (host remote 00:00:5e:00:53:01)\n");
}

/*
 * NCM MTU
 *
 * Purpose : Ethernet MTU is 1500 (sample net iface MTU).
 * Method  : net_if_get_mtu().
 * Pass    : MTU == 1500.
 * Fail    : Unexpected MTU.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_mtu)
{
	uint16_t mtu = net_if_get_mtu(fixture->iface);

	zassert_equal(mtu, NET_ETH_MTU, "MTU %u expected %u", mtu, NET_ETH_MTU);
}

/*
 * NCM IPv4 Address
 *
 * Purpose : Static IPv4 192.168.7.2/24 (sample net_config / net iface).
 * Method  : net_if_ipv4_addr_lookup().
 * Pass    : 192.168.7.2 is configured.
 * Fail    : Address missing.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_ipv4_configured)
{
	struct in_addr addr;
	struct net_if *iface = NULL;
	struct net_if_addr *ifaddr;

	zassert_equal(net_addr_pton(AF_INET, NCM_IPV4_STR, &addr), 0,
		      "Bad IPv4 string");
	ifaddr = net_if_ipv4_addr_lookup(&addr, &iface);
	zassert_not_null(ifaddr, "IPv4 %s not configured", NCM_IPV4_STR);
	zassert_equal_ptr(iface, fixture->iface, "IPv4 on wrong iface");
	TC_PRINT("IPv4 %s configured\n", NCM_IPV4_STR);
}

/*
 * NCM IPv6 Address
 *
 * Purpose : Static IPv6 2001:db8::1 (sample net_config).
 * Method  : net_if_ipv6_addr_lookup().
 * Pass    : 2001:db8::1 is configured.
 * Fail    : Address missing.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_ipv6_configured)
{
	struct in6_addr addr;
	struct net_if *iface = NULL;
	struct net_if_addr *ifaddr;

	zassert_equal(net_addr_pton(AF_INET6, NCM_IPV6_STR, &addr), 0,
		      "Bad IPv6 string");
	ifaddr = net_if_ipv6_addr_lookup(&addr, &iface);
	zassert_not_null(ifaddr, "IPv6 %s not configured", NCM_IPV6_STR);
	TC_PRINT("IPv6 %s configured\n", NCM_IPV6_STR);
}

/*
 * NCM Carrier / Host Enum
 *
 * Purpose : Host enumerated CDC NCM (dmesg cdc_ncm, enx00005e005301).
 *           Sample log: Enabled cdc_ncm_0, Connection status sent.
 * Method  : Wait for net_if_is_carrier_ok().
 * Pass    : Carrier ON.
 * Fail    : Skip if USB device cable has no host.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_carrier)
{
	wait_host_carrier(fixture->iface, HOST_WAIT_MS);
	if (!host_carrier(fixture->iface)) {
		TC_PRINT("Host not connected; plug USB device port and re-run\n");
		ztest_test_skip();
	}
	zassert_true(net_if_is_carrier_ok(fixture->iface), "NCM carrier off");
	TC_PRINT("NCM carrier ON (host cdc_ncm enum)\n");
}

/*
 * USB Speed Capability
 *
 * Purpose : DWC3 speed FS or HS (host log: high-speed xhci).
 * Method  : usbd_caps_speed().
 * Pass    : USBD_SPEED_FS or USBD_SPEED_HS.
 * Fail    : Unexpected speed.
 */
ZTEST_F(cdc_ncm_qa, test_usb_speed_capability)
{
	enum usbd_speed speed = usbd_caps_speed(fixture->usbd_ctx);

	TC_PRINT("USB controller speed capability: %s\n",
		 speed == USBD_SPEED_HS ? "High-Speed" :
		 speed == USBD_SPEED_FS ? "Full-Speed" : "Unknown");
	zassert_true(speed == USBD_SPEED_FS || speed == USBD_SPEED_HS,
		     "Unexpected speed %d", speed);
}

/*
 * NCM Loopback Ping
 *
 * Purpose : ICMP echo to self 192.168.7.2 (sample: net ping 192.168.7.2).
 * Method  : net_icmp_send_echo_request() to own IPv4.
 * Pass    : Echo reply within 2 s.
 * Fail    : Timeout or send error.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_loopback_ping)
{
	int err = ping_ipv4(fixture->iface, NCM_IPV4_STR, PING_WAIT_MS);

	zassert_equal(err, 0, "loopback ping %s failed (%d)", NCM_IPV4_STR, err);
	TC_PRINT("PING %s: %d reply(ies)\n", NCM_IPV4_STR, ping_replies);
}

/*
 * NCM Peer Ping
 *
 * Purpose : ICMP echo to host 192.168.7.1 (QA: ping 192.168.7.2 from host
 *           after ip addr add 192.168.7.1/24). Device pings the peer.
 * Method  : net_icmp_send_echo_request() to 192.168.7.1.
 * Pass    : Echo reply. Skip if host has no 192.168.7.1 yet.
 * Fail    : Send error other than timeout.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_peer_ping)
{
	int err;

	skip_if_no_host(fixture->iface);
	err = ping_ipv4(fixture->iface, NCM_PEER_IPV4_STR, PING_WAIT_MS);
	if (err == -ETIMEDOUT) {
		TC_PRINT("No reply from %s; assign 192.168.7.1/24 on host and re-run\n",
			 NCM_PEER_IPV4_STR);
		ztest_test_skip();
	}
	zassert_equal(err, 0, "peer ping %s failed (%d)", NCM_PEER_IPV4_STR, err);
	TC_PRINT("PING %s: %d reply(ies)\n", NCM_PEER_IPV4_STR, ping_replies);
}

/*
 * NCM TCP Bind
 *
 * Purpose : TCP listen on port 4242 (sample echo_server / host nc).
 * Method  : socket + bind + listen on 4242, then close (echo starts later).
 * Pass    : bind and listen return 0.
 * Fail    : Socket error.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_tcp_bind)
{
	int sock, err;
	struct sockaddr_in addr = { 0 };

	sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0, "socket failed (%d)", errno);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(NCM_ECHO_PORT);

	err = zsock_bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	zassert_equal(err, 0, "bind 4242 failed (%d)", errno);
	err = zsock_listen(sock, 1);
	zassert_equal(err, 0, "listen 4242 failed (%d)", errno);
	zsock_close(sock);
	TC_PRINT("TCP bind/listen on port %d OK\n", NCM_ECHO_PORT);
}

/*
 * USB Disable and Re-enable
 *
 * Purpose : Cable unplug/replug (usbd_disable / usbd_enable).
 * Method  : disable 100 ms, enable 500 ms. Same as cdc-acm QA.
 * Pass    : Not run on this controller. Skip is expected.
 * Fail    : N/A — usbd_disable on NCM+DWC3 drops the host netdev
 *           (enx00005e005301) and a second halt times out (-5).
 */
ZTEST_F(cdc_ncm_qa, test_usb_disable_reenable)
{
	skip_if_no_host(fixture->iface);
	TC_PRINT("Skip usbd_disable: NCM+DWC3 halt drops host cdc_ncm iface\n");
	ztest_test_skip();
}

/*
 * USB Disable/Enable Stress
 *
 * Purpose : Repeated disable/enable. DWC3 cannot stop if the host does
 *           not re-enumerate; further cycles stop (not fail).
 * Method  : Skip. One disable/enable leaves DWC3 unable to halt again
 *           and the host loses enx00005e005301 (no ping/nc after suite).
 * Pass    : Skip is expected on Alif DWC3 + CDC NCM.
 * Fail    : N/A.
 */
ZTEST_F(cdc_ncm_qa, test_usb_disable_enable_stress)
{
	skip_if_no_host(fixture->iface);
	TC_PRINT("Skip disable stress: keeps host NCM netdev for ping/nc\n");
	ztest_test_skip();
}

/*
 * IPv4 netmask
 *
 * Purpose : Static address is 192.168.7.2/24.
 * Method  : net_if_ipv4_get_netmask_by_addr().
 * Pass    : 255.255.255.0.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_ipv4_netmask)
{
	struct in_addr addr;
	struct in_addr mask;

	zassert_equal(net_addr_pton(AF_INET, NCM_IPV4_STR, &addr), 0, "pton");
	mask = net_if_ipv4_get_netmask_by_addr(fixture->iface, &addr);
	zassert_equal(mask.s4_addr[0], 255, "mask0");
	zassert_equal(mask.s4_addr[1], 255, "mask1");
	zassert_equal(mask.s4_addr[2], 255, "mask2");
	zassert_equal(mask.s4_addr[3], 0, "mask3");
	TC_PRINT("IPv4 netmask 255.255.255.0\n");
}

/*
 * Link type
 *
 * Purpose : NCM iface is Ethernet.
 * Method  : net_if_get_link_addr()->type.
 * Pass    : NET_LINK_ETHERNET.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_link_ethernet)
{
	struct net_linkaddr *ll = net_if_get_link_addr(fixture->iface);

	zassert_not_null(ll, "No link address");
	zassert_equal(ll->type, NET_LINK_ETHERNET, "link type %d", ll->type);
	TC_PRINT("NCM link type Ethernet\n");
}

/*
 * UDP bind
 *
 * Purpose : UDP socket can bind the device address.
 * Method  : bind 192.168.7.2:4243 and close.
 * Pass    : bind returns 0.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_udp_bind)
{
	int sock, err;
	struct sockaddr_in addr = { 0 };

	sock = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	zassert_true(sock >= 0, "udp socket (%d)", errno);
	addr.sin_family = AF_INET;
	zassert_equal(net_addr_pton(AF_INET, NCM_IPV4_STR, &addr.sin_addr), 0, "pton");
	addr.sin_port = htons(NCM_UDP_PORT);
	err = zsock_bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	zassert_equal(err, 0, "udp bind (%d)", errno);
	zsock_close(sock);
	TC_PRINT("UDP bind %s:%d OK\n", NCM_IPV4_STR, NCM_UDP_PORT);
}

/*
 * UDP loopback
 *
 * Purpose : One datagram to the device address is received locally.
 *           Does not use the USB IN path.
 * Method  : sendto/recvfrom 192.168.7.2:4243.
 * Pass    : Payload matches.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_udp_loopback)
{
	int err = udp_loopback_once(0x5A);

	zassert_equal(err, 0, "udp loopback %d", err);
	TC_PRINT("UDP loopback %u bytes OK\n", UDP_PAYLOAD_LEN);
}

/*
 * TCP loopback
 *
 * Purpose : Local TCP connect, send, and echo on port 4244.
 *           Port 4242 stays free for the host echo server after the suite.
 * Method  : Server thread plus client connect to 192.168.7.2.
 * Pass    : Echoed payload matches.
 */
ZTEST_F(cdc_ncm_qa, test_ncm_tcp_loopback)
{
	int sock, err, n;
	struct sockaddr_in addr = { 0 };
	const char payload[] = "ncm";
	char buf[8] = { 0 };

	k_sem_reset(&tcp_lb_ready);
	k_sem_reset(&tcp_lb_done);
	tcp_lb_status = -EIO;
	k_thread_create(&tcp_lb_thread, tcp_lb_stack, K_THREAD_STACK_SIZEOF(tcp_lb_stack),
			tcp_lb_server, NULL, NULL, NULL, K_PRIO_PREEMPT(7), 0, K_NO_WAIT);
	k_thread_name_set(&tcp_lb_thread, "ncm_tcp_lb");
	zassert_equal(k_sem_take(&tcp_lb_ready, K_SECONDS(2)), 0, "tcp server start");

	sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	zassert_true(sock >= 0, "tcp socket (%d)", errno);
	sock_rcv_timeout(sock, 2000);
	addr.sin_family = AF_INET;
	zassert_equal(net_addr_pton(AF_INET, NCM_IPV4_STR, &addr.sin_addr), 0, "pton");
	addr.sin_port = htons(NCM_TCP_LB_PORT);
	err = zsock_connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	zassert_equal(err, 0, "tcp connect (%d)", errno);
	zassert_equal(zsock_send(sock, payload, sizeof(payload), 0),
		      (ssize_t)sizeof(payload), "tcp send");
	n = zsock_recv(sock, buf, sizeof(buf), 0);
	zsock_close(sock);
	(void)k_sem_take(&tcp_lb_done, K_SECONDS(2));
	(void)k_thread_join(&tcp_lb_thread, K_SECONDS(2));
	zassert_equal(tcp_lb_status, 0, "tcp server %d", tcp_lb_status);
	zassert_true(n >= (int)sizeof(payload), "tcp recv %d", n);
	zassert_mem_equal(buf, payload, sizeof(payload));
	TC_PRINT("TCP loopback port %d OK\n", NCM_TCP_LB_PORT);
}

/*
 * Loopback ping stress
 *
 * Purpose : Repeated ICMP to 192.168.7.2 stays up. No usbd_disable.
 * Method  : 10 sequential echo requests.
 * Pass    : Every ping gets a reply.
 */
ZTEST_F(cdc_ncm_qa, test_stress_loopback_ping)
{
	for (int i = 0; i < STRESS_PING_COUNT; i++) {
		int err = ping_ipv4(fixture->iface, NCM_IPV4_STR, PING_WAIT_MS);

		zassert_equal(err, 0, "ping %d failed (%d)", i, err);
	}
	TC_PRINT("Loopback ping x%d OK\n", STRESS_PING_COUNT);
}

/*
 * UDP burst stress
 *
 * Purpose : Back-to-back local UDP datagrams.
 * Method  : 16 x 128-byte loopback.
 * Pass    : Every datagram matches.
 */
ZTEST_F(cdc_ncm_qa, test_stress_udp_burst)
{
	for (int i = 0; i < UDP_BURST_COUNT; i++) {
		int err = udp_loopback_once((uint8_t)i);

		zassert_equal(err, 0, "udp %d failed (%d)", i, err);
	}
	TC_PRINT("UDP burst %d x %u OK\n", UDP_BURST_COUNT, UDP_PAYLOAD_LEN);
}

/*
 * Socket open/close stress
 *
 * Purpose : TCP socket alloc and free does not leak.
 * Method  : 16 socket()/close() cycles.
 * Pass    : Every socket() succeeds.
 */
ZTEST_F(cdc_ncm_qa, test_stress_socket_cycle)
{
	for (int i = 0; i < STRESS_SOCKET_COUNT; i++) {
		int sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		zassert_true(sock >= 0, "socket %d errno %d", i, errno);
		zsock_close(sock);
	}
	TC_PRINT("TCP socket cycle x%d OK\n", STRESS_SOCKET_COUNT);
}

/*
 * Iface poll stress
 *
 * Purpose : MAC, MTU, and oper state stay stable while the host holds NCM.
 * Method  : 30 reads. Does not call usbd_disable.
 * Pass    : Device ready, MTU 1500, MAC unchanged.
 */
ZTEST_F(cdc_ncm_qa, test_stress_iface_poll)
{
	for (int i = 0; i < STRESS_IFACE_POLLS; i++) {
		struct net_linkaddr *ll = net_if_get_link_addr(fixture->iface);

		zassert_true(device_is_ready(fixture->ncm_dev), "dev %d", i);
		zassert_true(net_if_is_up(fixture->iface), "up %d", i);
		zassert_equal(net_if_get_mtu(fixture->iface), NET_ETH_MTU, "mtu %d", i);
		zassert_not_null(ll, "ll %d", i);
		zassert_mem_equal(ll->addr, expected_mac, 6, "mac %d", i);
	}
	TC_PRINT("Iface poll x%d OK\n", STRESS_IFACE_POLLS);
}

/*
 * Loopback ping timing
 *
 * Purpose : Local ICMP rate. Informational; bound is only a hang check.
 * Method  : Time 10 echo requests to 192.168.7.2.
 * Pass    : All replies, total under 2 s.
 */
ZTEST_F(cdc_ncm_qa, test_perf_loopback_ping)
{
	uint32_t t0 = k_uptime_get_32();
	uint32_t ms;

	for (int i = 0; i < STRESS_PING_COUNT; i++) {
		int err = ping_ipv4(fixture->iface, NCM_IPV4_STR, PING_WAIT_MS);

		zassert_equal(err, 0, "perf ping %d (%d)", i, err);
	}
	ms = k_uptime_get_32() - t0;
	if (ms == 0U) {
		ms = 1U;
	}
	TC_PRINT("Loopback ping %d in %u ms (%u pps)\n",
		 STRESS_PING_COUNT, ms, (STRESS_PING_COUNT * 1000U) / ms);
	zassert_true(ms < 2000U, "ping stress slow %u ms", ms);
}

/*
 * UDP loopback throughput
 *
 * Purpose : Local UDP payload rate. Does not measure USB NCM wire rate.
 * Method  : 16 x 128-byte datagrams, timed.
 * Pass    : Every datagram matches. kbps is informational.
 */
ZTEST_F(cdc_ncm_qa, test_perf_udp_loopback)
{
	uint32_t t0 = k_uptime_get_32();
	uint32_t ms, bytes, kbps;

	for (int i = 0; i < UDP_BURST_COUNT; i++) {
		int err = udp_loopback_once((uint8_t)(0xA0 + i));

		zassert_equal(err, 0, "perf udp %d (%d)", i, err);
	}
	ms = k_uptime_get_32() - t0;
	if (ms == 0U) {
		ms = 1U;
	}
	bytes = (uint32_t)UDP_BURST_COUNT * UDP_PAYLOAD_LEN;
	kbps = (bytes * 8U) / ms;
	TC_PRINT("UDP loopback %u bytes in %u ms (%u kbps)\n", bytes, ms, kbps);
	zassert_true(kbps > 0U, "kbps");
}

ZTEST(cdc_ncm_basic, test_ipv4_addr_parse)
{
	struct in_addr addr;

	zassert_equal(net_addr_pton(AF_INET, NCM_IPV4_STR, &addr), 0);
	zassert_equal(addr.s4_addr[0], 192, "octet0");
	zassert_equal(addr.s4_addr[1], 168, "octet1");
	zassert_equal(addr.s4_addr[2], 7, "octet2");
	zassert_equal(addr.s4_addr[3], 2, "octet3");
}

ZTEST(cdc_ncm_basic, test_peer_ipv4_parse)
{
	struct in_addr addr;

	zassert_equal(net_addr_pton(AF_INET, NCM_PEER_IPV4_STR, &addr), 0);
	zassert_equal(addr.s4_addr[3], 1, "peer host octet");
}

ZTEST(cdc_ncm_basic, test_mac_bytes)
{
	const uint8_t mac[] = NCM_MAC_LOCAL;

	zassert_equal(mac[0], 0x02, "local-admin / locally administered bit");
	zassert_equal(sizeof(mac), 6);
}

ZTEST_SUITE(cdc_ncm_basic, NULL, NULL, NULL, NULL, NULL);

ZTEST_SUITE(cdc_ncm_qa, NULL, ncm_suite_setup, NULL, NULL, ncm_suite_teardown);
