.. _executorch_kws_ethosu:

================================================
ExecutorTorch Keyword Spotting with Ethos-U NPU
================================================

Overview
=========

This document explains how to build, configure, and run the ExecutorTorch Keyword Spotting (KWS)
application using the Alif Semiconductor™ DevKit with Arm® Ethos-U NPU acceleration.

The application uses the PyTorch ExecutorTorch runtime with a Depthwise Separable CNN (DS-CNN)
model optimized using the Vela compiler. It demonstrates loading a pre-compiled model (``.pte``),
running INT8 inference on the Ethos-U NPU, and evaluating KWS results using static MFCC input data.

Key Features
-------------

- Loads ExecutorTorch pre-compiled models (``.pte``)
- Performs quantized INT8 inference on Ethos-U (U55/U85)
- Uses static MFCC audio features as input data
- Executes a DS-CNN keyword spotting network
- Recognizes 12 keyword classes
- Typical inference time: ~107 ms

Model Architecture
--------------------

**DS-CNN Small (Depthwise Separable CNN):**

- Input: 49 time steps × 10 MFCCs (490 elements, INT8)
- Output: 12 keyword classes
- Parameters: ~23K
- MAC operations: ~2.7M
- Quantization: INT8 symmetric per-channel

Supported Keywords
-------------------

The KWS model recognizes 12 keyword classes:

0. silence
1. unknown
2. yes
3. no
4. up
5. down
6. left
7. right
8. on
9. off
10. stop
11. go

Supported Boards
-----------------

+-------+--------+---------+-------------------------+
| Board | U55    | U85     | Notes                   |
+=======+========+=========+=========================+
| B1    | Yes    | No      | U55 only                |
+-------+--------+---------+-------------------------+
| E1C   | Yes    | No      | U55 only                |
+-------+--------+---------+-------------------------+
| E3    | Yes    | No      | U55 only                |
+-------+--------+---------+-------------------------+
| E4    | Yes    | Yes     | Dual NPU                |
+-------+--------+---------+-------------------------+
| E7    | Yes    | No      | U55 only                |
+-------+--------+---------+-------------------------+
| E8    | Yes    | Yes     | Dual NPU                |
+-------+--------+---------+-------------------------+

Core Type Restrictions
-----------------------

- **HE cores (rtss_he)**: U55 supports 128 MACs only
- **HP cores (rtss_hp)**: U55 supports 256 MACs only
- **U85**: Always 256 MACs (both HE and HP cores)

Prerequisites
=============

Hardware Requirements
---------------------
- Alif Devkit
- USB connection for flashing and serial console

Software Requirements
---------------------
- **Alif SDK**: Clone from `https://github.com/alifsemi/sdk-alif.git <https://github.com/alifsemi/sdk-alif.git>`_
- **West Tool**: For building Zephyr applications (refer to the `ZAS User Guide`_)
- **Arm GCC Compiler**: For compiling the application (part of the Zephyr SDK)
- **SE Tools**: For loading binaries (refer to the `ZAS User Guide`_)
- **ExecutorTorch module** (automatically configured via ``west executorch-setup``)

.. include:: note.rst


Building an ExecutorTorch Application with Zephyr
==================================================

Follow these steps to build the ExecutorTorch application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.


2. Set up ExecutorTorch:

   After running ``west update``, run the following command to set up ExecutorTorch.
   This step installs the required dependencies and applies Alif-specific configurations.

   .. code-block:: console

      west executorch-setup

Note: command automatically performs the following actions:

   - Initializes ExecutorTorch git submodules
   - Installs the ExecutorTorch Python package into the virtual environment
   - Runs ARM-specific setup scripts
   - Applies Alif-specific modifications (memory sections and KWS model registration)
   - Copies KWS model files to both the source and installed package locations

3. Build commands for applications on the HP core:

.. code-block:: console

   west build -p always \
   -b alif_e7_dk/ae722f80f55d5xx0/rtss_hp \
   -S alif/samples/modules/executorch/kws_ethosu/ \
   -- \
   -DET_PTE_FILE_PATH=./kws_u55_256.pte \
   -DET_PTE_SECTION=.rodata.model \
   -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256 \
   -DEXTRA_DTC_OVERLAY_FILE=\
   ${PWD}/alif/samples/modules/executorch/kws_ethosu/boards/enable_ethosu55.overlay

4. Build commands for applications on the HE core:

