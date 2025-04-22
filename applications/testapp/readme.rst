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

Automatically starting the Openthread
*************************************

Enable automatic start of the openthread stack

.. code-block:: console

   west build -p always -b alif_b1_dk_rtss_he alif/applications/testapp -- -DCONFIG_OPENTHREAD_MANUAL_START=n

Including the EthosU shell commands
***********************************

To build the sample, you first need to pull in the optional dependencies by running the following commands:

.. code-block:: console

   west config manifest.group-filter -- +optional
   west update

Build the application use the following commands:

.. code-block:: console

   west build -p always -b alif_b1_dk_rtss_he alif/applications/testapp -- -DEXTRA_CONF_FILE=overlay-ethosu.conf

Starting Openthread manually
****************************

To start openthread manually using commands issue the following commands.

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
