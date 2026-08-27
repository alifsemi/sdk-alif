.. _snippet-ospi-psram:

OSPI PSRAM Application
######################

Overview
********

This snippet selects an overlay fragment to enable the DesignWare MSPI
controller and an ISSI IS66WVH HyperRAM device on supported Alif boards.

The current MSPI HyperRAM configuration supports B1, E1C, E3, E5, and E7
boards with IS66WVH memory. E8 Engineering Kit boards use S80KS HyperRAM on
OSPI1.

Building and Running
********************

Example command to build:

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S ospi-psram ../alif/samples/drivers/spi_psram -p
   OR
   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/spi_psram -p -- -DSNIPPET=ospi-psram

The application can be found under :zephyr_file:`samples/drivers/spi_psram` in the Alif tree.
