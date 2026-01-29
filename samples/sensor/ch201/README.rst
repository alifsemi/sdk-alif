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
        [00:00:00.417,000] <inf> CH201_TOF: Device 0x9a98 name is ch201@29

        [00:00:00.455,000] <inf> CH201_TOF:     Object Range --> 0.223593 m, Amplitude --> 13981

        [00:00:01.459,000] <inf> CH201_TOF:     Object Range --> 0.221031 m, Amplitude --> 12765

        [00:00:02.463,000] <inf> CH201_TOF:     Object Range --> 0.221250 m, Amplitude --> 12678

        [00:00:03.468,000] <inf> CH201_TOF:     Object Range --> 0.221718 m, Amplitude --> 12918

        [00:00:04.472,000] <inf> CH201_TOF:     Object Range --> 0.221375 m, Amplitude --> 12182

        [00:00:05.476,000] <inf> CH201_TOF:     Object Range --> 0.220937 m, Amplitude --> 12983

        [00:00:06.480,000] <inf> CH201_TOF:     Object Range --> 0.223718 m, Amplitude --> 13207

        <repeats endlessly>
