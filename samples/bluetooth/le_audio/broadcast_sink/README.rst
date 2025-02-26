.. _bluetooth-broadcast-sink-sample:

BLE Audio Broadcast Sink Sample
###############################

Overview
********

This sample demonstrates the LE audio broadcast sink use-case.

It scans for broadcast sources and synchronises with the first compatible source found. To be
compatible, a source device must use a 48 kHz sampling rate and 10 ms frames. Otherwise most
settings should be supported.

Both stereo and mono sources are supported. In case of a mono source, the single channel will be
duplicated on both left and right I2S output streams.

Building and Running
********************

This sample can be found under :zephyr_file:`samples/bluetooth/broadcast_sink` in the
sdk-alif tree.

See :ref:`Alif bluetooth samples section <alif-bluetooth-samples>` for details.
