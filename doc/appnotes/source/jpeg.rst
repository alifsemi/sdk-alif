.. _jpeg:

============
JPEG Encoder
============

Introduction
============

This application note describes how to use the VeriSilicon Hantro VC9000E hardware JPEG encoder
on the Alif Ensemble SoC. Two sample applications are provided:

- **Standalone JPEG test**: Encodes a static NV12 test image embedded in the binary.
- **Video + JPEG pipeline**: Captures live camera frames via ISP and encodes them to JPEG in real time.

**Image data path (standalone)**:
Embedded NV12 image → JPEG Encoder → Compressed JPEG in memory

**Image data path (video pipeline)**:
CMOS sensor → CSI → CPI → ISP → NV12 frame → JPEG Encoder → Compressed JPEG in memory

Hardware Requirements
======================

VeriSilicon Hantro VC9000E JPEG Encoder
-----------------------------------------

The VC9000E is a hardware JPEG encoder IP integrated into the Alif Ensemble SoC.
Key features include:

- **Input formats**: YUV420 semi-planar (NV12, NV21)
- **Configurable quality factor**: Adjustable from 1 to 100
- **Hardware quantization**: Quantization tables programmed directly into encoder registers
- **AXI DMA**: Configurable burst length and outstanding transaction limits
- **Output**: Baseline JPEG bitstream (software-generated header + hardware-encoded scan data)

For the video pipeline mode, the following additional hardware is required:

- **camera sensor** connected via MIPI CSI-2
- **ISP** configured for NV12 output

Required Configuration
=======================

Standalone JPEG Test
---------------------

.. code-block:: kconfig

   CONFIG_VIDEO=y
   CONFIG_VIDEO_JPEG_HANTRO_VC9000E=y
   CONFIG_USE_ALIF_JPEG_SW_LIB=y

Video Pipeline with JPEG
--------------------------

.. code-block:: kconfig

   CONFIG_VIDEO=y
   CONFIG_VIDEO_MIPI_CSI2_DW=y
   CONFIG_VIDEO_JPEG_HANTRO_VC9000E=y
   CONFIG_USE_ALIF_JPEG_SW_LIB=y
   CONFIG_USE_ALIF_ISP_LIB=y
   CONFIG_ISP_LIB_SCALAR_MODULE=y
   CONFIG_ISP_LIB_DMSC_MODULE=y

.. include:: note.rst

Building the JPEG Application
================================

Follow these steps to build the JPEG application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository,
   please refer to the `ZAS User Guide`_

.. note::
   The build commands shown here are specifically for the Alif E8 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly.
   For more information, refer to the `ZAS User Guide`_, under the section Setting Up and Building Zephyr Applications.

Standalone Static Image Test
------------------------------

This application encodes an embedded 1280×720 NV12 test image using the JPEG encoder.

Build command for the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/jpeg/ \
     -S alif-dk-ak \

Video Pipeline with JPEG Encoding
-----------------------------------

This application captures live arx3a0_selfie camera frames
via ISP and encodes each frame to JPEG.

Build command for the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/video/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/video/boards/serial_camera_arx3a0_selfie.overlay \
       $PWD/../alif/samples/drivers/video/boards/jpeg.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/video/boards/isp.conf \
       $PWD/../alif/samples/drivers/video/boards/jpeg.conf"

Executing Binary on the DevKit
===============================

Standalone JPEG Test
---------------------

The standalone static image application is MRAM-bootable. Flash and execute using SE Tools:

.. code-block:: bash

   west flash

For detailed SE Tools flashing instructions, refer to the `ZAS User Guide`_.

Video Pipeline with JPEG
--------------------------

The video pipeline application is TCM-bootable. Use an Arm debugger (e.g., ULINKpro) to load and execute:

1. Connect the ULINKpro debugger to the DevKit.
2. Load the ELF file using Arm Development Studio or a compatible debug environment.
3. Start execution from the debugger.

Verification
=============

Once encoding completes successfully, the application logs provide:

- The **memory address** and **size** of the compressed JPEG output buffer.
- A ``dump binary memory`` command that can be used directly in the debugger console
  to save the JPEG data to a file on the host machine.

Use the logged command to dump the encoded JPEG image and verify it by opening the resulting
``.bin`` file with any standard image viewer (rename to ``.jpg`` if needed).

.. _ZAS User Guide: https://alifsemi.com/zas-user-guide
