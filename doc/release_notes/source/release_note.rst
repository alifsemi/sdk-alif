Introduction
============

The **Zephyr Alif SDK (ZAS)** is a comprehensive suite of tools that makes it possible to configure, build, and deploy applications for Alif's microcontrollers.

The Alif DevKit is a development board featuring an Alif multi-core SoC, offering both high-performance and low-power execution.

The **Ensemble DevKit (DK-E7)**  allows you to configure the E7 MCU to operate like other Ensemble MCUs with fewer cores, enabling exploration of the E5, E3, and E1 series devices using a single kit.

The **Ensemble E1C DevKit (DK-E1C)** is designed to explore the Compact series of Ensemble devices.

The **Balletto DevKit (DK-B1)** introduces the Balletto B1 series, a wireless MCU with integrated hardware acceleration for AI/ML workloads. It combines Bluetooth Low Energy 5.3 and 802.15.4 based Thread protocols, an Ethos-U55 microNPU for AI acceleration, and a Cortex-M55 MCU core.


Installing the SDK and Building the Application
===============================================

For detailed instructions, please refer to the **Getting Started User Guide**.

Host Requirements
=================

Hardware Requirements
~~~~~~~~~~~~~~~~~~~~~
- Personal Computer (PC) with an x86-64 based processor.
- Minimum 4GB RAM.
- At least 256GB of disk space.

Software Requirements
~~~~~~~~~~~~~~~~~~~~~
- **Ubuntu 20.04 64-bit** or Later

*Note: While different flavors of Linux distributions may also work, they have not been thoroughly tested.*

Toolchains
~~~~~~~~~~
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
=================================================

- **UART (Universal Asynchronous Receiver/Transmitter)**:
  Synopsys DW_apb_uart is a programmable Universal Asynchronous Receiver/Transmitter. This component is an AMBA 2.0-compliant Advanced Peripheral Bus (APB) supporting up to 8 ports. DW UART2 and UART4 are enabled in Zephyr for the RTSS-HP and RTSS-HE respectively.

- **MHU (Message Handling Unit)**:
  The MHUs facilitate interrupt-driven communication between two subsystems. Communication between two subsystems is made possible with one subsystem having the Sender and the other subsystem having the Receiver of a single MHU. There are 12 MHUs for Secure and 12 MHUs for Non-Secure access.

- **HWSEM (Hardware Semaphore)**:
  A need for synchronization while accessing a shared resource (memory or peripheral) by two or more independent processing subsystems is fulfilled by the HWSEM. By accessing shared resources with synchronization, the possibility of race conditions, deadlock, and abnormal behavior among independent processing subsystems can be avoided.

  Hardware Semaphore is supported in the E7 Series.

- **CDC-200 (Customizable Display Controller-200)**:
  The CDC from TES is a fully configurable VHDL IP to drive a pixel display. It can implement multiple layers and provides composition (blending) support.

- **GPIO (General-purpose input/output)**:
  GPIO is an uncommitted digital signal pin on an integrated circuit or electronic circuit (e.g., MCUs/MPUs) board which may be used as an input or output, or both, and is controllable by software.

- **LPSPI (Low Power Serial Peripheral Interface)**:
  The Synopsys DW_apb_ssi is a component of the Design Ware Advanced Peripheral Bus (DW_apb) and conforms to the AMBA Specification, Revision 2.0 from Arm®. DW LPSPI is available only on RTSS-HE. It works only in master mode.

- **SPI (Serial Peripheral Interface)**:
  The Synopsys Design Ware Cores Synchronous Serial Interface (DWC_ssi) is a configurable, synthesizable, and programmable component that is a full-duplex master or slave synchronous serial interface. It supports 4 SPI instances which can be used in both RTSS-HE and RTSS-HP. SPI1 instance is configured as master and all other SPI0, SPI2, and SPI3 instances are configured as slave.

- **LPI2C (Low Power Inter-Integrated Circuit)**:
  The Alif SoC includes the LPI2C (Low Power Inter-Integrated Circuit) controller, designed to enable efficient communication with peripheral devices while minimizing power consumption. Integrated into the Ensemble™ series devices, this controller supports low-power modes and allows for seamless data transfer in energy-sensitive applications.

- **I2C (Inter-Integrated Circuit)**:
  The DW_apb_i2c is a configurable, synthesizable, and programmable control bus that provides support for the communications link between integrated circuits in a system. DW I2C can be configured in either master or slave mode and can be operated in both RTSS-HE and RTSS-HP. We have two I2C instances enabled, i2c0 and i2c1.

