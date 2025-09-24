.. _appnote-zas-ethos-u85:

==============
Ethos-U85 NPU
==============

Introduction
============

Alif processors integrate dual-core Arm® Cortex®-A32 Application Processor cores and Cortex-M55 Real-Time Processor cores, implementing the Arm v8.1 instruction set with Helium M-Profile Vector Extension (MVE). Each Real-Time Processor core pairs with an Arm Ethos™-U55 microNPU, while the Arm Ethos-U85 NPU accelerator enhances AI inference efficiency alongside the Cortex-M55.

The Ethos-U85 delivers high-performance edge AI with transformer-based model support and 20% better energy efficiency than prior Ethos NPUs. Key features include:

- **Performance**: 256 GOP/s to 4 TOPs at 1 GHz
- **Scalability**: 128 to 2048 MACs
- **Energy Efficiency**: 20% lower than previous Ethos-U NPUs
- **Transformer Support**: Native compatibility

**Real-Time Processor Cores**:

- **RTSS-HP (Cortex-M55)**: Up to 400 MHz
- **RTSS-HE (Cortex-M55)**: Up to 160 MHz

.. note::
   Refer to the `Arm Ethos-U85 documentation <https://www.arm.com/products/silicon-ip-cpu/ethos/ethos-u85>`_ for detailed specifications.

Hardware Requirements
=====================

- Alif Ensemble DevKit (E4 or E8)
- USB cable (x1)

Software Requirements
=====================

- Terminal emulator (e.g., Tera Term)
- SE tool (for MRAM flashing)
- Alif Zephyr SDK: `GitHub <https://github.com/alifsemi/sdk-alif_1.4>`_

.. note::
   Disconnect the debugger during execution to enable the OFF state.

Setup
=====

The Alif Zephyr release builds the ``tflm_ethosu`` application for HE and HP M55 cores, using a Vela-compiled model (Ethos-U85, 256 MACs) embedded as a C array. The application verifies NPU functionality.

Building with GCC
-----------------

Build the ZAS Ethos application using the Alif Zephyr SDK. The instructions target E4/E8 DevKits; adapt for other DevKits as needed.

1. **Fetch the Alif Zephyr SDK**:

   .. code-block:: bash

      mkdir -p /home/$USER/Zephyr-Ensemble-E7-B0-RTSS-v0.5.0-Beta/sdk-alif
      cd /home/$USER/Zephyr-Ensemble-E7-B0-RTSS-v0.5.0-Beta/sdk-alif
      west init -m https://github.com/AlifSemiDev/sdk-alif.git --mr main
      west config manifest.project-filter -- +tflite-micro
      west update

2. **Navigate to the Zephyr directory**:

   .. code-block:: bash

      cd zephyr

3. **Clear the build directory**:

   .. code-block:: bash

      rm -rf build

4. **Build for HE or HP core**:

   **HE Core**:

   .. code-block:: bash

      west build -b alif_e4_dk_rtss_he/alif_e8_dk_rtss_he ../alif/samples/modules/tflite-micro/tflm_ethosu/ -p always -- -G"Unix Makefiles" -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256 -DCONFIG_SRAM_BASE_ADDRESS=0x02000000 -DCONFIG_SRAM_SIZE=4096

   **HP Core**:

   .. code-block:: bash

      west build -b alif_e4_dk_rtss_hp/alif_e8_dk_rtss_hp ../alif/samples/modules/tflite-micro/tflm_ethosu/ -p always -- -G"Unix Makefiles" -DETHOSU_TARGET_NPU_CONFIG=ethos-u85-256 -DCONFIG_SRAM_BASE_ADDRESS=0x02000000 -DCONFIG_SRAM_SIZE=4096

5. **Alternative build with Ninja**:

   .. code-block:: bash

      west build -b alif_e4_dk_rtss_he/alif_e8_dk_rtss_he samples/modules/tflite-micro/tflm_ethosu/
      west build -b alif_e4_dk_rtss_hp/alif_e8_dk_rtss_hp samples/modules/tflite-micro/tflm_ethosu/

Using the Application
====================

The application performs TFLite model inferencing on the Ethos-U85 for supported operators and on the M55 for others, using reference kernels, on the Alif Ensemble DevKit.

Limitations
===========

- Untested with ArmClang or open-source Clang.
- Supports only 256 MACs; other configurations require additional integration.

JSON Configuration Files
========================

For MRAM flashing:

