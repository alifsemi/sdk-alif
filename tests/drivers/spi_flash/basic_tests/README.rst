.._spi-flash-test:
##########################################################################################

SPI-Flash Test
###############

Overview
********

This directory contains functional tests for the OSPI flash driver on Alif Semiconductor boards.

Supported Boards:
	``alif_b1_dk/ab1c1f4m51820hh/rtss_he``
	``alif_e1c_dk/ae1c1f4051920hh/rtss_he``
	``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
	``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
	``alif_e7_dk/ae722f80f55d5xx/rtss_he``
	``alif_e7_dk/ae722f80f55d5xx/rtss_hp``
	``alif_e4_dk/ae402fa0e5597xx0/rtss_he``
	``alif_e4_dk/ae402fa0e5597xx0/rtss_hp``
	``alif_e3_dk/ae302f80f55d5xx/rtss_he``
	``alif_e3_dk/ae302f80f55d5xx/rtss_hp``

Building and Running
********************

The application will build only for a target that has a devicetree entry with
*:dt compatible:`snps,designware-ospi`*
as a compatible.
*The tests assume that the OSPI flash node (``ospi0``)  is available as ``&ospi_flash``
	and aliased accordingly through a board overlay.

How to Run
----------

**Example build commands**
.. code-block:: console

west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/spi_flash/basic_tests \
-- -DSNIPPET=alif-ospi-flash

XIP mode testcases
------------------

To run the OSPI flash tests in XIP mode, enable the following Kconfig option
in the test configuration:

.. code-block:: none

	CONFIG_TEST_XIP_MODE=y

