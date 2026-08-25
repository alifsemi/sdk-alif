OSPI Flash Test Suite
#####################

Overview
********

This test application verifies OSPI flash ``flash_read`` / ``flash_write`` /
``flash_erase`` plus user-configurable DT and HAL APIs on the ISSI IS25WX path.

The suite lives at::

    tests/drivers/flash/spi_flash/

Test sources do **not** read or write OSPI or AES registers. They use only:

* Zephyr flash APIs (``flash_read``, ``flash_write``, ``flash_erase``,
  ``flash_get_parameters``, ``flash_get_page_info_by_offs``)
* HAL handle APIs (``alif_hal_ospi_xip_enable``, ``alif_hal_ospi_xip_disable``,
  ``alif_hal_ospi_prepare_transfer``)
* User-configurable macros (DT properties, ``ISSI_XIP_*`` opcodes, Kconfig)

If a required driver or HAL API is missing or wrong, the case **fails**. That
is the signal for the driver developer. Cases that cannot be expressed without
register access were removed (see `Removed / not supported`_).

Test Structure
**************

* ``test_ospi_main.c`` - Basic erase / write / read / patterns
* ``test_ospi_boundary_tests.c`` - Boundary and cross-page
* ``test_ospi_negative_tests.c`` - Invalid parameters and unaligned access
* ``test_ospi_ctrl_tests.c`` - Controller DT/overlay values plus flash API
* ``test_ospi_xip_reg_tests.c`` - XIP window DT, HAL enable/disable, mapped vs ``flash_read``
* ``test_ospi_xip_tests.c`` - Sample-style mapped XIP (``CONFIG_ALIF_OSPI_FLASH_XIP=y`` only)
* ``test_ospi_perf_tests.c`` - Throughput (``CONFIG_TEST_OSPI_FLASH_PERF=y``)
* ``test_ospi_flash_dev_tests.c`` - Detection, geometry, program paths (``CONFIG_TEST_OSPI_FLASH_EXTRA=y``)
* ``test_ospi_irq_tests.c`` - IRQ routing from DT / NVIC (``CONFIG_TEST_OSPI_FLASH_EXTRA=y``)
* ``test_ospi_xip_transition_tests.c`` - HAL XIP enter/exit and mapped compares (``CONFIG_TEST_OSPI_FLASH_EXTRA=y``)
* ``test_ospi_flash_test.h`` - Shared offsets, node alias, helpers

Reserved flash regions
**********************

* ``SPI_FLASH_TEST_REGION_OFFSET`` - ``0x8000`` (32 KB)
* Destructive extra work uses sector 12 at ``0xC000``
* Do **not** erase offset ``0`` (possible boot image). The only case that
  touches offset 0 is the full-chip perf test, which is compiled out unless
  ``CONFIG_TEST_OSPI_FLASH_FULL_CHIP=y`` is set explicitly (see below).

Supported Boards
****************

* ``alif_b1_dk/ab1c1f4m51820ph0/rtss_he``
* ``alif_e1c_dk/ae1c1f4051920hh/rtss_he``
* ``alif_e8_dk/ae822fa0e5597xx0/rtss_he``
* ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp``
* ``alif_e7_dk/ae722f80f55d5xx/rtss_he``
* ``alif_e7_dk/ae722f80f55d5xx/rtss_hp``

OSPI flash is on **OSPI1** for E7/E8. OSPI0 on E8/B1/E1C is HyperRAM/PSRAM, not
this flash part. E7 has no ``ospi0`` node.

Building and Running
********************

The application builds only for targets with a ``snps,designware-ospi`` parent
and an ISSI flash node (snippet ``-S ospi-flash``).

.. code-block:: console

   west build -p always -b <board>\
     tests/drivers/flash/spi_flash/ -S ospi-flash

Extra API suites (flash device, IRQ routing, XIP transitions)::

   west build -p always -b <board> \
     tests/drivers/flash/spi_flash/ -S ospi-flash \
     -- -DCONFIG_TEST_OSPI_FLASH_EXTRA=y

