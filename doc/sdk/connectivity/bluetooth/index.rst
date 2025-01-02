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

With Alif's software development kit it's possible to use two different BLE host layers, the one included on Zephyr RTOS and the one provided as ROM code library.
We refer to Alif's ROM code BLE host layer as |**Alif_BLE**| from now on to make distinction to the Zephyr's BLE host layer.
When working with the Zephyr's BLE host layer implementation you should refer to the Zephyr documentation.

For BLE audio use cases Alif provides an LC3 codec as ROM code library - similarly to |**Alif_BLE**|. We refer on the codec as |**Alif_LC3**| from now on.

The |**Alif_BLE**|'s and |**Alif_LC3**|'s APIs are part of Alif's HAL layer. Samples demonstrating usage of the forementioned libraries are part of the SDK.
A list of samples given here is not exhaustive. Bear in mind that samples demonstrating usage of Zephyr's BLE host layer are found in the Zephyr's own samples-directory.

.. code-block:: console

   samples/bluetooth/
   ├── le_audio
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
