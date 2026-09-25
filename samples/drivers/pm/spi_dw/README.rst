.. _spi-dw-pm-sample:

SPI DW Power Management Demo
#############################

Overview
********

This sample demonstrates Zephyr power management states combined with SPI DW
loopback transfers on Alif RTSS cores. The application cycles through PM states
and performs an SPI master/slave loopback after each wake, verifying that the
SPI peripheral resumes correctly.

PM states exercised (determined at runtime by capability predicates):

- **S2RAM path** (TCM or SRAM0 retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  S2RAM STANDBY → S2RAM STOP → idle loop
- **SOFT_OFF path** (MRAM boot, no retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  SOFT_OFF (system resets on wakeup)

.. note::

   Use LPUART port for console logs on E1C and B1 DevKits.

Building and Running
********************

HE Core — TCM boot S2RAM (E7/E8/E1C/B1)
=========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-s2ram-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

HE Core — MRAM boot SOFT_OFF (E7/E8/E1C/B1)
=============================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-mram

HP Core — MRAM boot SOFT_OFF (E7/E8)
======================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-mram

HE Core — SRAM0 S2RAM (E8 only)
================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-s2ram-sram0

HP Core — SRAM0 S2RAM (E8 only)
================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/spi_dw
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: spi-dw-pm-s2ram-sram0

Sample Output (S2RAM path)
**************************

The output below is from an E7 DK HE core TCM-boot run (``APP_PM_WAKEUP_DEBUG 0``,
the default). Setting ``APP_PM_WAKEUP_DEBUG 1`` in ``main.c`` additionally prints
``PM wakeup: NVIC ISPR[x] = 0x...`` lines on each resume.

.. code-block:: console

   *** Booting Zephyr OS build v4.1.0-607-g7fafce9cf3ee ***
   [00:00:00.000,000] <inf> pm_spi_dw: alif_e7_dk (S2RAM): SPI DW PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
   [00:00:00.000,000] <inf> pm_spi_dw: POWER STATE SEQUENCE:
   [00:00:00.000,000] <inf> pm_spi_dw:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.000,000] <inf> pm_spi_dw:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.000,000] <inf> pm_spi_dw:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.000,000] <inf> pm_spi_dw:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.000,000] <inf> pm_spi_dw: [SPI Demo] before RUNTIME_IDLE
   [00:00:00.000,000] <inf> pm_spi_dw: Slave Transceive Iter= 1
   [00:00:00.101,000] <inf> pm_spi_dw: Master Transceive Iter= 1
   [00:00:00.107,000] <inf> pm_spi_dw: Master wrote:   a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
   [00:00:00.107,000] <inf> pm_spi_dw: Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
   [00:00:00.107,000] <inf> pm_spi_dw: SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
   [00:00:00.107,000] <inf> pm_spi_dw: slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
   [00:00:00.107,000] <inf> pm_spi_dw: slave read:  a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
   [00:00:00.107,000] <inf> pm_spi_dw: SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
   ... (iterations 2-10, one per second) ...
   [00:00:09.170,000] <inf> pm_spi_dw: Slave Transfer Successfully Completed
   [00:00:10.171,000] <inf> pm_spi_dw: Master Transfer Successfully Completed
   [00:00:10.171,000] <inf> pm_spi_dw: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:28.172,000] <inf> pm_spi_dw: Exited from RUNTIME_IDLE sleep
   [00:00:28.172,000] <inf> pm_spi_dw: Enter PM_STATE_SUSPEND_TO_IDLE for (10000 microseconds)
   [00:00:28.173,000] <inf> pm_spi_dw: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:28.173,000] <inf> pm_spi_dw: PM wakeup: SUSPEND_TO_IDLE (substate 0)
   [00:00:28.173,000] <inf> pm_spi_dw: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:28.183,000] <inf> pm_spi_dw: Exited from PM_STATE_SUSPEND_TO_IDLE
   [00:00:28.183,000] <inf> pm_spi_dw: [SPI Demo] after SUSPEND_TO_IDLE
   [00:00:28.183,000] <inf> pm_spi_dw: Slave Transceive Iter= 1
   [00:00:28.284,000] <inf> pm_spi_dw: Master Transceive Iter= 1
   [00:00:28.290,000] <inf> pm_spi_dw: Master wrote:   a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
   [00:00:28.290,000] <inf> pm_spi_dw: Master receive: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
   [00:00:28.290,000] <inf> pm_spi_dw: SUCCESS: SPI Master RX & Slave TX DATA IS MATCHING: 0
   [00:00:28.290,000] <inf> pm_spi_dw: slave wrote: 5a5a0000 5a5a0001 5a5a0002 5a5a0003 5a5a0004
   [00:00:28.290,000] <inf> pm_spi_dw: slave read:  a5a50000 a5a50001 a5a50002 a5a50003 a5a50004
   [00:00:28.290,000] <inf> pm_spi_dw: SUCCESS: SPI Master TX & Slave RX DATA IS MATCHING: 0
   ... (iterations 2-10, one per second) ...
   [00:00:37.353,000] <inf> pm_spi_dw: Slave Transfer Successfully Completed
   [00:00:38.354,000] <inf> pm_spi_dw: Master Transfer Successfully Completed
   [00:00:38.354,000] <inf> pm_spi_dw: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (6000000 microseconds)
   [00:00:38.355,000] <inf> pm_spi_dw: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:38.355,000] <inf> pm_spi_dw: PM wakeup: SUSPEND_TO_IDLE (substate 0)
   [00:00:38.355,000] <inf> pm_spi_dw: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:38.452,000] <inf> pm_spi_dw: PM enter: SUSPEND_TO_RAM (substate 0)
   [00:00:38.452,000] <inf> pm_spi_dw: PM wakeup: SUSPEND_TO_RAM (substate 0)
   [00:00:38.452,000] <inf> pm_spi_dw: PM exit:  SUSPEND_TO_RAM (substate 0)
   [00:00:44.405,000] <inf> pm_spi_dw: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   [00:00:44.405,000] <inf> pm_spi_dw: [SPI Demo] after S2RAM STANDBY
   ... (10 transfer iterations, one per second) ...
   [00:00:53.575,000] <inf> pm_spi_dw: Slave Transfer Successfully Completed
   [00:00:54.576,000] <inf> pm_spi_dw: Master Transfer Successfully Completed
   [00:00:54.576,000] <inf> pm_spi_dw: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (9000000 microseconds)
   [00:00:54.577,000] <inf> pm_spi_dw: PM enter: SUSPEND_TO_IDLE (substate 0)
   [00:00:54.577,000] <inf> pm_spi_dw: PM wakeup: SUSPEND_TO_IDLE (substate 0)
   [00:00:54.577,000] <inf> pm_spi_dw: PM exit:  SUSPEND_TO_IDLE (substate 0)
   [00:00:54.674,000] <inf> pm_spi_dw: PM enter: SUSPEND_TO_RAM (substate 1)
   [00:00:54.674,000] <inf> pm_spi_dw: PM wakeup: SUSPEND_TO_RAM (substate 1)
   [00:00:54.674,000] <inf> pm_spi_dw: PM exit:  SUSPEND_TO_RAM (substate 1)
   [00:01:03.622,000] <inf> pm_spi_dw: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   [00:01:03.622,000] <inf> pm_spi_dw: [SPI Demo] after S2RAM STOP
   ... (10 transfer iterations, one per second) ...
   [00:01:12.792,000] <inf> pm_spi_dw: Slave Transfer Successfully Completed
   [00:01:13.793,000] <inf> pm_spi_dw: Master Transfer Successfully Completed
   [00:01:13.793,000] <inf> pm_spi_dw: === SPI DW PM SEQUENCE COMPLETED ===
