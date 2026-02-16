.. _snippet-ospi-flash:

OSPI Flash Application
######################

Overview
********

This snippet selects appropriate overlay fragment to enable OSPI controller and Connected OSPI-Flash driver based on board dts.

Building and Running
********************

Example command to build:

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he -S ospi-flash ../alif/samples/drivers/spi_flash -p
   OR
   west build -b alif_b1_dk/ab1c1f4m51820hh/rtss_he ../alif/samples/drivers/spi_flash -p -- -DSNIPPET=alif-lfs-mram

The application can be found under :zephyr_file:`samples/drivers/spi_flash` in the Zephyr tree.

Application Ouptut:
*******************
ospi_flash@0 OSPI flash testing
========================================
****Flash Configured Parameters******
* Num Of Sectors : 32768
* Sector Size : 4096
* Page Size : 4096
* Erase value : 255
* Write Blk Size: 2
* Total Size in MB: 128

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
