.. _tflm_ethosu:

Arm(R) Ethos(TM)-U TensorFlow Lite Micro Application
#####################################################

Overview
********

ML inference application using TensorFlow Lite Micro (TFLM) with Arm Ethos-U NPU acceleration.
Runs keyword spotting CNN model optimized with Vela compiler.

Supported Boards
****************

+-------+--------+---------+---------+-------------------------+
| Board | U55    | U85     | APSS    | Notes                   |
+=======+========+=========+=========+=========================+
| B1    | Yes    | No      | No      | U55 only                |
+-------+--------+---------+---------+-------------------------+
| E1C   | Yes    | No      | No      | U55 only                |
+-------+--------+---------+---------+-------------------------+
| E3    | Yes    | No      | No      | U55 only                |
+-------+--------+---------+---------+-------------------------+
| E4    | Yes    | Yes     | No      | Dual NPU                |
+-------+--------+---------+---------+-------------------------+
| E7    | Yes    | No      | Yes     | U55 only                |
+-------+--------+---------+---------+-------------------------+
| E8    | Yes    | Yes     | Yes     | Dual NPU                |
+-------+--------+---------+---------+-------------------------+

Core Type Restrictions
**********************

- **HE cores (rtss_he)**: U55 supports 128 MACs only
- **HP cores (rtss_hp)**: U55 supports 256 MACs only
- **APSS cores (apss)**: U85 with 256 MACs only (Cortex-A32)
- **U85**: Always 256 MACs (HE, HP, and APSS cores)

Building and Running
********************

All build commands use the ``-S`` flag to apply snippets for NPU configuration.
By default this sample uses TCM/MRAM/SRAM memory and does not require external
OSPI flash. See `Running with OSPI Flash`_ for the OSPI configurations.

Building for Alif B1 DK
------------------------

**HE Core with U55-128:**

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
       -S ethos-u55-enable \
       ../alif/samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Building for Alif E1C DK
-------------------------

**HE Core with U55-128:**

.. code-block:: console

   west build -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
       -S ethos-u55-enable \
       ../alif/samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Building for Alif E7 DK
-----------------------

**HP Core with U55-256:**

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       -S ethos-u55-enable \
       ../alif/samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

**HE Core with U55-128:**

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       -S ethos-u55-enable \
       ../alif/samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Building for Alif E8 DK
-----------------------

**HP Core with U55-256:**

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       -S ethos-u55-enable \
       ../alif/samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

**HP Core with U85-256:**

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       -S ethos-u85-enable \
       ../alif/samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

**HE Core with U55-128:**

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       -S ethos-u55-enable \
       ../alif/samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

**APSS Core (Cortex-A32) with U85-256:**

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/apss \
       ../alif/samples/modules/tflite-micro/tflm_ethosu \
       -p always \
       -S ethos-u85-apss-enable \
       -- -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

.. note::

   The ``ethos-u85-apss-enable`` snippet defines the Ethos-U85 NPU node with
   GIC interrupt routing (GIC_SPI 355) and the SRAM1 memory region for the
   non-cacheable tensor arena. Using a snippet is the Zephyr-idiomatic approach
   and is consistent with how ``ethos-u55-enable`` and ``ethos-u85-enable`` work
   for RTSS cores. The tensor arena is placed in SRAM1 to ensure cache coherency
   with the NPU without explicit cache maintenance.

Running with OSPI Flash
***********************

The Secure Enclave boots the whole image from OSPI1 NOR flash. The model is
part of that image. Prepare the workspace as described in
:ref:`appnote-zephyr-hello-world-ospi` (MPU related changes) and link the
image to the OSPI1 XIP window:

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       -S ethos-u55-enable \
       ../alif/samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256 \
       -DCONFIG_FLASH_BASE_ADDRESS=0xC0000000 \
       -DCONFIG_FLASH_LOAD_OFFSET=0x0

For RTSS-HE use ``-DCONFIG_FLASH_BASE_ADDRESS=0xC0200000`` and
``-DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128``.

When the image is linked into an OSPI XIP window,
``CONFIG_TFLM_ETHOSU_MODEL_COPY_TO_RAM`` is enabled automatically: the model is
copied to DTCM at startup and the NPU only reads the RAM copy. NPU accesses
to the OSPI XIP window set up by the boot firmware fail with
``NPU status=0x00000010`` (command stream parse error). The console shows the
copy at startup:

.. code-block:: console

   Model copied to RAM: 0xc0014bf0 -> 0x20000a60 (76288 bytes)

Configuration Options
*********************

**NPU Configuration:**

- ``ETHOSU_TARGET_NPU_CONFIG``: ethos-u55-128, ethos-u55-256, or ethos-u85-256

**Snippets:**

- ``-S ethos-u55-enable``: Enable U55 NPU on RTSS cores (all boards)
- ``-S ethos-u85-enable``: Enable U85 NPU on RTSS cores (E4/E8 only)
- ``-S ethos-u85-apss-enable``: Enable U85 NPU on APSS (Cortex-A32) core (E8 only)

Snippets automatically apply the necessary device tree overlays to enable the NPU.

**Memory Placement:**

- ``CONFIG_TFLM_ETHOSU_MODEL_COPY_TO_RAM``: Copy the model to RAM before inference
  (default ``y`` when the image executes from OSPI)
- ``CONFIG_TFLM_ETHOSU_MODEL_RAM_SECTION``: Linker section of the RAM copy
  (default ``.bss.tflm_model_ram``)

**Performance Tuning:**

- ``NUM_INFERENCE_TASKS``: Worker threads (default: 1)
- ``NUM_JOB_TASKS``: Sender tasks (default: 2)
- ``NUM_JOBS_PER_TASK``: Inferences per task (default: 2)

Flashing
********

.. code-block:: console

   west flash
