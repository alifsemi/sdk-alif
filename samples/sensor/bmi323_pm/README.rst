.. _bmi323_pm:

BMI323 Power Management
#######################

Description
***********

This sample walks Alif power-management states and, after each wake,
polls the on-board BMI323 over I3C. The SoC is woken by the RTC (EWIC).
The sensor GPIO INT pin is not used; it is not LPGPIO and cannot wake
STOP/S2RAM.

Sequence (HE TCM boot)::

  BMI323 poll
  RUNTIME_IDLE  (~18 s)  -> poll
  SUSPEND_TO_IDLE        -> poll
  S2RAM STANDBY (~20 s)  -> poll
  S2RAM STOP    (~22 s)  -> poll

MRAM boot / HP core use SOFT_OFF instead of S2RAM (system resets on wake).

Building and Running
********************

Requires a BMI323 on I3C0 (Alif DevKit / AppKit). Build with both the
board sensor overlay and the HE PM snippet (RTC + SE off profile):

.. zephyr-app-commands::
   :zephyr-app: samples/sensor/bmi323_pm
   :board: alif_e7_dk/ae722f80f55d5xx/rtss_he
   :goals: build
   :gen-args: -S alif-dk-ak -S pm-system-off-he

For HP core use ``-S pm-system-off-hp`` instead of ``pm-system-off-he``.

Do not leave a debugger attached if you want STOP/SOFT_OFF; it holds the
core out of those states.

Sample Output
=============

HE TCM boot (S2RAM STANDBY and STOP; SOFT_OFF skipped)
------------------------------------------------------

``alif_e8_dk`` RTSS_HE, TCM boot. After S2RAM the I3C controller DAT is
restored and board targets keep their DTS dynamic address.

.. code-block:: console

    *** Booting Zephyr OS build 97fddffd316f ***
    Device 0xef30 name is bmi323@69000003b810431000
    [00:00:00.025,000] <inf> bmi323_pm: alif_e8_dk RTSS_HE (TCM boot): BMI323 PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM)
    [00:00:00.037,000] <inf> bmi323_pm: --- BMI323 poll before sleep ---
    Accel AX: 0.025803; AY: -0.001220; AZ: 0.980331 g        Gyro GX: 0.030518; GY: 0.488288; GZ: -0.305180 deg/s
    [00:00:00.066,000] <inf> bmi323_pm: POWER STATE SEQUENCE:
    [00:00:00.072,000] <inf> bmi323_pm:   1. PM_STATE_RUNTIME_IDLE
    [00:00:00.078,000] <inf> bmi323_pm:   2. PM_STATE_SUSPEND_TO_IDLE
    [00:00:00.085,000] <inf> bmi323_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
    [00:00:00.093,000] <inf> bmi323_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
    [00:00:00.101,000] <inf> bmi323_pm:   5. (SOFT_OFF skipped - TCM boot, using retention)
    [00:00:00.110,000] <inf> bmi323_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
    [00:00:18.119,000] <inf> bmi323_pm: Exited from RUNTIME_IDLE sleep
    [00:00:18.125,000] <inf> bmi323_pm: --- BMI323 poll after RUNTIME_IDLE ---
    Accel AX: 0.025376; AY: -0.001708; AZ: 0.980392 g        Gyro GX: 0.030518; GY: 0.503547; GZ: -0.274662 deg/s
    [00:00:18.149,000] <inf> bmi323_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (4000 microseconds)
    [00:00:18.164,000] <inf> bmi323_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
    [00:00:18.171,000] <inf> bmi323_pm: --- BMI323 poll after SUSPEND_TO_IDLE ---
    Accel AX: 0.023790; AY: -0.000549; AZ: 0.980514 g        Gyro GX: 0.045777; GY: 0.442511; GZ: -0.289921 deg/s
    [00:00:18.195,000] <inf> bmi323_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (20000000 microseconds)
    [00:00:38.208,000] <inf> bmi323_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
    [00:00:38.239,000] <inf> bmi323_pm: Main thread running - iteration 0 - tick: 38239
    [00:00:40.249,000] <inf> bmi323_pm: Main thread running - iteration 1 - tick: 40249
    [00:00:42.259,000] <inf> bmi323_pm: Main thread running - iteration 2 - tick: 42259
    [00:00:44.269,000] <inf> bmi323_pm: --- BMI323 poll after S2RAM (STANDBY) ---
    Accel AX: 0.026718; AY: -0.001647; AZ: 0.980880 g        Gyro GX: 0.106813; GY: 0.457770; GZ: -0.274662 deg/s
    [00:00:44.298,000] <inf> bmi323_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) for (22000000 microseconds)
    [00:01:06.311,000] <inf> bmi323_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
    [00:01:06.342,000] <inf> bmi323_pm: Main thread running - iteration 0 - tick: 66342
    [00:01:08.352,000] <inf> bmi323_pm: Main thread running - iteration 1 - tick: 68352
    [00:01:10.362,000] <inf> bmi323_pm: Main thread running - iteration 2 - tick: 70362
    [00:01:12.371,000] <inf> bmi323_pm: --- BMI323 poll after S2RAM (STOP) ---
    Accel AX: 0.025010; AY: 0.000488; AZ: 0.980270 g         Gyro GX: 0.045777; GY: 0.442511; GZ: -0.274662 deg/s
    [00:01:12.400,000] <inf> bmi323_pm: Skipping PM_STATE_SOFT_OFF (TCM boot, using retention instead)
    [00:01:12.410,000] <inf> bmi323_pm: === BMI323 PM TEST COMPLETED ===

