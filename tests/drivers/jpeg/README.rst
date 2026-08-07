.. _jpeg_hantro_vc9000e_test:

JPEG Driver Test
################

Overview
********

ZTest suite for the VeriSilicon Hantro VC9000E JPEG hardware encoder
driver (``verisilicon,hantro-vc9000e-jpeg``).

Each test exercises a distinct slice of the driver API; redundant or
fully-subsumed cases have been removed. Device-readiness is enforced
unconditionally by the suite-before hook and is therefore not a
separate test.

The suite is split into two groups: the **baseline** tests (which all
pass on the current driver) and the **newly added** tests, kept in a
dedicated section at the bottom of this document so they can be
identified at a glance.

Baseline tests
**************

#. Capabilities reporting (NV12 / NV21 presence, dimension / step
   bounds, EP_IN rejection).
#. ``video_set_format`` / ``video_get_format`` round-trip parametrized
   over both supported pixel formats (NV12 and NV21).
#. ``video_set_format`` rejection paths (too small, too large,
   unsupported pixel format, wrong endpoint).
#. ``VIDEO_CID_JPEG_COMPRESSION_QUALITY`` set/get round-trip and
   unsupported-CID rejection.
#. ``video_stream_start`` / ``video_stream_stop`` ``-EALREADY`` paths.
#. ``video_enqueue`` ``-EBUSY`` (single in-flight buffer constraint).
#. ``video_dequeue`` ``-EAGAIN`` on timeout (no enqueue performed).
#. Quality-level encode (q=10 + q=90): covers basic-encode sanity
   (SOI/EOI markers, ``bytesused > header_size``, ``bytesused < input``,
   compression ratio) and asserts the quality knob has the documented
   monotonic effect on output size. Uses the 1280x720 NV12 reference
   image shared with ``samples/drivers/jpeg``.
#. Multi-resolution encode (32x32, 320x240, 640x480 synthetic
   gradient) -- validates dimension handling and 16-px alignment.
#. Back-to-back encodes (5 iterations of the same buffer pair) --
   asserts byte-identical output size across iterations, proving the
   driver fully re-arms HW state between encodes.

Exporting encoded images
************************

The encode tests print **two** export hints per buffer: one for gdb
and one for J-Link Commander. Each line carries the buffer's start
address and ``bytesused`` so the captured file is a complete JPEG.

Example (output line wrapped for readability):

.. code-block:: console

   EXPORT[q=90] gdb   : dump binary memory "q90.jpg" 0x02151870 0x02181f88
   EXPORT[q=90] J-Link: savebin q90.jpg 0x02151870 0x30718

``dump binary memory`` (gdb) takes start + end addresses; ``savebin``
(J-Link Commander) takes start + size in bytes. Either command
produces a viewable ``.jpg`` file.

Requirements
************

* Alif Ensemble E8 Development/Application Kit (or any board exposing
  the ``jpeg0`` node with compatible
  ``verisilicon,hantro-vc9000e-jpeg``).
* Fixture: ``fixture_jpeg``.

Building and Running
********************

.. code-block:: console

   west build -b <board> <path>

The reference NV12 image used by the encoder tests is shared with the
JPEG sample:
``src/testing_images/1280x720.bin -> ../../../samples/drivers/jpeg/src/testing_images/1280x720.bin``.

Expected output (excerpt)
*************************

.. code-block:: console

   PASS - jpeg_hantro::test_jpeg_get_caps
   PASS - jpeg_hantro::test_jpeg_set_format_valid
   PASS - jpeg_hantro::test_jpeg_set_format_invalid
   PASS - jpeg_hantro::test_jpeg_set_get_ctrl_quality
   PASS - jpeg_hantro::test_jpeg_stream_already_started_stopped
   PASS - jpeg_hantro::test_jpeg_enqueue_busy
   PASS - jpeg_hantro::test_jpeg_dequeue_timeout
   PASS - jpeg_hantro::test_jpeg_encode_quality_levels
   PASS - jpeg_hantro::test_jpeg_encode_multi_resolutions
   PASS - jpeg_hantro::test_jpeg_back_to_back_encodes

