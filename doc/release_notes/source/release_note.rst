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

.. note::
   While other Linux distributions may work, they have not been thoroughly tested.

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

Communication Interfaces
~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - UART
     - Synopsys DW_apb_uart supporting up to 8 ports. UART2 and UART4 enabled for RTSS-HP and RTSS-HE subsystems.
   * - SPI
     - Synopsys DWC_ssi full-duplex interface with 4 instances. SPI1 (master), SPI0/SPI2/SPI3 (slaves).
   * - I2C
     - DW_apb_i2c with master/slave mode support. Two instances: i2c0 and i2c1.
   * - LPI2C
     - Low-power I2C controller for power-efficient peripheral communication.

Audio Interfaces
~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - I2S
     - DW_apb_i2s with four instances for digital audio. I2S3_b connects to internal microphone.
   * - LPI2S
     - Low-power DW_apb_lpi2s for digital audio signal processing.
   * - PDM
     - Supports eight PDM microphones, converting 1-bit PDM to 16-bit PCM.
   * - LPPDM
     - Low-power PDM supporting up to eight microphones with 1-bit PDM to 16-bit PCM conversion.

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
     - Message Handling Units for interrupt-driven inter-subsystem communication. 12 MHUs for Secure and 12 for Non-Secure access.
   * - HWSEM
     - Hardware semaphores for shared resource synchronization across subsystems.
   * - DMA
     - **E3/E4/E5/E6/E7/E8**: Three controllers (DMA0, DMA1, DMA2) with MUX. **B1/E1C**: DMA2 only.
   * - RTC
     - Low-Power Real-Time Counter (LPRTC) with 32-bit counter and interrupt generation.
   * - WDT
     - Watchdog timer for fault detection.

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
     - 32MB ISSI Flash (IS25WX256) with Zephyr flash APIs for erase, read, and write operations.

Security and Data Integrity
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - AES
     - Advanced Encryption Standard for on-the-fly decryption of XIP data from external memory.
   * - CRC
     - Supports CRC-8-CCITT, CRC-16-CCITT, CRC-32, and CRC-32C with flexible data processing via AHB.

Analog and Conversion
~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - **Peripheral**
     - **Description**
   * - DAC 12
     - 12-bit Digital to Analog Converter with 0 V to 1.8 V output range in Low-Power mode.

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

Known Issues
------------

- **BLE** le_periph_pm application has a RTC related issue which causes M55 RTC alarms to stop working randomly
- **BLE** audio Unicast initiator fails to open 2nd channel when using Ceva host stack
- **BLE** Auracast sink does not receive an encryption key sent by Auracast assistant when using Ceva host stack
- **BLE** Connection param update does not work with Ceva host stack, works fine with Zephyr host stack.

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
