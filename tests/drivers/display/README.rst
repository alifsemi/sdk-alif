.. _display_cdc200_test:

CDC200 Display Driver Test
###########################

Overview
********

ZTest suite for the Alif CDC200 display driver
(``tes,cdc-2.1``).

Two test suites are provided:

- **display_api**: Tests all display driver API functions (standard and CDC200-specific)
- **display_functional**: Tests helper functions and functional behavior

Each test exercises a distinct slice of the display driver API. The suite
covers basic capabilities reporting, framebuffer access, display writes,
blanking control, orientation changes, and buffer management.

Test Coverage
*************

API Tests (display_api suite)
*****************************

Standard Display API
====================

#. Display blanking on/off control.
#. Display read operations.
#. Display pixel format setting.
#. Display brightness setting.
#. Display contrast setting.
#. Capabilities reporting (resolution, pixel formats, orientation).
#. Display orientation changes (validates all 4 orientations).
#. CDC200 layer write (``cdc200_display_write``) API success.
#. Standard ``display_write()`` API success (MIPI DSI builds).
#. Multiple rectangle write operations.

CDC200-Specific API
===================

#. Display enable/disable control.
#. CDC200-specific capabilities (panel resolution, layer configuration).
#. Framebuffer access for both layers.
#. CDC200 display read operations.
#. Framebuffer swapping.
#. Default framebuffer restoration.

Functional Tests (display_functional suite)
*******************************************

#. Framebuffer solid color sweep (red, blue, green, white, black).
#. Framebuffer fill performance benchmark (word-based vs per-pixel memcpy).
#. Framebuffer readback verification (write pattern, read back, verify match).
#. Region clipping (out-of-bounds, partial out-of-bounds, valid write).
#. Display power cycle (disable/enable, operations while disabled, recovery).
#. Invalid layer index handling.
#. Undersized buffer / NULL parameter checks (skipped until driver is fixed).
#. Read cache coherency probe.

Requirements
************

* A board with a CDC200 node (``tes,cdc-2.1``). Panel interfaces:

  - **Parallel RGB**: all targets (E7, E8, E1C, B1)
  - **2-lane MIPI-DSI**: alif_e8_dk / alif_e7_dk
  - **1-lane MIPI-DSI**: alif_e1c_dk / alif_b1_dk
* Zephyr RTOS with display subsystem enabled.

Building and Running
********************

Parallel RGB (default, all targets):

.. code-block:: console

   west build -b <board> <test_app_path>

2-lane MIPI-DSI (ILI9806E / MW405 on E7/E8 DK):

.. code-block:: console

   west build -b <board> <test_app_path> -- \
     -DDTC_OVERLAY_FILE="boards/serial_display_2lane.overlay" \
     -DEXTRA_CONF_FILE="boards/serial_display.conf"

1-lane MIPI-DSI (ILI9488 on E1C/B1 DK):

.. code-block:: console

   west build -b <board> <test_app_path> -- \
     -DDTC_OVERLAY_FILE="boards/serial_display_1lane.overlay" \
     -DEXTRA_CONF_FILE="boards/serial_display.conf"

Twister scenarios in ``testcase.yaml``:

* ``drivers.display.cdc200`` — parallel RGB (all targets)
* ``drivers.display.cdc200.mipi_2lane`` — 2-lane MIPI-DSI (E7/E8)
* ``drivers.display.cdc200.mipi_1lane`` — 1-lane MIPI-DSI (E1C/B1)

Timeout is 180 s to cover the visual ``k_msleep()`` delays. Optional
``boards/overlay_bg*.overlay`` files are background-color experiments;
they disable the layers and are not part of the default suite.

Expected output
***************

.. code-block:: console

   PASS - display_api::test_display_blanking
   PASS - display_api::test_display_cdc200_enable
   PASS - display_api::test_display_cdc200_get_caps
   PASS - display_api::test_display_cdc200_get_framebuffer
   PASS - display_api::test_display_get_capabilities
   PASS - display_api::test_display_orientation
   PASS - display_api::test_display_write
   PASS - display_api::test_generic_display_write
   PASS - display_api::test_display_write_multiple_rects
   PASS - display_api::test_display_read
   PASS - display_api::test_display_set_pixel_format
   PASS - display_api::test_display_set_brightness
   PASS - display_api::test_display_set_contrast
   PASS - display_api::test_cdc200_display_read
   PASS - display_api::test_cdc200_swap_fb
   PASS - display_api::test_restore_fb
   PASS - display_functional::test_display_fb_fill_benchmark
   PASS - display_functional::test_display_fb_readback_verify
   PASS - display_functional::test_display_fb_solid_sweep
   PASS - display_functional::test_display_power_cycle
   PASS - display_functional::test_display_region_clipping
   PASS - display_functional::test_display_invalid_layer
   SKIP - display_functional::test_display_undersized_buffer
   SKIP - display_functional::test_display_null_params
   PASS - display_functional::test_display_read_cache_coherency

