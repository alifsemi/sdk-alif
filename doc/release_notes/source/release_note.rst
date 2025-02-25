.. _Release Notes:

Release Notes
=============

Introduction
------------
The **Zephyr Alif SDK (ZAS)** is a comprehensive suite of tools that makes it possible to configure, build, and deploy applications for Alif's microcontrollers.

The Alif DevKit is a development board featuring an Alif multi-core SoC, offering both high-performance and low-power execution.

The **Ensemble DevKit (DK-E7)**  allows you to configure the E7 MCU to operate like other Ensemble MCUs with fewer cores, enabling exploration of the E5, E3, and E1 series devices using a single kit.

The **Ensemble E1C DevKit (DK-E1C)** is designed to explore the Compact series of Ensemble devices.

The **Balletto DevKit (DK-B1)** introduces the Balletto B1 series, a wireless MCU with integrated hardware acceleration for AI/ML workloads. It combines Bluetooth Low Energy 5.3 and 802.15.4 based Thread protocols, an Ethos-U55 microNPU for AI acceleration, and a Cortex-M55 MCU core.

Installing the SDK and Building the Application
-----------------------------------------------

For detailed instructions, please refer to the `ZAS User Guide`_

Host Requirements
-----------------

Hardware Requirements
~~~~~~~~~~~~~~~~~~~~~

- Personal Computer (PC) with an x86-64 based processor
- Minimum 4GB RAM
- At least 256GB of disk space

Software Requirements
~~~~~~~~~~~~~~~~~~~~~

Ubuntu 20.04 64-bit or later

.. note::
   While other Linux distributions may work, they have not been thoroughly tested.

Toolchains
----------

The following toolchains have been tested for the SDK application:

.. list-table::
   :header-rows: 1

   * - Compiler
     - Version
     - Link
   * - GCC (GNU Compiler Collection)
     - v12.2.0
     - `GCC Download`_
   * - ArmCLang
     - v6.18
     - `ArmCLang Download`_
   * - LLVM (Low-Level Virtual Machine)
     - v17.0.1
     - `LLVM Download`_

Software Components
===================

The following are the software components used in the latest release.

+--------------+----------------------------------------+-------------+
| **Component**| **Source**                             | **Version** |
+==============+========================================+=============+
| Zephyr OS    | `Zephyr OS GitHub`_                    | v3.6-branch |
+--------------+----------------------------------------+-------------+

List of Supported Peripheral Devices and Features
-------------------------------------------------

- **UART (Universal Asynchronous Receiver/Transmitter)**:
  Synopsys DW_apb_uart is a programmable Universal Asynchronous Receiver/Transmitter. This AMBA 2.0-compliant Advanced Peripheral Bus (APB) component supports up to 8 ports. DW UART2 and UART4 are enabled in Zephyr for the RTSS-HP and RTSS-HE subsystems, respectively.

- **MHU (Message Handling Unit)**:
  MHUs enable interrupt-driven communication between subsystems. Each MHU pair consists of a Sender in one subsystem and a Receiver in another. The SoC provides 12 MHUs for Secure access and 12 for Non-Secure access.

- **HWSEM (Hardware Semaphore)**:
  HWSEM provides synchronization for shared resources (memory or peripherals) across independent subsystems, preventing race conditions, deadlocks, and abnormal behavior. Supported in the E7 series.

- **CDC-200 (Customizable Display Controller-200)**:
  The TES CDC-200 is a configurable VHDL IP for driving pixel displays, supporting multiple layers and composition (blending).

- **GPIO (General-Purpose Input/Output)**:
  Uncommitted digital signal pins controllable by software, usable as inputs, outputs, or both.

- **LPSPI (Low Power Serial Peripheral Interface)**:
  Synopsys DW_apb_ssi, an AMBA 2.0-compliant component, operates in master mode only on RTSS-HE.

- **SPI (Serial Peripheral Interface)**:
  Synopsys DWC_ssi is a full-duplex, configurable synchronous serial interface supporting 4 instances on RTSS-HE and RTSS-HP. SPI1 is configured as master; SPI0, SPI2, and SPI3 are slaves.

- **LPI2C (Low Power Inter-Integrated Circuit)**:
  A power-efficient controller in the Ensemble series for communication with peripherals in low-power applications.

- **I2C (Inter-Integrated Circuit)**:
  DW_apb_i2c supports master or slave mode with two enabled instances (i2c0 and i2c1) on RTSS-HE and RTSS-HP.

- **LPI2S (Low Power Inter-IC Sound)**:
  DW_apb_lpi2s processes digital audio signals on M55 HE.

- **I2S (Inter-IC Sound)**:
  DW_apb_i2s supports four instances for digital audio processing; I2S3_b connects internally to a microphone.

- **MIPI-DSI**:
  The DesignWare MIPI DSI Host Controller interfaces with DSI-compliant displays via the MIPI D-PHY layer.

- **MIPI-CSI2**:
  Uses MIPI D-PHY to transmit captured images to the SoC in individual frames.

- **RTC (Real-Time Counter)**:
  The Low-Power Real-Time Counter (LPRTC) in PD-0 operates in low-power states, supporting a 32-bit counter and interrupt generation.