- **LPI2S (Low Power Inter-IC Sound)**:
  The DW_apb_lpi2s is a configurable, synthesizable, and programmable component designed to be used in systems that process digital audio signals. An instance of LPI2S is available on M55 HE.

- **I2S (Inter-IC Sound)**:
  The DW_apb_i2s is a configurable, synthesizable, and programmable component designed to be used in systems that process digital audio signals. Four I2S instances are available. I2s3_b instance is internally connected to a microphone.

- **MIPI-DSI**:
  The DSI is part of a group of communication protocols defined by MIPI Alliance. The Designware Cores MIPI DSI Host Controller is a digital controller that implements all protocol functions defined in the MIPI DSI Specification. The DWC_mipi_dsi_host provides an interface between the system and the underlying physical layer (MIPI D-PHY) allowing communication with a DSI-compliant display.

- **MIPI-CSI2**:
  MIPI Camera Serial Interface 2 leverages the MIPI D-PHY Physical layer to communicate to the application processor or SoC. CSI2 device captures and transmits an image to the CSI2 host where the SoC resides. Before the image is transmitted, it is placed in the memory in individual frames.

- **RTC**:
  The Low-Power Real-Time Counter (LPRTC) module is a configurable high-range binary counter, which can generate an interrupt on a user-specified interval. The device includes one LPRTC module located in the PD-0 power domain, allowing it to run even when the device is in the lowest power state and power is present on VDD_BATT. The LPRTC module supports incrementing counter and comparator for interrupt generation, 32-bit counter width, and counter wrap mode.

- **DMA (Dynamic Memory Access)**:
  Offloads CPUs from data transfers between memory, peripherals, and cores. We have three controllers: DMA0 (general-purpose), DMA1 (RTSS-HP private), and DMA2 (RTSS-HE private). A MUX allows mapping peripherals to DMA0, enhancing data flow efficiency.

- **PWM (Pulse Width Modulation)**:
  The Alif UTIMER IP is designed to generate PWM signals on Alif development kits. It enables the configuration of the first 12 UTIMER channels, each producing two PWM signals, for a total of 24 simultaneous signals. Each UTIMER instance includes two dedicated compare blocks for PWM signal generation.

- **LPPDM (Low Power Pulse Density Modulation)**:
  The Alif SoC introduces support for Low Power Pulse Density Modulation (LPPDM), enhancing audio capabilities by offering efficient power management for audio capture. The LPPDM module allows simultaneous processing of up to eight PDM microphones, each mapped to its dedicated channel. It converts 1-bit PDM audio data into 16-bit PCM format for high-quality audio capture. The processed audio is accessible through APB registers, supporting a variety of audio applications.

- **PDM (Pulse Density Modulation)**:
  The ZAS now introduces a new PDM Audio module, significantly enhancing audio capabilities. This module supports up to eight PDM microphones simultaneously, each with its dedicated channel. The module efficiently converts 1-bit PDM audio data from each microphone into 16-bit PCM format, allowing for high-quality audio capture and processing. The processed audio data can be easily accessed by software through APB registers, opening up a wide range of possibilities for audio applications.

- **CRC (Cyclic Redundancy Check)**:
  The ZAS has been enhanced with the addition of Cyclic Redundancy Check (CRC) functionality. CRC is a powerful error detection and correction technique that can be utilized to ensure data integrity and reliability in various applications. ZAS supports several industry-standard CRC algorithms (CRC-8-CCITT, CRC-16-CCITT, CRC-32, CRC-32C) and allows for flexible data processing in 8-bit or 32-bit chunks. The CRC module also offers optional automatic byte and bit swapping, as well as customizable polynomials, to accommodate diverse application requirements. Developers can easily integrate this feature through the Advanced High-performance Bus (AHB) for robust error detection in their ZAS projects.

- **WDT (Watchdog Timer)**:
  Developers can now integrate the Zephyr Watchdog Timer (WDT) into their applications using the ZAS framework, enabling fault detection capabilities.

- **OSPI Flash (Octal SPI Flash)**:
  The Alif Devkit-e7 Board incorporates a 32MB ISSI Flash (IS25WX256), interfaced with the Octal SPI controller. This flash memory serves as a vital repository for firmware, configuration data, and essential information. Developers utilizing the Alif Semiconductor Zephyr SDK can harness the flash driver, which implements the Zephyr Standard flash APIs. These APIs empower you to perform the following operations on the flash:

  - **Erase**: Target specific sectors or the entire flash memory.
  - **Read**: Retrieve data from the flash memory.
  - **Write**: Store data into the flash.


