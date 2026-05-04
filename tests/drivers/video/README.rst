.. _alif-cpi-sample:

Alif CPI Sample
###############

Overview
********

This test exercises the Alif Camera Parallel Interface (CPI) / MIPI Camera
Serial Interface (CSI-2) video capture pipeline via the Zephyr
:dtcompatible:`alif,cam` driver. It validates the generic Zephyr ``video``
API (``video_get_caps``, ``video_set_format``, ``video_get_format``,
``video_buffer_alloc``, ``video_enqueue``, ``video_dequeue``,
``video_stream_start``, ``video_stream_stop``, ``video_flush``,
``video_set_signal``) and exercises end-to-end frame capture with one or
more connected sensors.

The test is organised into two ``ztest`` suites:

``cpi_manual_testcase``
  End-to-end / integration tests that bring up the full pipeline, capture
  frames, and exercise driver paths that depend on real sensor timing
  (e.g. image capture, frame dequeue, abort/flush flows, multi-camera
  switching). Intended to be run on hardware with a camera attached.

``cpi_api_testcase``
  Focused API-level unit tests that target a single ``video_*`` entry point
  in isolation, verify caps/format negotiation, buffer lifecycle,
  start/stop idempotency, flush semantics, and
  the ``video_get_format``-before-``video_set_format`` regression.

Supported Sensors
*****************

The pipeline format is selected at compile time from the device tree:

+--------------------------------+---------------------------+----------------+
| Sensor                         | Interface                 | Pixel format   |
+================================+===========================+================+
| ``aptina,mt9m114``             | Parallel CPI              | ``GREY``       |
+--------------------------------+---------------------------+----------------+

When ``CONFIG_USE_ALIF_ISP_LIB=y`` is enabled (see ``boards/isp.conf``),
the output format is forced to ``RGB888_PLANAR_PRIVATE`` and frames flow
through the ISP (``vsi,isp-pico``).

Building and Running
********************

The application only builds for targets whose device tree enables a
:dtcompatible:`alif,cam` node.

Parallel CPI + MT9M114 (RTSS-HP, E8 DK)
=======================================

.. code-block:: console

   west build -p -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       ../alif/tests/drivers/video/ \
       -- -DDTC_OVERLAY_FILE="$PWD/../alif/tests/drivers/video/boards/cpi_mt9m114.overlay"

Parallel CPI + MT9M114 (RTSS-HE)
================================

.. code-block:: console

   west build -p -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       ../alif/tests/drivers/video/ \
       -- -DDTC_OVERLAY_FILE="$PWD/../alif/tests/drivers/video/boards/cpi_mt9m114.overlay"

LP-CPI + MT9M114 (RTSS-HE)
==========================

.. code-block:: console

   west build -p -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       ../alif/tests/drivers/video/ \
       -- -DDTC_OVERLAY_FILE="$PWD/../alif/tests/drivers/video/boards/lpcpi_mt9m114.overlay"

Flash and open a serial console at 115200 8N1 to observe the test output.

Prj Options
***********

Key config options in :file:`prj.conf`:

- ``CONFIG_VIDEO=y`` — enables Zephyr video subsystem
- ``CONFIG_I2C=y`` / ``CONFIG_I2C_TARGET=y`` — sensor configuration bus
- ``CONFIG_ZTEST=y`` — ztest harness
- ``CONFIG_POLL=y`` (optional) — enables :c:func:`video_set_signal` code
  paths and the ``api_set_signal`` test case

Sample Output
*************

::

   *** Booting Zephyr OS ***
   Running TESTSUITE cpi_api_testcase
   ===================================================================
   START - api_a_get_format_before_set_format
   get_format before set_format: fmt=Y    640x480 pitch=640
    PASS - api_a_get_format_before_set_format in 0.012 seconds
   ===================================================================
   START - api_get_caps_valid_ep
   api_get_caps_valid_ep: first fmt=0x59382020 w=[640..640] h=[480..480]
    PASS - api_get_caps_valid_ep in 0.003 seconds
   ===================================================================
   ...
   TESTSUITE cpi_api_testcase succeeded

   Running TESTSUITE cpi_manual_testcase
   ===================================================================
   START - video_test_image_capture
   - Device name: cam@49033000
   - Capabilities:
     GREY width [640; 640; 0] height [480; 480; 0]
   - format: GREY 640x480
    PASS - video_test_image_capture in 7.214 seconds
   ===================================================================
   ...

   ------ TESTSUITE SUMMARY START ------
   SUITE PASS - 100.00% [cpi_api_testcase]:  pass = 17, fail = 0, skip = 0
   SUITE PASS - 100.00% [cpi_manual_testcase]: pass = 3,  fail = 0, skip = 0
   ------ TESTSUITE SUMMARY END ------

   PROJECT EXECUTION SUCCESSFUL

Notes
*****

- ``cpi_api_testcase`` runs first and leaves the device stopped / drained
  before handing off to ``cpi_manual_testcase`` (see
  :c:func:`api_suite_teardown` in ``src/video_api.c``).
- ``manual_suite_before`` is shared by both suites: it acquires the CAM /
  ISP device handle and force-stops any in-flight capture left over from
  a previous run (detected via the ``CAM_CTRL.BUSY`` bit).
- The ISP path (:dtcompatible:`vsi,isp-pico`) is only selected when
  ``CONFIG_USE_ALIF_ISP_LIB=y`` and the ``isp`` DT node is enabled.
