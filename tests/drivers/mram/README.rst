MRAM Flash Driver Tests
#######################

Overview
========

This test suite validates the Alif MRAM on Zephyr RTOS.
It uses the ZTest framework and covers core flash functionality,
LittleFS filesystem integration, performance benchmarking, and stress
testing.

The suite is split into two main configurations:

- Core flash driver tests (functional, performance, stress)
- Filesystem tests (LittleFS over MRAM)

Test Suites
===========

- **functional**: Verifies flash API behavior including erase, write,
  read, alignment handling, boundary checks, and data integrity.

- **performance**: Measures erase, write, and read throughput across
  the storage partition and reports average latency and bandwidth.

- **stress**: Exercises the driver under repeated operations,
  boundary-crossing writes, alternating patterns, full partition usage,
  and pseudo-random access patterns.

- **filesystem**: Validates LittleFS integration on MRAM including
  mount/unmount, file operations, persistence across remount, and
  filesystem statistics.

Manual Build Instructions
*************************

**Core flash tests:**

.. code-block:: console

   west build -p always -b <board> -S alif-mram tests/mram
   west flash

**Filesystem (LittleFS) tests:**

.. code-block:: console

   west build -p always -b <board> -S alif-mram-filesystem tests/mram
   west flash

Notes
=====

- All tests operate on the ``storage_partition`` defined in Devicetree.
- Flash erase alignment and write constraints are validated explicitly.
- MRAM allows overwrite without erase, and this behavior is tested.
- Filesystem tests ensure persistence across unmount/remount cycles.
