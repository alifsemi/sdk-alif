Alif Wi-Fi QA (single tree)
===========================

One directory: ``alif/tests/drivers/wifi``.
Compile **one suite at a time**. Do not mix SoftAP, P2P, STA, and zperf
in the same image.

App path
********

::

   ../alif/tests/drivers/wifi

Kconfig (no overlay .conf)
**************************

Run from ``~/zas-v2.4.0-rc3/zephyr``. ``prj.conf`` defaults to Murata 1YN.
Kconfig ``select`` pulls in DHCP server, stats, P2P, zperf, and mbedTLS.

STA functional (scan / connect / ping). Same image on E8 or E7 HP
(AIROC). B1 has no Wi-Fi module in this suite::

   west build -p always -d build-wifi-shell \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DCONFIG_WIFI_TEST_SHELL=y

   west build -p always -d build-wifi-shell \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DCONFIG_WIFI_TEST_SHELL=y

   west flash -d build-wifi-shell

STA zperf (TCP/UDP vs host iperf)::

   west build -p always -d build-wifi-zperf \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DCONFIG_WIFI_TEST_ZPERF=y

   west flash -d build-wifi-zperf

SoftAP::

   west build -p always -d build-wifi-ap \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DCONFIG_WIFI_TEST_AP=y

   west flash -d build-wifi-ap

P2P GO::

   west build -p always -d build-wifi-p2p \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DCONFIG_WIFI_TEST_P2P=y

   west flash -d build-wifi-p2p

2FY instead of 1YN (add ``-DCONFIG_CYW55500=y`` to any suite)::

   west build -p always -d build-wifi-shell \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DCONFIG_WIFI_TEST_SHELL=y -DCONFIG_CYW55500=y

   west build -p always -d build-wifi-zperf \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DCONFIG_WIFI_TEST_ZPERF=y -DCONFIG_CYW55500=y

Interactive (example SoftAP dir)::

   west build -d build-wifi-ap -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi
   west build -d build-wifi-ap -t menuconfig

Then **Alif Wi-Fi QA** → **Wi-Fi QA suite to compile**. Save, rebuild.

Optional overlays
*****************

==================== ==============================================
Overlay              Suite
==================== ==============================================
``overlay-shell.conf`` STA scan / connect / ping / status / stats
``overlay-ap.conf``    SoftAP ``Alif-AP`` ``192.168.4.1``
``overlay-p2p.conf``   Wi-Fi Direct GO ``DIRECT-Al`` ``192.168.50.1``
``overlay-zperf.conf`` STA TCP/UDP zperf vs host iperf
``1yn.conf``           Murata 1YN (CYW43439)
``2fy.conf``           Murata 2FY (CYW55500)
==================== ==============================================

Same images using overlays (1YN)::

   west build -p always -d build-wifi-shell \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DOVERLAY_CONFIG="overlay-shell.conf;1yn.conf"

   west build -p always -d build-wifi-zperf \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DOVERLAY_CONFIG="overlay-zperf.conf;1yn.conf"

   west build -p always -d build-wifi-ap \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DOVERLAY_CONFIG="overlay-ap.conf;1yn.conf"

   west build -p always -d build-wifi-p2p \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/wifi \
     -- -DOVERLAY_CONFIG="overlay-p2p.conf;1yn.conf"

Flash the matching build dir::

   west flash -d build-wifi-shell
   west flash -d build-wifi-zperf
   west flash -d build-wifi-ap
   west flash -d build-wifi-p2p

Sources
*******

- ``src/test_wifi_shell.c``
- ``src/test_wifi_ap.c``
- ``src/test_wifi_p2p.c``
- ``src/test_wifi_zperf.c``

CMake compiles **only** the suite you selected (Kconfig or overlay).

TestLink
********

``testlink/`` — XML + SPECS with numbered steps:

- SoftAP: ``TESTLINK_SPECS_ap.rst`` / ``testlink_wifi_ap.xml``
- P2P: ``TESTLINK_SPECS_p2p.rst`` / ``testlink_wifi_p2p.xml``
- STA shell: ``TESTLINK_SPECS_shell.rst`` / ``testlink_wifi_shell.xml``
- zperf: ``TESTLINK_SPECS_zperf.rst`` / ``testlink_wifi_zperf.xml``

Sample (manual ``uart:~$``) XML: ``testlink_wifi_*_sample.xml``.

AIROC features covered (1YN / 2FY)
**********************************

The Infineon AIROC driver implements scan, connect, disconnect, iface
status, SoftAP enable/disable, Wi-Fi stats, and P2P GO. QA covers those
paths. Sample-only shell commands that have no driver op are not treated
as automated PASS.

STA (``overlay-shell.conf``)
  scan (active + passive), connect, DHCP/gateway/netmask, ICMP ping,
  MAC (OTP), MTU, BSSID, RSSI, band/channel, security, beacon/DTIM,
  phy rate, ``twt_capable=false``, second connect ``-EALREADY``,
  scan while associated, SoftAP-while-STA ``-EBUSY``, stats,
  Ethernet MAC-filter capability, short PSK reject, PS/TWT/version
  ``-ENOTSUP``.
  Stress: repeated scan, connect/disconnect, scan-while-connected,
  status poll, ping burst.
  Perf: scan/connect/disconnect/DHCP timing, ping latency.

SoftAP (``overlay-ap.conf``)
  enable, IPv4/DHCP, MAC/MTU, SSID, mode AP, BSSID, security PSK,
  beacon/DTIM, stats, second enable, STA-while-AP ``-EBUSY``,
  ``ap_sta_disconnect`` ``-ENOTSUP``, leftover client DHCP/ping (SKIP
  without a phone), status-poll stress.

P2P GO (``overlay-p2p.conf``)
  ``group add`` 2437, GO IPv4/DHCP, MAC/MTU/SSID, link params, BSSID,
  stats, find/stop, second group add, connect/invite/list ``-ENOTSUP``,
  leftover client (SKIP without a phone), status-poll stress.

zperf (``overlay-zperf.conf``)
  TCP/UDP upload (256 and 1024), TCP/UDP stability, UDP loss,
  TCP after reconnect, TCP download listen (SKIP without host).
  Host iperf 2 server: ``iperf -s -p 5001`` (upload).
  Host download: ``iperf -c <board_ip> -p 5002 -t 10``.

Not implemented on these modules (assert ``-ENOTSUP`` or skip)
  TWT setup, STA power save, roam / 11k / 11v / 11r, RTS, P2P client /
  invite / persistent groups, ``ap_sta_disconnect``, configurable MAC
  (OTP only). 1YN is 2.4 GHz only; 2FY may associate on 5 GHz.

Notes
*****

- CMD8 SDIO errors then WHD firmware are expected.
- STA shell AP (E8): phone hotspot ``mahesh's S23`` / ``Aadhya13$`` (2.4 GHz).
  Office ``Alif-Wireless`` associates but often drops ICMP to ``10.0.0.1``.
- zperf uses office ``Alif-Wireless`` / ``Alif$2o24$`` so the board and the
  laptop stay on ``10.0.0.0/24``. Host: ``iperf.exe -s -p 5001`` (iperf 2)
  on the laptop (now ``10.0.0.79``). Start the server **before** reset.
- SoftAP leftover: join ``Alif-AP`` / ``12345678``, ping ``192.168.4.10``.
- P2P leftover: Wi-Fi Direct ``mahesh's S23`` / SSID ``DIRECT-Al`` / PSK ``Aadhya13$``, ping ``192.168.50.10``.
- Reboot before switching SoftAP ↔ P2P ↔ STA on the same board.
