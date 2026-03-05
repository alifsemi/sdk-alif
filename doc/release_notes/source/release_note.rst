.. _Release Notes:

Introduction
------------
The **Zephyr Alif SDK (ZAS)** is a comprehensive development suite, enabling developers to configure, build, and deploy applications for Alif Semiconductor's microcontrollers.

Supported Development Kits
~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Ensemble Series** - High-performance multi-core MCUs with Arm Cortex-M55 cores:

- **DK-E7**: Configurable to emulate E5, and E3 series devices (Ethos-U55 microNPUs)
- **DK-E8**: Configurable to emulate E6 and E4 series devices (Ethos-U55 and Ethos-U85 microNPUs)
- **DK-E1C**: Compact series development platform

**Balletto Series** - Wireless-enabled MCUs with AI/ML acceleration:

- **DK-B1**: Features Bluetooth Low Energy 5.3, 802.15.4 Thread support, Ethos-U55 microNPU, and Cortex-M55 core

Installing the SDK and Building the Application
-----------------------------------------------

For detailed instructions, please refer to the `ZAS User Guide`_.

Host Requirements
-----------------

- Ubuntu 22.04.5 LTS or above

Toolchains
----------

The following toolchains have been tested for the SDK application:

.. list-table::
   :header-rows: 1

   * - Toolchain
     - Version
     - Link
   * -  Zephyr SDK (GCC)
     - v0.17.0
     - `Zephyr SDK download`_

Software Components
-------------------

The following are the software components used in the latest release.

.. list-table::
   :header-rows: 1

   * - Component
     - Version
     - Link
   * -  Alif SDK
     - v2.0-zas-branch
     - `Alif SDK`_
   * -  Alif Zephyr RTOS
     - v2.0-zas-branch
     - `Alif SDK - Zephyr`_
   * -  Alif SDK - HAL
     - v2.0-zas-branch
     - `Alif SDK - HAL`_
   * -  Alif Secure Enclave (SE)
     - v1.109
     - `Alif Security Toolkit Quick Start Guide`_

.. note::
   This release requires Secure Enclave software version v1.109 or later for proper operation.

Supported Peripheral Drivers and Features
------------------------------------------

Networking Interfaces
~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - Ethernet
     - Ethernet Controller (ETH) compliant with IEEE 802.3-2008, featuring Reduced Media Independent Interface (RMII) for external PHY connectivity. Supports full-duplex and half-duplex operation, hardware flow control, and TCP/UDP checksum offloading to reduce CPU overhead. Integrated with Zephyr’s networking stack for DHCP-based IP assignment, socket APIs, and ICMP (ping) functionality.

Communication Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - UART
     - Synopsys DW_apb_uart supporting up to 8 ports. By default UART2 and UART4 are enabled and used as a console for RTSS-HE and RTSS-HP core respectively.
   * - SPI
     - Synopsys DWC_ssi full-duplex interface with 4 instances. SPI1 (master), SPI0/SPI2/SPI3 (slaves).
   * - I2C
     - DW_apb_i2c with master/slave mode support. Two instances: i2c0 and i2c1.
   * - LPI2C
     - Low-power I2C controller for power-efficient peripheral communication.
   * - I3C
     - Improved Inter-Integrated Circuit (I3C) interface supporting high-speed, low-power communication with dynamic addressing, in-band interrupts, and backward compatibility with I2C devices.
   * - LP-UART
     - Low-power Universal Asynchronous Receiver/Transmitter supporting extended sleep modes with wake-on-receive capability. Maintains serial communication during system low-power states with reduced power consumption compared to standard UART peripherals.
   * - LP-SPI
     - Low-power SPI controller capable of operating in deep sleep modes, enabling communication with external sensors and peripherals while minimizing power consumption, with wake-on-transfer support for event-driven applications in Zephyr.
   * - USB-Device
     - USB device mode support using the Synopsys DWC3 controller with Alif’s UDC driver. Includes CDC-ACM class implementation to enable the board to function as a virtual COM port, allowing serial communication with a host PC. Supports standard USB device enumeration and data transfer in Zephyr RTOS.
   * - CAN-FD
     - Controller Area Network (CAN) driver for 2-wire bus communication used to transmit sensor data and control information between system components. Commonly used in automotive applications for reliable communication between Electronic Control Units (ECUs).
   * - TOF Sensor
     - CH201 ultrasonic Time-of-Flight (ToF) sensor for proximity sensing and distance measurement. Supports I²C communication with interrupt-based event signaling.

Display Interfaces
~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - MIPI-DSI
     - MIPI Display Serial Interface supporting high-speed, low-power video data transmission to external LCD/OLED panels. Integrated with Zephyr’s display subsystem.
   * - CDC-200
     - Configurable Display Controller (CDC-200) PHY for MIPI-DSI, enabling reliable high-speed display link operation.
   * - Touch screen
     - Capacitive touch controller supporting multi-touch gestures via I2C or SPI. Provides touch coordinates and wake-on-touch capability.

