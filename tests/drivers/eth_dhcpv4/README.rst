.. _net_alif_ethernet_dhcpv4:

Alif Ethernet DHCPv4 (real hardware) test
##########################################

Overview
********

This ztest suite validates the **real** Alif Ethernet path end to end:

* Synopsys DesignWare MAC (DWMAC) driver  -- ``drivers/ethernet/eth_dwmac.c``
* Alif platform glue                      -- ``drivers/ethernet/eth_dwmac_alif_ensemble.c``
* DesignWare MDIO bus                      -- ``drivers/mdio/mdio_dwmac.c``
* Realtek RTL8201FR PHY                    -- ``drivers/ethernet/phy_realtek_rtl8201fr.c``

Working
=======

This suite enables the real driver stack
(``CONFIG_ETH_DWMAC=y`` + PHY + MDIO), brings the link up and obtains a real
lease from a real DHCP server.

Tests included
**************

Fully automated (need cable + DHCP server):

* ``test_eth_dhcpv4_client_build_alif_board``
* ``test_eth_dhcpv4_ethernet_interface_detection``
* ``test_eth_dhcpv4_dora_flow``
* ``test_eth_dhcpv4_ipv4_address_assignment_event``
* ``test_eth_dhcpv4_subnet_mask_reception``
* ``test_eth_dhcpv4_router_gateway_reception``
* ``test_eth_dhcpv4_lease_time_reception``
* ``test_eth_dhcpv4_ntp_option_callback``
* ``test_eth_dhcpv4_dns_option_validation``
* ``test_eth_dhcpv4_invalid_ntp_option_length_negative``
* ``test_eth_dhcpv4_ping_reachability_after_bind``
* ``test_eth_dhcpv4_background_ping_stability``
* ``test_eth_dhcpv4_packet_capture_mac_validation``
* ``test_eth_dhcpv4_lease_expiry_reacquire``
* ``test_eth_dhcpv4_stop_restart_stability``
* ``test_eth_dhcpv4_multiple_offered_addresses``

Environment/operator-gated (auto-validate when the condition is present,
otherwise ``ztest_test_skip()`` with on-console instructions):

* ``test_eth_dhcpv4_10mbps_link``
* ``test_eth_dhcpv4_100mbps_link``
* ``test_eth_dhcpv4_full_duplex_link``
* ``test_eth_dhcpv4_half_duplex_link``
* ``test_eth_dhcpv4_pool_exhaustion_negative``
* ``test_eth_dhcpv4_server_unavailable_negative``

Manual cable tests (gated by ``CONFIG_ETH_DHCPV4_MANUAL_CABLE_TESTS=y``):

* ``test_eth_dhcpv4_cable_disconnect_before_dhcp``
* ``test_eth_dhcpv4_cable_disconnect_after_bind``

Negative test suite (``test_eth_dhcpv4_negative``):

* ``test_comprehensive_negative`` — Validates ICMP NULL-context, NULL-destination,
  and DHCP option callback error paths.

.. note::

   The DHCP-dependent tests call an idempotent ``ensure_dhcp_bound()`` helper,
   so they pass regardless of the order ztest runs them in.

   Link speed/duplex tests read the negotiated PHY state via
   ``net_eth_get_phy()`` + ``phy_get_link_state()`` and assert only when the
   link actually negotiated that speed/duplex (force it on the switch port to
   exercise a specific case).

   ``lease_expiry_reacquire`` uses ``net_dhcpv4_restart()`` as a deterministic
   re-acquire trigger; true time-based expiry depends on the lease duration.

   The DHCP-dependent tests include a 500 ms settle delay after
   ``net_dhcpv4_stop()`` to allow the DWMAC RX path to re-arm before the
   next ``net_dhcpv4_start()``. This mitigates a race where broadcast OFFERs
   may be dropped.

Hardware prerequisites
**********************

* Alif DevKit (E7/E8) cabled to a LAN switch.
* An active DHCP server reachable on that LAN.
* Link LED on the RJ45 should be lit before/while running.

Build and run
*************

.. code-block:: console

   west build -p always \
     -b <board> \
     tests/drivers/eth_dhcpv4  \
     -S alif-dhcpv4-client

   west flash

where ``<board>`` is e.g. ``alif_e8_dk/ae822fa0e5597xx0/rtss_he`` or
``alif_e7_dk/ae722f80f55d5xx/rtss_hp``.

Watch the UART console for the ztest ``PASS``/``FAIL`` summary.

.. note::

   This test reuses the same ``-S alif-dhcpv4-client`` snippet as the
   ``samples/net/dhcpv4_client`` sample app. The snippet enables the Ethernet,
   MDIO and PHY devicetree nodes (disabled by default in the SoC devicetree)
   and selects the Alif DWMAC + RTL8201FR PHY driver stack. The test's own
   ``prj.conf`` only adds the ztest/DHCP/ICMP/statistics options on top.
