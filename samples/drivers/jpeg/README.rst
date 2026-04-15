.. _jpeg_test:

JPEG Encoder Test
##################

Overview
********

This sample demonstrates the use of the Alif JPEG hardware encoder driver.
It performs the following tests:

1. Basic JPEG encoding with a generated test pattern
2. Multiple quality level testing (10, 25, 50, 75, 90, 100)
3. Compression ratio

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

  [00:00:00.001,000] <inf> jpeg_test: Allocated output buffer at 0x2151870 with (691823 bytes)

  [00:00:00.001,000] <inf> jpeg_test: JPEG Encoder Capabilities:
  [00:00:00.001,000] <inf> jpeg_test:   Format: 0x3231564e, Size: 32x32 to 16384x16384
  [00:00:00.001,000] <inf> jpeg_test:   Format: 0x3132564e, Size: 32x32 to 16384x16384
  [00:00:00.001,000] <inf> jpeg_test: Starting JPEG encoding test...
  [00:00:00.001,000] <inf> jpeg_test: Format set: 1280x720, format: NV12
  [00:00:00.001,000] <inf> jpeg_test: Quality set to: 10
  [00:00:00.018,000] <inf> jpeg_test: Buffer enqueued
  [00:00:00.019,000] <inf> jpeg_test: Stream started, waiting for encoding...
  [00:00:00.021,000] <inf> jpeg_test: === JPEG Encoding Results ===
  [00:00:00.021,000] <inf> jpeg_test: Input size:  1382400 bytes (1280x720 YUV420)
  [00:00:00.021,000] <inf> jpeg_test: Output size: 30840 bytes (JPEG)
  [00:00:00.021,000] <inf> jpeg_test: Compression ratio: 44.82:1
  [00:00:00.021,000] <inf> jpeg_test: Encoding time: 3 ms
  [00:00:00.021,000] <inf> jpeg_test: JPEG header verified (SOI marker found)
  [00:00:00.021,000] <inf> jpeg_test: Jpeg: Capture Image: dump memory "/home/$USER/capture_img_q10.jpg" 0x02151870 0x021590e8

  [00:00:00.021,000] <inf> jpeg_test: Test completed successfully!
  [00:00:00.521,000] <inf> jpeg_test:
                                      === Testing Multiple Quality Levels ===
  [00:00:00.521,000] <inf> jpeg_test: Quality, Size(bytes), Ratio, Time(ms)
  [00:00:00.521,000] <inf> jpeg_test: ----------------------------------------
  [00:00:00.523,000] <inf> jpeg_test:  25,      54022,      25.59:1,     2
  [00:00:00.523,000] <inf> jpeg_test: Jpeg: Capture Image: dump memory "/home/$USER/capture_img_q25.jpg" 0x02151870 0x0215eb76

  [00:00:00.526,000] <inf> jpeg_test:  50,      81386,      16.99:1,     3
  [00:00:00.526,000] <inf> jpeg_test: Jpeg: Capture Image: dump memory "/home/$USER/capture_img_q50.jpg" 0x02151870 0x0216565a

  [00:00:00.529,000] <inf> jpeg_test:  75,     120279,      11.49:1,     3
  [00:00:00.529,000] <inf> jpeg_test: Jpeg: Capture Image: dump memory "/home/$USER/capture_img_q75.jpg" 0x02151870 0x0216ee47

  [00:00:00.531,000] <inf> jpeg_test: 100,     595949,      2.32:1,     2
  [00:00:00.531,000] <inf> jpeg_test: Jpeg: Capture Image: dump memory "/home/$USER/capture_img_q100.jpg" 0x02151870 0x021e305d

  [00:00:00.531,000] <inf> jpeg_test: Quality level test completed!
  [00:00:00.531,000] <inf> jpeg_test:
                                      === All Tests Completed Successfully ===
  [00:00:00.531,000] <inf> jpeg_test: Buffers released
