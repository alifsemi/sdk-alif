.. _shell-test-application:

Test application for BLE and 802.15.4
#####################################

Overview
********

Shell application with networking support with openthread and BLE shell commands.

Requirements
************

- Alif Balletto Development Kit

Optinally can also be used with Alif Ensemble Development Kit without BLE/15.4 connectivity

Automatically starting the OpenThread
*************************************

Enable automatic start of the OpenThread stack

.. code-block:: console

   west build alif/applications/testapp -- -DCONFIG_OPENTHREAD_MANUAL_START=n

Including the EthosU shell commands
***********************************

To build the sample, you first need to pull in the optional dependencies by running the following commands:

.. code-block:: console

   west config manifest.group-filter -- +optional
   west update

Build the application use the following commands:

.. code-block:: console

   west build alif/applications/testapp -- -DEXTRA_CONF_FILE=overlay-ethosu.conf

Starting OpenThread manually
****************************

To start openThread manually using commands issue the following commands.

.. code-block:: console

   ot ifconfig up
   ot thread start


Default settings when starting with automatic can be enabled by making new dataset and saving it.
This needs to be done only once if storage is not cleared.

.. code-block:: console

   ot dataset init new
   ot dataset channel 11
   ot dataset networkkey 00112233445566778899aabbccddeeff
   ot dataset panid 0x1234
   ot dataset meshlocalprefix fd11:22:: /64
   ot dataset activetimestamp 221212
   ot dataset networkname OpenThreadDemo
   ot dataset commit active

OpenThread Sleepy end device evaluate
*************************************

To build the sample with OpenThread Sleepy end device support with automatic start:

.. code-block:: console

   west build -- -DCONFIG_OPENTHREAD_MANUAL_START=n -DEXTRA_CONF_FILE=overlay-sed.conf

BLE radio and pinmux autostart
*******************

Automatically initialize the ES0 BLE radio on boot without manual shell commands.
This is useful for automated testing scenarios.

To enable autostart in prj.conf:

.. code-block:: ini

   CONFIG_ALIF_PWR_AUTOSTART=y
   # Optional: use pinmux_b instead of default pinmux_a
   CONFIG_ALIF_PWR_AUTOSTART_HCI_PINMUX_B=y

Or enable via build command:

.. code-block:: console

   west build alif/applications/testapp -- \
     -DCONFIG_ALIF_PWR_AUTOSTART=y \
     -DCONFIG_ALIF_PWR_AUTOSTART_HCI_PINMUX_B=y

This automatically executes the equivalent of:

.. code-block:: console

   pwr start --hpa
   pwr hci [--pinmux_b]

The feature is disabled by default (CONFIG_ALIF_PWR_AUTOSTART=n).