Display Support
~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 30 25 45

   * - **Display Variant**
     - **Interface**
     - **Supported Boards**
   * - MW405-ILI8906E
     - 2-Lane Serial (MIPI-DSI)
     - DevKit E7, DevKit E8
   * - ILI9488
     - 1-Lane Serial (MIPI-DSI)
     - E1C, B1 A5/A6
   * - Parallel Display
     - Parallel
     - DevKit E7, DevKit E8, E1C, B1 A5/A6

Audio Interfaces
~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - I2S
     - DW_apb_i2s up to four instances for digital audio.
   * - LPI2S
     - Low-power DW_apb_lpi2s for digital audio signal processing.
   * - PDM
     - Pulse Density Modulation (PDM) supports eight channels, each PDM interface consists of 4 data inputs, with each input carrying 2 audio channels (left and right).
   * - LPPDM
     - Low-Power Pulse Density Modulation supports eight channels, each LPPDM interface consists of 4 data inputs, with each input carrying 2 audio channels (left and right).

System Resources
~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - GPIO
     - General-purpose I/O pins controllable by software as inputs or outputs.
   * - MHU
     - Message Handling Units for interrupt-driven inter-subsystem communication.
   * - HWSEM
     - Hardware semaphores for shared resource synchronization across subsystems.
   * - RTC
     - Low-Power Real-Time Counter (LPRTC) with 32-bit counter and interrupt generation.
   * - WDT
     - Watchdog timer for fault detection.
   * - Clk-Ctrl
     - Clock control module manages peripheral clock generation and its gating.
   * - PinMUX
     - Pin multiplexer controlling GPIO pin function selection and routing of peripheral signals to physical pins, enabling flexible I/O configuration.
   * - System Power Management (suspend to ram)
     - Power management framework supporting deep sleep states including Suspend-to-RAM (S2RAM), where SRAM is retained, enabling fast resume and ultra-low power idle operation.
   * -  LP-GPIO
     - Low-power GPIO controller that maintains state and wake-up capability during system sleep modes, allowing external events to trigger resume from low-power states.
   * - EVTRTR
     - The Event Router (EVTRTR) is a module that can associate an event originated by one peripheral with an action executed by another.

Timers and PWM
~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - PWM
     - Alif UTIMER IP generating up to 24 simultaneous PWM signals across 12 channels.
   * - QDEC
     - Quadrature decoder mode for precise rotary encoder position tracking.
   * - UTimer Counter
     - Counter mode for event/clock pulse counting, frequency measurement, and timer-based scheduling.
   * - LP-Timer
     - Low-power timer for sleep/idle modes with wake-up events, periodic interrupts, and timekeeping.

Memory and Storage
~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**

   * - MRAM
     - Magnetoresistive RAM with Read and Write operations.

   * - OSPI Flash
     - | **ISSI Flash (IS25WX512)** support using Zephyr flash APIs for erase, read, and write operations.
       | **Macronix Flash (MX66UW)** support for high-speed execute-in-place (XIP) and data storage, with erase, read, and write operations through the Zephyr flash subsystem.

   * - SD
     - Secure Digital host controller supporting SD/SDIO/MMC protocols for external memory card interfacing, including command queuing and data transfer at high-speed rates.

   * - HexSPI support for AP memory PSRAM (APS512XXN)
     - HexSPI interface support for external APMEM device, enabling high-bandwidth external memory expansion.

Security and Data Integrity
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - CRC
     - Supports CRC-8-CCITT, CRC-16-CCITT, CRC-32, and CRC-32C with flexible data processing via AHB.
   * - Entropy
     - Hardware true random number generator (TRNG) providing high-quality entropy for cryptographic operations, compliant with NIST SP 800-90B, and integrated with Zephyr’s entropy subsystem for secure key generation and randomization.

Analog and Conversion
~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - DAC 12
     - 12-bit Digital to Analog Converter with 0 V to 1.8 V output range in Low-Power mode.
   * - ADC 12
     - 12-bit Analog-to-Digital Converter with configurable sampling rate and input channels, supporting general-purpose sensor measurements and fast conversion in both active and low-power operating modes.
   * - CMP
     - High-Speed Analog Comparator (CMP) module featuring rail-to-rail input, multi-channel support, and programmable reference voltage sourced from DAC6, internal Vref, or external pins. Includes programmable hysteresis (0 mV to 45 mV), output polarity control, configurable input filtering with interrupt generation, and shared analog control via the comp_reg1 register (accessible only through the CMP0 register map).

AI Acceleration
~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - Ethos
     - The Ethos U-55/U-85 NPU is a hardware acceleration solution
       integrated into Alif’s microcontroller platforms that leverages Arm
       Ethos microNPUs to boost machine learning inference performance
       for CNN and transformer models.
   * - Executorch
     - ML inference application using the PyTorch ExecutorTorch runtime with Arm Ethos-U NPU acceleration. Demonstrates keyword spotting using a quantized DS-CNN model with optimized inference performance.

