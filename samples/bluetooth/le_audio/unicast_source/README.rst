.. _bluetooth-unicast-source-sample:

BLE Unicast Audio Source Sample
#################################

Overview
********

This sample demonstrates the LE unicast audio source (client) use-case.

The central device name can be configured using `CONFIG_BLE_DEVICE_NAME`.

Peripheral device name can be configured using `CONFIG_BLE_PERIPHERAL_NAME` configuration option.
This is used to choose the correct unicast audio server for a connection.

Sample automatically discovers available unicast audio server's capabilities (PAC and ASCS) and
chooses the one that best matches the codec configuration. Stream count is limited to two (stereo LEFT and RIGHT channels).
Mono mode can be used as well if the peripheral reports only a single channel (ASE).

Note: Currently the sampling frequency is fixed at compile time and connot be changed at runtime.
This can be changed by modifying `sample-rate` in overlay :zephyr_file:`samples/bluetooth/le_audio/unicast_source/boards/alif_b1_dk_rtss_he.overlay`
 and recompiling the sample.

Building and Running
********************

This sample can be found under :zephyr_file:`samples/bluetooth/le_audio/unicast_source` in the sdk-alif tree.

See :ref:`Alif bluetooth samples section <alif-bluetooth-samples>` for details.
