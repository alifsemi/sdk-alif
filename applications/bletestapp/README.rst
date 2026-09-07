.. _ble-testapp:

BLE test application
####################

Overview
********

Shell-driven Balletto application used by CI and automation to start the
Alif BLE host, advertise a small GATT "Hello" service, and exercise
power-management profiles.

The image is **B1 only**. ``main.c`` fails the build on other SoC series
(``#error "Application works only with B1 devices"``).

Architecture
************

* ``src/main.c`` — cold start, PM notifier, BLE init/start, idle loop
* ``src/ble_handler.c`` — Alif BLE host (``CONFIG_BT_CUSTOM``), GAP
  peripheral, GATT Hello service
* ``src/appl_shell.c`` — ``ble_appl`` commands (intervals, name, sleep,
  off profile, wakeup timer)
* ``src/power_mgr.c`` / ``src/power_shell.c`` — SE run/off profiles and
  ``senc`` commands
* ``app.overlay`` — RTC0 idle timer, LPTIMER0, HCI UART, LPGPIO wakeup on
  P15.1, S2RAM STOP residency

BLE behavior
============

* Role: LE peripheral, pairing disabled
* Default device name: ``APPL_SHL`` (max 8 characters;
  ``DEVICE_NAME_LEN`` is 9 including the terminator)
* Default advertising intervals: 1000 / 1000 (units used by the Alif GAP
  API)
* Default connection intervals: 800 / 800
* GATT Hello service: one notify/read characteristic and one write
  characteristic
* After boot the app prints ``BLE testapp started!`` and blocks in
  ``appl_wait_to_continue()`` until ``ble_appl continue``

Power management
================

Sleep is locked at ``PRE_KERNEL_1`` (``app_prevent_off()``) so the core
does not enter ``SOFT_OFF`` or S2RAM until the operator allows it.

* ``ble_appl sleep <seconds>`` unlocks those states for the given period
* Default wakeup source is LPRTC; switch with
  ``ble_appl select_timer LPRTC|LPTIMER``
* Default idle wakeup is 20000 ms; connected wakeup is separate
  (``ble_appl interval``)
* On S2RAM resume, ``pm_notify_pre_device_resume()`` restores the SE run
  profile. ``SOFT_OFF`` resets the chip, so that callback is a no-op
* Disconnect the debugger when testing OFF / S2RAM; a connected probe can
  keep the core awake

Requirements
************

* Alif Balletto B1 DevKit or B1 EB
* SE Services for run/off profile programming
* UART console (UART2 in ``app.overlay``; sleep pinctrl is deleted to
  avoid extra characters around low-power transitions)

Building and flashing
*********************

From the Zephyr workspace directory:

.. code-block:: console

   west build -p auto \
     -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he \
     ../alif/applications/bletestapp

   west flash

Use the matching B1 board name for HH packages or the EB
(``alif_b1_eb/.../rtss_he``).

Typical shell sequence
**********************

.. code-block:: console

   ble_appl set-name APPL_SHL
   ble_appl adv-interval --min 1000 --max 1000
   ble_appl continue
   bt advertise on

Optional power sequence:

.. code-block:: console

   ble_appl select_timer LPRTC
   ble_appl set_offprofile STANDBY
   ble_appl sleep 30

``set-name`` must run before ``continue``. After ``continue``, the name
command returns ``-ENOEXEC``.

Shell commands
**************

``ble_appl``
============

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Command
     - Purpose
   * - ``continue [--reset_after 0/1]``
     - Release the boot barrier and start BLE
   * - ``set-name <name>``
     - Set the advertised name (cold boot only)
   * - ``adv-interval --min --max``
     - Advertising interval
   * - ``conn-interval --min --max``
     - Connection interval
   * - ``interval <ms> [--connected <ms>]``
     - M55 wakeup period while idle or connected
   * - ``sleep <seconds>``
     - Allow SOFT_OFF / S2RAM for the given time
   * - ``set_offprofile STOP|IDLE|STANDBY``
     - Program the SE off profile
   * - ``select_timer LPRTC|LPTIMER``
     - Wakeup counter

``bt``
======

``bt init``, ``bt advertise <off|on|nconn>``, ``bt disable``,
``bt name <name>``.

``senc``
========

Secure Enclave run/off profile get/set helpers and clock divider control
(``set_default_run_cfg``, ``set_run_cfg``, ``set_off_cfg``, and related
commands).

Constraints
***********

* Not supported on Ensemble (E1C/E7/E8) or APSS
* ``CONFIG_SHELL_AUTOSTART=n``; the app starts the UART shell itself
* ``CONFIG_BOOT_BANNER=n``
* Notification payload size is ``CONFIG_DATA_STRING_LENGTH`` (default 243,
  range 5–250)
* CI builds this app from ``automation/Jenkinsfile_Pr`` (``applications``
  list together with ``testapp``)
