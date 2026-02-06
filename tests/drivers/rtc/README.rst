.. _alarm_sample:

Counter Alarm Tests
#####################

Overview
********
This test suite validates the Real-Time Counter (RTC)
driver using the Zephyr Counter API, covering alarm configuration,
timing accuracy, cancellation, and error handling.

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

To enable wrap mode:

   .. code-block:: console

      rm -rf build; west build -b alif_e7_dk/ae722f80f55d5xx/rtss_he ../alif/tests/drivers/rtc -S rtc-wrap

Once the build completes, you can flash the application to the board using:

   .. code-block:: console

      west flash
