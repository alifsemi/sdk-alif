.. _zas-connection-ble:

Bluetooth - |Alif_BLE|
######################

.. figure:: /images/alif_ble_stack.drawio.png

.. toctree::
   :maxdepth: 1
   :caption: BLE

   audio.rst
   rom_abi.rst
   sample_basic_profile.rst

With Alif's software development kit it's possible to use two different BLE host layers, the one included on Zephyr RTOS and |**Alif_BLE**| provided as ROM code library.
When working with the Zephyr's BLE host layer implementation you should refer to the Zephyr documentation.

For BLE audio use cases Alif provides |**Alif_LC3**| codec as ROM code library.

The |**Alif_BLE**|'s and |**Alif_LC3**|'s APIs are part of Alif's HAL layer. Samples demonstrating usage of the forementioned libraries are part of the SDK.
A list of samples given here is not exhaustive. Bear in mind that samples demonstrating usage of Zephyr's BLE host layer are found in the Zephyr's own samples-directory.

.. code-block:: console

   samples/bluetooth/
   ├── le_audio
   │   ├── broadcast_sink
   │   └── broadcast_source
   ├── le_periph
   ├── le_periph_blinky
   ├── le_periph_blps
   ├── le_periph_cgms
   ├── le_periph_cpps
   ├── le_periph_cscps
   ├── le_periph_glps
   ├── le_periph_hr
   ├── le_periph_htpt
   ├── le_periph_past
   ├── le_periph_plxp
   ├── le_periph_prxp
   ├── le_periph_rscps
   ├── le_periph_ws
   ├── mesh_light_bulb
   └── smp_svr
   ...

   samples/lc3/
   └── lc3_codec
