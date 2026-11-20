.. _alif-m55-he-a32-pm-sample:

Alif M55-HE A32 Cluster Power-Off
#################################

Overview
********

This sample runs on RTSS-HE. It first asks Linux / TF-A to power down the
A32 cluster over non-secure MHU1, then uses the same Zephyr PM path as
:ref:`alif-pm-states-sample` to enter ``PM_STATE_SUSPEND_TO_RAM`` STANDBY
so ``es1_ppu`` can drop.

Sequence:

1. Wait for the Linux MHU1 client, send ``M55_PERIPH_OFF_REQ``, and ACK
   ``A32_GOING_OFF`` from TF-A.
2. Confirm SE PD-9 (``PD2_APPS``) is off.
3. Unlock S2RAM, keep IWIC locked, and sleep longer than the STANDBY
   min-residency so the idle thread applies the standby off-profile.

``sys_poweroff()`` is not used: it skips PM notifiers, so the SE off-profile
would not be applied and ``es1_ppu`` would stay on.

Requirements
************

* Alif Ensemble board with A32 + M55-HE
* Linux claiming non-secure HE MHU1 (``alif,a32-m55-shutdown`` tx/rx)
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

Captured on secure MHU pair 0. A pair-1 image prints ``MHU1``.

.. code-block:: console

   *** Booting Zephyr OS ***
   [00:00:00.004,000] <inf> m55_he_a32_pm: alif_e8_dk RTSS_HE (TCM boot): A32 off then HE S2RAM
   [00:00:00.013,000] <inf> a32_mhu: waiting 20 s for Linux MHU0 client
   [00:00:00.020,000] <inf> a32_mhu: 20 s left
   [00:00:01.025,000] <inf> a32_mhu: 19 s left
   [00:00:02.030,000] <inf> a32_mhu: 18 s left
   [00:00:03.035,000] <inf> a32_mhu: 17 s left
   [00:00:04.040,000] <inf> a32_mhu: 16 s left
   [00:00:05.045,000] <inf> a32_mhu: 15 s left
   [00:00:06.050,000] <inf> a32_mhu: 14 s left
   [00:00:07.055,000] <inf> a32_mhu: 13 s left
   [00:00:08.060,000] <inf> a32_mhu: 12 s left
   [00:00:09.065,000] <inf> a32_mhu: 11 s left
   [00:00:10.070,000] <inf> a32_mhu: 10 s left
   [00:00:11.075,000] <inf> a32_mhu: 9 s left
   [00:00:12.080,000] <inf> a32_mhu: 8 s left
   [00:00:13.085,000] <inf> a32_mhu: 7 s left
   [00:00:14.090,000] <inf> a32_mhu: 6 s left
   [00:00:15.095,000] <inf> a32_mhu: 5 s left
   [00:00:16.100,000] <inf> a32_mhu: 4 s left
   [00:00:17.105,000] <inf> a32_mhu: 3 s left
   [00:00:18.110,000] <inf> a32_mhu: 2 s left
   [00:00:19.115,000] <inf> a32_mhu: 1 s left
   [00:00:20.120,000] <inf> a32_mhu: sending 0xa320ff10 on MHU0 Ch0
   [00:00:20.126,000] <inf> a32_mhu: RX MHU0 Ch0 = 0xa320ff11
   [00:00:20.132,000] <inf> a32_mhu: got M55_PERIPH_OFF_REQ_ACK; waiting for PD9 OFF
   [00:00:20.140,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:20.247,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:20.354,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:20.461,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:20.568,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:20.675,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:20.782,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:20.889,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:20.996,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:21.103,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:21.210,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:21.317,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:21.424,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:21.531,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:21.638,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:21.745,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:21.852,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:21.959,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:22.066,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:22.173,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:22.280,000] <inf> a32_mhu: GET_RUN domains=0x3ed PD9(APSS)=ON
   [00:00:22.342,000] <inf> a32_mhu: RX MHU0 Ch0 = 0xa320ff01
   [00:00:22.588,000] <inf> a32_mhu: A32/PD9 OFF confirmed
   [00:00:22.593,000] <inf> m55_he_a32_pm: POWER STATE SEQUENCE:
   [00:00:22.599,000] <inf> m55_he_a32_pm:   1. A32/PD9 OFF
   [00:00:22.605,000] <inf> m55_he_a32_pm:   2. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:22.614,000] <inf> m55_he_a32_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (6000000 microseconds)
   [00:00:28.626,000] <inf> m55_he_a32_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===

Notes
*****

* Disconnect the debugger; it can hold cores out of OFF.
* Keep MHU opcodes in ``include/a32_going_off.h`` in sync with TF-A
  ``alif_mhu.h``.
* The ``pm-a32-off-he`` snippet enables S2RAM STANDBY, RTC idle timer,
  and the DTS off-profiles.
