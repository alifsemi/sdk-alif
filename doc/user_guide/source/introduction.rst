Introduction
============

In this user guide, we cover the following steps:

1. **Prerequisites for Building Zephyr OS:**
   Understand the requirements for building Zephyr OS, a small Real-Time Operating System designed for connected, resource-constrained, and embedded devices       for the real-time subsystems in Alif's devices.

2. **Building an Application Using the West Tool:**
   Learn how to build an application using the west tool. West is Zephyr’s meta tool written in Python, which calls CMake/Make commands on a Linux host—such      as Ubuntu.

3. **Programming the Application Binary onto ITCM:**
   Steps to program the application binary onto the ITCM of one of the real-time subsystems and booting the application.

4.  **Running the Application from MRAM:**
    Discover how to run applications directly from MRAM. Learn about the supported targets, specific MRAM boot addresses for RTSS-HE and RTSS-HP, and the           necessary build commands.

The Alif DevKit
---------------

The Alif DevKit is a development board featuring an Alif multi-core SoC, offering both high-performance and low-power execution.

* It includes multiple Cortex-M class processors (Ensemble E3 and E7 series) and additionally multiple Cortex-A class processors (Ensemble E7 series).

The E7 series processors have:

    * Real-Time Processor cores: Cortex-M55 processors with the Arm v8.1 instruction set (including Helium M-Profile Vector Extension) and an Arm Ethos™-U55 microNPU for AI/ML acceleration.

Real-Time Processor Cores
-------------------------

* High-Performance Arm Cortex-M55 (RTSS-HP) operating at up to 400 MHz
* High-Efficiency Arm Cortex-M55 (RTSS-HE) operating at up to 160 MHz

Zephyr RTOS and Toolchain
-------------------------

The real-time subsystems boot with Zephyr OS, a small RTOS for connected, resource-constrained, and embedded devices. Zephyr supports multiple architectures and is available under the Apache 2.0 license.

Zephyr uses a meta-tool called `west` to execute Kconfig, CMake and build system commands (Make or Ninja). CMake builds applications with the Zephyr kernel in two stages:

* **Configuration stage:** CMakeLists.txt build scripts are executed to generate host-native build scripts.

* **Build stage:** The generated build scripts are executed to compile the application.

Hardware and Software Requirements
----------------------------------

Hardware Requirement

~~~~~~~~~~~~~~~~~~~~~

* Alif Ensemble Devkit Gen2 (DK-E7).

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


Target Reference Board
~~~~~~~~~~~~~~~~~~~~~~

* Alif Devkit Ensemble E3
* Alif Devkit Ensemble E7

Software Components
~~~~~~~~~~~~~~~~~~~

The following software components are part of SDK:

.. list-table::
   :header-rows: 1

   * - Name
     - Path
     - Repository
   * - zephyr
     - zephyr
     - `zephyr_alif`_
   * - mcuboot_alif
     - bootloader/mcuboot
     - `mcuboot_alif`_
   * - cmsis_alif
     - modules/hal/cmsis
     - `cmsis_alif`_
   * - hal_alif
     - modules/hal/alif
     - `hal_alif`_
   * - sdk-alif
     - Alif Zephyr SDK
     - `sdk-alif`_