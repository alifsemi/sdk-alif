.. _video-sample:

Video Sample
#############################################

Overview
********

This sample can be used to capture frame using Parallel-bus or CSI camera sensors, and store it to
the memory. The sample also supports ISP in the video pipeline. The functional behaviour is:

* Camera sensor will send out frames along with synchronisation signals either to CAM/LPCAM
  controller directly (in case of the parallel camera sensor eg. MT9M114), or to Camera Serial
  Interface MIPI CSI2 and then to CAM controller (in case of serial camera sensor eg. ARX3A0).
* MIPI CSI2 uses DPHY interface to receive data serially from serial camera sensor and convert it
  to parallel data since CAM/LPCAM controller has parallel interface.
* CAM/LPCAM controller will convert the captured frame with desired features and save it into the
  memory.
* In case ISP is selected (serial_camera_arx3a0_selfie.overlay), Camera controller will pass the
  frame to ISP IP. The ISP then writes the processed frame to memory.

* In case JPEG is selected (jpeg.overlay and jpeg.conf), the ISP's processed
  output frame will be considerd for compression.Then the JPEG writes
  the compressed frame to output memory.

For E7 use serial_camera_arx3a0.overlay, while for E8 use serial_camera_arx3a0_standard.overlay,
in case ISP is not needed and Standard camera is required. For selfie camera with ISP on E8, use
serial_camera_arx3a0_selfie.overlay and isp.conf as OVERLAY_CONFIG. In case selfie camera without
ISP is needed, remove isp node from serial_camera_arx3a0_selfie.overlay.


Requirements
************

The sample utilizes the CAM Controller IP from alif and a camera sensor. It may also use the
MIPI-CSI2 IP from Synopsys if the serial camera sensor application is built from the command line.
The camera sensors used in the parallel camera case is the MT9M114, while serial camera include
ARX3A0 camera sensor.

The JPEG encoder is supported only on the alif_e8 board and
can be selected during the build process only for this target.

  - The JPEG module consumes one buffer from the video buffer pool.
    Since N_VID_BUFF requires a minimum of 2 buffers, the total buffer pool size
    (CONFIG_VIDEO_BUFFER_POOL_NUM_MAX) must be at least 3.

  - For a resolution of 480 × 480 with an NV12 input format,
    the minimum required buffer size (CONFIG_VIDEO_BUFFER_POOL_SZ_MAX) is 346223 bytes.

Supported Targets
*****************

* alif_e7_dk/rtss_hp
* alif_e7_dk/rtss_he
* alif_e1c_dk/rtss_he
* alif_b1_dk/rtss_he
* alif_e8_dk/rtss_hp
* alif_e8_dk/rtss_he

Tested Sensors
**************

* MT9M114 (Parallel)
* ARX3A0 (CSI)
* HM0360 (CSI)

Sample Output
*************

1. When Jpeg encoder is included:

