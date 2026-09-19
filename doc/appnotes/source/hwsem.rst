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

Follow these steps to build the HWSEM sample application using the Alif Zephyr
SDK:

For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr
repository, refer to the `ZAS User Guide`_.

Alif E7 DevKit
---------------

Build for SoC variant ``ae722f80f55d5xx``, hwsem0_test M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae722f80f55d5xx``, hwsem_test_all M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON


Build for SoC variant ``ae722f80f55d5xx``, hwsem0_test M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae722f80f55d5xx``, hwsem_test_all M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON

Build for SoC variant ``ae302f80f55d5xx``, hwsem0_test M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae302f80f55d5xx``, hwsem_test_all M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON


Build for SoC variant ``ae302f80f55d5xx``, hwsem0_test M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae302f80f55d5xx``, hwsem_test_all M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae302f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON

Alif E7 AppKit
----------------

Build for SoC variant ``ae722f80f55d5xx``, hwsem0_test M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae722f80f55d5xx``, hwsem_test_all M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON


Build for SoC variant ``ae722f80f55d5xx``, hwsem0_test M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae722f80f55d5xx``, hwsem_test_all M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_ak/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON

Alif E8 DevKit
---------------

Build for SoC variant ``ae822fa0e5597xx0``, hwsem0_test M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae822fa0e5597xx0``, hwsem_test_all M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON


Build for SoC variant ``ae822fa0e5597xx0``, hwsem0_test M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae822fa0e5597xx0``, hwsem_test_all M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON

Build for SoC variant ``ae402fa0e5597xx0``, hwsem0_test M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae402fa0e5597xx0``, hwsem_test_all M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON


Build for SoC variant ``ae402fa0e5597xx0``, hwsem0_test M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae402fa0e5597xx0``, hwsem_test_all M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae402fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON

Alif E8 AppKit
----------------

Build for SoC variant ``ae822fa0e5597xx0``, hwsem0_test M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae822fa0e5597xx0``, hwsem_test_all M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem/ \
     -- \
     -DHWSEM_ALL=ON


Build for SoC variant ``ae822fa0e5597xx0``, hwsem0_test M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/ipm/ipm_alif_hwsem


Build for SoC variant ``ae822fa0e5597xx0``, hwsem_test_all M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_ak/ae822fa0e5597xx0/rtss_hp \
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

.. code-block:: text

    I: Hardware Semaphore (HWSEM) example on alif_e7_dk
    I: hwsem_lock: HWSEM locked!
    I: Locked HWSEM0!
    I: Perform critical work here 1 !!!!
    I: hwsem_lock: Already locked HWSEM is locked again
    I: Locked HWSEM0!
    I: Perform critical work here 2 !!!!
    I: hwsem_unlock: HWSEM unlocked!
    I: Unlocked HWSEM0!
    I: hwsem_unlock: HWSEM unlocked!
    I: Unlocked HWSEM0!

All HWSEM Output Log
--------------------

.. code-block:: text

    I: Test all 16 Hardware Semaphores(HWSEM) on alif_e7_dk
    I: hwsem_trylock: HWSEM locked!
    I: Locked HWSEM0!
    I: Perform critical work here 1 !!!!
    I: hwsem_trylock: Already locked HWSEM is locked again
    I: Locked HWSEM0!
    I: Perform critical work here 2 !!!!
    I: hwsem_unlock: HWSEM unlocked!
    I: Unlocked HWSEM0!
    ...
    (repeat for HWSEM1 to HWSEM15 outputs)