Camera Interfaces
~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - CPI
     - Camera Parallel Interface supporting high-speed parallel data capture from CMOS/CCD image sensors (e.g., OV5640, MT9M114). Provides pixel clock, horizontal/vertical sync, and data lanes for raw image streaming.
   * - LPCPI
     - Low-Power Camera Parallel Interface optimized for energy-efficient imaging applications. Supports reduced clocking, sleep modes, and wake-on-frame while maintaining compatibility with standard parallel image sensors.
   * - MIPI-CSI
     - MIPI Camera Serial Interface (CSI-2) supporting high-speed serial transmission of pixel data from image sensors (e.g., ARX3A0). Data is received via D-PHY, processed through the PHY Protocol Interface (PPI), unpacked by the MIPI CSI-2 host controller, and delivered via the Image Pixel Interface (IPI) to the Camera Pixel Interface (CPI) for storage in memory over AXI. Integrated with Zephyr’s video input subsystem for streaming and frame capture.
   * - ISP
     - Image Signal Processor (ISP) for real-time enhancement and processing of raw image frames from camera sensors such as the ARX3A0. Supports features like auto-exposure, white balance, noise reduction, and color correction. Integrated with the video driver to enable capture and processing pipelines in Zephyr-based applications.


Camera Sensors Support
~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 25 50

   * - **Camera Sensor**
     - **Interface**
     - **Supported Boards**
   * - ARX3A0
     - Serial (MIPI-CSI)
     - DevKit E7, DevKit E8
   * - MT9M114
     - Parallel (CPI)
     - DevKit E7, DevKit E8, E1C, B1 A5/A6
   * - OV5640
     - Parallel (CPI)
     - E1C StartKit

Wireless Connectivity
~~~~~~~~~~~~~~~~~~~~~

**Bluetooth LE 5.3** (*Balletto B1 only*)

Two host stack options:

* **Alif BLE (ROM-based)**: Power-optimized with BLE ROM v1.2, reduced flash/RAM footprint
* **Zephyr BLE**: Standard Zephyr Bluetooth implementation for portability

Comprehensive BLE samples included:

* **LE Audio**: Auracast broadcast/sink, unicast audio source/sink
* **Profiles**: BAS, BLPS, CPPS, CSCPS, GLPS, HR, HTPT, PRXP, RSCPS, WS
* **Advanced**: Throughput testing, mesh light switch/bulb, SMP server
* **Power Management**: Low-power peripheral optimization

Breaking Changes
----------------

- **Balletto B1 Hardware Support**:
  This release supports only Balletto B1 revA6 or newer. Earlier revisions are not supported.

- **BLE ROM Version Configuration**:
  BLE ROM version is now hardware-specific and defined in device tree. Support for BLE ROM v1.0 has been removed. Only BLE ROM v1.2 is supported. The ROM version is automatically detected from hardware and cannot be manually configured by users.

- The CRC driver DTS property `crc_algo` has been corrected to `crc-algo`, and the `crc-algo` enum   values were changed to lowercase to align with Zephyr coding style.

- Corrected the SRAM1 mapping for Eagle SoC from `0x08000000` to `0x02400000`.

Planned Deprecations
--------------------

The following items are planned for deprecation in the next release:

- **clock-control**

  Enabling CGU clocks has been simplified. CGU clocks are now automatically
  enabled during clock-control initialization based on the status of the
  corresponding ``clocks`` devicetree nodes.

  Direct access to the CGU Clock Enable Register (``CGU_CLK_ENA``) is
  deprecated and will be removed in a future release.

  Applications and drivers should rely on the clock-control framework and
  devicetree ``clocks`` configuration instead of manually enabling clocks
  through the register.

- **E3-DK and E4-DK Board Support**

  Support for **E3-DK** and **E4-DK** board files will be removed. Support for **E3** and **E4** SoCs will continue to be available through **E8-DK** and **E7-DK** builds.

- **DMA Node Referencing**

  Referring to DMA nodes directly in device driver nodes will be deprecated. All drivers using DMA must reference DMA resources through **EVTRTR** nodes.

Known Issues
------------

- **BLE** le_periph_pm application has a RTC related issue which causes M55 RTC alarms to stop working randomly
- **BLE** audio Unicast initiator fails to open 2nd channel when using Ceva host stack
- **BLE** Auracast sink does not receive an encryption key sent by Auracast assistant when using Ceva host stack
- **BLE** Connection param update does not work with Ceva host stack, works fine with Zephyr host stack.
- **SPI1** DMA operations exhibit inconsistent behavior.
- **Touch Screen** events are intermittently dropped.
- **OSPI** boot has not been verified.
- When run from HE-MRAM, the **PM** demo application throws an error message.
- **LPCMP** sample is broken on all families.
- **LP Camera** node in overlay file needs to be updated to I2C1 for B1-DK.

External References
-------------------

- `ZAS User Guide`_

Copyright/Trademark
-------------------

The Alif logo is a trademark of Alif Semiconductor. Please refer to `Alif Trademarks`_.
Arm, Cortex, CoreSight, and Ethos are trademarks of Arm Limited (or its subsidiaries).
Zephyr is an open-source RTOS under the Apache License 2.0, maintained by the `Zephyr Project website`_.
The Zephyr logo is a trademark of The Linux Foundation, subject to The Linux Foundation's `Trademark Usage Guidelines`_.
All other names are property of their respective owners.
