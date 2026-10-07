.. _exerciser-app-sample:

Exerciser App Sample
####################

Overview
********

This sample starts one worker thread per enabled peripheral so several
Alif kit interfaces can run at the same time. Device tree decides which
workers are compiled and started. A missing node is skipped; it is not
a build error.

With ``CONFIG_SHELL=y`` (all supported boards), threads start after
the ``start_exerciser`` command.

Supported boards
================

All targets below are RTSS-HE.

* ``alif_e8_ak/ae822fa0e5597xx0/rtss_he`` — E8 AppKit
* ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he`` — B1 DevKit
* ``alif_b1_sk/ab1c1f4m51820ph0/rtss_he`` — B1 StarterKit
* ``alif_e1c_dk/ae1c1f4051920hh/rtss_he`` — E1C DevKit
* ``alif_e1c_sk/ae1c1f4051920hh/rtss_he`` — E1C StarterKit

Workers by board
================

================  =====  ======  ===========  ==  ===  ======  ======  =================  ====
Board             LED    OSPI    SPI loopback SD  USB  BMI323  WM8904  OV5675 viewfinder  ETH
================  =====  ======  ===========  ==  ===  ======  ======  =================  ====
B1 / E1C DevKit   yes    yes     yes          yes yes  yes     yes     no                 no
B1 / E1C SK       yes    no      yes          no  yes  no      no      no                 no
E8 AppKit         yes    yes     no           yes no   no      yes     yes                yes
================  =====  ======  ===========  ==  ===  ======  ======  =================  ====

E8 AppKit also needs the ``ov5675-cam`` app snippet (camera, ISP, display,
SD, codec) and the tree snippet ``alif-dhcpv4-client`` (Ethernet / DHCP),
``ospi-flash`` (OSPI flash).
Do not remap ``ns`` on E8 HE; keep the default 96 KB window.

Building
********

From the workspace root. Replace ``<app>`` with
``alif/tests/applications/exerciser_app``.

B1 DevKit::

   west build -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he <app>

B1 StarterKit::

   west build -b alif_b1_sk/ab1c1f4m51820ph0/rtss_he <app>

E1C DevKit::

   west build -b alif_e1c_dk/ae1c1f4051920hh/rtss_he <app>

E1C StarterKit::

   west build -b alif_e1c_sk/ae1c1f4051920hh/rtss_he <app>

E8 AppKit (snippets required)::

   west build -b alif_e8_ak/ae822fa0e5597xx0/rtss_he <app> \
     -S ov5675-cam -S alif-dhcpv4-client -S ospi-flash

Running
*******

Flash and open the console UART, then::

   start_exerciser

Sample output
=============

.. code-block:: console

   *** Booting Zephyr OS ***
   Exerciser App - Shell Control Mode
   Use 'start_exerciser' command to start all threads
   Shell ready. Type 'help' for available commands.
   uart:~$ start_exerciser
   Starting all exerciser threads...
   Blinky thread started
   ...
   All exerciser threads started successfully!

Which lines appear after start depends on the board table above.