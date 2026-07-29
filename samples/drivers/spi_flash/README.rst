
.. _spi-flash-test:

SPI-Flash Test
###############

Overview
********

This application verifies flash read, write, and erase operations through the
MSPI/OSPI interface. The flash driver is selected by the compatible in the
``ospi_flash`` devicetree node.

Currently supported Flash device is ISSI-IS25WX Flash.

Test modes
**********

The application selects its tests using ``CONFIG_MSPI_XIP``.

Indirect command mode
=====================

Use the following configuration to run the flash API read, write, sector
erase, and full erase tests:

.. code-block:: cfg

   CONFIG_MSPI_XIP=n
   CONFIG_FLASH_MSPI_XIP_READ=n

The flash ``xip-config`` may still describe the available memory-mapped
window, but its enable field must be zero:

.. code-block:: devicetree

   /* E7: 64-MB read-only XiP window, disabled */
   xip-config = <0 0x0 0x04000000 1>;

   /* E8: 128-MB read-only XiP window, disabled */
   xip-config = <0 0x0 0x08000000 1>;

XiP mode
========

Enable MSPI XiP and set the first ``xip-config`` cell to one:

.. code-block:: cfg

   CONFIG_MSPI_XIP=y

.. code-block:: devicetree

   /* E7: 64-MB read-only XiP window, enabled */
   xip-config = <1 0x0 0x04000000 1>;

   /* E8: 128-MB read-only XiP window, enabled */
   xip-config = <1 0x0 0x08000000 1>;

In XiP mode, this sample performs only a 16-byte memory-mapped read and dumps
the data. Set ``CONFIG_FLASH_MSPI_XIP_READ=y`` only when ``flash_read()`` must
also use the memory-mapped XiP window.

The ``xip-config`` cells are ``<enable address-offset size permission>``.
Permission value ``1`` selects read-only access. Memory-mapped writes are not
used; flash programming and erase operations must use the flash APIs in
indirect command mode.

E7 and E1C implement the XIP slave-enable register and their OSPI controller
nodes must contain:

.. code-block:: devicetree

   xip-ser-support;

E8 and B1 do not implement this register and must omit the property.

Use a pristine build when switching between XiP and indirect configurations
so that generated Kconfig and devicetree files are regenerated.


Building and Running
********************

Example command to build:

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -p
   OR
   west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he ../alif/samples/drivers/spi_flash -p -- -DSNIPPET=ospi-flash

Sample Output
=============

Indirect command-mode output:

.. code-block:: console

	ospi1@83002000 OSPI flash testing
	========================================
	Running indirect command-mode test cases

	Test 1: Flash erase
	Flash erase succeeded!

	Test 1: Flash write
	Attempting to write 4 bytes

	Test 1: Flash read
	Data read matches data written. Good!!

	Test 2: Flash Full Erase
	Successfully Erased whole Flash Memory
	Total errors after reading erased chip = 0

	Test 3: Flash erase
	Flash erase succeeded!

	Test 3: Flash write
	Attempting to write 1024 bytes

	Test 3: Flash read
	Data read matches data written. Good!!

	Test 4: write sector 16384
	Test 4: write sector 20480

	Sec4: Read and Verify written data

	Test 4: read sector 16384

	Data read matches data written. Good!!
	Sec5: Read and Verify written data

	Test 4: read sector 20480
	Data read matches data written. Good!!

	Test 4: Erase Sector 4 and 5
	Flash Erase from Sector 16384 Size to Erase 8192

	Multi-Sector erase succeeded!

	Test 4: read sector 16384
	Total errors after reading erased Sector 4 = 0

	Test 4: read sector 20480
	Total errors after reading erased Sector 5 = 0

	Multi-Sector Erase Test Succeeded !

XiP output:

.. code-block:: console

	ospi_flash@0 OSPI flash testing
	========================================
	Running XiP mode test

	XiP Read Test
	XiP data at 0xc0000000:
	00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f

