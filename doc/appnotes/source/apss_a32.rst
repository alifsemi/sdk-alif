.. _appnote-apss-a32:

===========================================
Running Zephyr on the Cortex-A32 based APSS
===========================================

Introduction
============

This application note describes how to build, flash, and boot a Zephyr application
on the Cortex-A32 based Application Processor SubSystem (APSS) of Alif Ensemble
SoC series E7 and E8 on their respective development kits. The Zephyr board targets
operate in a single-core, non-SMP configuration on one core of the dual-core
Cortex-A32 cluster present in the APSS, running in the AArch32 Normal World
after Trusted Firmware-A (TF-A) has initialized the system in the Secure World
and handed over control.

Supported Board Targets
=======================

.. list-table::
   :widths: 35 65
   :header-rows: 1
   :align: left

   * - Board Target
     - Description
   * - ``alif_e8_dk/ae822fa0e5597xx0/apss``
     - Alif Ensemble E8 DevKit — APSS Cortex-A32 cluster
   * - ``alif_e7_dk/ae722f80f55d5xx/apss``
     - Alif Ensemble E7 DevKit — APSS Cortex-A32 cluster

Hardware Features
=================

The following hardware features are supported by the board targets:

.. list-table::
   :widths: 30 70
   :header-rows: 1
   :align: left

   * - Feature
     - Details
   * - CPU
     - ARM Cortex-A32 (ARMv8-A, AArch32 execution state), single core
   * - Interrupt controller
     - ARM GICv2
   * - System timer
     - ARM architecture timer
   * - UART console
     - NS16550 — UART2, 115200 baud, 8N1
   * - Flash / code storage
     - MRAM (XIP), Zephyr image at ``0x80100000``
   * - SRAM
     - 4 MB at ``0x02000000``

Boot Overview
=============

After the Secure Enclave (SE) releases the Cortex-A32 core, Trusted
Firmware-A (TF-A) BL32 runs first. It executes in the AArch32 Secure World,
initializes the system and then hands control to Zephyr running in the AArch32
Normal World. Both the TF-A binary and the Zephyr binary reside in MRAM and
execute in place (XIP).

.. include:: prerequisites.rst

Building the Application
========================

The example below uses the ``samples/hello_world`` application.

**E8 DevKit**

.. code-block:: console

   west build -p always -b alif_e8_dk/ae822fa0e5597xx0/apss samples/hello_world

**E7 DevKit**

.. code-block:: console

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/apss samples/hello_world

The build output binary is located at ``build/zephyr/zephyr.bin``.

Flashing with the Alif Security Toolkit (SETOOLS)
==================================================

Flashing the APSS targets requires two binaries to be separately downloaded in
addition to the Zephyr application binary:

- **TF-A BL32 binary** (``bl32.bin``) — the secure world bootloader.
- **SETOOLS JSON configuration file** — describes the memory layout and boot flags
  for all images.

The M55 stub binaries (``m55_stub_hp.bin`` and ``m55_stub_he.bin``) referenced in
the JSON configuration are part of a standard SETOOLS installation and require no
separate download.

Required Downloads
------------------

Download the pre-built TF-A binary for your board and the common SETOOLS JSON
configuration from the links below.

**TF-A BL32 binary**

- E8 DevKit: `bl32_e8.bin`_
- E7 DevKit: `bl32_e7.bin`_

**SETOOLS JSON configuration** (common to both E7 and E8)

- `bl32-z.json`_

.. note::
   The JSON configuration file is identical for both boards. Use the TF-A binary that
   matches your DevKit.

Flashing Steps
--------------

The JSON configuration file programs three images into MRAM:

- TF-A BL32 at ``0x80002000`` (boot entry point)
- Zephyr application at ``0x80100000``
- M55 stub images for the Cortex-M55 cores

Follow these steps to flash the board:

1. Copy the Zephyr binary to the SE tool images directory:

   .. code-block:: console

      cp build/zephyr/zephyr.bin <SE tool folder>/build/images/

2. Copy the TF-A binary to the SE tool images directory, naming it ``bl32.bin``.
   Use the binary that matches your DevKit:

   .. code-block:: console

      cp bl32_e8.bin <SE tool folder>/build/images/bl32.bin

3. Copy the JSON configuration to the SE tool config directory:

   .. code-block:: console

      cp bl32-z.json <SE tool folder>/build/config/

4. Generate the Application Table of Contents (ATOC) and write to MRAM:

   .. code-block:: console

      cd <SE tool folder>
      ./app-gen-toc --filename build/config/bl32-z.json
      ./app-write-mram -p

For the E7 DevKit, substitute ``bl32_e7.bin`` in step 2; the JSON file and all
other steps are identical.

.. note::
   Ensure your user has sufficient permissions to access the SE-UART device
   (``dialout`` group). See the `ZAS User Guide`_ for SE tool setup details.

JSON Configuration Reference
-----------------------------

The following shows the structure of the SE tool JSON configuration used for the
APSS targets. This is provided for reference; use the pre-built JSON file
downloaded above.

.. code-block:: json

   {
       "DEVICE": {
           "disabled": false,
           "binary": "app-device-config.json",
           "version": "0.5.00",
           "signed": true
       },
       "BOOTLOAD": {
           "binary": "bl32.bin",
           "version": "0.4.3",
           "mramAddress": "0x80002000",
           "signed": true,
           "cpu_id": "A32_0",
           "flags": ["boot"]
       },
       "A32_APP": {
           "binary": "zephyr.bin",
           "version": "1.0.0",
           "mramAddress": "0x80100000",
           "signed": true,
           "cpu_id": "A32_0"
       },
       "HP_APP": {
           "disabled": false,
           "binary": "m55_stub_hp.bin",
           "version": "1.0.0",
           "signed": true,
           "loadAddress": "0x50000000",
           "cpu_id": "M55_HP",
           "flags": ["load", "boot"]
       },
       "HE_APP": {
           "disabled": false,
           "binary": "m55_stub_he.bin",
           "version": "1.0.0",
           "signed": true,
           "loadAddress": "0x58000000",
           "cpu_id": "M55_HE",
           "flags": ["load", "boot"]
       }
   }

Key fields:

- ``BOOTLOAD``: TF-A BL32 binary, flashed to MRAM at ``0x80002000``. The
  ``"boot"`` flag instructs the SE to start execution here after reset.
- ``A32_APP``: Zephyr binary, placed at ``0x80100000`` in MRAM. TF-A transfers
  control to this address when exiting the secure world.
- ``HP_APP`` / ``HE_APP``: M55 stub images for the Cortex-M55 cores.
  These stubs are part of the standard SETOOLS installation.

Console Output
==============

Connect a serial terminal to UART2 on the DevKit at **115200 baud, 8N1** to
observe boot messages.

After reset, TF-A initializes the system and then Zephyr starts:

.. code-block:: text

   NOTICE:  SP_MIN: v2.10.8(debug):APSS-v2.2.0
   NOTICE:  SP_MIN: Built : 06:40:38, May 19 2026
   INFO:    ARM GICv2 driver initialized
   INFO:    SP_MIN: Initializing runtime services
   INFO:    SP_MIN: Preparing exit to normal world
   INFO:    Entry point address = 0x80100000
   INFO:    SPSR = 0x1d3
   *** Booting Zephyr OS build v4.1.0-590-g20ed52279fad ***
   Hello World! alif_e8_dk
