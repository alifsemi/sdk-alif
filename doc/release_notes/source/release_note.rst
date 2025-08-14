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

- **MHU (Message Handling Unit)**:
  MHUs enable interrupt-driven communication between subsystems. Each MHU pair consists of a Sender in one subsystem and a Receiver in another. The SoC provides 12 MHUs for Secure access and 12 for Non-Secure access.

- **HWSEM (Hardware Semaphore)**:
  HWSEM provides synchronization for shared resources (memory or peripherals) across independent subsystems, preventing race conditions, deadlocks, and abnormal behavior. Supported in the E7 series.

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

- **RTC (Real-Time Counter)**:
  The Low-Power Real-Time Counter (LPRTC) in PD-0 operates in low-power states, supporting a 32-bit counter and interrupt generation.

- **DMA (Direct Memory Access)**:
  Three controllers (DMA0: general-purpose, DMA1: RTSS-HP private, DMA2: RTSS-HE private) offload data transfers, with a MUX for peripheral mapping to DMA0.

- **PWM (Pulse Width Modulation)**:
  Alif UTIMER IP generates up to 24 simultaneous PWM signals across 12 channels.

- **WDT (Watchdog Timer)**:
  Integrates Zephyr’s WDT for fault detection.

- **MRAM**:
  Supports MRAM Read and Write.

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
