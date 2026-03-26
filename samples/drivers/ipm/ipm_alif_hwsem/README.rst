.. _ipm_ipm_alif_hwsem_sample:

HWSEM test Sample
#################

Overview
********

A semaphore is a variable or abstract data type that provides
a simple but useful abstraction for controlling access by multiple
cores/processes to a common resource in a multi-core environment like
there is in the Ensemble DevKit.
The DevKit supports a total of 16 hardware semaphores (HWSEM0 - HWSEM15).
Their purpose is to control and facilitate access to a shared resources
or perform critical operations without conflicts or race conditions between
two or more cores.

The testcases,
 * hwsem0_test.c - locks and unlocks HWSEM0 twice after performing
   dummy critical operations. The call to lock HWSEM0 is blocked until
   HWSEM0 becomes free.
 * hwsem_test_all.c - Tries to lock and unlock all 16 HWSEMs. If a specific
   HWSEM is already locked it return -EBUSY or negative value on error.

Building and Running
********************

The application will build only for a target that has a devicetree entry
with :dt compatible:`alif,hwsem` as a compatible.
It does not work on QEMU.
In this example below the :ref:`alif_e7_dk/ae722f80f55d5xx/rtss_he` board is used.

1. Building HWSEM0 testcase.

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/ipm/ipm_alif_hwsem
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build

Sample output
*************

.. code-block:: console
   I: Hardware Semaphore (HWSEM) example on alif_e7_dk

   I: Locked HWSEM0!

   I: Perform critical work here 1 !!!!

   I: Locked HWSEM0!

   I: Perform critical work here 2 !!!!

   I: Unlocked HWSEM0!

   I: Unlocked HWSEM0!

2. Building testcase to test all 16 HWSEMs.

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/ipm/ipm_alif_hwsem
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :gen-args: -DHWSEM_ALL=ON

Sample output
*************

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
