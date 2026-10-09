.. _appnote-zas-PSRAM:

========
PSRAM
========

Introduction
=============

The ``spi_psram`` sample tests and verifies PSRAM and HyperRAM devices over
the OSPI/HexSPI interface. The application builds only when the selected
target has a devicetree node with one of these compatibles:

- ``alif,apmemory-aps512xxn`` (AP Memory APS512XXN PSRAM)
- ``alif,infineon-s80ks2564`` (Infineon S80KS HyperRAM)
- ``issi,is66wvhxx`` (ISSI IS66WVH HyperRAM)

Use the matching overlay in ``snippets/ospi-psram`` for the device under test.

The HexSPI0/OSPI0 SS0 instance is connected to the APS512XXN device. It
operates in both x8 and x16 transfer mode. The ``x16-data-transfer-mode``
property must be present in the ``aps512xxn`` node to use x16 transfer mode.
AP Memory APS512XXN PSRAM is supported on the Alif E8 AppKit
(``alif_e8_ak``).

The Infineon S80KS HyperRAM is connected over OSPI1.
Build this configuration with the Alif E8 DevKit
(``alif_e8_dk``) target.

The ISSI IS66WVH HyperRAM is connected to OSPI0 on the Alif B1 DevKit
(``alif_b1_dk``). The compatible is ``issi,is66wvhxx``, and the XIP
window starts at ``0xA0000000``. This configuration is supported on the
M55 HE core only. Select ``b1_dk_ospi0.overlay`` using the
``ospi-psram`` snippet.

ISSI IS66WVH HyperRAM is also supported on OSPI0 on the Alif E7 DevKit
(``alif_e7_dk``), on both M55 HE and HP cores, using ``e7_dk_ospi0.overlay``.

.. note::

   DW OSPI controllers on Alif boards support operation in either OctalSPI or HexSPI mode depending on the configuration. Hence, OSPI nodes can also be referred to as HexSPI (HSPI) nodes.

Driver Description
==================

The APS512XXN driver is fully functional within the Zephyr framework. The ``spi_psram`` component from the ALIF sdk-alif repository has been integrated with the APS512XXN MEMC driver and verified successfully.

The E8 AppKit APS512XXN overlay configures OSPI0 at 200 MHz in x16 mode
with ``latency-code = <7>``. It enables ``enable-signal-delay`` on the OSPI0
controller and supplies per-signal delay values. The driver validates these
values and preserves the configured RXDS delays during XIP setup.
Per-signal delays require Ensemble Gen2; without the enable property, the
driver uses the legacy RXDS setting. Tune the controller delays and RAM
latency when changing the bus frequency.

The Infineon S80KS HyperRAM driver is integrated through the same
``spi_psram`` sample and the MEMC OSPI path. S80KS is connected to
OSPI1 SS0, with an XIP window at
``0xC0000000``. The overlay sets the OSPI1 bus speed to 200 MHz. If a
different frequency is required, tune the corresponding OSPI and RAM
parameters for stable operation.

For debugging and console output:

- UART4 is used on the M55 HP core.
- UART2 is used on the M55 HE core.

Supported Devices and Overlays
==============================

The sample selects the RAM device from the ``spi-psram`` alias. Board
overlays under ``snippets/ospi-psram`` provide that alias.

.. list-table::
   :header-rows: 1
   :widths: 22 28 18 32

   * - Device
     - Compatible
     - Interface
     - Overlay / board
   * - APS512XXN PSRAM
     - ``alif,apmemory-aps512xxn``
     - HexSPI0/OSPI0 SS0
     - ``snippets/ospi-psram/e8_ak_ospi0.overlay``
       (selected with ``-S ospi-psram`` for E8 AppKit)
   * - S80KS HyperRAM
     - ``alif,infineon-s80ks2564``
     - OSPI1 SS0
     - ``snippets/ospi-psram/e8_ek_ospi1.overlay``
       (selected with ``-S ospi-psram`` for E8 DevKit)
   * - IS66WVH HyperRAM
     - ``issi,is66wvhxx``
     - OSPI0
     - ``snippets/ospi-psram/e7_dk_ospi0.overlay``
       (selected with ``-S ospi-psram`` for E7 DevKit, M55 HE and HP)
   * - IS66WVH HyperRAM
     - ``issi,is66wvhxx``
     - OSPI0
     - ``snippets/ospi-psram/b1_dk_ospi0.overlay``
       (selected with ``-S ospi-psram`` for B1 DevKit, M55 HE only)

