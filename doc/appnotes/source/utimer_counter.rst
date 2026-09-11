.. _appnote-zas-utimer-counter:

==============
UTimer Counter
==============

Introduction
=============

The Alif UTimer IP on the Alif DevKit supports counter mode, enabling precise counting of events or clock pulses for applications such as frequency measurement, event counting, or timer-based scheduling. This application note provides a guide to configuring, building, and testing the Zephyr counter sample application (``samples/drivers/counter/alarm/``) using the UTimer as a counter.

Furthermore, the UTIMER is integrated into the Alarm application as a demo application, where it functions as expected. The same demo app is also utilized by the RTC (Real-Time Clock) and LPTIMER. To facilitate configuration, separate overlay and config files for the RTC, UTIMER, and LPTIMER reside in the board’s directory of the Alarm application. Users can select these files using the west build command.

.. include:: prerequisites.rst

.. include:: note.rst

Build a UTimer Counter Application with Zephyr
===============================================

Follow these steps to build the UTimer Counter application using the Alif Zephyr SDK:


1. For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E7 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section ``Setting Up and Building Zephyr Applications``.

2. Build command for application on the M55 HE core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay


3. Build command for application on the M55 HP core:

.. code-block:: console

   west build -p always \
     -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
     samples/drivers/counter/alarm \
     -- -DDTC_OVERLAY_FILE=$PWD/samples/drivers/counter/alarm/boards/alif_utimer.overlay

Once the build command completes successfully, executable images will be generated and placed in the ``build/zephyr`` directory. Both ``.bin`` (binary) and ``.elf`` (Executable and Linkable Format) files will be available.

Executing Binary on the DevKit
================================

To execute binaries on the DevKit, follow the command:

.. code-block:: console

   west flash

Expected Result
===============

When running the emulator-based counter application:

- The application configures the UTimer as a counter in the emulated environment.
- Simulated input pulses increment the counter, and the alarm callback triggers when the counter reaches the configured threshold.
- Output is printed to the host terminal, showing counter values or alarm events.

Console Output
===============

.. code-block:: text

   Counter alarm sample

   Set alarm in 2 sec (800000000 ticks)
   !!! Alarm !!!
   Now: 2

   Set alarm in 4 sec (1600000000 ticks)
   !!! Alarm !!!
   Now: 6

   Set alarm in 8 sec (3200000000 ticks)
   !!! Alarm !!!
   Now: 3

   Set alarm in 5 sec (2105032704 ticks)
   !!! Alarm !!!
   Now: 8

   Set alarm in 10 sec (4210065408 ticks)
   !!! Alarm !!!
   Now: 8

   Set alarm in 10 sec (4125163520 ticks)
   !!! Alarm !!!
   Now: 7

   Set alarm in 9 sec (3955359744 ticks)
   !!! Alarm !!!
   Now: 7

   Set alarm in 9 sec (3615752192 ticks)
   !!! Alarm !!!
   Now: 5

   Set alarm in 7 sec (2936537088 ticks)
   !!! Alarm !!!
   Now: 1

   Set alarm in 3 sec (1578106880 ticks)
   !!! Alarm !!!
   Now: 5

   Set alarm in 7 sec (3156213760 ticks)
   !!! Alarm !!!
   Now: 3

   Set alarm in 5 sec (2017460224 ticks)
   !!! Alarm !!!
   Now: 8

   Set alarm in 10 sec (4043920448 ticks)
   !!! Alarm !!!
   Now: 7

   Set alarm in 9 sec (3774873600 ticks)
   !!! Alarm !!!
   Now: 6

   Set alarm in 8 sec (3234779904 ticks)
   !!! Alarm !!!
   Now: 3

   Set alarm in 5 sec (2214592512 ticks)
   !!! Alarm !!!
   Now: 9

PM Support
==========

The Alif UTIMER counter sample supports Zephyr Power Management (PM) states
on Alif Ensemble and Balletto RTSS cores. The application verifies that the
``counter_alif_utimer`` driver continues to program a compare alarm and
advance the counter after each supported PM transition.

The following PM states are supported:

* **PM_STATE_RUNTIME_IDLE**: Light sleep. The CPU clock is gated using WFI
  for quick wakeup.
* **PM_STATE_SUSPEND_TO_IDLE**: CPU sleep with IWIC. Devices remain active
  without device-PM overhead. An LPM timer is required.
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention on the HE
  core when booting from TCM.

  * Substate 0: STANDBY
  * Substate 1: STOP

* **PM_STATE_SOFT_OFF**: Deepest sleep without retention. The system performs
  a full reset on wakeup.

A 2-second UTIMER0 channel 0 alarm is used as the device under test (DUT).
The alarm runs before entering ``RUNTIME_IDLE`` and again after each wakeup
so that the counter value can be verified. RTC on the HE core or LPTIMER0
on the HP core is used as the wake source, not the UTIMER under test.

**HE core**

* TCM boot (VTOR = ``0x0``): Supports ``RUNTIME_IDLE``,
  ``SUSPEND_TO_IDLE``, S2RAM STANDBY, and S2RAM STOP. Execution resumes
  after each state. ``SOFT_OFF`` is skipped.