Mapped window reads need ``CONFIG_TEST_OSPI_XIP_LIVE=y`` (default when the extra
suites are on). A bad XIP descriptor is a bus fault, so on an unproven board::

   west build -p always -b <board> \
     tests/drivers/flash/spi_flash/ -S ospi-flash \
     -- -DCONFIG_TEST_OSPI_FLASH_EXTRA=y -DCONFIG_TEST_OSPI_XIP_LIVE=n

Driver-owned XIP image (replaces most suites with ``test_ospi_xip`` +
``test_ospi_xipreg``)::

   west build -p always -b <board> \
     tests/drivers/flash/spi_flash/ -S ospi-flash \
     -- -DCONFIG_ALIF_OSPI_FLASH_XIP=y

Frequency / timing overlays in ``boards/``:

* ``ospi1_50Mhz.overlay``, ``ospi1_100Mhz.overlay``, ``ospi1_200Mhz.overlay``,
* ``ospi0_80Mhz.overlay``

Configuration Flags
*******************

* ``-DCONFIG_TEST_OSPI_FLASH_PERF=y`` - Bounded performance suite (never erases offset 0)
* ``-DCONFIG_TEST_OSPI_FLASH_FULL_CHIP=y`` - Add the destructive full-chip
  erase/write/read case to the perf suite. Wipes offset 0 and takes a long
  time (64 MiB on E7, 128 MiB on E8). Requires ``CONFIG_TEST_OSPI_FLASH_PERF=y``.
  Never enable on a board that boots from this flash.
* ``-DCONFIG_ALIF_OSPI_FLASH_XIP=y`` - Driver enables XIP at init
* ``-DCONFIG_TEST_OSPI_FLASH_EXTRA=y`` - Extra flash-device, IRQ, and XIP-transition suites
* ``-DCONFIG_TEST_OSPI_XIP_LIVE=y`` - Dereference the memory-mapped XIP window

Running Tests
*************

.. code-block:: console

   west build -t run

   west build -t run -- -s test_ospi_flash
   west build -t run -- -s test_ospi_perf
   west build -t run -- -s test_ospi_xip
   west build -t run -- -s test_ospi_boundary
   west build -t run -- -s test_ospi_negative
   west build -t run -- -s test_ospi_ctrl
   west build -t run -- -s test_ospi_xipreg
   west build -t run -- -s test_ospi_xiptrans
   west build -t run -- -s test_ospi_flashdev
   west build -t run -- -s test_ospi_irq

Removed / not supported
***********************

These cases cannot be expressed with public flash/HAL APIs or user DT macros.
They were **deleted** rather than skipped. Register-level coverage belongs in
the driver, not this suite.

Controller field sweeps (need CTRLR0 / SPI_CTRLR0 / SSIENR writes)
==================================================================

* ``OSPI_BusSpeed_ZeroDisable`` (BAUDR = 0)
* ``OSPI_DataBits_MultipleConstraint`` (tautology on write-block-size)
* ``OSPI_FrameFormat_Config`` (CTRLR0.SPI_FRF)
* ``OSPI_HyperBus_FrameFormat_Enable`` (CTRLR0.SPI_HYPERBUS_EN)
* ``OSPI_AddrLength_Config`` / ``OSPI_InstLength_Config``
* ``OSPI_InstAddr_BothZero_Illegal``
* ``OSPI_TransType_Config``
* ``OSPI_SSTE_ContinuousMode_Config``
* ``OSPI_TMOD_Config``
* ``OSPI_SCPOL_SCPH_Config``
* ``OSPI_ModeBits_Config``
* ``OSPI_RXSampleDelay_OverflowClamp`` (OSPI RX_SAMPLE_DELAY, not DT ``rx-ds-delay``)
* ``OSPI_Enable_Disable`` (SSIENR)
* ``OSPI_Write_Lockout_WhileEnabled``
* ``OSPI_Abort_Transfer``

IRQ / AES status fields (need IMR / RISR / ICR / AES_INTR)
==========================================================

