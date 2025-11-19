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

Building for Alif B1 DK (HP Core with U55)
------------------------------------------

.. zephyr-app-commands::
   :zephyr-app: samples/modules/tflite-micro/tflm_ethosu/
   :board: alif_b1_dk/ab1c1f1m41820xx0/rtss_hp
   :goals: build
   :gen-args: -DDTC_OVERLAY_FILE="boards/enable_ethosu55.overlay"

Building for Alif E1C DK (HP Core with U55)
-------------------------------------------

.. zephyr-app-commands::
   :zephyr-app: samples/modules/tflite-micro/tflm_ethosu/
   :board: alif_e1c_dk/ae510f0agv81xx0/rtss_hp
   :goals: build
   :gen-args: -DDTC_OVERLAY_FILE="boards/enable_ethosu55.overlay"

Building for Alif E7 DK (HP Core with U55)
------------------------------------------

.. zephyr-app-commands::
   :zephyr-app: samples/modules/tflite-micro/tflm_ethosu/
   :board: alif_e7_dk/ae722f80f55d5xx0/rtss_hp
   :goals: build
   :gen-args: -DDTC_OVERLAY_FILE="boards/enable_ethosu55.overlay"

Building for Alif E8 DK (HP Core with U85)
------------------------------------------

.. zephyr-app-commands::
   :zephyr-app: samples/modules/tflite-micro/tflm_ethosu/
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :gen-args: -DDTC_OVERLAY_FILE="boards/enable_ethosu85.overlay" -DCONFIG_ETHOSU_TARGET_NPU_CONFIG=\"ethos-u85-256\"

Configuration Options
*********************

**NPU Configuration:**

- ``ETHOSU_TARGET_NPU_CONFIG``: ethos-u55-128, ethos-u55-256, or ethos-u85-256

**Overlay Files:**

- ``boards/enable_ethosu55.overlay``: Enable U55 NPU
- ``boards/enable_ethosu85.overlay``: Enable U85 NPU (E4/E8 only)

**Performance Tuning:**

- ``NUM_INFERENCE_TASKS``: Worker threads (default: 1)
- ``NUM_JOB_TASKS``: Sender tasks (default: 2)
- ``NUM_JOBS_PER_TASK``: Inferences per task (default: 2)
