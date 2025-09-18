.. _Release Notes:

Release Notes
=============

Introduction
------------
The **Zephyr Alif SDK (ZAS)** is a comprehensive suite of tools that makes it possible to configure, build, and deploy applications for Alif's microcontrollers.

The Alif DevKit is a development board featuring an Alif multi-core SoC, offering both high-performance and low-power execution.

The **Ensemble DevKit (DK-E7)**  allows you to configure the E7 MCU to operate like other Ensemble MCUs with fewer cores, enabling exploration of the E5, E3, and E1 series devices using a single kit.

The **Ensemble DevKit (DK-E8)**  allows you to configure the E8 MCU to operate like other Ensemble MCUs with fewer cores, enabling exploration of the E6, and E4 series devices using a single kit.

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

Software Components
-------------------

The following are the software components used in the latest release.

+--------------+----------------------------------------+-------------+
| **Component**| **Source**                             | **Version** |
+==============+========================================+=============+
| Zephyr OS    | `Zephyr OS GitHub`_                    | v4.1-branch |
+--------------+----------------------------------------+-------------+

List of Supported Peripheral Devices and Features
-------------------------------------------------

- **UART (Universal Asynchronous Receiver/Transmitter)**:
  Synopsys DW_apb_uart is a programmable Universal Asynchronous Receiver/Transmitter. This AMBA 2.0-compliant Advanced Peripheral Bus (APB) component supports up to 8 ports. DW UART2 and UART4 are enabled in Zephyr for the RTSS-HP and RTSS-HE subsystems, respectively.

- **SPI (Serial Peripheral Interface)**:
  Synopsys DWC_ssi is a full-duplex, configurable synchronous serial interface supporting 4 instances on RTSS-HE and RTSS-HP. SPI1 is configured as master; SPI0, SPI2, and SPI3 are slaves.

- **LPI2C (Low Power Inter-Integrated Circuit)**:
  A power-efficient controller in the Ensemble series for communication with peripherals in low-power applications.

- **I2C (Inter-Integrated Circuit)**:
  DW_apb_i2c supports master or slave mode with two enabled instances (i2c0 and i2c1) on RTSS-HE and RTSS-HP.

- **I2S (Inter-IC Sound)**:
  DW_apb_i2s supports four instances for digital audio processing; I2S3_b connects internally to a microphone.

- **RTC (Real-Time Counter)**:
  The Low-Power Real-Time Counter (LPRTC) in PD-0 operates in low-power states, supporting a 32-bit counter and interrupt generation.

- **DMA (Direct Memory Access)**:
  Three controllers (DMA0: general-purpose, DMA1: RTSS-HP private, DMA2: RTSS-HE private) offload data transfers, with a MUX for peripheral mapping to DMA0.

- **PWM (Pulse Width Modulation)**:
  Alif UTIMER IP generates up to 24 simultaneous PWM signals across 12 channels.

- **Quadrature Decoder (QDEC)**:
  Alif UTIMER IP on Ensemble Devkit supports QDEC mode for precise rotary encoder position tracking.

- **UTimer Counter**:
  Alif UTIMER IP on Ensemble Devkit supports counter mode for precise event or clock pulse counting, enabling frequency measurement, event counting, and timer-based scheduling.

- **PDM (Pulse Density Modulation)**:
  Enhances audio with support for eight PDM microphones, converting 1-bit PDM to 16-bit PCM.

- **CRC (Cyclic Redundancy Check)**:
  Supports CRC-8-CCITT, CRC-16-CCITT, CRC-32, and CRC-32C with flexible data processing via AHB.

- **OSPI Flash (Octal SPI Flash)**:
  The Alif DevKit-E7 includes a 32MB ISSI Flash (IS25WX256) with Zephyr flash APIs for erase, read, and write operations.

- **ADC (Analog-to-Digital Converter)**:
  Features ADC12 (12-bit, 8 channels) and ADC24 (24-bit, 4 differential channels) for analog-to-digital conversion.

- **LPTimer (Low-Power Timer)**:
  A 32-bit timer in the M55 core for precise low-power timing.

- **SDMMC (Secure Digital Multimedia Card)**:
  Alif SDMMC driver supports eMMC/SD interfaces.

- **DAC 12 (Digital to Analog Converter)**:
  Features a DAC12 module that converts 12-bit digital values to analog voltages, with a 0 V to 1.8 V output range in Low-Power mode.

Known Issues
------------


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