* MRAM boot (VTOR >= ``0x80000000``): Supports ``RUNTIME_IDLE``,
  ``SUSPEND_TO_IDLE``, and ``SOFT_OFF``. The system resets and restarts
  ``main()`` after ``SOFT_OFF``.

**HP core**

* Supports ``RUNTIME_IDLE`` followed by ``SOFT_OFF``. S2RAM is not supported.

The ``counter-pm-mram`` and ``counter-pm-s2ram-tcm`` snippets enable the
UTIMER0 counter. No jumper is required.

Building and Running PM Sample
------------------------------

Follow these steps to build the PM Sample application using the Alif Zephyr SDK:

For instructions on fetching the Alif Zephyr SDK and navigating to the Zephyr repository, refer to the `ZAS User Guide`_.

.. note::
   The build commands shown here are specifically for the Alif E8 DevKit.
   To build the application for other boards, modify the board name in the build command accordingly. For more information, refer to the `ZAS User Guide`_, under the section ``Setting Up and Building Zephyr Applications``.

HE Core - TCM Boot S2RAM (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the counter PM sample for the HE core using the following command:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-s2ram-tcm \
     -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
     -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
     -DCONFIG_FLASH_SIZE=256

HE Core - MRAM Boot SOFT_OFF (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the counter PM sample for the HE core using the following command:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

HP Core - MRAM Boot SOFT_OFF (E8)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Build the counter PM sample for the HP core using the following command:

.. code-block:: console

   west build -p always \
     -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp \
     ../alif/samples/drivers/pm/counter_alif \
     -S counter-pm-mram

Replace the board with ``alif_e7_dk/ae722f80f55d5xx/rtss_he`` or
``rtss_hp`` as applicable. The same PM sample can also be used with the
supported ``alif_b1_dk`` and ``alif_e1c_dk`` boards. The TCM snippet is
supported only on the HE core.

PM Test Sequence
----------------

The S2RAM PM test performs the following sequence:

1. Run the UTIMER counter alarm before entering the PM states.
2. Enter ``PM_STATE_RUNTIME_IDLE`` and wake up.
3. Enter ``PM_STATE_SUSPEND_TO_IDLE`` and wake up.
4. Run the UTIMER counter alarm after ``SUSPEND_TO_IDLE``.
5. Enter ``PM_STATE_SUSPEND_TO_RAM`` with substate 0 (STANDBY) and wake up.
6. Run the UTIMER counter alarm after S2RAM STANDBY.
7. Enter ``PM_STATE_SUSPEND_TO_RAM`` with substate 1 (STOP) and wake up.
8. Run the UTIMER counter alarm after S2RAM STOP.
9. Complete the PM sequence.

The SOFT_OFF configuration follows the supported PM sequence before
entering ``PM_STATE_SOFT_OFF``. The system resets when it wakes from
``SOFT_OFF`` and restarts ``main()``.

PM Support Verification
-----------------------

After each supported PM state, the application runs the UTIMER counter alarm
and checks that the counter continues to operate correctly.

A successful S2RAM test displays messages similar to the following:

.. code-block:: console

   === before RUNTIME_IDLE: counter alarm ===
   counter value: 800004260
   Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   Exited from RUNTIME_IDLE sleep
   Enter PM_STATE_SUSPEND_TO_IDLE for (10000 microseconds)
   PM enter: SUSPEND_TO_IDLE (substate 0)
   PM exit:  SUSPEND_TO_IDLE (substate 0)
   === after SUSPEND_TO_IDLE: counter alarm ===
   counter value: 1600008520
   Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   PM enter: SUSPEND_TO_RAM (substate 0)
   PM wakeup: SUSPEND_TO_RAM (substate 0)
   PM exit:  SUSPEND_TO_RAM (substate 0)
   === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===
   === after S2RAM STANDBY: counter alarm ===
   counter value: 2400012780
   Enter PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   PM enter: SUSPEND_TO_RAM (substate 1)
   PM wakeup: SUSPEND_TO_RAM (substate 1)
   PM exit:  SUSPEND_TO_RAM (substate 1)
   === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 1: STOP) ===
   === after S2RAM STOP: counter alarm ===
   counter value: 800004260
   === COUNTER PM SEQUENCE COMPLETED ===

Notes
-----

* **Debugger**: Disconnect the debugger before testing. It can block
  ``SOFT_OFF`` and S2RAM entry.
* **UART hub**: Set ``BOOT_DELAY`` if the UART hub drops the first lines
  after reset.
* **Sleep durations**:

  * ``RUNTIME_IDLE``: 18 seconds
  * ``SUSPEND_TO_IDLE``: 10 milliseconds
  * S2RAM STANDBY: 6 seconds
  * S2RAM STOP: 9 seconds
  * ``SOFT_OFF``: 10 seconds

* **SUSPEND_TO_IDLE**: Requires
  ``CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER``. The HE core enables it
  with RTC. The HP core skips this state unless an LPM timer is configured.
* **Retention**: The HE core retains SERAM and TCM when booting from TCM.
* **Power measurement**: Disable unused peripherals to obtain a clean
  power trace.
