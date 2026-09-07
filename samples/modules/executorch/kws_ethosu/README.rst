.. _executorch_kws_ethosu:

executorch Keyword Spotting with Ethos-U NPU
#############################################

Overview
********

ML inference application using PyTorch executorch runtime with Arm Ethos-U NPU acceleration.
Runs a Depthwise Separable CNN (DS-CNN) keyword spotting model optimized with Vela compiler.

This sample demonstrates:

- Loading pre-compiled executorch models (.pte files)
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

**Tested and Verified:**

+-------+--------+---------+---------+-------------------------+
| Board | U55    | U85     | APSS    | Status                  |
+=======+========+=========+=========+=========================+
| B1    | Yes    | No      | No      | **Tested**              |
+-------+--------+---------+---------+-------------------------+
| E1C   | Yes    | No      | No      | **Tested**              |
+-------+--------+---------+---------+-------------------------+
| E7    | Yes    | No      | No      | **Tested** (U55 only)   |
+-------+--------+---------+---------+-------------------------+
| E8    | Yes    | Yes     | Yes     | **Tested**              |
+-------+--------+---------+---------+-------------------------+

**Expected to Work (Not Yet Tested):**

+-------+--------+---------+---------+-------------------------+
| Board | U55    | U85     | APSS    | Notes                   |
+=======+========+=========+=========+=========================+
| E3    | Yes    | No      | No      | U55 only                |
+-------+--------+---------+---------+-------------------------+
| E4    | Yes    | Yes     | No      | Dual NPU                |
+-------+--------+---------+---------+-------------------------+

Core Type Restrictions
**********************

- **HE cores (rtss_he)**: U55 supports 128 MACs only
- **HP cores (rtss_hp)**: U55 supports 256 MACs only
- **APSS core (apss)**: U85 with 256 MACs only (Cortex-A32); the U55 is not
  reachable from APSS, and requesting it fails the build
- **U85**: Always 256 MACs (HE, HP and APSS cores)

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
- executorch module (automatically configured via ``west executorch-setup``)

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

   # 4. Enable executorch module
   west config manifest.project-filter -- +executorch

   # 5. Update all modules
   west update

   # 6. Setup executorch (installs dependencies, applies Alif overrides)
   west executorch-setup

This ``west executorch-setup`` command automatically:

- Initializes executorch git submodules
- Installs executorch Python package to virtual environment
- Applies Alif-specific modifications (KWS model registration, CMakeLists)
- Copies KWS model files to both source and installed package locations

.. note::
   The Arm Corstone FVP simulator is **not required** for running on real Alif
   hardware and is skipped by default. To install it (for FVP simulation only):

   .. code-block:: console

      west executorch-setup --fvp

Building the Model
******************

Generate executorch .pte Model Files
=======================================

**For Ethos-U55 (256 MACs):**

.. code-block:: console

   python -m modules.lib.executorch.examples.arm.aot_arm_compiler \
       --system_config=RTSS_HP_SRAM_MRAM \
       --config=alif/samples/modules/executorch/ensemble_vela.ini \
       --model_name=kws \
       --quantize \
       --delegate \
       -t ethos-u55-256 \
       --output=kws_u55_256.pte

**For Ethos-U85 (256 MACs):**

.. code-block:: console

   python -m modules.lib.executorch.examples.arm.aot_arm_compiler \
       --system_config=RTSS_HP_SRAM_MRAM \
       --config=alif/samples/modules/executorch/ensemble_vela.ini \
       --model_name=kws \
       --quantize \
       --delegate \
       -t ethos-u85-256 \
       --output=kws_u85_256.pte

**For Ethos-U55 (128 MACs):**

.. code-block:: console

   python -m modules.lib.executorch.examples.arm.aot_arm_compiler \
       --system_config=RTSS_HE_SRAM_MRAM \
       --config=alif/samples/modules/executorch/ensemble_vela.ini \
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

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       -S ethos-u55-enable \
       alif/samples/modules/executorch/kws_ethosu/ \
       -p always -- \
       -DET_PTE_FILE_PATH=./kws_u55_256.pte \
       -DET_PTE_SECTION=.rodata.model \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Building for Alif E8 DK (HP Core with U85)