.. note::

   For S80KS HyperRAM, make sure the MPU entry for the OSPI1 XIP region is
   configured as read/write with SRAM attributes. On the S80KS overlay this
   region starts at ``0xC0000000``.

.. include:: prerequisites.rst

.. include:: note.rst

Build a PSRAM Application with Zephyr
=====================================

Follow these steps to build the PSRAM application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the
   Zephyr repository, refer to the `ZAS User Guide`_, under the section
   ``Setting Up and Building Zephyr Applications``.

.. note::
   AP Memory APS512XXN PSRAM is supported on the **Alif E8 AppKit**.
   Infineon S80KS HyperRAM is built with the **Alif E8 DevKit** target.
   ISSI IS66WVH HyperRAM is supported on the **Alif E7 DevKit** (M55 HE
   and HP) and **Alif B1 DevKit** (M55 HE only).

APS512XXN PSRAM (E8 AppKit)
---------------------------

1. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
   -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
   -S ospi-psram \
   ../alif/samples/drivers/spi_psram/

2. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
   -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
   -S ospi-psram \
   ../alif/samples/drivers/spi_psram/

S80KS HyperRAM (E8 DevKit)
--------------------------

The ``snippets/ospi-psram/e8_ek_ospi1.overlay`` file provides the devicetree
configuration for the Infineon S80KS HyperRAM on the engineering board.

S80KS is connected over OSPI1. Build the sample with the Alif E8 DevKit
board target.

Select the overlay with ``-S ospi-psram``:

1. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
   -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
   -S ospi-psram \
   ../alif/samples/drivers/spi_psram/

2. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
   -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
   -S ospi-psram \
   ../alif/samples/drivers/spi_psram/

IS66WVH HyperRAM (E7 DevKit)
----------------------------

The ``ospi-psram`` snippet selects ``e7_dk_ospi0.overlay`` for E7 DevKit
HE and HP targets. The HAL OSPI driver uses OSPI0 at 100 MHz with six
wait cycles and the XiP region starting at ``0xA0000000``.

.. code-block:: console

   west build -p always \
   -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
   -S ospi-psram \
   ../alif/samples/drivers/spi_psram

Replace ``rtss_he`` with ``rtss_hp`` for the HP core.

IS66WVH HyperRAM (B1 DevKit)
----------------------------

The ISSI IS66WVH HyperRAM is connected to OSPI0 on the Alif B1 DevKit.
The XIP window starts at ``0xA0000000``. This sample is supported on the
M55 HE core only.

The ``ospi-psram`` snippet selects
``snippets/ospi-psram/b1_dk_ospi0.overlay`` for this board target.

1. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
   -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
   -S ospi-psram \
   ../alif/samples/drivers/spi_psram

Executing Binary on the AppKit or DevKit
========================================

To execute binaries on the AppKit or DevKit, follow the command:

.. code-block:: console

   west flash

Expected Logs
===============

Below is an example for E8 AppKit with 64 MB APS512XXN at 200 MHz:

.. code-block:: text

   PSRAM XIP mode demo app started
   Configured OSPI bus speed: 200000000 Hz (200 MHz)
   Test address range: 0xa0000000 - 0xa3ffffff : 64 MB

   Writing data to the XIP region:
   Reading back:
   Done, total errors = 0
