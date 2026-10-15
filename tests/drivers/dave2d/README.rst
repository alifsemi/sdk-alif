.. _dave2d_test:

D/AVE 2D Driver Test
####################

Overview
********

Ztest suite for the Alif D/AVE 2D GPU driver (``tes,dave2d``). The test opens
the GPU, renders primitives into an RGB565 framebuffer in SRAM0, and checks
the resulting pixels. It does not run the LVGL sample applications.

Cases
*****

- Clear the framebuffer to a solid colour.
- Filled box, with pixels inside and outside the box.
- Horizontal line.
- Filled circle.
- Filled triangle.
- Blit of an RGB565 source.
- Constant-alpha blend of red over black.
- Clip rectangle discards drawing outside the clip.
- Thirty-two back-to-back frames.
- Framebuffer pitch wider than the width, and an odd width.
- Boxes in the top-left and bottom-right corners.
- RGB888 source blit into RGB565.
- Circle ring (non-zero width) rather than a filled disc.
- Antialiased edge of a box that starts on a half pixel.
- ``d1_copytovidmem`` followed by a blit, with no extra CPU cache flush.
- ``d1_cacheblockflush`` keeps a dirty pattern.
- Idle ``d1_queryirq`` times out instead of reporting an interrupt.
- Inverted clip, NULL framebuffer and NULL blit source return errors.
- NULL device handle is rejected by the calls that test it in every build.
- A polygon with fewer than three points is rejected and draws nothing.
- A zero-width or zero-height box draws nothing.
- A box that lies fully outside the framebuffer draws nothing.
- An unknown blend factor returns an error.
- Rendermode 0, a 1x1 framebuffer and an inverted clip height return errors.
- A negative circle radius is rejected and draws nothing.
- Colour index 5 is rejected.
- A second ``d2_inithw`` while the GPU is already bound is rejected.
- ``d2_renderbox`` with a NULL device is rejected. On a build that skips the
  check this case faults instead of returning.

Requirements
************

* Alif Ensemble E8 DevKit (RTSS-HE or RTSS-HP). The GPU node is added by the
  test overlay; a panel is not required.

Building and running
********************

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       alif/tests/drivers/dave2d
   west flash

   west twister -T alif/tests/drivers/dave2d \
       -p alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       -p alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
       --device-testing --device-serial /dev/ttyACM0
