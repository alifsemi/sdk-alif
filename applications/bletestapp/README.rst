BLE testapp for test automation
###############################

Overview
********

BLE peripheral test application for Alif Balletto boards. The application
boots into a shell and waits; the test setup configures BLE and power
parameters over the shell and then starts the application with
``ble_appl continue``.

Building
********

Ceva BLE host stack (Alif ROM, default):

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he alif/applications/bletestapp

Zephyr BLE host stack:

.. code-block:: console

   west build -p always -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he alif/applications/bletestapp \
      -- -DCONFIG_BT_ZEPHYR=y

Use ``-p always`` when switching between the stacks, otherwise the previous
selection stays in the CMake cache.

Shell commands
**************

BLE application (``ble_appl``)
==============================

Parameters keep their defaults unless set before ``continue``.

``ble_appl set-name <name>``
   BLE device name (default ``APPL_SHL``). Only allowed before ``continue``.

``ble_appl adv-interval --min <slots> --max <slots>``
   Advertising interval in 0.625 ms units (default 1000).

``ble_appl conn-interval --min <units> --max <units>``
   Connection interval in 1.25 ms units (default 800).

``ble_appl interval <ms> --connected <ms>``
   Application wakeup interval when advertising / when connected
   (default 20000 ms).

``ble_appl select_timer <LPRTC|LPTIMER>``
   Low-power counter used as the wakeup source.

``ble_appl set_offprofile <STOP|IDLE|STANDBY>``
   Power state used while sleeping.

   - ``STOP``: everything is in STOP. Only BLE stays active, on its own timer.
   - ``STANDBY``: Corstone stays powered while the subsystem is off.
   - ``IDLE``: Corstone and the Secure Enclave stay powered while the subsystem is off.

``ble_appl continue [--reset_after <0|1>]``
   Start the BLE application. With ``--reset_after 1`` the BLE stack is
   restarted after each sleep period.

``ble_appl sleep <seconds>``
   Allow the system to sleep for the given time.

Secure Enclave power configuration (``senc``)
=============================================

``senc get_default_run_cfg`` / ``senc get_default_off_cfg``
   Print the current run / off power profile.

``senc set_default_run_cfg`` / ``senc set_default_off_cfg``
   Apply the default run / off power profile.

``senc set_run_cfg ...`` / ``senc set_off_cfg ...``
   Set individual profile fields (power domains, DCDC voltage, clock
   sources, memory blocks, ...). See ``senc <command> -h`` and ``aipm.h``.

``senc clocks_set_divider ...``
   Set a clock divider through the Secure Enclave.
