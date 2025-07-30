.. _appnote-zas-utimer-counter:

==============
UTimer Counter
==============

Introduction
============

The Alif UTimer IP on the Alif Ensemble Devkit supports counter mode, enabling precise counting of events or clock pulses for applications such as frequency measurement, event counting, or timer-based scheduling. This application note provides a guide to configuring, building, and testing the Zephyr counter sample application (``samples/drivers/counter/alarm/``) using the UTimer as a counter.

Hardware Requirements and Setup
===============================

Hardware Requirements
---------------------

To run the Counter application, you need:

- Alif Ensemble Devkit
- Debugger: ULINKpro or JLink

Software Requirements
=====================

To develop and run Counter applications on the Alif Ensemble Devkit with Zephyr, you need:

- **Alif SDK**: Clone from `https://github.com/alifsemi/sdk-alif <https://github.com/alifsemi/sdk-alif>`_
- **West Tool**: For building Zephyr applications (installed via ``pip install west``)
- **Arm GCC Compiler**: For compiling the application (part of the Zephyr SDK)
- **Arm DS (Development Studio)** or equivalent IDE for debugging
- **SE Tools (optional)**: For loading binaries (refer to Alif documentation)

Compilation Procedure
=====================

Fetching the Alif Zephyr SDK
----------------------------

1. For instructions on fetching the Alif Zephyr SDK, please refer to the `ZAS User Guide`_

Building the Application
------------------------

Navigate to the SDK directory and build the application with TCM Memory. The provided configuration and overlay files ensure the UTimer functions as a counter, with RTC and Timer functionalities disabled.

.. note::
   The application is designed for the Alif Ensemble E7 DevKit. Modify the sample code as needed for other DevKits.

.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_he samples/drivers/counter/alarm/ \
       -DOVERLAY_CONFIG=$PWD/zephyr/samples/drivers/counter/alarm/boards/alif_utimer.conf \
       -DDTC_OVERLAY_FILE=$PWD/zephyr/samples/drivers/counter/alarm/boards/alif_utimer.overlay \
       -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0 -DCONFIG_FLASH_SIZE=256

Locating Output Files
---------------------

- **Executable File**: ``zephyr.exe``
- **Location**: ``./build/zephyr/``

Loading Binary on Alif Ensemble Devkit
======================================

To load and debug the Counter application on the DevKit using ULINKpro in Arm DS, follow these steps:

1. Open the **Debug Configuration** window with *Create, manage, and run configurations*.

   .. figure:: _static/debug_config_window.png
      :alt: Debug Configuration Window
      :align: center

      Debug Configuration Window

2. In the **Connection** tab, ensure the correct Core and ULINKpro selections are made. In the **Select Target** section, choose:

   - ``Cortex-M55_0`` for M55-HP core
   - ``Cortex-M55_1`` for M55-HE core

   .. figure::_static/connections_tab.png
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

When running the emulator-based counter application:

- The application configures the UTimer as a counter in the emulated environment.
- Simulated input pulses increment the counter, and the alarm callback triggers when the counter reaches the configured threshold.
- Output is printed to the host terminal, showing counter values or alarm events.



Sample Output
=============
   .. figure:: _static/sample_output.png
      :alt: Sample Output of Counter Application
      :align: center

      Sample Output of the Counter Application

