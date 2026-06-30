Alif SDIO Wi-Fi zperf Tests
===========================

Overview
********

This test suite validates network throughput over the Alif SDIO Wi-Fi
connection using the Zephyr ``zperf`` utility. It measures TCP and UDP
upload performance against an external iperf server.

The suite connects to a configured access point during setup, acquires
an IP address via DHCP, and then runs a series of throughput and
stability tests.

Test Suites
***********

**wifi_zperf suite (10 tests):**

- Verify Wi-Fi is connected and DHCP lease is active
- TCP upload with default packet size
- TCP upload with large packets (1024 bytes)
- UDP upload with configured packet size and rate
- UDP upload with large packets (1024 bytes)
- TCP throughput stability across multiple runs
- UDP packet loss measurement
- TCP upload to unreachable server (negative test)
- UDP upload with zero duration (edge case)
- Disconnect and clean up

Supported Boards
****************

- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
- ``alif_e8_ak/ae822fa0e5597xx0/rtss_hp``

Board-specific overlays and config fragments are located under
``boards/``.

Prerequisites
*************

- Alif DevKit with an Infineon AIROC Wi-Fi module (Murata 2FY)
- Wi-Fi access point with the SSID/PSK configured in ``Kconfig``
  (default: ``Alif-Wireless`` / ``Alif$2o24$``)
- An iperf server reachable from the Wi-Fi network at the IP and port
  configured in ``Kconfig`` (default: ``10.0.0.76:5001``)

Configuration
*************

Key Kconfig options in ``prj.conf``:

- ``CONFIG_WIFI=y`` — enable Wi-Fi subsystem
- ``CONFIG_WIFI_AIROC=y`` — enable AIROC Wi-Fi driver
- ``CONFIG_CYW55500=y`` — select CYW55500 chipset
- ``CONFIG_SDIO_STACK=y`` — enable SDIO stack
- ``CONFIG_SDHC_DWC=y`` — enable DesignWare SDHC controller
- ``CONFIG_NET_ZPERF=y`` — enable zperf throughput utility
- ``CONFIG_NET_SOCKETS=y`` — enable socket API (required by zperf)
- ``CONFIG_NET_DHCPV4=y`` — enable DHCPv4 client
- ``CONFIG_ZTEST=y`` — enable Zephyr test framework

Kconfig tunables (see ``Kconfig``):

- ``WIFI_ZPERF_TEST_SSID`` — target network SSID
- ``WIFI_ZPERF_TEST_PSK`` — target network PSK
- ``WIFI_ZPERF_SERVER_IP`` — iperf server IPv4 address
- ``WIFI_ZPERF_SERVER_PORT`` — iperf server port
- ``WIFI_ZPERF_TEST_DURATION_SEC`` — upload duration per test
- ``WIFI_ZPERF_TCP_PACKET_SIZE`` — TCP upload packet size
- ``WIFI_ZPERF_UDP_PACKET_SIZE`` — UDP upload packet size
- ``WIFI_ZPERF_UDP_RATE_KBPS`` — UDP target rate
- ``WIFI_ZPERF_MIN_TCP_THROUGHPUT_KBPS`` — minimum passing TCP throughput
- ``WIFI_ZPERF_MIN_UDP_THROUGHPUT_KBPS`` — minimum passing UDP throughput

Building and Running
********************

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       tests/drivers/wifi/wifi_zperf/

.. code-block:: console

   west build -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
       tests/drivers/wifi/wifi_zperf/
