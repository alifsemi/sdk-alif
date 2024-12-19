.. _zas-connection-ble:

Alif Bluetooth Documentation
############################

.. figure:: /_static/alif_ble_stack.drawio.png

With Alif's software development kit it's possible to use two different BLE host layers, the one included on Zephyr RTOS and the one provided as ROM code library.
We refer to Alif's ROM code BLE host layer as |**Alif_BLE**| from now on to make distinction to the Zephyr's BLE host layer.
When working with the Zephyr's BLE host layer implementation you should refer to the Zephyr documentation.

For BLE audio use cases Alif provides an LC3 codec as ROM code library - similarly to |**Alif_BLE**|. We refer on the codec as |**Alif_LC3**| from now on.

.. toctree::
   :maxdepth: 4
   :caption: Contents

   samples.rst
   sample.rst
   audiosource.rst
   audiosink.rst
   rom_abi.rst
