.. _i2c-dw-pm-sample:

I2C DW Power Management Demo
############################

Overview
********

This sample demonstrates Zephyr power management states combined with I2C DW
loopback transfers on Alif RTSS cores. **I2C0 is master** and **I2C1 is slave**.
The application cycles through PM states and performs one write plus one read
after each wake, verifying that the I2C peripheral resumes correctly.

PM states exercised (determined at runtime by capability predicates):

- **S2RAM path** (TCM or SRAM0 retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  S2RAM STANDBY → S2RAM STOP → idle loop
- **SOFT_OFF path** (MRAM boot, no retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  SOFT_OFF (system resets on wakeup)

Connect I2C0 SCL/SDA to I2C1 SCL/SDA (and a common GND) for the on-board
loopback.

.. note::

   B1 and E1C use UART2 for console logs.

Building and Running
********************

HE Core — TCM boot S2RAM (E7/E8/E1C/B1)
=======================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/i2c_dw
   :board: alif_b1_dk/ab1c1f4m51820ph0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: i2c-dw-pm-s2ram-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0

HE Core — MRAM boot SOFT_OFF (E7/E8/E1C/B1)
===========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/i2c_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: i2c-dw-pm-mram

HP Core — MRAM boot SOFT_OFF (E7/E8)
====================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/i2c_dw
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: i2c-dw-pm-mram

HE Core — SRAM0 S2RAM (E8 only)
===============================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/i2c_dw
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: i2c-dw-pm-s2ram-sram0

HP Core — SRAM0 S2RAM (E8 only)
===============================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/i2c_dw
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: i2c-dw-pm-s2ram-sram0
