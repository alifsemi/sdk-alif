.. _appnote-HWSEM:

=======
HWSEM
=======

Introduction
============

This document provides detailed instructions on how to create, compile, and run a demo application for the Hardware Semaphore (HWSEM). The HWSEM is a mechanism used to coordinate concurrency between processor cores when accessing shared resources such as memory regions or peripherals.

.. include:: prerequisites.rst

.. include:: note.rst

Build a HWSEM Application with Zephyr
=====================================

Follow these steps to build the HWSEM application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section ``Setting Up and Building Zephyr Applications``.

2. Build command for application on the hwsem0_test M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


3. Build command for application on the hwsem_test_all M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON


4. Build command for application on the hwsem0_test M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


5. Build command for application on the hwsem_test_all M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
==============================

To execute the binary on the DevKit, run:

.. code-block:: console

    west flash

Console Outputs
===============

This section provides the console outputs for both single HWSEM and all HWSEM test runs on the **alif_e7_dk** board.

Single HWSEM Test Output
------------------------

.. code-block:: console

   I: Hardware Semaphore (HWSEM) example on alif_e7_dk

   I: Locked HWSEM0!

   I: Perform critical work here 1 !!!!

   I: Locked HWSEM0!

   I: Perform critical work here 2 !!!!

   I: Unlocked HWSEM0!

   I: Unlocked HWSEM0!

All HWSEM Output Log
--------------------

.. code-block:: console

   I: Test all 16 Hardware Semaphores(HWSEM) on alif_e7_dk

   I: Locked HWSEM0!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM0!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM0!
   I: Unlocked HWSEM0!

   I: Locked HWSEM1!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM1!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM1!
   I: Unlocked HWSEM1!

   I: Locked HWSEM2!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM2!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM2!
   I: Unlocked HWSEM2!

   I: Locked HWSEM3!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM3!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM3!
   I: Unlocked HWSEM3!

   I: Locked HWSEM4!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM4!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM4!
   I: Unlocked HWSEM4!

   I: Locked HWSEM5!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM5!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM5!
   I: Unlocked HWSEM5!

   I: Locked HWSEM6!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM6!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM6!
   I: Unlocked HWSEM6!

   I: Locked HWSEM7!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM7!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM7!
   I: Unlocked HWSEM7!

   I: Locked HWSEM8!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM8!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM8!
   I: Unlocked HWSEM8!

   I: Locked HWSEM9!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM9!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM9!
   I: Unlocked HWSEM9!

   I: Locked HWSEM10!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM10!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM10!
   I: Unlocked HWSEM10!

   I: Locked HWSEM11!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM11!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM11!
   I: Unlocked HWSEM11!

   I: Locked HWSEM12!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM12!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM12!
   I: Unlocked HWSEM12!

   I: Locked HWSEM13!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM13!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM13!
   I: Unlocked HWSEM13!

   I: Locked HWSEM14!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM14!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM14!
   I: Unlocked HWSEM14!

   I: Locked HWSEM15!
   I: Perform critical work here 1 !!!!
   I: Locked HWSEM15!
   I: Perform critical work here 2 !!!!
   I: Unlocked HWSEM15!
   I: Unlocked HWSEM15!
