.. _executorch_kws_ethosu:

ExecutorTorch Keyword Spotting with Ethos-U NPU
################################################

Overview
********

ML inference application using PyTorch ExecutorTorch runtime with Arm Ethos-U NPU acceleration.
Runs a Depthwise Separable CNN (DS-CNN) keyword spotting model optimized with Vela compiler.

This sample demonstrates:

- Loading pre-compiled ExecutorTorch models (.pte files)
- Running quantized int8 inference on Ethos-U NPU
- Using static MFCC audio features as input
- Classifying 12 keyword classes with ~107ms inference time

Model Architecture
******************

**DS-CNN Small** (Depthwise Separable CNN):

- Input: 49 time steps × 10 MFCC features (490 elements, int8 quantized)
- Output: 12 classes (softmax logits)
- Parameters: ~23K
- Operations: ~2.7M MACs
- Quantization: INT8 symmetric per-channel

Supported Keywords
******************

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
****************

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
**********************

- **HE cores (rtss_he)**: U55 supports 128 MACs only
- **HP cores (rtss_hp)**: U55 supports 256 MACs only
- **U85**: Always 256 MACs (both HE and HP cores)

Requirements
************

Hardware
========

- Alif Semiconductor development kit (B1, E1C, E3, E4, E7, or E8)
- USB connection for flashing and serial console

Software
========

- Zephyr SDK with Alif support
- Python 3.10 or later
- ExecutorTorch module (automatically configured via ``west executorch-setup``)

Setup Instructions
******************

Initial Workspace Setup
=======================

For a fresh project, follow these steps:

.. code-block:: console

   # 1. Create workspace directory
   mkdir sdk-alif
   cd sdk-alif

   # 2. Create Python virtual environment
   python3 -m venv .zephyr_venv
   source .zephyr_venv/bin/activate

   # 3. Initialize West workspace
   west init -m git@github.com:alifsemi/sdk-alif.git

   # 4. Enable ExecutorTorch module
   west config manifest.project-filter -- +executorch

   # 5. Update all modules
   west update

   # 6. Setup ExecutorTorch (installs dependencies, applies Alif overrides)
   west executorch-setup

This ``west executorch-setup`` command automatically:

- Initializes ExecutorTorch git submodules
- Installs ExecutorTorch Python package to virtual environment
- Runs ARM-specific setup scripts
- Applies Alif-specific modifications (memory sections, KWS model registration)
- Copies KWS model files to both source and installed package locations

Building the Model
******************

Generate ExecutorTorch .pte Model Files
=======================================

**For Ethos-U55 (256 MACs):**

.. code-block:: console

   python -m modules.lib.executorch.examples.arm.aot_arm_compiler \
       --system_config=RTSS_HP_SRAM_MRAM \
       --config=alif/scripts/ensemble_vela.ini \
       --model_name=kws \
       --quantize \
       --delegate \
       -t ethos-u55-256 \
       --output=kws_u55_256.pte

**For Ethos-U85 (256 MACs):**

.. code-block:: console

   python -m modules.lib.executorch.examples.arm.aot_arm_compiler \
       --system_config=RTSS_HP_SRAM_MRAM \
       --config=alif/scripts/ensemble_vela.ini \
       --model_name=kws \
       --quantize \
       --delegate \
       -t ethos-u85-256 \
       --output=kws_u85_256.pte

**For Ethos-U55 (128 MACs):**

.. code-block:: console

   python -m modules.lib.executorch.examples.arm.aot_arm_compiler \
       --system_config=RTSS_HE_SRAM_MRAM \
       --config=alif/scripts/ensemble_vela.ini \
       --model_name=kws \
       --quantize \
       --delegate \
       -t ethos-u55-128 \
       --output=kws_u55_128.pte

Model Compiler Options
======================

