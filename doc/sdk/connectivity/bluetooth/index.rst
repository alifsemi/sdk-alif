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

With Alif's software development kit it's possible to use two different BLE host layers:

* Zephyr's Bluetooth Host stack ("Zephyr BLE") (see :ref:`Zephyr Bluetooth <zephyr:bluetooth>`)
* |**Alif_BLE**| Host stack provided as a ROM code library ("Alif BLE")

.. note::

   These are two alternative options. Choose the one that best fits your product goals:

   * Zephyr BLE: best for code reuse and portability across Zephyr-based SDKs. Leverages Zephyr's upstream APIs, docs, and ecosystem.
   * |**Alif_BLE**| (ROM): optimized for power. The host stack runs from ROM, reducing flash/RAM footprint and typically lowering power consumption for BLE use cases.

When working with Zephyr's BLE host implementation you should refer to the Zephyr documentation (see :ref:`Zephyr Bluetooth <zephyr:bluetooth>`).

For BLE audio use cases Alif provides |**Alif_LC3**| codec as a ROM code library.

The |**Alif_BLE**| and |**Alif_LC3**| APIs are part of Alif's HAL layer. Samples demonstrating usage of these libraries are part of the SDK.
Bear in mind that samples demonstrating the Zephyr BLE host stack are found in Zephyr's own samples directory.

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
