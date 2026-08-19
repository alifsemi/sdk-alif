..
   Copyright (C) Alif Semiconductor - All Rights Reserved.
   Use, distribution and modification of this code is permitted under the
   terms stated in the Alif Semiconductor Software License Agreement

   You should have received a copy of the Alif Semiconductor Software
   License Agreement with this file. If not, please write to:
   contact@alifsemi.com, or visit: https://alifsemi.com/license

.. _lvgl_gui:

LVGL XML Widgets
################

Overview
********

This sample builds an LVGL UI from a **declarative XML description** and renders
it on the MIPI-DSI MW405 panel of the Alif Ensemble E8 DevKit, with all drawing
offloaded to the Alif D/AVE 2D (GPU2D) accelerator.

The UI is authored in :file:`ui/screen_main.xml` and contains common LVGL widgets / drawings:

#. ``lv_image``    - image (Alif logo)
#. ``lv_label``    - text
#. ``lv_button``   - button with a child label
#. ``lv_slider``   - slider
#. ``lv_bar``      - progress bar
#. ``lv_switch``   - on/off switch
#. ``lv_checkbox`` - checkbox
#. ``lv_arc``      - arc / gauge
#. ``lv_led``      - LED indicator
#. ``lv_line``     - polyline
#. ``lv_obj``      - styled rounded-rectangle panel (base drawing object)

XML workflow
************

LVGL's XML feature can be used two ways:

* a **runtime loader** (``lv_xml_*``) that parses XML on the device, or
* an **offline export** where the LVGL UI Editor generates C from the XML and
  that C is compiled into the firmware.

The Alif LVGL fork used by this workspace does **not** ship the runtime XML
loader (there is no ``lv_xml_*`` API in the module), so this sample uses the
offline-export workflow, which is also the recommended path for MCU targets.
Rather than requiring the full LVGL UI Editor (which needs a container
toolchain), the sample bundles a small host-side generator:

* :file:`ui/screen_main.xml` is the UI **source of truth** (the declarative
  description of the widgets and their properties).
* :file:`scripts/gen_ui_xml.py` reads that XML and emits the exported C. Each
  block in ``ui_xml_create()`` maps 1:1 to a tag in the XML, in the same order,
  so the two can be reviewed side by side.
* :file:`src/ui_xml.h` is the stable interface (``ui_xml_create()``); the
  matching ``ui_xml.c`` is **generated** and is not checked in.

The generator is not limited to the widgets used here: it understands the
standard LVGL widget set (labels, buttons, images, bars, sliders, arcs,
switches, checkboxes, dropdowns, rollers, text areas, spinboxes, scales,
tables, charts, calendars, tabviews, lists, windows, and more), the full
``style_<prop>`` system (with ``part`` / ``state`` selectors), flags, states
and flex layout. Unknown tags or attributes are rejected with an error, so
typos fail the build instead of being silently ignored. Widgets that need
extra ``CONFIG_LV_USE_*`` options must have those enabled in
:file:`prj.conf`.

The ``CMakeLists.txt`` wires the generator in with an ``add_custom_command``
that emits ``ui_xml.c`` into the **build tree** (never the source tree) and
rebuilds it whenever ``ui/screen_main.xml`` or the generator changes. To change
the UI, edit the XML and rebuild. To inspect the exported C directly, run the
generator and let it print to stdout::

   python scripts/gen_ui_xml.py ui/screen_main.xml

The optional :file:`ui/project.xml` and :file:`ui/globals.xml` descriptors let
the LVGL UI Editor open and preview ``screen_main.xml``; they are not used by
the firmware build.

Display and GPU
***************

The display pipeline (CDC200 controller, MIPI-DSI host, MW405 panel at
480x800 RGB565) and the D/AVE 2D GPU are set up exactly as in the
:ref:`lvgl_dave2d_benchmark` sample:

* the board overlays add the ``gpu2d`` node and the ``d2-inst`` alias, override
  the CDC200 timings for the MW405 panel and select the CDC200 as
  ``zephyr,display``;
* :file:`include/bsp_api.h` is the compatibility shim the LVGL 9.5.0 D/AVE 2D
  draw unit needs (it includes ``bsp_api.h``);
* the D/AVE 2D heap (512 KB) lives in SRAM1 and the double-buffered full-screen
  VDBs + LVGL memory pool are relocated to SRAM0 by :file:`lvgl_sram.ld`;
* DIRECT-mode rendering is used: the two VDBs double as the CDC200 scan-out
  framebuffers and ``main.c`` page-flips via ``cdc200_swap_fb()`` at each vblank.

Building and running
********************

RTSS-HE:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       alif/samples/modules/lvgl/gui
   west flash

RTSS-HP:

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       alif/samples/modules/lvgl/gui
   west flash

After flashing, press the board **RESET** button. The MW405 panel shows the
widgets stacked vertically, and the console prints::

   XML widgets UI created
