.. _display_cdc200_test:

CDC200 Display Driver Test
###########################

Overview
********

ZTest suite for the Alif CDC200 display driver
(``alif,cdc200``).

Two test suites are provided:
- **display_api**: Tests all display driver API functions (standard and CDC200-specific)
- **display_functional**: Tests helper functions and functional behavior

Each test exercises a distinct slice of the display driver API. The suite
covers basic capabilities reporting, framebuffer access, display writes,
blanking control, orientation changes, and buffer management.

Test Coverage
************

API Tests (display_api suite)
******************************

Standard Display API
====================
#. Display blanking on/off control.
#. Display read operations.
#. Display pixel format setting.
#. Display brightness setting.
#. Display contrast setting.
#. Capabilities reporting (resolution, pixel formats, orientation).
#. Display orientation changes (validates all 4 orientations).
#. Basic display write operations.
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
********************************************
#. Framebuffer solid color sweep (red, blue, green, white, black).
#. Framebuffer fill performance benchmark (word-based vs per-pixel memcpy).
#. Framebuffer readback verification (write pattern, read back, verify match).
#. Region clipping (negative coordinates, out-of-bounds, partial out-of-bounds).
#. Display power cycle (disable/enable, test operations while disabled, verify recovery).

Requirements
************

* Alif Ensemble E8 Development/Application Kit (or any board exposing
  the display node with compatible ``alif,cdc200``).
* Zephyr RTOS with display subsystem enabled.

Building and Running
********************

.. code-block:: console

   west build -b <board> <test_app_path>

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

Test Details
************

API Tests
========

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
  Performs a basic display write operation to test the rendering
  pipeline.

test_display_write_multiple_rects
  Tests writing multiple colored rectangles to different positions on
  the display to verify coordinate handling.

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
=================

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
  with 3-second delays for visual observation. The write method can be
  selected via the DISPLAY_FB_WRITE_METHOD macro:
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
  handles power state transitions, rejects operations gracefully when disabled,
  and recovers functionality after a power cycle.

test_display_region_clipping
  Tests region clipping behavior by attempting writes with various invalid
  coordinates:
  - Negative coordinates (-10, -10)
  - Coordinates beyond display resolution
  - Partial out-of-bounds rectangles (rectangle extends beyond edge)
  - Valid write at (100, 100) as control
  The test validates that the driver either rejects invalid coordinates or
  clips them appropriately, while valid writes succeed.
