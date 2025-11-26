.. _appnote-zephyr-hello-world-ospi:

=======================================
Building Hello World for OSPI NOR Flash
=======================================

Introduction
============

This application note describes how to build and execute a "HelloWorld" application using the Zephyr RTOS, configured to run from OSPI1 NOR flash on the Alif E7 DevKit. The application prints a "Hello World" message along with the board name and targets the Real-Time Subsystem High-Performance (RTSS-HP) and Real-Time Subsystem High-Efficiency (RTSS-HE) cores. Optionally, the Zephyr Application Binary (ZAS) can be encrypted with AES for secure execution.

.. include:: prerequisites.rst


Execution Address
===================

- **RTSS-HP**: Runs from OSPI1 NOR flash at 0xC0000000.
- **RTSS-HE**: Runs from OSPI1 NOR flash at 0xC0200000.

Hardware Connections and Setup
==============================

No additional hardware connections are required beyond the standard Alif DevKit setup. Ensure the OSPI1 NOR flash is accessible and properly configured in the system firmware.

Building Binary Executable from OSPI Region
===========================================================

Below are the required configurations for two different setups: RTSS-HE and RTSS-HP.

Required Configuration Settings
-------------------------------

Add the following line to disable the ARM MPU (Memory Protection Unit) for both configurations:

.. code-block:: text

   CONFIG_ARM_MPU=n

Additional Defines for RTSS-HE
------------------------------

For the RTSS-HE (High Efficiency) configuration, use these flash memory settings:

.. code-block:: text

   CONFIG_FLASH_BASE_ADDRESS=0xC0200000

Additional Defines for RTSS-HP
------------------------------

For the RTSS-HP (High Performance) configuration, use these flash memory settings:

.. code-block:: text

   CONFIG_FLASH_BASE_ADDRESS=0xC0000000
   CONFIG_FLASH_LOAD_OFFSET=0x0

Example Build Commands
----------------------

Below are example build commands using the west tool for each configuration.

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

**RTSS-HE Example**

This example builds the hello world sample for the RTSS-HE target on the alif_e7_dk/ae722f80f55d5xx/rtss_he board:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/hello_world \
       -DCONFIG_ARM_MPU=n \
       -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000 \

**RTSS-HP Example**

This command builds the uart/echo_bot driver sample for the RTSS-HP target on the alif_e7_dk/ae722f80f55d5xx/rtss_hp board:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/drivers/uart/echo_bot \
       -DCONFIG_ARM_MPU=n \
       -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 \
       -DCONFIG_FLASH_LOAD_OFFSET=0x0 \

Encrypting the ZAS Application Binary (Optional)
================================================

To secure the application, encrypt the ZAS binary using a 16-byte AES key.

Encrypt the Binary
------------------

.. code-block:: bash

   /home/$USER/ZAS-v1.2.0/prebuilt-images/CSPI_AES128_ECB \
       -i build/zephyr/zephyr.bin \
       -o build/zephyr/zephyr_en.bin \
       -k '0123456789ABCDEF' \
       -d 1

- ``-k '0123456789ABCDEF'``: Example 16-byte AES key (replace with your own key).
- ``-d 1``: Enables encryption.

Save the Encrypted Binary
-------------------------

**RTSS-HP**

.. code-block:: bash

   cp build/zephyr/zephyr_en.bin /home/$USER/app-release-exec-linux/build/images/zephyr_e7_rtsshp_ospi1_en_helloworld.bin

**RTSS-HE**

.. code-block:: bash

   cp build/zephyr/zephyr_en.bin /home/$USER/app-release-exec-linux/build/images/zephyr_e7_rtsshe_ospi1_en_helloworld.bin

Executing Binary on DevKit
=============================

Flash the binary using the PC tool
-----------------------------------

Flash the ZAS binary (encrypted or unencrypted) to OSPI1 NOR flash.

Program ATOC and Boot
---------------------

Use the appropriate configuration file to program the Application Table of Contents (ATOC) into MRAM and boot the application:

- **RTSS-HP (Unencrypted)**: ``/home/$USER/app-release-exec-linux/build/config/zephyr_e7_rtsshp_ospi1_helloworld.json``
- **RTSS-HE (Unencrypted)**: ``/home/$USER/app-release-exec-linux/build/config/zephyr_e7_rtsshe_ospi1_helloworld.json``
- **RTSS-HP (Encrypted)**: ``/home/$USER/app-release-exec-linux/build/config/zephyr_e7_rtsshp_ospi1_en_helloworld.json``
- **RTSS-HE (Encrypted)**: ``/home/$USER/app-release-exec-linux/build/config/zephyr_e7_rtsshe_ospi1_en_helloworld.json``

Console Output
================

Below is the expected console output for RTSS-HP and RTSS-HE:

**RTSS-HP (0xC0000000)**

.. code-block:: text

   Hello! I'm your echo bot.
   Tell me something and press enter:

**RTSS-HE (0xC0200000)**

.. code-block:: text

   Hello World! alif_e7_dk/ae722f80f55d5xx/rtss_he

Observation
===========

- The application successfully boots from OSPI1 NOR flash.
- The board name in the output verifies the correct target configuration.