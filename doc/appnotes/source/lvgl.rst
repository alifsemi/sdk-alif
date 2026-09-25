.. _lvgl:

====
LVGL
====

Introduction
============

This application note describes the LVGL samples on the Alif E8 DevKit.
Both samples draw on the MIPI-DSI MW405 panel (480x800, RGB565) and offload
rendering to the D/AVE 2D (GPU2D) accelerator.

.. include:: prerequisites.rst

.. include:: note.rst

LVGL XML Widgets
================

``samples/modules/lvgl/gui`` builds the UI from ``ui/screen_main.xml``.
A host-side generator writes the C that the firmware compiles. The panel
shows common widgets (image, label, button, slider, bar, switch, checkbox,
arc, LED, line, and a styled panel).

Edit ``ui/screen_main.xml`` and rebuild to change the UI.

RTSS-HE
-------

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/modules/lvgl/gui

RTSS-HP
-------

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/modules/lvgl/gui

After flashing, press the board RESET button. The MW405 panel shows the
widgets stacked vertically, and the console prints ``XML widgets UI created``.

LVGL D/AVE 2D Benchmark
=======================

``samples/modules/lvgl/benchmark`` runs the LVGL benchmark with rendering
offloaded to D/AVE 2D. Each scene's frames-per-second result is shown on
the panel.

RTSS-HE
-------

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/modules/lvgl/benchmark

RTSS-HP
-------

.. code-block:: console

   west build -p auto \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/modules/lvgl/benchmark
