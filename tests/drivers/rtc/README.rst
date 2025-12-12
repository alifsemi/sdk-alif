.. _alarm_sample:

Counter Alarm Tests
#####################

Overview
********
This tests  provides an example of alarm testcases using counter API.
It sets an alarm with an initial delay of 2 seconds. At each alarm
expiry, a new alarm is configured with a delay multiplied by 2.

.. note::
   In case of 1Hz frequency (RTC for example), precision is 1 second.
   Therefore, the sample output may differ in 1 second

Requirements
************

This sample requires the support of a timer IP compatible with alarm setting.

References
**********

- :ref:`disco_l475_iot1_board`

Building and Running
********************
By Default The Testcode runs for TCM
1. To Build for MRAM, Run the below Command
	rm -rf build; west build -b alif_e7_dk_rtss_he Zephyr_tests/tests/drivers/alif_RTC  -- -G "Unix Makefiles" -DMRAM=ON

2. To Run for OSPI, Run the below Command

        rm -rf build; west build -b alif_e7_dk_rtss_he Zephyr_tests/tests/drivers/alif_RTC  -- -G "Unix Makefiles" -DOSPI=ON
*************************************************************************
				NOTE
*************************************************************************

Sample Output
=============

 .. code-block:: console

    Counter alarm sample

    Set alarm in 2 sec
    !!! Alarm !!!
    Now: 2
    Set alarm in 4 sec
    !!! Alarm !!!
    Now: 6
    Set alarm in 8 sec
    !!! Alarm !!!
    Now: 14
    Set alarm in 16 sec
    !!! Alarm !!!
    Now: 30

    <repeats endlessly>
