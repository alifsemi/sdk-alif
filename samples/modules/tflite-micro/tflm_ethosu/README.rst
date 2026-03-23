.. _tflm_ethosu:

Arm(R) Ethos(TM)-U TensorFlow Lite Micro Application
#####################################################

Overview
********

ML inference application using TensorFlow Lite Micro (TFLM) with Arm Ethos-U NPU acceleration.
Runs keyword spotting CNN model optimized with Vela compiler.

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

Building for Alif E3 DK
-----------------------

**HP Core with U55-256:**

.. code-block:: console

   west build -b alif_e3_dk/ae302f80f55d5xx/rtss_hp \
       -S ethos-u55-enable \
       samples/modules/tflite-micro/tflm_ethosu \
       -p always -- \
       -DETHOSU_TARGET_NPU_CONFIG=ethos-u55-256

**HE Core with U55-128:**

.. code-block:: console

   west build -b alif_e3_dk/ae302f80f55d5xx/rtss_he \
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

Configuration Options
*********************

**NPU Configuration:**

- ``ETHOSU_TARGET_NPU_CONFIG``: ethos-u55-128, ethos-u55-256, or ethos-u85-256

**Snippets:**

- ``-S ethos-u55-enable``: Enable U55 NPU (all boards)
- ``-S ethos-u85-enable``: Enable U85 NPU (E4/E8 only)

Snippets automatically apply the necessary device tree overlays to enable the NPU.

**Performance Tuning:**

- ``NUM_INFERENCE_TASKS``: Worker threads (default: 1)
- ``NUM_JOB_TASKS``: Sender tasks (default: 2)
- ``NUM_JOBS_PER_TASK``: Inferences per task (default: 2)

Flashing
********

.. code-block:: console

   west flash
