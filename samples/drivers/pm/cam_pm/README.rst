.. _cam-pm-states-sample:

Alif Power Management States Demo for Camera
############################################

Overview
********

This sample demonstrates Zephyr power management states for camera on Alif RTSS cores, showcasing
different PM state transitions with RTC wakeup capabilities. The sample illustrates:

* **PM_STATE_SUSPEND_TO_IDLE**: CPU sleep with IWIC (Internal WIC), devices remain active (no device PM overhead)
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention (HE core only)

  * Substate 0 (STANDBY): Medium power savings
  * Substate 1 (STOP): Higher power savings

* **PM_STATE_SOFT_OFF**: Deepest sleep, no retention, full system reset on wakeup

The behavior differs between HP and HE cores:

**HE Core (with retention support)**:

* When booting from **TCM** (VTOR = 0x0):

  * Demonstrates RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP
  * Skips SOFT_OFF (uses retention instead)
  * Resumes execution after each state

* When booting from **MRAM** (VTOR >= 0x80000000):

  * Demonstrates RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF
  * System resets and restarts from main() after SOFT_OFF

**HP Core (no retention support)**:

* Only SOFT_OFF is available for deeper sleep (no S2RAM support)
* Demonstrates RUNTIME_IDLE, then SOFT_OFF
* System resets and restarts from main() after SOFT_OFF

Requirements
************

* Alif Ensemble or Balletto development board
* RTC peripheral enabled for wakeup
* SE Services for power profile configuration

Supported Boards
****************

* alif_e7_dk_rtss_he
* alif_e1c_dk_rtss_he
* alif_b1_dk_rtss_he
* alif_e8_dk_rtss_he
* alif_e7_dk_rtss_hp (HP core)
* alif_e8_dk_rtss_hp (HP core)

Building and Running
********************

Build and flash the sample as follows, replacing the board name with
your target board.

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/pm/cam_pm
   :board: alif_e8_dk/ae822fa0e5597xx0/rtss_he
   :goals: build
   :gen-args: -S alif-dk

Flash the binary using SE Tools. See :ref:`programming_an_application` for details.

Sample Output
*************

HE Core - TCM Boot (with retention, SOFT_OFF skipped)
======================================================

