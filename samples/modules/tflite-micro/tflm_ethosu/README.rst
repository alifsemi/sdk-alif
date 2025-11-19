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

Build Notes
***********

Refer to the SDK User Guide for build and flash instructions.

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