- **RTSS-HE**: `zephyr_rtss_mram_he.json <http://10.10.10.28/QA/SE_TOOLS/json_files/zephyr_b1/zephyr_rtss_mram_he.json>`_
- **RTSS-HP**: `zephyr_rtss_mram_hp.json <http://10.10.10.28/QA/SE_TOOLS/json_files/zephyr_b1/zephyr_rtss_mram_hp.json>`_

Flashing the Binary
===================

1. Copy the binary (e.g., ``zephyr_e7_rtsshe_ethosu.bin`` or ``zephyr_e7_rtsshp_ethosu.bin``) and the corresponding JSON file to the SE tool directory.
2. Flash to MRAM using the SE tool:

   .. code-block:: bash

      python3 app-gen-toc.py
      python3 app-write-mram.py

3. Disconnect the debugger.
4. Reset the DevKit to run the application.

Sample Output
=============

.. code-block:: text
   :class: no-copy

   [00:00:00.000,000] <dbg> ethos_u: ethosu_zephyr_init: Ethos-U DTS info. base_address=0x400e1000, secure_enable=1, privilege_enable=1
   [00:00:00.003,000] <dbg> ethos_u: ethosu_zephyr_init: Version. major=0, minor=16, patch=0
   I: Initializing NPU: base_address=0x400e1000, fast_memory=0x0, fast_memory_size=0, secure=1, privileged=1
   E: Failed to initialize device. Driver has not been compiled for this product (ethosu_device_u85.c:96)
   E: Failed to initialize Ethos-U device (ethosu_driver.c:453)
   [00:00:00.012,000] <err> ethos_u: Failed to initialize NPU with ethosu_init().
   [00:00:00.014,000] <dbg> ethos_u: ethosu_zephyr_init: Ethos-U DTS info. base_address=0x49042000, secure_enable=1, privilege_enable=1
   [00:00:00.017,000] <dbg> ethos_u: ethosu_zephyr_init: Version. major=0, minor=16, patch=0
   I: Initializing NPU: base_address=0x49042000, fast_memory=0x0, fast_memory_size=0, secure=1, privileged=1
   I: Soft reset NPU
   I: New NPU driver registered (handle: 0x21e8ea0, NPU: 0x49042000)
   *** Booting Zephyr OS build 5dee35a0f25a ***
   sender 0: Sending inference. job=0x21ea8f0, name=keyword_spotting_cnn_small_int8
   runner 0: Received inference job. job=0x21ea8f0
   I: Acquiring NPU driver handle
   D: ethosu_reserve_driver(): NPU driver handle 0x21e8ea0 reserved
   D: ethosu_invoke_async(): OPTIMIZER_CONFIG
   I: Optimizer release nbr: 0 patch: 1
   I: Optimizer config. product=2, cmd_stream_version=1, macs_per_cc=8, num_axi_ext=1, num_axi_sram=2, custom_dma=0
   I: Optimizer config. arch version=2.0.0
   I: Ethos-U config. product=2, cmd_stream_version=1, macs_per_cc=8, num_axi_ext=1, num_axi_sram=2, custom_dma=0
   I: Ethos-U. arch version=2.0.0
   D: ethosu_invoke_async(): NOP
   D: ethosu_invoke_async(): NOP
   D: ethosu_invoke_async(): NOP
   D: ethosu_invoke_async(): COMMAND_STREAM
   I: handle_command_stream: cmd_stream=0x80026a30, cms_length 1029
   I: Soft reset NPU
   D: ethosu_dev_set_clock_and_power(): CMD=0x00000000
   D: ethosu_dev_run_command_stream(): QBASE=0x0000000080026a30, QSIZE=4116, cmd_stream_ptr=0x80026a30
   D: ethosu_dev_run_command_stream(): BASEP0=0x00000000800153d0
   D: ethosu_dev_run_command_stream(): BASEP1=0x0000000002000a20
   D: ethosu_dev_run_command_stream(): BASEP2=0x0000000002000a20
   D: ethosu_dev_run_command_stream(): BASEP3=0x000000000200c120
   D: ethosu_dev_run_command_stream(): BASEP4=0x0000000002000aa0
   D: ethosu_dev_run_command_stream(): CMD=0x00000001
   D: ethosu_dev_set_clock_and_power(): CMD=0x0000000c
   D: ethosu_wait(): Inference finished successfully...
   D: ethosu_release_driver(): NPU driver handle 0x21e8ea0 released
   runner 0: Sending inference response. job=0x21ea8f0
   [Output continues with similar inference cycles...]
   sender 1: Received job response. job=0x21eb144, status=0
   exit