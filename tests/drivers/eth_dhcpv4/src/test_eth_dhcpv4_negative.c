/* Copyright Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 */

/*
 * Negative test scenarios for the Alif Ethernet DHCPv4 stack.
 *
 * These tests deliberately feed invalid arguments to the public networking
 * APIs that the positive DHCPv4 suite relies on (ICMP echo / ping context and
 * the DHCPv4 option-callback registration) and confirm that each API rejects
 * the bad input with the documented error code instead of crashing or
 * silently succeeding.
 *
 * Only APIs that are documented to validate their arguments and return an
 * error code are exercised here. APIs that take ownership of a raw pointer
 * without NULL-checking (e.g. net_dhcpv4_start()/net_if_up()) are intentionally
 * NOT passed NULL, because that is undefined behaviour rather than a testable
 * negative path.
 *
 * Style follows the Alif driver negative tests (e.g. test_spi_negative.c):
 * Kconfig-gated, subtest_* helpers returning 0/1, and a single comprehensive
 * ZTEST that aggregates the per-subtest results.
 */

#include <zephyr/sys/util_macro.h>

#if IS_ENABLED(CONFIG_TEST_ETH_DHCPV4_NEGATIVE)

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/icmp.h>
#include <zephyr/net/dhcpv4.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(alif_eth_dhcpv4_neg, LOG_LEVEL_INF);

static struct net_if *neg_iface;

/* Dummy ICMP reply handler used only to build a *valid* context where the
 * test needs a non-NULL handler (so the failure under test is the bad
 * argument, not the handler).
 */
static int neg_icmp_handler(struct net_icmp_ctx *ctx, struct net_pkt *pkt,
			    struct net_icmp_ip_hdr *ip_hdr,
			    struct net_icmp_hdr *icmp_hdr, void *user_data)
{
	ARG_UNUSED(ctx);
	ARG_UNUSED(pkt);
	ARG_UNUSED(ip_hdr);
	ARG_UNUSED(icmp_hdr);
	ARG_UNUSED(user_data);

	return 0;
}

#if defined(CONFIG_NET_DHCPV4_OPTION_CALLBACKS)
/* Dummy DHCP option handler for building a valid (but unregistered) callback. */
static void neg_dhcp_option_handler(struct net_dhcpv4_option_callback *cb,
				    size_t length,
				    enum net_dhcpv4_msg_type msg_type,
				    struct net_if *iface)
{
	ARG_UNUSED(cb);
	ARG_UNUSED(length);
	ARG_UNUSED(msg_type);
	ARG_UNUSED(iface);
}
#endif /* CONFIG_NET_DHCPV4_OPTION_CALLBACKS */

/* ------------------------------------------------------------------ */
/* ICMP negative subtests                                             */
/* ------------------------------------------------------------------ */

