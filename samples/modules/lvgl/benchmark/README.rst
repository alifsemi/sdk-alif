.. _lvgl_dave2d_benchmark:

LVGL D/AVE 2D Benchmark
#######################

Overview
********

This sample runs the upstream LVGL ``lv_demo_benchmark`` application with all
rendering offloaded to the Alif D/AVE 2D (GPU2D) accelerator. It displays the
benchmark scenes on the MIPI-DSI MW405 panel of the Alif Ensemble E8 DevKit and
reports the measured frames-per-second of each scene on the panel.

The sample is intentionally thin: it reuses the LVGL benchmark and widgets demo
sources shipped with the LVGL module, lets the Zephyr LVGL glue create the
display and rendering buffers, and only adds the small amount of Alif-specific
glue required to drive the GPU.

What it takes to accelerate LVGL with D/AVE 2D
**********************************************

* **Kconfig** - enable the GPU draw unit and the D/AVE 2D library:

  .. code-block:: cfg

     CONFIG_DAVE2D=y
     CONFIG_LV_USE_DRAW_DAVE2D=y

  ``CONFIG_DAVE2D`` exposes the D/AVE 2D ``d0``/``d1``/``d2`` headers globally and
  the LVGL draw unit (``src/draw/renesas/dave2d``) is compiled in when
  ``LV_USE_DRAW_DAVE2D`` is set.

* **hal_data.h shim** - LVGL's D/AVE 2D draw unit was written against the Renesas
  FSP and includes ``hal_data.h``, from which it obtains the D/AVE 2D driver, the
  framebuffer colour format (``g_display0_cfg``) and the D-cache configuration
  (``BSP_CFG_DCACHE_ENABLED``). Zephyr does not provide this header, so the sample
  ships a small compatibility shim in :file:`include/hal_data.h` that maps the TES
  D/AVE 2D driver headers, reports the framebuffer format that matches the LVGL
  colour depth, and enables the draw unit's cache maintenance. The shim is added
  to the global include path with ``zephyr_include_directories()`` so it is
  visible to the LVGL module library build (not just the application):

  .. code-block:: cmake

     zephyr_include_directories(include)

* **Cache coherency** - the Alif Cortex-M55 runs with its D-cache enabled and the
  GPU buffers live in cacheable SRAM, so the CPU and the GPU must be kept
  coherent. The draw unit already performs the required clean/invalidate around
  the GPU source and destination buffers, gated by ``BSP_CFG_DCACHE_ENABLED``;
  the shim enables those paths, which use portable CMSIS (``SCB_*``) and the Alif
  ``d1_cacheblockflush()`` helper.

* **Devicetree** - the GPU node is not part of the SoC devicetree, so the board
  overlays add it together with the ``d2-inst`` alias the D/AVE 2D ``d1`` layer
  uses to locate the GPU:

  .. code-block:: devicetree

     aliases { d2-inst = &gpu2d; };
     soc {
         gpu2d: dave2d@49040000 {
             compatible = "tes,dave2d";
             reg = <0x49040000 0x1000>;
             interrupts = <332 2>;
             clocks = <&clockctrl ALIF_GPU_CLK>;
         };
     };

* **D/AVE 2D heap** - the LVGL draw unit allocates its display lists and render
  buffers from the D/AVE 2D heap during ``lv_init()``. Because the Zephyr LVGL
  glue calls ``lv_init()`` automatically from an ``APPLICATION``-level
  ``SYS_INIT()`` hook, the application initialises the heap from an earlier
  ``POST_KERNEL`` hook (see :file:`src/main.c`).

* **Memory placement** - the D/AVE 2D heap and full-screen double-buffered VDBs
  together exceed what DTCM can provide, so they are placed in the larger
  globally-addressable SRAM banks where the CPU, GPU and CDC200 can all access
  them:

  - the D/AVE 2D heap (512 KB) is placed in SRAM1 via ``__alif_sram1_section``;
  - the two full-screen LVGL VDBs (``.lvgl_buf``) and memory pool
    (``.lvgl_heap``) are relocated to SRAM0 by :file:`lvgl_sram.ld`
    (``CONFIG_LV_Z_VDB_CUSTOM_SECTION`` / ``CONFIG_LV_Z_MEMORY_POOL_CUSTOM_SECTION``).

  In DIRECT mode the VDBs double as the CDC200 scan-out framebuffers (zero-copy
  page-flip via ``cdc200_swap_fb()``), so no separate framebuffer allocation is
  needed.

Display
*******

The board overlays enable the CDC200 controller, the MIPI-DSI host and the
MW405 panel (480x800, configured here as RGB565 to match LVGL's default 16-bit
colour depth) and select the CDC200 as ``zephyr,display``.

Powering the display pipeline (E8)
==================================

On the Ensemble E8 the MIPI-DSI/D-PHY, the CDC200 and the GPU are power- and
clock-gated out of reset, so the application restores a run profile through the
Secure Enclave before the drivers initialise (see ``display_pipeline_power_init()``
in :file:`src/main.c`, registered at ``SYS_INIT(PRE_KERNEL_1, 46)``):

* It enables the **HFOSC (38.4 MHz)** oscillator, which is the MIPI D-PHY PLL
  reference clock (the ``dphy`` node's ``ref-frequency``). Without it the D-PHY
  PLL never locks and the panel cannot attach to the DSI host
  (``D-PLL not locked to desired frequency`` / ``Could not attach to MIPI-DSI host``).
* It powers the MIPI TX/RX/PLL D-PHYs and the PHY LDO
  (``phy_pwr_gating``) and ungates the CDC200, MIPI-DSI and GPU2D clocks
  (``ip_clock_gating``).
* It keeps SRAM0 (LVGL buffers) and SRAM1 (D/AVE 2D heap) powered
  (``memory_blocks``).
* On the E8 DevKit the MIPI lanes pass through a camera/display mux; the
  application drives the mux GPIO (``cam-disp-mux-gpios``, on ``gpio14``) to the
  display position, so the overlays also enable ``&gpio14``.

The CDC200 scan-out engine is also not started by its own driver init, and the
generic ``display_blanking_off()`` API (which LVGL's Zephyr glue calls) is a
no-op for it. The application therefore starts it explicitly with
``cdc200_set_enable(display_dev, true)`` in :file:`src/main.c`; without it the
panel stays dark even though the DSI link and backlight are up.

.. note::

   The overlays target the MW405 panel wired as the Ensemble devicetree default.
   If your DevKit ships a different panel, adjust the ``cdc200`` timings,
   ``pixel-fmt-l1`` and the MIPI-DSI panel child node accordingly.

Building and Running
********************

RTSS-HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       alif/samples/modules/lvgl/benchmark

RTSS-HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       alif/samples/modules/lvgl/benchmark

After flashing, the benchmark scenes are rendered on the panel and each scene's
frames-per-second result is shown on screen.