===========================================

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       -S ethos-u85-enable \
       alif/samples/modules/executorch/kws_ethosu/ \
       -p always -- \
       -DET_PTE_FILE_PATH=./kws_u85_256.pte \
       -DET_PTE_SECTION=.rodata.model \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Building for Alif E8 DK (HE Core with U85)
===========================================

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       -S ethos-u85-enable \
       alif/samples/modules/executorch/kws_ethosu/ \
       -p always -- \
       -DET_PTE_FILE_PATH=./kws_u85_256.pte \
       -DET_PTE_SECTION=.rodata.model \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

Building for Alif E7 DK (HP Core with U55)
===========================================

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       -S ethos-u55-enable \
       alif/samples/modules/executorch/kws_ethosu/ \
       -p always -- \
       -DET_PTE_FILE_PATH=./kws_u55_256.pte \
       -DET_PTE_SECTION=.rodata.model \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

Building for Alif E7 DK (HE Core with U55-128)
===============================================

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       -S ethos-u55-enable \
       alif/samples/modules/executorch/kws_ethosu/ \
       -p always -- \
       -DET_PTE_FILE_PATH=./kws_u55_128.pte \
       -DET_PTE_SECTION=.rodata.model \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Building for Alif E1C DK (HE Core with U55-128)
===============================================

.. code-block:: console

   west build -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
       -S ethos-u55-enable \
       alif/samples/modules/executorch/kws_ethosu/ \
       -p always -- \
       -DET_PTE_FILE_PATH=./kws_u55_128.pte \
       -DET_PTE_SECTION=.rodata.model \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Building for Alif B1 DK (HE Core with U55-128)
==============================================

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
       -S ethos-u55-enable \
       alif/samples/modules/executorch/kws_ethosu/ \
       -p always -- \
       -DET_PTE_FILE_PATH=./kws_u55_128.pte \
       -DET_PTE_SECTION=.rodata.model \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Building for Alif E8 DK (APSS Core, Cortex-A32, with U85)
==========================================================

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/apss \
       -S ethos-u85-apss-enable \
       alif/samples/modules/executorch/kws_ethosu/ \
       -p always -- \
       -DET_PTE_FILE_PATH=./kws_u85_256.pte \
       -DET_PTE_SECTION=.rodata.model \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

The ``.pte`` file is the same one used for the RTSS U85 builds. Vela's system
config only models memory latency, and the NPU base addresses are supplied by
the runtime rather than baked into the model, so no APSS-specific export is
needed.

The ``ethos-u85-apss-enable`` snippet is what makes this build possible. APSS
board DTS files are standalone -- they do not include the RTSS DTSI that
declares ``ethosu1`` and ``sram1`` -- so the snippet adds both: the Ethos-U85
node with GIC interrupt routing (``GIC_SPI 355``), and the SRAM1 memory region
used for the NPU-visible buffers. Building for APSS without it fails with a
message pointing at the snippet.

APSS Core Notes (Cortex-A32)
****************************

The A32 port differs from the Cortex-M55 builds in four ways, all of which are
handled automatically by the sample.

**Coherency comes from the memory type, not cache maintenance.** On RTSS, Alif's
Ethos-U callbacks (``modules/hal/alif/drivers/ethos_u``) clean and invalidate the
D-cache around each inference, and ``ethosu_address_remap()`` rewrites DTCM
addresses to the global alias the NPU can reach. Those callbacks are compiled
for Cortex-M only, so on APSS the core driver's weak no-op cache hooks stay in
place. The buffers the NPU reads and writes -- the Ethos-U scratch carved out of
the temp allocator, and the fast-scratch region -- are therefore linked into
SRAM1, which ``soc/alif/ensemble/common/mmu_regions.c`` maps as non-cacheable
normal memory. This matches the ``NPU_MEM_ATTR_*`` settings that
``zephyr/modules/hal_ethos_u`` programs for U85.

**There is no DTCM or ITCM.** The method allocator pool, which only the CPU
touches, lands in ``.bss``; on APSS the linker's RAM region already *is* SRAM0,
so no explicit section attribute is needed. The RTSS ``.alif_sram0`` output
section does not exist here at all, because it is defined in
``soc/alif/ensemble/common/sram.ld``, which only the RTSS linker script pulls in.

