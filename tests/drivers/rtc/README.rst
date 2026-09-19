.. _alarm_sample:

Counter Alarm Tests
#####################

Overview
********
This test suite validates the Real-Time Counter (RTC)
driver using the Zephyr Counter API, covering alarm configuration,
timing accuracy, cancellation, and error handling.

LPRTC wakeup from SOFT_OFF / S2RAM is a separate ``test_rtc_suspend_wake``
suite (like ``counter_basic``). Enable it with ``-S rtc-suspend-wake``.

.. note::
   In case of 1Hz frequency (RTC for example), precision is 1 second.
   Therefore, the sample output may differ in 1 second


Requirements
************

This sample requires the support of a timer IP compatible with alarm setting.

Building and Running
********************
By Default The Testcode runs for MRAM

   .. code-block:: console

      rm -rf build; west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/tests/drivers/rtc

To enable wrap mode and prescaler tests (prescaler 16384 -> 2 Hz,
1 tick = 0.5 s):

   .. code-block:: console

      rm -rf build; west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/tests/drivers/rtc -S rtc-wrap-prescaler

Change ``prescaler`` in ``snippets/rtc-wrap-prescaler/alif_rtc_wrap_prescaler.overlay``
to test other rates, for example ``32768`` (1 Hz, 1 s) or ``8192`` (4 Hz, 0.25 s).

To verify LPRTC on both 32 kHz sources (LFXO and LFRC):

   .. code-block:: console

      rm -rf build; west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/tests/drivers/rtc -S rtc-clk-source

The test reads ``runp.aon_clk_src`` via ``se_service_get_run_cfg()``
(LFRC or LFXO), runs a 2 s alarm on the current source, switches only
``aon_clk_src`` with ``se_service_set_run_cfg()``, and runs the alarm
again. Other run-profile fields are left unchanged.

To test LPRTC wakeup from SOFT_OFF (MRAM) or S2RAM (TCM):

   .. code-block:: console

      rm -rf build; west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp ../alif/tests/drivers/rtc -S rtc-suspend-wake

Once the build completes, you can flash the application to the board using:

   .. code-block:: console

      west flash
