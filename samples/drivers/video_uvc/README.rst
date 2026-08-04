.. zephyr:code-sample:: video_uvc
   :name: USB Video Class (UVC) webcam
   :relevant-api: usbd_uvc usbd_api video_interface

   Stream camera frames to a host as a standard USB Video Class webcam.

Overview
********

This sample captures camera frames on an Alif board and streams them to a host
PC as a standard **USB Video Class (UVC)** webcam. When the board is connected
it enumerates as a camera; opening it in any UVC application (``ffplay``,
``cheese``, OBS, the Windows Camera app, ...) pulls a continuous sequence of
frames.

The pipeline and the advertised UVC format are selected at build time from the
devicetree of the target board:

* **E1C SK / B1 SK** — an **OV5640** parallel camera on the LP-CAM bus,
  streamed as **uncompressed RGB565 480x272**. Uses the upstream Alif CPI
  (LP-CAM) driver and the OV5640 default DVP configuration.
* **E8 DK (RTSS-HP)** — an **ARX3A0**, **MT9M114** or **OV5675** MIPI
  CSI-2 camera through the **ISP** and the **Hantro VC9000E hardware JPEG
  encoder**, streamed as **MJPEG** (ARX3A0 at 560x560, MT9M114 at 1280x720,
  OV5675 at 1024x768). There is no default sensor; it is chosen at build time
  by the devicetree overlay (see below).

The flow is:

#. On boot the camera pipeline is initialized and USB is enabled. The board
   appears as a webcam.
#. When the host opens the stream (UVC Probe/Commit), the sample begins
   capturing frames.
#. Each frame is captured (and, on the E8, JPEG-encoded) and pushed out the UVC
   bulk IN endpoint as a sequence of UVC payloads. The host pulls frames at its
   own pace, which throttles the capture loop.

Pipeline behaviour:

* Capture uses at least two buffers kept in flight: the camera/ISP driver
  re-arms the next buffer at end-of-frame while the previous one is streamed
  over USB, so the sensor streams continuously rather than power-cycling every
  frame.
* On the E8, each captured NV12 frame is compressed by the hardware JPEG
  encoder before being sent; the frame size is variable (the encoder decides).
* On the E1C / B1, an uncompressed RGB565 frame is a fixed size
  (``width * height * 2`` bytes) sent straight from the capture buffer.

Implementation notes:

* The UVC class is the **in-tree USB Video Class**
  (:kconfig:option:`CONFIG_USBD_VIDEO_CLASS`) on top of the
  :kconfig:option:`CONFIG_USB_DEVICE_STACK_NEXT` USB device stack. It is
  instantiated from the devicetree ``uvc`` node
  (``compatible = "zephyr,uvc-device"``) in each board overlay, and itself
  registers as a Zephyr video device. The application binds it to a source
  device with ``uvc_set_video_dev()`` before enabling USB; the class then
  generates its USB descriptors from that device's capabilities and negotiates
  format / frame rate with the host over Probe/Commit.
* The advertised format follows the connected source's capabilities:
  **MJPEG** when the source is the JPEG encoder (``jpeg0``) on the E8, otherwise
  uncompressed **RGB565** (GUID ``RGBP`` = Linux ``V4L2_PIX_FMT_RGB565``) from
  the LP-CAM. The pipeline is selected by the devicetree predicate
  ``UVC_OUTPUT_MJPEG`` in ``main.c``.
* The application runs the capture pipeline itself (on the E8, ISP capture plus
  a per-frame hardware JPEG encode) and feeds each finished frame to the UVC
  device with ``video_enqueue()``, then reclaims it with ``video_dequeue()``.
  Because the host pulls at its own pace, that dequeue throttles the capture
  loop to host consumption.
* The class streams over a **bulk** IN endpoint, which has no alternate
  setting to stop a stream: the host stops it with a CLEAR_FEATURE
  (ENDPOINT_HALT) request on that endpoint instead. The class treats this, and
  a USB bus reset, as the end of the stream, after which the UVC device reports
  no committed format (``video_get_format()`` returning ``-EAGAIN``). The
  application then stops capture and waits for the host to open the stream
  again.
* A cable unplug or bus suspend is reported only to the application, through
  the USB device stack messages, so the sample also pauses sending while the
  cable is unplugged or the bus is suspended. If the host stops reading without
  closing the stream, a 2 s send timeout ends the capture session.
