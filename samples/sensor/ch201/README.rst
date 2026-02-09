.. _ch201:

CH201: TDK Invense Time of Fligh Sensor device
##########################################

Description
***********

This sample application periodically measures the Range and Amplitude
displaying the values on the console.

Wiring
*******

This sample uses an external breakout for the sensor. A devicetree
overlay must be provided to identify the I2C bus and GPIO
used to control the sensor.

Building and Running
********************

This project outputs sensor data to the console. It requires a CH201 sensor.
It should work with any platform featuring a I2C peripheral interface.
It does not work on QEMU.
In this example below the :ref:`alif_e8_ak/ae822fa0e5597xx0/rtss_hp` board is used.

After providing a devicetree overlay that specifies the sensor location,
build this sample app using:

For Alif boards:

.. zephyr-app-commands::
   :zephyr-app: samples/sensor/ch201
   :board: alif_e8_ak/ae822fa0e5597xx0/rtss_hp
   :goals: build
   :gen-args: -S alif-ak

Sample Output
=============

.. code-block:: console
        [00:00:00.003,000] <inf> CH201_TOF:     *****CH201 Range Sensor Demo*****

        [00:00:00.010,000] <inf> CH201_TOF:     SonicLib version: 4.7.1

        [00:00:00.016,000] <inf> CH201_TOF:     Starting CH201 group...

        [00:00:01.313,000] <inf> CH201_TOF:     Object Range --> 0.201000 m, Amplitude --> 24377

        [00:00:02.313,000] <inf> CH201_TOF:     Object Range --> 0.201000 m, Amplitude --> 24338

        [00:00:03.313,000] <inf> CH201_TOF:     Object Range --> 0.201218 m, Amplitude --> 24505

        [00:00:04.313,000] <inf> CH201_TOF:     Object Range --> 0.200343 m, Amplitude --> 24528

        [00:00:05.313,000] <inf> CH201_TOF:     Object Range --> 0.201000 m, Amplitude --> 24712

        <repeats endlessly>
