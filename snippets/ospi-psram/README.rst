.. _snippet-ospi-psram:

OSPI PSRAM Application
######################

Overview
********

This snippet selects an overlay fragment to enable the OSPI controller and connected PSRAM
device based on the selected Alif board.

Building and Running
********************

Example command to build:

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S ospi-psram ../alif/samples/drivers/spi_psram -p
   OR
   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/spi_psram -p -- -DSNIPPET=ospi-psram

The application can be found under :zephyr_file:`samples/drivers/spi_psram` in the Alif tree.