- ``--system_config``: Memory configuration (RTSS_HP_SRAM_MRAM or RTSS_HE_SRAM_MRAM)
- ``--config``: Vela compiler configuration file
- ``--model_name``: Model name (kws)
- ``--quantize``: Enable INT8 quantization
- ``--delegate``: Use Ethos-U delegate for NPU acceleration
- ``-t``: Target NPU configuration
- ``--output``: Output .pte file name

Building and Running
********************

Building for Alif E8 DK (HP Core with U55)
===========================================

.. zephyr-app-commands::
   :zephyr-app: alif/samples/modules/executorch/kws_ethosu/
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :gen-args: -DET_PTE_FILE_PATH=./kws_u55_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256 -DEXTRA_DTC_OVERLAY_FILE=${PWD}/alif/samples/modules/executorch/kws_ethosu/boards/enable_ethosu55.overlay
   :compact:

Or using west build directly:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       alif/samples/modules/executorch/kws_ethosu/ -- \
       -DET_PTE_FILE_PATH=./kws_u55_256.pte \
       -DET_PTE_SECTION=.rodata.model \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256 \
       -DEXTRA_DTC_OVERLAY_FILE=${PWD}/alif/samples/modules/executorch/kws_ethosu/boards/enable_ethosu55.overlay

Building for Alif E8 DK (HP Core with U85)
===========================================

.. zephyr-app-commands::
   :zephyr-app: alif/samples/modules/executorch/kws_ethosu/
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :gen-args: -DET_PTE_FILE_PATH=./kws_u85_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256 -DEXTRA_DTC_OVERLAY_FILE=${PWD}/alif/samples/modules/executorch/kws_ethosu/boards/enable_ethosu85.overlay
   :compact:

Or using west build directly:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       alif/samples/modules/executorch/kws_ethosu/ -- \
       -DET_PTE_FILE_PATH=./kws_u85_256.pte \
       -DET_PTE_SECTION=.rodata.model \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256 \
       -DEXTRA_DTC_OVERLAY_FILE=${PWD}/alif/samples/modules/executorch/kws_ethosu/boards/enable_ethosu85.overlay

Building for Alif E7 DK (HP Core with U55)
===========================================

.. zephyr-app-commands::
   :zephyr-app: alif/samples/modules/executorch/kws_ethosu/
   :board: alif_e7_dk/ae722f80f55d5xx0/rtss_hp
   :goals: build
   :gen-args: -DET_PTE_FILE_PATH=./kws_u55_256.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256 -DEXTRA_DTC_OVERLAY_FILE=${PWD}/alif/samples/modules/executorch/kws_ethosu/boards/enable_ethosu55.overlay
   :compact:

Building for Alif E7 DK (HE Core with U55-128)
===============================================

.. zephyr-app-commands::
   :zephyr-app: alif/samples/modules/executorch/kws_ethosu/
   :board: alif_e7_dk/ae722f80f55d5as0/rtss_he
   :goals: build
   :gen-args: -DET_PTE_FILE_PATH=./kws_u55_128.pte -DET_PTE_SECTION=.rodata.model -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128 -DEXTRA_DTC_OVERLAY_FILE=${PWD}/alif/samples/modules/executorch/kws_ethosu/boards/enable_ethosu55.overlay
   :compact:

Flashing
********

.. code-block:: console

   west flash

Sample Output
*************

.. code-block:: console

   *** Booting Zephyr OS build 3220a641d125 ***

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
**********

- `ExecutorTorch Documentation <https://pytorch.org/executorch/>`_
- `Arm Ethos-U NPU <https://developer.arm.com/ip-products/processors/machine-learning/arm-ethos-u>`_
- `Vela Compiler <https://github.com/nxp-imx/ethos-u-vela>`_
- `Alif Semiconductor <https://alifsemi.com>`_
- `Google Speech Commands Dataset <https://arxiv.org/abs/1804.03209>`_
- `DS-CNN Paper <https://arxiv.org/abs/1711.07128>`_
