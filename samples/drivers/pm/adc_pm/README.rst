.. _alif-adc-pm-sample:

Alif ADC Power Management Demo
##############################

Overview
********

This sample demonstrates ADC operation combined with Zephyr power management
on Alif RTSS cores. It reads the on-chip temperature sensor (ADC channel 6)
before and after PM state transitions to verify that ADC continues to work
across:

* **PM_STATE_RUNTIME_IDLE**: Light sleep with quick wakeup (CPU clock gating via WFI)
* **PM_STATE_SUSPEND_TO_IDLE**: CPU sleep with IWIC, devices remain active
* **PM_STATE_SUSPEND_TO_RAM (S2RAM)**: Deep sleep with retention (HE core, TCM boot)

  * Substate 0 (STANDBY)
  * Substate 1 (STOP)

* **PM_STATE_SOFT_OFF**: Deepest sleep, no retention, full reset on wakeup

The available deep-sleep states follow the same HE/HP and TCM/MRAM rules as
the :ref:`alif-pm-states-sample` sample.

Requirements
************

* Alif Ensemble or Balletto development board
* ADC instance 0 enabled (temperature sensor on channel 6)
* RTC peripheral enabled for wakeup
* SE Services for power profile configuration

Supported Boards
****************

* alif_e7_dk_rtss_he
* alif_e1c_dk_rtss_he
* alif_b1_dk_rtss_he
* alif_e8_dk_rtss_he
* alif_e7_dk_rtss_hp
* alif_e8_dk_rtss_hp

Building and Running
********************

Build for HE core (TCM boot with retention):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/samples/drivers/pm/adc_pm \
       -S pm-system-off-he \
       -DCONFIG_FLASH_BASE_ADDRESS=0x0 \
       -DCONFIG_FLASH_LOAD_OFFSET=0x0 \
       -DCONFIG_FLASH_SIZE=256

Build for HE core (MRAM boot):

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_he \
       ../alif/samples/drivers/pm/adc_pm \
       -S pm-system-off-he

Build for HP core:

.. code-block:: console

   west build -p auto -b alif_e7_dk/ae722f80f55d5xx/rtss_hp \
       ../alif/samples/drivers/pm/adc_pm \
       -S pm-system-off-hp

Flash the binary using SE Tools. See :ref:`programming_an_application` for details.

Sample Output
*************

.. code-block:: console

   *** Booting Zephyr OS ***
   [00:00:00.004,000] <inf> adc_pm: alif_e7_dk RTSS_HE (TCM boot): PM states demo (RUNTIME_IDLE, SUSPEND_TO_IDLE, S2RAM)
   [00:00:00.016,000] <inf> adc_pm: POWER STATE SEQUENCE:
   [00:00:00.022,000] <inf> adc_pm:   1. PM_STATE_RUNTIME_IDLE
   [00:00:00.029,000] <inf> adc_pm:   2. PM_STATE_SUSPEND_TO_IDLE
   [00:00:00.036,000] <inf> adc_pm:   3. PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY)
   [00:00:00.044,000] <inf> adc_pm:   4. PM_STATE_SUSPEND_TO_RAM (substate 1: STOP)
   [00:00:00.053,000] <inf> adc_pm:   5. (SOFT_OFF skipped - TCM boot, using retention)
   [00:00:00.062,000] <inf> adc_pm:  0xXXX
   [00:00:00.068,000] <inf> adc_pm: Current temp 21.2 C
   [00:00:00.074,000] <inf> adc_pm: Enter RUNTIME_IDLE sleep for (18000000 microseconds)
   [00:00:18.083,000] <inf> adc_pm: Exited from RUNTIME_IDLE sleep
   [00:00:18.089,000] <inf> adc_pm: Enter PM_STATE_SUSPEND_TO_IDLE for (4000 microseconds)
   [00:00:18.104,000] <inf> adc_pm: Exited from PM_STATE_SUSPEND_TO_IDLE
   [00:00:18.111,000] <inf> adc_pm: Enter PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) for (20000000 microseconds)
   [00:00:38.127,000] <inf> adc_pm: Current temp 21.2 C
   [00:00:38.133,000] <inf> adc_pm: === Resumed from PM_STATE_SUSPEND_TO_RAM (substate 0: STANDBY) ===

Notes
*****

* Disconnect the debugger before testing; it prevents cores from entering OFF states
* The temperature reading is only valid for 12-bit ADC output on channel 6
* **SUSPEND_TO_IDLE** requires ``CONFIG_CORTEX_M_SYSTICK_LPM_TIMER_COUNTER`` and
  the ``suspend_idle`` node enabled in the overlay
* See the :ref:`alif-pm-states-sample` README for sleep durations and retention details
