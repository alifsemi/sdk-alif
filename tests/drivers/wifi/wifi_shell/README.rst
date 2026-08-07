Alif SDIO Wi-Fi Shell Tests
============================

Overview
********

This test suite validates the Alif SDIO Wi-Fi driver (Infineon AIROC/WHD)
using the Zephyr Wi-Fi management API and Ztest framework.

The tests exercise scan, connect, disconnect, status query, ICMP ping,
and stress/negative test paths. All state is managed through a shared
``wifi_test_ctx`` structure and synchronized with a semaphore.

Test Suites
***********

**wifi_shell suite (13 tests):**

- Scan for APs and verify at least one is found
- Connect to a configured SSID/PSK with retry logic
- Query interface status (SSID, band, channel, security, RSSI)
- Ping the network gateway via ICMP echo request
- Disconnect from the AP
- Reconnect after explicit disconnect
- Stress: repeated scan iterations
- Stress: connect/disconnect cycles
- Performance: scan timing benchmark
- Performance: connect timing benchmark
- Performance: ping latency (min/avg/max)
- Negative: connect with non-existent SSID (should fail)
- Negative: disconnect when not connected (should not crash)

**wifi_shell_destructive suite (1 test):**

- Negative: connect with wrong PSK on valid SSID. On some firmware
  (CYW43439) this may trigger a firmware trap, so this suite is
  isolated and should be run last.

Supported Boards
****************

- ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
- ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``

Board-specific overlays and config fragments are located under
``boards/``.

Prerequisites
*************

- Alif DevKit with an Infineon AIROC Wi-Fi module (Murata 1YN or 2FY)
- Wi-Fi access point with the SSID/PSK configured in ``Kconfig``
  (default: ``Alif-Wireless`` / ``Alif$2o24$``)

Configuration
*************

Key Kconfig options in ``prj.conf``:

- ``CONFIG_WIFI=y`` — enable Wi-Fi subsystem
- ``CONFIG_WIFI_AIROC=y`` — enable AIROC Wi-Fi driver
- ``CONFIG_SDIO_STACK=y`` — enable SDIO stack for SDHC communication
- ``CONFIG_SDHC_DWC=y`` — enable DesignWare SDHC controller
- ``CONFIG_NET_DHCPV4=y`` — enable DHCPv4 client
- ``CONFIG_ZTEST=y`` — enable Zephyr test framework

Kconfig tunables (see ``Kconfig``):

- ``WIFI_SHELL_TEST_SSID`` — target network SSID
- ``WIFI_SHELL_TEST_PSK`` — target network PSK
- ``WIFI_SHELL_CONNECT_ATTEMPTS`` — connect retry count
- ``WIFI_SHELL_SCAN_TIMEOUT_SEC`` — scan timeout
- ``WIFI_SHELL_CONNECT_TIMEOUT_SEC`` — connect timeout
- ``WIFI_SHELL_PING_ATTEMPTS`` — ICMP ping retry count

Module Configurations
*********************

Two Wi-Fi module variants are supported via overlay config files:

**1YN (CYW43439 — Murata 1YN):**

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       tests/drivers/wifi/wifi_shell/ \
       -DOVERLAY_CONFIG=1yn.conf

**2FY (CYW55500 — Murata 2FY):**

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       tests/drivers/wifi/wifi_shell/ \
       -DOVERLAY_CONFIG=2fy.conf

Building and Running
********************

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       tests/drivers/wifi/wifi_shell/

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       tests/drivers/wifi/wifi_shell/
