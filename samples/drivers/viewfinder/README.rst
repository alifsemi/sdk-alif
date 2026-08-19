.. _viewfinder-sample:

Viewfinder Sample
#################

Overview
********

This sample captures continuous frames from the ARX3A0 camera sensor and
displays them on the MIPI DSI panel. The camera feeds frames via MIPI CSI-2
to the CAM controller.
On E8 targets, the ISP is included in the pipeline for image processing
(demosaic, scalar, color conversion). On E7 targets, the ISP is not used
and frames are processed in software via the AIPL demosaicing library.

The functional behaviour is:

* ARX3A0 camera sensor sends frames over MIPI CSI-2 (DPHY) to the
  CSI-2 receiver.
* The CSI-2 receiver converts the serial data to parallel and passes
  it to the CAM controller.
* On E8 (ISP enabled): the CAM controller forwards frames to the ISP,
  which performs image processing and outputs YUV420 frames to memory.
  The application then converts these to RGB565 using AIPL and writes
  directly to the display framebuffer.
* On E7 (no ISP): the CAM controller outputs raw Y10P Bayer frames to
  memory. The application demosaics these to RGB565 using the AIPL
  library and writes to the display framebuffer.

The display output is shown on the MIPI DSI panel (480x800), with the
camera image occupying the top 480x480 region and the Alif logo
displayed below it.

Build Configuration
*******************

For E7 (no ISP)::

   west build -b <board> -- \
     -DDTC_OVERLAY_FILE=boards/arx3a0_mipi_viewfinder.overlay

For E8 with ISP::

   west build -b <board> -- \
     -DDTC_OVERLAY_FILE=boards/arx3a0_mipi_isp_viewfinder.overlay \
     -DOVERLAY_CONFIG=boards/isp.conf

Requirements
************

The sample utilizes the CAM Controller IP, MIPI CSI-2 receiver
(Synopsys DesignWare), and the ARX3A0 camera sensor. On E8, the Alif
ISP is also used in the pipeline. The AIPL (Alif Image Processing
Library) is used for color conversion and demosaicing.

Tested Sensors
**************

* ARX3A0 (CSI)

Sample Output
*************

.. code-block:: console

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
