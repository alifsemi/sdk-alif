.. _appnote-zas-utimer-counter:

==============
UTimer Counter
==============

Introduction
=============

The Alif UTimer IP on the Alif DevKit supports counter mode, enabling precise counting of events or clock pulses for applications such as frequency measurement, event counting, or timer-based scheduling. This application note provides a guide to configuring, building, and testing the Zephyr counter sample application (``samples/drivers/counter/alarm/``) using the UTimer as a counter.

Furthermore, the UTIMER is integrated into the Alarm application as a demo application, where it functions as expected. The same demo app is also utilized by the RTC (Real-Time Clock) and LPTIMER. To facilitate configuration, separate overlay and config files for the RTC, UTIMER, and LPTIMER reside in the board’s directory of the Alarm application. Users can select these files using the west build command.

.. include:: prerequisites.rst

.. include:: note.rst

Build a Utimer Counter Application with Zephyr
===============================================

Follow these steps to build the UTimer counter application using the Alif Zephyr SDK:

For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr
repository, refer to the `ZAS User Guide`_.

Alif E7 DevKit
---------------

Build for SoC variant ``ae722f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ae722f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ae302f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ae302f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Alif E7 AppKit
---------------

Build for SoC variant ``ae722f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ae722f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Alif E8 DevKit
---------------

Build for SoC variant ``ae822fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ae822fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ae402fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ae402fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Alif E8 AppKit
---------------

Build for SoC variant ``ae822fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ae822fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Alif E1C DevKit
----------------

Build for SoC variant ``ae1c1f4051920hh``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Alif B1 DevKit
---------------

Build for SoC variant ``ab1c1f4m51820ph0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ab1c1f4m51820hh0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ab1c1f1m41820hh0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Build for SoC variant ``ab1c1f1m41820ph0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit, follow the command:

.. code-block:: console

   west flash

Expected Result
===============

When running the emulator-based counter application:

- The application configures the UTimer as a counter in the emulated environment.
- Simulated input pulses increment the counter, and the alarm callback triggers when the counter reaches the configured threshold.
- Output is printed to the host terminal, showing counter values or alarm events.

Console Output
===============

.. code-block:: text

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
