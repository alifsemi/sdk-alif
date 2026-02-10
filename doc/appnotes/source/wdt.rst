.. _appnote-zas-WDT_RTSS:

=====
WDT
=====

Introduction
=============

The Real-Time Subsystem Watchdog Timer (WDT_RTSS) is a 32-bit down-counter timer designed to monitor system health and ensure reliable operation. Its primary function is to count down over a predefined period, during which it expects to be serviced (i.e., "fed") by the system to confirm normal operation. If the system fails to service the watchdog before the countdown expires, the WDT_RTSS triggers a recovery action—such as a Non-Maskable Interrupt (NMI) or a CPU reset—to detect and recover from errant or stalled system behavior.

The device integrates up to two WDT_RTSS instances:

- **WDT_HP:** Dedicated to the Arm® Cortex®-M55 High-Performance (M55-HP) processor
- **WDT_HE:** Dedicated to the Arm® Cortex®-M55 High-Efficiency (M55-HE) processor

Key Features of the WDT_RTSS Module:

- 32-bit down-counter architecture
- Counter decrements by one on each rising edge of the watchdog clock
- Configurable generation of a Non-Maskable Interrupt (NMI) upon timeout
- Configurable CPU reset upon timeout

.. include:: prerequisites.rst

.. include:: note.rst

Building an WDT Application with Zephyr
=========================================

The Watchdog Timer (WDT) is integrated into the standard samples/drivers/watchdog application as a demonstration.

Follow these steps to build the WDT application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository,      please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.


2. Build commands for applications on the M55 HE core:

.. code-block:: bash

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/watchdog/


3. Build commands for applications on the M55 HP core:

.. code-block:: bash

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/watchdog/


Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.


Executing Binary on the DevKit
==============================================

To execute binaries on the DevKit follow the command

.. code-block:: bash

   west flash


Console Output
===============

::

  Watchdog sample application
  Attempting to test pre-reset callback
  Feeding watchdog 5 times
  Feeding watchdog...
  Feeding watchdog...
  Feeding watchdog...
  Feeding watchdog...
  Feeding watchdog...
  Waiting for reset...
  Handled things..ready to reset


