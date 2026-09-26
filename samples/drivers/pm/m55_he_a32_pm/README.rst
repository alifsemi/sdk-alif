.. _alif-m55-he-a32-pm-sample:

Alif M55-HE A32 Cluster Power-Off
#################################

Overview
********

This sample runs on RTSS-HE. It first asks Linux / TF-A to power down the
A32 cluster over MHU0, then uses the same Zephyr PM path as
:ref:`alif-pm-states-sample` to enter ``PM_STATE_SUSPEND_TO_RAM`` STANDBY
so ``es1_ppu`` can drop.

Sequence:

1. Wait for the Linux MHU0 client, send ``M55_PERIPH_OFF_REQ``, and ACK
   ``A32_GOING_OFF`` from TF-A.
2. Confirm SE PD-9 (``PD2_APPS``) is off.
3. Unlock S2RAM, keep IWIC locked, and sleep longer than the STANDBY
   min-residency so the idle thread applies the standby off-profile.

``sys_poweroff()`` is not used: it skips PM notifiers, so the SE off-profile
would not be applied and ``es1_ppu`` would stay on.

Requirements
************

* Alif Ensemble board with A32 + M55-HE
* Linux claiming HE MHU0 (rxdb2/txdb2)
* TF-A PSCI ``SYSTEM_OFF`` that sends ``A32_GOING_OFF``
* HE TCM boot (S2RAM retention)
* RTC wakeup (``rtc0``)

Building and Running
********************

.. code-block:: console

   west build -p auto -b alif_e8_dk/ae822fa0e5597xx0/rtss_he \
       ../alif/samples/drivers/pm/m55_he_a32_pm \
       -S pm-a32-off-he \
       -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
       -DCONFIG_FLASH_SIZE=256

Flash the HE binary, boot Linux on A32, then reset HE. See
:ref:`programming_an_application` for programming details.

Sample Output
*************

.. code-block:: console

   *** Booting Zephyr OS ***
   [00:00:00.004,000] <inf> m55_he_a32_pm: alif_e8_dk RTSS_HE (TCM boot): A32 off then HE S2RAM
   [00:00:00.020,000] <inf> a32_mhu: waiting 20 s for Linux MHU0 client
   [00:00:20.030,000] <inf> a32_mhu: sending 0xa320ff10 on MHU0 Ch0
   [00:00:20.040,000] <inf> a32_mhu: RX MHU0 Ch0 = 0xa320ffa1
   [00:00:20.050,000] <inf> a32_mhu: A32/PD9 OFF confirmed
   [00:00:20.060,000] <inf> m55_he_a32_pm: POWER STATE SEQUENCE:
   [00:00:20.066,000] <inf> m55_he_a32_pm:   1. A32/PD9 OFF
   [00:00:20.072,000] <inf> m55_he_a32_pm:   2. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:40.090,000] <inf> m55_he_a32_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===

Notes
*****

* Disconnect the debugger; it can hold cores out of OFF.
* Keep MHU opcodes in ``include/a32_going_off.h`` in sync with TF-A
  ``alif_mhu.h``.
* The ``pm-a32-off-he`` snippet enables S2RAM STANDBY, RTC idle timer,
  and the DTS off-profiles.
