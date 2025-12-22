.. _appnote-zas-utimer-counter:

==============
UTimer Counter
==============

Introduction
=============

The Alif UTimer IP on the Alif Devkit supports counter mode, enabling precise counting of events or clock pulses for applications such as frequency measurement, event counting, or timer-based scheduling. This application note provides a guide to configuring, building, and testing the Zephyr counter sample application (``samples/drivers/counter/alarm/``) using the UTimer as a counter.

Furthermore, the UTIMER is integrated into the Alarm application as a demo application, where it functions as expected. The same demo app is also utilized by the RTC (Real-Time Clock) and LPTIMER. To facilitate configuration, separate overlay and config files for the RTC, UTIMER, and LPTIMER reside in the board’s directory of the Alarm application. Users can select these files using the west build command.

.. include:: prerequisites.rst

.. include:: note.rst

Building an Utimer Counter Application with Zephyr
===================================================

Follow these steps to build the utimer counter application using the Alif Zephyr SDK:


1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository,      please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HE core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he samples/drivers/counter/alarm/
       -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

3. Build commands for applications on the M55 HP core:

.. code-block:: bash

   west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp samples/drivers/counter/alarm/
       -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

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



Console Output
===============

::

   Counter alarm sample

   Set alarm in 2 sec (800000000 ticks)
   !!! Alarm !!!
   Now: 2

   Set alarm in 4 sec (1600000000 ticks)
   !!! Alarm !!!
   Now: 6

   Set alarm in 8 sec (3200000000 ticks)
   !!! Alarm !!!
   Now: 3

   Set alarm in 5 sec (2105032704 ticks)
   !!! Alarm !!!
   Now: 8

   Set alarm in 10 sec (4210065408 ticks)
   !!! Alarm !!!
   Now: 8

   Set alarm in 10 sec (4125163520 ticks)
   !!! Alarm !!!
   Now: 7

   Set alarm in 9 sec (3955359744 ticks)
   !!! Alarm !!!
   Now: 7

   Set alarm in 9 sec (3615752192 ticks)
   !!! Alarm !!!
   Now: 5

   Set alarm in 7 sec (2936537088 ticks)
   !!! Alarm !!!
   Now: 1

   Set alarm in 3 sec (1578106880 ticks)
   !!! Alarm !!!
   Now: 5

   Set alarm in 7 sec (3156213760 ticks)
   !!! Alarm !!!
   Now: 3

   Set alarm in 5 sec (2017460224 ticks)
   !!! Alarm !!!
   Now: 8

   Set alarm in 10 sec (4043920448 ticks)
   !!! Alarm !!!
   Now: 7

   Set alarm in 9 sec (3774873600 ticks)
   !!! Alarm !!!
   Now: 6

   Set alarm in 8 sec (3234779904 ticks)
   !!! Alarm !!!
   Now: 3

   Set alarm in 5 sec (2214592512 ticks)
   !!! Alarm !!!
   Now: 9