----

Unique scenarios (expected to PASS)
===================================

* ``test_jpeg_pitch_with_padding`` -- NV12 with ``pitch (384) > width
  (320)``. Validates that the driver programs ROWLENGTH /
  LUMA_STRIDE / CHROMA_STRIDE from ``fmt.pitch`` (not ``fmt.width``).
* ``test_jpeg_non_aligned_dimensions`` -- 300x200 NV12 (``xfill=2``,
  ``yfill=8``). Exercises the MCU-padding paths in ``set_format``.
* ``test_jpeg_encode_nv21_end_to_end`` -- full encode pipeline run
  with ``VIDEO_PIX_FMT_NV21`` (the baseline ``set_format_valid`` test
  only verifies the format round-trip, not the encode path).
* ``test_jpeg_encode_format_switch`` -- two consecutive encodes at
  640x480 then 320x240 to prove ``set_format`` correctly re-programs
  all dimension-dependent HW registers between encodes.
* ``test_jpeg_enqueue_without_input_buffer`` -- negative path:
  enqueue with ``input_buffer == NULL`` must return ``-EINVAL``.
* ``test_jpeg_jfif_structural_validation`` -- deep marker walk:
  asserts presence of APP0/JFIF, DQT, SOF0, DHT, SOS and EOI in
  order, guaranteeing the stream is decodable.
* ``test_jpeg_encode_large_1920x1080`` -- FHD stress encode. Skipped
  automatically (``ztest_test_skip``) if
  ``CONFIG_VIDEO_BUFFER_POOL_SZ_MAX`` is below 3,200,000.
* ``test_jpeg_set_format_unsupported_hrm_formats`` -- HRM negative
  contract: asserts ``set_format`` rejects YUYV (4:2:2), GREY/Y8
  (4:0:0), RGB565 and XRGB32. These are HRM-listed IP-supported
  formats that the current Zephyr driver does not expose.
* ``test_jpeg_encode_max_width`` -- wide-stride / address-math stress
  at 8192x32 NV12 (toward the HRM-documented 16K max width).
  Auto-skipped if the buffer pool cannot accommodate it.

Performance benchmark suite (jpeg_perf)
========================================

The ``jpeg_perf`` suite contains informational performance tests that
measure encode latency and throughput. These tests have no hard
pass/fail thresholds (the numbers are advisory) but will fail if the
cycle counter is not running or encodes error out.

* ``test_jpeg_encode_performance_1280x720`` -- runs one warm-up encode
  plus 10 timed iterations at 1280x720 NV12, q=75, using
  ``k_cycle_get_32()``. Reports mean / min / max per-frame latency in
  microseconds, input-pixel throughput in MB/s, and frames per second (FPS).
* ``test_jpeg_encode_performance_1920x1080`` -- same benchmark at
  1920x1080 (2MP FHD) resolution to assess performance scaling. Uses
  synthetic gradient pattern instead of the reference image. Auto-skipped
  if the buffer pool cannot accommodate the larger buffers. Reports the
  same metrics as the 1280x720 test.

API contract validation tests
==============================

* ``test_jpeg_get_ctrl_input_buffer_roundtrip`` -- verifies that
  ``get_ctrl(VIDEO_CID_JPEG_INPUT_BUFFER)`` returns the value
  previously set via ``set_ctrl``.
* ``test_jpeg_quality_range_validation`` -- verifies that
  ``set_ctrl(VIDEO_CID_JPEG_COMPRESSION_QUALITY)`` validates range
  (1-100) and rejects values of 0 and >100 with ``-EINVAL``.
* ``test_jpeg_aa_initial_state_rejects_stream_start`` -- verifies that
  ``stream_start`` requires a prior ``set_format``; on fresh init
  ``data->fmt`` is zero (``width=0``, ``height=0``) yet ``stream_start``
  must fail. The test name is prefixed with ``aa_`` so it sorts
  alphabetically first and runs while the driver state is still
  untouched (the suite-before hook only does ``device_is_ready``).
  Auto-skipped if the driver pre-populates a default format.
