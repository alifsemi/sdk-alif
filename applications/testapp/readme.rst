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
