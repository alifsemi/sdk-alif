.. _snippet-ospi-psram:

OSPI PSRAM Snippet
###################

Overview
********

This snippet selects the board overlay for AP Memory APS512XXN PSRAM on E8
AppKit and E8 DevKit targets (OSPI0), or ISSI HyperRAM on B1/E7 DevKit
(OSPI0). The S80KS engineering-board configuration is selected explicitly;
it is not provided by this snippet.

The E8 AppKit overlay configures APS512XXN at 200 MHz in x16 mode with
``latency-code = <7>``. It sets ``enable-signal-delay`` on OSPI0 to apply
the configured per-signal delays on Ensemble Gen2. Tune the delays and
RAM latency when changing the bus frequency.

Building and Running
********************

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/spi_psram
   :board: alif_e8_ak/ae822fa0e5597xx0/rtss_he
   :goals: build
   :gen-args: -S ospi-psram
   :compact:

B1 DevKit HyperRAM
******************

The ``b1_dk_ospi0.overlay`` fragment enables ISSI IS66WVH HyperRAM on
OSPI0 for the B1 DevKit ``ab1c1f4m51820ph0/rtss_he`` target at 80 MHz.

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/spi_psram
   :board: alif_b1_dk/ab1c1f4m51820ph0/rtss_he
   :goals: build
   :snippets: ospi-psram
   :west-args: -p always
   :compact:

E7 DevKit HyperRAM
******************

The ``e7_dk_ospi0.overlay`` fragment enables ISSI IS66WVH HyperRAM using the
Alif HAL OSPI driver on E7 DevKit HE and HP targets. It configures OSPI0 at
100 MHz with six wait cycles and a 64 MiB XiP window at ``0xA0000000``.

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S ospi-psram ../alif/samples/drivers/spi_psram

Replace ``rtss_he`` with ``rtss_hp`` for the HP core.

Engineering Board HyperRAM
*************************

The ``e8_ek_ospi1.overlay`` fragment configures Infineon S80KS HyperRAM on
the E8 engineering board using the E8 DevKit build target. Select it with
``DTC_OVERLAY_FILE`` instead of ``-S ospi-psram``; the DevKit snippet overlay
configures a different device (APS512XXN on OSPI0). The S80KS overlay
configures OSPI1 at 100 MHz:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/spi_psram -- -DDTC_OVERLAY_FILE="$PWD/../alif/snippets/ospi-psram/e8_ek_ospi1.overlay"

Replace ``rtss_he`` with ``rtss_hp`` for the HP core. Configure the OSPI1
XIP MPU region as read/write with SRAM attributes. Run this command from the
Zephyr repository directory so the ``$PWD`` path resolves to the overlay.

Application Output
******************

Example for E8 AppKit with 64 MB APS512XXN at 200 MHz:

.. code-block:: console

   PSRAM XIP mode demo app started
   Configured OSPI bus speed: 200000000 Hz (200 MHz)
   Test address range: 0xa0000000 - 0xa3ffffff : 64 MB

   Writing data to the XIP region:
   Reading back:
   Done, total errors = 0
