.. _appnote-entropy:

=========
ENTROPY
=========

Introduction
=============

This document provides instructions on how to create, compile, and run a demo application for Entropy, which refers to the measure of unpredictability or randomness typically gathered from hardware or software sources. Such entropy is essential for cryptographic and security-related operations.

.. include:: prerequisites.rst

Building Entropy Application in Zephyr
=======================================

Follow these steps to build the Entropy application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the M55 HE core:

.. code-block:: bash

    west build -p always -b alif_e7_dk_rtss_he tests/drivers/entropy/api/

3. Build commands for applications on the M55 HP core:

.. code-block:: bash

    west build -p always -b alif_e7_dk_rtss_hp tests/drivers/entropy/api/

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit, follow the command:

.. code-block:: bash

    west flash

Console Output
===============

.. code-block:: text

   *** Booting Zephyr OS 4.1 build ZAS 2.0  4c97d8baffec ***
   Running TESTSUITE entropy_api
   ===========================================================
   START - test_entropy_get_entropy
   random device is 0x8078, name is rng
     0x3b
     0x39
     0x7b
     0x1f
     0xce
     0x8c
     0x47
     0xc6
     0xc4
   PASS - test_entropy_get_entropy in 0.031 seconds
   ===========================================================
   TESTSUITE entropy_api succeeded

   ------ TESTSUITE SUMMARY START ------
   SUITE PASS - 100.00% [entropy_api]: pass = 1, fail = 0, skip = 0, total = 1 duration = 0.031 seconds
     - PASS - [entropy_api.test_entropy_get_entropy] duration = 0.031 seconds
   ------ TESTSUITE SUMMARY END ------
   ===========================================================
   PROJECT EXECUTION SUCCESSFUL

.. include:: west_debug.rst