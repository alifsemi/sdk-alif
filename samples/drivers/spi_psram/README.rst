
.. _psram-test:

PS RAM Test
###############

Overview
********

This is the test application to test and verify PSRAM (APS512XXN) over OSPI interface.
This test application is only available for E4/E8 SOC boards.


Building and Running
********************

The application will build only for a target that has a devicetree entry with
*:dt compatible:`alif,apmemory-aps512xxn`* as a compatible.
Use respective ovderlay files in boards folder for AP PSRAM HyperRam with build command.

Sample Output
=============

.. code-block:: console


PSRAM XIP mode demo app started
Writing data to the XIP region:
Reading back:
Done, total errors = 0