**There is no FPU.** ``CONFIG_FPU`` depends on ``CPU_HAS_FPU``, which Cortex-A32
does not select in this tree, so the sample enables it from its own ``Kconfig``
with ``configdefault`` instead of ``prj.conf``. Assigning it unconditionally
would abort the APSS build, since Zephyr promotes unsatisfied Kconfig
assignments to errors.

**The two quantize kernels have to be registered by the sample.** The ``.pte``
is core-agnostic, but the kernel registry is not. ``aot_arm_compiler`` runs
``ReplaceQuantNodesPass`` on every Ethos-U export, so the int8 boundary around
the delegated subgraph is emitted as ``cortex_m::quantize_per_tensor.out`` and
``cortex_m::dequantize_per_tensor.out`` rather than the ``quantized_decomposed``
equivalents. On RTSS these come from ExecuTorch's ``cortex_m`` backend, which
the Zephyr module only builds when ``CONFIG_CPU_CORTEX_M`` is set, because most
of that backend's kernels wrap CMSIS-NN. Without them ``Method::load()`` fails
with ``Missing operator`` (``Error::NotFound``, ``0x14``).

These two kernels are the backend's only CMSIS-NN-free ones: plain C++ with a
Helium fast path behind ``__ARM_FEATURE_MVE`` and a scalar fallback otherwise.
The sample's ``CMakeLists.txt`` therefore compiles just those two from the
backend's own sources on APSS and generates a registration lib for them,
selecting operators from the model so only what the ``.pte`` actually calls is
registered. The same ``.pte`` consequently runs on both core types, and a model
that needs any *other* ``cortex_m`` operator fails at configure time with an
explicit message instead of at method load on the target.

One further difference is worth knowing about if you extend the sample: because
APSS enables the MMU, Zephyr defaults ``COMMON_LIBC_MALLOC_ARENA_SIZE`` to 16KB
rather than the "all remaining RAM" that the RTSS builds get. That is far more
than this application needs -- it only ever heap-allocates a few small
``std::vector`` objects, since ExecuTorch itself allocates from the method and
temp pools -- but a model with substantial CPU-side (non-delegated) work may
need ``CONFIG_COMMON_LIBC_MALLOC_ARENA_SIZE`` raised.

Resulting memory layout on ``alif_e8_dk/ae822fa0e5597xx0/apss``:

+---------------------------+--------------+--------------------------------+
| Buffer                    | Region       | Cacheability                   |
+===========================+==============+================================+
| Model ``.pte``            | MRAM (XIP)   | Cacheable, read-only           |
+---------------------------+--------------+--------------------------------+
| Method allocator pool     | SRAM0        | Cacheable (CPU only)           |
+---------------------------+--------------+--------------------------------+
| Ethos-U scratch (temp)    | SRAM1        | **Non-cacheable** (CPU + NPU)  |
+---------------------------+--------------+--------------------------------+
| Ethos-U fast scratch      | SRAM1        | **Non-cacheable** (CPU + NPU)  |
+---------------------------+--------------+--------------------------------+

The application logs the address and size of all three pools at startup, so the
placement can be confirmed on the console before the first inference.

Flashing
********

.. code-block:: console

   west flash

APSS images boot through TF-A, so the A32 build additionally needs the BL32
binary to be flashed alongside the application:

.. code-block:: console

   west flash --bl32-bin /path/to/bl32_e8.bin

Sample Output
*************

.. code-block:: console

   *** Booting Zephyr OS build 3220a641d125 ***

   ========================================
   executorch Keyword Spotting Demo
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

- `executorch Documentation <https://pytorch.org/executorch/>`_
- `Arm Ethos-U NPU <https://developer.arm.com/ip-products/processors/machine-learning/arm-ethos-u>`_
- `Vela Compiler <https://github.com/nxp-imx/ethos-u-vela>`_
- `Alif Semiconductor <https://alifsemi.com>`_
- `Google Speech Commands Dataset <https://arxiv.org/abs/1804.03209>`_
- `DS-CNN Paper <https://arxiv.org/abs/1711.07128>`_