.. code-block:: console

	*** Booting Zephyr OS build 120cef2bee8a ***
	[00:00:00.711,000] <inf> cam_pm: alif_e8_dk RTSS_HE (TCM boot): PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM)
	[00:00:00.722,000] <inf> cam_pm: RTSS_HE: PM states demo WITH Camera Capture
	[00:00:00.730,000] <inf> cam_pm: POWER STATE SEQUENCE:
	[00:00:00.735,000] <inf> cam_pm:   1. PM_STATE_RUNTIME_IDLE
	[00:00:00.741,000] <inf> cam_pm:   2. PM_STATE_SUSPEND_TO_IDLE
	[00:00:00.748,000] <inf> cam_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
	[00:00:00.756,000] <inf> cam_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
	[00:00:00.764,000] <inf> cam_pm:   5. (SOFT_OFF skipped - TCM boot, using retention)
	[00:00:00.772,000] <inf> cam_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
	[00:00:00.780,000] <inf> cam_pm: - Device name: cam@49030000
	[00:00:00.786,000] <inf> cam_pm: Selected camera: Selfie
	[00:00:00.792,000] <inf> cam_pm: - Capabilities:
	[00:00:00.797,000] <inf> cam_pm:   Y10P width (min, max, step)[560; 560; 0] height (min, max, step)[560; 560; 0]
	[00:00:00.807,000] <inf> cam_pm: Setting format: Y10P 560x560 pitch=1120 on ep=-4
	[00:00:01.536,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:00:01.541,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:00:01.550,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY state: 0xf,0
	[00:00:01.563,000] <inf> cam_pm: - format: Y10P 560x560
	[00:00:01.568,000] <inf> cam_pm: Width - 560, Pitch - 1120, Height - 560, Buff size - 627200
	[00:00:01.577,000] <inf> cam_pm: - addr - 0x2000068, size - 627200, bytesused - 0
	[00:00:01.614,000] <inf> cam_pm: capture buffer[0]: dump binary memory "/home/$USER/capture_0.bin" 0x02000068 0x02099267 -r
	[00:00:01.625,000] <inf> cam_pm: - addr - 0x2099270, size - 627200, bytesused - 0
	[00:00:01.661,000] <inf> cam_pm: capture buffer[1]: dump binary memory "/home/$USER/capture_1.bin" 0x02099270 0x0213246f -r
	[00:00:08.674,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:00:08.681,000] <inf> cam_pm: Camera thread is now suspended (polling for resume)
	[00:00:08.689,000] <inf> arx3a0: PM: Suspended arx3a0_selfie@36
	[00:00:08.696,000] <inf> cam_pm: Camera Capture is Suspended
	[00:00:26.703,000] <inf> cam_pm: Exited from RUNTIME_IDLE sleep
	[00:00:26.709,000] <inf> cam_pm: Camera: Try to Resume...
	[00:00:26.715,000] <inf> cam_pm: Camera: resume signal sent
	[00:00:26.721,000] <inf> cam_pm: Camera Capture is starting...
	[00:00:26.727,000] <inf> cam_pm: Camera: Woken up by PM resume
	[00:00:26.733,000] <inf> cam_pm: Resuming sensor device via PM
	[00:00:27.461,000] <inf> cam_pm: Re-applying video format...
	[00:00:28.187,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:00:28.192,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:00:28.201,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY state: 0xf,0
	[00:00:28.214,000] <inf> cam_pm: video_set_format returned: 0
	[00:00:28.220,000] <inf> cam_pm: Waiting for sensor to stabilize...
	[00:00:29.287,000] <inf> cam_pm: Capture started
	[00:00:29.354,000] <inf> cam_pm: Got frame 0! size: 627200; timestamp 29354 ms
	[00:00:29.554,000] <inf> cam_pm: Got frame 1! size: 627200; timestamp 29554 ms
	[00:00:29.754,000] <inf> cam_pm: Got frame 2! size: 627200; timestamp 29754 ms
	[00:00:29.761,000] <inf> cam_pm: FPS: 5.000000
	[00:00:29.954,000] <inf> cam_pm: Got frame 3! size: 627200; timestamp 29954 ms
	[00:00:29.961,000] <inf> cam_pm: FPS: 5.000000
	[00:00:30.154,000] <inf> cam_pm: Got frame 4! size: 627200; timestamp 30154 ms
	[00:00:30.161,000] <inf> cam_pm: FPS: 5.000000
	[00:00:30.354,000] <inf> cam_pm: Got frame 5! size: 627200; timestamp 30354 ms
	[00:00:30.361,000] <inf> cam_pm: FPS: 5.000000
	[00:00:30.554,000] <inf> cam_pm: Got frame 6! size: 627200; timestamp 30554 ms
	[00:00:30.561,000] <inf> cam_pm: FPS: 5.000000
	[00:00:30.754,000] <inf> cam_pm: Got frame 7! size: 627200; timestamp 30754 ms
	[00:00:30.761,000] <inf> cam_pm: FPS: 5.000000
	[00:00:30.954,000] <inf> cam_pm: Got frame 8! size: 627200; timestamp 30954 ms
	[00:00:30.961,000] <inf> cam_pm: FPS: 5.000000
	[00:00:31.154,000] <inf> cam_pm: Got frame 9! size: 627200; timestamp 31154 ms
	[00:00:31.161,000] <inf> cam_pm: FPS: 5.000000
	[00:00:31.166,000] <inf> cam_pm: Calling video flush.
	[00:00:31.172,000] <inf> cam_pm: Flush done.
	[00:00:31.176,000] <inf> cam_pm: Calling video stream stop.
	[00:00:31.182,000] <inf> cam_pm: Stream stop done.
	[00:00:31.289,000] <inf> cam_pm: Capture completed after 10 frames, waiting for PM cycle
	[00:00:31.297,000] <inf> cam_pm: Camera Capture cycle completed
	[00:00:31.303,000] <inf> cam_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (4000 microseconds)
	[00:00:31.312,000] <inf> cam_pm: Camera: putting thre[00:00:31.317,000] <inf> cam_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
	[00:00:31.323,000] <dbg> cam_pm: app_pm_lock_deeper_states: Unlocked deeper power state(s) (S2RAM)
	[00:00:31.333,000] <inf> cam_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (20000000 microseconds)
	ad into wait for PM resume
	[00:00:31.346,000] <inf> cam_pm: Camera thread is now suspended (polling for resume)
	[00:00:31.354,000] <inf> arx3a0: PM: Suspended arx3a0_selfie@36
	[00:00:31.361,000] <inf> cam_pm: Camera Capture is Suspended: for Deep Sleep
	[00:00:51.373,000] <inf> cam_pm: Camera: Try to Resume...
	[00:00:51.379,000] <inf> cam_pm: Camera: resume signal sent
	[00:00:51.385,000] <inf> cam_pm: Camera Capture is Resumed: for Deep Sleep
	[00:00:51.392,000] <inf> cam_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
	[00:00:51.402,000] <inf> cam_pm: Camera: Woken up by PM resume
	[00:00:51.408,000] <inf> cam_pm: Resuming sensor device via PM
	[00:00:52.136,000] <inf> cam_pm: Re-applying video format...
	[00:00:52.862,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:00:52.867,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:00:52.876,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY state: 0xf,0
	[00:00:52.889,000] <inf> cam_pm: video_set_format returned: 0
	[00:00:52.895,000] <inf> cam_pm: Waiting for sensor to stabilize...
	[00:00:53.962,000] <inf> cam_pm: Capture started
	[00:00:54.029,000] <inf> cam_pm: Got frame 0! size: 627200; timestamp 54029 ms
	[00:00:54.229,000] <inf> cam_pm: Got frame 1! size: 627200; timestamp 54229 ms
	[00:00:54.429,000] <inf> cam_pm: Got frame 2! size: 627200; timestamp 54429 ms
	[00:00:54.436,000] <inf> cam_pm: FPS: 5.000000
	[00:00:54.629,000] <inf> cam_pm: Got frame 3! size: 627200; timestamp 54629 ms
	[00:00:54.636,000] <inf> cam_pm: FPS: 5.000000
	[00:00:54.829,000] <inf> cam_pm: Got frame 4! size: 627200; timestamp 54829 ms
	[00:00:54.836,000] <inf> cam_pm: FPS: 5.000000
	[00:00:55.029,000] <inf> cam_pm: Got frame 5! size: 627200; timestamp 55029 ms
	[00:00:55.036,000] <inf> cam_pm: FPS: 5.000000
	[00:00:55.229,000] <inf> cam_pm: Got frame 6! size: 627200; timestamp 55229 ms
	[00:00:55.236,000] <inf> cam_pm: FPS: 5.000000
	[00:00:55.429,000] <inf> cam_pm: Got frame 7! size: 627200; timestamp 55429 ms
	[00:00:55.436,000] <inf> cam_pm: FPS: 5.000000
	[00:00:55.629,000] <inf> cam_pm: Got frame 8! size: 627200; timestamp 55629 ms
	[00:00:55.636,000] <inf> cam_pm: FPS: 5.000000
	[00:00:55.829,000] <inf> cam_pm: Got frame 9! size: 627200; timestamp 55829 ms
	[00:00:55.836,000] <inf> cam_pm: FPS: 5.000000
	[00:00:55.841,000] <inf> cam_pm: Calling video flush.
	[00:00:55.847,000] <inf> cam_pm: Flush done.
	[00:00:55.851,000] <inf> cam_pm: Calling video stream stop.
	[00:00:55.857,000] <inf> cam_pm: Stream stop done.
	[00:00:55.964,000] <inf> cam_pm: Capture completed after 10 frames, waiting for PM cycle
	[00:00:55.972,000] <inf> cam_pm: Camera Capture cycle completed (after STANDBY)
	[00:00:55.980,000] <inf> cam_pm: Main thread running - iteration 0 - tick: 55980
	[00:00:55.987,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:00:57.988,000] <inf> cam_pm: Main thread running - iteration 1 - tick: 57988
	[00:00:59.996,000] <inf> cam_pm: Main thread running - iteration 2 - tick: 59996
	[00:01:02.004,000] <inf> cam_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (22000000 microseconds)
	[00:01:02.014,000] <inf> cam_pm: Camera thread is now suspended (polling for resume)
	[00:01:02.022,000] <inf> arx3a0: PM: Suspended arx3a0_selfie@36
	[00:01:02.028,000] <inf> cam_pm: Camera Capture is Suspended: for Deep Sleep
	[00:01:24.039,000] <inf> cam_pm: Camera: Try to Resume...
	[00:01:24.044,000] <inf> cam_pm: Camera: resume signal sent
	[00:01:24.050,000] <inf> cam_pm: Camera Capture is Resumed: for Deep Sleep
	[00:01:24.058,000] <inf> cam_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
	[00:01:24.067,000] <inf> cam_pm: Camera: Woken up by PM resume
	[00:01:24.073,000] <inf> cam_pm: Resuming sensor device via PM
	[00:01:24.801,000] <inf> cam_pm: Re-applying video format...
	[00:01:25.527,000] <inf> dphy_dw: RX-DDR clock: 400000000
	[00:01:25.532,000] <dbg> dphy_dw: dphy_dw_slave_setup: hsfrequency - 9, osc_freq - 1e9
	[00:01:25.541,000] <dbg> dphy_dw: dphy_dw_slave_setup: PHY RX status: 0x00010000, STOPSTATE: 0x00010003, RX DPHY state: 0xf,0
	[00:01:25.554,000] <inf> cam_pm: video_set_format returned: 0
	[00:01:25.560,000] <inf> cam_pm: Waiting for sensor to stabilize...
	[00:01:26.627,000] <inf> cam_pm: Capture started
	[00:01:26.694,000] <inf> cam_pm: Got frame 0! size: 627200; timestamp 86694 ms
	[00:01:26.894,000] <inf> cam_pm: Got frame 1! size: 627200; timestamp 86894 ms
	[00:01:27.094,000] <inf> cam_pm: Got frame 2! size: 627200; timestamp 87094 ms
	[00:01:27.101,000] <inf> cam_pm: FPS: 5.000000
	[00:01:27.294,000] <inf> cam_pm: Got frame 3! size: 627200; timestamp 87294 ms
	[00:01:27.301,000] <inf> cam_pm: FPS: 5.000000
	[00:01:27.494,000] <inf> cam_pm: Got frame 4! size: 627200; timestamp 87494 ms
	[00:01:27.501,000] <inf> cam_pm: FPS: 5.000000
	[00:01:27.694,000] <inf> cam_pm: Got frame 5! size: 627200; timestamp 87694 ms
	[00:01:27.701,000] <inf> cam_pm: FPS: 5.000000
	[00:01:27.894,000] <inf> cam_pm: Got frame 6! size: 627200; timestamp 87894 ms
	[00:01:27.901,000] <inf> cam_pm: FPS: 5.000000
	[00:01:28.094,000] <inf> cam_pm: Got frame 7! size: 627200; timestamp 88094 ms
	[00:01:28.101,000] <inf> cam_pm: FPS: 5.000000
	[00:01:28.294,000] <inf> cam_pm: Got frame 8! size: 627200; timestamp 88294 ms
	[00:01:28.301,000] <inf> cam_pm: FPS: 5.000000
	[00:01:28.494,000] <inf> cam_pm: Got frame 9! size: 627200; timestamp 88494 ms
	[00:01:28.501,000] <inf> cam_pm: FPS: 5.000000
	[00:01:28.506,000] <inf> cam_pm: Calling video flush.
	[00:01:28.512,000] <inf> cam_pm: Flush done.
	[00:01:28.516,000] <inf> cam_pm: Calling video stream stop.
	[00:01:28.522,000] <inf> cam_pm: Stream stop done.
	[00:01:28.629,000] <inf> cam_pm: Capture completed after 10 frames, waiting for PM cycle
	[00:01:28.637,000] <inf> cam_pm: Camera Capture cycle completed (after STOP)
	[00:01:28.644,000] <inf> cam_pm: Main thread running - iteration 0 - tick: 88644
	[00:01:28.652,000] <inf> cam_pm: Camera: putting thread into wait for PM resume
	[00:01:30.653,000] <inf> cam_pm: Main thread running - iteration 1 - tick: 90653
	[00:01:32.661,000] <inf> cam_pm: Main thread running - iteration 2 - tick: 92661
	[00:01:34.669,000] <inf> cam_pm: Skipping PM_STATE_SOFT_OFF (TCM boot, using retention instead)
	[00:01:34.677,000] <inf> cam_pm: === POWER STATE SEQUENCE COMPLETED ===
	[00:01:34.685,000] <inf> cam_pm: Camera: requesting thread stop
	[00:01:39.700,000] <inf> cam_pm: Camera Capture is Stopped (PM sequence done)
	[00:01:39.707,000] <dbg> cam_pm: app_pm_lock_deeper_states: Locked deeper power state(s) (S2RAM)

Notes
*****

* **Debugger**: Disconnect debugger before testing - it prevents cores from entering OFF states
* **UART Hub**: If using USB hub for UART, set BOOT_DELAY to avoid missing logs after power cycle
* **Sleep Durations**:

  * RUNTIME_IDLE: 18 seconds
  * SUSPEND_TO_IDLE: 4 milliseconds
  * S2RAM STANDBY: 20 seconds
  * S2RAM STOP: 22 seconds
  * SOFT_OFF: 26 seconds

* **SUSPEND_TO_IDLE Details**:

  * Requires LPM timer support (``CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER``)
  * HE core: Enabled by default with RTC as LPM timer
  * HP core: Skipped by default (RTC shared with HE, can be enabled if needed)
  * Uses IWIC (Internal WIC) for wake sources - lighter than EWIC
  * Only interrupts 0-63 can wake from IWIC mode
  * Devices remain active - no suspend/resume overhead (zephyr,pm-device-disabled)
  * Exit latency (~500µs) primarily from RTC register read/write for time compensation
  * Suitable for frequent, low-latency wake scenarios
  * Enable in device tree with ``suspend_idle`` node status = "okay"

* **CONFIG_POWEROFF**: Alternative mode to test sys_poweroff() instead of PM state sequence
* **Retention Memory**: HE core retains SERAM and optionally TCM (when booting from TCM)
* **Power Measurement**: For accurate power measurements, ensure all unused peripherals are disabled
