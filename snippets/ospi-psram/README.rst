.. _snippet-ospi-psram:

OSPI PSRAM Application
######################

Overview
********

This snippet selects the appropriate overlay fragment to enable OSPI0 and the
connected APS512XXN PSRAM device based on board dts.

Building and Running
********************

Example command to build:

.. code-block:: console

   west build -b alif_e8_ak/ae822fa0e5597xx0/rtss_he -S ospi-psram ../alif/samples/drivers/spi_psram -p
   OR
   west build -b alif_e8_ak/ae822fa0e5597xx0/rtss_he ../alif/samples/drivers/spi_psram -p -- -DSNIPPET=ospi-psram

Application Output
******************

.. code-block:: console

   PSRAM XIP mode demo app started
   Writing data to the XIP region:
   Reading back:
   Done, total errors = 0
