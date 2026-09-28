.. _bmi323_pm:

BMI323 Power Management
#######################

Description
***********

This sample walks Alif power-management states and, after each wake,
polls the on-board BMI323 over I3C. The SoC is woken by the RTC (EWIC).
The sensor GPIO INT pin is not used; it is not LPGPIO and cannot wake
STOP/S2RAM.

Sequence (HE TCM / E8 SRAM0 boot)::

  BMI323 poll
  RUNTIME_IDLE  (~18 s)  -> poll
  SUSPEND_TO_IDLE (~10 ms) -> poll
  S2RAM STANDBY (~6 s)   -> poll
  S2RAM STOP    (~9 s)   -> poll

MRAM boot / HP without SRAM0 use SOFT_OFF instead of S2RAM (system
resets on wake).

Building and Running
********************

Requires a BMI323 on I3C0 (Alif DevKit / AppKit). Build with the board
sensor overlay and a ``pm-system-off-*`` snippet from
``samples/drivers/pm/system_off`` (RTC + SE off profile):

.. zephyr-app-commands::
   :zephyr-app: samples/sensor/bmi323_pm
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :gen-args: -S alif-dk-ak -S pm-system-off-s2ram-tcm

HE TCM boot uses ``pm-system-off-s2ram-tcm``. HE or HP MRAM boot
(SOFT_OFF) uses ``-S pm-system-off-mram``. E8 SRAM0 S2RAM uses
``-S pm-system-off-s2ram-sram0``.

Do not leave a debugger attached if you want STOP/SOFT_OFF; it holds the
core out of those states.

Sample Output
=============

HE TCM boot (S2RAM STANDBY and STOP; SOFT_OFF skipped)
------------------------------------------------------

``alif_e8_dk`` RTSS_HE, TCM boot. After S2RAM the I3C controller DAT is
restored and board targets keep their DTS dynamic address.

.. code-block:: console

    *** Booting Zephyr OS build c7432c33430e ***
    Device 0xf4b0 name is bmi323@69000003b810431000
    [00:00:00.032,000] <inf> bmi323_pm: alif_e8_dk (S2RAM): BMI323 PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM STANDBY, S2RAM STOP)
    [00:00:00.045,000] <inf> bmi323_pm: --- BMI323 poll before sleep ---
    Accel AX: 0.021350; AY: -0.010675; AZ: 0.978623 g        Gyro GX: 0.106813; GY: 0.457770; GZ: -0.289921 deg/s
    [00:00:00.099,000] <inf> bmi323_pm: POWER STATE SEQUENCE:
    [00:00:00.105,000] <inf> bmi323_pm:   1. PM_STATE_RUNTIME_IDLE
    [00:00:00.111,000] <inf> bmi323_pm:   2. PM_STATE_SUSPEND_TO_IDLE
    [00:00:00.117,000] <inf> bmi323_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
    [00:00:00.126,000] <inf> bmi323_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
    [00:00:00.134,000] <inf> bmi323_pm:   5. (SOFT_OFF skipped - using retention)
    [00:00:00.141,000] <inf> bmi323_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
    [00:00:18.151,000] <inf> bmi323_pm: Exited from RUNTIME_IDLE sleep
    [00:00:18.157,000] <inf> bmi323_pm: --- BMI323 poll after RUNTIME_IDLE ---
    Accel AX: 0.022448; AY: -0.012383; AZ: 0.979111 g        Gyro GX: 0.076295; GY: 0.457770; GZ: -0.244144 deg/s
    [00:00:18.184,000] <inf> bmi323_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (10000 microseconds)
    [00:00:18.206,000] <inf> bmi323_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
    [00:00:18.213,000] <inf> bmi323_pm: --- BMI323 poll after SUSPEND_TO_IDLE ---
    Accel AX: 0.023607; AY: -0.010614; AZ: 0.978379 g        Gyro GX: 0.045777; GY: 0.442511; GZ: -0.335698 deg/s
    [00:00:18.240,000] <inf> bmi323_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (6000000 microseconds)
    [00:00:24.253,000] <inf> bmi323_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
    [00:00:24.292,000] <inf> bmi323_pm: Main thread running - iteration 0 - tick: 24292
    [00:00:26.303,000] <inf> bmi323_pm: Main thread running - iteration 1 - tick: 26303
    [00:00:28.313,000] <inf> bmi323_pm: Main thread running - iteration 2 - tick: 28313
    [00:00:30.324,000] <inf> bmi323_pm: --- BMI323 poll after S2RAM (STANDBY) ---
    Accel AX: 0.023363; AY: -0.011651; AZ: 0.978806 g        Gyro GX: 0.000000; GY: 0.473029; GZ: -0.244144 deg/s
    [00:00:30.358,000] <inf> bmi323_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (9000000 microseconds)
    [00:00:39.371,000] <inf> bmi323_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
    [00:00:39.410,000] <inf> bmi323_pm: Main thread running - iteration 0 - tick: 39410
    [00:00:41.421,000] <inf> bmi323_pm: Main thread running - iteration 1 - tick: 41421
    [00:00:43.431,000] <inf> bmi323_pm: Main thread running - iteration 2 - tick: 43431
    [00:00:45.442,000] <inf> bmi323_pm: --- BMI323 poll after S2RAM (STOP) ---
    Accel AX: 0.021228; AY: -0.011590; AZ: 0.980148 g        Gyro GX: 0.061036; GY: 0.411993; GZ: -0.320439 deg/s
    [00:00:45.476,000] <inf> bmi323_pm: Skipping PM_STATE_SOFT_OFF (using retention instead)
    [00:00:45.484,000] <inf> bmi323_pm: === BMI323 PM TEST COMPLETED ===

