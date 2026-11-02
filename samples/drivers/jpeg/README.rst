.. _jpeg_test:

JPEG Encoder Test
##################

Overview
********

This sample demonstrates the use of the Alif JPEG hardware encoder driver.
It encodes an embedded 1280x720 NV12 image. One test is selected by a
``TEST_JPEG_*`` macro at the top of ``src/main.c``. Set the test under
exercise to ``1`` and the others to ``0``, then rebuild.

The default build is basic encoding at quality 10. The log below is from
that build. The other tests are described after it.

Requirements
************

* Alif Ensemble E8 Development/Application Kit
* JPEG hardware encoder peripheral

Building and Running
********************

The application will build only for a target that has a devicetree entry
with :dt compatible:`verisilicon,hantro-vc9000e-jpeg` as a compatible.
It does not work on QEMU.
In this example below the :ref:`alif_e8_dk/ae822fa0e5597xx0/rtss_hp` board is used.

Use the snippet "alif-dk-ak" to build jpeg app on all alif boards.

.. zephyr-app-commands::

   :zephyr-app: ../alif/samples/drivers/jpeg
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :gen-args: -S alif-dk-ak

Sample Output
*************

.. code-block:: console

   [00:00:00.001,000] <inf> jpeg_hantro_vc9000e: VeriSilicon Hantro VC9000E JPEG encoder initialized
   [00:00:00.001,000] <inf> jpeg_test: === VeriSilicon Hantro VC9000E JPEG Encoder Test ===
   [00:00:00.001,000] <inf> jpeg_test: JPEG device ready: jpeg@49044000
   [00:00:00.001,000] <inf> jpeg_test: Allocated input buffer at 0x2000068 with (1382400 bytes)

   [00:00:00.018,000] <inf> jpeg_test: Allocated output buffer at 0x2151870 with (691823 bytes)

   [00:00:00.018,000] <inf> jpeg_test: JPEG Encoder Capabilities:
   [00:00:00.018,000] <inf> jpeg_test:   Format: 0x3231564e, Size: 32x32 to 16384x16384
   [00:00:00.018,000] <inf> jpeg_test:   Format: 0x3132564e, Size: 32x32 to 16384x16384
   [00:00:00.018,000] <inf> jpeg_test: Starting JPEG encoding test...
   [00:00:00.018,000] <inf> jpeg_test: Format: 1280x720 NV12, quality=10
   [00:00:00.021,000] <inf> jpeg_test: === JPEG Encoding Results ===
   [00:00:00.021,000] <inf> jpeg_test: Input size:  1382400 bytes (1280x720 YUV420)
   [00:00:00.021,000] <inf> jpeg_test: Output size: 30840 bytes (JPEG)
   [00:00:00.021,000] <inf> jpeg_test: Compression ratio: 44.82:1
   [00:00:00.021,000] <inf> jpeg_test: JPEG header verified (SOI marker found)
   [00:00:00.021,000] <inf> jpeg_test: Jpeg: Capture Image: dump memory "/home/$USER/capture_img_q10.jpg" 0x02151870 0x021590e8

   [00:00:00.021,000] <inf> jpeg_test: Test completed successfully!
   [00:00:00.521,000] <inf> jpeg_test: === Jpeg Test Completed Successfully ===

Other encode tests
******************

Each test logs a debugger dump command. The file name is fixed. The
buffer address depends on the link.

RGB24 thumbnail
===============

``TEST_JPEG_THUMB_RGB24`` embeds a 64x48 JFIF RGB24 thumbnail in the
JPEG header.

.. code-block:: console

   <inf> jpeg_test: === Testing JPEG Encoding with RGB24 Thumbnail ===
   <inf> jpeg_test: Thumbnail prepared: 64x48 RGB24 (9216 bytes)
   <inf> jpeg_test: Jpeg: Capture Image: dump memory "/home/$USER/capture_thumb_rgb24.jpg" ...

JPEG thumbnail
==============

