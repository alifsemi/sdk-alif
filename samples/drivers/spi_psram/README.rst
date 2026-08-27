.. _psram-test:

OSPI PSRAM Test
###############

Overview
********

This sample verifies RAM connected through the Alif DesignWare MSPI controller
in memory-mapped mode. The test writes an incrementing pattern directly to the
mapped address range and reads it back for verification.

The sample supports:

* ISSI IS66/67WVH HyperRAM devices using ``issi,is66wvh``.

E8 boards use an APS512XXN AP Memory PSRAM device rather than IS66WVH
HyperRAM. The existing E8 overlay is retained, but E8 support is not enabled
by the ``ospi-psram`` snippet until the AP Memory driver is migrated to MSPI.

On E1, E3, E5, and E7 series devices, the OSPI RAM path uses 16-bit data
transactions. The sample writes and verifies ``uint16_t`` data and runs multiple
MPU attribute readback cases:

* Device
* Normal non-cacheable
* SRAM cacheable

On other supported series, the sample uses 32-bit data and performs a direct
write/read verification without changing MPU attributes at runtime.

Building and Running
********************

Use the ``ospi-psram`` snippet to select the board-specific OSPI PSRAM overlay:

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he -S ospi-psram ../alif/samples/drivers/spi_psram -p

The same snippet can also be passed through CMake:

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/samples/drivers/spi_psram -p -- -DSNIPPET=ospi-psram

The snippet maps supported board families to the matching OSPI PSRAM overlay:

* B1 boards: OSPI0 ISSI HyperRAM overlay.
* E3, E5, and E7 boards: OSPI0 ISSI HyperRAM overlay.

Sample Output
*************

.. code-block:: console

   PSRAM XIP mode demo app started
   Test configuration:
     XIP base        : 0xa0000000
     RAM size        : 67108864 bytes
     Access width    : 16-bit
     Pattern         : incrementing 16-bit value, wraps at data width
     MPU test cases  : Device, Normal non-cacheable, SRAM cacheable

   Test case: Device
     Write attribute : Device
     Read attribute  : Device
   Writing data to the XIP region:
   Reading back in Device mode:
   Device total errors = 0

   Test case: Normal non-cacheable
     Write attribute : Device
     Read attribute  : Normal non-cacheable
   Reading back in Normal non-cacheable mode:
   Normal non-cacheable total errors = 0

   Test case: SRAM cacheable
     Write attribute : Device
     Read attribute  : SRAM cacheable
   Reading back in SRAM cacheable mode:
   SRAM cacheable total errors = 0
   Done
