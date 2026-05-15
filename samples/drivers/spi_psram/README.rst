
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
Use the ``ospi-psram`` snippet to enable the OSPI controller and AP PSRAM
HyperRAM nodes.

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/spi_psram
   :board: alif_e8_ak/ae822fa0e5597xx0/rtss_he
   :goals: build
   :gen-args: -S ospi-psram
   :compact:

Sample Output
=============

.. code-block:: console


PSRAM XIP mode demo app started
Writing data to the XIP region:
Reading back:
Done, total errors = 0