``test_generic_display_write`` is skipped when ``CONFIG_MIPI_DSI`` is not
enabled. ``test_display_undersized_buffer`` and ``test_display_null_params``
are skipped until the driver adds the corresponding validation.

Test Details
************

API Tests
=========

test_display_blanking
  Validates display blanking on/off control for power management.

test_display_read
  Tests reading pixel data from the display framebuffer.

test_display_set_pixel_format
  Tests setting the display pixel format to the current format.

test_display_set_brightness
  Tests display brightness control. Skips if not supported by driver.

test_display_set_contrast
  Tests display contrast control. Skips if not supported by driver.

test_display_get_capabilities
  Validates basic display capabilities including resolution,
  supported pixel formats, and current orientation.

test_display_orientation
  Tests display orientation changes with graceful handling if the
  hardware does not support orientation changes.

test_display_write
  Calls ``cdc200_display_write()`` for one full-width row and asserts
  that the call succeeds. This is an API success check, not a visual
  full-screen fill. Colors are packed for the active pixel format.

test_generic_display_write
  Calls the standard Zephyr ``display_write()`` API for one full-width
  row and asserts success. Runs when MIPI DSI is enabled; skipped
  otherwise.

test_display_write_multiple_rects
  Tests writing multiple colored rectangles to different positions on
  the display to verify coordinate handling. Rectangle colors are packed
  for the active pixel format (ARGB8888 / RGB888 / RGB565).

test_display_cdc200_enable
  Tests display enable/disable functionality through the CDC200-specific
  control interface.

test_display_cdc200_get_caps
  Retrieves CDC200-specific capabilities including panel resolution
  and layer configuration for both display layers.

test_display_cdc200_get_framebuffer
  Retrieves framebuffer information for layer 0 and optionally layer 1,
  validating address and size parameters.

test_cdc200_display_read
  Tests reading pixel data from CDC200-specific layer framebuffer.

test_cdc200_swap_fb
  Tests framebuffer swapping for a specific layer.

test_restore_fb
  Tests restoring default framebuffers for all layers.

Functional Tests
================

test_display_fb_fill_benchmark
  Benchmarks framebuffer fill performance by comparing word-based fill
  against per-pixel memcpy. Measures execution time in cycles and
  nanoseconds for both methods and calculates the speedup ratio. This
  test validates the performance improvement of word-based direct memory
  access over per-pixel operations.

test_display_fb_readback_verify
  Validates framebuffer write and read integrity by writing a checkerboard
  pattern to a 64x64 pixel region at position (100, 100), reading back the
  same region using cdc200_display_read(), and verifying that the read data
  matches the written data byte-for-byte. This test ensures both the write
  and read paths are functioning correctly and data is preserved through
  the framebuffer operations.

test_display_fb_solid_sweep
  Tests framebuffer solid color sweep (red, blue, green, white, black)
  with 3-second delays for visual observation. Colors are packed for the
  active pixel format. The write method can be selected via the
  DISPLAY_FB_WRITE_METHOD macro:

  - DISPLAY_FB_WRITE_DIRECT (default): Uses cdc200_get_framebuffer()
    and word-based fill for direct memory access
  - DISPLAY_FB_WRITE_API: Uses cdc200_display_write() API with 10-row
    buffer allocated in system RAM, written in a row-strip pattern to fill
    the entire screen

test_display_power_cycle
  Tests display power cycle by disabling the display via cdc200_set_enable(),
  attempting write and read operations while disabled (to verify robustness),
  re-enabling the display, waiting for power up, and verifying functionality by
  clearing the display to white. This test ensures the display driver correctly
  handles power state transitions and recovers functionality after a power cycle.

test_display_region_clipping
  Tests region clipping behavior by attempting writes with various invalid
  coordinates:

  - Negative coordinates (-10, -10) — skipped (known driver fault, waiting
    for a fix)
  - Coordinates beyond display resolution
  - Partial out-of-bounds rectangles (rectangle extends beyond edge)
  - Valid write at (100, 100) as control

  Out-of-bounds cases currently log the driver return value. A valid write
  must succeed.

test_display_invalid_layer
  Passes invalid layer indices (2, 5, 255) to CDC200 APIs and expects
  ``-EINVAL`` from write/read, and no output modification from void APIs.

test_display_undersized_buffer
  Skipped until the driver validates ``desc->buf_size`` against
  width * height * pixel size.

test_display_null_params
  Skipped until the driver rejects NULL ``desc`` / ``buf`` with ``-EINVAL``.

test_display_read_cache_coherency
  Writes a pattern directly into the framebuffer (no cache flush) and
  reads it back via ``cdc200_display_read()``. Documents cache behavior
  on the read path.