``TEST_JPEG_THUMB_JPEG`` encodes a 64x48 thumbnail first, then embeds
that JPEG in the full-frame header (JFXX).

.. code-block:: console

   <inf> jpeg_test: === Testing JPEG Encoding with JPEG Thumbnail (two-pass) ===
   <inf> jpeg_test: Pass 1: Encoding 64x48 thumbnail...
   <inf> jpeg_test: Pass 2: Encoding 1280x720 with JPEG thumbnail...
   <inf> jpeg_test: Jpeg: Capture Image: dump memory "/home/$USER/capture_thumb_jpeg.jpg" ...

Palette thumbnail
=================

``TEST_JPEG_THUMB_PALETTE`` embeds a 64x48 JFXX palette thumbnail.

.. code-block:: console

   <inf> jpeg_test: === Testing JPEG Encoding with Palette Thumbnail ===
   <inf> jpeg_test: Jpeg: Capture Image: dump memory "/home/$USER/capture_thumb_palette.jpg" ...

Quality levels
==============

``TEST_JPEG_QUALITY_LEVELS`` encodes the same frame at qualities 25, 50,
75, and 100 in one stream. The dumps are ``capture_img_q25.jpg``,
``capture_img_q50.jpg``, ``capture_img_q75.jpg``, and
``capture_img_q100.jpg``.

.. code-block:: console

   <inf> jpeg_test: === Testing Multiple Quality Levels ===
   <inf> jpeg_test: Quality, Size(bytes), Ratio

Hardware crop
=============

``TEST_JPEG_CROPPING`` encodes the center 640x360 window. The source
format stays 1280x720. The crop origin is x=320, y=180.

.. code-block:: console

   <inf> jpeg_test: === Testing JPEG Encoding with Hardware Crop ===
   <inf> jpeg_test: Crop: x=320 y=180 w=640 h=360
   <inf> jpeg_test: Output JPEG should be 640x360 center crop
   <inf> jpeg_test: Jpeg: Capture Image: dump memory "/home/$USER/capture_crop.jpg" ...

Frame slice
===========

``TEST_JPEG_SLICE`` encodes the frame in bands of 4 MCU rows (64 pixels).
A 720-line picture is 12 slices, and the last slice is 16 pixels. The
slices are concatenated into one JPEG. Restart markers ``FFD0`` through
``FFD7`` mark the slice boundaries.

.. code-block:: console

   <inf> jpeg_test: === Starting JPEG Slice-Mode Test ===
   <inf> jpeg_test: Format: 1280x720 NV12, slice height=64 px (4 MCU rows), total slices=12
   <inf> jpeg_test: Jpeg: Capture Slice JPEG: dump memory "/home/$USER/capture_slice.jpg" ...
   <inf> jpeg_test: Play with: ffplay capture_slice.jpg

MJPEG concat
============

``TEST_JPEG_MJPEG`` encodes 30 frames at quality 10 and concatenates
them. A white band steps down the picture so the stream shows motion.
Each frame is trimmed at the EOI marker before it is appended.

.. code-block:: console

   <inf> jpeg_test: === Testing MJPEG Concat (.mjpg) Encoding ===
   <inf> jpeg_test: Format: 1280x720 NV12, quality=10, frames=30, fps=30
   <inf> jpeg_test: Jpeg: Capture MJPEG: dump memory "/home/$USER/capture_video.mjpg" ...
   <inf> jpeg_test: Play with:  ffplay -framerate 30 capture_video.mjpg

MJPEG AVI
=========

``TEST_JPEG_AVI`` encodes the same 30 frames and muxes them into one AVI.

.. code-block:: console

   <inf> jpeg_test: === Testing MJPEG-in-AVI (.avi) Encoding ===
   <inf> jpeg_test: Format: 1280x720 NV12, quality=10, frames=30, fps=30
   <inf> jpeg_test: Jpeg: Capture AVI: dump memory "/home/$USER/capture_video.avi" ...
   <inf> jpeg_test: Play with:  ffplay capture_video.avi
