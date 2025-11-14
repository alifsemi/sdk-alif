
_spi-flash-test:
##########################################################################################

Overview
********

This directory contains functional tests for the OSPI flash
driver on Alif Semiconductor boards.

Supported Boards:
	``alif_b1_dk/ab1c1f4m51820hh/rtss_he``
	``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
	``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
	``alif_e7_dk/ae722f80f55d5xx/rtss_he``
	``alif_e7_dk/ae722f80f55d5xx/rtss_hp	``

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

code-block:: bash

	west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he ../alif/tests/drivers/spi_flash/basic_tests \
		-DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/spi_flash/basic_tests/boards/alif_b1_dk_rtss_he.overlay

	west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he ../alif/tests/drivers/spi_flash/basic_tests \
		-DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/spi_flash/basic_tests/boards/alif_e8_dk_rtss_he.overlay

note::

	To run the same test on an E7 board, just change the **board** and the
	**overlay** file in the above command. For example:

   .. code-block:: bash

	west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/tests/drivers/spi_flash/basic_tests \
		-DDTC_OVERLAY_FILE=$PWD/../alif/tests/drivers/spi_flash/basic_tests/boards/alif_e7_dk_rtss_he.overlay
