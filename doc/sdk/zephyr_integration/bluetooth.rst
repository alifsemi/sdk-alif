..
    Bluetooth integration with Zephyr

.. _zas-zephyr-integration-bluetooth:

Bluetooth Integration
######################

This section describes how the Alif SDK integrates with Zephyr's Bluetooth stack.

Overview
********

The Alif SDK provides a custom Bluetooth implementation (|Alif_BLE|).

.. note::

   Bluetooth Host options in the Alif SDK:

   * Zephyr BLE: best for code reuse and portability across Zephyr-based SDKs. Leverages Zephyr's upstream APIs, docs, and ecosystem (see :ref:`Zephyr Bluetooth <zephyr:bluetooth>`).
   * Alif BLE (CevaWave, ROM-based): optimized for power and reduced memory footprint because the host stack resides in ROM.

   Choose the option that best fits your product goals. When using Zephyr BLE, refer to the Zephyr documentation (see :ref:`Zephyr Bluetooth <zephyr:bluetooth>`). When using Alif BLE, refer to the Alif SDK samples and HAL APIs.

Alif BLE
********

Configuration
-------------

To use Alif's Bluetooth stack with Zephyr, add the following to your project's ``prj.conf`` file:

.. code-block:: kconfig

    # Alif's BLE stack
    CONFIG_BT=y
    CONFIG_BT_CUSTOM=y

For audio applications using LC3 codec:

.. code-block:: kconfig

    # Audio
    CONFIG_ALIF_ROM_LC3_CODEC=y
    CONFIG_ALIF_BLE_AUDIO=y

Limitations
-----------

* Some Zephyr Bluetooth features may not be available in the Alif implementation
* The |Alif_BLE| stack has specific power and performance characteristics that may differ from other Bluetooth implementations

Zephyr BLE
**********

To use Zephyr's Bluetooth stack instead of |Alif_BLE|, configure your project's ``prj.conf`` file with standard Zephyr Bluetooth options:

.. code-block:: kconfig

    # Zephyr's BLE stack
    CONFIG_BT=y
    CONFIG_BT_PERIPHERAL=y
    CONFIG_BT_CENTRAL=y
    CONFIG_BT_DEVICE_NAME="Zephyr Device"

For specific Bluetooth features, refer to the Zephyr Bluetooth documentation (see :ref:`Zephyr Bluetooth <zephyr:bluetooth>`) for available configuration options.

**Note:** Do not enable ``CONFIG_BT_CUSTOM`` when using Zephyr's Bluetooth stack. The ``CONFIG_BT_CUSTOM`` option is only for |Alif_BLE|.
