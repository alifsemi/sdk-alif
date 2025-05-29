.. _appnote-zas-qdec:

====
QDEC
====

Introduction
============

The Alif UTIMER IP on the Alif Ensemble Devkit supports Quadrature Decoder (QDEC) mode, enabling precise position tracking of a mechanical rotary encoder. This mode is ideal for applications requiring angular position feedback, such as motor control, robotics, or user interface dials. This application note guides developers through configuring, building, and running a Zephyr-based QDEC application (``samples/sensor/qdec/``) using the UTIMER peripheral on the Alif Ensemble Devkit.

Hardware Requirements and Setup
===============================

Hardware Requirements
---------------------

To run the QDEC application, you need:

- Alif Ensemble Devkit
- Debugger: ULINKpro or JLink

Software Requirements
=====================

To develop and run QDEC applications on the Alif Ensemble Devkit with Zephyr, you need:

- **Alif SDK**: Clone from `https://github.com/alifsemi/sdk-alif <https://github.com/alifsemi/sdk-alif>`_
- **West Tool**: For building Zephyr applications (installed via ``pip install west``)
- **Arm GCC Compiler**: For compiling the application (part of the Zephyr SDK)
- **Arm DS (Development Studio)** or equivalent IDE for debugging
- **SE Tools (optional)**: For loading binaries (refer to Alif documentation)

Compilation Procedure
=====================

Fetching the Alif Zephyr SDK
----------------------------

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

Building the Application
------------------------

Navigate to the SDK directory and build the application with TCM Memory. Refer to the User Guide for all other build options.

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_he samples/sensor/qdec/ \
       -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0 -DCONFIG_FLASH_SIZE=256

Locating Output Files
---------------------

- **Binary File**: ``zephyr.bin``
- **Location**: ``./build/zephyr/``

Loading Binary on Alif Ensemble Devkit
======================================

To load and debug the QDEC application on the DevKit-E7 or DevKit-E1C using ULINKpro in Arm DS, follow these steps:

1. Open the **Debug Configuration** window with *Create, manage, and run configurations*.

   .. figure:: _static/debug_config_window.png
      :alt: Debug Configuration Window
      :align: center

      Debug Configuration Window

2. In the **Connection** tab, ensure the correct Core and ULINKpro selections are made. In the **Select Target** section, choose:

   - ``Cortex-M55_0`` for M55-HP core
   - ``Cortex-M55_1`` for M55-HE core

   .. figure:: _static/connections_tab.png
      :alt: Connection Tab Settings
      :align: center

      Connection Tab Settings

3. Go to the **Debugger** tab and select **Connect Only**. Use the ``loadfile`` command to specify the path to your application's ``.elf`` executable file.

4. Click the **Debug** symbol to load the debugging information, then click **Apply** and **Debug** to start the debugging process.

   .. figure:: _static/debugger_tab.png
      :alt: Debugger Tab Settings
      :align: center

      Debugger Tab Settings

Expected Result
===============

Once the application is loaded and the mechanical encoder is connected:

- The devkit runs the QDEC sample, reading the UTIMER counter in quadrature decoder mode.
- The angular position is printed every second to the console via UART4 (M55 HE core).

Sample Output:

.. code-block:: text

   *** Booting Zephyr OS build zas-v1.2-109-g81dbfb8f3841 ***
   Quadrature decoder sensor test
   Quadrature encoder emulator enabled with 100 ms period
   Position = 0 degrees
   Position = 7 degrees
   Position = 14 degrees
   Position = 21 degrees
   Position = 28 degrees
   Position = 36 degrees
   Position = 43 degrees
   Position = 50 degrees
   Position = 57 degrees
   Position = 64 degrees
   Position = 72 degrees
   Position = 79 degrees
   Position = 86 degrees
   Position = 93 degrees
   …

