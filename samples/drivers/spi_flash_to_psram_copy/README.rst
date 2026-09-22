
.. _spi-flash-to-psram-copy-test:

SPI-Flash-to-PSRAM Copy Example
###############################

Overview
********

This is the test application to test and verify the copying of data from the OSPI flash to the PSRAM XiP window.
The test also verifies the read operations on the OSPI flash and PSRAM write operations.
As Alif B1 DK board supports both OSPI flash and PSRAM but has only one OSPI controller,
the test ensures proper switching between the flash and PSRAM devices.
At the end of the test, bus is left in the PSRAM XiP mode after the copy operation.

Tested on the Alif B1 DK board with the OSPI flash and PSRAM devices.

Layout produced by test.bin: 8-byte magic, offset-encoded words, CRC32 tail.
#define TEST_MAGIC		"ALIFPSRM"
#define TEST_MAGIC_LEN		8U

Building and Running
********************

Generating the test binary. In this example, we create a 4MB test binary.
You can use a different size by changing the --size argument. For this example we wanted to use
a size bigger than MRAM:

.. code-block:: console

	python3 gen_test_bin.py --size 4M
	ls -l test.bin

Copy the test.bin to the OSPI flash.
You can use for example https://github.com/alifsemi/alif_usb-to-ospi-flasher to program the OSPI flash with the test binary.

Example command to build in sdk root directory:

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he ./alif/samples/drivers/spi_flash_to_psram_copy -p

Or for the Alif E1C DK board:

.. code-block:: console

   west build -b alif_e1c_dk/ae1c1f4051920hh/rtss_he ./alif/samples/drivers/spi_flash_to_psram_copy -p

Flash the test application:

.. code-block:: console

   west flash

Sample Output
=============

.. code-block:: console

	main: start
	copy: 4194304 bytes flash@0x0 -> psram@0x0 (chunk 8192B)
	copy: done (4194304 bytes, bus left in PSRAM XiP mode)
	verify: magic OK (ALIFPSRM)
	verify: offset-word pattern OK (0x8..0x3ffff8)
	verify: crc32 OK (calc 0xafb7a905, stored 0xafb7a905)
	verify: PASS -- 4194304 bytes copied to PSRAM XiP @0xa0000000