.. code-block:: console
  [00:00:00.000,000] <inf> csi2_dw: #rx_dphy_ids: 1
  [00:00:00.664,000] <inf> jpeg_hantro_vc9000e: VeriSilicon Hantro VC9000E JPEG encoder initialized
  *** Booting Zephyr OS build 7ef5799d8b5a ***
  [00:00:00.664,000] <inf> video_app: - Device name: isp@49046000
  [00:00:00.664,000] <inf> video_app: Selected camera: Selfie
  [00:00:00.664,000] <inf> video_app: - Capabilities:

  [00:00:00.664,000] <inf> video_app:   Y10P width (min, max, step)[560; 560; 0] height (min, max, step)[560; 560; 0]
  [00:00:01.204,000] <inf> dphy_dw: RX-DDR clock: 400000000
  [00:00:01.204,000] <inf> video_app: - format: NV12 480x480
  [00:00:01.204,000] <inf> video_app: Width - 480, Pitch - 720, Height - 480, Buff size - 345600
  [00:00:01.204,000] <inf> video_app: JPEG: device ready: jpeg@49044000
  [00:00:01.204,000] <inf> video_app: JPEG: Encoder Capabilities:
  [00:00:01.204,000] <inf> video_app:   Format: 0x3231564e, Size: 32x32 to 16384x16384
  [00:00:01.204,000] <inf> video_app:   Format: 0x3132564e, Size: 32x32 to 16384x16384
  [00:00:01.204,000] <inf> video_app: Jpeg: Outbuf allocated at 0x02000060 with 231023 bytes

  [00:00:01.209,000] <inf> video_app: - addr - 0x20386d8, size - 345600, bytesused - 0
  [00:00:01.215,000] <inf> video_app: capture buffer[0]: dump binary memory "/home/$USER/capture_0.bin" 0x020386d8 0x0208ccd7 -r

  [00:00:01.215,000] <inf> video_app: - addr - 0x208cce0, size - 345600, bytesused - 0
  [00:00:01.221,000] <inf> video_app: capture buffer[1]: dump binary memory "/home/$USER/capture_1.bin" 0x0208cce0 0x020e12df -r

  [00:00:08.223,000] <inf> video_app: Capture started
  [00:00:08.290,000] <inf> video_app: Got frame 0! size: 345600; timestamp 8290 ms
  [00:00:08.291,000] <inf> video_app: FPS: 0.0
  [00:00:08.291,000] <inf> video_app: Starting JPEG encoding...
  [00:00:08.291,000] <inf> video_app: Jpeg: Format set: 480x480, format: NV12
  [00:00:08.291,000] <inf> video_app: === JPEG Encoding Success===
  [00:00:08.291,000] <inf> video_app: Jpeg: Capture Image: dump memory "/home/$USER/capture_cp_0.jpg" 0x02000060 0x02002e4a

  [00:00:08.491,000] <inf> video_app: Got frame 1! size: 345600; timestamp 8491 ms
  [00:00:08.491,000] <inf> video_app: FPS: 4.975124
  [00:00:08.491,000] <inf> video_app: Starting JPEG encoding...
  [00:00:08.491,000] <inf> video_app: Jpeg: Format set: 480x480, format: NV12
  [00:00:08.491,000] <inf> video_app: === JPEG Encoding Success===
  [00:00:08.491,000] <inf> video_app: Jpeg: Capture Image: dump memory "/home/$USER/capture_cp_1.jpg" 0x02000060 0x02002f94

  [00:00:08.691,000] <inf> video_app: Got frame 2! size: 345600; timestamp 8691 ms
  [00:00:08.691,000] <inf> video_app: FPS: 5.000000
  [00:00:08.691,000] <inf> video_app: Starting JPEG encoding...
  [00:00:08.691,000] <inf> video_app: Jpeg: Format set: 480x480, format: NV12
  [00:00:08.691,000] <inf> video_app: === JPEG Encoding Success===
  [00:00:08.691,000] <inf> video_app: Jpeg: Capture Image: dump memory "/home/$USER/capture_cp_2.jpg" 0x02000060 0x02002f58

  [00:00:08.891,000] <inf> video_app: Got frame 3! size: 345600; timestamp 8891 ms
  [00:00:08.891,000] <inf> video_app: FPS: 5.000000
  [00:00:08.891,000] <inf> video_app: Starting JPEG encoding...
  [00:00:08.891,000] <inf> video_app: Jpeg: Format set: 480x480, format: NV12
  [00:00:08.891,000] <inf> video_app: === JPEG Encoding Success===
  [00:00:08.891,000] <inf> video_app: Jpeg: Capture Image: dump memory "/home/$USER/capture_cp_3.jpg" 0x02000060 0x02002fd6

  [00:00:09.091,000] <inf> video_app: Got frame 4! size: 345600; timestamp 9091 ms
  [00:00:09.091,000] <inf> video_app: FPS: 5.000000
  [00:00:09.091,000] <inf> video_app: Starting JPEG encoding...
  [00:00:09.091,000] <inf> video_app: Jpeg: Format set: 480x480, format: NV12
  [00:00:09.091,000] <inf> video_app: === JPEG Encoding Success===
  [00:00:09.091,000] <inf> video_app: Jpeg: Capture Image: dump memory "/home/$USER/capture_cp_4.jpg" 0x02000060 0x02002fa9

  [00:00:09.091,000] <inf> video_app: Jpeg: Output Buffer released
  [00:00:09.091,000] <inf> video_app: Calling video flush.
  [00:00:09.091,000] <inf> video_app: Calling video stream stop.

