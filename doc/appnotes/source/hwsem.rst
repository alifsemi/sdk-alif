.. _appnote-HWSEM:

=======
HWSEM
=======

Introduction
=============

This document provides detailed instructions on how to create, compile, and run a demo application for the Hardware Semaphore (HWSEM). The HWSEM is a mechanism used to coordinate concurrency between processor cores when accessing shared resources such as memory regions or peripherals.

.. include:: prerequisites.rst

.. include:: note.rst

Building an HWSEM Application with Zephyr
==========================================

Follow these steps to build the HWSEM application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

2. Build commands for applications on the hwsem0_test M55 HE core:

.. code-block:: bash

    west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_alif_hwsem/

3. Build commands for applications on the hwsem_test_all M55 HE core:

.. code-block:: bash

    west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/ipm/ipm_alif_hwsem/ -DHWSEM_ALL=ON

4. Build commands for applications on the hwsem0_test M55 HP core:

.. code-block:: bash

    west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_alif_hwsem/

5. Build commands for applications on the hwsem_test_all M55 HP core:

.. code-block:: bash

    west build -p always -b alif_e7_dk/ae722f80f55d5xx/rtss_hp ../alif/samples/drivers/ipm/ipm_alif_hwsem/ -DHWSEM_ALL=ON

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit, follow the command:

.. code-block:: bash

    west flash

Console Outputs
================

This section provides the console outputs for both single HWSEM and all HWSEM test runs on the **alif_e7_dk** board.

Single HWSEM Test Output
--------------------------

.. code-block:: console

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
---------------------

.. code-block:: console

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
