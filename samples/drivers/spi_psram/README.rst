
.. _psram-test:

PS RAM Test
###############

Overview
********

This is the test application to test and verify PSRAM/HyperRAM devices over
the OSPI interface.
This test application is only available for E4/E8 SOC boards.


Building and Running
********************

The application will build only for a target that has a devicetree entry with
*:dt compatible:`alif,apmemory-aps512xxn`* or
*:dt compatible:`alif,infineon-s80ks2564`* as a compatible.
Use respective overlay files in the boards folder for APS PSRAM or S80KS
HyperRAM with the build command.

S80KS HyperRAM Test
===================

The ``alif_hex_s80ks.overlay`` file provides the devicetree configuration for
testing the Infineon S80KS HyperRAM with this sample.

This overlay was tested on the Alif E8 CSP engineering board. On the Alif E8
engineering board, the S80KS HyperRAM is connected over OSPI1, but the sample is
still built using the Alif E8 DevKit board target.

To test the S80KS HyperRAM on the Alif E8 engineering board, rename
``alif_hex_s80ks.overlay`` to the overlay name expected by the selected DevKit
build target. For example, for the ``e8_ae822 HE`` build, rename it to
``alif_e8_dk_ae822fa0e5597xx0_rtss_he.overlay``.

Also make sure the MPU entry for the OSPI1 XiP region is configured as
read/write with SRAM attributes.

Sample Output
=============

.. code-block:: console


PSRAM XIP mode demo app started
Writing data to the XIP region:
Reading back:
Done, total errors = 0
