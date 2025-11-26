.. _appnote-zas-power-management:

================
Power Management
================

Introduction
============

Power management in the Alif SoC is highly intricate and can be classified into two categories: Individual CPU states and SoC device states (Global Power States). The SoC device states make use of aiPM (Autonomous Intelligent Power Management), which can be tuned to provide fine granularity and achieve the required objectives. In this document, we will discuss individual CPU states and provide guidance on invoking aiPM services to transition into global device states.

.. note::
   Please refer to the aiPM Service API document to learn more about controlling the global clocks and power domains in the SoC.


Prerequisites
===============

Hardware Requirements
---------------------

- Alif Devkit
- USB cable (x1)
- FTDI USB (x1)

Software Requirements
---------------------
- **Alif SDK**: Clone from `https://github.com/alifsemi/sdk-alif.git <https://github.com/alifsemi/sdk-alif.git>`_
- **West Tool**: For building Zephyr applications (refer to the `ZAS User Guide`_)
- **Arm GCC Compiler**: For compiling the application (part of the Zephyr SDK)
- **SE Tools**: For loading binaries (refer to the `ZAS User Guide`_)

.. note::
   Please ensure that the debugger is not connected while running this application. If the debugger is connected, it will prevent the core from entering the OFF state.

Setup
======

The Power Management demo application shipped with ZAS uses RTSS_HE to enter the local OFF state and then subsequently enter the STOP mode of the SoC. The RTC is used as the wakeup source for the HE Subsystem. The demo application can be modified to use other wakeup sources such as LPGPIO/LPTIMER for the HP/HE domain. The logs are pushed through the console UART.


.. figure:: _static/power_management_setup_diagram.png
   :alt: Power Management Setup Diagram
   :align: center

   Example Power Management Setup


ZAS Power Management Application
==================================

This sample illustrates the following power management states:

* **PM_STATE_RUNTIME_IDLE**: Light sleep state with quick wakeup.
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention (HE core only).
    * Substate 0 (STANDBY): Medium power savings.
    * Substate 1 (STOP): Higher power savings.
* **PM_STATE_SOFT_OFF**: Deepest sleep, no retention, full system reset on wakeup.

The behaviour differs between HE and HP cores:

HE Core
=======

* When booting from TCM (VTOR = 0x0, with retention support):
    * Demonstrates ``RUNTIME_IDLE``, ``S2RAM STANDBY``, ``S2RAM STOP``
    * Skips ``SOFT_OFF`` (uses retention instead)
    * Resumes execution after each state.
* When booting from MRAM (VTOR >= 0x80000000) (CONFIG_PM_S2RAM=n):
    * Demonstrates ``RUNTIME_IDLE``, ``SOFT_OFF``
    * System resets and restarts from ``main()`` after ``SOFT_OFF``.

HP Core (no retention support)
==============================

.. note::
   The PM-demo app supports HP only from MRAM and not from TCM.

* Only ``SOFT_OFF`` is available (no S2RAM support)
* Demonstrates ``RUNTIME_IDLE``, then ``SOFT_OFF``
* System resets and restarts from ``main()`` after ``SOFT_OFF``.

How to Use the Application
==========================

This sample application can be used for basic power measurement and demonstrates how to power off a subsystem in the RTSS cores of the Alif Ensemble.

.. note::
   If using a USB hub to connect the UART, it is advised to set the BOOT_DELAY to 5 seconds to ensure UART logs are not missed on the PC after reset. This sample is specific to a single subsystem. For the complete SoC to transition to global states (IDLE/STANDBY/STOP), it requires voting from all the remaining subsystems in the SoC.

.. include:: note.rst

Building an PM Application with Zephyr
========================================

Follow these steps to build the Power Management Application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.


2. Build commands for HE application from TCM:

.. code-block:: bash

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
    ../alif/samples/drivers/pm/system_off \
    -S pm-system-off-he \
    -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
    -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
    -DCONFIG_FLASH_SIZE=256

3. Build commands for HP application from MRAM:

.. code-block:: bash

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
    ../alif/samples/drivers/pm/system_off \
    -S pm-system-off-hp


Executing Binary on the DevKit
==============================

To execute binaries on the DevKit, follow these steps:

1. Create a JSON configuration file for the SE tool (example assumes RTSS_HE boots from TCM)

.. code-block:: json

   {
       "A32_APP": {
           "disabled": true,
           "binary": "a32_stub_0.bin",
           "version": "1.0.0",
           "signed": true,
           "loadAddress": "0x02000000",
           "cpu_id": "A32_0",
           "flags": ["load", "boot"]
       },
       "HP_APP": {
           "disabled": true,
           "binary": "m55_stub_hp.bin",
           "version": "1.0.0",
           "signed": true,
           "loadAddress": "0x50000000",
           "cpu_id": "M55_HP",
           "flags": ["load", "boot"]
       },
       "HE_APP": {
           "disabled": false,
           "binary": "M55_HE.bin",
           "version": "1.0.0",
           "signed": true,
           "loadAddress": "0x58000000",
           "cpu_id": "M55_HE",
           "flags": ["load", "boot"]
       },
       "DEVICE": {
           "disabled": false,
           "binary": "app-device-config.json",
           "version": "0.5.00",
           "signed": true
       }
   }

2. Flash the application:

   a. Copy the generated binary (e.g., ``zephyr.bin``) into ``<SE tool folder>/build/images``
   b. Copy the JSON configuration file into ``<SE tool folder>/build/config``
   c. Run the following commands in ``<SE tool folder>``:

.. code-block:: bash

   ./app-gen-toc --filename build/config/<your_config_name>.json
   ./app-write-mram

Console Output
================

After the cores boot following a reset, the following prints will be displayed on the console:

.. code-block:: text

   [00:00:00.004,000] <inf> pm_system_off: alif_e7_dk RTSS_HE (TCM boot): PM states demo (RUNTIME_IDLE, S2RAM)
   [00:00:00.015,000] <inf> pm_system_off: POWER STATE SEQUENCE:
   [00:00:00.021,000] <inf> pm_system_off: 1. PM_STATE_RUNTIME_IDLE
   [00:00:00.027,000] <inf> pm_system_off: 2. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.036,000] <inf> pm_system_off: 3. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.044,000] <inf> pm_system_off: 4. (SOFT_OFF skipped - TCM boot, using retention)
   [00:00:00.053,000] <inf> pm_system_off: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.063,000] <inf> pm_system_off: Exited from RUNTIME_IDLE sleep
   [00:00:18.069,000] <inf> pm_system_off: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (20000000 microseconds)
   [00:00:38.081,000] <inf> pm_system_off: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   [00:00:38.090,000] <inf> pm_system_off: Main thread running - iteration 0 - tick: 38090
   [00:00:40.100,000] <inf> pm_system_off: Main thread running - iteration 1 - tick: 40100
   [00:00:42.109,000] <inf> pm_system_off: Main thread running - iteration 2 - tick: 42109
   [00:00:44.118,000] <inf> pm_system_off: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (22000000 microseconds)
   [00:01:06.129,000] <inf> pm_system_off: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   [00:01:06.138,000] <inf> pm_system_off: Main thread running - iteration 0 - tick: 66138
   [00:01:08.148,000] <inf> pm_system_off: Main thread running - iteration 1 - tick: 68148
   [00:01:10.157,000] <inf> pm_system_off: Main thread running - iteration 2 - tick: 70157
   [00:01:12.166,000] <inf> pm_system_off: Skipping PM_STATE_SOFT_OFF (TCM boot, using retention instead)
   [00:01:12.175,000] <inf> pm_system_off: === POWER STATE SEQUENCE COMPLETED ===

