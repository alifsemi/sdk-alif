.. _appnote-zas-PSRAM:

========
PSRAM
========

Introduction
=============

Currently, **AP PSRAM** support is available only on the **E8 AppKit** board (``alif_e8_ak``).

The **HexSPI0/OSPI0 SS0** instance is connected to the **APS512XXN** device and supports both **x8** and **x16** transfer modes.
To enable the **x16 transfer mode**, the ``x16-data-transfer-mode`` property must be present in the ``aps512xxn`` node.

.. note::

   DW OSPI controllers on ALIF boards support operation in either **OctalSPI** or **HexSPI** mode depending on the configuration.
   Therefore, OSPI nodes may also be referred to as **HexSPI (HSPI)** nodes.

Driver Description
-------------------

The **APS512XXN** driver is fully functional within the **Zephyr** framework.
The **spi_psram** component from the **ALIF sdk-alif** repository has been integrated with the APS512XXN MEMC driver and verified successfully.

The APS512XXN device is connected to the **OSPI0 SS0** instance of the **DW OSPI** peripheral.
It currently supports operation up to **100 MHz**. If a higher or different frequency is required, the corresponding **OSPI and RAM parameters** must be tuned accordingly to ensure stable operation.

For debugging and console output:
- **UART2** is used on the **M55 HP** core.
- **UART4** is used on the **M55 HE** core.

.. include:: prerequisites.rst

.. include:: note.rst

Building an PSRAM Application with Zephyr
===========================================

Follow these steps to build the PSRAM Application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The PSRAM feature is supported **only** on the **Alif E8 AppKit**.
   It is not applicable to other boards.

2. Build commands for applications on the M55 HE core:

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
   ../alif/samples/drivers/spi_psram/ \
   -DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/spi_psram/boards/alif_aps512xxn_psram.overlay

3. Build commands for applications on the M55 HP core:


.. code-block:: bash

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
   ../alif/samples/drivers/spi_psram/ \
   -DDTC_OVERLAY_FILE=$PWD/../alif/samples/drivers/spi_psram/boards/alif_aps512xxn_psram.overlay

Executing Binary on the AppKit
===============================

To execute binaries on the AppKit follow the command

.. code-block:: bash

   west flash

Expected Logs
===============

Below is an example of the expected console output:

.. code-block:: console

   PSRAM XIP mode demo app started
   Writing data to the XIP region:
   Reading back: Done,
   total errors = 0
