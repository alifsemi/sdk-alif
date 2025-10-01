.. _appnote-zas-utimer-counter:

==============
UTimer Counter
==============

Introduction
=============

The Alif UTimer IP on the Alif Devkit supports counter mode, enabling precise counting of events or clock pulses for applications such as frequency measurement, event counting, or timer-based scheduling. This application note provides a guide to configuring, building, and testing the Zephyr counter sample application (``samples/drivers/counter/alarm/``) using the UTimer as a counter.

Furthermore, the UTIMER is integrated into the Alarm application as a demo application, where it functions as expected. The same demo app is also utilized by the RTC (Real-Time Clock) and LPTIMER. To facilitate configuration, separate overlay and config files for the RTC, UTIMER, and LPTIMER reside in the board’s directory of the Alarm application. Users can select these files using the west build command.

Prerequisites
===============

Hardware Requirements
---------------------

To run the counter application, you need:

- **Alif Devkit**
- **Debugger: JLink (optional)**

Software Requirements
-----------------------

To develop and run counter applications on the Alif Devkit with Zephyr, you need:

- **Alif SDK**: Clone from `https://github.com/alifsemi/sdk-alif <https://github.com/alifsemi/sdk-alif>`_
- **West Tool**: For building Zephyr applications (installed via ``pip install west``)
- **Arm GCC Compiler**: For compiling the application (part of the Zephyr SDK)
- **SE Tools (optional)**: For loading binaries (refer to Alif documentation)

Building Counter Application in Zephyr
========================================

Follow these steps to build the counter application using the Alif Zephyr SDK:

Navigate to the SDK directory and build the application with TCM Memory. The provided configuration and overlay files ensure the UTimer functions as a counter, with RTC and Timer functionalities disabled.

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository,      please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, please modify the board name in the build command accordingly. For more information, refer to the ZAS User Guide.

2. Build commands for applications on the M55 HE core using the Ninja build command:


.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_he samples/drivers/counter/alarm/ \
       -DOVERLAY_CONFIG=$PWD/zephyr/samples/drivers/counter/alarm/boards/alif_utimer.conf \
       -DDTC_OVERLAY_FILE=$PWD/zephyr/samples/drivers/counter/alarm/boards/alif_utimer.overlay \
       -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0 -DCONFIG_FLASH_SIZE=256

3. Build commands for applications on the M55 HP core using the Ninja build command:


.. code-block:: bash

   rm -rf build
   west build -b alif_e7_dk_rtss_hp samples/drivers/counter/alarm/ \
       -DOVERLAY_CONFIG=$PWD/zephyr/samples/drivers/counter/alarm/boards/alif_utimer.conf \
       -DDTC_OVERLAY_FILE=$PWD/zephyr/samples/drivers/counter/alarm/boards/alif_utimer.overlay \
       -DCONFIG_FLASH_BASE_ADDRESS=0 -DCONFIG_FLASH_LOAD_OFFSET=0 -DCONFIG_FLASH_SIZE=256

.. note::
   To address various scenarios, such as utilizing MRAM or flash addresses and employing alternative compilers like LLVM or ARMCLANG, refer to the document AUGD0008_Getting-Started-with-ZAS-for-Ensemble-v0.5.0-Beta.

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.


Executing Binary on the DevKit
==============================================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash


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

