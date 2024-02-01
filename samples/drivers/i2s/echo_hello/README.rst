.. _i2s_echo_hello_sample:

I2S Echo Hello Sample
###############

Overview
********

This sample demonstrates how to use an I2S driver in a simple processing of
an audio stream. It configures first the tx stream and send a hello from sample
file and starts the RX streams. whatever receives in the RX part through a
microphone sends to tx and played on a loudspeaker.

Requirements
************


Building and Running
********************

The code can be found in :zephyr_file:`samples/drivers/i2s/echo_hello`.

To build and flash the application:

.. zephyr-app-commands::
   :zephyr-app: samples/drivers/i2s/echo_hello
   :board: alif_e7_dk_rtss_hp or alif_e7_dk_rtss_he
   :goals: build flash
   :compact:
