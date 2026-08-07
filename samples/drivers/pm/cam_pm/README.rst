.. _cam-pm-states-sample:

Alif Power Management States Demo for Camera
############################################

Overview
********

This sample demonstrates Zephyr power management states for camera on Alif RTSS cores, showcasing
different PM state transitions with RTC wakeup capabilities. The sample illustrates:

PM states exercised (determined at runtime by capability predicates):

- **S2RAM path** (TCM or SRAM0 retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  S2RAM STANDBY → S2RAM STOP → idle loop
- **SOFT_OFF path** (MRAM boot, no retention): RUNTIME_IDLE → SUSPEND_TO_IDLE →
  SOFT_OFF (system resets on wakeup)


Requirements
************

* Alif Ensemble development board
* RTC peripheral enabled for wakeup
* SE Services for power profile configuration

Supported Boards
****************

* alif_e8_dk_rtss_he
* alif_e8_dk_rtss_hp (HP core)

Building and Running
********************

Build and flash the sample as follows, replacing the board name with
your target board.

HE Core — ARX3A0 Sensor TCM boot S2RAM (E8)
=========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/cam_pm
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: devkit-he-arx3a0-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

HP Core — ARX3A0 Sensor MRAM boot SOFT_OFF (E8)
=============================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/cam_pm
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: devkit-hp-arx3a0-mram

HE Core — OV5675 Sensor TCM boot S2RAM (E8)
=========================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/cam_pm
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :west-args: -p auto
   :snippets: devkit-he-ov5675-tcm
   :gen-args: -DCONFIG_FLASH_BASE_ADDRESS=0x0 -DCONFIG_FLASH_LOAD_OFFSET=0x0 -DCONFIG_FLASH_SIZE=256

HP Core — OV5675 Sensor MRAM boot SOFT_OFF (E8)
=============================================

.. zephyr-app-commands::
   :zephyr-app: ../alif/samples/drivers/pm/cam_pm
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :west-args: -p auto
   :snippets: devkit-hp-ov5675-mram

Sample Output
*************

HE Core - ARX3A0 Sensor TCM Boot
================================

.. code-block:: console

	[00:00:00.000,000] <dbg> dphy_dw: dphy_dw_init: MMIO Address expmst: 0x4903f000
	[00:00:00.000,000] <dbg> dphy_dw: dphy_dw_init: MMIO Address dsi: 0x49032000
	[00:00:00.000,000] <dbg> dphy_dw: dphy_dw_init: MMIO Address csi: 0x49033000
	[00:00:00.000,000] <dbg> dphy_dw: dphy_dw_init: Config Clk: 25000000 Ref Clk: 38400000
	[00:00:00.000,000] <inf> csi2_dw: #rx_dphy_ids: 1
	*** Booting Zephyr OS build f5f7e7ad63ea ***
	[00:00:00.027,000] <inf> cam_pm: alif_e8_dk (S2RAM): CAM PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
	[00:00:00.027,000] <inf> cam_pm: CAM: PM states demo WITH Camera Capture
	[00:00:00.027,000] <inf> cam_pm: POWER STATE SEQUENCE:
	[00:00:00.027,000] <inf> cam_pm:   1. PM_STATE_RUNTIME_IDLE
	[00:00:00.027,000] <inf> cam_pm:   2. PM_STATE_SUSPEND_TO_IDLE
	[00:00:00.027,000] <inf> cam_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
	[00:00:00.027,000] <inf> cam_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
	[00:00:00.027,000] <inf> cam_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
	[00:00:00.027,000] <inf> cam_pm: skip starting a new capture cycle this round
	[00:00:00.027,000] <inf> cam_pm: - Device name: isp@49046000
	[00:00:00.027,000] <inf> cam_pm: Selected camera: Selfie
	[00:00:00.027,000] <inf> cam_pm: - Capabilities:
	[00:00:00.027,000] <inf> cam_pm:   Y10P width (min, max, step)[1296; 1296; 0] height (min, max, step)[972; 972; 0]
	[00:00:00.027,000] <inf> cam_pm: Setting format: Y10P 1296x972 pitch=2592 on ep=-3
	[00:00:00.054,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:00:00.054,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:00:00.055,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY state: 0xf, DPHY ID0
	[00:00:00.055,000] <inf> cam_pm: - format: PRGB 480x480
	[00:00:00.055,000] <inf> cam_pm: Width - 480, Pitch - 1440, Height - 480, Buff size - 691200
	[00:00:00.055,000] <inf> cam_pm: - addr - 0x2000060, size - 691200, bytesused - 0
	[00:00:00.086,000] <inf> cam_pm: capture buffer[0]: dump binary memory "/home/$USER/capture_0.bin" 0x02000060 0x020a8c5f -r
	[00:00:00.086,000] <inf> cam_pm: - addr - 0x20a8c68, size - 691200, bytesused - 0
	[00:00:00.118,000] <inf> cam_pm: capture buffer[1]: dump binary memory "/home/$USER/capture_1.bin" 0x020a8c68 0x02151867 -r
	[00:00:07.119,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:00:07.119,000] <inf> cam_pm: Camera thread is now suspended (polling for resume)
	[00:00:07.119,000] <inf> ov5675: PM: Suspended ov5675_selfie@10
	[00:00:07.119,000] <inf> cam_pm: Camera Capture is Suspended
	[00:00:25.120,000] <inf> cam_pm: Exited from RUNTIME_IDLE sleep
	[00:00:25.120,000] <inf> cam_pm: Camera: Try to Resume...
	[00:00:25.120,000] <inf> cam_pm: Camera: resume signal sent
	[00:00:25.120,000] <inf> cam_pm: Camera Capture is starting...
	[00:00:25.120,000] <inf> cam_pm: Camera: Woken up by PM resume
	[00:00:25.120,000] <inf> ISP: PM: Suspended isp@49046000
	[00:00:25.120,000] <inf> ISP: PM: Resumed isp@49046000
	[00:00:25.120,000] <inf> cam_pm: Resuming sensor device via PM
	[00:00:25.217,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:00:25.217,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:00:25.218,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY state: 0xf, DPHY ID0
	[00:00:25.218,000] <inf> cam_pm: video_set_format returned: 0
	[00:00:25.218,000] <inf> cam_pm: Waiting for sensor to stabilize...
	[00:00:26.281,000] <inf> cam_pm: Capture started
	[00:00:26.314,000] <inf> cam_pm: Got frame 0! size: 691200; timestamp 26314 ms
	[00:00:26.345,000] <inf> cam_pm: Got frame 1! size: 691200; timestamp 26345 ms
	[00:00:26.377,000] <inf> cam_pm: Got frame 2! size: 691200; timestamp 26377 ms
	[00:00:26.377,000] <inf> cam_pm: FPS: 31.250000
	[00:00:26.408,000] <inf> cam_pm: Got frame 3! size: 691200; timestamp 26408 ms
	[00:00:26.408,000] <inf> cam_pm: FPS: 32.258065
	[00:00:26.440,000] <inf> cam_pm: Got frame 4! size: 691200; timestamp 26440 ms
	[00:00:26.440,000] <inf> cam_pm: FPS: 31.250000
	[00:00:26.471,000] <inf> cam_pm: Got frame 5! size: 691200; timestamp 26471 ms
	[00:00:26.471,000] <inf> cam_pm: FPS: 32.258065
	[00:00:26.503,000] <inf> cam_pm: Got frame 6! size: 691200; timestamp 26503 ms
	[00:00:26.503,000] <inf> cam_pm: FPS: 31.250000
	[00:00:26.535,000] <inf> cam_pm: Got frame 7! size: 691200; timestamp 26535 ms
	[00:00:26.535,000] <inf> cam_pm: FPS: 31.250000
	[00:00:26.566,000] <inf> cam_pm: Got frame 8! size: 691200; timestamp 26566 ms
	[00:00:26.566,000] <inf> cam_pm: FPS: 32.258065
	[00:00:26.598,000] <inf> cam_pm: Got frame 9! size: 691200; timestamp 26598 ms
	[00:00:26.598,000] <inf> cam_pm: FPS: 31.250000
	[00:00:26.598,000] <inf> cam_pm: Calling video flush.
	[00:00:28.598,000] <inf> cam_pm: Flush done.
	[00:00:28.598,000] <inf> cam_pm: Calling video stream stop.
	[00:00:28.598,000] <inf> cam_pm: Stream stop done.
	[00:00:28.699,000] <inf> cam_pm: Capture completed after 10 frames, waiting for PM cycle
	[00:00:28.699,000] <inf> cam_pm: Camera Capture cycle completed
	[00:00:28.699,000] <inf> cam_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (10000 microseconds)
	[00:00:28.699,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:00:28.710,000] <inf> cam_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
	[00:00:28.710,000] <inf> cam_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (6000000 microseconds)
	[00:00:28.710,000] <inf> cam_pm: Camera thread is now suspended (polling for resume)
	[00:00:28.710,000] <inf> ov5675: PM: Suspended ov5675_selfie@10
	[00:00:28.710,000] <inf> cam_pm: Camera Capture is Suspended: for Deep Sleep
	[00:00:28.774,000] <inf> ISP: PM: Suspended isp@49046000
	[00:00:28.774,000] <inf> power_domain: Disabled power domain 6
	[00:00:28.774,000] <inf> power_domain: Disabled power domain 2
	[00:00:28.777,000] <inf> cam_pm: PM enter: SUSPEND_TO_RAM (substate 0)
	[00:00:28.775,000] <inf> cam_pm: PM wakeup: SUSPEND_TO_RAM (substate 0)
	[00:00:28.775,000] <inf> ISP: PM: Resumed isp@49046000
	[00:00:28.775,000] <inf> cam_pm: PM exit:  SUSPEND_TO_RAM (substate 0)
	[00:00:34.711,000] <inf> cam_pm: Camera: Try to Resume...
	[00:00:34.711,000] <inf> cam_pm: Camera: resume signal sent
	[00:00:34.711,000] <inf> cam_pm: Camera Capture is Resumed: for Deep Sleep
	[00:00:34.711,000] <inf> cam_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
	[00:00:34.711,000] <inf> cam_pm: Camera: Woken up by PM resume
	[00:00:34.711,000] <inf> ISP: PM: Suspended isp@49046000
	[00:00:34.711,000] <inf> ISP: PM: Resumed isp@49046000
	[00:00:34.711,000] <inf> cam_pm: Resuming sensor device via PM
	[00:00:34.811,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:00:34.811,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:00:34.812,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY state: 0xf, DPHY ID0
	[00:00:34.812,000] <inf> cam_pm: video_set_format returned: 0
	[00:00:34.812,000] <inf> cam_pm: Waiting for sensor to stabilize...
	[00:00:35.875,000] <inf> cam_pm: Capture started
	[00:00:35.908,000] <inf> cam_pm: Got frame 0! size: 691200; timestamp 35908 ms
	[00:00:35.939,000] <inf> cam_pm: Got frame 1! size: 691200; timestamp 35939 ms
	[00:00:35.971,000] <inf> cam_pm: Got frame 2! size: 691200; timestamp 35971 ms
	[00:00:35.971,000] <inf> cam_pm: FPS: 31.250000
	[00:00:36.002,000] <inf> cam_pm: Got frame 3! size: 691200; timestamp 36002 ms
	[00:00:36.002,000] <inf> cam_pm: FPS: 32.258065
	[00:00:36.034,000] <inf> cam_pm: Got frame 4! size: 691200; timestamp 36034 ms
	[00:00:36.034,000] <inf> cam_pm: FPS: 31.250000
	[00:00:36.065,000] <inf> cam_pm: Got frame 5! size: 691200; timestamp 36065 ms
	[00:00:36.065,000] <inf> cam_pm: FPS: 32.258065
	[00:00:36.097,000] <inf> cam_pm: Got frame 6! size: 691200; timestamp 36097 ms
	[00:00:36.097,000] <inf> cam_pm: FPS: 31.250000
	[00:00:36.129,000] <inf> cam_pm: Got frame 7! size: 691200; timestamp 36129 ms
	[00:00:36.129,000] <inf> cam_pm: FPS: 31.250000
	[00:00:36.160,000] <inf> cam_pm: Got frame 8! size: 691200; timestamp 36160 ms
	[00:00:36.160,000] <inf> cam_pm: FPS: 32.258065
	[00:00:36.192,000] <inf> cam_pm: Got frame 9! size: 691200; timestamp 36192 ms
	[00:00:36.192,000] <inf> cam_pm: FPS: 31.250000
	[00:00:36.192,000] <inf> cam_pm: Calling video flush.
	[00:00:38.192,000] <inf> cam_pm: Flush done.
	[00:00:38.192,000] <inf> cam_pm: Calling video stream stop.
	[00:00:38.192,000] <inf> cam_pm: Stream stop done.
	[00:00:38.293,000] <inf> cam_pm: Capture completed after 10 frames, waiting for PM cycle
	[00:00:38.293,000] <inf> cam_pm: Camera Capture cycle completed (after STANDBY)
	[00:00:38.293,000] <inf> cam_pm: Main thread running - iteration 0 - tick: 38293
	[00:00:38.293,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:00:40.294,000] <inf> cam_pm: Main thread running - iteration 1 - tick: 40294
	[00:00:42.295,000] <inf> cam_pm: Main thread running - iteration 2 - tick: 42295
	[00:00:44.296,000] <inf> cam_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (9000000 microseconds)
	[00:00:44.296,000] <inf> cam_pm: Camera thread is now suspended (polling for resume)
	[00:00:44.296,000] <inf> ov5675: PM: Suspended ov5675_selfie@10
	[00:00:44.296,000] <inf> cam_pm: Camera Capture is Suspended: for Deep Sleep
	[00:00:44.333,000] <inf> ISP: PM: Suspended isp@49046000
	[00:00:44.333,000] <inf> power_domain: Disabled power domain 6
	[00:00:44.333,000] <inf> power_domain: Disabled power domain 2
	[00:00:44.336,000] <inf> cam_pm: PM enter: SUSPEND_TO_RAM (substate 1)
	[00:00:44.334,000] <inf> cam_pm: PM wakeup: SUSPEND_TO_RAM (substate 1)
	[00:00:44.334,000] <inf> ISP: PM: Resumed isp@49046000
	[00:00:44.334,000] <inf> cam_pm: PM exit:  SUSPEND_TO_RAM (substate 1)
	[00:00:53.297,000] <inf> cam_pm: Camera: Try to Resume...
	[00:00:53.297,000] <inf> cam_pm: Camera: resume signal sent
	[00:00:53.297,000] <inf> cam_pm: Camera Capture is Resumed: for Deep Sleep
	[00:00:53.297,000] <inf> cam_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
	[00:00:53.297,000] <inf> cam_pm: Camera: Woken up by PM resume
	[00:00:53.297,000] <inf> ISP: PM: Suspended isp@49046000
	[00:00:53.297,000] <inf> ISP: PM: Resumed isp@49046000
	[00:00:53.297,000] <inf> cam_pm: Resuming sensor device via PM
	[00:00:53.396,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:00:53.396,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:00:53.396,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY state: 0xf, DPHY ID0
	[00:00:53.396,000] <inf> cam_pm: video_set_format returned: 0
	[00:00:53.396,000] <inf> cam_pm: Waiting for sensor to stabilize...
	[00:00:54.459,000] <inf> cam_pm: Capture started
	[00:00:54.492,000] <inf> cam_pm: Got frame 0! size: 691200; timestamp 54492 ms
	[00:00:54.523,000] <inf> cam_pm: Got frame 1! size: 691200; timestamp 54523 ms
	[00:00:54.555,000] <inf> cam_pm: Got frame 2! size: 691200; timestamp 54555 ms
	[00:00:54.555,000] <inf> cam_pm: FPS: 31.250000
	[00:00:54.586,000] <inf> cam_pm: Got frame 3! size: 691200; timestamp 54586 ms
	[00:00:54.586,000] <inf> cam_pm: FPS: 32.258065
	[00:00:54.618,000] <inf> cam_pm: Got frame 4! size: 691200; timestamp 54618 ms
	[00:00:54.618,000] <inf> cam_pm: FPS: 31.250000
	[00:00:54.649,000] <inf> cam_pm: Got frame 5! size: 691200; timestamp 54649 ms
	[00:00:54.649,000] <inf> cam_pm: FPS: 32.258065
	[00:00:54.681,000] <inf> cam_pm: Got frame 6! size: 691200; timestamp 54681 ms
	[00:00:54.681,000] <inf> cam_pm: FPS: 31.250000
	[00:00:54.713,000] <inf> cam_pm: Got frame 7! size: 691200; timestamp 54713 ms
	[00:00:54.713,000] <inf> cam_pm: FPS: 31.250000
	[00:00:54.744,000] <inf> cam_pm: Got frame 8! size: 691200; timestamp 54744 ms
	[00:00:54.744,000] <inf> cam_pm: FPS: 32.258065
	[00:00:54.776,000] <inf> cam_pm: Got frame 9! size: 691200; timestamp 54776 ms
	[00:00:54.776,000] <inf> cam_pm: FPS: 31.250000
	[00:00:54.776,000] <inf> cam_pm: Calling video flush.
	[00:00:56.776,000] <inf> cam_pm: Flush done.
	[00:00:56.776,000] <inf> cam_pm: Calling video stream stop.
	[00:00:56.776,000] <inf> cam_pm: Stream stop done.
	[00:00:56.877,000] <inf> cam_pm: Capture completed after 10 frames, waiting for PM cycle
	[00:00:56.877,000] <inf> cam_pm: Camera Capture cycle completed (after STOP)
	[00:00:56.877,000] <inf> cam_pm: Main thread running - iteration 0 - tick: 56877
	[00:00:56.877,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:00:58.878,000] <inf> cam_pm: Main thread running - iteration 1 - tick: 58878
	[00:01:00.879,000] <inf> cam_pm: Main thread running - iteration 2 - tick: 60879
	[00:01:02.880,000] <inf> cam_pm: === CAM PM SEQUENCE COMPLETED ===
	[00:01:02.880,000] <inf> cam_pm: Camera: requesting thread stop
	[00:01:02.880,000] <inf> cam_pm: Camera: thread stopping (stop requested during suspend)
	[00:01:02.880,000] <inf> cam_pm: Camera: thread stopped
	[00:01:02.880,000] <inf> cam_pm: Camera Capture is Stopped (PM sequence done)
