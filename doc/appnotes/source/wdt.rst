.. _appnote-zas-WDT_RTSS:

=====
WDT
=====

Introduction
============

The Real-Time Subsystem Watchdog Timer module, hereinafter referred to as WDT_RTSS, is a timer based on a 32-bit down-counter. The basic function of the WDT_RTSS is to count for a fixed period, during which it expects to be serviced by the system, indicating normal operation. The WDT_RTSS provides a mechanism to detect errant system behavior and recover from an unknown state by causing a system reset if the count period elapses without intervention.

The device includes up to two WDT_RTSS modules:

* WDT_HP: Dedicated to the Arm Cortex-M55 High-Performance (M55-HP) processor
* WDT_HE: Dedicated to the Arm Cortex-M55 High-Efficiency (M55-HE) processor

Features
-----------

The WDT_RTSS module supports the following main features:

* 32-bit down-counter
* Counter decrements by one on each positive watchdog clock edge
* Configurable NMI generation upon watch period expiration
* Configurable CPU reset upon watch period expiration

.. include:: prerequisites.rst

.. include:: note.rst

Build a WDT Application with Zephyr
=====================================

The Watchdog Timer (WDT) is integrated into the standard samples/drivers/watchdog application as a demonstration.

Follow these steps to build the WDT application using the Alif Zephyr SDK:

For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr
repository, refer to the `ZAS User Guide`_.

Alif E7 DevKit
---------------

Build for SoC variant ``ae722f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ae722f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ae302f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ae302f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     samples/drivers/watchdog -S alif-wdt

Alif E7 AppKit
---------------

Build for SoC variant ``ae722f80f55d5xx``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ae722f80f55d5xx``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     samples/drivers/watchdog -S alif-wdt

Alif E8 DevKit
---------------

Build for SoC variant ``ae822fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ae822fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ae402fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ae402fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     samples/drivers/watchdog -S alif-wdt

Alif E8 AppKit
---------------

Build for SoC variant ``ae822fa0e5597xx0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ae822fa0e5597xx0``, M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     samples/drivers/watchdog -S alif-wdt

Alif E1C DevKit
----------------

Build for SoC variant ``ae1c1f4051920hh``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Alif B1 DevKit
---------------

Build for SoC variant ``ab1c1f4m51820ph0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ab1c1f4m51820hh0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ab1c1f1m41820hh0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820hh0/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Build for SoC variant ``ab1c1f1m41820ph0``, M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_b1_dk/ab1c1f1m41820ph0/rtss_he \
     samples/drivers/watchdog -S alif-wdt

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit, follow the command:

.. code-block:: console

   west flash

Console Output
===============

.. code-block:: text

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