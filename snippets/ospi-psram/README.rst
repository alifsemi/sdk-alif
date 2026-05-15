.. _snippet-ospi-psram:

OSPI PSRAM Snippet
###################

Overview
********

This snippet selects the appropriate overlay fragment to enable OSPI0 and the
connected APS512XXN PSRAM device based on board DTS.

Building and Running
********************

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/spi_psram
   :board: alif_e8_ak/ae822fa0e5597xx0/rtss_he
   :goals: build
   :gen-args: -S ospi-psram
   :compact:

Application Output
******************

.. code-block:: console

   PSRAM XIP mode demo app started
   Writing data to the XIP region:
   Reading back:
   Done, total errors = 0