* ``OSPI_TXFIFO_Threshold_IRQ``
* ``OSPI_RXFIFO_Threshold_IRQ``
* ``OSPI_TXFTLR_StartLevel``
* ``OSPI_IRQ_Mask_Individual``
* ``OSPI_IRQ_CombinedLine_ORing``
* ``OSPI_IRQ_ReadToClear_TXEICR``
* ``OSPI_IRQ_ReadToClear_RXOICR_RXUICR``
* ``OSPI_IRQ_CombinedClear_ICR``
* ``AES_IRQ_SPI_ErrResp``
* ``AES_IRQ_RegErrResp``

XIP on-to-off, XIP-off program, and other duplicates
====================================================

XIP-on write / XIP-off read is not a supported product path. Indirect erase
and write are already covered by ``test_ospi_main.c``. These were removed:

* ``OSPI_XIP_OFF_IndirectWrite`` / ``OSPI_XIP_OFF_Erase``
* ``OSPI_XIP_ON_to_OFF_Read`` / ``Write`` / ``Erase`` / ``ContinuousTransfer``
* ``OSPI_XIP_ON_OFF_ON``
* ``OSPI_XIP_StateMachine_Transition`` / ``Init_Reentrancy`` /
  ``ActiveTransfer_Transition`` / ``LongDuration_Transition``
* ``OSPI_XIP_OFF_to_ON_Read`` (same compare as memory-mapped / data-integrity)
* ``XIP_OFF_ProgrammingInterface_FullAccess``
* ``XIP_OFF_IndirectMode_IsDefaultPath`` / ``XIP_Disabled_DefaultState_Reset``
* ``XIP_EnableDisable_Transition_Clean`` / ``XIP_OFF_to_ON_StateIsolation``
* ``OSPI_XIP_OFF_AHB_Access`` (window DT already checked in mapped-read)
* Suite-end ``flash_still_healthy`` cases in the XIP files (``flash_read``
  is already proven in ``test_ospi_main.c``)

XIP register-bank and PSRAM cases
=================================

* PSRAM / HyperRAM transition cases (no PSRAM on the E7 OSPI flash instance)
* ``OSPI_XIP_InvalidConfiguration`` (writes a bad XIP_CTRL)
* ``OSPI_XIP_FIFO_State_Transition``
* ``OSPI_XIP_Reset_StateRecovery``
* ``OSPI_XIP_ContinuousTransfer_Entry`` (XIP_CTRL.CONT_XFER_EN + SSTE)
* ``OSPI_XIP_IdleTimeout_Watchdog``
* ``OSPI_XIP_Write_Model`` / ``OSPI_XIP_Write_Precondition_Enforcement``
* ``OSPI_XIP_HardcodedDFS_Write``
* ``OSPI_XIP_DataMask_OctalDDRWrite``
* ``OSPI_XIP_ConfigBank_Isolation``
* ``OSPI_XIP_WriteAddrLength_EncodingDiff``
* ``OSPI_XIP_OFF_NoSPITrigger`` / ``OSPI_XIP_OFF_Interrupt`` / ``OSPI_XIP_OFF_DMA``

Entire leftover register suites (files removed)
===============================================

* ``test_ospi_fifo_dma_tests.c``
* ``test_ospi_aes_tests.c``
* ``test_ospi_power_tests.c``
* ``test_ospi_platform_tests.c``
* ``test_ospi_rxds_tests.c``
* ``test_ospi_hyperbus_tests.c``
* ``test_ospi_reg_common.c`` / ``test_ospi_reg_common.h``

Driver notes that affect tests
******************************

* ``ISSI_Flags``: init sets ``FLASH_POWER`` only. ``FLASH_INIT`` is defined in
  ``flash_ospi_is25wx.h`` but is never written. Tests assert ``FLASH_POWER``.
* ``xip-base-address`` is the SoC window (for example ``0xC0000000`` on E7).
  The driver programs XIP opcodes and wait cycles at init; it enables XIP
  only when ``CONFIG_ALIF_OSPI_FLASH_XIP=y``. Extra suites call
  ``alif_hal_ospi_xip_enable()`` themselves.
* ``west flash`` / ``alif_flash`` programs MRAM, not OSPI.
