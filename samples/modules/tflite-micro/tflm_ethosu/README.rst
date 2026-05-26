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
| E4    | Yes    | Yes     | Yes     | Dual NPU                |
+-------+--------+---------+---------+-------------------------+
| E7    | Yes    | No      | Yes     | U55 only (no NPU on A32)|
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
This sample uses TCM/MRAM/SRAM memory and does not require external OSPI flash.

Building for Alif B1 DK
------------------------

**HP Core with U55-256:**

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820hh0/rtss_hp \
       -S ethos-u55-enable \
       samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

**HE Core with U55-128:**

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820hh0/rtss_he \
       -S ethos-u55-enable \
       samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Building for Alif E1C DK
-------------------------

**HE Core with U55-128:**

.. code-block:: console

   west build -b alif_e1c_dk/ae1c1f4051920hh/rtss_he \
       -S ethos-u55-enable \
       samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Building for Alif E7 DK
-----------------------

**HP Core with U55-256:**

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       -S ethos-u55-enable \
       samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

**HE Core with U55-128:**

.. code-block:: console

   west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       -S ethos-u55-enable \
       samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

Building for Alif E8 DK
-----------------------

**HP Core with U55-256:**

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       -S ethos-u55-enable \
       samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

**HP Core with U85-256:**

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       -S ethos-u85-enable \
       samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256

**HE Core with U55-128:**

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       -S ethos-u55-enable \
       samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-128

**APSS Core (Cortex-A32) with U85-256:**

.. code-block:: console

   west build -b alif_e8_dk/ae822fa0e5597xx0/apss \
       samples/modules/tflite-micro/tflm_ethosu \
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

Configuration Options
*********************

**NPU Configuration:**

- ``ETHOSU_TARGET_NPU_CONFIG``: ethos-u55-128, ethos-u55-256, or ethos-u85-256

**Snippets:**

- ``-S ethos-u55-enable``: Enable U55 NPU on RTSS cores (all boards)
- ``-S ethos-u85-enable``: Enable U85 NPU on RTSS cores (E4/E8 only)
- ``-S ethos-u85-apss-enable``: Enable U85 NPU on APSS (Cortex-A32) core (E4/E8 only)

Snippets automatically apply the necessary device tree overlays to enable the NPU.

**Performance Tuning:**

- ``NUM_INFERENCE_TASKS``: Worker threads (default: 1)
- ``NUM_JOB_TASKS``: Sender tasks (default: 2)
- ``NUM_JOBS_PER_TASK``: Inferences per task (default: 2)

Flashing
********

.. code-block:: console

   west flash
