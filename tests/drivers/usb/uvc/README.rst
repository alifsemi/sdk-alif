Alif USB UVC Tests
===================

Overview
********

Device-side ztest for the E8 DK USB Video Class webcam, same pipeline as
``samples/drivers/video_uvc``. One sensor per image:

======== ============== ================ =========== ==========
Sensor   ISP input      NV12 / JPEG      NV12 bytes  JPEG cap
======== ============== ================ =========== ==========
MT9M114  Y10P 1288x728  1280x720         1382400     921600
ARX3A0   Y10P 560x560   560x560          470400      313600
OV5675   Y10P 1296x972  1024x768         1179648     786432
======== ============== ================ =========== ==========

USB product **Alif UVC Video Capture**, PID ``0x0009``. The OV5675 frame is
the ISP downscale of the sensor's 1296x972 mode.

The test does not call ``usbd_disable``. On DWC3 that drops the host
webcam. After the suite a thread waits for the host COMMIT and streams,
so the camera can be opened once the UART shows ``PROJECT EXECUTION
SUCCESSFUL``.

``test_host_commit_mjpeg`` skips until the host has committed. JPEG size
and capture time are the on-device sensor and encoder, not the USB wire
rate.

Suites
******

Eleven cases. Each one groups the checks that belong to the same feature.

**uvc_basic**

- ``test_pipeline_geometry`` — sensor, fourccs, Y10P input, NV12 frame, JPEG cap
- ``test_usb_identity`` — ``2fe3:0009``, Alif UVC Video Capture

**uvc_qa**

===================================== ==========================================
Test                                  Check
===================================== ==========================================
``test_devices_ready``                ISP, JPEG encoder, and UVC device
``test_pipeline_formats``             Y10P input, NV12 capture, JPEG in and out
``test_caps_and_buffers``             JPEG and UVC caps, both buffer pools
``test_usb_class``                    USB speed, idle before COMMIT, queue flush
``test_capture_jpeg``                 One NV12 frame and a JPEG starting ``FF D8``
``test_stress``                       30 format reads, 4 capture restarts, 8 JPEGs
``test_perf``                         4 capture times and 4 encode times
``test_host_commit_mjpeg``            SKIP until the host opens the camera
``test_usb_disable_reenable``         SKIP (keeps the host webcam)
===================================== ==========================================

Building
********

From ``zephyr/``, E8 DK RTSS-HP. Pick the sensor that is fitted.

MT9M114::

   west build -p always -d build-uvc-mt9 \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/usb/uvc \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_mt9m114.overlay \
     -DOVERLAY_CONFIG=boards/mt9m114.conf

ARX3A0::

   west build -p always -d build-uvc-arx \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/usb/uvc \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_arx3a0.overlay \
     -DOVERLAY_CONFIG=boards/arx3a0.conf

OV5675::

   west build -p always -d build-uvc-ov \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/tests/drivers/usb/uvc \
     -- \
     -DEXTRA_DTC_OVERLAY_FILE=boards/alif_e8_dk_ae822fa0e5597xx0_rtss_hp_ov5675.overlay \
     -DOVERLAY_CONFIG=boards/ov5675.conf

Host
****

Connect the USB device port. After the suite passes, use the size for the
sensor that was built::

   v4l2-ctl --list-devices
   ffplay -f v4l2 -input_format mjpeg -video_size 1280x720 /dev/videoN
   ffplay -f v4l2 -input_format mjpeg -video_size 560x560 /dev/videoN
   ffplay -f v4l2 -input_format mjpeg -video_size 1024x768 /dev/videoN

UART should show ``Ready to transfer 'JPEG'`` at that sensor's size from
``usbd_uvc`` and ``Capture session started``.

TestLink: ``testlink/testlink_uvc.xml``.