HE MRAM boot (SOFT_OFF; S2RAM skipped)
--------------------------------------

``alif_e8_dk`` RTSS_HE, MRAM boot. SOFT_OFF has no retention: the core
resets on RTC wakeup, so the console stops at the enter-SOFT_OFF lines
and the next output is a fresh boot banner.

.. code-block:: console

    *** Booting Zephyr OS build 97fddffd316f ***
    Device 0x8000efc8 name is bmi323@69000003b810431000
    [00:00:00.025,000] <inf> bmi323_pm: alif_e8_dk (SOFT_OFF): BMI323 PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)
    [00:00:00.037,000] <inf> bmi323_pm: --- BMI323 poll before sleep ---
    Accel AX: 0.034648; AY: -0.006039; AZ: 0.978806 g        Gyro GX: 0.106813; GY: 0.427252; GZ: -0.274662 deg/s
    [00:00:00.067,000] <inf> bmi323_pm: POWER STATE SEQUENCE:
    [00:00:00.072,000] <inf> bmi323_pm:   1. PM_STATE_RUNTIME_IDLE
    [00:00:00.079,000] <inf> bmi323_pm:   2. PM_STATE_SUSPEND_TO_IDLE
    [00:00:00.085,000] <inf> bmi323_pm:   3. (S2RAM skipped - no retention)
    [00:00:00.092,000] <inf> bmi323_pm:   4. PM_STATE_SOFT_OFF
    [00:00:00.098,000] <inf> bmi323_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
    [00:00:18.107,000] <inf> bmi323_pm: Exited from RUNTIME_IDLE sleep
    [00:00:18.113,000] <inf> bmi323_pm: --- BMI323 poll after RUNTIME_IDLE ---
    Accel AX: 0.032574; AY: -0.006893; AZ: 0.977952 g        Gyro GX: 0.091554; GY: 0.457770; GZ: -0.213626 deg/s
    [00:00:18.137,000] <inf> bmi323_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (10000 microseconds)
    [00:00:18.152,000] <inf> bmi323_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
    [00:00:18.159,000] <inf> bmi323_pm: --- BMI323 poll after SUSPEND_TO_IDLE ---
    Accel AX: 0.033428; AY: -0.005063; AZ: 0.977891 g        Gyro GX: 0.076295; GY: 0.381475; GZ: -0.289921 deg/s
    [00:00:18.183,000] <inf> bmi323_pm: Skipping PM_STATE_SUSPEND_TO_RAM (no retention)
    [00:00:18.191,000] <inf> bmi323_pm: Enter PM_STATE_SOFT_OFF for (10000000 microseconds)
    [00:00:18.199,000] <inf> bmi323_pm: Note: SOFT_OFF has no retention - system will reset on wakeup