.. code-block:: console

   west build -p always \
   -b alif_e7_dk/ae722f80f55d5xx0/rtss_he \
   -S alif/samples/modules/executorch/kws_ethosu/ \
   -- \
   -DET_PTE_FILE_PATH=./kws_u55_256.pte \
   -DET_PTE_SECTION=.rodata.model \
   -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256 \
   -DEXTRA_DTC_OVERLAY_FILE=\
   ${PWD}/alif/samples/modules/executorch/kws_ethosu/boards/enable_ethosu55.overlay

Once the build command completes successfully, executable images will be generated and placed in the `build/zephyr` directory. Both `.bin` (binary) and `.elf` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
===============================

To execute binaries on the DevKit, follow the command:

.. code-block:: console

   west flash

Sample Output
==============

.. code-block:: console

   *** Booting Zephyr OS build ***

   ========================================
   ExecutorTorch Keyword Spotting Demo
   ========================================

   I [executorch:main.cpp:279 main()] Ethos-U backend registered successfully
   I [executorch:main.cpp:285 main()] Model PTE at 0x8021eb50, Size: 35280 bytes
   I [executorch:main.cpp:291 main()] Model data loaded. Size: 35280 bytes.
   I [executorch:main.cpp:303 main()] Model loaded, has 1 methods
   I [executorch:main.cpp:311 main()] Running method: forward
   I [executorch:main.cpp:323 main()] Method allocator pool size: 1572864 bytes
   I [executorch:main.cpp:338 main()] Setting up planned buffer 0, size 2464.
   I [executorch:main.cpp:359 main()] Loading method...
   I [executorch:main.cpp:373 main()] Method 'forward' loaded successfully
   I [executorch:main.cpp:375 main()] Preparing input tensor with static KWS data...
   I [executorch:main.cpp:376 main()] Input data size: 490 bytes
   I [executorch:main.cpp:390 main()] Input prepared successfully
   I [executorch:main.cpp:392 main()]
   --- Starting inference ---
   I [executorch:main.cpp:408 main()] Inference completed in 107 ms
   I [executorch:main.cpp:414 main()]
   --- Inference Results ---
   I [executorch:main.cpp:418 main()] Predicted keyword: "left" (class 6)
   I [executorch:main.cpp:422 main()]
   Output tensor values:
   I [executorch:main.cpp:430 main()]   output[0]: scalar_type=Float numel=12
   I [executorch:main.cpp:442 main()]     [0] silence: -0.0672 (q: 0x77)
   I [executorch:main.cpp:442 main()]     [1] unknown: 0.0218 (q: 0x82)
   I [executorch:main.cpp:442 main()]     [2] yes: 0.0690 (q: 0x88)
   I [executorch:main.cpp:442 main()]     [3] no: -0.0265 (q: 0x7c)
   I [executorch:main.cpp:442 main()]     [4] up: -0.0511 (q: 0x79)
   I [executorch:main.cpp:442 main()]     [5] down: -0.0586 (q: 0x78)
   I [executorch:main.cpp:442 main()]     [6] left: 0.1220 (q: 0x8f)
   I [executorch:main.cpp:442 main()]     [7] right: 0.0520 (q: 0x86)
   I [executorch:main.cpp:442 main()]     [8] on: 0.0331 (q: 0x84)
   I [executorch:main.cpp:442 main()]     [9] off: -0.0785 (q: 0x75)
   I [executorch:main.cpp:442 main()]     [10] stop: -0.1192 (q: 0x70)
   I [executorch:main.cpp:442 main()]     [11] go: -0.0369 (q: 0x7b)
   I [executorch:main.cpp:454 main()]
   --- Verification ---
   I [executorch:main.cpp:460 main()] SUCCESS: Output shape verified (12 classes)
   I [executorch:main.cpp:461 main()] (Value verification skipped - using untrained model)
   I [executorch:main.cpp:484 main()]
   ========================================
   I [executorch:main.cpp:485 main()] Keyword Spotting Demo Complete
   I [executorch:main.cpp:486 main()] Inference time: 107 ms
   I [executorch:main.cpp:487 main()] Result: PASS
   I [executorch:main.cpp:488 main()] ========================================


References
===========

Refer to the `ExecutorTorch Documentation`_ for more details.

Refer to the `Arm Ethos-U NPU`_ for architecture details.

Refer to the `Vela Compiler`_ for optimization guidance.

Refer to the `Alif Semiconductor`_ website for board documentation.

Refer to the `Google Speech Commands Dataset`_ for dataset details.

Refer to the `DS-CNN Paper`_ for model architecture insights.