- **AES (Advanced Encryption Standard) Support**:
  When booting from OSPI in XIP (Execute-in-Place) mode, the data read from external memory can be decrypted on-the-fly using the AES engine. The AES engine plays a crucial role in ensuring secure execution of code directly from the external flash memory.

- **ADC (Analog-to-Digital Converters)**:
  Enables the conversion of continuous analog signals like voltage and current into discrete digital values. These digital representations can then be easily processed and analyzed by microcontrollers and various digital devices. Alif Semiconductor™ offers two ADC controllers: the ADC12 and ADC24. The ADC12, with its 12-bit resolution, provides 8 channels (6 external and 2 internal) for analog signal input, including a dedicated channel for a temperature sensor and voltage reference. For applications demanding higher precision, ADC24 (Differential-only) provides a 24-bit resolution and 4 differential channels.

- **LPTimer (Low-Power Timer)**:
  The M55 core incorporates a 32-bit LPTIMER module, allowing precise timing and efficient scheduling of low-power applications.

- **Parallel Camera**:
  The Alif SoC now supports capturing camera frames via LPCAM/CAM instances using the parallel interface of the video driver. This interface enables seamless integration with camera sensors such as the MT9M114.

- **AiPM**:
  The Alif SoC now supports AiPM for advanced power management, offering precise control over global SoC device states. AiPM enables fine-tuning of power modes to optimize energy consumption across the system. The SoC can autonomously transition into low-power modes, such as STOP and OFF, based on various wake-up sources including RTC, LPGPIO, and LPTIMER. The Alif SoC supports autonomous transitions between CPU states and SoC power modes using AiPM. It can also control the retention blocks, power domains, and clock sources/frequency available in the SoC.

- **Ethos U55**:
  The Alif SoC supports AI/ML acceleration via the Arm Ethos™-U55 microNPU paired with the Cortex-M55 processors in the Ensemble series. This provides enhanced performance for machine learning tasks on real-time processor cores. The Cortex-M55 implements the Arm v8.1 instruction set with Helium M-Profile Vector Extension (MVE), enabling high-efficiency data processing and AI inference acceleration.

- **MCU-BOOT**:
  MCU-boot is a secure bootloader for 32-bit microcontrollers, providing a common hardware and OS-independent framework for secure booting of applications and recoverable software upgrades. MCU-boot relies on the hardware abstraction layer (HAL) provided by the target OS and consists of two parts: the core bootloader library and the bootloader application. The bootloader framework mentioned above is primarily provided by the bootloader library, while the bootloader application is supplied by the target platform or OS. ZAS supports using MCU-boot as the bootloader and for device firmware upgrades. In this case, the application part of the bootloader will be provided by Zephyr. MCU-boot relies on the hardware porting layers provided by Zephyr, especially the flash map abstraction layer and the flash storage partition description provided by the Zephyr device tree, to enable its functionality.

- **BLE**:
  The Alif BLE host stack in Balletto B1 ROM conserves flash space. BLE is supported only in the Balletto series.

- **LC3**:
  The Alif LC3 Codec in Balletto B1 ROM supports BLE isochronous audio data encoding and decoding.

Known Issues
============

1. The Zephyr device driver for CDC200 only supports ARGB8888, RGB888, and RGB565 formats. However, this is just a subset of all the features supported by the CDC200 IP. This limitation maybe addressed in future releases.

2. In the demo application, restrict the use of formats to ARGB8888 for Layer 2. Layer 2 directly copies an image from a C array to the framebuffer (FB) in ARGB8888 format. Formats for Layer 1 can be modified.

3. Building Zephyr applications from the DTCM of RTSS-HP and RTSS-HE fails with the open-source Clang compiler (LLVM).

4. The Ethos-u application has not been tested for compilation with ArmClang and open-source Clang compilers.

5. Ethos-u application lacks support for building and running from MRAM or ITCM. It currently runs from SRAM0 (0x0200 0000).

6. The RTSS_HE and RTSS_HP I2S application operate from SRAM0 (0x0200 0000) and DTCM (0x5080 0000) in a non-XIP mode.

7. Compiling the I2S application with the open-source Clang compiler results in failure.

8. Camera:

   a. Video buffer allocations to SRAM1 region are non-standard.

   b. Support for RGB formats needs to be added.

   c. The CMOS sensor and CSI bus are configured for RAW10 format, while the Camera controller is configured for RAW8 format due to a lack of proper post-processing tools. The configuration of the CMOS sensor for RAW8 format needs         	reworking.

   d. Not tested with the LLVM toolchain.

   e. The following is a list of issues fixed w.r.t the functionality of peripheral devices.

