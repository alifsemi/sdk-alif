.. _appnote-zephyr-hello-world-ospi:

=======================================
Building Hello World for OSPI NOR Flash
=======================================

Introduction
============

This application note describes how to build and execute a "HelloWorld" application using the Zephyr RTOS, configured to run from OSPI1 NOR flash on the Alif E7 DevKit. The application prints a "Hello World" message along with the board name and targets the Real-Time Subsystem High-Performance (RTSS-HP) and Real-Time Subsystem High-Efficiency (RTSS-HE) cores. Optionally, the Zephyr Application Binary (ZAS) can be encrypted with AES for secure execution.

Execution Path
==============

- **RTSS-HP**: Runs from OSPI1 NOR flash at 0xC0000000.
- **RTSS-HE**: Runs from OSPI1 NOR flash at 0xC0200000.

Hardware Requirements
=====================

- **Alif DevKit**: Provides the hardware platform, including the RTSS-HP (Cortex-M55_0) and RTSS-HE (Cortex-M55_1) cores, with OSPI1 NOR flash for non-volatile storage and execution.
- **OSPI1 NOR Flash**: A non-volatile memory mapped at 0xC0000000, configured in Execute-in-Place (XiP) mode to run the application binary directly.

Hardware Connections and Setup
==============================

No additional hardware connections are required beyond the standard Alif DevKit setup. Ensure the OSPI1 NOR flash is accessible and properly configured in the system firmware.

Software Requirements
=====================

- **Zephyr RTOS**: Provides the framework for building and running the application.
- **West Tool**: Used to manage Zephyr builds.
- **GCC Compiler**: Included in the Zephyr SDK for compiling the application.
- **AES Encryption Tool (Optional)**: ``/home/$USER/ZAS-v1.2.0-Beta/prebuilt-images/CSPI_AES128_ECB`` for encrypting the binary.

.. note:: In the above commands, `v1.2.0` is an example version. Replace `Zephyr-Ensemble-E7-B0-RTSS-v1.2.0-Beta` with the latest version of the Alif Zephyr SDK.

Building the Application Binary Executable from OSPI Region
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

Kconfig Modifications
---------------------

To ensure proper execution from OSPI1 NOR flash, modify the ``soc/arm/alif_ensemble/e7/Kconfig.series`` file by commenting out the following lines:

.. code-block:: text

   # select INIT_ARCH_HW_AT_BOOT
   # select PLATFORM_SPECIFIC_INIT

Example Build Commands
----------------------

Below are example build commands using the west tool for each configuration.

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

**RTSS-HE Example**

This example builds the hello world sample for the RTSS-HE target on the alif_e7_dk_rtss_he board:

.. code-block:: bash

   west build -b alif_e7_dk_rtss_he samples/hello_world \
       -DCONFIG_ARM_MPU=n \
       -DCONFIG_FLASH_BASE_ADDRESS=0xC0200000 \
       -p always

**RTSS-HP Example**

This command builds the uart/echo_bot driver sample for the RTSS-HP target on the alif_e7_dk_rtss_hp board:

.. code-block:: bash

   west build -b alif_e7_dk_rtss_hp samples/drivers/uart/echo_bot \
       -DCONFIG_ARM_MPU=n \
       -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 \
       -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
       -p always

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

Executing Binary on DevKit-E7
=============================

Flash the Binary
----------------

Flash the ZAS binary (encrypted or unencrypted) to OSPI1 NOR flash.

Program ATOC and Boot
---------------------

Use the appropriate configuration file to program the Application Table of Contents (ATOC) into MRAM and boot the application:

- **RTSS-HP (Unencrypted)**: ``/home/$USER/app-release-exec-linux/build/config/zephyr_e7_rtsshp_ospi1_helloworld.json``
- **RTSS-HE (Unencrypted)**: ``/home/$USER/app-release-exec-linux/build/config/zephyr_e7_rtsshe_ospi1_helloworld.json``
- **RTSS-HP (Encrypted)**: ``/home/$USER/app-release-exec-linux/build/config/zephyr_e7_rtsshp_ospi1_en_helloworld.json``
- **RTSS-HE (Encrypted)**: ``/home/$USER/app-release-exec-linux/build/config/zephyr_e7_rtsshe_ospi1_en_helloworld.json``

Sample Output
=============

Below is the expected console output for RTSS-HP and RTSS-HE:

**RTSS-HP (0xC0000000)**

.. code-block:: text

   *** Booting Zephyr OS build zas-v1.2-30-g25c1cf9151af ***
   Hello! I'm your echo bot.
   Tell me something and press enter:

**RTSS-HE (0xC0200000)**

.. code-block:: text

   *** Booting Zephyr OS build zas-v1.2-30-g25c1cf9151af ***
   Hello World! alif_e7_dk_rtss_he

Observation
===========

- The application successfully boots from OSPI1 NOR flash.
- The board name in the output verifies the correct target configuration.