.. _lvgl_dave2d_test:

LVGL and D/AVE 2D Test
######################

Overview
********

Ztest suite for LVGL rendering on the Alif D/AVE 2D GPU. The cases call LVGL
draw and widget APIs and check pixels. They do not run
``alif/samples/modules/lvgl``.

The D/AVE 2D driver itself is covered by :ref:`dave2d_test`.

What is checked
***************

- The ``DAVE2D`` draw unit is registered after ``lv_init()``.
- Solid fill, 50% opacity, border, RGB565 image blit, label, line, arc and
  triangle, rendered into a 64x64 canvas in SRAM0.
- The common widget set (label, button, slider, bar, switch, checkbox, arc,
  LED, line, image) refreshes. A solid red object, the panel background and
  the green image are read back from the CDC200 framebuffer.
- Rounded rectangle, horizontal red-to-blue gradient, box shadow, a 2x scaled
  image, and an RGB888 image. The gradient, shadow and RGB888 image are not
  taken by the D/AVE unit, so the software renderer has to draw them.

Requirements
************

* Alif Ensemble E8 DevKit with the MW405 panel, RTSS-HE or RTSS-HP.

Building and running
********************

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       alif/tests/modules/lvgl
   west flash

   west twister -T alif/tests/modules/lvgl -T alif/tests/drivers/dave2d \
       -p alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       -p alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       --device-testing --device-serial /dev/ttyACM0