The following is a list of known issues related to the functionality of peripheral devices.

.. list-table::
   :header-rows: 1

   * - **Alif-ID**
     - **Description**
   * - `PSBT-189`
     - Unable to configure the UART driver with odd, mark, or space parity (UART_CFG_PARITY_ODD, UART_CFG_PARITY_MARK, UART_CFG_PARITY_SPACE).
   * - `PSBT-190`
     - Unable to configure the UART driver with the following stop bit configurations: UART_CFG_STOP_BITS_0_5 and UART_CFG_STOP_BITS_1_5.
   * - `PSBT-613`
     - Communication messages not transmitted between RTSS-HE and RTSS-HP via TCM in the MHU0/MHU1 sample app.
   * - `PSBT-656`
     - LPSPI Master RX & Slave TX data mismatch at 3 MHz frequency with and without DMA.
   * - `PSBT-659`
     - SPI Master RX & Slave TX data mismatch.
   * - `PSBT-672`
     - [Zephyr] UART auto flow control RTS/CTS functionality not enabled/implemented for the E7 DevKit board.
   * - `PSBT-730`
     - OSPI boot failing on HP/HE core with no boot prints using ZAS-v1.0.0 release.
   * - `PSBT-735`
     - OSPI flash operations (read/write/erase) failing at clock speeds of 40MHz or higher on M55-HP/M55-HE cores with ZAS-v1.0.0 release.
   * - `PSBT-479`
     - [Zephyr-v3.3] Compilation of the ethosu application using armclang is failing due to newlib.h not found with TCM cm55_he/cm55_hp using Zephyr-E7-B0-v0.2.0-Beta.
   * - `PSBT-840`
     - [SPARK-Balletto-B1C-A1-DM-HE-ZAS] No prints for the display demo application for MIPI-DSI 1-lane display panel when built using the ARMCLANG compiler.
   * - `PSBT-843`
     - [SPARK-Balletto-B1C-A1-DM-HE-ZAS] No image for the display demo application for CDC200 parallel display panel when built using the ARMCLANG compiler.
   * - `PSBT-625`
     - [Zephyr WDT] Watchdog reset functionality not working.
   * - `PSBT-848`
     - [SPARK-Balletto-B1C-A1-DM-HE-ZAS] No logs visible on the UART2 console for I2S with OSPI (TCM and MRAM are working fine).
   * - `PSBT-846`
     - [SPARK-Balletto-B1C-A1-DM-HE-ZAS] Warnings observed in all build logs when FLASH_ADDRESS_IN_SINGLE_FIFO_LOCATION is set using ZAS-v1.1 release.
   * - `PSBT-847`
     - [SPARK-Balletto-B1C-A1-DM-HE-ZAS] ERROR: Stack overflow on CPU 0 when running the LC3 Codec application.
   * - `PSBT-850`
     - [SPARK-Balletto-B1C-A1-DM-HE-ZAS] Using the BLE application 'ALIF_HR', the device is not listed among the Bluetooth devices when using the mobile application.
   * - `PSBT-831`
     - [SPARK-Balletto-B1C-A1-DM-HE-ZAS] Unable to build the mcuboot app using the armclang toolchain.
   * - `PSBT-857`
     - [SPARK-Balletto-B1C-A1-DM-HE-ZAS] Build error when trying to build the BLE application using the ARM-Clang compiler.
   * - `PSBT-858`
     - [SPARK-Balletto-B1C-A1-DM-HE-ZAS] Build error when trying to build the LC3 codec application using the ARM-Clang and LLVM compiler.

External References
===================

-  ZAS User Guide `ZAS User Guide`_
-  ZAS Application Notes `ZAS Application Notes`_

Copyright/Trademark
===================

The Alif logo is a trademark of Alif Semiconductor. For additional information about Alif Semiconductor trademarks, please refer to `Alif Trademarks`_.
Arm, Cortex, CoreSight, and Ethos are registered trademarks or trademarks of Arm Limited (or its subsidiaries) in the US and/or elsewhere.

This project utilizes Zephyr, an open-source real-time operating system (RTOS) maintained by the Zephyr Project. Zephyr is licensed under the Apache License 2.0. For more information, please visit the `Zephyr Project website`_.

The Zephyr logo is a trademark of The Linux Foundation. Usage of the Zephyr trademark and logo is subject to The Linux Foundation’s Trademark Usage Guidelines.

All other product or service names are the property of their respective owners.