HE MRAM boot (SOFT_OFF; S2RAM skipped)
--------------------------------------

``alif_e8_dk`` RTSS_HE, MRAM boot. SOFT_OFF has no retention: the core
resets on RTC wakeup, so the console stops at the enter-SOFT_OFF lines
and the next output is a fresh boot banner.

.. code-block:: console

    *** Booting Zephyr OS build 97fddffd316f ***
    Device 0x8000efc8 name is bmi323@69000003b810431000
    [00:00:00.025,000] <inf> bmi323_pm: alif_e8_dk RTSS_HE (MRAM boot): BMI323 PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, SOFT_OFF)
    [00:00:00.037,000] <inf> bmi323_pm: --- BMI323 poll before sleep ---
    Accel AX: 0.034648; AY: -0.006039; AZ: 0.978806 g        Gyro GX: 0.106813; GY: 0.427252; GZ: -0.274662 deg/s
    [00:00:00.067,000] <inf> bmi323_pm: POWER STATE SEQUENCE:
    [00:00:00.072,000] <inf> bmi323_pm:   1. PM_STATE_RUNTIME_IDLE
    [00:00:00.079,000] <inf> bmi323_pm:   2. PM_STATE_SUSPEND_TO_IDLE
    [00:00:00.085,000] <inf> bmi323_pm:   3. (S2RAM skipped - MRAM boot)
    [00:00:00.092,000] <inf> bmi323_pm:   4. PM_STATE_SOFT_OFF
    [00:00:00.098,000] <inf> bmi323_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
    [00:00:18.107,000] <inf> bmi323_pm: Exited from RUNTIME_IDLE sleep
    [00:00:18.113,000] <inf> bmi323_pm: --- BMI323 poll after RUNTIME_IDLE ---
    Accel AX: 0.032574; AY: -0.006893; AZ: 0.977952 g        Gyro GX: 0.091554; GY: 0.457770; GZ: -0.213626 deg/s
    [00:00:18.137,000] <inf> bmi323_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (4000 microseconds)
    [00:00:18.152,000] <inf> bmi323_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
    [00:00:18.159,000] <inf> bmi323_pm: --- BMI323 poll after SUSPEND_TO_IDLE ---
    Accel AX: 0.033428; AY: -0.005063; AZ: 0.977891 g        Gyro GX: 0.076295; GY: 0.381475; GZ: -0.289921 deg/s
    [00:00:18.183,000] <inf> bmi323_pm: Skipping PM_STATE_SUSPEND_TO_RAM (MRAM boot)
    [00:00:18.191,000] <inf> bmi323_pm: Enter PM_STATE_SOFT_OFF for (26000000 microseconds)
    [00:00:18.199,000] <inf> bmi323_pm: Note: SOFT_OFF has no retention - system will reset on wakeup
