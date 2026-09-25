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

Use the matching overlay in ``samples/drivers/spi_psram/boards`` for the
device under test.

The HexSPI0/OSPI0 SS0 instance is connected to the APS512XXN device. It
operates in both x8 and x16 transfer mode. The ``x16-data-transfer-mode``
property must be present in the ``aps512xxn`` node to use x16 transfer mode.
AP PSRAM support is available only on the Alif E8 AppKit (``alif_e8_ak``).

The Infineon S80KS HyperRAM is connected over OSPI1. Build this
configuration with the Alif E8 DevKit (``alif_e8_dk``) target.

.. note::

   DW OSPI controllers on Alif boards support operation in either OctalSPI or HexSPI mode depending on the configuration. Hence, OSPI nodes can also be referred to as HexSPI (HSPI) nodes.

Driver Description
==================

The APS512XXN driver is fully functional within the Zephyr framework. The ``spi_psram`` component from the ALIF sdk-alif repository has been integrated with the APS512XXN MEMC driver and verified successfully.

The APS512XXN device is connected to the HexSPI0/OSPI0 SS0 instance of the DW OSPI (HexSPI) peripheral. It currently supports operation up to 100 MHz. If a different frequency is required, the corresponding HexSPI/OSPI and RAM parameters must be tuned accordingly to ensure stable operation.

The Infineon S80KS HyperRAM driver is integrated through the same
``spi_psram`` sample and the MEMC OSPI path. S80KS is connected to
OSPI1 SS0, with an XIP window at
``0xC0000000``. The overlay sets the OSPI1 bus speed to 40 MHz. If a
different frequency is required, tune the corresponding OSPI and RAM
parameters for stable operation.

For debugging and console output:

- UART4 is used on the M55 HP core.
- UART2 is used on the M55 HE core.

Supported Devices and Overlays
==============================

The sample selects the RAM device from the ``spi-psram`` alias. Board
overlays under ``samples/drivers/spi_psram/boards`` provide that alias.

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
     - ``alif_e8_ak_ae822fa0e5597xx0_rtss_he.overlay`` /
       ``alif_e8_ak_ae822fa0e5597xx0_rtss_hp.overlay``
       (picked automatically for E8 AppKit)
   * - S80KS HyperRAM
     - ``alif,infineon-s80ks2564``
     - OSPI1 SS0
     - ``alif_hex_s80ks.overlay`` (build with the E8 DevKit target)

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
   AP Memory APS512XXN PSRAM is supported **only** on the **Alif E8 AppKit**.
   Infineon S80KS HyperRAM is built with the **Alif E8 DevKit** target.

APS512XXN PSRAM (E8 AppKit)
---------------------------

1. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
   -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
   ../alif/samples/drivers/spi_psram/

2. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
   -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
   ../alif/samples/drivers/spi_psram/

S80KS HyperRAM (E8 DevKit)
--------------------------

The ``alif_hex_s80ks.overlay`` file provides the devicetree configuration
for the Infineon S80KS HyperRAM.

S80KS is connected over OSPI1. Build the sample with the Alif E8 DevKit
board target.

Pass the overlay on the build command:

1. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
   -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
   ../alif/samples/drivers/spi_psram/ \
   -- -DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay

2. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
   -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
   ../alif/samples/drivers/spi_psram/ \
   -- -DDTC_OVERLAY_FILE=boards/alif_hex_s80ks.overlay

Alternatively, rename ``alif_hex_s80ks.overlay`` to the overlay name
expected by the selected DevKit target so Zephyr picks it up automatically.
For the ``e8_ae822`` HE build, that name is
``alif_e8_dk_ae822fa0e5597xx0_rtss_he.overlay``.

Executing Binary on the AppKit or DevKit
========================================

To execute binaries on the AppKit or DevKit, follow the command:

.. code-block:: console

   west flash

Expected Logs
===============

Below is an example of the expected console output:

.. code-block:: text

   PSRAM XIP mode demo app started
   Writing data to the XIP region:
   Reading back:
   Done, total errors = 0
