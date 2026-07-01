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

HE Core - OV5675 Sensor TCM Boot
================================

.. code-block:: console

	[00:00:00.000,000] <dbg> dphy_dw: dphy_dw_init: MMIO Address expmst: 0x4903f000
	[00:00:00.000,000] <dbg> dphy_dw: dphy_dw_init: MMIO Address dsi: 0x49032000
	[00:00:00.000,000] <dbg> dphy_dw: dphy_dw_init: MMIO Address csi: 0x49033000
	[00:00:00.000,000] <dbg> dphy_dw: dphy_dw_init: Config Clk: 25000000 Ref Clk: 38400000
	[00:00:00.000,000] <inf> csi2_dw: #rx_dphy_ids: 1
	*** Booting Zephyr OS build 6a4e3155624e ***
	[00:00:00.027,000] <inf> cam_pm: alif_e8_dk (S2RAM): CAM PM demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY,
	S2RAM STOP)
	[00:00:00.027,000] <inf> cam_pm: CAM: PM states demo WITH Camera Capture
	[00:00:00.027,000] <inf> cam_pm: POWER STATE SEQUENCE:
	[00:00:00.027,000] <inf> cam_pm:   1. PM_STATE_RUNTIME_IDLE
	[00:00:00.027,000] <inf> cam_pm:   2. PM_STATE_SUSPEND_TO_IDLE
	[00:00:00.027,000] <inf> cam_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
	[00:00:00.027,000] <inf> cam_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
	[00:00:00.027,000] <inf> cam_pm: Enter RUNTIME_IDLE sleep for (10000000 microseconds)
	[00:00:00.027,000] <inf> cam_pm: skip starting a new capture cycle this round
	[00:00:00.027,000] <inf> cam_pm: - Device name: isp@49046000
	[00:00:00.027,000] <inf> cam_pm: Selected camera: Selfie
	[00:00:00.027,000] <inf> cam_pm: - Capabilities:
	[00:00:00.027,000] <inf> cam_pm:   Y10P width (min, max, step)[1296; 1296; 0] height (min, max, step)[972; 972; 0]
	[00:00:00.027,000] <inf> cam_pm:   Y10P width (min, max, step)[1920; 1920; 0] height (min, max, step)[1080; 1080; 0]
	[00:00:00.027,000] <inf> cam_pm:   Y10P width (min, max, step)[1280; 1280; 0] height (min, max, step)[720; 720; 0]
	[00:00:00.027,000] <inf> cam_pm:   Y10P width (min, max, step)[640; 640; 0] height (min, max, step)[480; 480; 0]
	[00:00:00.027,000] <inf> cam_pm: Setting format: Y10P 640x480 pitch=1280 on ep=-3
	[00:00:00.054,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:00:00.054,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:00:00.055,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY
	state: 0xf,DPHY ID: 0
	[00:00:00.055,000] <inf> cam_pm: - format: PRGB 480x480
	[00:00:00.055,000] <inf> cam_pm: Width - 480, Pitch - 1440, Height - 480, Buff size - 691200
	[00:00:00.055,000] <inf> cam_pm: - addr - 0x2000060, size - 691200, bytesused - 0
	[00:00:00.086,000] <inf> cam_pm: capture buffer[0]: dump binary memory "/home/$USER/capture_0.bin" 0x02000060
	0x020a8c5f -r
	[00:00:00.086,000] <inf> cam_pm: - addr - 0x20a8c68, size - 691200, bytesused - 0
	[00:00:00.118,000] <inf> cam_pm: capture buffer[1]: dump binary memory "/home/$USER/capture_1.bin" 0x020a8c68
	0x02151867 -r
	[00:00:07.119,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:00:07.119,000] <inf> cam_pm: Camera thread is now suspended (polling for resume)
	[00:00:07.119,000] <inf> cam_pm: Camera Capture is Suspended
	[00:00:17.120,000] <inf> cam_pm: Exited from RUNTIME_IDLE sleep
	[00:00:17.120,000] <inf> cam_pm: Camera: Try to Resume...
	[00:00:17.120,000] <inf> cam_pm: Camera: resume signal sent
	[00:00:17.120,000] <inf> cam_pm: Camera Capture is starting...
	[00:00:17.120,000] <inf> cam_pm: Camera: Woken up by PM resume
	[00:00:17.120,000] <inf> cam_pm: Waiting for sensor to stabilize...
	[00:00:18.184,000] <inf> cam_pm: Capture started
	[00:00:18.224,000] <inf> cam_pm: Got frame 0! size: 691200; timestamp 18224 ms
	[00:00:18.255,000] <inf> cam_pm: Got frame 1! size: 691200; timestamp 18255 ms
	[00:00:18.287,000] <inf> cam_pm: Got frame 2! size: 691200; timestamp 18287 ms
	[00:00:18.287,000] <inf> cam_pm: FPS: 31.2
	[00:00:18.318,000] <inf> cam_pm: Got frame 3! size: 691200; timestamp 18318 ms
	[00:00:18.318,000] <inf> cam_pm: FPS: 32.2
	[00:00:18.350,000] <inf> cam_pm: Got frame 4! size: 691200; timestamp 18350 ms
	[00:00:18.350,000] <inf> cam_pm: FPS: 31.2
	[00:00:18.381,000] <inf> cam_pm: Got frame 5! size: 691200; timestamp 18381 ms
	[00:00:18.381,000] <inf> cam_pm: FPS: 32.2
	[00:00:18.413,000] <inf> cam_pm: Got frame 6! size: 691200; timestamp 18413 ms
	[00:00:18.413,000] <inf> cam_pm: FPS: 31.2
	[00:00:18.445,000] <inf> cam_pm: Got frame 7! size: 691200; timestamp 18445 ms
	[00:00:18.445,000] <inf> cam_pm: FPS: 31.2
	[00:00:18.476,000] <inf> cam_pm: Got frame 8! size: 691200; timestamp 18476 ms
	[00:00:18.476,000] <inf> cam_pm: FPS: 32.2
	[00:00:18.508,000] <inf> cam_pm: Got frame 9! size: 691200; timestamp 18508 ms
	[00:00:18.508,000] <inf> cam_pm: FPS: 31.2
	[00:00:18.508,000] <inf> cam_pm: Calling video flush.
	[00:00:20.508,000] <inf> cam_pm: Flush done.
	[00:00:20.508,000] <inf> cam_pm: Calling video stream stop.
	[00:00:20.508,000] <inf> cam_pm: Stream stop done.
	[00:00:20.609,000] <inf> cam_pm: Capture completed after 10 frames, waiting for PM cycle
	[00:00:20.609,000] <inf> cam_pm: Camera Capture cycle completed
	[00:00:20.609,000] <inf> cam_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (10000 microseconds)
	[00:00:20.609,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:00:20.620,000] <inf> cam_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
	[00:00:20.620,000] <inf> cam_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (6000000 microseconds)
	[00:00:20.620,000] <inf> cam_pm: Camera Capture is Suspended: for Deep Sleep
	[00:00:20.669,000] <inf> ISP: PM: Suspended isp@49046000
	[00:00:20.669,000] <dbg> dphy_dw: dphy_dw_pm_action: PM: Suspended d-phy@4903f000
	[00:00:20.670,000] <inf> power_domain: Disabled power domain 6
	[00:00:20.670,000] <inf> power_domain: Disabled power domain 2
	[00:00:20.672,000] <inf> cam_pm: PM enter: SUSPEND_TO_RAM (substate 0)
	[00:00:20.671,000] <inf> cam_pm: PM wakeup: SUSPEND_TO_RAM (substate 0)
	[00:00:20.671,000] <dbg> dphy_dw: dphy_dw_pm_action: PM: Resumed d-phy@4903f000
	[00:00:20.671,000] <inf> ISP: PM: Resumed isp@49046000
	[00:00:20.671,000] <inf> cam_pm: PM exit:  SUSPEND_TO_RAM (substate 0)
	[00:00:26.621,000] <inf> cam_pm: Camera: Try to Resume...
	[00:00:26.621,000] <inf> cam_pm: Camera: resume signal sent
	[00:00:26.621,000] <inf> cam_pm: Camera Capture is Resumed: for Deep Sleep
	[00:00:26.621,000] <inf> cam_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
	[00:00:26.621,000] <inf> cam_pm: Camera: Woken up by PM resume
	[00:00:26.674,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:00:26.674,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:00:26.675,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY
	state: 0xf,DPHY ID: 0
	[00:00:26.675,000] <inf> cam_pm: Waiting for sensor to stabilize...
	[00:00:27.738,000] <inf> cam_pm: Capture started
	[00:00:27.779,000] <inf> cam_pm: Got frame 0! size: 691200; timestamp 27779 ms
	[00:00:27.810,000] <inf> cam_pm: Got frame 1! size: 691200; timestamp 27810 ms
	[00:00:27.842,000] <inf> cam_pm: Got frame 2! size: 691200; timestamp 27842 ms
	[00:00:27.842,000] <inf> cam_pm: FPS: 31.2
	[00:00:27.873,000] <inf> cam_pm: Got frame 3! size: 691200; timestamp 27873 ms
	[00:00:27.873,000] <inf> cam_pm: FPS: 32.2
	[00:00:27.905,000] <inf> cam_pm: Got frame 4! size: 691200; timestamp 27905 ms
	[00:00:27.905,000] <inf> cam_pm: FPS: 31.2
	[00:00:27.936,000] <inf> cam_pm: Got frame 5! size: 691200; timestamp 27936 ms
	[00:00:27.936,000] <inf> cam_pm: FPS: 32.2
	[00:00:27.968,000] <inf> cam_pm: Got frame 6! size: 691200; timestamp 27968 ms
	[00:00:27.968,000] <inf> cam_pm: FPS: 31.2
	[00:00:28.000,000] <inf> cam_pm: Got frame 7! size: 691200; timestamp 28000 ms
	[00:00:28.000,000] <inf> cam_pm: FPS: 31.2
	[00:00:28.031,000] <inf> cam_pm: Got frame 8! size: 691200; timestamp 28031 ms
	[00:00:28.031,000] <inf> cam_pm: FPS: 32.2
	[00:00:28.063,000] <inf> cam_pm: Got frame 9! size: 691200; timestamp 28063 ms
	[00:00:28.063,000] <inf> cam_pm: FPS: 31.2
	[00:00:28.063,000] <inf> cam_pm: Calling video flush.
	[00:00:30.063,000] <inf> cam_pm: Flush done.
	[00:00:30.063,000] <inf> cam_pm: Calling video stream stop.
	[00:00:30.063,000] <inf> cam_pm: Stream stop done.
	[00:00:30.164,000] <inf> cam_pm: Capture completed after 10 frames, waiting for PM cycle
	[00:00:30.164,000] <inf> cam_pm: Camera Capture cycle completed (after STANDBY)
	[00:00:30.164,000] <inf> cam_pm: Main thread running - iteration 0 - tick: 30164
	[00:00:30.164,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:00:32.165,000] <inf> cam_pm: Main thread running - iteration 1 - tick: 32165
	[00:00:34.166,000] <inf> cam_pm: Main thread running - iteration 2 - tick: 34166
	[00:00:36.167,000] <inf> cam_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (9000000 microseconds)
	[00:00:36.167,000] <inf> cam_pm: Camera Capture is Suspended: for Deep Sleep
	[00:00:36.189,000] <inf> ISP: PM: Suspended isp@49046000
	[00:00:36.189,000] <dbg> dphy_dw: dphy_dw_pm_action: PM: Suspended d-phy@4903f000
	[00:00:36.190,000] <inf> power_domain: Disabled power domain 6
	[00:00:36.190,000] <inf> power_domain: Disabled power domain 2
	[00:00:36.192,000] <inf> cam_pm: PM enter: SUSPEND_TO_RAM (substate 1)
	[00:00:36.190,000] <inf> cam_pm: PM wakeup: SUSPEND_TO_RAM (substate 1)
	[00:00:36.190,000] <dbg> dphy_dw: dphy_dw_pm_action: PM: Resumed d-phy@4903f000
	[00:00:36.190,000] <inf> ISP: PM: Resumed isp@49046000
	[00:00:36.190,000] <inf> cam_pm: PM exit:  SUSPEND_TO_RAM (substate 1)
	[00:00:45.168,000] <inf> cam_pm: Camera: Try to Resume...
	[00:00:45.168,000] <inf> cam_pm: Camera: resume signal sent
	[00:00:45.168,000] <inf> cam_pm: Camera Capture is Resumed: for Deep Sleep
	[00:00:45.168,000] <inf> cam_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
	[00:00:45.168,000] <inf> cam_pm: Camera: Woken up by PM resume
	[00:00:45.221,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:00:45.221,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:00:45.222,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY
	state: 0xf,DPHY ID: 0
	[00:00:45.222,000] <inf> cam_pm: Waiting for sensor to stabilize...
	[00:00:46.285,000] <inf> cam_pm: Capture started
	[00:00:46.326,000] <inf> cam_pm: Got frame 0! size: 691200; timestamp 46326 ms
	[00:00:46.357,000] <inf> cam_pm: Got frame 1! size: 691200; timestamp 46357 ms
	[00:00:46.389,000] <inf> cam_pm: Got frame 2! size: 691200; timestamp 46389 ms
	[00:00:46.389,000] <inf> cam_pm: FPS: 31.2
	[00:00:46.420,000] <inf> cam_pm: Got frame 3! size: 691200; timestamp 46420 ms
	[00:00:46.420,000] <inf> cam_pm: FPS: 32.2
	[00:00:46.452,000] <inf> cam_pm: Got frame 4! size: 691200; timestamp 46452 ms
	[00:00:46.452,000] <inf> cam_pm: FPS: 31.2
	[00:00:46.483,000] <inf> cam_pm: Got frame 5! size: 691200; timestamp 46483 ms
	[00:00:46.483,000] <inf> cam_pm: FPS: 32.2
	[00:00:46.515,000] <inf> cam_pm: Got frame 6! size: 691200; timestamp 46515 ms
	[00:00:46.515,000] <inf> cam_pm: FPS: 31.2
	[00:00:46.547,000] <inf> cam_pm: Got frame 7! size: 691200; timestamp 46547 ms
	[00:00:46.547,000] <inf> cam_pm: FPS: 31.2
	[00:00:46.578,000] <inf> cam_pm: Got frame 8! size: 691200; timestamp 46578 ms
	[00:00:46.578,000] <inf> cam_pm: FPS: 32.2
	[00:00:46.610,000] <inf> cam_pm: Got frame 9! size: 691200; timestamp 46610 ms
	[00:00:46.610,000] <inf> cam_pm: FPS: 31.2
	[00:00:46.610,000] <inf> cam_pm: Calling video flush.
	[00:00:48.610,000] <inf> cam_pm: Flush done.
	[00:00:48.610,000] <inf> cam_pm: Calling video stream stop.
	[00:00:48.610,000] <inf> cam_pm: Stream stop done.
	[00:00:48.711,000] <inf> cam_pm: Capture completed after 10 frames, waiting for PM cycle
	[00:00:48.711,000] <inf> cam_pm: Camera Capture cycle completed (after STOP)
	[00:00:48.711,000] <inf> cam_pm: Main thread running - iteration 0 - tick: 48711
	[00:00:48.711,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:00:50.712,000] <inf> cam_pm: Main thread running - iteration 1 - tick: 50712
	[00:00:52.713,000] <inf> cam_pm: Main thread running - iteration 2 - tick: 52713
	[00:00:54.714,000] <inf> cam_pm: === CAM PM SEQUENCE COMPLETED ===
	[00:00:54.714,000] <inf> cam_pm: Camera: requesting thread stop
	[00:00:54.714,000] <inf> cam_pm: Camera: thread stopping (suspend)
	[00:00:54.714,000] <inf> cam_pm: Camera: thread stopped
	[00:00:54.714,000] <inf> cam_pm: Camera Capture is Stopped (PM sequence done)