/* net_icmp_init_ctx() must reject a NULL context. */
static int subtest_icmp_init_null_ctx(void)
{
	int ret = net_icmp_init_ctx(NULL, NET_ICMPV4_ECHO_REPLY, 0,
				    neg_icmp_handler);

	if (ret != -EINVAL) {
		LOG_ERR("  icmp_init_null_ctx: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

/* net_icmp_init_ctx() must reject a NULL handler. */
static int subtest_icmp_init_null_handler(void)
{
	struct net_icmp_ctx ctx;
	int ret = net_icmp_init_ctx(&ctx, NET_ICMPV4_ECHO_REPLY, 0, NULL);

	if (ret != -EINVAL) {
		LOG_ERR("  icmp_init_null_handler: expected -EINVAL, got %d",
			ret);
		return 1;
	}
	return 0;
}

/* net_icmp_cleanup_ctx() must reject a NULL context. */
static int subtest_icmp_cleanup_null(void)
{
	int ret = net_icmp_cleanup_ctx(NULL);

	if (ret != -EINVAL) {
		LOG_ERR("  icmp_cleanup_null: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

/* net_icmp_send_echo_request() must reject a NULL context. */
static int subtest_icmp_send_null_ctx(void)
{
	struct sockaddr_in dst = {
		.sin_family = AF_INET,
	};
	struct net_icmp_ping_params params = {0};
	int ret;

	(void)net_addr_pton(AF_INET, "192.0.2.1", &dst.sin_addr);

	ret = net_icmp_send_echo_request(NULL, neg_iface,
					 (struct sockaddr *)&dst,
					 &params, NULL);
	if (ret != -EINVAL) {
		LOG_ERR("  icmp_send_null_ctx: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

/* net_icmp_send_echo_request() must reject a NULL destination. */
static int subtest_icmp_send_null_dst(void)
{
	struct net_icmp_ctx ctx;
	struct net_icmp_ping_params params = {0};
	int ret;
	int init;

	init = net_icmp_init_ctx(&ctx, NET_ICMPV4_ECHO_REPLY, 0,
				 neg_icmp_handler);
	if (init != 0) {
		LOG_ERR("  icmp_send_null_dst: ctx init failed %d", init);
		return 1;
	}

	ret = net_icmp_send_echo_request(&ctx, neg_iface, NULL, &params, NULL);
	(void)net_icmp_cleanup_ctx(&ctx);

	if (ret != -EINVAL) {
		LOG_ERR("  icmp_send_null_dst: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

/* net_icmp_send_echo_request() must reject an out-of-range packet priority. */
static int subtest_icmp_send_bad_priority(void)
{
	struct net_icmp_ctx ctx;
	struct sockaddr_in dst = {
		.sin_family = AF_INET,
	};
	struct net_icmp_ping_params params = {0};
	int ret;
	int init;

	(void)net_addr_pton(AF_INET, "192.0.2.1", &dst.sin_addr);
	params.priority = NET_MAX_PRIORITIES; /* one past the last valid value */

	init = net_icmp_init_ctx(&ctx, NET_ICMPV4_ECHO_REPLY, 0,
				 neg_icmp_handler);
	if (init != 0) {
		LOG_ERR("  icmp_send_bad_priority: ctx init failed %d", init);
		return 1;
	}

	ret = net_icmp_send_echo_request(&ctx, neg_iface,
					 (struct sockaddr *)&dst,
					 &params, NULL);
	(void)net_icmp_cleanup_ctx(&ctx);

	if (ret != -EINVAL) {
		LOG_ERR("  icmp_send_bad_priority: expected -EINVAL, got %d",
			ret);
		return 1;
	}
	return 0;
}

/* ------------------------------------------------------------------ */
/* DHCPv4 option-callback negative subtests                           */
/* ------------------------------------------------------------------ */

#if defined(CONFIG_NET_DHCPV4_OPTION_CALLBACKS)

/* net_dhcpv4_add_option_callback() must reject a NULL callback. */
static int subtest_dhcp_add_opt_null(void)
{
	int ret = net_dhcpv4_add_option_callback(NULL);

	if (ret != -EINVAL) {
		LOG_ERR("  dhcp_add_opt_null: expected -EINVAL, got %d", ret);
		return 1;
	}
	return 0;
}

/* net_dhcpv4_add_option_callback() must reject a callback with no handler. */
static int subtest_dhcp_add_opt_null_handler(void)
{
	struct net_dhcpv4_option_callback cb = {0};
	int ret = net_dhcpv4_add_option_callback(&cb);

	if (ret != -EINVAL) {
		LOG_ERR("  dhcp_add_opt_null_handler: expected -EINVAL, got %d",
			ret);
		return 1;
	}
	return 0;
}

/* net_dhcpv4_remove_option_callback() must reject a NULL callback. */
static int subtest_dhcp_remove_opt_null(void)
{
	int ret = net_dhcpv4_remove_option_callback(NULL);

	if (ret != -EINVAL) {
		LOG_ERR("  dhcp_remove_opt_null: expected -EINVAL, got %d",
			ret);
		return 1;
	}
	return 0;
}

/* Removing a validly-initialised callback that was never added must fail. */
static int subtest_dhcp_remove_unregistered(void)
{
	struct net_dhcpv4_option_callback cb;
	uint8_t storage[4];
	int ret;

	net_dhcpv4_init_option_callback(&cb, neg_dhcp_option_handler,
					42 /* NTP */, storage, sizeof(storage));

	ret = net_dhcpv4_remove_option_callback(&cb);
	if (ret != -EINVAL) {
		LOG_ERR("  dhcp_remove_unregistered: expected -EINVAL, got %d",
			ret);
		return 1;
	}
	return 0;
}

#endif /* CONFIG_NET_DHCPV4_OPTION_CALLBACKS */

/* ------------------------------------------------------------------ */
/* Comprehensive negative test                                        */
/* ------------------------------------------------------------------ */

ZTEST(test_eth_dhcpv4_negative, test_comprehensive_negative)
{
	int fail = 0;
	int f;

	zassert_not_null(neg_iface,
			 "No Ethernet interface; check the DWMAC node/overlay");

	TC_PRINT("=== Ethernet DHCPv4 Negative Validation ===\n");

	f = subtest_icmp_init_null_ctx();
	fail += f;
	TC_PRINT("  icmp_init_null_ctx     : %s\n", f ? "FAIL" : "OK");

	f = subtest_icmp_init_null_handler();
	fail += f;
	TC_PRINT("  icmp_init_null_handler : %s\n", f ? "FAIL" : "OK");

	f = subtest_icmp_cleanup_null();
	fail += f;
	TC_PRINT("  icmp_cleanup_null      : %s\n", f ? "FAIL" : "OK");

	f = subtest_icmp_send_null_ctx();
	fail += f;
	TC_PRINT("  icmp_send_null_ctx     : %s\n", f ? "FAIL" : "OK");

	f = subtest_icmp_send_null_dst();
	fail += f;
	TC_PRINT("  icmp_send_null_dst     : %s\n", f ? "FAIL" : "OK");

	f = subtest_icmp_send_bad_priority();
	fail += f;
	TC_PRINT("  icmp_send_bad_priority : %s\n", f ? "FAIL" : "OK");

#if defined(CONFIG_NET_DHCPV4_OPTION_CALLBACKS)
	f = subtest_dhcp_add_opt_null();
	fail += f;
	TC_PRINT("  dhcp_add_opt_null      : %s\n", f ? "FAIL" : "OK");

	f = subtest_dhcp_add_opt_null_handler();
	fail += f;
	TC_PRINT("  dhcp_add_opt_no_handler: %s\n", f ? "FAIL" : "OK");

	f = subtest_dhcp_remove_opt_null();
	fail += f;
	TC_PRINT("  dhcp_remove_opt_null   : %s\n", f ? "FAIL" : "OK");

	f = subtest_dhcp_remove_unregistered();
	fail += f;
	TC_PRINT("  dhcp_remove_unregister : %s\n", f ? "FAIL" : "OK");
#endif /* CONFIG_NET_DHCPV4_OPTION_CALLBACKS */

	TC_PRINT("=== Result: %d failures ===\n", fail);
	zassert_equal(fail, 0, "Negative validation: %d failures", fail);
}

static void *neg_setup(void)
{
	neg_iface = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
	return NULL;
}

ZTEST_SUITE(test_eth_dhcpv4_negative, NULL, neg_setup, NULL, NULL, NULL);

#endif /* CONFIG_TEST_ETH_DHCPV4_NEGATIVE */
