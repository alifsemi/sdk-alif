.. _bluetooth-unicast-sink-sample:

BLE Unicast Audio Sink Sample
#################################

Overview
********

This sample demonstrates the LE unicast audio sink (server) use-case.

The peripheral device name can be configured using `CONFIG_BLE_DEVICE_NAME`.

Sample setup required codec configuration capabilities (PAC) and Audio Stream Control Services (ASCS)
which can be used.

This sample can be tested with an Android, PC or Alif's B1 DevKit.

Note: Currently the sampling frequency is fixed at compile time and connot be changed at runtime.
This can be changed by modifying `sample-rate` in overlay :zephyr_file:`samples/bluetooth/le_audio/unicast_sink/boards/alif_b1_dk_rtss_he.overlay`
 and recompiling the sample.

Building and Running
********************

This sample can be found under :zephyr_file:`samples/bluetooth/le_audio/unicast_sink` in the sdk-alif tree.

See :ref:`Alif bluetooth samples section <alif-bluetooth-samples>` for details.