* The class streams frame data **zero-copy**: the USB controller reads directly
  from the capture / JPEG buffers. On these Alif SoCs the DWC3 controller can
  only bus-master the non-secure RAM region, so the class's USB buffer pool is
  placed there by the SoC linker script (``soc/alif/*/common/alif_ns.ld``);
  the capture buffers already live in USB-reachable RAM.

Requirements
************

* One of:

  * Alif E1C SK or B1 SK board with a populated OV5640 module, or
  * Alif E8 DK board with an ARX3A0, MT9M114 or OV5675 camera module.

* USB cable from the board to a host PC.

Supported Targets
*****************

* ``alif_e1c_sk/ae1c1f4051920hh/rtss_he`` (OV5640, RGB565)
* ``alif_b1_sk/ab1c1f4m51820ph0/rtss_he`` (OV5640, RGB565)
* ``alif_e8_dk/ae822fa0e5597xx0/rtss_hp`` (ARX3A0, MT9M114 or OV5675 -> ISP ->
  JPEG, MJPEG)

Building and Running
********************

E1C SK and B1 SK
================

Both boards use the OV5640 on the LP-CAM and need no extra arguments.

E1C SK (OV5640, RGB565):

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/video_uvc
   :board: alif_e1c_sk/ae1c1f4051920hh/rtss_he
   :goals: build flash
   :compact:

B1 SK (OV5640, RGB565):

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/video_uvc
   :board: alif_b1_sk/ab1c1f4m51820ph0/rtss_he
   :goals: build flash
   :compact:

E8 DK
=====

Every E8 DK build automatically applies the common board overlay
``alif_e8_dk_ae822fa0e5597xx0_rtss_hp.overlay`` (USB, ISP/CPI graph, CSI
receiver, i2c-mux, JPEG encoder, UVC). It wires **no default sensor**: the
ARX3A0, MT9M114 and OV5675 are peers, so every E8 build adds one sensor overlay
with ``EXTRA_DTC_OVERLAY_FILE`` (which merges on top of the auto-applied board
overlay) plus that sensor's extra Kconfig fragment with ``EXTRA_CONF_FILE``.

ARX3A0 (ISP -> JPEG, MJPEG):

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/video_uvc
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build flash
   :gen-args: -DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_arx3a0.overlay -DEXTRA_CONF_FILE=boards/arx3a0.conf
   :compact:

MT9M114 (ISP -> JPEG, MJPEG):

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/video_uvc
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build flash
   :gen-args: -DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_mt9m114.overlay -DEXTRA_CONF_FILE=boards/mt9m114.conf
   :compact:

.. note::

   The MT9M114 shares I2C address ``0x5D`` with the GT911 touch controller on
   the MW405 display board, which sits on the same ``i2c1`` bus. The MT9M114
   overlay holds the GT911 in reset (a GPIO hog on the E8 DK RESETn, gpio6.5)
   so the two do not collide; this is why ``mt9m114.conf`` also enables
   :kconfig:option:`CONFIG_GPIO_HOGS`. If a display board is attached, the
   touch controller will be held in reset while this build runs.

OV5675 (ISP -> JPEG, MJPEG):

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/video_uvc
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build flash
   :gen-args: -DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_ov5675.overlay -DEXTRA_CONF_FILE=boards/ov5675.conf
   :compact:

The OV5675 streams its full-FOV 1296x972 RAW10 mode; the ISP debayers and
downscales it to 1024x768 NV12 before the JPEG encoder. 1024x768 keeps the
sensor's native 4:3 aspect ratio and is 16-pixel aligned, so the encoder needs
no edge padding. Unlike the MT9M114 the OV5675 sits at I2C address ``0x10`` and
is powered from plain ``power-gpios``, so it needs no GT911 reset-hold hog and
no fixed-regulator Kconfig.

Viewing the stream
==================

After flashing, connect the USB cable to the host PC. The board enumerates as
a UVC webcam. To view the stream on Linux:

.. code-block:: console

   # find the device node, e.g. /dev/video0
   v4l2-ctl --list-devices

   # E1C / B1: uncompressed RGB565 480x272
   ffplay -f v4l2 -input_format rgb565le -video_size 480x272 /dev/videoN

   # E8 (ARX3A0): MJPEG 560x560
   ffplay -f v4l2 -input_format mjpeg -video_size 560x560 /dev/videoN

   # E8 (MT9M114): MJPEG 1280x720
   ffplay -f v4l2 -input_format mjpeg -video_size 1280x720 /dev/videoN

   # E8 (OV5675): MJPEG 1024x768
   ffplay -f v4l2 -input_format mjpeg -video_size 1024x768 /dev/videoN

   # grab a single still (E8 ARX3A0 MJPEG)
   ffmpeg -y -f v4l2 -input_format mjpeg -video_size 560x560 \
       -i /dev/videoN -frames:v 1 frame.jpg

Sample Output
*************

.. code-block:: console

   *** Booting Zephyr OS build ... ***
   [00:00:00.074,000] <inf> video_usbout_uvc: - Device name: lpcam@43003000
   [00:00:00.074,000] <inf> video_usbout_uvc: - capture format: RGBP 480x272
   [00:00:00.074,000] <inf> video_usbout_uvc: - capture buffer 0: 261120 bytes at 0x200c2040
   [00:00:00.078,000] <inf> video_usbout_uvc: UVC webcam enabled - waiting for host to open the stream.
   [00:00:05.123,000] <inf> usbd_uvc: Host sent a VideoStreaming COMMIT control
   [00:00:05.124,000] <inf> usbd_uvc: Ready to transfer 'RGBP' 480x272
   [00:00:05.125,000] <inf> video_usbout_uvc: Capture session started (2 buffers)
