.. _viewfinder:

==========
Viewfinder
==========

Introduction
============

This application note describes how to capture continuous frames from an
ARX3A0, MT9M114, or OV5675 camera sensor and display them on the MIPI DSI
panel.
The camera feeds frames via MIPI CSI-2 to the CAM controller.

On E8 targets, the ISP is included in the pipeline for image processing
(demosaic, scaler, color conversion). On E7 targets, the ISP is not used:
ARX3A0 frames are demosaiced in software via AIPL; MT9M114 outputs
RGB565 and is center-cropped into the preview.

**Image data path (E8, ISP enabled)**::

   ARX3A0 / MT9M114 / OV5675 → MIPI CSI-2 (DPHY) → CSI-2 receiver → CAM controller
   → ISP → YUV420 in memory → AIPL RGB565 → display framebuffer

**Image data path (E7, no ISP)**::

   ARX3A0 → MIPI CSI-2 (DPHY) → CSI-2 receiver → CAM controller
   → Y10P Bayer in memory → AIPL demosaic RGB565 → display framebuffer

Overview
========

The functional behaviour is:

- The ARX3A0 camera sensor sends frames over MIPI CSI-2 (DPHY) to the
  CSI-2 receiver.
- The CSI-2 receiver converts the serial data to parallel and passes
  it to the CAM controller.
- On E8 (ISP enabled): the CAM controller forwards frames to the ISP,
  which performs image processing and outputs YUV420 frames to memory.
  The application then converts these to RGB565 using AIPL and writes
  directly to the display framebuffer.
- On E7 (no ISP): the CAM controller outputs raw Y10P Bayer frames to
  memory. The application demosaics these to RGB565 using the AIPL
  library and writes to the display framebuffer.
- MT9M114 (CSI) uses the same CSI-2 path. Without ISP the sensor
  outputs RGB565 at 648x488. With ISP it outputs Y10P; the ISP
  converts to YUV420. ``CONFIG_ISP_LIB_CSM_MODULE`` must stay enabled
  or MT9 frames appear black or green.
- OV5675 (CSI, E8 with ISP only) uses the same CSI-2 path. The sensor
  outputs Y10P at 640x480. The ISP converts that to YUV420 and scales
  it to the 480x480 preview. This build supports the compact module at
  I2C address ``0x10`` only. The overlay does not instantiate a sensor
  at ``0x36``, so a legacy module will not initialize.

The display output is shown on the MIPI DSI panel (480x800), with the
camera image occupying the top 480x480 region and the Alif logo
displayed below it.

Tested Sensors
==============

- ARX3A0 (CSI)
- MT9M114 (CSI)
- OV5675 (CSI, compact module, I2C ``0x10``)

Requirements
============

The sample uses the CAM Controller IP, MIPI CSI-2 receiver
(Synopsys DesignWare), and the ARX3A0/MT9M114/OV5675 camera sensor.
On E8, the Alif ISP is also used in the pipeline.
The AIPL (Alif Image Processing Library) is used for color conversion
and demosaicing.

.. include:: prerequisites.rst

.. include:: note.rst

Build a Viewfinder Application with Zephyr
==========================================

Follow these steps to build the viewfinder application using the Alif Zephyr SDK:

1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E7 and E8 DevKits.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section ``Setting Up and Building Zephyr Applications``.

E7 with ARX3A0 (no ISP)
-----------------------

2. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/viewfinder/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay"

3. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/viewfinder/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_viewfinder.overlay"

E8 with ARX3A0 and ISP
----------------------

4. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/viewfinder/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

5. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/viewfinder/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/arx3a0_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

E7 with MT9M114 (no ISP)
------------------------

6. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     ../alif/samples/drivers/viewfinder/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

7. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     ../alif/samples/drivers/viewfinder/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

E8 with MT9M114 and ISP
-----------------------

8. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/viewfinder/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

9. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/viewfinder/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/mt9m114_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf;$PWD/../alif/samples/drivers/viewfinder/boards/serial_camera_mt9m114.conf"

.. note::

   For MT9M114 with ISP, ``CONFIG_ISP_LIB_CSM_MODULE`` must stay enabled
   or MT9 frames appear black or green.

E8 with OV5675 and ISP
----------------------

10. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/viewfinder/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

11. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/viewfinder/ \
     -- \
     -DDTC_OVERLAY_FILE="$PWD/../alif/samples/drivers/viewfinder/boards/ov5675_mipi_isp_viewfinder.overlay" \
     -DOVERLAY_CONFIG="$PWD/../alif/samples/drivers/viewfinder/boards/isp.conf"

.. note::

   The OV5675 overlay selects I2C address ``0x10`` (compact module).
   There is no legacy ``0x36`` overlay in this sample.

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
==============================

To execute binaries on the DevKit, follow the command:

.. code-block:: console

   west flash

Console Output
==============

The following output is observed in the console during execution of the viewfinder application:

.. code-block:: text

   [00:00:00.000,000] <inf> csi2_dw: #rx_dphy_ids: 1
   [00:00:01.450,000] <inf> panel_mw405: MW-405 Configuration.
   *** Booting Zephyr OS build 3a2b84d96961 ***
   [00:00:01.460,000] <inf> video_app: Enable Ensemble-DSI Device video mode.
   [00:00:01.460,000] <inf> video_app: Video device: isp@49046000
   [00:00:01.460,000] <inf> video_app: Selected camera: Selfie
   [00:00:01.460,000] <inf> video_app: Sensor Capabilities:
   [00:00:01.460,000] <inf> video_app:   Y10P 560x560 (wxh)
   [00:00:01.461,000] <inf> dphy_dw: RX-DDR clock: 400000000
   [00:00:01.461,000] <inf> video_app: Format: YU12 480x480
   [00:00:01.461,000] <inf> video_app: Width - 480, Pitch - 720, Height - 480, Buff size - 345600
   [00:00:01.461,000] <inf> video_app: Buffer 0: addr 0x20bb860, size 345600, 480x480
   [00:00:01.476,000] <inf> video_app: Buffer 1: addr 0x210fe68, size 345600, 480x480
   [00:00:01.492,000] <inf> video_app: Conversion buffer (framebuffer): addr - 0x2000000, size - 345600
   [00:00:02.492,000] <inf> video_app: Display cdc200@49031000: 480x800
   [00:00:02.515,000] <inf> video_app: Capture started
   [00:00:08.383,000] <inf> video_app: Frame 30 | FPS ~5
   [00:00:14.383,000] <inf> video_app: Frame 60 | FPS ~5
   [00:00:20.384,000] <inf> video_app: Frame 90 | FPS ~5
   [00:00:26.384,000] <inf> video_app: Frame 120 | FPS ~5
   [00:00:32.384,000] <inf> video_app: Frame 150 | FPS ~5
