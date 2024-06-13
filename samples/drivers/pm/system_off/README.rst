.. _alif-system-off-sample:

Alif System Off demo
#####################

Overview
********

This sample can be used for basic power measurement and as an example of
subsystem off of RTSS cores in Alif Ensemble. The functional behavior is:

* Display the last reset/wakeup reason
* Set RTC for 20sec which make sure system goes to idle task(Normal sleep)
* Wait for the RTC interrupt
* Set the RTC again for 20sec and turn the system off
* System reboots once the RTC interrupt triggers and the above steps continue

Note: If using a USB hub to connect the UART, it is advised to set the
BOOT_DELAY to make sure UART logs are not missed in the PC after reset.


This sample is specific to single subsystem. For the SoC to transition
to global states(IDLE/STANDBY/STOP), it requires voting from all the remaining
subsystem in the SoC.

Flashing the binary:
User should use the SETOOLS package which can be downloaded from our website
for flashing.
