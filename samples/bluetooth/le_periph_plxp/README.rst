.. _bluetooth-periph-plxp-sample:

BLE Pulse Oximeter Sample
#########################

Overview
********

Application to demonstrate the use of the Pulse Oximeter BLE profile.

Requirements
************

* Alif Balletto Development Kit

Building and Running
********************

This sample can be found under :zephyr_file:`samples/bluetooth/le_periph_plxp` in the
sdk-alif tree.

Build the sample:

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he alif/samples/bluetooth/le_periph_plxp

PTS Testing Mode
****************

For Profile Test Specification (PTS) qualification testing, enable the IUT (Implementation
Under Test) interface to use button controls for manual feature indication triggering.

Enable PTS testing mode by adding to ``prj.conf``:

.. code-block:: kconfig

   CONFIG_IUT_TESTER_ENABLED=y

Or build with the configuration flag:

.. code-block:: console

   west build -b alif_b1_dk/ab1c1f4m51820ph0/rtss_he alif/samples/bluetooth/le_periph_plxp -- -DCONFIG_IUT_TESTER_ENABLED=y

When enabled:

* Button 0 triggers PLX Features indication (for bonded/paired devices)
* The ``test_button.overlay`` device tree overlay is automatically loaded
* Useful for PTS test cases requiring manual control like ``PLXS/SEN/SGGIT/ISFC/BV-08-C``

Features
********

* BLE Secure Connections pairing support
* Continuous PLX measurement notifications
* PLX Features indication on reconnection for bonded devices
* Bond data persistence (RAM-based during power cycle)
* Manual features indication via button (PTS testing mode)