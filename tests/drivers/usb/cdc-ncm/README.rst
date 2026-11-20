Alif USB CDC NCM Driver Tests
=============================

Overview
********
Ztest QA for USB CDC Network Control Model (NCM), matching
``samples/net/sockets/echo_server`` with Alif overlays:

- ``overlay-usbd_next.conf``
- ``boards/alif-cdc-ncm-common.conf``
- ``boards/alif-cdc-ncm-common.overlay``

Device IPv4 ``192.168.7.2``, host ``192.168.7.1``, TCP echo port ``4242``.
USB ``2fe3:0001``, Product **Alif CDC NCM Ethernet**.

Suites
******

**cdc_ncm_basic** (no USB host):

- ``test_ipv4_addr_parse``
- ``test_peer_ipv4_parse``
- ``test_mac_bytes``

**cdc_ncm_qa**:

================================== =================================================
Test case                          QA check
================================== =================================================
``test_ncm_eth_device_ready``      ``cdc_ncm_eth0`` bound
``test_ncm_iface_up``              ``net iface`` oper=UP
``test_ncm_mac_local``             ``02:00:00:00:00:01``
``test_ncm_mtu``                   MTU 1500
``test_ncm_ipv4_configured``       ``192.168.7.2``
``test_ncm_ipv6_configured``       ``2001:db8::1``
``test_ncm_carrier``               Host enum / carrier ON
``test_usb_speed_capability``      DWC3 FS or HS
``test_ncm_loopback_ping``         ``net ping 192.168.7.2``
``test_ncm_peer_ping``             Ping host ``192.168.7.1``
``test_ncm_tcp_bind``              TCP listen port 4242
``test_ncm_ipv4_netmask``          ``255.255.255.0``
``test_ncm_link_ethernet``         Link type Ethernet
``test_ncm_udp_bind``              UDP bind ``192.168.7.2:4243``
``test_ncm_udp_loopback``          Local UDP datagram
``test_ncm_tcp_loopback``          Local TCP echo on port 4244
``test_stress_loopback_ping``      10 ICMP echoes to self
``test_stress_udp_burst``          16 x 128-byte local UDP
``test_stress_socket_cycle``       16 TCP socket open/close
``test_stress_iface_poll``         30 polls of MAC, MTU, oper UP
``test_perf_loopback_ping``        10 pings, rate informational
``test_perf_udp_loopback``         Local UDP kbps informational
``test_usb_disable_reenable``      SKIP on DWC3+NCM (keeps host netdev)
``test_usb_disable_enable_stress`` SKIP on DWC3+NCM (keeps host netdev)
================================== =================================================

Stress and performance stay on the device IP stack. They do not call
``usbd_disable``, so the host ``cdc_ncm`` interface remains up for
``ping`` / ``nc`` after the suite. USB wire throughput is not measured
here; that needs a host iperf after the suite.

After the suite, TCP echo stays up on ``192.168.7.2:4242`` for host
``ping`` / ``nc``.

Host-side QA (after flash)
**************************

Connect the USB **device** port. Expected dmesg::

   Product: Alif CDC NCM Ethernet
   Manufacturer: Alif Semiconductor
   cdc_ncm ... CDC NCM (NO ZLP), 00:00:5e:00:53:01

::

   sudo ip link set enx00005e005301 up
   sudo ip addr add 192.168.7.1/24 dev enx00005e005301
   ping 192.168.7.2
   nc 192.168.7.2 4242

Building
********

From ``zephyr/``::

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/tests/drivers/usb/cdc-ncm \
     -- -DDTC_OVERLAY_FILE=boards/alif_cdc_ncm.overlay

Sample (non-ztest)::

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/net/sockets/echo_server \
     -- -DEXTRA_CONF_FILE="overlay-usbd_next.conf;boards/alif-cdc-ncm-common.conf" \
        -DDTC_OVERLAY_FILE=boards/alif-cdc-ncm-common.overlay

TestLink: ``testlink_cdc_ncm.xml`` (ztest) and ``testlink_cdc_ncm_sample.xml`` (sample).