- **DMA (Direct Memory Access)**:
  Three controllers (DMA0: general-purpose, DMA1: RTSS-HP private, DMA2: RTSS-HE private) offload data transfers, with a MUX for peripheral mapping to DMA0.

- **PWM (Pulse Width Modulation)**:
  Alif UTIMER IP generates up to 24 simultaneous PWM signals across 12 channels.

- **LPPDM (Low Power Pulse Density Modulation)**:
  Supports up to eight PDM microphones, converting 1-bit PDM to 16-bit PCM audio.

- **PDM (Pulse Density Modulation)**:
  Enhances audio with support for eight PDM microphones, converting 1-bit PDM to 16-bit PCM.

- **CRC (Cyclic Redundancy Check)**:
  Supports CRC-8-CCITT, CRC-16-CCITT, CRC-32, and CRC-32C with flexible data processing via AHB.

- **WDT (Watchdog Timer)**:
  Integrates Zephyr’s WDT for fault detection.

- **OSPI Flash (Octal SPI Flash)**:
  The Alif DevKit-E7 includes a 32MB ISSI Flash (IS25WX256) with Zephyr flash APIs for erase, read, and write operations.

- **AES (Advanced Encryption Standard)**:
  Enables on-the-fly decryption of XIP data from external memory.

- **ADC (Analog-to-Digital Converter)**:
  Features ADC12 (12-bit, 8 channels) and ADC24 (24-bit, 4 differential channels) for analog-to-digital conversion.

- **LPTimer (Low-Power Timer)**:
  A 32-bit timer in the M55 core for precise low-power timing.

- **Parallel Camera**:
  Supports frame capture via LPCAM/CAM with sensors like MT9M114.

- **AiPM (Advanced Intelligent Power Management)**:
  Optimizes power modes (STOP, OFF) with autonomous transitions and wake-up sources.

- **Ethos U55**:
  Pairs with Cortex-M55 for AI/ML acceleration using Arm v8.1 and Helium MVE.

- **MCU-BOOT**:
  A secure bootloader for firmware upgrades, leveraging Zephyr’s HAL.

- **BLE (Bluetooth Low Energy)**:
  Supported in Balletto B1 with a host stack in ROM.

- **LC3 (Low Complexity Communication Codec)**:
  In Balletto B1 ROM for BLE isochronous audio.

- **CANFD (Controller Area Network Flexible Data-Rate)**:
  Supports ECU communication with error detection and higher data rates.

- **Touch Screen**:
  GT911 touch screen supports 5-point touch via I2C for 7"-8" displays.

- **I3C (Improved Inter-Integrated Circuit)**:
  A next-gen interface with dynamic addressing and multi-master support.

Known Issues
------------

1. The Zephyr CDC200 driver supports only ARGB8888, RGB888, and RGB565 formats (subset of CDC200 IP capabilities).
2. Demo application restricts Layer 2 to ARGB8888; Layer 1 formats are configurable.
3. Building from DTCM fails with open-source Clang (LLVM).
4. Ethos-U application untested with ArmClang and Clang.
5. Ethos-U lacks MRAM/ITCM support; runs from SRAM0 (0x0200 0000).
6. I2S applications on RTSS-HE/HP run from SRAM0/DTCM in non-XIP mode.
7. I2S compilation fails with Clang.
8. Camera:
   - Non-standard video buffer allocations to SRAM1.
   - RGB format support pending.
   - CMOS/CSI configured for RAW10, but Camera controller uses RAW8; RAW8 rework needed.
   - Untested with LLVM.
9. No LPCMP sample application.

Peripheral Device Issues
------------------------

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Alif-ID
     - Description
   * - PSBT 189
     - UART driver cannot configure odd, mark, or space parity.
   * - PSBT 465
     - Warnings during driver builds with LLVM in Zephyr v3.3.
   * - PSBT 876
     - SPI data mismatch across test cases on Ensemble-E7.
   * - PSBT 880
     - UART data corruption at 2500000 bps on Ensemble-E7.
   * - PSBT 881
     - Touchscreen fails when booting from OSPI on Ensemble-E7.
   * - PSBT 882
     - Touchscreen driver lacks multi-touch support on Ensemble-E7.
   * - PSBT 891
     - Console prints limited to one core when booting from TCM on Ensemble-E7.
   * - PSBT 894
     - MCUboot fails to find a bootable image on Ensemble-E7.
   * - PSBT 898
     - Warnings with GCC, ArmCLang, or LLVM on Ensemble-E7.
   * - PSBT 900
     - Warnings in entropy app with ArmClang/LLVM during MRAM booting on Ensemble-E7.
   * - PSBT 901
     - CMake warning with ArmClang on Ensemble-E7.

External References
-------------------

- ZAS User Guide `ZAS User Guide`_

Copyright/Trademark
-------------------

The Alif logo is a trademark of Alif Semiconductor. please refer to `Alif Trademarks`_.
Arm, Cortex, CoreSight, and Ethos are trademarks of Arm Limited (or its subsidiaries).
Zephyr is an open-source RTOS under the Apache License 2.0, maintained by the Zephyr Project <https://www.zephyrproject.org/>.
The Zephyr logo is a trademark of The Linux Foundation, subject to its Trademark Usage Guidelines <https://www.linuxfoundation.org/trademark-usage/>.
All other names are property of their respective owners.