2. When Jpeg encoder is not included:

.. code-block:: console

  [00:00:00.000,000] <inf> csi2_dw: #rx_dphy_ids: 1
  *** Booting Zephyr OS build 9951f106e882 ***
  [00:00:00.662,000] <inf> video_app: - Device name: cam@49030000
  [00:00:00.662,000] <inf> video_app: - Capabilities:

  [00:00:00.662,000] <inf> video_app:   Y10P width (min, max, step)[560; 560; 0] height (min, max, step)[560; 560; 0]
  [00:00:01.202,000] <inf> dphy_dw: RX-DDR clock: 400000000
  [00:00:01.203,000] <inf> video_app: - format: Y10P 560x560
  [00:00:01.203,000] <inf> video_app: Width - 560, Pitch - 1120, Height - 560, Buff size - 627200
  [00:00:01.203,000] <inf> video_app: - addr - 0x2000060, size - 627200, bytesused - 0
  [00:00:01.231,000] <inf> video_app: capture buffer[0]: dump binary memory "/home/$USER/capture_0.bin" 0x02000060 0x0209925f -r

  [00:00:01.231,000] <inf> video_app: - addr - 0x2099268, size - 627200, bytesused - 0
  [00:00:01.258,000] <inf> video_app: capture buffer[1]: dump binary memory "/home/$USER/capture_1.bin" 0x02099268 0x02132467 -r

  [00:00:08.260,000] <inf> video_app: Capture started
  [00:00:08.328,000] <inf> video_app: Got frame 0! size: 627200; timestamp 8328 ms
  [00:00:08.328,000] <inf> video_app: FPS: 0.0
  [00:00:08.528,000] <inf> video_app: Got frame 1! size: 627200; timestamp 8528 ms
  [00:00:08.528,000] <inf> video_app: FPS: 5.000000
  [00:00:08.728,000] <inf> video_app: Got frame 2! size: 627200; timestamp 8728 ms
  [00:00:08.728,000] <inf> video_app: FPS: 5.000000
  [00:00:08.928,000] <inf> video_app: Got frame 3! size: 627200; timestamp 8928 ms
  [00:00:08.928,000] <inf> video_app: FPS: 5.000000
  [00:00:09.128,000] <inf> video_app: Got frame 4! size: 627200; timestamp 9128 ms
  [00:00:09.128,000] <inf> video_app: FPS: 5.000000
  [00:00:09.328,000] <inf> video_app: Got frame 5! size: 627200; timestamp 9328 ms
  [00:00:09.328,000] <inf> video_app: FPS: 5.000000
  [00:00:09.528,000] <inf> video_app: Got frame 6! size: 627200; timestamp 9528 ms
  [00:00:09.528,000] <inf> video_app: FPS: 5.000000
  [00:00:09.728,000] <inf> video_app: Got frame 7! size: 627200; timestamp 9728 ms
  [00:00:09.728,000] <inf> video_app: FPS: 5.000000
  [00:00:09.928,000] <inf> video_app: Got frame 8! size: 627200; timestamp 9928 ms
  [00:00:09.928,000] <inf> video_app: FPS: 5.000000
  [00:00:10.128,000] <inf> video_app: Got frame 9! size: 627200; timestamp 10128 ms
  [00:00:10.128,000] <inf> video_app: FPS: 5.000000
  [00:00:10.128,000] <inf> video_app: Calling video flush.
  [00:00:10.128,000] <inf> video_app: Calling video stream stop.
